#include "mainwindow.h"

#include "baudrate.h"
#include "mytablemodel.h"
#include "dialogs/opendlg.h"
#include "dialogs/initdlg.h"
#include "dialogs/paradialog.h"
#include "dialogs/filterdialog.h"
#include "dialogs/siftdialog.h"
#include "dialogs/valuedialog.h"
#include "dialogs/infodialog.h"
#include "dialogs/listsenddlg.h"
#include "dialogs/filedlg.h"
#include "dialogs/savefiledlg.h"
#include "dialogs/datadlg.h"
#include "dialogs/updatedlg.h"

#include <QAction>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QDesktopServices>
#include <QDialog>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include <QTableView>
#include <QTabWidget>
#include <QTableWidget>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QProgressBar>
#include <QScrollBar>
#include <limits>
#include <memory>
#include <QRegularExpression>
#include <QFormLayout>
#include <QGridLayout>
#include <QTime>
#include <QUrl>
#include <QVBoxLayout>
#include <algorithm>

namespace {

// 解析 "00 01 02" / "0x..." 形式的十六进制串
QByteArray parseHexBytes(const QString &text, bool *ok = nullptr)
{
    QString s = text;
    s.remove(QLatin1Char(' '));
    s.remove(QLatin1Char('\t'));
    if (s.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive))
        s = s.mid(2);
    if (ok) *ok = !s.isEmpty() && (s.size() % 2 == 0);
    return QByteArray::fromHex(s.toLatin1());
}

QString formatHexBytes(const QByteArray &data)
{
    return QString::fromLatin1(data.toHex(' ').toUpper());
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , recvThread_(&device_)
    , sendThread_{ sendThread(&device_), sendThread(&device_) }
{
    // RX 批次由工作线程通过 QueuedConnection 进入主线程，必须先注册完整类型。
    qRegisterMetaType<ModelItem>("ModelItem");
    qRegisterMetaType<QList<ModelItem>>("QList<ModelItem>");
    model_ = new MyTableModel(this);
    buildUi();

    setWindowTitle(QStringLiteral("HLD_CANFDToolPro v1.1 双Bank版"));

    // 连接收发信号
    connect(&recvThread_, &recvThread::recvData, this, &MainWindow::onRecvData, Qt::QueuedConnection);
    for (int i = 0; i < 2; ++i) {
        connect(&sendThread_[i], &sendThread::endSend, this, &MainWindow::onEndSend);
        connect(&sendThread_[i], &sendThread::sendFailed, this, &MainWindow::onSendFailed);
    }

    // 统计刷新定时器
    connect(&rateTimer_, &QTimer::timeout, this, &MainWindow::refreshRate);
    rateElapsed_.start();
    // 监控允许 50 ms 甚至更短的查询周期。队列取数必须明显快于查询周期，
    // 否则会先积累多帧 TX、再积累多帧 RX，列表和记录文件看起来就会成组。
    rateTimer_.setTimerType(Qt::PreciseTimer);
    rateTimer_.start(10);

    // 与 v1.21.0 相同：自动查询由 Qt 主线程定时器直接触发，不放入接收线程。
    queryTimer_.setTimerType(Qt::PreciseTimer);
    queryTimer_.setInterval(config_.intValue(QStringLiteral("monitor/queryInterval"), 1000));
    connect(&queryTimer_, &QTimer::timeout, this, [this] {
        if (!device_.isOpen()) return;
        sendCommand(0x01, {}, false, true);
        ++querySendCount_;
    });

    // RX 由 10 ms 主定时器非阻塞批量读取。该路径与硬件探针一致，
    // 避免厂商 DLL 的通道句柄在额外工作线程中始终返回空缓冲区。

    // 设备插拔检测定时器（低频轮询）
    connect(&devicePollTimer_, &QTimer::timeout, this, &MainWindow::pollDeviceStatus);
    devicePollTimer_.start(1500);
    pollDeviceStatus();


    loadConfigIntoUi();
    QMetaObject::connectSlotsByName(this);
    appendLog(QStringLiteral("HLD_CANFDToolPro v1.1 双Bank版启动。"));
}

MainWindow::~MainWindow()
{
    recvThread_.stop();
    recvThread_.wait(2000);
    for (int i = 0; i < 2; ++i) {
        sendThread_[i].stop();
        sendThread_[i].wait(2000);
    }
    if (monitorSaving_) monitorCsvWriter_.stop();
    monitorCsvWriter_.join();
    device_.close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 先停止定时器和收发线程，再关闭 CAN 设备，确保设备句柄被正确释放。
    queryTimer_.stop();
    devicePollTimer_.stop();

    recvThread_.stop();
    recvThread_.wait(2000);
    for (int i = 0; i < 2; ++i) {
        sendThread_[i].stop();
        sendThread_[i].wait(2000);
    }

    if (monitorSaving_) monitorCsvWriter_.stop();
    monitorCsvWriter_.join();
    if (realSaving_) {
        csvWriter_.stop();
        csvWriter_.join();
        realSaving_ = false;
    }

    device_.close();
    saveConfig();
    event->accept();
}

// ---------------------------------------------------------------------------
// 主界面
// ---------------------------------------------------------------------------
void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(central);

    // 顶部工具栏（单行）：打开设备 + 状态指示 + 常用功能
    auto *topBar = new QHBoxLayout();
    openDeviceBtn_ = new QPushButton(tr("打开设备"), central);
    connect(openDeviceBtn_, &QPushButton::clicked, this, &MainWindow::toggleDevice);
    deviceStatusLabel_ = new QLabel(tr("设备未打开"), central);
    topBar->addWidget(openDeviceBtn_);
    topBar->addWidget(deviceStatusLabel_);
    topBar->addSpacing(16);
    auto addBtn = [&](const QString &text, void (MainWindow::*slot)()) {
        auto *b = new QPushButton(text, central);
        connect(b, &QPushButton::clicked, this, slot);
        topBar->addWidget(b);
    };
    auto *canSettingsBtn = new QPushButton(tr("CAN设置"), central);
    canSettingsBtn->setMinimumWidth(105);
    auto *canSettingsMenu = new QMenu(canSettingsBtn);
    canSettingsMenu->addAction(tr("参数设置"), this, &MainWindow::openParameterSetting);
    canSettingsMenu->addAction(tr("滤波设置"), this, &MainWindow::openFilterSetting);
    canSettingsMenu->addSeparator();
    canSettingsMenu->addAction(tr("设备信息"), this, &MainWindow::openDeviceInfo);
    canSettingsMenu->addAction(tr("固件升级"), this, &MainWindow::openFirmwareUpgrade);
    canSettingsBtn->setMenu(canSettingsMenu);
    topBar->addWidget(canSettingsBtn);
    topBar->addStretch();
    addBtn(tr("关于"), &MainWindow::showAbout);
    rootLayout->addLayout(topBar);

    // 标签页
    auto *tabs = new QTabWidget(central);

    // ---- Tab 0：接收 ----
    auto *recvPage = new QWidget(tabs);
    auto *recvLayout = new QVBoxLayout(recvPage);

    // 双通道发送面板
    auto *chanRow = new QHBoxLayout();
    chanRow->addWidget(buildChannelPanel(0));
    chanRow->addWidget(buildChannelPanel(1));
    recvLayout->addLayout(chanRow);

    // 收发表 + 工具栏
    tableView_ = new QTableView(recvPage);
    tableView_->setObjectName(QStringLiteral("tableView"));
    tableView_->setModel(model_);
    tableView_->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView_->horizontalHeader()->setStretchLastSection(true);
    tableView_->verticalHeader()->setVisible(false);
    // v1.21 列表视觉：RX 阴影横向连续，不让单元格网格切成一段一段。
    tableView_->setShowGrid(false);
    tableView_->setStyleSheet(QStringLiteral(
        "QTableView { border:1px solid #d5d9df; background:#fff; }"
        "QTableView::item { border:0; padding:2px 6px; }"
        "QTableView::item:selected { background:#B9D7F5; color:#111; }"));

    auto *tableBar = new QHBoxLayout();
    realSaveCheck_ = new QCheckBox(tr("实时保存"), recvPage);
    connect(realSaveCheck_, &QCheckBox::toggled, this, &MainWindow::onRealSaveToggled);
    auto *cleanBtn = new QPushButton(tr("清空列表"), recvPage);
    cleanBtn->setObjectName(QStringLiteral("cleanListBtn"));
    checkBox_ = new QCheckBox(tr("暂停显示"), recvPage);
    checkBox_->setObjectName(QStringLiteral("checkBox"));
    mergeCheck_ = new QCheckBox(tr("合并列表数据"), recvPage);
    connect(mergeCheck_, &QCheckBox::toggled, this, &MainWindow::toggleMerge);
    auto *resetBtn = new QPushButton(tr("复位计数"), recvPage);
    connect(resetBtn, &QPushButton::clicked, this, &MainWindow::resetCount);
    tableBar->addWidget(cleanBtn);
    tableBar->addWidget(checkBox_);
    tableBar->addWidget(mergeCheck_);
    tableBar->addWidget(resetBtn);
    tableBar->addStretch();
    tableBar->addWidget(realSaveCheck_);
    recvLayout->addLayout(tableBar);
    recvLayout->addWidget(tableView_, 1);

    // 统计栏
    auto *statBar = new QHBoxLayout();
    ch1RxLabel_ = new QLabel(tr("通道1接收:0 (0帧/s)"), recvPage);
    ch1TxLabel_ = new QLabel(tr("通道1发送:0 (0帧/s)"), recvPage);
    ch2RxLabel_ = new QLabel(tr("通道2接收:0 (0帧/s)"), recvPage);
    ch2TxLabel_ = new QLabel(tr("通道2发送:0 (0帧/s)"), recvPage);
    statBar->addWidget(ch1RxLabel_);
    statBar->addWidget(ch1TxLabel_);
    statBar->addWidget(ch2RxLabel_);
    statBar->addWidget(ch2TxLabel_);
    statBar->addStretch();
    recvLayout->addLayout(statBar);

    // 日志
    logEdit_ = new QPlainTextEdit(recvPage);
    logEdit_->setReadOnly(true);
    logEdit_->setMaximumHeight(120);
    recvLayout->addWidget(logEdit_);

    tabs->addTab(recvPage, tr("通用"));
    tabs->addTab(makeMonitorPage(), tr("监控"));
    tabs->addTab(makeProtectPage(), tr("保护点"));
    tabs->addTab(makeParameterPage(), tr("参数"));
    tabs->addTab(makeIapPage(), tr("IAP升级"));

    rootLayout->addWidget(tabs, 1);

    setCentralWidget(central);
    resize(1200, 760);
}


