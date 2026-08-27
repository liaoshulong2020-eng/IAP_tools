#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "candevice.h"
#include "configmanager.h"
#include "csvwriter.h"
#include "iapupgrade.h"
#include "recvthread.h"
#include "sendthread.h"

#include <array>
#include <QElapsedTimer>
#include <QHash>
#include <QMainWindow>
#include <QTimer>

class QAction;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;
class QSpinBox;
class QTableView;
class QTableWidget;
class QGroupBox;
class MyTableModel;
class listSendDlg;

// 单通道发送面板控件集合（对象名与 on_* 自动连接对应）
struct ChannelPanel {
    QComboBox  *protocol = nullptr;  // ch?ProtocolCombo
    QComboBox  *frameType = nullptr; // ch?FrameTypeCombo
    QComboBox  *frameData = nullptr; // ch?FrameDataCombo
    QComboBox  *sendType  = nullptr; // ch?SendTypeCombo
    QComboBox  *sendLen   = nullptr; // ch?sendLenCombo
    QLineEdit  *idEdit    = nullptr; // ch?SendIDEdit
    QLineEdit  *dataEdit  = nullptr; // ch?SendDataEdit
    QLineEdit  *countEdit = nullptr; // ch?SendCountEdit
    QLineEdit  *invEdit   = nullptr; // ch?SendInvEdit
    QCheckBox  *idAdd     = nullptr; // ch?IDAddChk
    QCheckBox  *dataAdd   = nullptr; // ch?DataAddChk
    QCheckBox  *continuous= nullptr; // 持续发送（不限次数）
    QPushButton *sendBtn  = nullptr; // ch?SendBtn
    QPushButton *stopBtn  = nullptr; // ch?StopSendBtn
};

// 监控页：一个电源模块节点（10 节点之一）
struct MonitorUi {
    QLabel *channelLabel = nullptr;
    QLabel *id = nullptr;
    QLabel *voltage = nullptr;
    QLabel *current = nullptr;
    QLabel *power = nullptr;
    QLabel *temperature = nullptr;
    QLabel *version = nullptr;
    QLabel *count = nullptr;
    QLabel *communication = nullptr;
    QLabel *overVoltage = nullptr;
    QLabel *underVoltage = nullptr;
    QLabel *overCurrent = nullptr;
    QLabel *overTemperature = nullptr;
    QLabel *powerStatus = nullptr;
    QLabel *sharing = nullptr;
    QPushButton *powerControl = nullptr;
    QPushButton *calibrate = nullptr;
    QDoubleSpinBox *actualVoltage = nullptr;
    QDoubleSpinBox *targetVoltage = nullptr;
    QTimer *commFlash = nullptr;
    std::array<QWidget*, 19> cells{};
    quint64 frames = 0;
    quint64 lastCommMs = 0;
    uint32_t deviceId = 0;
    int channel = -1;
    double currentValue = 0.0;
    bool powerOn = false;
    bool visible = false;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 自动连接槽（对象名 on_<object>_<signal>）
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_cleanListBtn_clicked();
    void on_checkBox_clicked(bool checked);
    void on_ch1SendBtn_clicked();
    void on_ch1StopSendBtn_clicked();
    void on_ch2SendBtn_clicked();
    void on_ch2StopSendBtn_clicked();
    void on_ch1ProtocolCombo_currentIndexChanged(int index);
    void on_ch2ProtocolCombo_currentIndexChanged(int index);
    void on_ch1FrameDataCombo_currentIndexChanged(int index);
    void on_ch2FrameDataCombo_currentIndexChanged(int index);
    void on_ch1sendLenCombo_currentIndexChanged(int index);
    void on_ch2sendLenCombo_activated(int index);
    void on_ch1SendDataEdit_textEdited(const QString &text);
    void on_ch2SendDataEdit_textEdited(const QString &text);
    void on_ch1SendDataEdit_textChanged(const QString &text);
    void on_ch2SendDataEdit_textChanged(const QString &text);
    void on_tableView_clicked(const QModelIndex &index);
    void on_tableView_pressed(const QModelIndex &index);

    // 自定义槽
    void setCh1SendData(int len);
    void setCh2SendData(int len);

    // 菜单动作
    void openDeviceOperations();
    void openDeviceInfo();
    void openParameterSetting();
    void openFilterSetting();
    void openListSend();
    void openFileSend();
    void openRealSave();
    void openFirmwareUpgrade();
    void clearLog();
    void resetCount();
    void setListBufferCount();
    void toggleMerge(bool checked);
    void onRealSaveToggled(bool checked);
    void switchLanguage(const QString &langFile);
    void showAbout();
    void openHelpFolder();

