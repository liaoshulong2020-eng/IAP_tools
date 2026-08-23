#ifndef CANDEVICE_H
#define CANDEVICE_H

#include "inc/controlcanfd.h"

#include <QLibrary>
#include <QString>
#include <array>
#include <mutex>
#include <shared_mutex>
#include <cstdint>

// 波特率档位（仲裁域 / 数据域），数值 = 实际 bit/s
struct BaudRates {
    static constexpr int abitCount = 12;
    static constexpr quint32 abit[abitCount] = {
        1000000, 800000, 500000, 250000, 125000, 100000,
        50000, 40000, 20000, 10000, 5000, 33000
    };
    static constexpr int dbitCount = 8;
    static constexpr quint32 dbit[dbitCount] = {
        5000000, 4000000, 2000000, 1000000, 800000, 500000, 250000, 125000
    };
};

// ControlCANFD.dll 的设备封装（运行时动态加载）。
// 一个 CanDevice 对应一个物理设备，内部管理 0/1 两个通道句柄。
class CanDevice
{
public:
    CanDevice();
    ~CanDevice();

    bool load(QString *error = nullptr);

    // 打开设备；成功返回 true。device_type 通常为 ZCAN_USBCANFD_200U(41)
    bool open(quint32 deviceType, quint32 deviceIndex, QString *error = nullptr);
    bool isOpen() const { return device_ != nullptr; }
    bool isOnline();   // 已打开设备是否在线（被拔出时返回 false）
    bool probePresent(quint32 deviceType, quint32 deviceIndex);  // 探测设备是否插入
    void close();

    // 参数设置（须在 init 之前）
    bool setCanfdStandard(int standard, QString *error = nullptr);          // 0=ISO 1=BOSCH
    bool setAbitBaud(quint32 baud, QString *error = nullptr);
    bool setDbitBaud(quint32 baud, QString *error = nullptr);
    bool setCustomBaudrate(const QString &rate, QString *error = nullptr);
    bool setResistanceEnable(bool enable, QString *error = nullptr);

    // 初始化 + 启动通道
    bool initChannel(int channel, bool canfd = true, int mode = 0, QString *error = nullptr);
    bool startChannel(int channel, QString *error = nullptr);
    bool resetChannel(int channel, QString *error = nullptr);
    bool isChannelStarted(int channel) const;

    // 滤波（init 之后、start 之前，成组调用）
    bool clearFilter(int channel, QString *error = nullptr);
    bool setFilterMode(int channel, int mode, QString *error = nullptr);     // 0=标准帧 1=扩展帧
    bool setFilterStartID(int channel, quint32 id, QString *error = nullptr);
    bool setFilterEndID(int channel, quint32 id, QString *error = nullptr);
    bool ackFilter(int channel, QString *error = nullptr);
    // 一次性配置一组滤波（清除→模式→起止→生效）
    bool applyFilter(int channel, int mode, quint32 start, quint32 end, QString *error = nullptr);

    // 收发
    bool sendCan(int channel, quint32 id, bool extended, bool remote,
                 const QByteArray &data, int sendType = 0, QString *error = nullptr);
    bool sendCanFd(int channel, quint32 id, bool extended, bool remote, bool brs,
                   const QByteArray &data, int sendType = 0, QString *error = nullptr);

    quint32 getReceiveNum(int channel, int type);                     // type: TYPE_CAN/TYPE_CANFD
    quint32 receiveCan(int channel, ZCAN_Receive_Data *out, quint32 cap);
    quint32 receiveCanFd(int channel, ZCAN_ReceiveFD_Data *out, quint32 cap);

    // 设备信息
    bool getDeviceInfo(ZCAN_DEVICE_INFO *info, QString *error = nullptr);

    // 固件升级（接口定义见 controlcanfd.h）
    bool firmwareUpdate(const QString &filePath, QString *error = nullptr);

private:
    template <typename T>
    T resolve(const char *name) { return reinterpret_cast<T>(dll_.resolve(name)); }

    QLibrary dll_;
    DEVICE_HANDLE device_ = nullptr;
    IProperty *property_ = nullptr;
    std::array<CHANNEL_HANDLE, 2> channels_{ nullptr, nullptr };
    // 收发允许并发；设备打开、关闭和复位仍独占，防止句柄释放竞态。
    mutable std::shared_mutex deviceMutex_;
    quint32 deviceType_ = ZCAN_USBCANFD_200U;
    quint32 deviceIndex_ = 0;
    quint32 abitBaud_ = 125000;
    quint32 dbitBaud_ = 5000000;
    quint32 canfdStandard_ = 0;
};

#endif // CANDEVICE_H