QGroupBox *MainWindow::buildChannelPanel(int channel)
{
    const QString prefix = chPrefix(channel);
    const QString title = tr("通道 %1").arg(channel + 1);

    auto *group = new QGroupBox(title, this);
    auto *grid = new QGridLayout(group);

    ChannelPanel &p = ch_[channel];

    auto addLabel = [&](int row, int col, const QString &text) {
        grid->addWidget(new QLabel(text, group), row, col);
    };

    // 第一行：协议 / 帧类型 / 帧数据 / 发送方式
    addLabel(0, 0, tr("协议"));
    p.protocol = new QComboBox(group);
    p.protocol->setObjectName(prefix + QStringLiteral("ProtocolCombo"));
    p.protocol->addItems({ QStringLiteral("CAN"), QStringLiteral("CAN-FD") });
    grid->addWidget(p.protocol, 0, 1);

    addLabel(0, 2, tr("帧类型"));
    p.frameType = new QComboBox(group);
    p.frameType->setObjectName(prefix + QStringLiteral("FrameTypeCombo"));
    p.frameType->addItems({ tr("标准帧"), tr("扩展帧") });
    grid->addWidget(p.frameType, 0, 3);

    addLabel(0, 4, tr("帧数据"));
    p.frameData = new QComboBox(group);
    p.frameData->setObjectName(prefix + QStringLiteral("FrameDataCombo"));
    p.frameData->addItems({ tr("数据帧"), tr("远程帧") });
    grid->addWidget(p.frameData, 0, 5);

    addLabel(0, 6, tr("重发策略"));
    p.sendType = new QComboBox(group);
    p.sendType->setObjectName(prefix + QStringLiteral("SendTypeCombo"));
    p.sendType->addItems({ tr("失败自动重发"), tr("失败不重发") });
    p.sendType->setItemData(0, tr("遵循 CAN 正常机制：发送失败时由控制器自动重发"), Qt::ToolTipRole);
    p.sendType->setItemData(1, tr("每帧只尝试一次；发送失败后不自动重发"), Qt::ToolTipRole);
    p.sendType->setToolTip(tr("正常发送：失败后由 CAN 控制器自动重发\n"
                              "单次发送：每帧只尝试一次，失败后不重发"));
    grid->addWidget(p.sendType, 0, 7);

    // 第二行：ID / 数据
    addLabel(1, 0, tr("ID(0x)："));
    p.idEdit = new QLineEdit(group);
    p.idEdit->setObjectName(prefix + QStringLiteral("SendIDEdit"));
    grid->addWidget(p.idEdit, 1, 1, 1, 3);

    addLabel(1, 4, tr("长度"));
    p.sendLen = new QComboBox(group);
    p.sendLen->setObjectName(prefix + QStringLiteral("sendLenCombo"));
    for (int i = 0; i <= 8; ++i)
        p.sendLen->addItem(QString::number(i));
    p.sendLen->setCurrentIndex(8);
    grid->addWidget(p.sendLen, 1, 5);

    p.idAdd = new QCheckBox(tr("ID自增"), group);
    p.idAdd->setObjectName(prefix + QStringLiteral("IDAddChk"));
    grid->addWidget(p.idAdd, 1, 6, 1, 2);

    addLabel(2, 0, tr("数据"));
    p.dataEdit = new QLineEdit(group);
    p.dataEdit->setObjectName(prefix + QStringLiteral("SendDataEdit"));
    grid->addWidget(p.dataEdit, 2, 1, 1, 5);

    p.dataAdd = new QCheckBox(tr("数据自增"), group);
    p.dataAdd->setObjectName(prefix + QStringLiteral("DataAddChk"));
    grid->addWidget(p.dataAdd, 2, 6, 1, 2);

    // 第三行：次数 / 间隔 / 自增
    addLabel(3, 0, tr("次数"));
    p.countEdit = new QLineEdit(group);
    p.countEdit->setObjectName(prefix + QStringLiteral("SendCountEdit"));
    grid->addWidget(p.countEdit, 3, 1);

    addLabel(3, 2, tr("间隔(ms)"));
    p.invEdit = new QLineEdit(group);
    p.invEdit->setObjectName(prefix + QStringLiteral("SendInvEdit"));
    grid->addWidget(p.invEdit, 3, 3);
    auto *intervalApply = new QTimer(group);
    intervalApply->setSingleShot(true);
    intervalApply->setInterval(2000);
    connect(p.invEdit, &QLineEdit::textEdited, intervalApply, qOverload<>(&QTimer::start));
    connect(intervalApply, &QTimer::timeout, this, [this, channel] {
        bool ok = false;
        const int interval = ch_[channel].invEdit->text().toInt(&ok);
        if (ok && interval >= 0 && sendThread_[channel].isRunning())
            sendThread_[channel].setInterval(interval);
    });

    p.continuous = new QCheckBox(tr("持续发送"), group);
    p.continuous->setToolTip(tr("不限发送次数，直到点击停止"));
    grid->addWidget(p.continuous, 3, 4, 1, 2);
    connect(p.continuous, &QCheckBox::toggled, p.countEdit, &QWidget::setDisabled);

    // 第四行：普通发送 / 停止 / 序列发送
    p.sendBtn = new QPushButton(tr("发送"), group);
    p.sendBtn->setObjectName(prefix + QStringLiteral("SendBtn"));
    p.stopBtn = new QPushButton(tr("停止"), group);
    p.stopBtn->setObjectName(prefix + QStringLiteral("StopSendBtn"));
    auto *sequenceBtn = new QPushButton(tr("序列发送"), group);
    connect(sequenceBtn, &QPushButton::clicked, this, [this, channel] { openSequenceSend(channel); });
    grid->addWidget(p.sendBtn, 4, 0, 1, 3);
    grid->addWidget(p.stopBtn, 4, 3, 1, 3);
    grid->addWidget(sequenceBtn, 4, 6, 1, 2);

    return group;
}

// ---------------------------------------------------------------------------
// 自动连接槽
// ---------------------------------------------------------------------------
void MainWindow::on_pushButton_clicked() { openDeviceOperations(); }
void MainWindow::on_pushButton_2_clicked() { openDeviceInfo(); }
void MainWindow::on_pushButton_3_clicked() { openParameterSetting(); }
void MainWindow::on_pushButton_4_clicked() { openFilterSetting(); }

void MainWindow::on_cleanListBtn_clicked()
{
    model_->clear();
    appendLog(tr("清空列表"));
}

void MainWindow::on_checkBox_clicked(bool checked)
{
    pauseDisplay_ = checked;
}

void MainWindow::on_ch1SendBtn_clicked() { startSend(0); }
void MainWindow::on_ch2SendBtn_clicked() { startSend(1); }
void MainWindow::on_ch1StopSendBtn_clicked() { stopSend(0); }
void MainWindow::on_ch2StopSendBtn_clicked() { stopSend(1); }

void MainWindow::on_ch1ProtocolCombo_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    // CAN(0)/CANFD(1) 切换时，长度下拉范围不同
    const int max = ch_[0].protocol->currentIndex() == 1 ? 64 : 8;
    ch_[0].sendLen->blockSignals(true);
    ch_[0].sendLen->clear();
    for (int i = 0; i <= max; ++i)
        ch_[0].sendLen->addItem(QString::number(i));
    ch_[0].sendLen->setCurrentIndex(8);
    ch_[0].sendLen->blockSignals(false);
    setCh1SendData(8);
}

void MainWindow::on_ch2ProtocolCombo_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    const int max = ch_[1].protocol->currentIndex() == 1 ? 64 : 8;
    ch_[1].sendLen->blockSignals(true);
    ch_[1].sendLen->clear();
    for (int i = 0; i <= max; ++i)
        ch_[1].sendLen->addItem(QString::number(i));
    ch_[1].sendLen->setCurrentIndex(8);
    ch_[1].sendLen->blockSignals(false);
    setCh2SendData(8);
}

void MainWindow::on_ch1FrameDataCombo_currentIndexChanged(int index)
{
    Q_UNUSED(index);
}
void MainWindow::on_ch2FrameDataCombo_currentIndexChanged(int index)
{
    Q_UNUSED(index);
}

void MainWindow::on_ch1sendLenCombo_currentIndexChanged(int index)
{
    setCh1SendData(index);
}

void MainWindow::on_ch2sendLenCombo_activated(int index)
{
    setCh2SendData(index);
}

void MainWindow::on_ch1SendDataEdit_textEdited(const QString &text)
{
    Q_UNUSED(text);
}
void MainWindow::on_ch2SendDataEdit_textEdited(const QString &text)
{
    Q_UNUSED(text);
}
void MainWindow::on_ch1SendDataEdit_textChanged(const QString &text)
{
    Q_UNUSED(text);
}
void MainWindow::on_ch2SendDataEdit_textChanged(const QString &text)
{
    Q_UNUSED(text);
}

void MainWindow::on_tableView_clicked(const QModelIndex &index)
{
    Q_UNUSED(index);
}

void MainWindow::on_tableView_pressed(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    // 右键筛选菜单
    QMenu menu(this);
    QAction *filterAct = menu.addAction(tr("筛选"));
    QAction *showAllAct = menu.addAction(tr("显示全部"));
    menu.addSeparator();
    QAction *clearAllAct = menu.addAction(tr("删除全部筛选条件"));

    QAction *chosen = menu.exec(QCursor::pos());
    if (chosen == filterAct)
        SiftDialog::filterColumn(this, model_, index.column());
    else if (chosen == showAllAct)
        SiftDialog::showAllColumn(model_, index.column());
    else if (chosen == clearAllAct)
        SiftDialog::clearAllFilters(model_);
}

// ---------------------------------------------------------------------------
// 发送数据长度联动
// ---------------------------------------------------------------------------
void MainWindow::setCh1SendData(int len)
{
    bool ok = false;
    QByteArray data = parseHexBytes(ch_[0].dataEdit->text(), &ok);
    if (!ok)
        data.clear();
    data.resize(len);  // 不足补 0，超出截断
    ch_[0].dataEdit->setText(formatHexBytes(data));
}

void MainWindow::setCh2SendData(int len)
{
    bool ok = false;
    QByteArray data = parseHexBytes(ch_[1].dataEdit->text(), &ok);
    if (!ok)
        data.clear();
    data.resize(len);  // 不足补 0，超出截断
    ch_[1].dataEdit->setText(formatHexBytes(data));
}

// ---------------------------------------------------------------------------
// 菜单动作
// ---------------------------------------------------------------------------
void MainWindow::openDeviceOperations()
{
    initDlg dlg(&device_, this);
    dlg.exec();
}

void MainWindow::openDeviceInfo()
{
    INfDialog dlg(&device_, this);
    dlg.exec();
}

void MainWindow::openParameterSetting()
{
    const int ch = SiftDialog::chooseChannel(this);
    if (ch < 0)
        return;
    ParaDialog dlg(&device_, ch, this);
    dlg.exec();
}

void MainWindow::openFilterSetting()
{
    const int ch = SiftDialog::chooseChannel(this);
    if (ch < 0)
        return;
    FilterDialog dlg(&device_, ch, this);
    dlg.exec();
}

void MainWindow::openListSend()
{
    openSequenceSend(0);
}

void MainWindow::openSequenceSend(int channel)
{
    if (sequenceDlg_[channel]) {
        sequenceDlg_[channel]->show();
        sequenceDlg_[channel]->raise();
        sequenceDlg_[channel]->activateWindow();
        return;
    }

    auto *dlg = new listSendDlg(&device_, channel, ch_[channel].protocol->currentIndex() == 1, this);
    sequenceDlg_[channel] = dlg;
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(dlg, &QObject::destroyed, this, [this, channel] { sequenceDlg_[channel] = nullptr; });
    connect(dlg, &listSendDlg::sequenceStarted, this, [this](int ch, int count, bool loop) {
        appendLog(tr("通道%1 序列发送开始：%2帧，%3模式。")
                  .arg(ch + 1).arg(count).arg(loop ? tr("循环") : tr("单次")));
    });
    connect(dlg, &listSendDlg::frameSent, this, [this](const ModelItem &item) {
        onSendedData(QList<ModelItem>{ item });
        appendLog(tr("序列发送 通道%1：ID=0x%2，数据=%3")
                  .arg(item.channel + 1)
                  .arg(item.id, 0, 16)
                  .arg(QString::fromLatin1(item.data.toHex(' ').toUpper())));
    });
    connect(dlg, &listSendDlg::sequenceFinished, this, [this](int ch) {
        appendLog(tr("通道%1 序列单次发送完成。").arg(ch + 1));
    });
    connect(dlg, &listSendDlg::sequenceStopped, this, [this](int ch) {
        appendLog(tr("通道%1 序列发送已停止。").arg(ch + 1));
    });
    dlg->show();
}

void MainWindow::openFileSend()
{
    fileDlg dlg(&device_, this);
    dlg.exec();
}

void MainWindow::openRealSave()
{
    savefileDlg dlg(this);
    if (dlg.exec() != QDialog::Accepted || dlg.filePath().isEmpty())
        return;

    if (realSaving_) {
        csvWriter_.stop();
        realSaving_ = false;
    }

    QString err;
    if (!csvWriter_.start(dlg.filePath(),
                          QStringLiteral("序号,系统时间,通道,方向,ID,数据\n"),
                          &err)) {
        QMessageBox::warning(this, tr("警告"), tr("无法打开保存文件。%1").arg(err));
        return;
    }
    saveSeq_ = 0;
    realSaving_ = true;
    appendLog(tr("实时保存已开始：%1").arg(dlg.filePath()));
}

void MainWindow::openFirmwareUpgrade()
{
    UpdateDlg dlg(&device_, this);
    dlg.exec();
}

void MainWindow::clearLog()
{
    logEdit_->clear();
}

void MainWindow::resetCount()
{
    rxCount_[0] = rxCount_[1] = 0;
    txCount_[0] = txCount_[1] = 0;
    lastRx_[0] = lastRx_[1] = 0;
    lastTx_[0] = lastTx_[1] = 0;
    rxRate_[0] = rxRate_[1] = 0.0;
    txRate_[0] = txRate_[1] = 0.0;
    refreshRate();
}

void MainWindow::setListBufferCount()
{
    valueDialog dlg(tr("数据列表缓存"), tr("最大显示帧数："),
                    config_.intValue(QStringLiteral("listCount"), 1000), 1, 1000000, this);
    if (dlg.exec() == QDialog::Accepted) {
        const int n = dlg.value();
        model_->setLimit(n);
        config_.setValue(QStringLiteral("listCount"), QString::number(n));
        appendLog(tr("数据列表缓存已设置为 %1").arg(n));
    }
}

void MainWindow::toggleMerge(bool checked)
{
    model_->setMerge(checked);
    if (checked)
        model_->clear();   // 开启合并时清空已有滚动数据，避免淹没合并后的 ID
    config_.setValue(QStringLiteral("merge"), checked ? QStringLiteral("1") : QStringLiteral("0"));
}

