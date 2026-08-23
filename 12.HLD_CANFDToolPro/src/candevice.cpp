#include "candevice.h"

#include <QByteArray>
#include <QFile>
#include <cstring>

CanDevice::CanDevice() : dll_(QStringLiteral("ControlCANFD")) {}

CanDevice::~CanDevice()
{
    close();
}

bool CanDevice::load(QString *error)
{
    if (dll_.isLoaded() || dll_.load())
        return true;
    if (error)
        *error = QStringLiteral("加载 ControlCANFD.dll 失败：%1").arg(dll_.errorString());
    return false;
}

bool CanDevice::open(quint32 deviceType, quint32 deviceIndex, QString *error)
{
    std::unique_lock<std::shared_mutex> lock(deviceMutex_);
    if (device_ != nullptr)
        return true;
    if (!load(error))
        return false;

    auto fn = resolve<DEVICE_HANDLE (*)(UINT, UINT, UINT)>("ZCAN_OpenDevice");
    device_ = fn ? fn(deviceType, deviceIndex, 0) : nullptr;
    if (device_ == nullptr) {
        if (error)
            *error = QStringLiteral("设备打开失败，请检查设备类型、索引号和 USB 驱动。");
        return false;
    }

    deviceType_ = deviceType;
    deviceIndex_ = deviceIndex;

    // 获取属性接口（失败不致命，但波特率/滤波等设置将不可用）
    auto getProp = resolve<IProperty *(*)(DEVICE_HANDLE)>("GetIProperty");
    property_ = getProp ? getProp(device_) : nullptr;
    return true;
}

void CanDevice::close()
{
    std::unique_lock<std::shared_mutex> lock(deviceMutex_);
    for (auto &ch : channels_) {
        if (ch) {
            auto reset = resolve<UINT (*)(CHANNEL_HANDLE)>("ZCAN_ResetCAN");
            if (reset)
                reset(ch);
            ch = nullptr;
        }
    }
    if (property_) {
        auto release = resolve<UINT (*)(IProperty *)>("ReleaseIProperty");
        if (release)
            release(property_);
        property_ = nullptr;
    }
    if (device_) {
        auto closeFn = resolve<UINT (*)(DEVICE_HANDLE)>("ZCAN_CloseDevice");
        if (closeFn)
            closeFn(device_);
        device_ = nullptr;
    }
}

bool CanDevice::setCanfdStandard(int standard, QString *error)
{
    Q_UNUSED(error);
    canfdStandard_ = quint32(standard);
    return true;
}

bool CanDevice::setAbitBaud(quint32 baud, QString *error)
{
    Q_UNUSED(error);
    abitBaud_ = baud;
    return true;
}

bool CanDevice::setDbitBaud(quint32 baud, QString *error)
{
    Q_UNUSED(error);
    dbitBaud_ = baud;
    return true;
}

bool CanDevice::setCustomBaudrate(const QString &rate, QString *error)
{
    if (!property_ || !property_->SetValue) {
        if (error) *error = QStringLiteral("设备属性接口不可用");
        return false;
    }
    for (int ch = 0; ch < 2; ++ch) {
        const QByteArray path = (QString::number(ch) + "/baud_rate_custom").toUtf8();
        if (property_->SetValue(path.constData(), rate.toUtf8().constData()) != 1) {
            if (error) *error = QStringLiteral("设置自定义波特率失败！");
            return false;
        }
    }
    return true;
}

bool CanDevice::setResistanceEnable(bool enable, QString *error)
{
    auto fn = resolve<UINT (*)(DEVICE_HANDLE, UINT, UINT)>("ZCAN_SetResistanceEnable");
    for (int ch = 0; ch < 2; ++ch) {
        if (!fn || fn(device_, UINT(ch), enable ? 1U : 0U) != STATUS_OK) {
            if (error) *error = QStringLiteral("设置终端电阻失败！");
            return false;
        }
    }
    return true;
}