    // 收发
    void onRecvData(const QList<ModelItem> &items);
    void onSendedData(const QList<ModelItem> &items);
    void onEndSend();
    void onSendFailed(const QString &error);

private:
    void buildUi();
    QGroupBox *buildChannelPanel(int channel);
    void refreshRate();
    void appendLog(const QString &msg);
    void startSend(int channel);
    void stopSend(int channel);
    void openSequenceSend(int channel);
    bool parseSendData(int channel, QByteArray &data, quint32 &id);
    void saveConfig();
    void loadConfigIntoUi();
    QString chPrefix(int channel) const;
    QString csvRow(const ModelItem &item);
    QString monitorCsvRow(const ModelItem &item);
    void toggleMonitorRecording(bool enabled);
    void toggleDevice();
    void updateStatus();
    void pollDeviceStatus();

    // 电源监控协议
    static uint8_t crc8(const QByteArray &data);
    void sendCommand(uint8_t command, const QByteArray &payload = {}, bool withCrc = false, bool quiet = false);
    void sendCommandToDevice(int channel, uint32_t id, uint8_t command, const QByteArray &payload = {}, bool withCrc = false);
    void updateMonitor(const ModelItem &frame);
    void updateCurrentSharing();
    void parseProtocol(const ModelItem &frame);
    QWidget *makeMonitorPage();
    QWidget *makeProtectPage();
    QWidget *makeParameterPage();
    QWidget *makeIapPage();

    // 设备与线程
    CanDevice device_;
    recvThread recvThread_;
    sendThread sendThread_[2];
    IapUpgrade iap_;
    listSendDlg *sequenceDlg_[2]{ nullptr, nullptr };

    // UI
    ChannelPanel ch_[2];
    QTableView *tableView_ = nullptr;
    MyTableModel *model_ = nullptr;
    QLabel *ch1RxLabel_ = nullptr;
    QLabel *ch1TxLabel_ = nullptr;
    QLabel *ch2RxLabel_ = nullptr;
    QLabel *ch2TxLabel_ = nullptr;
    QPlainTextEdit *logEdit_ = nullptr;
    QCheckBox *checkBox_ = nullptr;  // 暂停显示
    QCheckBox *realSaveCheck_ = nullptr;  // 实时保存
    QCheckBox *mergeCheck_ = nullptr;      // 合并列表数据
    QPushButton *openDeviceBtn_ = nullptr; // 打开设备
    QLabel *deviceStatusLabel_ = nullptr;  // 设备状态指示
    QTableWidget *llcTable_ = nullptr;
    QTableWidget *pfcTable_ = nullptr;
    QProgressBar *busLoad_ = nullptr;
    QLabel *queryCountLabel_ = nullptr;   // 查询发送计数
    QLabel *totalSharingLabel_ = nullptr; // 总均流度

    // 监控协议状态
    std::array<MonitorUi, 10> monitor_{};
    QHash<uint32_t, int> monitorSlots_;
    std::array<uint32_t, 2> masterIds_{ 0x20, 0 };
    std::array<bool, 2> commandChannels_{ true, false };
    quint8 queryFrameCount_ = 0;
    quint8 versionFrameCount_ = 0;
    bool protectQueryInProgress_ = false;

    // 统计
    quint64 rxCount_[2] = { 0, 0 };
    quint64 txCount_[2] = { 0, 0 };
    quint64 lastRx_[2] = { 0, 0 };
    quint64 lastTx_[2] = { 0, 0 };
    double rxRate_[2] = { 0.0, 0.0 };
    double txRate_[2] = { 0.0, 0.0 };
    QElapsedTimer rateElapsed_;
    QTimer rateTimer_;
    QTimer queryTimer_;                 // v1.21：主线程稳定周期查询
    QTimer devicePollTimer_;
    quint64 querySendCount_ = 0;

    ConfigManager config_;
    bool pauseDisplay_ = false;
    bool devicePresent_ = false;
    bool languageSwitching_ = false;

    // 实时保存（后台写线程）
    CsvWriter csvWriter_;
    CsvWriter monitorCsvWriter_;
    bool realSaving_ = false;
    bool monitorSaving_ = false;
    quint64 saveSeq_ = 0;
    quint64 monitorSaveSeq_ = 0;
    QString realSavePath_;
    QString monitorSavePath_;
};

#endif // MAINWINDOW_H