void MainWindow::onRealSaveToggled(bool checked)
{
    if (checked) {
        // 自动生成保存路径并开始记录
        const QString dir = QCoreApplication::applicationDirPath() + QStringLiteral("/data");
        QDir().mkpath(dir);
        realSavePath_ = dir + QStringLiteral("/CANFD_")
                + QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_hhmmss"))
                + QStringLiteral(".csv");

        QString err;
        if (!csvWriter_.start(realSavePath_,
                              QStringLiteral("序号,系统时间,通道,方向,ID,数据\n"),
                              &err)) {
            QMessageBox::warning(this, tr("警告"), tr("无法开始保存。%1").arg(err));
            realSaveCheck_->blockSignals(true);
            realSaveCheck_->setChecked(false);
            realSaveCheck_->blockSignals(false);
            return;
        }
        saveSeq_ = 0;
        realSaving_ = true;
        appendLog(tr("实时保存已开始：%1").arg(realSavePath_));
    } else {
        if (!realSaving_)
            return;
        csvWriter_.stop();
        realSaving_ = false;
        appendLog(tr("实时保存已停止：%1").arg(realSavePath_));

        if (QMessageBox::question(this, tr("实时保存"),
                                  tr("是否打开已保存的数据文件？"))
            == QMessageBox::Yes) {
            csvWriter_.join();  // 确保写线程完成落盘后再打开
            QDesktopServices::openUrl(QUrl::fromLocalFile(realSavePath_));
        }
    }
}

void MainWindow::toggleDevice()
{
    if (device_.isOpen()) {
        device_.close();
        openDeviceBtn_->setText(tr("打开设备"));
        appendLog(tr("设备已关闭"));
    } else {
        QString error;
        if (!device_.open(ZCAN_USBCANFD_200U, 0, &error)) {
            QMessageBox::warning(this, tr("警告"), error);
            updateStatus();
            return;
        }
        // 应用配置波特率（默认 125k）
        ConfigManager cfg;
        const int abitIdx = qBound(0, cfg.intValue(QStringLiteral("ch1paramABIT1"), 6), ABIT_BAUD_COUNT - 1);
        const int dbitIdx = qBound(0, cfg.intValue(QStringLiteral("ch1paramABIT2"), 3), DBIT_BAUD_COUNT - 1);
        QString berr;
        if (!device_.setAbitBaud(ABIT_BAUDS[abitIdx].value, &berr))
            appendLog(tr("设置仲裁波特率失败：%1").arg(berr));
        if (!device_.setDbitBaud(DBIT_BAUDS[dbitIdx].value, &berr))
            appendLog(tr("设置数据波特率失败：%1").arg(berr));
        if (!device_.setCanfdStandard(cfg.intValue(QStringLiteral("ch1paramcanFDStandard"), 0), &berr))
            appendLog(tr("设置CANFD标准失败：%1").arg(berr));
        appendLog(tr("波特率：仲裁=%1bps 数据=%2bps").arg(ABIT_BAUDS[abitIdx].value).arg(DBIT_BAUDS[dbitIdx].value));

        for (int ch = 0; ch < 2; ++ch) {
            if (!device_.initChannel(ch, true, 0, &error) || !device_.startChannel(ch, &error))
                appendLog(tr("通道%1启动失败：%2").arg(ch + 1).arg(error));
        }
        openDeviceBtn_->setText(tr("关闭设备"));
        appendLog(tr("设备已打开（默认 125k）"));
    }
    updateStatus();
}

void MainWindow::updateStatus()
{
    if (device_.isOpen()) {
        const bool c1 = device_.isChannelStarted(0);
        const bool c2 = device_.isChannelStarted(1);
        deviceStatusLabel_->setText(tr("设备已打开  通道1:%1  通道2:%2")
                                    .arg(c1 ? tr("已启动") : tr("未启动"))
                                    .arg(c2 ? tr("已启动") : tr("未启动")));
        deviceStatusLabel_->setStyleSheet(QStringLiteral("color:#27ae60;font-weight:bold;"));
    } else if (devicePresent_) {
        deviceStatusLabel_->setText(tr("设备已插入（未打开）"));
        deviceStatusLabel_->setStyleSheet(QStringLiteral("color:#e67e22;font-weight:bold;"));
    } else {
        deviceStatusLabel_->setText(tr("设备未插入"));
        deviceStatusLabel_->setStyleSheet(QStringLiteral("color:#c0392b;font-weight:bold;"));
    }
}

void MainWindow::pollDeviceStatus()
{
    if (device_.isOpen()) {
        // 已打开：检测是否被拔出
        if (!device_.isOnline()) {
            appendLog(tr("检测到设备已拔出"));
            device_.close();
            openDeviceBtn_->setText(tr("打开设备"));
            devicePresent_ = false;
        }
    } else {
        // 未打开：探测是否已插入
        devicePresent_ = device_.probePresent(ZCAN_USBCANFD_200U, 0);
    }
    updateStatus();
}

void MainWindow::switchLanguage(const QString &langFile)
{
    config_.setValue(QStringLiteral("Language"), langFile);
    QMessageBox::information(this, tr("语言"),
                             tr("语言将在重启软件后生效。"));
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, tr("关于 HLD_CANFDToolPro"),
                       QStringLiteral("<div style='text-align:center'>"
                                      "<h2>HLD_CANFDToolPro</h2>"
                                      "<p><b>版本：v1.0</b></p>"
                                      "<p>USB CAN/CAN-FD 通信、分析与设备调试工具</p>"
                                      "<p><b>深圳中瀚蓝盾电源有限公司</b></p>"
                                      "<p>Copyright &copy; 2025 深圳中瀚蓝盾电源有限公司<br>"
                                      "All rights reserved.</p>"
                                      "</div>"));
}

void MainWindow::openBankManager()
{
    if (auto *existing = findChild<QDialog*>(QStringLiteral("bankManagerDialog"))) {
        existing->show();
        existing->raise();
        existing->activateWindow();
        return;
    }
    auto *dlg = new QDialog(this);
    dlg->setObjectName(QStringLiteral("bankManagerDialog"));
    dlg->setWindowTitle(tr("双 Bank 管理"));
    dlg->setWindowModality(Qt::NonModal);
    dlg->resize(680, 430);
    auto *root = new QVBoxLayout(dlg);
    auto *form = new QGridLayout;
    auto *target = new QComboBox(dlg); target->addItems({tr("LLC（包地址2）"), tr("PFC（包地址1）")});
    auto *channel = new QComboBox(dlg); channel->addItems({tr("通道1"), tr("通道2")});
    auto *canId = new QLineEdit(QStringLiteral("AA55"), dlg);
    form->addWidget(new QLabel(tr("目标设备："), dlg),0,0); form->addWidget(target,0,1);
    form->addWidget(new QLabel(tr("CAN通道："), dlg),0,2); form->addWidget(channel,0,3);
    form->addWidget(new QLabel(tr("IAP CAN ID："), dlg),1,0); form->addWidget(canId,1,1,1,3);
    root->addLayout(form);
    auto *status = new QLabel(tr("点击“刷新状态”读取当前物理 Bank、MAP、BMD 和镜像有效性。"), dlg);
    status->setWordWrap(true); root->addWidget(status);
    auto *buttons = new QGridLayout;
    auto add = [&](const QString &text,int row,int col){auto *b=new QPushButton(text,dlg);buttons->addWidget(b,row,col);return b;};
    auto *refresh=add(tr("刷新状态"),0,0); auto *verifyA=add(tr("校验 Bank A"),0,1); auto *verifyB=add(tr("校验 Bank B"),0,2);
    auto *switchA=add(tr("切换并试运行 A"),1,0); auto *switchB=add(tr("切换并试运行 B"),1,1);
    auto *confirm=add(tr("确认当前 Bank"),1,2); auto *rollback=add(tr("回滚到另一 Bank"),2,0);
    root->addLayout(buttons);
    auto *warning = new QLabel(tr("安全限制：目标 Bank 的 Bootloader 向量表、APP 向量表和 CRC 必须全部有效才能切换。切换后设备自动复位。"), dlg);
    warning->setWordWrap(true); warning->setStyleSheet(QStringLiteral("color:#b45309;")); root->addWidget(warning);
    auto *log = new QPlainTextEdit(dlg); log->setReadOnly(true); root->addWidget(log,1);
    QList<QPushButton*> actionButtons{refresh,verifyA,verifyB,switchA,switchB,confirm,rollback};
    auto runAction = [this,dlg,target,channel,canId,status,log,actionButtons](int action) {
        if (iap_.running()) { QMessageBox::information(dlg,tr("Bank管理"),tr("已有 IAP 操作正在执行。")); return; }
        bool ok=false; const uint32_t id=canId->text().trimmed().toUInt(&ok,16);
        if(!ok||id==0||id>0x1FFFFFFFU){QMessageBox::warning(dlg,tr("Bank管理"),tr("请输入合法的29位十六进制 CAN ID。"));return;}
        if(action>=4&&action<=5&&QMessageBox::warning(dlg,tr("切换 Bank"),tr("确认切换到 Bank %1 并立即复位设备？").arg(action==4?'A':'B'),QMessageBox::Yes|QMessageBox::No)!=QMessageBox::Yes)return;
        if(action==7&&QMessageBox::warning(dlg,tr("回滚 Bank"),tr("确认切换到另一有效 Bank 并立即复位设备？"),QMessageBox::Yes|QMessageBox::No)!=QMessageBox::Yes)return;
        for(auto *b:actionButtons) b->setEnabled(false);
        status->setText(tr("正在执行 Bank 操作…"));
        IapUpgrade::Options options; options.canId=id; options.targetAddress=uint8_t(target->currentIndex()?1:2); options.bankAction=action;
        const int ch=channel->currentIndex();
        iap_.start(options,[this,ch,id](const QByteArray &packet){for(int o=0;o<packet.size();o+=8){QString e;if(!device_.sendCan(ch,id,true,false,packet.mid(o,8),0,&e))return false;}return true;},
            [this,log,status](const QString &text){QMetaObject::invokeMethod(this,[log,status,text]{log->appendPlainText(QTime::currentTime().toString(QStringLiteral("hh:mm:ss "))+text);status->setText(text);},Qt::QueuedConnection);},
            [](int){},[this,status,actionButtons](bool success){QMetaObject::invokeMethod(this,[status,actionButtons,success]{for(auto *b:actionButtons)b->setEnabled(true);if(!success)status->setText(QObject::tr("操作失败：请检查 CAN、IAP ID 或目标 Bank 有效性。"));},Qt::QueuedConnection);});
    };
    connect(refresh,&QPushButton::clicked,dlg,[runAction]{runAction(1);}); connect(verifyA,&QPushButton::clicked,dlg,[runAction]{runAction(2);});
    connect(verifyB,&QPushButton::clicked,dlg,[runAction]{runAction(3);}); connect(switchA,&QPushButton::clicked,dlg,[runAction]{runAction(4);});
    connect(switchB,&QPushButton::clicked,dlg,[runAction]{runAction(5);}); connect(confirm,&QPushButton::clicked,dlg,[runAction]{runAction(6);});
    connect(rollback,&QPushButton::clicked,dlg,[runAction]{runAction(7);}); connect(dlg,&QDialog::finished,this,[this]{iap_.cancel();});
    dlg->show();
}