bool CanDevice::initChannel(int channel, bool canfd, int mode, QString *error)
{
    std::unique_lock<std::shared_mutex> lock(deviceMutex_);
    if (device_ == nullptr) {
        if (error) *error = QStringLiteral("设备未打开！");
        return false;
    }
    if (channel < 0 || channel > 1) {
        if (error) *error = QStringLiteral("无效通道");
        return false;
    }

    // 与 v1.21.0 ZCan::startChannel 完全相同：每个通道紧邻 init 前设置参数。
    auto setAbit = resolve<UINT (*)(DEVICE_HANDLE, UINT, UINT)>("ZCAN_SetAbitBaud");
    auto setDbit = resolve<UINT (*)(DEVICE_HANDLE, UINT, UINT)>("ZCAN_SetDbitBaud");
    auto setStandard = resolve<UINT (*)(DEVICE_HANDLE, UINT, UINT)>("ZCAN_SetCANFDStandard");
    if (!setAbit || !setDbit || !setStandard
        || setAbit(device_, UINT(channel), abitBaud_) != STATUS_OK
        || setDbit(device_, UINT(channel), dbitBaud_) != STATUS_OK
        || setStandard(device_, UINT(channel), canfdStandard_) != STATUS_OK) {
        if (error) *error = QStringLiteral("通道%1参数设置失败").arg(channel + 1);
        return false;
    }

    auto fn = resolve<CHANNEL_HANDLE (*)(DEVICE_HANDLE, UINT, ZCAN_CHANNEL_INIT_CONFIG *)>("ZCAN_InitCAN");
    if (!fn) {
        if (error) *error = QStringLiteral("无法解析 ZCAN_InitCAN");
        return false;
    }

    ZCAN_CHANNEL_INIT_CONFIG config;
    std::memset(&config, 0, sizeof(config));
    config.can_type = canfd ? TYPE_CANFD : TYPE_CAN;
    if (canfd)
        config.canfd.mode = static_cast<BYTE>(mode);
    else
        config.can.mode = static_cast<BYTE>(mode);

    CHANNEL_HANDLE h = fn(device_, static_cast<UINT>(channel), &config);
    if (h == nullptr) {
        if (error) *error = QStringLiteral("CAN 初始化失败！");
        return false;
    }
    channels_[channel] = h;
    return true;
}

bool CanDevice::startChannel(int channel, QString *error)
{
    std::unique_lock<std::shared_mutex> lock(deviceMutex_);
    if (channel < 0 || channel > 1 || channels_[channel] == nullptr) {
        if (error) *error = QStringLiteral("通道未初始化");
        return false;
    }
    auto fn = resolve<UINT (*)(CHANNEL_HANDLE)>("ZCAN_StartCAN");
    if (!fn || fn(channels_[channel]) != STATUS_OK) {
        if (error) *error = QStringLiteral("CAN 启动失败！");
        return false;
    }
    return true;
}

bool CanDevice::resetChannel(int channel, QString *error)
{
    std::unique_lock<std::shared_mutex> lock(deviceMutex_);
    if (channel < 0 || channel > 1 || channels_[channel] == nullptr)
        return true;
    auto fn = resolve<UINT (*)(CHANNEL_HANDLE)>("ZCAN_ResetCAN");
    if (!fn || fn(channels_[channel]) != STATUS_OK) {
        if (error) *error = QStringLiteral("CAN 复位失败！");
        return false;
    }
    return true;
}

bool CanDevice::isChannelStarted(int channel) const
{
    return channel >= 0 && channel <= 1 && channels_[channel] != nullptr;
}

bool CanDevice::clearFilter(int channel, QString *error)
{
    if (channel < 0 || channel > 1 || channels_[channel] == nullptr) {
        if (error) *error = QStringLiteral("通道未初始化");
        return false;
    }
    auto fn = resolve<UINT (*)(CHANNEL_HANDLE)>("ZCAN_ClearFilter");
    if (!fn || fn(channels_[channel]) != STATUS_OK) {
        if (error) *error = QStringLiteral("清除滤波失败！");
        return false;
    }
    return true;
}

bool CanDevice::setFilterMode(int channel, int mode, QString *error)
{
    if (channel < 0 || channel > 1 || channels_[channel] == nullptr) {
        if (error) *error = QStringLiteral("通道未初始化");
        return false;
    }
    auto fn = resolve<UINT (*)(CHANNEL_HANDLE, UINT)>("ZCAN_SetFilterMode");
    if (!fn || fn(channels_[channel], static_cast<UINT>(mode)) != STATUS_OK) {
        if (error) *error = QStringLiteral("设置滤波模式失败！");
        return false;
    }
    return true;
}