void MainWindow::openHelpFolder()
{
    const QString dir = QCoreApplication::applicationDirPath() + QStringLiteral("/help");
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

// ---------------------------------------------------------------------------
// 收发
// ---------------------------------------------------------------------------
void MainWindow::onRecvData(const QList<ModelItem> &items)
{
    for (const ModelItem &it : items) {
        ++rxCount_[it.channel];
        if (realSaving_)
            csvWriter_.append(csvRow(it));
        iap_.feed(it.id, it.data);
        try {
            updateMonitor(it);
        } catch (const std::exception &e) {
            appendLog(QStringLiteral("updateMonitor 异常: %1").arg(e.what()));
        } catch (...) {
            appendLog(QStringLiteral("updateMonitor 未知异常"));
        }
        try {
            parseProtocol(it);
        } catch (const std::exception &e) {
            appendLog(QStringLiteral("parseProtocol 异常: %1").arg(e.what()));
        } catch (...) {
            appendLog(QStringLiteral("parseProtocol 未知异常"));
        }
        if (monitorSaving_)
            monitorCsvWriter_.append(monitorCsvRow(it));
    }

    if (pauseDisplay_)
        return;
    const bool followBottom = tableView_->verticalScrollBar()->maximum() == 0
        || tableView_->verticalScrollBar()->value() >= tableView_->verticalScrollBar()->maximum() - 2;
    const int budget = qMax(256, model_->limit());
    const QList<ModelItem> display = items.size() > budget ? items.mid(items.size() - budget) : items;
    model_->appendBatch(display);
    if (followBottom)
        QTimer::singleShot(0, tableView_, [this] { tableView_->scrollToBottom(); });
}

void MainWindow::onSendedData(const QList<ModelItem> &items)
{
    for (const ModelItem &it : items) {
        ++txCount_[it.channel];
        if (realSaving_)
            csvWriter_.append(csvRow(it));
        if (monitorSaving_)
            monitorCsvWriter_.append(monitorCsvRow(it));
    }
    if (!pauseDisplay_) {
        const int budget = qMax(256, model_->limit());
        model_->appendBatch(items.size() > budget ? items.mid(items.size() - budget) : items);
    }
}

void MainWindow::onEndSend()
{
    appendLog(tr("数据发送完成！"));
}

void MainWindow::onSendFailed(const QString &error)
{
    appendLog(tr("发送失败：%1").arg(error));
}

void MainWindow::refreshRate()
{
    // 厂商 API 的 timeout=0，主线程只读取当前已有帧，不等待硬件。
    // 每个通道一次最多批量取 1024 帧，统一交给 UI，避免逐帧刷新。
    constexpr quint32 BatchSize = 1024;
    QVector<ZCAN_Receive_Data> canFrames(BatchSize);
    QVector<ZCAN_ReceiveFD_Data> fdFrames(BatchSize);
    QList<ModelItem> received;
    for (int channel = 0; channel < 2; ++channel) {
        if (!device_.isChannelStarted(channel)) continue;
        const quint32 canCount = device_.receiveCan(channel, canFrames.data(), BatchSize);
        received.reserve(received.size() + int(canCount));
        for (quint32 i = 0; i < canCount; ++i) {
            const auto &r = canFrames[int(i)];
            ModelItem item;
            item.systemTime = QDateTime::currentMSecsSinceEpoch();
            item.timestamp = r.timestamp; item.channel = channel;
            item.extended = IS_EFF(r.frame.can_id); item.remote = IS_RTR(r.frame.can_id);
            item.id = GET_ID(r.frame.can_id); item.dlc = r.frame.can_dlc;
            item.data = QByteArray(reinterpret_cast<const char *>(r.frame.data), r.frame.can_dlc);
            received.push_back(std::move(item));
        }
        const quint32 fdCount = device_.receiveCanFd(channel, fdFrames.data(), BatchSize);
        received.reserve(received.size() + int(fdCount));
        for (quint32 i = 0; i < fdCount; ++i) {
            const auto &r = fdFrames[int(i)];
            ModelItem item;
            item.systemTime = QDateTime::currentMSecsSinceEpoch();
            item.timestamp = r.timestamp; item.channel = channel; item.fd = true;
            item.extended = IS_EFF(r.frame.can_id); item.remote = IS_RTR(r.frame.can_id);
            item.brs = (r.frame.flags & CANFD_BRS) != 0;
            item.id = GET_ID(r.frame.can_id); item.dlc = r.frame.len;
            item.data = QByteArray(reinterpret_cast<const char *>(r.frame.data), r.frame.len);
            received.push_back(std::move(item));
        }
    }
    if (!received.isEmpty()) onRecvData(received);

    for (int channel = 0; channel < 2; ++channel) {
        const QList<ModelItem> sent = sendThread_[channel].drain(20000);
        if (!sent.isEmpty()) onSendedData(sent);
    }

    if (!rateElapsed_.isValid()) {
        rateElapsed_.start();
        for (int c = 0; c < 2; ++c) {
            lastRx_[c] = rxCount_[c];
            lastTx_[c] = txCount_[c];
        }
    }

    const qint64 elapsed = rateElapsed_.elapsed();

    // 速率按 1 秒窗口统计，避免在 0 与峰值之间来回跳
    if (elapsed >= 1000) {
        const double sec = elapsed / 1000.0;
        for (int c = 0; c < 2; ++c) {
            const double instRx = (rxCount_[c] - lastRx_[c]) / sec;
            const double instTx = (txCount_[c] - lastTx_[c]) / sec;
            // 指数移动平均：速率变化平缓，不频繁跳动
            rxRate_[c] = rxRate_[c] * 0.55 + instRx * 0.45;
            txRate_[c] = txRate_[c] * 0.55 + instTx * 0.45;
            lastRx_[c] = rxCount_[c];
            lastTx_[c] = txCount_[c];
        }
        rateElapsed_.restart();
    }

    ch1RxLabel_->setText(QStringLiteral("通道1接收:%1 (%2帧/s)").arg(rxCount_[0]).arg(rxRate_[0], 0, 'f', 1));
    ch1TxLabel_->setText(QStringLiteral("通道1发送:%1 (%2帧/s)").arg(txCount_[0]).arg(txRate_[0], 0, 'f', 1));
    ch2RxLabel_->setText(QStringLiteral("通道2接收:%1 (%2帧/s)").arg(rxCount_[1]).arg(rxRate_[1], 0, 'f', 1));
    ch2TxLabel_->setText(QStringLiteral("通道2发送:%1 (%2帧/s)").arg(txCount_[1]).arg(txRate_[1], 0, 'f', 1));
    if (queryCountLabel_)
        queryCountLabel_->setText(tr("查询发送: %1 次").arg(querySendCount_));
}

void MainWindow::appendLog(const QString &msg)
{
    logEdit_->appendPlainText(QStringLiteral("[%1] %2")
                              .arg(QTime::currentTime().toString(QStringLiteral("hh:mm:ss.zzz")))
                              .arg(msg));
}

QString MainWindow::csvRow(const ModelItem &it)
{
    const QString sysTime = QDateTime::fromMSecsSinceEpoch(it.systemTime)
                                .toString(QStringLiteral("hh:mm:ss.zzz"));
    const QString idText = QStringLiteral("0x%1").arg(it.id, 0, 16).toUpper();
    return QStringLiteral("%1,=\"%2\",%3,%4,%5,%6\n")
        .arg(saveSeq_++)
        .arg(sysTime)
        .arg(it.channel + 1)
        .arg(it.transmit ? QStringLiteral("发送") : QStringLiteral("接收"))
        .arg(idText)
        .arg(QString::fromLatin1(it.data.toHex(' ').toUpper()));
}

QString MainWindow::monitorCsvRow(const ModelItem &it)
{
    const QString time = QDateTime::fromMSecsSinceEpoch(it.systemTime)
        .toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz"));
    // 解析列：电压、电流、功率、温度、四种保护状态、设备状态。
    // CNT 属于协议内部计数，不再写入面向用户的数据记录。
    QStringList parsed(9, QString());
    const int slot = monitorSlots_.value(it.id, -1);
    if (slot >= 0 && slot < int(monitor_.size())) {
        const auto &m = monitor_[slot];
        parsed = { m.voltage ? m.voltage->text() : QString(),
                   m.current ? m.current->text() : QString(), m.power ? m.power->text() : QString(),
                   m.temperature ? m.temperature->text() : QString(),
                   m.overVoltage ? m.overVoltage->text() : QString(),
                   m.underVoltage ? m.underVoltage->text() : QString(),
                   m.overCurrent ? m.overCurrent->text() : QString(),
                   m.overTemperature ? m.overTemperature->text() : QString(),
                   m.powerStatus ? m.powerStatus->text() : QString() };
    }
    const QString idText = QStringLiteral("0x") + QString::number(it.id, 16).toUpper();
    return QStringLiteral("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15\n")
        .arg(monitorSaveSeq_++).arg(time).arg(it.channel + 1)
        .arg(it.transmit ? QStringLiteral("TX") : QStringLiteral("RX"))
        .arg(idText).arg(QString::fromLatin1(it.data.toHex(' ').toUpper()))
        .arg(parsed.value(0)).arg(parsed.value(1)).arg(parsed.value(2)).arg(parsed.value(3))
        .arg(parsed.value(4)).arg(parsed.value(5)).arg(parsed.value(6)).arg(parsed.value(7))
        .arg(parsed.value(8));
}

void MainWindow::toggleMonitorRecording(bool enabled)
{
    if (enabled) {
        const QString dir = QCoreApplication::applicationDirPath() + QStringLiteral("/data");
        QDir().mkpath(dir);
        monitorSavePath_ = dir + QStringLiteral("/Monitor_")
            + QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_hhmmss")) + QStringLiteral(".csv");
        QString error;
        if (!monitorCsvWriter_.start(monitorSavePath_,
            QStringLiteral("序号,系统时间,通道,方向,ID,数据,电压,电流,功率,温度,过压,欠压,过流,过温,状态\n"), &error)) {
            QMessageBox::warning(this, tr("数据记录"), tr("无法开始记录：%1").arg(error));
            if (auto *check = qobject_cast<QCheckBox *>(sender())) {
                check->blockSignals(true); check->setChecked(false); check->blockSignals(false);
            }
            return;
        }
        monitorSaveSeq_ = 0;
        monitorSaving_ = true;
        statusBar()->showMessage(tr("监控数据记录已启用"), 2500);
        return;
    }
    if (!monitorSaving_) return;
    monitorCsvWriter_.stop();
    monitorCsvWriter_.join();
    monitorSaving_ = false;

    QMessageBox box(QMessageBox::Question, tr("数据记录"), tr("监控数据已经保存，是否打开记录文件？"), QMessageBox::NoButton, this);
    auto *openButton = box.addButton(tr("打开"), QMessageBox::AcceptRole);
    box.addButton(tr("取消"), QMessageBox::RejectRole);
    auto *saveAsButton = box.addButton(tr("另存为"), QMessageBox::ActionRole);
    box.exec();
    if (box.clickedButton() == openButton) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(monitorSavePath_));
    } else if (box.clickedButton() == saveAsButton) {
        const QString target = QFileDialog::getSaveFileName(this, tr("另存监控数据"),
            QFileInfo(monitorSavePath_).fileName(), tr("CSV 文件 (*.csv)"));
        if (!target.isEmpty()) {
            if (QFile::exists(target)) QFile::remove(target);
            if (!QFile::copy(monitorSavePath_, target))
                QMessageBox::warning(this, tr("另存为"), tr("无法复制记录文件。"));
        }
    }
}

// ---------------------------------------------------------------------------
// 发送逻辑
// ---------------------------------------------------------------------------
bool MainWindow::parseSendData(int channel, QByteArray &data, quint32 &id)
{
    bool ok = false;
    data = parseHexBytes(ch_[channel].dataEdit->text(), &ok);
    if (!ok) {
        QMessageBox::warning(this, tr("警告"), tr("Data format error, use hex bytes like \"00 01 02\"."));
        return false;
    }

    QString idText = ch_[channel].idEdit->text().trimmed();
    idText.remove(QStringLiteral("0x"));
    id = idText.toUInt(&ok, 16);
    if (!ok) {
        QMessageBox::warning(this, tr("警告"), tr("ID格式错误。"));
        return false;
    }
    return true;
}

void MainWindow::startSend(int channel)
{
    if (!device_.isChannelStarted(channel)) {
        QMessageBox::warning(this, tr("警告"),
                             tr("通道%1未打开！").arg(channel + 1));
        return;
    }

    QByteArray data;
    quint32 id = 0;
    if (!parseSendData(channel, data, id))
        return;

    const ChannelPanel &p = ch_[channel];
    const bool extended = p.frameType->currentIndex() == 1;
    const bool remote = p.frameData->currentIndex() == 1;
    const bool fd = p.protocol->currentIndex() == 1;

    // 帧 ID 范围校验
    const quint32 maxId = extended ? 0x1FFFFFFFu : 0x7FFu;
    if (id > maxId) {
        QMessageBox::warning(this, tr("警告"),
                             extended ? tr("发送失败，扩展帧ID范围为0~0x1FFFFFFF！")
                                      : tr("发送失败，标准帧ID范围为0~0x7FF！"));
        return;
    }

    sendThread::Config cfg;
    cfg.channel = channel;
    cfg.fd = fd;
    cfg.extended = extended;
    cfg.remote = remote;
    cfg.brs = false;  // 加速位，后续可按需加界面开关
    cfg.id = id;
    cfg.data = data;
    cfg.continuous = p.continuous->isChecked();
    cfg.count = static_cast<quint64>(p.countEdit->text().toULongLong());
    cfg.intervalMs = p.invEdit->text().toInt();
    cfg.idIncrement = p.idAdd->isChecked();
    cfg.dataIncrement = p.dataAdd->isChecked();
    cfg.sendType = p.sendType->currentIndex();

    sendThread_[channel].start(cfg);

    if (cfg.continuous)
        appendLog(tr("通道%1 开始发送数据：持续发送，发送间隔%2ms...")
                  .arg(channel + 1).arg(cfg.intervalMs));
    else
        appendLog(tr("通道%1 开始发送数据：共%2帧，发送间隔%3ms...")
                  .arg(channel + 1).arg(cfg.count).arg(cfg.intervalMs));
}

void MainWindow::stopSend(int channel)
{
    sendThread_[channel].stop();
    if (sequenceDlg_[channel])
        sequenceDlg_[channel]->stopSending();
    appendLog(tr("数据发送已中止！"));
}

// ---------------------------------------------------------------------------
// 配置
// ---------------------------------------------------------------------------
QString MainWindow::chPrefix(int channel) const
{
    return channel == 0 ? QStringLiteral("ch1") : QStringLiteral("ch2");
}