bool CanDevice::setFilterStartID(int channel, quint32 id, QString *error)
{
    if (channel < 0 || channel > 1 || channels_[channel] == nullptr) {
        if (error) *error = QStringLiteral("通道未初始化");
        return false;
    }
    auto fn = resolve<UINT (*)(CHANNEL_HANDLE, UINT)>("ZCAN_SetFilterStartID");
    if (!fn || fn(channels_[channel], id) != STATUS_OK) {
        if (error) *error = QStringLiteral("设置滤波起始 ID 失败！");
        return false;
    }
    return true;
}

bool CanDevice::setFilterEndID(int channel, quint32 id, QString *error)
{
    if (channel < 0 || channel > 1 || channels_[channel] == nullptr) {
        if (error) *error = QStringLiteral("通道未初始化");
        return false;
    }
    auto fn = resolve<UINT (*)(CHANNEL_HANDLE, UINT)>("ZCAN_SetFilterEndID");
    if (!fn || fn(channels_[channel], id) != STATUS_OK) {
        if (error) *error = QStringLiteral("设置滤波结束 ID 失败！");
        return false;
    }
    return true;
}

bool CanDevice::ackFilter(int channel, QString *error)
{
    if (channel < 0 || channel > 1 || channels_[channel] == nullptr) {
        if (error) *error = QStringLiteral("通道未初始化");
        return false;
    }
    auto fn = resolve<UINT (*)(CHANNEL_HANDLE)>("ZCAN_AckFilter");
    if (!fn || fn(channels_[channel]) != STATUS_OK) {
        if (error) *error = QStringLiteral("滤波生效失败！");
        return false;
    }
    return true;
}

bool CanDevice::applyFilter(int channel, int mode, quint32 start, quint32 end, QString *error)
{
    if (!clearFilter(channel, error))
        return false;
    if (!setFilterMode(channel, mode, error))
        return false;
    if (!setFilterStartID(channel, start, error))
        return false;
    if (!setFilterEndID(channel, end, error))
        return false;
    return ackFilter(channel, error);
}

bool CanDevice::sendCan(int channel, quint32 id, bool extended, bool remote,
                        const QByteArray &data, int sendType, QString *error)
{
    if (channel < 0 || channel > 1 || channels_[channel] == nullptr) {
        if (error) *error = QStringLiteral("通道未启动！");
        return false;
    }
    auto fn = resolve<UINT (*)(CHANNEL_HANDLE, ZCAN_Transmit_Data *, UINT)>("ZCAN_Transmit");
    if (!fn) {
        if (error) *error = QStringLiteral("无法解析 ZCAN_Transmit");
        return false;
    }
    ZCAN_Transmit_Data tx;
    std::memset(&tx, 0, sizeof(tx));
    tx.frame.can_id = MAKE_CAN_ID(id, extended ? 1 : 0, remote ? 1 : 0, 0);
    tx.frame.can_dlc = static_cast<BYTE>(qMin<int>(data.size(), CAN_MAX_DLEN));
    std::memcpy(tx.frame.data, data.constData(), static_cast<size_t>(tx.frame.can_dlc));
    tx.transmit_type = static_cast<UINT>(sendType);

    const UINT sent = fn(channels_[channel], &tx, 1);
    if (sent != 1) {
        if (error) *error = QStringLiteral("CAN 数据发送失败");
        return false;
    }
    return true;
}

bool CanDevice::sendCanFd(int channel, quint32 id, bool extended, bool remote, bool brs,
                          const QByteArray &data, int sendType, QString *error)
{
    if (channel < 0 || channel > 1 || channels_[channel] == nullptr) {
        if (error) *error = QStringLiteral("通道未启动！");
        return false;
    }
    auto fn = resolve<UINT (*)(CHANNEL_HANDLE, ZCAN_TransmitFD_Data *, UINT)>("ZCAN_TransmitFD");
    if (!fn) {
        if (error) *error = QStringLiteral("无法解析 ZCAN_TransmitFD");
        return false;
    }
    ZCAN_TransmitFD_Data tx;
    std::memset(&tx, 0, sizeof(tx));
    tx.frame.can_id = MAKE_CAN_ID(id, extended ? 1 : 0, remote ? 1 : 0, 0);
    tx.frame.len = static_cast<BYTE>(qMin<int>(data.size(), CANFD_MAX_DLEN));
    tx.frame.flags = brs ? CANFD_BRS : 0;
    std::memcpy(tx.frame.data, data.constData(), static_cast<size_t>(tx.frame.len));
    tx.transmit_type = static_cast<UINT>(sendType);

    const UINT sent = fn(channels_[channel], &tx, 1);
    if (sent != 1) {
        if (error) *error = QStringLiteral("CANFD 数据发送失败");
        return false;
    }
    return true;
}