void MainWindow::loadConfigIntoUi()
{
    for (int c = 0; c < 2; ++c) {
        const QString p = chPrefix(c);
        ChannelPanel &panel = ch_[c];
        panel.protocol->setCurrentIndex(config_.intValue(p + QStringLiteral("ProtocolCombo"), 0));
        panel.frameType->setCurrentIndex(config_.intValue(p + QStringLiteral("FrameTypeCombo"), 0));
        panel.frameData->setCurrentIndex(config_.intValue(p + QStringLiteral("FrameDataCombo"), 0));
        panel.sendType->setCurrentIndex(config_.intValue(p + QStringLiteral("SendTypeCombo"), 0));
        panel.idEdit->setText(config_.value(p + QStringLiteral("SendIDEdit"), QString::number(c)));
        panel.dataEdit->setText(config_.value(p + QStringLiteral("SendDataEdit"),
                                             QStringLiteral("00 01 02 03 04 05 06 07")));
        panel.countEdit->setText(config_.value(p + QStringLiteral("SendCountEdit"), QStringLiteral("10000000")));
        panel.invEdit->setText(config_.value(p + QStringLiteral("SendInvEdit"), QStringLiteral("100")));
        panel.idAdd->setChecked(config_.boolValue(p + QStringLiteral("IDAddChk"), false));
        panel.dataAdd->setChecked(config_.boolValue(p + QStringLiteral("DataAddChk"), false));
    }

    model_->setLimit(config_.intValue(QStringLiteral("listCount"), 1000));
    const bool merge = config_.boolValue(QStringLiteral("merge"), false);
    model_->setMerge(merge);
    if (mergeCheck_) mergeCheck_->setChecked(merge);
}

void MainWindow::saveConfig()
{
    for (int c = 0; c < 2; ++c) {
        const QString p = chPrefix(c);
        const ChannelPanel &panel = ch_[c];
        config_.setValue(p + QStringLiteral("ProtocolCombo"), QString::number(panel.protocol->currentIndex()));
        config_.setValue(p + QStringLiteral("FrameTypeCombo"), QString::number(panel.frameType->currentIndex()));
        config_.setValue(p + QStringLiteral("FrameDataCombo"), QString::number(panel.frameData->currentIndex()));
        config_.setValue(p + QStringLiteral("SendTypeCombo"), QString::number(panel.sendType->currentIndex()));
        config_.setValue(p + QStringLiteral("SendIDEdit"), panel.idEdit->text());
        config_.setValue(p + QStringLiteral("SendDataEdit"), panel.dataEdit->text());
        config_.setValue(p + QStringLiteral("SendCountEdit"), panel.countEdit->text());
        config_.setValue(p + QStringLiteral("SendInvEdit"), panel.invEdit->text());
        config_.setValue(p + QStringLiteral("IDAddChk"), panel.idAdd->isChecked() ? QStringLiteral("1") : QStringLiteral("0"));
        config_.setValue(p + QStringLiteral("DataAddChk"), panel.dataAdd->isChecked() ? QStringLiteral("1") : QStringLiteral("0"));
    }
    config_.setValue(QStringLiteral("listCount"), QString::number(model_->limit()));
    config_.save(ConfigManager::defaultFilePath());
}


// ---------------------------------------------------------------------------
// 电源监控协议
// ---------------------------------------------------------------------------
uint8_t MainWindow::crc8(const QByteArray &data)
{
    uint8_t crc = 0;
    for (char byte : data) {
        crc ^= uint8_t(byte);
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc & 0x80) ? uint8_t((crc << 1) ^ 0x07) : uint8_t(crc << 1);
    }
    return crc;
}

void MainWindow::sendCommand(uint8_t command, const QByteArray &payload, bool withCrc, bool quiet)
{
    if (iap_.running()) {
        statusBar()->showMessage(tr("在线升级进行中，普通控制命令已忽略"), 3000);
        return;
    }
    QByteArray frame(8, '\0');
    if (command == 0x01) frame[0] = char(queryFrameCount_++);
    else if (command == 0x02) frame[0] = char(versionFrameCount_++);
    frame[1] = char(command);
    const int copy = qMin(payload.size(), withCrc ? 5 : 6);
    for (int i = 0; i < copy; ++i) frame[i + 2] = payload[i];
    if (withCrc) {
        QByteArray check;
        check.append(char(command));
        check.append(payload.left(copy));
        frame[copy + 2] = char(crc8(check));
    }
    bool attempted = false, ok = true;
    QString err;
    for (int channel = 0; channel < 2; ++channel) {
        if (!commandChannels_[channel]) continue;
        attempted = true;
        const uint32_t id = masterIds_[channel];
        const bool sent = device_.sendCan(channel, id, true, false, frame, 0, &err);
        ok = ok && sent;
        if (!sent && !quiet)
            appendLog(tr("通道%1发送失败：%2").arg(channel + 1).arg(err));
        if (sent) {
            ++txCount_[channel];
            ModelItem it;
            it.systemTime = QDateTime::currentMSecsSinceEpoch();
            it.channel = channel;
            it.transmit = true;
            it.extended = true;
            it.id = id;
            it.dlc = frame.size();
            it.data = frame;
            if (realSaving_) csvWriter_.append(csvRow(it));
            if (monitorSaving_) monitorCsvWriter_.append(monitorCsvRow(it));
            if (!pauseDisplay_) model_->append(it);
        }
    }
    if ((!attempted || !ok) && !quiet)
        statusBar()->showMessage(tr("CAN 通道未启动或发送失败"), 2500);
}

void MainWindow::sendCommandToDevice(int channel, uint32_t id, uint8_t command, const QByteArray &payload, bool withCrc)
{
    if (iap_.running()) {
        statusBar()->showMessage(tr("在线升级进行中，控制命令已忽略"), 3000);
        return;
    }
    if (channel < 0 || channel > 1 || !device_.isOpen()) {
        statusBar()->showMessage(tr("设备或对应 CAN 通道尚未启动"), 2500);
        return;
    }
    QByteArray frame(8, '\0');
    frame[1] = char(command);
    const int copy = qMin(payload.size(), withCrc ? 5 : 6);
    for (int i = 0; i < copy; ++i) frame[i + 2] = payload[i];
    if (withCrc) {
        QByteArray check;
        check.append(char(command));
        check.append(payload.left(copy));
        frame[copy + 2] = char(crc8(check));
    }
    QString err;
    if (!device_.sendCan(channel, id, true, false, frame, 0, &err)) {
        statusBar()->showMessage(tr("无法向 0x%1 发送命令").arg(id, 0, 16), 2500);
        return;
    }
    ++txCount_[channel];
    ModelItem item;
    item.systemTime = QDateTime::currentMSecsSinceEpoch();
    item.channel = channel; item.transmit = true; item.extended = true;
    item.id = id; item.dlc = frame.size(); item.data = frame;
    if (realSaving_) csvWriter_.append(csvRow(item));
    if (monitorSaving_) monitorCsvWriter_.append(monitorCsvRow(item));
    if (!pauseDisplay_) model_->append(item);
    if (command == 0x04 || command == 0x05)
        statusBar()->showMessage(tr("已向 0x%1 发送%2命令").arg(id, 0, 16).arg(command == 0x05 ? tr("开启") : tr("关闭")), 2000);
}

void MainWindow::updateMonitor(const ModelItem &frame)
{
    if (!monitor_[0].id) return;
    if (frame.id < 0xA0000U || frame.id > 0xA0009U) return;
    if (frame.data.size() < 2) return;
    int slot = monitorSlots_.value(frame.id, -1);
    if (slot < 0) {
        if (monitorSlots_.size() >= int(monitor_.size())) return;
        slot = monitorSlots_.size();
        monitorSlots_.insert(frame.id, slot);
    }
    auto &m = monitor_[slot];
    if (!m.visible) {
        m.visible = true;
        for (auto *cell : m.cells) if (cell) cell->show();
    }
    ++m.frames;
    m.id->setText(QStringLiteral("0x") + QString::number(frame.id, 16).toUpper());
    const QByteArray &d = frame.data;
    if (uint8_t(d[1]) == 0x81 && d.size() >= 8) {
        const double voltage = (uint8_t(d[3]) | (uint8_t(d[4]) << 8)) / 1000.0;
        const double current = (uint8_t(d[5]) | (uint8_t(d[6]) << 8)) / 10.0;
        const int temperature = int(int8_t(d[7]));
        const uint8_t state = uint8_t(d[2]);
        m.deviceId = frame.id;
        m.channel = frame.channel;
        m.powerOn = (state & 0x01);
        m.currentValue = current;
        m.count->setText(QStringLiteral("0x%1").arg(uint8_t(d[0]), 2, 16, QLatin1Char('0')).toUpper());
        if (state & 0x20) {
            m.lastCommMs = quint64(QDateTime::currentMSecsSinceEpoch());
            m.communication->setStyleSheet(QStringLiteral("color:#20c55a;font-size:18px;"));
            m.commFlash->start();
        }
        m.voltage->setText(QString::number(voltage, 'f', 2) + QStringLiteral(" V"));
        m.current->setText(QString::number(current, 'f', 1) + QStringLiteral(" A"));
        m.power->setText(QString::number(voltage * current, 'f', 1) + QStringLiteral(" W"));
        m.temperature->setText(QString::number(temperature) + QStringLiteral(" °C"));
        auto setProtection = [](QLabel *label, bool fault) {
            label->setText(fault ? QObject::tr("故障") : QObject::tr("正常"));
            label->setStyleSheet(fault ? QStringLiteral("color:#d32f2f;font-weight:600;") : QStringLiteral("color:#6f7782;"));
        };
        setProtection(m.overVoltage, state & 0x02);
        setProtection(m.underVoltage, state & 0x04);
        setProtection(m.overCurrent, state & 0x08);
        setProtection(m.overTemperature, state & 0x10);
        m.powerStatus->setText(m.powerOn ? tr("开启") : tr("关闭"));
        m.powerStatus->setStyleSheet(m.powerOn ? QStringLiteral("color:#19934a;font-weight:600;") : QStringLiteral("color:#c8473a;font-weight:600;"));
        m.powerControl->setEnabled(true);
        m.powerControl->setText(m.powerOn ? tr("关闭") : tr("开启"));
        m.calibrate->setEnabled(true);
    } else if (uint8_t(d[1]) == 0x82 && d.size() >= 8) {
        const uint16_t version = uint8_t(d[6]) | (uint8_t(d[7]) << 8);
        m.version->setText(QStringLiteral("V%1.%2.%3  %4-%5-%6")
            .arg(version / 100).arg((version / 10) % 10).arg(version % 10)
            .arg(uint8_t(d[3]), 2, 10, QLatin1Char('0')).arg(uint8_t(d[4]), 2, 10, QLatin1Char('0')).arg(uint8_t(d[5]), 2, 10, QLatin1Char('0')));
        m.count->setText(QStringLiteral("0x%1").arg(uint8_t(d[0]), 2, 16, QLatin1Char('0')).toUpper());
    }
    updateCurrentSharing();
}

void MainWindow::updateCurrentSharing()
{
    // 收集活跃模块电流（电流 > 0.1A 视为在线）
    QList<double> currents;
    for (int i = 0; i < int(monitor_.size()); ++i) {
        if (monitor_[i].visible && monitor_[i].currentValue > 0.1)
            currents.append(monitor_[i].currentValue);
    }

    double totalSharing = 100.0;
    double avgCurrent = 0.0;
    if (currents.size() >= 2) {
        double maxCurrent = currents.first();
        double minCurrent = currents.first();
        double sum = 0.0;
        for (double c : currents) {
            if (c > maxCurrent) maxCurrent = c;
            if (c < minCurrent) minCurrent = c;
            sum += c;
        }
        avgCurrent = sum / currents.size();
        if (avgCurrent >= 0.01) {
            totalSharing = (1.0 - (maxCurrent - minCurrent) / avgCurrent) * 100.0;
            if (totalSharing < 0.0) totalSharing = 0.0;
            if (totalSharing > 100.0) totalSharing = 100.0;
        }
    }

    for (int i = 0; i < int(monitor_.size()); ++i) {
        auto &m = monitor_[i];
        if (!m.visible || !m.sharing) continue;
        if (m.currentValue <= 0.1) {
            m.sharing->setText(QStringLiteral("---"));
            m.sharing->setStyleSheet(QStringLiteral("color:#6f7782;"));
            continue;
        }
        if (currents.size() < 2 || avgCurrent < 0.01) {
            m.sharing->setText(QStringLiteral("100%"));
            m.sharing->setStyleSheet(QStringLiteral("color:#19934a;font-weight:600;"));
        } else {
            const double percent = (m.currentValue - avgCurrent) / avgCurrent * 100.0;
            m.sharing->setText(QStringLiteral("%1%").arg(percent, 0, 'f', 1));
            m.sharing->setStyleSheet(qAbs(percent) > 5.0
                ? QStringLiteral("color:#d32f2f;font-weight:600;")
                : QStringLiteral("color:#19934a;font-weight:600;"));
        }
    }

    if (totalSharingLabel_) {
        totalSharingLabel_->setText(tr("总均流度: %1%").arg(totalSharing, 0, 'f', 1));
    }
}

void MainWindow::parseProtocol(const ModelItem &frame)
{
    if (frame.data.size() < 8 || !llcTable_ || !pfcTable_) return;
    const QByteArray &d = frame.data;
    const int col = int(frame.id) - 0xA0000 + 1;
    if (col < 1 || col > 10) return;
    auto u16 = [&d](int offset) { return uint16_t(uint8_t(d[offset]) | (uint16_t(uint8_t(d[offset + 1])) << 8)); };
    auto set = [col](QTableWidget *table, int row, const QString &value) {
        if (table && row >= 0 && row < table->rowCount()) table->item(row, col)->setText(value);
    };
    switch (uint8_t(d[1])) {
    case 0x83: set(llcTable_, 0, QString::number(u16(2)/1000.0,'f',3)); set(llcTable_, 1, QString::number(u16(4)/1000.0,'f',3)); set(llcTable_, 2, QString::number(u16(6)/1000.0,'f',3)); break;
    case 0x84: set(llcTable_, 4, QString::number(u16(2)/10.0,'f',1)); set(llcTable_, 5, QString::number(u16(4)/10.0,'f',1)); set(llcTable_, 6, QString::number(u16(6)/10.0,'f',1)); break;
    case 0x85: set(llcTable_, 7, QString::number(u16(2)/10.0,'f',1)); set(llcTable_, 8, QString::number(u16(4)/10.0,'f',1)); break;
    case 0x86: set(llcTable_, 11, QString::number(u16(2)/1000.0,'f',3)); set(llcTable_, 12, QString::number(u16(4)/1000.0,'f',3)); set(llcTable_, 13, QString::number(u16(6)/1000.0,'f',3)); break;
    case 0xbe: set(llcTable_, 9, QString::number(u16(2))); set(llcTable_, 10, QString::number(u16(4))); break;
    case 0x87: set(pfcTable_, 6, QString::number(u16(2)/10.0,'f',1)); set(pfcTable_, 7, QString::number(u16(4)/10.0,'f',1)); break;
    case 0x88: set(pfcTable_, 8, QString::number(u16(2)/10.0,'f',1)); set(pfcTable_, 9, QString::number(u16(4)/10.0,'f',1)); break;
    case 0x89: set(pfcTable_, 0, QString::number(u16(2)/10.0,'f',1)); set(pfcTable_, 1, QString::number(u16(4)/10.0,'f',1)); break;
    case 0x8a: set(pfcTable_, 2, QString::number(u16(2)/10.0,'f',1)); set(pfcTable_, 3, QString::number(u16(4)/10.0,'f',1)); break;
    case 0x8b: set(pfcTable_, 4, QString::number(u16(2)/10.0,'f',1)); set(pfcTable_, 5, QString::number(u16(4)/10.0,'f',1)); break;
    case 0x8c: set(pfcTable_, 10, QString::number(u16(2)/10.0,'f',1)); set(pfcTable_, 11, QString::number(u16(4)/10.0,'f',1)); set(pfcTable_, 12, QString::number(u16(6)/10.0,'f',1)); break;
    case 0x8d: set(pfcTable_, 13, QString::number(u16(2)/10.0,'f',1)); set(pfcTable_, 14, QString::number(u16(4)/10.0,'f',1)); set(pfcTable_, 15, QString::number(int16_t(u16(6)))); break;
    case 0x8e: set(pfcTable_, 16, QString::number(uint8_t(d[2]))); set(pfcTable_, 17, QString::number(uint8_t(d[3]))); set(pfcTable_, 18, QString::number(u16(4)/10.0,'f',1)); set(pfcTable_, 19, QStringLiteral("0x%1").arg(u16(6), 4, 16, QLatin1Char('0')).toUpper()); break;
    }
}

QWidget *MainWindow::makeMonitorPage()
{
    auto *page = new QWidget;
    auto *v = new QVBoxLayout(page);
    v->setContentsMargins(8, 8, 8, 8);

    auto *controls = new QHBoxLayout;
    auto *addressBox = new QGroupBox(tr("CAN 主机地址设置"), page);
    auto *addressGrid = new QGridLayout(addressBox);
    std::array<QCheckBox*, 2> enabled{};
    std::array<QLineEdit*, 2> address{};
    std::array<QPushButton*, 2> shortAddress{}, longAddress{};
    for (int i = 0; i < 2; ++i) {
        enabled[i] = new QCheckBox(QString(tr("CAN通道%1")).arg(i + 1), addressBox);
        enabled[i]->setChecked(commandChannels_[i]);
        address[i] = new QLineEdit(QString::number(masterIds_[i], 16).toUpper(), addressBox);
        shortAddress[i] = new QPushButton(i == 0 ? QStringLiteral("0x20") : QStringLiteral("0x21"), addressBox);
        longAddress[i] = new QPushButton(i == 0 ? QStringLiteral("0xB0000") : QStringLiteral("0xBB208"), addressBox);
        addressGrid->addWidget(enabled[i], i, 0);
        addressGrid->addWidget(address[i], i, 1);
        addressGrid->addWidget(shortAddress[i], i, 2);
        addressGrid->addWidget(longAddress[i], i, 3);
        connect(enabled[i], &QCheckBox::toggled, this, [this, i](bool on) { commandChannels_[i] = on; });
        connect(address[i], &QLineEdit::editingFinished, this, [this, i, field = address[i]] {
            bool ok = false;
            const auto value = field->text().toUInt(&ok, 16);
            if (ok) masterIds_[i] = value;
        });
    }
    auto setBothAddresses = [this, address](const QString &value) {
        for (int i = 0; i < 2; ++i) { address[i]->setText(value); masterIds_[i] = value.toUInt(nullptr, 16); }
    };
    connect(shortAddress[0], &QPushButton::clicked, this, [setBothAddresses] { setBothAddresses(QStringLiteral("20")); });
    connect(longAddress[0], &QPushButton::clicked, this, [setBothAddresses] { setBothAddresses(QStringLiteral("B0000")); });
    connect(shortAddress[1], &QPushButton::clicked, this, [setBothAddresses] { setBothAddresses(QStringLiteral("21")); });
    connect(longAddress[1], &QPushButton::clicked, this, [setBothAddresses] { setBothAddresses(QStringLiteral("BB208")); });
    auto *autoRefresh = new QCheckBox(tr("自动刷新"), addressBox);
    auto *frequency = new QSpinBox(addressBox);
    frequency->setRange(50, 60000);
    frequency->setValue(50);   // 默认自动刷新频率 50ms
    frequency->setSuffix(QStringLiteral(" ms"));
    addressGrid->addWidget(autoRefresh, 0, 4);
    addressGrid->addWidget(frequency, 1, 4);
    frequency->setValue(queryTimer_.interval());
    connect(autoRefresh, &QCheckBox::toggled, this, [this, frequency](bool on) {
        if (on) {
            queryTimer_.start(frequency->value());
            appendLog(tr("自动刷新已开启，间隔%1ms").arg(frequency->value()));
        } else {
            queryTimer_.stop();
            appendLog(tr("自动刷新已关闭"));
        }
    });
    connect(frequency, qOverload<int>(&QSpinBox::valueChanged), this, [this, autoRefresh](int value) {
        queryTimer_.setInterval(value);
        config_.setValue(QStringLiteral("monitor/queryInterval"), QString::number(value));
        if (autoRefresh->isChecked() && !queryTimer_.isActive()) queryTimer_.start();
    });
    controls->addWidget(addressBox, 2);

    auto *system = new QGroupBox(tr("系统控制"), page);
    auto *systemRow = new QHBoxLayout(system);
    auto *startSystem = new QPushButton(tr("启动"), system);
    auto *stopSystem = new QPushButton(tr("停止"), system);
    auto *record = new QCheckBox(tr("启用数据记录"), system);
    systemRow->addWidget(startSystem);
    systemRow->addWidget(stopSystem);
    systemRow->addWidget(record);
    connect(startSystem, &QPushButton::clicked, this, [this] { sendCommand(0x05); });
    connect(stopSystem, &QPushButton::clicked, this, [this] { sendCommand(0x04); });
    connect(record, &QCheckBox::toggled, this, &MainWindow::toggleMonitorRecording);
    controls->addWidget(system, 1);

    auto *calibration = new QGroupBox(tr("电压校准"), page);
    auto *calibrationGrid = new QGridLayout(calibration);
    auto *actual = new QDoubleSpinBox(calibration);
    auto *targetVoltage = new QDoubleSpinBox(calibration);
    for (auto *value : { actual, targetVoltage }) {
        value->setRange(0, 65.535);
        value->setDecimals(3);
        value->setValue(12.0);
    }
    auto *calibrate = new QPushButton(tr("执行校准"), calibration);
    calibrationGrid->addWidget(new QLabel(tr("实际(V)"), calibration), 0, 0);
    calibrationGrid->addWidget(actual, 0, 1);
    calibrationGrid->addWidget(new QLabel(tr("目标(V)"), calibration), 1, 0);
    calibrationGrid->addWidget(targetVoltage, 1, 1);
    calibrationGrid->addWidget(calibrate, 2, 0, 1, 2);
    connect(calibrate, &QPushButton::clicked, this, [this, actual, targetVoltage] {
        const uint16_t factor = uint16_t(qRound(actual->value() * 1000));
        const uint16_t theor = uint16_t(qRound(targetVoltage->value() * 1000));
        QByteArray a; a.append(char(factor)); a.append(char(factor >> 8)); sendCommand(0x22, a);
        QByteArray b; b.append(char(theor)); b.append(char(theor >> 8)); sendCommand(0x23, b);
        QByteArray check;
        check.append(char(0x24)); check.append(char(factor)); check.append(char(factor >> 8));
        check.append(char(0)); check.append(char(0)); check.append(char(theor)); check.append(char(theor >> 8));
        check.append(char(0)); check.append(char(0));
        QByteArray crc; crc.append(char(crc8(check))); sendCommand(0x24, crc);
    });
    controls->addWidget(calibration, 1);

    auto *quick = new QGroupBox(tr("快捷命令 / 总线负载"), page);
    auto *quickGrid = new QGridLayout(quick);
    const QList<QPair<QString, uint8_t>> commands = { { tr("存储FLASH"), 0x11 }, { tr("加载FLASH"), 0x12 }, { tr("读取状态"), 0x01 }, { tr("版本信息"), 0x02 } };
    for (int i = 0; i < commands.size(); ++i) {
        auto *button = new QPushButton(commands[i].first, quick);
        quickGrid->addWidget(button, i / 2, i % 2);
        connect(button, &QPushButton::clicked, this, [this, cmd = commands[i].second] { sendCommand(cmd); });
    }
    busLoad_ = new QProgressBar(quick);
    busLoad_->setRange(0, 100);
    busLoad_->setFormat(tr("总线负载 %p%"));
    quickGrid->addWidget(busLoad_, 2, 0, 1, 2);
    controls->addWidget(quick, 1);
    v->addLayout(controls);

    auto *titleRow = new QHBoxLayout;
    auto *title = new QLabel(tr("通道数据（自动识别 10 个模块）"), page);
    title->setStyleSheet(QStringLiteral("font-weight:600;"));
    queryCountLabel_ = new QLabel(tr("查询发送: 0 次"), page);
    queryCountLabel_->setStyleSheet(QStringLiteral("color:#555;"));
    totalSharingLabel_ = new QLabel(tr("总均流度: 100.0%"), page);
    totalSharingLabel_->setStyleSheet(QStringLiteral("color:#19934a;font-weight:600;"));
    titleRow->addWidget(title);
    titleRow->addStretch();
    titleRow->addWidget(totalSharingLabel_);
    titleRow->addWidget(queryCountLabel_);
    v->addLayout(titleRow);

    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(2);
    const QStringList headers = { tr("通道"), tr("通讯"), tr("CNT"), tr("设备 ID"), tr("电压"), tr("电流"), tr("功率"), tr("温度"), tr("均流"), tr("版本"), tr("过压"), tr("欠压"), tr("过流"), tr("过温"), tr("状态"), tr("控制"), tr("实际(V)"), tr("目标(V)"), tr("校准") };
    for (int c = 0; c < headers.size(); ++c) {
        auto *label = new QLabel(headers[c], page);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet(QStringLiteral("font-weight:600;padding:6px;"));
        grid->addWidget(label, 0, c);
    }
    for (int r = 0; r < 10; ++r) {
        auto &m = monitor_[r];
        m.channelLabel = new QLabel(QStringLiteral("CH%1").arg(r + 1, 2, 10, QLatin1Char('0')), page);
        m.channelLabel->setStyleSheet(QStringLiteral("font-weight:600;color:#001f7a;"));
        m.communication = new QLabel(QStringLiteral("●"), page);
        m.communication->setStyleSheet(QStringLiteral("color:#909090;font-size:18px;"));
        m.count = new QLabel(QStringLiteral("---"), page);
        m.id = new QLabel(QStringLiteral("---"), page);
        m.voltage = new QLabel(QStringLiteral("0.00 V"), page);
        m.current = new QLabel(QStringLiteral("0.00 A"), page);
        m.power = new QLabel(QStringLiteral("0.0 W"), page);
        m.temperature = new QLabel(QStringLiteral("0 °C"), page);
        m.version = new QLabel(QStringLiteral("-"), page);
        m.overVoltage = new QLabel(tr("正常"), page);
        m.underVoltage = new QLabel(tr("正常"), page);
        m.overCurrent = new QLabel(tr("正常"), page);
        m.overTemperature = new QLabel(tr("正常"), page);
        m.powerStatus = new QLabel(tr("关闭"), page);
        m.sharing = new QLabel(QStringLiteral("---"), page);
        m.powerControl = new QPushButton(tr("开启"), page);
        m.powerControl->setEnabled(false);
        m.actualVoltage = new QDoubleSpinBox(page);
        m.targetVoltage = new QDoubleSpinBox(page);
        for (auto *input : { m.actualVoltage, m.targetVoltage }) {
            input->setRange(0, 65.535);
            input->setDecimals(3);
            input->setValue(12.0);
            input->setButtonSymbols(QAbstractSpinBox::NoButtons);
            input->setMaximumWidth(74);
            input->setAlignment(Qt::AlignCenter);
        }
        m.calibrate = new QPushButton(tr("应用"), page);
        m.calibrate->setEnabled(false);
        m.commFlash = new QTimer(page);
        m.commFlash->setSingleShot(true);
        m.commFlash->setInterval(100);

        QList<QLabel*> labels = { m.channelLabel, m.communication, m.count, m.id, m.voltage, m.current, m.power, m.temperature, m.sharing, m.version, m.overVoltage, m.underVoltage, m.overCurrent, m.overTemperature, m.powerStatus };
        for (int c = 0; c < labels.size(); ++c) {
            labels[c]->setAlignment(Qt::AlignCenter);
            labels[c]->setMinimumHeight(38);
            if (r % 2) labels[c]->setStyleSheet(labels[c]->styleSheet() + QStringLiteral("background:#f3f5ff;"));
            grid->addWidget(labels[c], r + 1, c);
            m.cells[c] = labels[c];
            labels[c]->hide();
        }
        m.powerControl->setMinimumHeight(30);
        grid->addWidget(m.powerControl, r + 1, 15); m.cells[15] = m.powerControl;
        grid->addWidget(m.actualVoltage, r + 1, 16); m.cells[16] = m.actualVoltage;
        grid->addWidget(m.targetVoltage, r + 1, 17); m.cells[17] = m.targetVoltage;
        grid->addWidget(m.calibrate, r + 1, 18); m.cells[18] = m.calibrate;
        for (int c = 15; c < 19; ++c) m.cells[c]->hide();
        connect(m.commFlash, &QTimer::timeout, this, [this, r] {
            auto &item = monitor_[r];
            if (item.communication) item.communication->setStyleSheet(QStringLiteral("color:#909090;font-size:18px;"));
        });
        connect(m.powerControl, &QPushButton::clicked, this, [this, r] {
            auto &item = monitor_[r];
            if (item.channel < 0 || !item.deviceId) return;
            sendCommandToDevice(item.channel, item.deviceId, item.powerOn ? 0x04 : 0x05);
        });
        connect(m.calibrate, &QPushButton::clicked, this, [this, r] {
            auto &item = monitor_[r];
            if (item.channel < 0 || !item.deviceId) return;
            const uint16_t act = uint16_t(qRound(item.actualVoltage->value() * 1000));
            const uint16_t tgt = uint16_t(qRound(item.targetVoltage->value() * 1000));
            QByteArray a; a.append(char(act)); a.append(char(act >> 8)); sendCommandToDevice(item.channel, item.deviceId, 0x22, a);
            QByteArray b; b.append(char(tgt)); b.append(char(tgt >> 8)); sendCommandToDevice(item.channel, item.deviceId, 0x23, b);
            QByteArray check;
            check.append(char(0x24)); check.append(char(act)); check.append(char(act >> 8));
            check.append(char(0)); check.append(char(0)); check.append(char(tgt)); check.append(char(tgt >> 8));
            check.append(char(0)); check.append(char(0));
            QByteArray crc; crc.append(char(crc8(check))); sendCommandToDevice(item.channel, item.deviceId, 0x24, crc);
        });
    }
    grid->setColumnMinimumWidth(0, 42);
    grid->setColumnMinimumWidth(1, 30);
    grid->setColumnMinimumWidth(2, 46);
    grid->setColumnMinimumWidth(3, 72);
    for (int c = 4; c <= 7; ++c) grid->setColumnMinimumWidth(c, 58);
    grid->setColumnMinimumWidth(8, 52);
    grid->setColumnMinimumWidth(9, 72);
    grid->setColumnStretch(9, 1);
    v->addLayout(grid);
    v->addStretch();
    return page;
}