quint32 CanDevice::getReceiveNum(int channel, int type)
{
    if (channel < 0 || channel > 1 || channels_[channel] == nullptr)
        return 0;
    auto fn = resolve<UINT (*)(CHANNEL_HANDLE, BYTE)>("ZCAN_GetReceiveNum");
    return fn ? fn(channels_[channel], static_cast<BYTE>(type)) : 0;
}

quint32 CanDevice::receiveCan(int channel, ZCAN_Receive_Data *out, quint32 cap)
{
    if (channel < 0 || channel > 1 || channels_[channel] == nullptr)
        return 0;
    auto num = resolve<UINT (*)(CHANNEL_HANDLE, BYTE)>("ZCAN_GetReceiveNum");
    auto fn = resolve<UINT (*)(CHANNEL_HANDLE, ZCAN_Receive_Data *, UINT, int)>("ZCAN_Receive");
    const quint32 count = num ? qMin<quint32>(num(channels_[channel], TYPE_CAN), cap) : 0;
    return count && fn ? fn(channels_[channel], out, count, 0) : 0;
}

quint32 CanDevice::receiveCanFd(int channel, ZCAN_ReceiveFD_Data *out, quint32 cap)
{
    if (channel < 0 || channel > 1 || channels_[channel] == nullptr)
        return 0;
    auto num = resolve<UINT (*)(CHANNEL_HANDLE, BYTE)>("ZCAN_GetReceiveNum");
    auto fn = resolve<UINT (*)(CHANNEL_HANDLE, ZCAN_ReceiveFD_Data *, UINT, int)>("ZCAN_ReceiveFD");
    const quint32 count = num ? qMin<quint32>(num(channels_[channel], TYPE_CANFD), cap) : 0;
    return count && fn ? fn(channels_[channel], out, count, 0) : 0;
}

bool CanDevice::getDeviceInfo(ZCAN_DEVICE_INFO *info, QString *error)
{
    std::shared_lock<std::shared_mutex> lock(deviceMutex_);
    if (device_ == nullptr) {
        if (error) *error = QStringLiteral("设备未打开！");
        return false;
    }
    auto fn = resolve<UINT (*)(DEVICE_HANDLE, ZCAN_DEVICE_INFO *)>("ZCAN_GetDeviceInf");
    if (!fn || fn(device_, info) != STATUS_OK) {
        if (error) *error = QStringLiteral("设备信息获取失败！");
        return false;
    }
    return true;
}

bool CanDevice::firmwareUpdate(const QString &filePath, QString *error)
{
    if (device_ == nullptr) {
        if (error) *error = QStringLiteral("设备未打开！");
        return false;
    }
    auto fn = resolve<DWORD (*)(DWORD, DWORD, const char *)>("Firmware_Update");
    if (!fn) {
        if (error) *error = QStringLiteral("无法解析 Firmware_Update");
        return false;
    }
    const QByteArray path = QFile::encodeName(filePath);
    if (fn(deviceType_, deviceIndex_, path.constData()) != STATUS_OK) {
        if (error) *error = QStringLiteral("固件升级失败，请重新插拔设备并重启软件！");
        return false;
    }
    return true;
}


bool CanDevice::isOnline()
{
    std::shared_lock<std::shared_mutex> lock(deviceMutex_);
    if (device_ == nullptr)
        return false;
    auto fn = resolve<UINT (*)(DEVICE_HANDLE)>("ZCAN_IsDeviceOnLine");
    if (!fn)
        return false;
    return fn(device_) == STATUS_ONLINE;
}

bool CanDevice::probePresent(quint32 deviceType, quint32 deviceIndex)
{
    std::unique_lock<std::shared_mutex> lock(deviceMutex_);
    if (!load(nullptr))
        return false;
    auto openFn = resolve<DEVICE_HANDLE (*)(UINT, UINT, UINT)>("ZCAN_OpenDevice");
    if (!openFn)
        return false;
    DEVICE_HANDLE h = openFn(deviceType, deviceIndex, 0);
    if (h == nullptr)
        return false;
    auto closeFn = resolve<UINT (*)(DEVICE_HANDLE)>("ZCAN_CloseDevice");
    if (closeFn)
        closeFn(h);
    return true;
}