QWidget *MainWindow::makeProtectPage()
{
    auto *page = new QWidget;
    auto *root = new QVBoxLayout(page);
    auto *buttons = new QHBoxLayout;
    auto *scan = new QPushButton(tr("扫描设备"), page);
    auto *query = new QPushButton(tr("查询保护点"), page);
    buttons->addWidget(scan);
    buttons->addWidget(query);
    buttons->addStretch();
    root->addLayout(buttons);

    auto makeTable = [](const QStringList &rows) {
        auto *table = new QTableWidget(rows.size(), 11);
        QStringList headers{ QStringLiteral("参数名称") };
        for (int i = 0; i < 10; ++i) headers << QStringLiteral("0x%1").arg(0xA0000 + i, 5, 16, QLatin1Char('0')).toUpper();
        table->setHorizontalHeaderLabels(headers);
        table->verticalHeader()->hide();
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        for (int r = 0; r < rows.size(); ++r) {
            table->setItem(r, 0, new QTableWidgetItem(rows[r]));
            for (int c = 1; c < 11; ++c) table->setItem(r, c, new QTableWidgetItem(QStringLiteral("---")));
        }
        table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        return table;
    };
    llcTable_ = makeTable({ tr("LLC软件过压点(V)"), tr("LLC硬件过压点(V)"), tr("LLC欠压点(V)"), tr("LLC欠压恢复点(V)"), tr("LLC恒流点(A)"), tr("LLC软件过流点(A)"), tr("LLC过流恢复点(A)"), tr("LLC软件短路点(A)"), tr("LLC硬件短路点(A)"), tr("LLC过温点(°C)"), tr("LLC过温恢复点(°C)"), tr("LLC目标电压(V)"), tr("LLC校准电压(V)"), tr("LLC参考电压(V)"), tr("LLC KP参数"), tr("LLC KI参数") });
    pfcTable_ = makeTable({ tr("PFC_VBUS软件过压点(V)"), tr("PFC_VBUS硬件过压点(V)"), tr("PFC_VBUS欠压点(V)"), tr("PFC_VBUS欠压恢复点(V)"), tr("PFC_INPUT软件过流点(A)"), tr("PFC_INPUT硬件过流点(A)"), tr("PFC_INPUT输入过压点(V)"), tr("PFC_INPUT输入过压恢复点(V)"), tr("PFC_INPUT输入欠压点(V)"), tr("PFC_INPUT输入欠压恢复点(V)"), tr("PFC_VBUS目标电压(V)"), tr("PFC VBUS参考电压"), tr("PFC VBUS实际电压"), tr("PFC 输入电压实际值(V)"), tr("PFC 电流环参考值(A)"), tr("PFC NTC温度"), tr("PFC 工作状态"), tr("PFC 开关频率(kHz)"), tr("PFC 占空比(%)"), tr("PFC 状态标志(hex)") });

    auto *split = new QSplitter(Qt::Vertical, page);
    auto *llcBox = new QGroupBox(tr("LLC 参数"), split);
    auto *lv = new QVBoxLayout(llcBox);
    lv->addWidget(llcTable_);
    auto *pfcBox = new QGroupBox(tr("PFC 参数"), split);
    auto *pv = new QVBoxLayout(pfcBox);
    pv->addWidget(pfcTable_);
    split->addWidget(llcBox);
    split->addWidget(pfcBox);
    root->addWidget(split, 1);

    connect(scan, &QPushButton::clicked, this, [this] { sendCommand(0x03); });
    connect(query, &QPushButton::clicked, this, [this, query] {
        if (protectQueryInProgress_) return;
        if (!device_.isOpen()) { QMessageBox::warning(this, tr("查询保护点"), tr("请先打开 CAN 设备。")); return; }
        bool hasTarget = false;
        for (int channel = 0; channel < 2; ++channel) hasTarget = hasTarget || (commandChannels_[channel] && masterIds_[channel] != 0);
        if (!hasTarget) { QMessageBox::warning(this, tr("查询保护点"), tr("请启用至少一个通道并设置有效主机地址。")); return; }
        protectQueryInProgress_ = true;
        query->setEnabled(false);
        query->setText(tr("查询中..."));
        const QList<uint8_t> llc{ 0x3e, 0x3f, 0x40, 0x41, 0x44 };
        const QList<uint8_t> pfc{ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 };
        int delay = 0;
        for (int round = 0; round < 2; ++round) {
            for (const auto command : llc) { QTimer::singleShot(delay, this, [this, command] { sendCommand(command); }); delay += 120; }
            delay += 250;
            for (const auto command : pfc) { QTimer::singleShot(delay, this, [this, command] { sendCommand(command); }); delay += 180; }
            if (round == 0) delay += 350;
        }
        QTimer::singleShot(delay, this, [this, query] { protectQueryInProgress_ = false; query->setEnabled(true); query->setText(tr("查询保护点")); });
    });
    return page;
}

QWidget *MainWindow::makeParameterPage()
{
    auto *page = new QWidget;
    auto *root = new QVBoxLayout(page);
    auto *params = new QGroupBox(tr("参数设置"), page);
    auto *form = new QGridLayout(params);

    struct Parameter { QString name; uint8_t command; };
    const QList<Parameter> items = { { tr("KP"), 0x28 }, { tr("KI"), 0x29 }, { tr("测试参数1"), 0x42 }, { tr("测试参数2"), 0x43 } };
    int row = 0;
    for (const auto &item : items) {
        auto *value = new QDoubleSpinBox(params);
        value->setRange(-100000, 100000);
        value->setDecimals(7);
        value->setSingleStep(0.0001);
        auto *button = new QPushButton(tr("设置"), params);
        form->addWidget(new QLabel(item.name + QStringLiteral(":"), params), row, 0);
        form->addWidget(value, row, 1);
        form->addWidget(button, row, 2);
        connect(button, &QPushButton::clicked, this, [this, value, item] {
            QString text = value->cleanText();
            const int dot = text.indexOf('.');
            int decimals = dot < 0 ? 0 : text.mid(dot + 1).remove(QRegularExpression(QStringLiteral("0+$"))).size();
            decimals = qBound(0, decimals, 4);
            uint32_t scale = 1;
            for (int i = 0; i < decimals; ++i) scale *= 10;
            const qint64 raw = qRound64(value->value() * double(scale));
            if (raw < std::numeric_limits<qint32>::min() || raw > std::numeric_limits<quint32>::max()) {
                QMessageBox::warning(this, tr("参数错误"), tr("当前数值使用自动缩放因子后超出32位范围。"));
                return;
            }
            auto bytes = [](uint32_t number) {
                QByteArray p(4, '\0');
                for (int i = 0; i < 4; ++i) p[i] = char((number >> (i * 8)) & 0xff);
                return p;
            };
            sendCommand(0x27, bytes(scale), true);
            const QByteArray payload = bytes(uint32_t(raw));
            QTimer::singleShot(20, this, [this, command = item.command, payload] { sendCommand(command, payload, true); });
            statusBar()->showMessage(tr("%1=%2，自动缩放因子=%3").arg(item.name).arg(value->value(), 0, 'g', 8).arg(scale), 3000);
        });
        ++row;
    }
    root->addWidget(params);

    auto *temperature = new QGroupBox(tr("温度设置"), page);
    auto *tempForm = new QHBoxLayout(temperature);
    auto *over = new QSpinBox(temperature);
    over->setRange(-40, 200);
    over->setValue(85);
    auto *recover = new QSpinBox(temperature);
    recover->setRange(-40, 200);
    recover->setValue(75);
    auto *autoRecover = new QCheckBox(tr("过温恢复模式"), temperature);
    auto *setTemp = new QPushButton(tr("设置温度"), temperature);
    tempForm->addWidget(new QLabel(tr("过温点(°C):"), temperature));
    tempForm->addWidget(over);
    tempForm->addWidget(new QLabel(tr("恢复点(°C):"), temperature));
    tempForm->addWidget(recover);
    tempForm->addWidget(autoRecover);
    tempForm->addWidget(setTemp);
    tempForm->addStretch();
    connect(setTemp, &QPushButton::clicked, this, [this, over, recover, autoRecover] {
        QByteArray p1(2, '\0'); p1[0] = char(over->value() & 0xff); p1[1] = char((over->value() >> 8) & 0xff); sendCommand(0x0f, p1);
        QByteArray p2(2, '\0'); p2[0] = char(recover->value() & 0xff); p2[1] = char((recover->value() >> 8) & 0xff); sendCommand(0x10, p2, true);
        sendCommand(autoRecover->isChecked() ? 0x14 : 0x13);
    });
    root->addWidget(temperature);

    auto *uart = new QGroupBox(tr("LLC UART 模式"), page);
    auto *uartRow = new QHBoxLayout(uart);
    auto *mode = new QComboBox(uart);
    mode->addItems({ tr("原副边通讯"), tr("VOFA调试") });
    auto *apply = new QPushButton(tr("应用模式"), uart);
    uartRow->addWidget(new QLabel(tr("模式:"), uart));
    uartRow->addWidget(mode);
    uartRow->addWidget(apply);
    uartRow->addWidget(new QLabel(tr("VOFA 与 PFC 通讯共用 LLC 的 PA9/PA10，同一时间只能选择一种。"), uart));
    uartRow->addStretch();
    connect(apply, &QPushButton::clicked, this, [this, mode] {
        QByteArray p;
        p.append(char(mode->currentIndex() == 0 ? 1 : 0));
        p.append(char(0xa5));
        p.append(char(0x5a));
        sendCommand(0x45, p);
    });
    root->addWidget(uart);
    root->addStretch();
    return page;
}

QWidget *MainWindow::makeIapPage()
{
    auto *page = new QWidget;
    auto *root = new QVBoxLayout(page);
    auto *settings = new QGroupBox(tr("IAP 升级配置"), page);
    auto *grid = new QGridLayout(settings);
    auto *target = new QComboBox(settings);
    target->addItems({ tr("LLC"), tr("PFC") });
    auto *channel = new QComboBox(settings);
    channel->addItems({ tr("通道1"), tr("通道2") });
    auto *mode = new QComboBox(settings);
    mode->addItems({ tr("节点ID 0xA0000~0xA0007"), tr("固定ID 0xAA55"), tr("自定义ID/范围") });
    auto *customIds = new QLineEdit(settings);
    customIds->setPlaceholderText(tr("例如 B0001-B0008，或 B0001,B0003,AA55"));
    customIds->setVisible(false);
    auto *file = new QLineEdit(settings);
    file->setReadOnly(true);
    auto *browse = new QPushButton(tr("选择"), settings);
    auto *nodes = new QWidget(settings);
    auto *nodeRow = new QHBoxLayout(nodes);
    QList<QCheckBox*> nodeChecks;
    nodeRow->setContentsMargins(0, 0, 0, 0);
    for (int i = 0; i < 8; ++i) {
        auto *check = new QCheckBox(QString(tr("节点%1")).arg(i), nodes);
        nodeChecks << check;
        nodeRow->addWidget(check);
    }
    nodeRow->addStretch();
    grid->addWidget(new QLabel(tr("目标:"), settings), 0, 0);
    grid->addWidget(target, 0, 1);
    grid->addWidget(new QLabel(tr("CAN通道:"), settings), 0, 2);
    grid->addWidget(channel, 0, 3);
    grid->addWidget(new QLabel(tr("CAN ID:"), settings), 1, 0);
    grid->addWidget(mode, 1, 1, 1, 3);
    grid->addWidget(new QLabel(tr("节点:"), settings), 2, 0);
    grid->addWidget(nodes, 2, 1, 1, 3);
    grid->addWidget(new QLabel(tr("自定义ID:"), settings), 3, 0);
    grid->addWidget(customIds, 3, 1, 1, 3);
    grid->addWidget(new QLabel(tr("固件:"), settings), 4, 0);
    grid->addWidget(file, 4, 1, 1, 2);
    grid->addWidget(browse, 4, 3);
    auto *actions = new QHBoxLayout;
    auto *start = new QPushButton(tr("批量升级"), settings);
    auto *stop = new QPushButton(tr("停止"), settings);
    stop->setEnabled(false);
    auto *state = new QLabel(tr("就绪，请确认 CAN 已启动"), settings);
    actions->addWidget(start);
    actions->addWidget(stop);
    actions->addWidget(state);
    actions->addStretch();
    grid->addLayout(actions, 5, 1, 1, 3);
    root->addWidget(settings);
    auto *addressSettings = new QGroupBox(tr("设备 IAP 地址（掉电保存）"), page);
    auto *addressLayout = new QHBoxLayout(addressSettings);
    auto *currentAddress = new QLineEdit(QStringLiteral("AA55"), addressSettings);
    auto *newAddress = new QLineEdit(QStringLiteral("A0000"), addressSettings);
    auto *setAddress = new QPushButton(tr("写入地址"), addressSettings);
    currentAddress->setPlaceholderText(tr("当前地址/救援地址"));
    newAddress->setPlaceholderText(tr("新29位扩展帧ID"));
    addressLayout->addWidget(new QLabel(tr("当前ID:"), addressSettings));
    addressLayout->addWidget(currentAddress);
    addressLayout->addWidget(new QLabel(tr("新ID:"), addressSettings));
    addressLayout->addWidget(newAddress);
    addressLayout->addWidget(setAddress);
    root->addWidget(addressSettings);
    connect(target, &QComboBox::currentIndexChanged, this, [setAddress, addressSettings](int index) {
        const bool llcTarget = index == 0;
        setAddress->setEnabled(llcTarget);
        addressSettings->setToolTip(llcTarget ? QObject::tr("修改 LLC 对外使用的 IAP CAN ID")
                                             : QObject::tr("PFC 经 LLC 网关升级，共用 LLC 的 CAN ID，无需单独设置 PFC CAN ID"));
    });
    auto *bankSettings = new QGroupBox(tr("双 Bank 管理"), page);
    auto *bankLayout = new QHBoxLayout(bankSettings);
    auto *bankHint = new QLabel(tr("查看当前运行 Bank、镜像有效性，并执行校验、切换、确认或回滚。"), bankSettings);
    bankHint->setWordWrap(true);
    auto *bankManage = new QPushButton(tr("打开 Bank 管理"), bankSettings);
    bankLayout->addWidget(bankHint, 1);
    bankLayout->addWidget(bankManage);
    connect(bankManage, &QPushButton::clicked, this, &MainWindow::openBankManager);
    root->addWidget(bankSettings);
    auto *progress = new QProgressBar(page);
    progress->setRange(0, 100);
    root->addWidget(progress);
    auto *hint = new QLabel(tr("DBIAP 为掉电安全双 Bank 整包（可同时升级 Bootloader+APP）；BIN/HEX 继续使用旧产品单 APP 升级。PFC 路径为 CAN→LLC→UART→PFC，上位机会在写入前确认 PFC Bootloader 已真实应答；包内地址 LLC=2、PFC=1。"), page);
    hint->setWordWrap(true);
    root->addWidget(hint);
    auto *log = new QPlainTextEdit(page);
    log->setReadOnly(true);
    log->setPlaceholderText(tr("IAP 日志"));
    root->addWidget(log, 1);

    connect(browse, &QPushButton::clicked, this, [this, file] {
        const auto path = QFileDialog::getOpenFileName(this, tr("选择固件"), {}, tr("固件文件 (*.dbiap *.bin *.hex)"));
        if (!path.isEmpty()) file->setText(path);
    });
    connect(mode, &QComboBox::currentIndexChanged, this, [nodes, customIds, start](int index) {
        nodes->setVisible(index == 0);
        customIds->setVisible(index == 2);
        start->setText(index == 0 ? tr("批量升级") : (index == 1 ? tr("固定ID升级") : tr("自定义ID升级")));
    });
    connect(setAddress, &QPushButton::clicked, this, [this, currentAddress, newAddress, target, channel, log, progress, setAddress] {
        bool currentOk = false, newOk = false;
        const uint32_t currentId = currentAddress->text().trimmed().toUInt(&currentOk, 16);
        const uint32_t nextId = newAddress->text().trimmed().toUInt(&newOk, 16);
        if (!currentOk || !newOk || currentId == 0 || nextId == 0 ||
            currentId > 0x1FFFFFFFU || nextId > 0x1FFFFFFFU) {
            QMessageBox::warning(this, tr("IAP 地址"), tr("请输入合法的29位十六进制CAN ID。")); return;
        }
        const int canChannel = channel->currentIndex();
        const uint8_t targetAddress = uint8_t(target->currentIndex() == 0 ? 2 : 1);
        IapUpgrade::Options options;
        options.canId = currentId; options.targetAddress = targetAddress;
        options.addressOnly = true; options.newCanId = nextId;
        setAddress->setEnabled(false); progress->setValue(0);
        log->appendPlainText(QStringLiteral("--- 修改 IAP 地址 0x%1 -> 0x%2 ---").arg(currentId,0,16).arg(nextId,0,16).toUpper());
        iap_.start(options,
            [this, canChannel, currentId](const QByteArray &packet) {
                for (int offset = 0; offset < packet.size(); offset += 8) {
                    QString err;
                    if (!device_.sendCan(canChannel, currentId, true, false, packet.mid(offset, 8), 0, &err)) return false;
                }
                return true;
            },
            [this, log](const QString &text) { QMetaObject::invokeMethod(this, [log, text] { log->appendPlainText(text); }, Qt::QueuedConnection); },
            [this, progress](int value) { QMetaObject::invokeMethod(this, [progress, value] { progress->setValue(value); }, Qt::QueuedConnection); },
            [this, setAddress](bool ok) { QMetaObject::invokeMethod(this, [setAddress, ok] { setAddress->setEnabled(true); if (!ok) QMessageBox::warning(setAddress, QObject::tr("IAP 地址"), QObject::tr("地址写入失败。可尝试使用救援地址 AA55。")); }, Qt::QueuedConnection); });
    });

    connect(start, &QPushButton::clicked, this, [this, file, log, state, progress, start, stop, target, channel, mode, customIds, nodeChecks] {
        if (file->text().isEmpty()) { QMessageBox::warning(this, tr("固件升级"), tr("请先选择 DBIAP、BIN 或 HEX 固件文件。")); return; }
        auto ids = std::make_shared<QList<uint32_t>>();
        if (mode->currentIndex() == 1) {
            *ids << 0xAA55U;
        } else if (mode->currentIndex() == 0) {
            for (int i = 0; i < nodeChecks.size(); ++i) if (nodeChecks[i]->isChecked()) *ids << 0xA0000U + uint32_t(i);
        } else {
            const auto parts = customIds->text().split(',', Qt::SkipEmptyParts);
            for (const auto &raw : parts) {
                const auto item = raw.trimmed();
                const int dash = item.indexOf('-');
                bool ok1 = false, ok2 = false;
                const uint32_t first = item.left(dash < 0 ? item.size() : dash).toUInt(&ok1, 16);
                const uint32_t last = dash < 0 ? first : item.mid(dash + 1).toUInt(&ok2, 16);
                if (dash < 0) ok2 = ok1;
                if (!ok1 || !ok2 || first == 0 || last < first || last > 0x1FFFFFFFU || last - first > 255U) {
                    QMessageBox::warning(this, tr("固件升级"), tr("自定义ID格式错误或范围过大。")); return;
                }
                for (uint32_t id = first; id <= last; ++id) if (!ids->contains(id)) *ids << id;
            }
        }
        if (ids->isEmpty()) { QMessageBox::warning(this, tr("固件升级"), tr("请至少选择一个节点。")); return; }
        auto index = std::make_shared<int>(0);
        auto next = std::make_shared<std::function<void()>>();
        const int canChannel = channel->currentIndex();
        const uint8_t targetAddress = uint8_t(target->currentIndex() == 0 ? 2 : 1);
        const QString firmware = file->text();
        log->clear();
        progress->setValue(0);
        start->setEnabled(false);
        stop->setEnabled(true);
        state->setText(tr("批量升级中"));
        *next = [this, ids, index, next, canChannel, targetAddress, firmware, log, state, progress, start, stop]() {
            if (*index >= ids->size()) {
                start->setEnabled(true); stop->setEnabled(false); state->setText(tr("全部节点升级完成")); *next = {}; return;
            }
            const uint32_t canId = ids->at(*index);
            log->appendPlainText(QStringLiteral("--- 开始升级 0x%1 (%2/%3) ---").arg(canId, 5, 16, QLatin1Char('0')).arg(*index + 1).arg(ids->size()));
            IapUpgrade::Options options{ firmware, canId, targetAddress };
            options.dualBank = firmware.endsWith(QStringLiteral(".dbiap"), Qt::CaseInsensitive);
            iap_.start(options,
                [this, canChannel, canId](const QByteArray &packet) {
                    for (int offset = 0; offset < packet.size(); offset += 8) {
                        QString err;
                        if (!device_.sendCan(canChannel, canId, true, false, packet.mid(offset, 8), 0, &err)) return false;
                    }
                    return true;
                },
                [this, log](const QString &text) {
                    QMetaObject::invokeMethod(this, [log, text] { log->appendPlainText(QTime::currentTime().toString(QStringLiteral("hh:mm:ss ")) + text); }, Qt::QueuedConnection);
                },
                [this, progress](int value) {
                    QMetaObject::invokeMethod(this, [progress, value] { progress->setValue(value); }, Qt::QueuedConnection);
                },
                [this, index, next, start, stop, state](bool ok) {
                    QMetaObject::invokeMethod(this, [index, next, start, stop, state, ok] {
                        if (!ok) { start->setEnabled(true); stop->setEnabled(false); state->setText(tr("升级失败或已取消")); *next = {}; return; }
                        ++*index;
                        (*next)();
                    }, Qt::QueuedConnection);
                });
        };
        (*next)();
    });
    connect(stop, &QPushButton::clicked, this, [this, state] {
        state->setText(tr("正在停止..."));
        iap_.cancel();
    });
    return page;
}
