using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Threading;
using System.IO;
using ZLGCAN;
using System.Globalization;
using ClosedXML.Excel;
using SeeSharpTools.JY.GUI;

namespace CAN_TOOLS
{
    public partial class MainForm : Form
    {
        private volatile bool isTimerProcessing = false;
        private readonly object timerLock = new object();
        private System.Threading.Timer? _dataProcessTimer = null;
        private readonly System.Collections.Concurrent.ConcurrentQueue<(uint id, byte[] data, uint ch)> _dataQueue
            = new System.Collections.Concurrent.ConcurrentQueue<(uint, byte[], uint)>();
        private int _dataProcessPending = 0;
        private int _dataQueueLength = 0;
        private long _lastUiDataRefreshMs = 0;
        private const int DATA_PROCESS_INTERVAL_MS = 100;
        private const int DATA_PROCESS_BATCH_LIMIT = 5000;
        private const int DATA_QUEUE_WARN_LIMIT = 50000;
        private const int DATA_QUEUE_DROP_LIMIT = 80000;
        private const int UI_DATA_REFRESH_INTERVAL_MS = 200;

        // 命令定义（匹配下位机）
        private const byte CMD_QUERY = 0x01;
        private const byte CMD_VERSION = 0x02;
        private const byte CMD_SCAN = 0x03;
        private const byte CMD_STOP = 0x04;
        private const byte CMD_START = 0x05;
        private const byte CMD_STORE_FLASH = 0x11;
        private const byte CMD_LOAD_FLASH = 0x12;
        private const byte CMD_TEMP_RECOVER_OFF = 0x13;
        private const byte CMD_TEMP_RECOVER_ON = 0x14;
        private const byte CMD_OVERTEMP_POINT = 0x0F;      // 15 - 过温点（16位，不带CRC）
        private const byte CMD_OVERTEMP_REC_POINT = 0x10;  // 16 - 过温恢复点（16位，带CRC）
        private const byte CMD_FACTOR_VOLTAGE = 0x22;      // 校准电压（16位，不带CRC）
        private const byte CMD_THEOR_VOLTAGE = 0x23;       // 理论电压（16位，不带CRC）
        private const byte CMD_VOLTAGE_CRC = 0x24;         // 电压CRC校验
        private const byte CMD_KP = 0x28;                  // 40 - KP参数（32位，带CRC）
        private const byte CMD_KI = 0x29;                  // 41 - KI参数（32位，带CRC）  
        private const byte CMD_TEST = 0x42;                // 测试参数（32位，不带CRC）
        private const byte CMD_TEST2 = 0x43;               // 测试参数2（32位，带CRC）
        private const byte CMD_UART_MODE = 0x45;           // LLC UART模式切换：0=VOFA调试，1=原副边通讯
        private const byte CMD_LLC_VOLTAGE_PROTECT = 0x3F; // 查询LLC电压保护点
        private const byte CMD_LLC_OCP_PROTECT = 0x40; // 查询LLC过流保护点
        private const byte CMD_LLC_OSP_PROTECT = 0x41; // 查询LLC短路保护点
        private const byte CMD_LLC_OUT_PARA = 0x44; // 查询LLC输出参数
        private const byte CMD_LLC_TEMP_PROTECT = 0x3E; // 查询LLC温度保护点
	
        // PFC (原边) 查询命令 —— 对端 MCU variables_define_app.h 中 CommandType 枚举
        private const byte CMD_PFC_INPUT_OVP  = 0x30; // 查询PFC输入过压点
        private const byte CMD_PFC_INPUT_UVP  = 0x31; // 查询PFC输入欠压点
        private const byte CMD_PFC_OUTPUT_OVP = 0x32; // 查询PFC输出过压点
        private const byte CMD_PFC_OUTPUT_UVP = 0x33; // 查询PFC输出欠压点
        private const byte CMD_PFC_INPUT_OCP  = 0x34; // 查询PFC输入过流点
        private const byte CMD_PFC_DATA       = 0x35; // 查询PFC运行数据
        private const byte CMD_PFC_DATA_LIVE1 = 0x36; // 查询PFC实时数据1
        private const byte CMD_PFC_DATA_LIVE2 = 0x37; // 查询PFC实时数据2

        // 缩放因子
        private const int KP_KI_SCALE_FACTOR = 100;

        // 改进的负载检测相关变量
        private DateTime _lastBusLoadCalculationTime = DateTime.Now;
        private DateTime _lastDisplayTime = DateTime.MinValue;

        // 分别统计CAN和CANFD帧
        private int _frameCountCh1_CAN = 0;
        private int _frameCountCh1_CANFD = 0;
        private int _frameCountCh2_CAN = 0;
        private int _frameCountCh2_CANFD = 0;
        private int _totalFrameCount = 0;

        // 发送帧统计
        private int _sentFrameCountCh1 = 0;
        private int _sentFrameCountCh2 = 0;
        private int _totalSentFrameCount = 0;

        private double _currentBusLoadCh1 = 0.0;
        private double _currentBusLoadCh2 = 0.0;
        private double _totalBusLoad = 0.0;
        private readonly object _busLoadLock = new object();

        // 负载计算和显示参数
        private const int BUS_LOAD_CALCULATION_INTERVAL_MS = 1000;
        private const int DISPLAY_INTERVAL_SECONDS = 5;
        private const int AVERAGE_CAN_FRAME_BITS = 64;
        private const int AVERAGE_CANFD_FRAME_BITS = 100;

        const int NULL = 0;
        const int CANFD_BRS = 0x01;
        const int CANFD_ESI = 0x02;
        const int CAN_MAX_DLC = 8;
        const int CAN_MAX_DLEN = 8;
        const int CANFD_MAX_DLC = 15;
        const int CANFD_MAX_DLEN = 64;
        const uint CAN_EFF_FLAG = 0x80000000U;
        const uint CAN_RTR_FLAG = 0x40000000U;
        const uint CAN_ERR_FLAG = 0x20000000U;
        const uint CAN_ID_FLAG = 0x1FFFFFFFU;

        // 8个模块的均流度相关变量
        private readonly object _currentSharingLock = new object();
        private Dictionary<uint, double> _moduleCurrents = new Dictionary<uint, double>();
        private Dictionary<uint, double> _moduleSharingPercent = new Dictionary<uint, double>();
        private List<uint> _activeModules = new List<uint>();
        private double _totalCurrentSharing = 100.0;
        private DateTime _lastCurrentSharingCalculation = DateTime.Now;
        private const int CURRENT_SHARING_UPDATE_INTERVAL_MS = 1000;

        // 目标模块ID范围定义（10个模块）
        private const uint TARGET_ID_START = 0xA0000;
        private const uint TARGET_ID_END = 0xA0009;

        // 通道数量常量
        private const int CHANNEL_COUNT = 10;

        // 保持原有的字典以兼容现有代码
        private Dictionary<uint, double> _channelCurrents = new Dictionary<uint, double>();
        private Dictionary<uint, uint> _idToModuleMap = new Dictionary<uint, uint>();
        private double _currentSharing = 100.0;

        DeviceInfo[] kDeviceType =
        {
            new DeviceInfo(Define.ZCAN_USBCANFD_200U, 2),
            new DeviceInfo(Define.ZCAN_USBCANFD_100U, 1),
            new DeviceInfo(Define.ZCAN_USBCANFD_MINI, 1)
        };

        uint[] kAbitTiming =
        {
            1000000, 800000, 500000, 250000, 125000, 100000, 50000
        };

        uint[] kDbitTiming =
        {
            5000000, 4000000, 2000000, 1000000
        };

        int channel_index_;
        IntPtr device_handle_;
        IntPtr channel_handle_;
        IntPtr channel_handle2_;
        IProperty property_;
        RecvDataThread? recv_data_thread_;

        bool m_bOpen = false;
        bool m_bStart = false;

        // 显示控件字典
        private Dictionary<string, Label> displayLabels = new Dictionary<string, Label>();
        private Dictionary<string, ProgressBar> progressBars = new Dictionary<string, ProgressBar>();

        // 设备信息映射表
        private Dictionary<uint, DeviceData> devicesInfoMap = new Dictionary<uint, DeviceData>();
        private Dictionary<uint, uint> idToChannelMap = new Dictionary<uint, uint>();
        private Dictionary<uint, uint> channelToPureId = new Dictionary<uint, uint>(); // display ch → pureId
        private readonly object _idMapLock = new object();
        private Dictionary<uint, DateTime> deviceLastResponseTime = new Dictionary<uint, DateTime>();
        private readonly int DEVICE_TIMEOUT_SECONDS = 10;
        private List<uint> registeredIds = new List<uint>();

        // Excel记录相关
        private string? _excelFilePath = null;
        private readonly System.Collections.Concurrent.ConcurrentQueue<(DateTime ts, uint id, double voltage, double current, double temperature, uint channel, byte powerStatus)> _excelQueue
            = new System.Collections.Concurrent.ConcurrentQueue<(DateTime, uint, double, double, double, uint, byte)>();
        private readonly object _excelWriteLock = new object();
        private System.Threading.Timer? _excelFlushTimer = null;

        // 帧计数器（自动 0x00~0xFF 循环，与旧版一致）
        private byte _queryFrameCnt = 0;
        private byte _versionFrameCnt = 0;
        private CanIapUpgradeSession? _iapUpgradeSession;
        private CancellationTokenSource? _iapUpgradeCts;
        private volatile bool _iapUpgradeInProgress = false;
        private bool _autoRefreshWasEnabledBeforeIap = false;
        private TextBox? _iapFileTextBox;
        private ComboBox? _iapTargetComboBox;
        private ComboBox? _iapChannelComboBox;
        private ComboBox? _iapCanIdModeComboBox;
        private CheckedListBox? _iapNodesCheckedListBox;
        private ProgressBar? _iapProgressBar;
        private Button? _iapStartButton;
        private Button? _iapStopButton;
        private Label? _iapStatusLabel;
        private Label? _iapProgressValueLabel;
        private RichTextBox? _iapLogTextBox;
        private ComboBox? _uartModeComboBox;
        private Label? _uartModeStatusLabel;

        // 每通道发送计数（TX）
        private readonly Dictionary<uint, uint> _txCountPerChannel = new Dictionary<uint, uint>();

        // 通讯指示灯：每通道最后收到数据的时间 + 闪烁定时器
        private readonly DateTime[] _lastCommTime = new DateTime[CHANNEL_COUNT + 1]; // 索引1~10
        private readonly System.Threading.Timer?[] _commFlashTimers = new System.Threading.Timer?[CHANNEL_COUNT + 1];
        private const int COMM_RED_TIMEOUT_MS = 3000;  // 3秒无数据变红
        private const int GREEN_FLASH_MS = 100;         // 绿色持续100ms后变灰

        // 偏移量
        private double voltage_offset = 0;
        private double current_offset = 0;

        public MainForm()
        {
            InitializeComponent();
            NormalizeMainLayout();
            InitializeCurrentSharingDisplay();
            // 设置默认选项（放在构造函数中，避免Designer重写时丢失）
            comboBox_device.SelectedIndex = 0;    // USBCANFD-200U
            comboBox_index.SelectedIndex = 0;
            comboBox_mode.SelectedIndex = 0;      // 正常
            comboBox_ABIT.SelectedIndex = 4;      // 125kbps
            comboBox_ABIT2.SelectedIndex = 0;     // 5Mbps
            comboBox_frametype.SelectedIndex = 1; // 扩展帧
            comboBox_protocol.SelectedIndex = 0;  // CAN
            comboBox_channel.SelectedIndex = 0;
            // displayLabels 注册（放在构造函数中，避免Designer重写时丢失）
            RegisterDisplayLabels();
            // 在代码中创建电源开关指示灯（避免修改庞大的 Designer.cs）
            CreatePowerLedLabels();
            CreateFirmwareUpgradeTab();
            CreateUartModeControls();
            Text = "CAN工具 - 5800上位机 - IAP版";
        }

        private void NormalizeMainLayout()
        {
            panel_log.Height = 240;
            panel_log.MinimumSize = new Size(0, 180);
            panel_log.MaximumSize = new Size(0, 320);
            panel_log.Padding = new Padding(8, 6, 8, 8);

            richTextBox_recv.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
            checkBox_autoScroll.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            button_clear.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            checkBox_autoScroll.AutoSize = false;
            checkBox_autoScroll.Text = "保持最新";
            button_clear.Text = "清空";

            groupRecv.Resize += (_, _) => LayoutGlobalLogPanel();
            LayoutGlobalLogPanel();
        }

        private void CreateUartModeControls()
        {
            var group = new GroupBox
            {
                Text = "LLC UART模式",
                Location = new Point(430, 90),
                Size = new Size(430, 182),
                Anchor = AnchorStyles.Top | AnchorStyles.Left
            };

            var modeLabel = new Label
            {
                Text = "模式",
                Location = new Point(18, 34),
                Size = new Size(48, 24),
                TextAlign = ContentAlignment.MiddleLeft
            };

            _uartModeComboBox = new ComboBox
            {
                DropDownStyle = ComboBoxStyle.DropDownList,
                Location = new Point(70, 32),
                Size = new Size(200, 25)
            };
            _uartModeComboBox.Items.AddRange(new object[]
            {
                "调试模式（VOFA）",
                "原副边通讯模式"
            });
            _uartModeComboBox.SelectedIndex = 1;

            var applyButton = new Button
            {
                Text = "应用模式",
                Location = new Point(286, 30),
                Size = new Size(112, 30)
            };
            applyButton.Click += UartModeApplyButton_Click;

            _uartModeStatusLabel = new Label
            {
                Text = "默认原副边通讯；升级或读取PFC数据时使用此模式。",
                Location = new Point(18, 72),
                Size = new Size(392, 44),
                ForeColor = Color.DimGray
            };

            var hintLabel = new Label
            {
                Text = "VOFA与PFC通讯共用LLC的PA9/PA10，同一时间只能选择一种。",
                Location = new Point(18, 122),
                Size = new Size(392, 42),
                ForeColor = Color.DarkSlateGray
            };

            group.Controls.Add(modeLabel);
            group.Controls.Add(_uartModeComboBox);
            group.Controls.Add(applyButton);
            group.Controls.Add(_uartModeStatusLabel);
            group.Controls.Add(hintLabel);
            tabSettings.Controls.Add(group);
        }

        private void LayoutGlobalLogPanel()
        {
            if (groupRecv.ClientSize.Width <= 0 || groupRecv.ClientSize.Height <= 0)
                return;

            const int margin = 8;
            const int titleHeight = 22;
            const int sideWidth = 126;
            int sideLeft = Math.Max(margin, groupRecv.ClientSize.Width - sideWidth - margin);

            checkBox_autoScroll.Location = new Point(sideLeft, titleHeight + 2);
            checkBox_autoScroll.Size = new Size(sideWidth, 26);
            button_clear.Location = new Point(sideLeft, titleHeight + 36);
            button_clear.Size = new Size(82, 32);

            richTextBox_recv.Location = new Point(margin, titleHeight);
            richTextBox_recv.Size = new Size(
                Math.Max(120, sideLeft - margin * 2),
                Math.Max(60, groupRecv.ClientSize.Height - titleHeight - margin));
        }

        private void RegisterDisplayLabels()
        {
            displayLabels["id_ch1"] = lbl_id_val_01; displayLabels["voltage_ch1"] = lbl_v_val_01;
            displayLabels["current_ch1"] = lbl_a_val_01; displayLabels["power_ch1"] = lbl_w_val_01;
            displayLabels["temp_ch1"] = lbl_t_val_01;
            displayLabels["version_ch1"] = lbl_ver_val_01; displayLabels["comm_led_ch1"] = lbl_comm_01;
            displayLabels["share_ch1_label"] = lbl_share_val_01; displayLabels["tx_cnt_ch1"] = lbl_cnt_val_01;
            displayLabels["led_overvolt_ch1"] = led_ov_01; displayLabels["led_undervolt_ch1"] = led_uv_01;
            displayLabels["led_overcurr_ch1"] = led_oc_01; displayLabels["led_overtemp_ch1"] = led_ot_01;

            displayLabels["id_ch2"] = lbl_id_val_02; displayLabels["voltage_ch2"] = lbl_v_val_02;
            displayLabels["current_ch2"] = lbl_a_val_02; displayLabels["power_ch2"] = lbl_w_val_02;
            displayLabels["temp_ch2"] = lbl_t_val_02;
            displayLabels["version_ch2"] = lbl_ver_val_02; displayLabels["comm_led_ch2"] = lbl_comm_02;
            displayLabels["share_ch2_label"] = lbl_share_val_02; displayLabels["tx_cnt_ch2"] = lbl_cnt_val_02;
            displayLabels["led_overvolt_ch2"] = led_ov_02; displayLabels["led_undervolt_ch2"] = led_uv_02;
            displayLabels["led_overcurr_ch2"] = led_oc_02; displayLabels["led_overtemp_ch2"] = led_ot_02;

            displayLabels["id_ch3"] = lbl_id_val_03; displayLabels["voltage_ch3"] = lbl_v_val_03;
            displayLabels["current_ch3"] = lbl_a_val_03; displayLabels["power_ch3"] = lbl_w_val_03;
            displayLabels["temp_ch3"] = lbl_t_val_03;
            displayLabels["version_ch3"] = lbl_ver_val_03; displayLabels["comm_led_ch3"] = lbl_comm_03;
            displayLabels["share_ch3_label"] = lbl_share_val_03; displayLabels["tx_cnt_ch3"] = lbl_cnt_val_03;
            displayLabels["led_overvolt_ch3"] = led_ov_03; displayLabels["led_undervolt_ch3"] = led_uv_03;
            displayLabels["led_overcurr_ch3"] = led_oc_03; displayLabels["led_overtemp_ch3"] = led_ot_03;

            displayLabels["id_ch4"] = lbl_id_val_04; displayLabels["voltage_ch4"] = lbl_v_val_04;
            displayLabels["current_ch4"] = lbl_a_val_04; displayLabels["power_ch4"] = lbl_w_val_04;
            displayLabels["temp_ch4"] = lbl_t_val_04;
            displayLabels["version_ch4"] = lbl_ver_val_04; displayLabels["comm_led_ch4"] = lbl_comm_04;
            displayLabels["share_ch4_label"] = lbl_share_val_04; displayLabels["tx_cnt_ch4"] = lbl_cnt_val_04;
            displayLabels["led_overvolt_ch4"] = led_ov_04; displayLabels["led_undervolt_ch4"] = led_uv_04;
            displayLabels["led_overcurr_ch4"] = led_oc_04; displayLabels["led_overtemp_ch4"] = led_ot_04;

            displayLabels["id_ch5"] = lbl_id_val_05; displayLabels["voltage_ch5"] = lbl_v_val_05;
            displayLabels["current_ch5"] = lbl_a_val_05; displayLabels["power_ch5"] = lbl_w_val_05;
            displayLabels["temp_ch5"] = lbl_t_val_05;
            displayLabels["version_ch5"] = lbl_ver_val_05; displayLabels["comm_led_ch5"] = lbl_comm_05;
            displayLabels["share_ch5_label"] = lbl_share_val_05; displayLabels["tx_cnt_ch5"] = lbl_cnt_val_05;
            displayLabels["led_overvolt_ch5"] = led_ov_05; displayLabels["led_undervolt_ch5"] = led_uv_05;
            displayLabels["led_overcurr_ch5"] = led_oc_05; displayLabels["led_overtemp_ch5"] = led_ot_05;

            displayLabels["id_ch6"] = lbl_id_val_06; displayLabels["voltage_ch6"] = lbl_v_val_06;
            displayLabels["current_ch6"] = lbl_a_val_06; displayLabels["power_ch6"] = lbl_w_val_06;
            displayLabels["temp_ch6"] = lbl_t_val_06;
            displayLabels["version_ch6"] = lbl_ver_val_06; displayLabels["comm_led_ch6"] = lbl_comm_06;
            displayLabels["share_ch6_label"] = lbl_share_val_06; displayLabels["tx_cnt_ch6"] = lbl_cnt_val_06;
            displayLabels["led_overvolt_ch6"] = led_ov_06; displayLabels["led_undervolt_ch6"] = led_uv_06;
            displayLabels["led_overcurr_ch6"] = led_oc_06; displayLabels["led_overtemp_ch6"] = led_ot_06;

            displayLabels["id_ch7"] = lbl_id_val_07; displayLabels["voltage_ch7"] = lbl_v_val_07;
            displayLabels["current_ch7"] = lbl_a_val_07; displayLabels["power_ch7"] = lbl_w_val_07;
            displayLabels["temp_ch7"] = lbl_t_val_07;
            displayLabels["version_ch7"] = lbl_ver_val_07; displayLabels["comm_led_ch7"] = lbl_comm_07;
            displayLabels["share_ch7_label"] = lbl_share_val_07; displayLabels["tx_cnt_ch7"] = lbl_cnt_val_07;
            displayLabels["led_overvolt_ch7"] = led_ov_07; displayLabels["led_undervolt_ch7"] = led_uv_07;
            displayLabels["led_overcurr_ch7"] = led_oc_07; displayLabels["led_overtemp_ch7"] = led_ot_07;

            displayLabels["id_ch8"] = lbl_id_val_08; displayLabels["voltage_ch8"] = lbl_v_val_08;
            displayLabels["current_ch8"] = lbl_a_val_08; displayLabels["power_ch8"] = lbl_w_val_08;
            displayLabels["temp_ch8"] = lbl_t_val_08;
            displayLabels["version_ch8"] = lbl_ver_val_08; displayLabels["comm_led_ch8"] = lbl_comm_08;
            displayLabels["share_ch8_label"] = lbl_share_val_08; displayLabels["tx_cnt_ch8"] = lbl_cnt_val_08;
            displayLabels["led_overvolt_ch8"] = led_ov_08; displayLabels["led_undervolt_ch8"] = led_uv_08;
            displayLabels["led_overcurr_ch8"] = led_oc_08; displayLabels["led_overtemp_ch8"] = led_ot_08;

            displayLabels["id_ch9"] = lbl_id_val_09; displayLabels["voltage_ch9"] = lbl_v_val_09;
            displayLabels["current_ch9"] = lbl_a_val_09; displayLabels["power_ch9"] = lbl_w_val_09;
            displayLabels["temp_ch9"] = lbl_t_val_09;
            displayLabels["version_ch9"] = lbl_ver_val_09; displayLabels["comm_led_ch9"] = lbl_comm_09;
            displayLabels["share_ch9_label"] = lbl_share_val_09; displayLabels["tx_cnt_ch9"] = lbl_cnt_val_09;
            displayLabels["led_overvolt_ch9"] = led_ov_09; displayLabels["led_undervolt_ch9"] = led_uv_09;
            displayLabels["led_overcurr_ch9"] = led_oc_09; displayLabels["led_overtemp_ch9"] = led_ot_09;

            displayLabels["id_ch10"] = lbl_id_val_10; displayLabels["voltage_ch10"] = lbl_v_val_10;
            displayLabels["current_ch10"] = lbl_a_val_10; displayLabels["power_ch10"] = lbl_w_val_10;
            displayLabels["temp_ch10"] = lbl_t_val_10;
            displayLabels["version_ch10"] = lbl_ver_val_10; displayLabels["comm_led_ch10"] = lbl_comm_10;
            displayLabels["share_ch10_label"] = lbl_share_val_10; displayLabels["tx_cnt_ch10"] = lbl_cnt_val_10;
            displayLabels["led_overvolt_ch10"] = led_ov_10; displayLabels["led_undervolt_ch10"] = led_uv_10;
            displayLabels["led_overcurr_ch10"] = led_oc_10; displayLabels["led_overtemp_ch10"] = led_ot_10;

            displayLabels["load_text"] = lbl_load;
            progressBars["total_load"] = progressBar_load;

            // DataGridView 预填充参数名行（放在构造函数，避免Designer重写时丢失）
            if (dataGridView_protect.Rows.Count == 0)
            {
                dataGridView_protect.Rows.Add("LLC软件过压点(V)");
                dataGridView_protect.Rows.Add("LLC硬件过压点(V)");
                dataGridView_protect.Rows.Add("LLC欠压点(V)");
                dataGridView_protect.Rows.Add("LLC欠压恢复点(V)");
                dataGridView_protect.Rows.Add("LLC恒流点(A)");
                dataGridView_protect.Rows.Add("LLC软件过流点(A)");
                dataGridView_protect.Rows.Add("LLC过流恢复点(A)");
                dataGridView_protect.Rows.Add("LLC软件短路点(A)");
                dataGridView_protect.Rows.Add("LLC硬件短路点(A)");
                dataGridView_protect.Rows.Add("LLC过温点(°C)");
                dataGridView_protect.Rows.Add("LLC过温恢复点(°C)");
                dataGridView_protect.Rows.Add("LLC目标电压(V)");
                dataGridView_protect.Rows.Add("LLC校准电压(V)");
                dataGridView_protect.Rows.Add("LLC参考电压(V)");
                dataGridView_protect.Rows.Add("LLC KP参数");
                dataGridView_protect.Rows.Add("LLC KI参数");
            }
            if (dataGridView_pfc.Rows.Count == 0)
            {
                dataGridView_pfc.Rows.Add("PFC_VBUS软件过压点(V)");
                dataGridView_pfc.Rows.Add("PFC_VBUS硬件过压点(V)");
                dataGridView_pfc.Rows.Add("PFC_VBUS欠压点(V)");
                dataGridView_pfc.Rows.Add("PFC_VBUS欠压恢复点(V)");
                dataGridView_pfc.Rows.Add("PFC_INPUT软件过流点(A)");
                dataGridView_pfc.Rows.Add("PFC_INPUT硬件过流点(A)");
                dataGridView_pfc.Rows.Add("PFC_INPUT输入过压点(V)");
                dataGridView_pfc.Rows.Add("PFC_INPUT输入过压恢复点(V)");
                dataGridView_pfc.Rows.Add("PFC_INPUT输入欠压点(V)");
                dataGridView_pfc.Rows.Add("PFC_INPUT输入欠压恢复点(V)");
                dataGridView_pfc.Rows.Add("PFC_VBUS目标电压(V)");
                dataGridView_pfc.Rows.Add("PFC VBUS参考电压");
                dataGridView_pfc.Rows.Add("PFC VBUS实际电压");
                dataGridView_pfc.Rows.Add("PFC 输入电压实际值(V)");
                dataGridView_pfc.Rows.Add("PFC 电流环参考值(A)");
                dataGridView_pfc.Rows.Add("PFC NTC温度");
                dataGridView_pfc.Rows.Add("PFC 工作状态");
                dataGridView_pfc.Rows.Add("PFC 开关频率(kHz)");
                dataGridView_pfc.Rows.Add("PFC 占空比(%)");
                dataGridView_pfc.Rows.Add("PFC 状态标志(hex)");
            }
        }

        private void CreateFirmwareUpgradeTab()
        {
            var tabUpgrade = new TabPage("固件升级");
            var root = new TableLayoutPanel
            {
                Dock = DockStyle.Fill,
                ColumnCount = 1,
                RowCount = 4,
                Padding = new Padding(14),
                BackColor = Color.FromArgb(247, 248, 250),
                AutoScroll = true
            };
            root.RowStyles.Add(new RowStyle(SizeType.AutoSize));
            root.RowStyles.Add(new RowStyle(SizeType.Absolute, 48));
            root.RowStyles.Add(new RowStyle(SizeType.AutoSize));
            root.RowStyles.Add(new RowStyle(SizeType.Percent, 100));

            var settings = new GroupBox
            {
                Text = "IAP升级配置",
                Dock = DockStyle.Top,
                AutoSize = true,
                AutoSizeMode = AutoSizeMode.GrowAndShrink,
                Padding = new Padding(14, 18, 14, 14),
                Margin = new Padding(0, 0, 0, 10)
            };

            var settingsLayout = new TableLayoutPanel
            {
                Dock = DockStyle.Top,
                AutoSize = true,
                ColumnCount = 4,
                RowCount = 5,
                Padding = new Padding(0),
                Margin = new Padding(0)
            };
            settingsLayout.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 58));
            settingsLayout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 50));
            settingsLayout.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 72));
            settingsLayout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 50));

            Label MakeFieldLabel(string text) => new Label
            {
                Text = text,
                Dock = DockStyle.Fill,
                TextAlign = ContentAlignment.MiddleLeft,
                Margin = new Padding(0, 4, 8, 4),
                AutoSize = false
            };

            _iapTargetComboBox = new ComboBox
            {
                Dock = DockStyle.Fill,
                DropDownStyle = ComboBoxStyle.DropDownList,
                Margin = new Padding(0, 4, 18, 4)
            };
            _iapTargetComboBox.Items.AddRange(new object[] { "LLC", "PFC" });
            _iapTargetComboBox.SelectedIndex = 0;

            _iapChannelComboBox = new ComboBox
            {
                Dock = DockStyle.Fill,
                DropDownStyle = ComboBoxStyle.DropDownList,
                Margin = new Padding(0, 4, 0, 4)
            };
            _iapChannelComboBox.Items.AddRange(new object[] { "通道1", "通道2" });
            _iapChannelComboBox.SelectedIndex = 0;

            _iapCanIdModeComboBox = new ComboBox
            {
                Dock = DockStyle.Fill,
                DropDownStyle = ComboBoxStyle.DropDownList,
                Margin = new Padding(0, 4, 0, 4)
            };
            _iapCanIdModeComboBox.Items.AddRange(new object[]
            {
                "节点ID 0xA0000~0xA0007",
                "固定ID 0xAA55"
            });
            _iapCanIdModeComboBox.SelectedIndex = 0;

            var nodeRow = new FlowLayoutPanel
            {
                Dock = DockStyle.Fill,
                AutoSize = true,
                WrapContents = true,
                Margin = new Padding(0, 2, 0, 6)
            };
            _iapNodesCheckedListBox = new CheckedListBox
            {
                Size = new Size(640, 44),
                MinimumSize = new Size(360, 44),
                IntegralHeight = false,
                CheckOnClick = true,
                MultiColumn = true,
                ColumnWidth = 76,
                Margin = new Padding(0, 0, 12, 0)
            };
            _iapNodesCheckedListBox.Height = Math.Max(44, _iapNodesCheckedListBox.ItemHeight + 14);
            nodeRow.MinimumSize = new Size(0, _iapNodesCheckedListBox.Height + 4);
            for (int node = 0; node < 8; node++)
            {
                _iapNodesCheckedListBox.Items.Add($"节点{node}", false);
            }
            var selectAllNodesButton = new Button { Text = "全选", Size = new Size(70, 28), Margin = new Padding(0, 2, 0, 0) };
            selectAllNodesButton.Click += (_, _) =>
            {
                if (_iapNodesCheckedListBox == null) return;
                bool shouldCheck = _iapNodesCheckedListBox.CheckedItems.Count != _iapNodesCheckedListBox.Items.Count;
                for (int i = 0; i < _iapNodesCheckedListBox.Items.Count; i++)
                    _iapNodesCheckedListBox.SetItemChecked(i, shouldCheck);
            };
            nodeRow.Controls.Add(_iapNodesCheckedListBox);
            nodeRow.Controls.Add(selectAllNodesButton);

            var browseRow = new TableLayoutPanel
            {
                Dock = DockStyle.Fill,
                ColumnCount = 2,
                AutoSize = true,
                Margin = new Padding(0, 4, 0, 4)
            };
            browseRow.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
            browseRow.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 86));
            _iapFileTextBox = new TextBox
            {
                Dock = DockStyle.Fill,
                ReadOnly = true,
                Margin = new Padding(0, 0, 10, 0)
            };
            var browseButton = new Button { Text = "选择", Dock = DockStyle.Fill };
            browseButton.Click += IapBrowseButton_Click;
            browseRow.Controls.Add(_iapFileTextBox, 0, 0);
            browseRow.Controls.Add(browseButton, 1, 0);

            var actionRow = new FlowLayoutPanel
            {
                Dock = DockStyle.Fill,
                AutoSize = false,
                Height = 48,
                WrapContents = true,
                Margin = new Padding(0, 8, 0, 0)
            };
            _iapStartButton = new Button
            {
                Text = "批量升级",
                Size = new Size(170, 40),
                Font = new Font(Font.FontFamily, 10F, FontStyle.Bold),
                Margin = new Padding(0, 0, 12, 0)
            };
            _iapStartButton.Click += IapStartButton_Click;
            _iapStopButton = new Button { Text = "停止", Size = new Size(96, 40), Enabled = false, Margin = new Padding(0, 0, 18, 0) };
            _iapStopButton.Click += IapStopButton_Click;
            _iapStatusLabel = new Label
            {
                Text = "就绪，请确认CAN已启动",
                AutoSize = true,
                ForeColor = Color.DimGray,
                Margin = new Padding(0, 10, 0, 0)
            };
            actionRow.Controls.Add(_iapStartButton);
            actionRow.Controls.Add(_iapStopButton);
            actionRow.Controls.Add(_iapStatusLabel);

            var nodesLabel = MakeFieldLabel("节点");
            void RefreshIapCanIdModeUi()
            {
                bool legacyMode = _iapCanIdModeComboBox.SelectedIndex == 1;
                nodesLabel.Visible = !legacyMode;
                nodeRow.Visible = !legacyMode;
                _iapStartButton.Text = legacyMode ? "固定ID升级" : "批量升级";
                if (_iapStatusLabel != null && !(_iapStopButton?.Enabled ?? false))
                {
                    _iapStatusLabel.Text = legacyMode
                        ? "固定ID模式使用 0xAA55，不需要选择节点"
                        : "就绪，请确认CAN已启动";
                    _iapStatusLabel.ForeColor = Color.DimGray;
                }
            }
            _iapCanIdModeComboBox.SelectedIndexChanged += (_, _) => RefreshIapCanIdModeUi();

            settingsLayout.Controls.Add(MakeFieldLabel("目标"), 0, 0);
            settingsLayout.Controls.Add(_iapTargetComboBox, 1, 0);
            settingsLayout.Controls.Add(MakeFieldLabel("CAN通道"), 2, 0);
            settingsLayout.Controls.Add(_iapChannelComboBox, 3, 0);
            settingsLayout.Controls.Add(MakeFieldLabel("CAN ID"), 0, 1);
            settingsLayout.Controls.Add(_iapCanIdModeComboBox, 1, 1);
            settingsLayout.SetColumnSpan(_iapCanIdModeComboBox, 3);
            settingsLayout.Controls.Add(nodesLabel, 0, 2);
            settingsLayout.Controls.Add(nodeRow, 1, 2);
            settingsLayout.SetColumnSpan(nodeRow, 3);
            settingsLayout.Controls.Add(MakeFieldLabel("固件"), 0, 3);
            settingsLayout.Controls.Add(browseRow, 1, 3);
            settingsLayout.SetColumnSpan(browseRow, 3);
            settingsLayout.Controls.Add(new Label(), 0, 4);
            settingsLayout.Controls.Add(actionRow, 1, 4);
            settingsLayout.SetColumnSpan(actionRow, 3);
            RefreshIapCanIdModeUi();

            settings.Controls.Add(settingsLayout);

            var progressLayout = new TableLayoutPanel
            {
                Dock = DockStyle.Left,
                Width = 760,
                ColumnCount = 3,
                Margin = new Padding(0, 0, 0, 8),
                Padding = new Padding(0)
            };
            progressLayout.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 74));
            progressLayout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
            progressLayout.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 62));
            progressLayout.Controls.Add(new Label
            {
                Text = "升级进度",
                Dock = DockStyle.Fill,
                TextAlign = ContentAlignment.MiddleLeft,
                ForeColor = Color.DimGray
            }, 0, 0);
            _iapProgressBar = new ProgressBar
            {
                Dock = DockStyle.Fill,
                Style = ProgressBarStyle.Continuous,
                Margin = new Padding(0, 10, 10, 8)
            };
            _iapProgressValueLabel = new Label
            {
                Text = "0%",
                Dock = DockStyle.Fill,
                TextAlign = ContentAlignment.MiddleRight,
                Font = new Font(Font.FontFamily, 9F, FontStyle.Bold),
                ForeColor = Color.FromArgb(28, 94, 32)
            };
            progressLayout.Controls.Add(_iapProgressBar, 1, 0);
            progressLayout.Controls.Add(_iapProgressValueLabel, 2, 0);

            var hint = new Label
            {
                Dock = DockStyle.Top,
                AutoSize = true,
                Text = "固定ID模式使用扩展帧ID 0xAA55；节点ID模式使用扩展帧ID 0xA0000~0xA0007；IAP包内地址仍与串口工具一致：LLC=2，PFC=1。",
                ForeColor = Color.DimGray,
                TextAlign = ContentAlignment.TopLeft,
                Margin = new Padding(0, 0, 0, 10)
            };

            var logGroup = new GroupBox
            {
                Text = "IAP日志",
                Dock = DockStyle.Fill,
                Padding = new Padding(10, 18, 10, 10),
                Margin = new Padding(0)
            };
            _iapLogTextBox = new RichTextBox
            {
                Dock = DockStyle.Fill,
                ReadOnly = true,
                BorderStyle = BorderStyle.FixedSingle,
                BackColor = Color.White,
                Font = new Font("Consolas", 9F),
                HideSelection = false
            };
            logGroup.Controls.Add(_iapLogTextBox);

            root.Controls.Add(settings, 0, 0);
            root.Controls.Add(progressLayout, 0, 1);
            root.Controls.Add(hint, 0, 2);
            root.Controls.Add(logGroup, 0, 3);
            tabUpgrade.Controls.Add(root);
            tabControl.Controls.Add(tabUpgrade);
        }

        private void UpdateDisplay(string key, string value)
        {
            if (displayLabels.TryGetValue(key, out var label))
            {
                if (label.InvokeRequired)
                {
                    label.BeginInvoke(new Action(() => label.Text = value));
                }
                else
                {
                    label.Text = value;
                }
            }
        }

        private void UpdateStatusIndicator(string key, bool isError)
        {
            if (displayLabels.TryGetValue(key, out var indicator))
            {
                if (indicator.InvokeRequired)
                {
                    indicator.BeginInvoke(new Action(() =>
                    {
                        indicator.BackColor = isError ? Color.Red : Color.Green;
                    }));
                }
                else
                {
                    indicator.BackColor = isError ? Color.Red : Color.Green;
                }
            }
        }

        private void InitializeCurrentSharingDisplay()
        {
            lock (_currentSharingLock)
            {
                _moduleCurrents.Clear();
                _moduleSharingPercent.Clear();
                _activeModules.Clear();

                // 初始化10个模块的数据结构
                for (uint id = TARGET_ID_START; id <= TARGET_ID_END; id++)
                {
                    _moduleCurrents[id] = 0.0;
                    _moduleSharingPercent[id] = 0.0;
                }

                // 保持原有逻辑兼容
                for (uint id = TARGET_ID_START; id <= TARGET_ID_END; id++)
                {
                    uint moduleIndex = id - TARGET_ID_START + 1; // 模块1~10
                    _channelCurrents[moduleIndex] = 0.0;
                    _idToModuleMap[id] = moduleIndex;
                }

                Console.WriteLine($"已初始化均流度计算，监控模块ID范围：0x{TARGET_ID_START:X} - 0x{TARGET_ID_END:X}（共{CHANNEL_COUNT}个通道）");
            }
        }

        // CAN ID 辅助方法
        public uint MakeCanId(uint id, int eff, int rtr, int err)
        {
            uint ueff = (uint)(eff != 0 ? 1 : 0);
            uint urtr = (uint)(rtr != 0 ? 1 : 0);
            uint uerr = (uint)(err != 0 ? 1 : 0);
            return id | (ueff << 31) | (urtr << 30) | (uerr << 29);
        }

        public bool IsEFF(uint id) => (id & CAN_EFF_FLAG) != 0;
        public bool IsRTR(uint id) => (id & CAN_RTR_FLAG) != 0;
        public bool IsERR(uint id) => (id & CAN_ERR_FLAG) != 0;
        public uint GetId(uint id) => id & CAN_ID_FLAG;

        // 事件处理方法
        private void Button_open_Click(object? sender, EventArgs e)
        {
            uint device_type_index = (uint)comboBox_device.SelectedIndex;
            uint device_index = (uint)comboBox_index.SelectedIndex;

            device_handle_ = Method.ZCAN_OpenDevice(kDeviceType[device_type_index].device_type, device_index, 0);
            if (NULL == (int)device_handle_)
            {
                MessageBox.Show("打开设备失败,请检查设备类型和设备索引号是否正确", "提示",
                        MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }
            m_bOpen = true;
            EnableCtrl(true);
            button_open.Enabled = false;
            button_init.Enabled = true;
            button_close.Enabled = true;

            // 自动初始化并启动
            Button_init_Click(sender, e);
            Button_start_Click(sender, e);
        }

        private void Button_init_Click(object? sender, EventArgs e)
        {
            if (!m_bOpen)
            {
                MessageBox.Show("设备还没打开", "提示", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }

            uint type = kDeviceType[comboBox_device.SelectedIndex].device_type;
            bool canfdDevice = type == Define.ZCAN_USBCANFD_100U ||
                type == Define.ZCAN_USBCANFD_200U ||
                type == Define.ZCAN_USBCANFD_MINI;

            IntPtr ptr = Method.GetIProperty(device_handle_);
            if (NULL == (int)ptr)
            {
                MessageBox.Show("获取属性接口失败", "提示", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }

            property_ = Marshal.PtrToStructure<IProperty>(ptr);

            // 设置波特率
            if (!SetBaudrateFD('A', kAbitTiming[comboBox_ABIT.SelectedIndex]))
                return;
            if (!SetBaudrateFD('D', kDbitTiming[comboBox_ABIT2.SelectedIndex]))
                return;

            ZCAN_CHANNEL_INIT_CONFIG config_ = new ZCAN_CHANNEL_INIT_CONFIG();
            config_.canfd.mode = (byte)comboBox_mode.SelectedIndex;
            config_.can_type = canfdDevice ? (uint)Define.TYPE_CANFD : (uint)Define.TYPE_CAN;

            IntPtr pConfig = IntPtr.Zero;
            try
            {
                pConfig = Marshal.AllocHGlobal(Marshal.SizeOf(config_));
                Marshal.StructureToPtr(config_, pConfig, true);

                channel_handle_ = Method.ZCAN_InitCAN(device_handle_, 0, pConfig);
                channel_handle2_ = Method.ZCAN_InitCAN(device_handle_, 1, pConfig);
            }
            finally
            {
                if (pConfig != IntPtr.Zero)
                    Marshal.FreeHGlobal(pConfig);
            }

            if (NULL == (int)channel_handle_ || NULL == (int)channel_handle2_)
            {
                MessageBox.Show("初始化CAN失败", "提示", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }

            button_init.Enabled = false;
            button_start.Enabled = true;
        }

        private void Button_start_Click(object? sender, EventArgs e)
        {
            if (Method.ZCAN_StartCAN(channel_handle_) != Define.STATUS_OK ||
                Method.ZCAN_StartCAN(channel_handle2_) != Define.STATUS_OK)
            {
                MessageBox.Show("启动CAN失败", "提示", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }

            button_start.Enabled = false;
            button_reset.Enabled = true;
            m_bStart = true;
            StartDataProcessTimer();

            if (recv_data_thread_ == null)
            {
                recv_data_thread_ = new RecvDataThread();
                recv_data_thread_.RecvCANData += this.AddCANData;
                recv_data_thread_.RecvFDData += this.AddFDData;
                recv_data_thread_.SetChannelHandle(channel_handle_, channel_handle2_);
                recv_data_thread_.SetStart(m_bStart);
            }
            else
            {
                recv_data_thread_.SetChannelHandle(channel_handle_, channel_handle2_);
                recv_data_thread_.SetStart(m_bStart);
            }
        }

        private void Button_reset_Click(object? sender, EventArgs e)
        {
            if (Method.ZCAN_ResetCAN(channel_handle_) != Define.STATUS_OK ||
                Method.ZCAN_ResetCAN(channel_handle2_) != Define.STATUS_OK)
            {
                MessageBox.Show("复位CAN失败", "提示", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }

            button_init.Enabled = true;
            button_start.Enabled = true;
            button_reset.Enabled = false;
            m_bStart = false;
            StopDataProcessTimer();

            if (recv_data_thread_ != null)
            {
                recv_data_thread_.SetStart(false);
            }
        }

        private void Button_close_Click(object? sender, EventArgs e)
        {
            StopDataProcessTimer();

            if (recv_data_thread_ != null)
            {
                recv_data_thread_.SetStart(false);
            }

            Method.ZCAN_CloseDevice(device_handle_);
            m_bOpen = false;
            EnableCtrl(false);
            button_open.Enabled = true;
            button_close.Enabled = false;
            button_init.Enabled = true;
            button_start.Enabled = true;
            button_reset.Enabled = true;
        }

        private void Button_send_Click(object? sender, EventArgs e)
        {
            if (textBox_senddata.Text.Length == 0)
                return;

            uint id = Convert.ToUInt32(textBox_ID.Text, 16);
            string data = textBox_senddata.Text;
            int frame_type_index = comboBox_frametype.SelectedIndex;
            int protocol_index = comboBox_protocol.SelectedIndex;
            int send_type_index = comboBox_sendtype.SelectedIndex;
            int chan = comboBox_channel.SelectedIndex;

            uint result;

            if (protocol_index == 0) // CAN
            {
                IntPtr ptr = IntPtr.Zero;
                try
                {
                    ZCAN_Transmit_Data can_data = new ZCAN_Transmit_Data();
                    can_data.frame.can_id = MakeCanId(id, frame_type_index, 0, 0);
                    can_data.frame.data = new byte[8];
                    can_data.frame.can_dlc = (byte)SplitData(data, ref can_data.frame.data, CAN_MAX_DLEN);
                    can_data.transmit_type = (uint)send_type_index;

                    ptr = Marshal.AllocHGlobal(Marshal.SizeOf(can_data));
                    Marshal.StructureToPtr(can_data, ptr, true);
                    result = Method.ZCAN_Transmit((chan == 0) ? channel_handle_ : channel_handle2_, ptr, 1);
                }
                finally
                {
                    if (ptr != IntPtr.Zero)
                        Marshal.FreeHGlobal(ptr);
                }
            }
            else // CANFD
            {
                IntPtr ptr = IntPtr.Zero;
                try
                {
                    ZCAN_TransmitFD_Data canfd_data = new ZCAN_TransmitFD_Data();
                    canfd_data.frame.can_id = MakeCanId(id, frame_type_index, 0, 0);
                    canfd_data.frame.data = new byte[64];
                    canfd_data.frame.len = (byte)SplitData(data, ref canfd_data.frame.data, CANFD_MAX_DLEN);
                    canfd_data.transmit_type = (uint)send_type_index;
                    canfd_data.frame.flags = 0;

                    ptr = Marshal.AllocHGlobal(Marshal.SizeOf(canfd_data));
                    Marshal.StructureToPtr(canfd_data, ptr, true);
                    result = Method.ZCAN_TransmitFD((chan == 0) ? channel_handle_ : channel_handle2_, ptr, 1);
                }
                finally
                {
                    if (ptr != IntPtr.Zero)
                        Marshal.FreeHGlobal(ptr);
                }
            }

            if (result != 1)
            {
                AddError();
            }
        }

        private void Button_setParams_Click(object? sender, EventArgs e)
        {
            string kpText = textBox_kp.Text.Trim();
            string kiText = textBox_ki.Text.Trim();
            string testText = textBox_test.Text.Trim();
            string testText2 = textBox_test2.Text.Trim();

            bool kpValid = float.TryParse(kpText, out float kpFloat);
            bool kiValid = float.TryParse(kiText, out float kiFloat);
            bool testValid = float.TryParse(testText, out float testFloat);
            bool testValid2 = float.TryParse(testText2, out float testFloat2);

            if (kpValid)
            {
                SendFloatParameter(CMD_KP, kpFloat, KP_KI_SCALE_FACTOR);
                MessageBox.Show($"KP参数设置为: {kpFloat:F3}", "参数设置", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }

            if (kiValid)
            {
                SendFloatParameter(CMD_KI, kiFloat, KP_KI_SCALE_FACTOR);
                MessageBox.Show($"KI参数设置为: {kiFloat:F3}", "参数设置", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }

            if (testValid)
            {
                SendFloatParameter(CMD_TEST, testFloat, KP_KI_SCALE_FACTOR);
                MessageBox.Show($"测试参数1设置为: {testFloat:F3}", "参数设置", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }

            if (testValid2)
            {
                SendFloatParameter(CMD_TEST2, testFloat2, KP_KI_SCALE_FACTOR);
                MessageBox.Show($"测试参数2设置为: {testFloat2:F3}", "参数设置", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }

            if (!kpValid && !kiValid && !testValid && !testValid2)
            {
                MessageBox.Show("请输入至少一个有效的浮点数字", "输入错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void Button_setTemp_Click(object? sender, EventArgs e)
        {
            // 发送过温恢复模式命令
            if (checkBox_tempRecover.Checked)
            {
                SendSimpleCommand(CMD_TEMP_RECOVER_ON);
            }
            else
            {
                SendSimpleCommand(CMD_TEMP_RECOVER_OFF);
            }

            string overTempText = textBox_overTemp.Text.Trim();
            string recoverTempText = textBox_recoverTemp.Text.Trim();

            bool overTempValid = ushort.TryParse(overTempText, out ushort overTempValue);
            bool recoverTempValid = ushort.TryParse(recoverTempText, out ushort recoverTempValue);

            if (overTempValid)
            {
                Send16BitSimple(CMD_OVERTEMP_POINT, overTempValue);
                MessageBox.Show($"过温点设置为: {overTempValue}°C", "温度设置", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }

            if (recoverTempValid)
            {
                Send16BitWithCRC(CMD_OVERTEMP_REC_POINT, recoverTempValue);
                MessageBox.Show($"过温恢复点设置为: {recoverTempValue}°C", "温度设置", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }

            if (!overTempValid && !recoverTempValid)
            {
                MessageBox.Show("请输入至少一个有效的温度值", "输入错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void Button_calibrate_Click(object? sender, EventArgs e)
        {
            timer1.Enabled = false;
            Thread.Sleep(5);
            Button_reset_Click(null, null);
            Thread.Sleep(5);
            Button_init_Click(null, null);
            Thread.Sleep(5);
            Button_start_Click(null, null);
            Thread.Sleep(100);

            string calibrationInput = textBox_calibrationInput.Text;
            string calibrationTarget = textBox_calibrationTarget.Text;

            bool isInputValid = decimal.TryParse(calibrationInput, NumberStyles.Float, CultureInfo.InvariantCulture, out decimal inputValue);
            bool isTargetValid = decimal.TryParse(calibrationTarget, NumberStyles.Float, CultureInfo.InvariantCulture, out decimal targetValue);

            if (!isInputValid || !isTargetValid)
            {
                string errorMessage = "";
                if (!isInputValid && !isTargetValid)
                {
                    errorMessage = "输入和目标都无效。请输入有效的数字格式。";
                }
                else if (!isInputValid)
                {
                    errorMessage = "输入无效。请输入有效的数字格式。";
                }
                else
                {
                    errorMessage = "目标无效。请输入有效的数字格式。";
                }

                MessageBox.Show(errorMessage, "输入错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
                timer1.Enabled = true;
                return;
            }

            try
            {
                // 转换为下位机期望的格式（毫伏）
                ushort factorVoltage = (ushort)(inputValue * 1000);
                ushort theorVoltage = (ushort)(targetValue * 1000);

                // 发送校准电压和理论电压
                Send16BitSimple(CMD_FACTOR_VOLTAGE, factorVoltage);
                Thread.Sleep(10);
                Send16BitSimple(CMD_THEOR_VOLTAGE, theorVoltage);
                Thread.Sleep(10);

                // 计算并发送CRC校验
                byte factorVoltageLow = (byte)(factorVoltage & 0xFF);
                byte factorVoltageHigh = (byte)((factorVoltage >> 8) & 0xFF);
                byte theorVoltageLow = (byte)(theorVoltage & 0xFF);
                byte theorVoltageHigh = (byte)((theorVoltage >> 8) & 0xFF);

                byte[] crcData = new byte[9] {
                    CMD_VOLTAGE_CRC,
                    factorVoltageLow, factorVoltageHigh,
                    0x00, 0x00,
                    theorVoltageLow, theorVoltageHigh,
                    0x00, 0x00
                };

                byte calculatedCrc = CalculateCRC8(crcData, 0, crcData.Length);

                // 发送CRC校验字节
                byte[] crcFrame = new byte[8] { 0x00, CMD_VOLTAGE_CRC, calculatedCrc, 0x00, 0x00, 0x00, 0x00, 0x00 };
                SendCANFrame(crcFrame, 3);

                MessageBox.Show($"校准数据发送完成\n实际电压: {inputValue}V\n目标电压: {targetValue}V\nCRC: 0x{calculatedCrc:X2}",
                               "校准完成", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"校准过程出错: {ex.Message}", "错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            finally
            {
                if (checkBox_autoRefresh.Checked)
                {
                    timer1.Enabled = true;
                }
            }
        }

        private void CheckBox_dataRecord_CheckedChanged(object? sender, EventArgs e)
        {
            if (checkBox_dataRecord.Checked)
            {
                string dateTimeFormat = DateTime.Now.ToString("yyyy-MM-dd_HH_mm_ss");
                string fileName = $"DeviceData_{dateTimeFormat}.xlsx";
                _excelFilePath = Path.Combine(Environment.CurrentDirectory, fileName);
                // 启动定时刷写（每5秒批量写一次）
                _excelFlushTimer = new System.Threading.Timer(_ => FlushExcelQueue(), null, 5000, 5000);
                MessageBox.Show($"Excel记录已启用\n文件路径: {_excelFilePath}", "Excel记录", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            else
            {
                // 停止定时器，最后刷写一次
                _excelFlushTimer?.Dispose();
                _excelFlushTimer = null;
                FlushExcelQueue();
                string? savedPath = _excelFilePath;
                _excelFilePath = null;
                if (!string.IsNullOrEmpty(savedPath) && File.Exists(savedPath))
                {
                    var result = MessageBox.Show(
                        $"Excel记录已停用\n文件：{Path.GetFileName(savedPath)}\n\n是否打开该文件？",
                        "Excel记录", MessageBoxButtons.YesNo, MessageBoxIcon.Information);
                    if (result == DialogResult.Yes)
                    {
                        try { System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo(savedPath) { UseShellExecute = true }); }
                        catch (Exception ex) { MessageBox.Show($"无法打开文件: {ex.Message}", "错误", MessageBoxButtons.OK, MessageBoxIcon.Error); }
                    }
                }
                else
                {
                    MessageBox.Show("Excel记录已停用（无数据写入）", "Excel记录", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
            }
        }

        // 继续添加其他必要的方法...
        private void UartModeApplyButton_Click(object? sender, EventArgs e)
        {
            if (_iapUpgradeInProgress)
            {
                MessageBox.Show("升级中不能切换UART模式。", "提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
                return;
            }

            if (!m_bStart)
            {
                MessageBox.Show("请先启动CAN。", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            byte mode = (byte)(_uartModeComboBox?.SelectedIndex == 0 ? 0x00 : 0x01);
            byte[] dataBytes = new byte[8] { 0x00, CMD_UART_MODE, mode, 0xA5, 0x5A, 0x00, 0x00, 0x00 };
            SendCANFrame(dataBytes, 5, overrideId1: 0x20, overrideId2: 0x20);

            string modeText = mode == 0 ? "调试模式（VOFA）" : "原副边通讯模式";
            if (_uartModeStatusLabel != null)
            {
                _uartModeStatusLabel.Text = $"已发送切换命令：{modeText}";
                _uartModeStatusLabel.ForeColor = mode == 0 ? Color.DarkOrange : Color.DarkGreen;
            }
            AppendRecvLog($"已发送LLC UART模式切换：{modeText}");
        }

        private void SendSimpleCommand(byte command, uint? overrideMasterId = null)
        {
            if (_iapUpgradeInProgress)
            {
                AppendIapLog("升级中，已忽略普通控制命令");
                return;
            }

            if (!m_bStart)
            {
                MessageBox.Show("请先启动CAN", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            // Byte0 填帧计数（与旧版一致，MCU 会原数回传）
            byte frameCnt = 0x00;
            if (command == CMD_QUERY)
                frameCnt = _queryFrameCnt++;
            else if (command == CMD_VERSION)
                frameCnt = _versionFrameCnt++;

            byte[] dataBytes = new byte[8] { frameCnt, command, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            SendCANFrame(dataBytes, 8, overrideId1: overrideMasterId, overrideId2: overrideMasterId);
            // 注意：不在这里更新 CNT 显示，等 MCU 回传后才更新对应通道
        }

        private void SendCANFrame(byte[] dataBytes, int dataLength, uint? overrideId1 = null, uint? overrideId2 = null)
        {
            try
            {
                uint master_id1 = 0, master_id2 = 0;
                bool ch1Enabled = checkBox_ch1Enable.Checked;
                bool ch2Enabled = checkBox_ch2Enable.Checked;

                if (overrideId1.HasValue)
                {
                    // overrideId 优先：无论复选框是否勾选，都使用指定 ID 发送
                    master_id1 = overrideId1.Value;
                }
                else if (ch1Enabled)
                {
                    if (string.IsNullOrEmpty(textBox_masterAddr1.Text))
                    {
                        checkBox_ch1Enable.Checked = false;
                        return;
                    }
                    master_id1 = Convert.ToUInt32(textBox_masterAddr1.Text, 16);
                }
                if (overrideId2.HasValue)
                {
                    master_id2 = overrideId2.Value;
                }
                else if (ch2Enabled)
                {
                    if (string.IsNullOrEmpty(textBox_masterAddr2.Text))
                    {
                        checkBox_ch2Enable.Checked = false;
                        return;
                    }
                    master_id2 = Convert.ToUInt32(textBox_masterAddr2.Text, 16);
                }

                int frame_type_index = 1; // 扩展帧
                int send_type_index = 0; // 正常发送
                uint result;

                // CAN发送
                if (overrideId1.HasValue || ch1Enabled)
                {
                    IntPtr ptr = IntPtr.Zero;
                    try
                    {
                        ZCAN_Transmit_Data can_data = new ZCAN_Transmit_Data();
                        can_data.frame.can_id = MakeCanId(master_id1, frame_type_index, 0, 0);
                        can_data.frame.data = new byte[8];
                        Array.Copy(dataBytes, can_data.frame.data, Math.Min(dataBytes.Length, 8));
                        can_data.frame.can_dlc = (byte)dataLength;
                        can_data.transmit_type = (uint)send_type_index;

                        ptr = Marshal.AllocHGlobal(Marshal.SizeOf(can_data));
                        Marshal.StructureToPtr(can_data, ptr, true);
                        result = Method.ZCAN_Transmit(channel_handle_, ptr, 1);
                    }
                    finally
                    {
                        if (ptr != IntPtr.Zero)
                            Marshal.FreeHGlobal(ptr);
                    }
                }

                if (overrideId2.HasValue || ch2Enabled)
                {
                    IntPtr ptr = IntPtr.Zero;
                    try
                    {
                        ZCAN_Transmit_Data can_data = new ZCAN_Transmit_Data();
                        can_data.frame.can_id = MakeCanId(master_id2, frame_type_index, 0, 0);
                        can_data.frame.data = new byte[8];
                        Array.Copy(dataBytes, can_data.frame.data, Math.Min(dataBytes.Length, 8));
                        can_data.frame.can_dlc = (byte)dataLength;
                        can_data.transmit_type = (uint)send_type_index;

                        ptr = Marshal.AllocHGlobal(Marshal.SizeOf(can_data));
                        Marshal.StructureToPtr(can_data, ptr, true);
                        result = Method.ZCAN_Transmit(channel_handle2_, ptr, 1);
                    }
                    finally
                    {
                        if (ptr != IntPtr.Zero)
                            Marshal.FreeHGlobal(ptr);
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"发送CAN帧异常: {ex.Message}", "错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void SendCANData(byte command, uint value)
        {
            try
            {
                uint master_id1 = 0, master_id2 = 0;
                bool ch1Enabled = checkBox_ch1Enable.Checked;
                bool ch2Enabled = checkBox_ch2Enable.Checked;

                if (ch1Enabled)
                {
                    master_id1 = Convert.ToUInt32(textBox_masterAddr1.Text, 16);
                }
                if (ch2Enabled)
                {
                    master_id2 = Convert.ToUInt32(textBox_masterAddr2.Text, 16);
                }

                // 将值拆分为四个字节
                byte byte1 = (byte)(value & 0xFF);
                byte byte2 = (byte)((value >> 8) & 0xFF);
                byte byte3 = (byte)((value >> 16) & 0xFF);
                byte byte4 = (byte)((value >> 24) & 0xFF);

                // 构建最终的字节数组
                byte[] dataBytes = new byte[8];
                dataBytes[0] = 0x00;
                dataBytes[1] = command;
                dataBytes[2] = byte1;
                dataBytes[3] = byte2;
                dataBytes[4] = byte3;
                dataBytes[5] = byte4;
                dataBytes[6] = 0x00;
                dataBytes[7] = 0x00;

                SendCANFrame(dataBytes, 8);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"发送CAN数据异常: {ex.Message}", "错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        // 16位数据命令发送（不带CRC）
        private void Send16BitSimple(byte command, ushort value)
        {
            byte[] dataBytes = new byte[8]
            {
                0x00,                         // [0] 保留位
                command,                      // [1] 命令
                (byte)(value & 0xFF),         // [2] 低字节
                (byte)((value >> 8) & 0xFF),  // [3] 高字节
                0x00, 0x00, 0x00, 0x00       // [4-7] 保留位
            };
            SendCANFrame(dataBytes, 4); // 4字节有效数据
        }

        // 16位数据命令发送（带CRC）
        private void Send16BitWithCRC(byte command, ushort value)
        {
            byte lowByte = (byte)(value & 0xFF);
            byte highByte = (byte)((value >> 8) & 0xFF);

            // 计算CRC（命令+2字节数据）
            byte[] crcData = new byte[3] { command, lowByte, highByte };
            byte crc = CalculateCRC8(crcData, 0, crcData.Length);

            byte[] dataBytes = new byte[8]
            {
                0x00,        // [0] 保留位
                command,     // [1] 命令
                lowByte,     // [2] 低字节
                highByte,    // [3] 高字节
                crc,         // [4] CRC
                0x00, 0x00, 0x00 // [5-7] 保留位
            };
            SendCANFrame(dataBytes, 5); // 5字节有效数据
        }

        // 浮点数参数发送（转换为32位+CRC）
        private void SendFloatParameter(byte command, float value, int scaleFactor = KP_KI_SCALE_FACTOR)
        {
            uint scaledValue = (uint)(value * scaleFactor);
            Send32BitWithCRC(command, scaledValue);
        }

        // 32位数据+CRC命令发送
        private void Send32BitWithCRC(byte command, uint value)
        {
            byte byte0 = (byte)(value & 0xFF);
            byte byte1 = (byte)((value >> 8) & 0xFF);
            byte byte2 = (byte)((value >> 16) & 0xFF);
            byte byte3 = (byte)((value >> 24) & 0xFF);

            // 计算CRC（命令+4字节数据）
            byte[] crcData = new byte[5] { command, byte0, byte1, byte2, byte3 };
            byte crc = CalculateCRC8(crcData, 0, crcData.Length);

            byte[] dataBytes = new byte[8]
            {
                0x00,    // [0] 保留位
                command, // [1] 命令
                byte0,   // [2] 最低字节
                byte1,   // [3]
                byte2,   // [4]
                byte3,   // [5] 最高字节
                crc,     // [6] CRC
                0x00     // [7] 保留位
            };
            SendCANFrame(dataBytes, 7); // 7字节有效数据
        }

        private byte CalculateCRC8(byte[] data, int startIndex, int length)
        {
            if (data == null)
                throw new ArgumentNullException(nameof(data));

            if (startIndex < 0 || length < 0 || startIndex + length > data.Length)
                throw new ArgumentOutOfRangeException("索引或长度超出数组范围");

            const byte polynomial = 0x07; // CRC8-CCITT 多项式
            byte crc = 0;

            for (int i = startIndex; i < startIndex + length; i++)
            {
                crc ^= data[i];
                for (int j = 0; j < 8; j++)
                {
                    if ((crc & 0x80) != 0)
                    {
                        crc = (byte)((crc << 1) ^ polynomial);
                    }
                    else
                    {
                        crc <<= 1;
                    }
                }
            }

            return crc;
        }

        private bool SetBaudrateFD(char ad, uint baud)
        {
            string value = baud.ToString();

            if (ad == 'A')
            {
                if (1 != property_.SetValue("0/canfd_abit_baud_rate", value))
                    return false;
                return 1 == property_.SetValue("1/canfd_abit_baud_rate", value);
            }
            else if (ad == 'D')
            {
                if (1 != property_.SetValue("0/canfd_dbit_baud_rate", value))
                    return false;
                return 1 == property_.SetValue("1/canfd_dbit_baud_rate", value);
            }
            return false;
        }

        private int SplitData(string data, ref byte[] transData, int maxLen)
        {
            if (string.IsNullOrEmpty(data) || transData == null)
                return 0;

            int retLen = 0;
            string[] dataArray = data.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);

            for (int i = 0; i < Math.Min(maxLen, dataArray.Length) && i < transData.Length; i++)
            {
                string hexString = dataArray[i].Trim();

                if (string.IsNullOrWhiteSpace(hexString) || hexString.Length < 2)
                    continue;

                try
                {
                    transData[retLen] = Convert.ToByte(hexString.Substring(0, 2), 16);
                    retLen++;
                }
                catch
                {
                    // 忽略转换错误
                }
            }

            return retLen;
        }

        private void EnableCtrl(bool opened)
        {
            comboBox_device.Enabled = !opened;
            comboBox_index.Enabled = !opened;
            button_open.Enabled = !opened;
        }

        private void ComboBox_device_SelectedIndexChanged(object? sender, EventArgs e)
        {
            comboBox_channel.Items.Clear();
            int channelCount = (int)kDeviceType[comboBox_device.SelectedIndex].channel_count;
            for (int i = 0; i < channelCount; i++)
            {
                comboBox_channel.Items.Add(i);
            }
            comboBox_channel.SelectedIndex = 0;
        }

        private void CheckBox_autoRefresh_CheckedChanged(object? sender, EventArgs e)
        {
            if (_iapUpgradeInProgress)
            {
                timer1.Enabled = false;
                return;
            }

            timer1.Enabled = checkBox_autoRefresh.Checked;
            if (checkBox_autoRefresh.Checked)
            {
                UpdateTimerInterval();
            }
        }

        private void TextBox_readFrequency_TextChanged(object? sender, EventArgs e)
        {
            UpdateTimerInterval();
        }

        private void UpdateTimerInterval()
        {
            if (int.TryParse(textBox_readFrequency.Text, out int interval))
            {
                // 限制频率范围：最小50ms，最大10000ms（10秒）
                interval = Math.Max(50, Math.Min(10000, interval));

                if (timer1.Interval != interval)
                {
                    timer1.Interval = interval;

                    // 如果输入的值被限制了，更新文本框显示
                    if (textBox_readFrequency.Text != interval.ToString())
                    {
                        textBox_readFrequency.Text = interval.ToString();
                    }
                }
            }
            else
            {
                // 如果输入无效，恢复为默认值1000ms
                textBox_readFrequency.Text = "1000";
                timer1.Interval = 1000;
            }
        }

        private void ValidateAndEnableCheckbox(TextBox textBox, CheckBox checkBox)
        {
            if (textBox == null || checkBox == null) return;

            if (string.IsNullOrWhiteSpace(textBox.Text))
            {
                checkBox.Enabled = false;
                return;
            }

            if (uint.TryParse(textBox.Text, System.Globalization.NumberStyles.HexNumber, null, out uint address))
            {
                if (address <= 0x1FFFFFFF)
                {
                    checkBox.Enabled = true;
                }
                else
                {
                    checkBox.Enabled = false;
                    MessageBox.Show("地址超出扩展帧的有效范围。请输入一个小于等于 0x1FFFFFFF 的地址。",
                        "地址错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            else
            {
                checkBox.Enabled = false;
                MessageBox.Show("请输入有效的十六进制地址。", "地址错误", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void Timer1_Tick(object? sender, EventArgs e)
        {
            if (!Monitor.TryEnter(timerLock))
            {
                return;
            }

            try
            {
                if (isTimerProcessing)
                    return;

                isTimerProcessing = true;

                // 计算总线负载
                CalculateBusLoad();

                if (_iapUpgradeInProgress)
                {
                    CalculateCurrentSharing();
                    CheckCommTimeout();
                    return;
                }

                // 发送查询命令（自动刷新固定使用 0x20 作为主机地址）
                if (m_bStart)
                {
                    SendSimpleCommand(CMD_QUERY, overrideMasterId: 0x20);
                }

                // 计算均流度
                CalculateCurrentSharing();

                // 检查通讯超时（超时变红）
                CheckCommTimeout();
            }
            catch (Exception ex)
            {
                Console.WriteLine($"定时器处理异常: {ex.Message}");
            }
            finally
            {
                isTimerProcessing = false;
                Monitor.Exit(timerLock);
            }
        }

        private void StopDataProcessTimer()
        {
            _dataProcessTimer?.Dispose();
            _dataProcessTimer = null;
            Interlocked.Exchange(ref _dataProcessPending, 0);
            while (_dataQueue.TryDequeue(out _)) { }
            Interlocked.Exchange(ref _dataQueueLength, 0);
        }

        private void StartDataProcessTimer()
        {
            StopDataProcessTimer();
            _dataProcessTimer = new System.Threading.Timer(_ =>
            {
                if (Interlocked.Exchange(ref _dataProcessPending, 1) != 0)
                    return;

                try
                {
                    int queueSize = Volatile.Read(ref _dataQueueLength);
                    if (queueSize > DATA_QUEUE_WARN_LIMIT)
                    {
                        Console.WriteLine($"警告：数据队列堆积 {queueSize} 条");
                    }

                    var latestFrames = new Dictionary<(uint actualId, byte command), (uint id, byte[] data, uint displayChannel)>();
                    int processedCount = 0;
                    DateTime now = DateTime.Now;

                    while (_dataQueue.TryDequeue(out var item) && processedCount < DATA_PROCESS_BATCH_LIMIT)
                    {
                        Interlocked.Decrement(ref _dataQueueLength);
                        processedCount++;

                        uint actualId = GetId(item.id);
                        uint displayChannel = RegisterId(actualId);
                        if (displayChannel == 0 || displayChannel > CHANNEL_COUNT || item.data.Length < 2)
                            continue;

                        lock (_idMapLock)
                        {
                            deviceLastResponseTime[actualId] = now;
                        }

                        UpdateRealtimeModel(item.id, item.data, displayChannel, actualId);
                        latestFrames[(actualId, item.data[1])] = (item.id, item.data, displayChannel);
                    }

                    if (latestFrames.Count == 0)
                    {
                        Interlocked.Exchange(ref _dataProcessPending, 0);
                        return;
                    }

                    long nowMs = Environment.TickCount64;
                    long lastUiRefreshMs = Interlocked.Read(ref _lastUiDataRefreshMs);
                    if (nowMs - lastUiRefreshMs < UI_DATA_REFRESH_INTERVAL_MS ||
                        Interlocked.CompareExchange(ref _lastUiDataRefreshMs, nowMs, lastUiRefreshMs) != lastUiRefreshMs)
                    {
                        Interlocked.Exchange(ref _dataProcessPending, 0);
                        return;
                    }

                    if (IsDisposed || !IsHandleCreated)
                    {
                        Interlocked.Exchange(ref _dataProcessPending, 0);
                        return;
                    }

                    BeginInvoke(new Action(() =>
                    {
                        try
                        {
                            foreach (var (id, data, displayChannel) in latestFrames.Values)
                            {
                                uint actualId = GetId(id);
                                UpdateDisplay($"id_ch{displayChannel}", $"0x{actualId:X5}");
                                ParseAndDisplayData(data, displayChannel, actualId, false);
                            }
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine($"数据解析异常: {ex.Message}");
                        }
                        finally
                        {
                            Interlocked.Exchange(ref _dataProcessPending, 0);
                        }
                    }));
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"数据处理异常: {ex.Message}");
                    Interlocked.Exchange(ref _dataProcessPending, 0);
                }
            }, null, DATA_PROCESS_INTERVAL_MS, DATA_PROCESS_INTERVAL_MS);
        }

        private void CalculateBusLoad()
        {
            lock (_busLoadLock)
            {
                DateTime now = DateTime.Now;
                double elapsedSeconds = (now - _lastBusLoadCalculationTime).TotalSeconds;

                if (elapsedSeconds < 1.0)
                    return;

                try
                {
                    uint currentBaudRate = kAbitTiming[comboBox_ABIT.SelectedIndex];
                    uint currentCANFDBaudRate = kDbitTiming[comboBox_ABIT2.SelectedIndex];

                    // 计算通道1负载率
                    double transmissionTimeCh1_CAN = (_frameCountCh1_CAN * AVERAGE_CAN_FRAME_BITS) / (double)currentBaudRate;
                    double transmissionTimeCh1_CANFD = (_frameCountCh1_CANFD * AVERAGE_CANFD_FRAME_BITS) / (double)currentCANFDBaudRate;
                    _currentBusLoadCh1 = ((transmissionTimeCh1_CAN + transmissionTimeCh1_CANFD) / elapsedSeconds) * 100.0;

                    // 计算通道2负载率
                    double transmissionTimeCh2_CAN = (_frameCountCh2_CAN * AVERAGE_CAN_FRAME_BITS) / (double)currentBaudRate;
                    double transmissionTimeCh2_CANFD = (_frameCountCh2_CANFD * AVERAGE_CANFD_FRAME_BITS) / (double)currentCANFDBaudRate;
                    _currentBusLoadCh2 = ((transmissionTimeCh2_CAN + transmissionTimeCh2_CANFD) / elapsedSeconds) * 100.0;

                    // 计算总负载率
                    _totalBusLoad = _currentBusLoadCh1 + _currentBusLoadCh2;
                    _totalBusLoad = Math.Min(100.0, Math.Max(0.0, _totalBusLoad));

                    // 更新显示
                    UpdateBusLoadDisplay();

                    // 重置计数器
                    _frameCountCh1_CAN = 0;
                    _frameCountCh1_CANFD = 0;
                    _frameCountCh2_CAN = 0;
                    _frameCountCh2_CANFD = 0;
                    _totalFrameCount = 0;
                    _lastBusLoadCalculationTime = now;
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"计算总线负载异常: {ex.Message}");
                }
            }
        }

        private void UpdateBusLoadDisplay()
        {
            try
            {
                if (progressBars.TryGetValue("total_load", out var progressBar))
                {
                    this.BeginInvoke(new Action(() =>
                    {
                        progressBar.Value = Math.Min(100, Math.Max(0, (int)_totalBusLoad));
                    }));
                }

                if (displayLabels.TryGetValue("load_text", out var label))
                {
                    this.BeginInvoke(new Action(() =>
                    {
                        label.Text = $"{_totalBusLoad:F1}%";
                        label.ForeColor = _totalBusLoad > 80 ? Color.Red : Color.Black;
                    }));
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"更新负载显示异常: {ex.Message}");
            }
        }

        private void CalculateCurrentSharing()
        {
            lock (_currentSharingLock)
            {
                try
                {
                    var validCurrents = _activeModules.Select(id => _moduleCurrents[id]).Where(current => current > 0.1).ToList();

                    if (validCurrents.Count < 1)
                    {
                        _totalCurrentSharing = 100.0;
                        UpdateCurrentSharingDisplay();
                        return;
                    }

                    if (validCurrents.Count == 1)
                    {
                        _totalCurrentSharing = 100.0;
                    }
                    else
                    {
                        double maxCurrent = validCurrents.Max();
                        double minCurrent = validCurrents.Min();
                        double avgCurrent = validCurrents.Average();

                        if (avgCurrent < 0.01)
                        {
                            _totalCurrentSharing = 100.0;
                        }
                        else
                        {
                            double currentDifference = maxCurrent - minCurrent;
                            _totalCurrentSharing = (1.0 - currentDifference / avgCurrent) * 100.0;
                            _totalCurrentSharing = Math.Max(0.0, Math.Min(100.0, _totalCurrentSharing));
                        }

                        // 计算每个模块的均流度百分比
                        foreach (var id in _activeModules)
                        {
                            if (_moduleCurrents[id] > 0.1)
                            {
                                _moduleSharingPercent[id] = ((_moduleCurrents[id] - avgCurrent) / avgCurrent) * 100.0;
                            }
                            else
                            {
                                _moduleSharingPercent[id] = 0.0;
                            }
                        }
                    }

                    UpdateCurrentSharingDisplay();
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"计算均流度异常: {ex.Message}");
                }
            }
        }

        private void UpdateCurrentSharingDisplay()
        {
            try
            {
                this.BeginInvoke(new Action(() =>
                {
                    // 更新总均流度标签
                    if (displayLabels.TryGetValue("total_share_label", out var totalLabel))
                    {
                        totalLabel.Text = $"{_totalCurrentSharing:F1}%";
                        totalLabel.ForeColor = _totalCurrentSharing >= 90 ? Color.Green
                            : _totalCurrentSharing >= 75 ? Color.Orange : Color.Red;
                    }
                    if (progressBars.TryGetValue("total_share_bar", out var totalBar))
                    {
                        totalBar.Value = Math.Min(100, Math.Max(0, (int)_totalCurrentSharing));
                    }

                    for (int i = 1; i <= CHANNEL_COUNT; i++)
                    {
                        // 更新通道面板内的均流标签（share_ch{i}_label）
                        string keyInline = $"share_ch{i}_label";
                        // 更新右侧汇总面板的电流值和偏差百分比
                        string keyCurrent = $"share_current_ch{i}";
                        string keyPercent = $"share_percent_ch{i}";
                        string keyBar = $"share_bar_ch{i}";

                        if (i <= _activeModules.Count)
                        {
                            uint moduleId = _activeModules[i - 1];
                            double current = _moduleCurrents[moduleId];
                            double sharingPercent = _moduleSharingPercent[moduleId];
                            string sign = sharingPercent >= 0 ? "+" : "";

                            // 通道面板内联显示
                            if (displayLabels.TryGetValue(keyInline, out var inlineLabel))
                            {
                                inlineLabel.Text = $"{current:F1}A ({sign}{sharingPercent:F1}%)";
                                double absDeviation = Math.Abs(sharingPercent);
                                inlineLabel.ForeColor = absDeviation <= 5.0 ? Color.Green
                                    : absDeviation <= 10.0 ? Color.DarkGreen
                                    : absDeviation <= 15.0 ? Color.Orange : Color.Red;
                            }

                            // 右侧汇总面板
                            if (displayLabels.TryGetValue(keyCurrent, out var curLabel))
                            {
                                curLabel.Text = $"{current:F1}A";
                                curLabel.ForeColor = Color.Black;
                            }
                            if (displayLabels.TryGetValue(keyPercent, out var pctLabel))
                            {
                                pctLabel.Text = $"{sign}{sharingPercent:F1}%";
                                double absDeviation = Math.Abs(sharingPercent);
                                pctLabel.ForeColor = absDeviation <= 5.0 ? Color.Green
                                    : absDeviation <= 10.0 ? Color.DarkGreen
                                    : absDeviation <= 15.0 ? Color.Orange : Color.Red;
                            }
                            if (progressBars.TryGetValue(keyBar, out var shareBar))
                            {
                                // 映射 -100%~+100% 到 0~200
                                int barVal = Math.Min(200, Math.Max(0, (int)(sharingPercent + 100)));
                                shareBar.Value = barVal;
                            }
                            // 更新电流进度条（假设最大电流200A）
                            if (progressBars.TryGetValue($"current_bar_ch{i}", out var curBar))
                            {
                                curBar.Value = Math.Min(100, Math.Max(0, (int)(current / 2.0)));
                            }
                        }
                        else
                        {
                            if (displayLabels.TryGetValue(keyInline, out var inlineLabel))
                            {
                                inlineLabel.Text = "---";
                                inlineLabel.ForeColor = Color.Gray;
                            }
                            if (displayLabels.TryGetValue(keyCurrent, out var curLabel))
                            {
                                curLabel.Text = "0.0A";
                                curLabel.ForeColor = Color.Gray;
                            }
                            if (displayLabels.TryGetValue(keyPercent, out var pctLabel))
                            {
                                pctLabel.Text = "---";
                                pctLabel.ForeColor = Color.Gray;
                            }
                            if (progressBars.TryGetValue(keyBar, out var shareBar))
                                shareBar.Value = 100;
                            if (progressBars.TryGetValue($"current_bar_ch{i}", out var curBar))
                                curBar.Value = 0;
                        }
                    }
                }));
            }
            catch (Exception ex)
            {
                Console.WriteLine($"更新均流度显示异常: {ex.Message}");
            }
        }

        // 数据接收和处理方法
        private void AddCANData(ZCAN_Receive_Data[] data, uint len, uint ch)
        {
            if (len > 0)
                _lastCommTime[Math.Min(CHANNEL_COUNT, (int)ch + 1)] = DateTime.Now;

            lock (_busLoadLock)
            {
                if (ch == 0)
                    _frameCountCh1_CAN += (int)len;
                else if (ch == 1)
                    _frameCountCh2_CAN += (int)len;
                _totalFrameCount += (int)len;
            }

            for (uint i = 0; i < len; ++i)
            {
                ZCAN_Receive_Data can = data[i];
                uint id = data[i].frame.can_id;
                if (id == 0x0) continue;

                byte[] frameDataCopy = new byte[8];
                Array.Copy(can.frame.data, frameDataCopy, Math.Min(8, can.frame.data.Length));
                _iapUpgradeSession?.HandleCanFrame(GetId(id), frameDataCopy, can.frame.can_dlc);
                EnqueueReceivedData(id, frameDataCopy, ch);
            }
        }

        private void AddFDData(ZCAN_ReceiveFD_Data[] data, uint len, uint ch)
        {
            if (len > 0)
                _lastCommTime[Math.Min(CHANNEL_COUNT, (int)ch + 1)] = DateTime.Now;

            lock (_busLoadLock)
            {
                if (ch == 0)
                    _frameCountCh1_CANFD += (int)len;
                else if (ch == 1)
                    _frameCountCh2_CANFD += (int)len;
                _totalFrameCount += (int)len;
            }

            for (uint i = 0; i < len; ++i)
            {
                ZCAN_ReceiveFD_Data canfd = data[i];
                uint id = data[i].frame.can_id;

                if (id == 0x0) continue;

                if (canfd.frame.len >= 8)
                {
                    byte[] frameDataCopy = new byte[8];
                    Array.Copy(canfd.frame.data, frameDataCopy, 8);
                    _iapUpgradeSession?.HandleCanFrame(GetId(id), frameDataCopy, canfd.frame.len);
                    EnqueueReceivedData(id, frameDataCopy, ch);
                }
            }
        }

        private void EnqueueReceivedData(uint id, byte[] data, uint ch)
        {
            int queueLength = Interlocked.Increment(ref _dataQueueLength);
            _dataQueue.Enqueue((id, data, ch));

            if (queueLength <= DATA_QUEUE_DROP_LIMIT)
                return;

            int dropCount = Math.Min(queueLength - DATA_PROCESS_BATCH_LIMIT, DATA_PROCESS_BATCH_LIMIT);
            for (int i = 0; i < dropCount && _dataQueue.TryDequeue(out _); i++)
                Interlocked.Decrement(ref _dataQueueLength);
        }

        private uint RegisterId(uint id)
        {
            uint actualId = GetId(id);
            lock (_idMapLock)
            {
                if (idToChannelMap.TryGetValue(actualId, out uint existingChannel))
                    return existingChannel;

                if (registeredIds.Count >= CHANNEL_COUNT)
                    return 0;

                uint channel = (uint)registeredIds.Count + 1;
                registeredIds.Add(actualId);
                idToChannelMap[actualId] = channel;
                channelToPureId[channel] = actualId;
                return channel;
            }
        }

        private void UpdateRealtimeModel(uint id, byte[] frameData, uint ch, uint actualId)
        {
            if (frameData.Length < 8 || frameData[1] != 0x81)
                return;

            ushort voltage = (ushort)(frameData[3] | (frameData[4] << 8));
            ushort current = (ushort)(frameData[5] | (frameData[6] << 8));
            sbyte temperature = (sbyte)frameData[7];
            byte powerStatusByte = frameData[2];
            double currentValue = current / 10.0 + current_offset;

            if (actualId >= TARGET_ID_START && actualId <= TARGET_ID_END)
                UpdateChannelCurrent(actualId, currentValue);

            if (checkBox_dataRecord.Checked && _excelFilePath != null)
                LogDataToExcel(actualId, voltage / 1000.0, currentValue, temperature, ch, powerStatusByte);
        }

        private void ParseAndDisplayData(byte[] frameData, uint ch, uint id, bool updateRealtimeModel = true)
        {
            if (frameData.Length < 8) return;

            byte cmd_receive = frameData[1];
            uint actualId = GetId(id);

            if ((actualId & 0xFF0000) == 0xA0000 || (actualId & 0xFF0000) == 0x10000)
            {
                switch (cmd_receive)
                {
                    case 0x81:  // RETURN_BIT_POWER
                        Command_Read_Data(id, frameData, ch, updateRealtimeModel);
                        break;
                    case 0x82:  // VERSION_CMD
                        Command_Read_Version(id, frameData, ch);
                        break;
                    case 0x83:  // RETURN_BIT_VOLTAGE_PROTECT
                        ParseVoltageProtectData(id, frameData);
                        break;
                    case 0x84:  // RETURN_BIT_OCP_PROTECT
                        ParseOCPProtectData(id, frameData);
                        break;
                    case 0x85:  // RETURN_BIT_OSP_PROTECT
                        ParseOSPProtectData(id, frameData);
                        break;
                    case 0x86:  // RETURN_BIT_VOLTAGE_PARA
                        ParseLLCOutParaData(id, frameData);
                        break;
                    case 0xBE:  // 温度保护点
                        ParseTempProtectData(id, frameData);
                        break;
                    case CMD_UART_MODE:
                        ParseUartModeAck(frameData);
                        break;

                    // ── PFC (原边) 保护/数据帧 ──
                    case 0x87:  // RETURN_BIT_PFC_INPUT_OVP
                        ParsePfcInputOvpData(id, frameData);
                        break;
                    case 0x88:  // RETURN_BIT_PFC_INPUT_UVP
                        ParsePfcInputUvpData(id, frameData);
                        break;
                    case 0x89:  // RETURN_BIT_PFC_OUTPUT_OVP
                        ParsePfcOutputOvpData(id, frameData);
                        break;
                    case 0x8A:  // RETURN_BIT_PFC_OUTPUT_UVP
                        ParsePfcOutputUvpData(id, frameData);
                        break;
                    case 0x8B:  // RETURN_BIT_PFC_INPUT_OCP
                        ParsePfcInputOcpData(id, frameData);
                        break;
                    case 0x8C:  // RETURN_BIT_PFC_DATA (vbus)
                        ParsePfcVbusData(id, frameData);
                        break;
                    case 0x8D:  // RETURN_BIT_PFC_LIVE1
                        ParsePfcLive1Data(id, frameData);
                        break;
                    case 0x8E:  // RETURN_BIT_PFC_LIVE2
                        ParsePfcLive2Data(id, frameData);
                        break;
                }
            }
        }

        private void ParseUartModeAck(byte[] frameData)
        {
            if (frameData.Length < 3) return;

            byte mode = frameData[2];
            string modeText = mode == 0 ? "调试模式（VOFA）" :
                              mode == 1 ? "原副边通讯模式" :
                              mode == 2 ? "IAP预留模式" :
                              $"未知模式 0x{mode:X2}";

            BeginInvoke(new Action(() =>
            {
                if (_uartModeComboBox != null && mode <= 1)
                    _uartModeComboBox.SelectedIndex = mode;

                if (_uartModeStatusLabel != null)
                {
                    _uartModeStatusLabel.Text = $"LLC已确认：{modeText}";
                    _uartModeStatusLabel.ForeColor = mode == 0 ? Color.DarkOrange : Color.DarkGreen;
                }
            }));

            AppendRecvLog($"LLC UART模式确认：{modeText}");
        }

        private void Command_Read_Data(uint id, byte[] frameData, uint ch, bool updateRealtimeModel = true)
        {
            if (frameData == null || frameData.Length < 8) return;

            // Byte0：MCU 回传的数据帧计数，只在 0x81 帧里更新 CNT 标签
            byte dataCnt = frameData[0];
            if (ch >= 1 && ch <= CHANNEL_COUNT)
                UpdateDisplay($"tx_cnt_ch{ch}", $"0x{dataCnt:X2}");

            byte powerStatusByte = frameData[2];
            ushort voltage = (ushort)(frameData[3] | (frameData[4] << 8));
            ushort current = (ushort)(frameData[5] | (frameData[6] << 8));
            sbyte temperature = (sbyte)frameData[7];

            // 解析电源状态
            bool commStatus = (powerStatusByte & 0x20) != 0; // bit5：通讯正常位
            bool powerState = (powerStatusByte & 0x01) != 0;
            bool overVoltage = (powerStatusByte & 0x02) != 0;
            bool underVoltage = (powerStatusByte & 0x04) != 0;
            bool overCurrent = (powerStatusByte & 0x08) != 0;
            bool overTemp = (powerStatusByte & 0x10) != 0;

            string voltageStr = (voltage / 1000.0 + voltage_offset).ToString("F2");
            string currentStr = (current / 10.0 + current_offset).ToString("F1");
            double powerValue = voltage * current / 10000.0;
            string powerStr = powerValue.ToString("F1");

            // 纯 CAN ID
            uint actualId = GetId(id);

            // 更新测量数据
            UpdateDisplay($"voltage_ch{ch}", voltageStr + " V");
            UpdateDisplay($"current_ch{ch}", currentStr + " A");
            UpdateDisplay($"power_ch{ch}", powerStr + " W");
            UpdateDisplay($"temp_ch{ch}", temperature.ToString() + " °C");
            UpdateDisplay($"id_ch{ch}", $"0x{actualId:X5}");

            // 更新状态指示灯
            UpdateStatusIndicator($"led_overvolt_ch{ch}", overVoltage);
            UpdateStatusIndicator($"led_undervolt_ch{ch}", underVoltage);
            UpdateStatusIndicator($"led_overcurr_ch{ch}", overCurrent);
            UpdateStatusIndicator($"led_overtemp_ch{ch}", overTemp);

            // 通讯正常指示灯：仅在 bit5=1 时触发绿色闪烁
            if (commStatus)
            {
                UpdateCommLed(ch, true);
            }

            // 电源开关指示灯：文字变为"开启"/"关闭"，颜色：绿色=开启，红色=关闭，灰色=未连接
            string powerLedKey = $"led_pwr_ch{ch}";
            if (displayLabels.TryGetValue(powerLedKey, out var powerLed))
            {
                string ledText = powerState ? "开启" : "关闭";
                Color ledColor = powerState ? Color.Green : Color.Red;
                if (powerLed.InvokeRequired)
                {
                    powerLed.BeginInvoke(new Action(() =>
                    {
                        powerLed.Text = ledText;
                        powerLed.BackColor = ledColor;
                        powerLed.ForeColor = Color.White;
                        powerLed.Visible = true;
                        powerLed.BringToFront();
                    }));
                }
                else
                {
                    powerLed.Text = ledText;
                    powerLed.BackColor = ledColor;
                    powerLed.ForeColor = Color.White;
                    powerLed.Visible = true;
                    powerLed.BringToFront();
                }
            }

            // 更新均流度计算
            if (updateRealtimeModel && actualId >= TARGET_ID_START && actualId <= TARGET_ID_END)
            {
                UpdateChannelCurrent(actualId, current / 10.0 + current_offset);
            }

            // Excel记录
            if (updateRealtimeModel && checkBox_dataRecord.Checked && _excelFilePath != null)
            {
                LogDataToExcel(id, voltage / 1000.0, current / 10.0, temperature, ch, powerStatusByte);
            }
        }

        private void Command_Read_Version(uint id, byte[] frameData, uint ch)
        {
            if (frameData == null || frameData.Length < 8) return;

            // Byte0：版本帧的帧计数
            byte versionCnt = frameData[0];
            byte year = frameData[3];
            byte month = frameData[4];
            byte day = frameData[5];
            ushort versionLow = (ushort)(frameData[6] | (frameData[7] << 8));

            string versionStr = $"V{versionLow / 100}.{(versionLow / 10) % 10}.{versionLow % 10}";
            string dateStr = $"{year:D2}-{month:D2}-{day:D2}";

            // 版本列：版本号 + 日期 + 版本帧计数
            UpdateDisplay($"version_ch{ch}", $"{versionStr}  {dateStr}  CNT:0x{versionCnt:X2}");

            // ID 已在 AddCANData 更新，这里补充一次确保准确
            uint actualId = GetId(id);
            UpdateDisplay($"id_ch{ch}", $"0x{actualId:X5}");
        }

        // 发送保护点查询命令到所有已注册设备
        private void SendProtectQueryCommands()
        {
            if (!m_bStart) return;
            // 保护点查询命令通过主机地址广播发送（与普通查询命令相同方式）
            // LLC 保护点
            byte[] dataBytes3E = new byte[8] { 0x00, CMD_LLC_TEMP_PROTECT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            byte[] dataBytes3F = new byte[8] { 0x00, CMD_LLC_VOLTAGE_PROTECT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            byte[] dataBytes40 = new byte[8] { 0x00, CMD_LLC_OCP_PROTECT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            byte[] dataBytes41 = new byte[8] { 0x00, CMD_LLC_OSP_PROTECT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            byte[] dataBytes44 = new byte[8] { 0x00, CMD_LLC_OUT_PARA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            SendCANFrame(dataBytes3E, 2);
            SendCANFrame(dataBytes3F, 2);
            SendCANFrame(dataBytes40, 2);
            SendCANFrame(dataBytes41, 2);
            SendCANFrame(dataBytes44, 2);

            // PFC 保护点查询（对应 MCU variables_define_app.h 中 CommandType 0x30~0x35）
            byte[] dataBytes30 = new byte[8] { 0x00, CMD_PFC_INPUT_OVP,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            byte[] dataBytes31 = new byte[8] { 0x00, CMD_PFC_INPUT_UVP,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            byte[] dataBytes32 = new byte[8] { 0x00, CMD_PFC_OUTPUT_OVP, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            byte[] dataBytes33 = new byte[8] { 0x00, CMD_PFC_OUTPUT_UVP, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            byte[] dataBytes34 = new byte[8] { 0x00, CMD_PFC_INPUT_OCP,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            byte[] dataBytes35 = new byte[8] { 0x00, CMD_PFC_DATA,       0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            byte[] dataBytes36 = new byte[8] { 0x00, CMD_PFC_DATA_LIVE1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            byte[] dataBytes37 = new byte[8] { 0x00, CMD_PFC_DATA_LIVE2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            SendCANFrame(dataBytes30, 2);
            SendCANFrame(dataBytes31, 2);
            SendCANFrame(dataBytes32, 2);
            SendCANFrame(dataBytes33, 2);
            SendCANFrame(dataBytes34, 2);
            SendCANFrame(dataBytes35, 2);
            SendCANFrame(dataBytes36, 2);
            SendCANFrame(dataBytes37, 2);
        }

        private void SendCommandToDevice(uint deviceId, byte command)
        {
            byte[] dataBytes = new byte[8] { 0x00, command, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            try
            {
                int frame_type_index = 1; // 扩展帧
                ZCAN_Transmit_Data can_data = new ZCAN_Transmit_Data();
                can_data.frame.can_id = MakeCanId(deviceId, frame_type_index, 0, 0);
                can_data.frame.data = new byte[8];
                Array.Copy(dataBytes, can_data.frame.data, 8);
                can_data.frame.can_dlc = 8;
                can_data.transmit_type = 0;
                IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf(can_data));
                try
                {
                    Marshal.StructureToPtr(can_data, ptr, true);
                    Method.ZCAN_Transmit(channel_handle_, ptr, 1);
                }
                finally { Marshal.FreeHGlobal(ptr); }
            }
            catch (Exception ex) { Console.WriteLine($"SendCommandToDevice异常: {ex.Message}"); }
        }

        private void ParseVoltageProtectData(uint id, byte[] d)
        {
            if (d.Length < 8) return;
            ushort ovpSoft = (ushort)(d[2] | (d[3] << 8));
            ushort ovpHard = (ushort)(d[4] | (d[5] << 8));
            ushort uvp = (ushort)(d[6] | (d[7] << 8));
            lock (devicesInfoMap)
            {
                if (!devicesInfoMap.TryGetValue(id, out DeviceData dev)) dev = new DeviceData { ID = id };
                dev.LLC_Output_OverVoltage_Soft_Point = ovpSoft / 1000.0f;
                dev.LLC_Output_OverVoltage_Hard_Point = ovpHard / 1000.0f;
                dev.LLC_Output_UnderVoltage_Point = uvp / 1000.0f;
                devicesInfoMap[id] = dev;
            }
            RefreshProtectGrid(id);
        }

        private void ParseOCPProtectData(uint id, byte[] d)
        {
            if (d.Length < 8) return;
            ushort target = (ushort)(d[2] | (d[3] << 8));
            ushort soft = (ushort)(d[4] | (d[5] << 8));
            ushort recovery = (ushort)(d[6] | (d[7] << 8));
            lock (devicesInfoMap)
            {
                if (!devicesInfoMap.TryGetValue(id, out DeviceData dev)) dev = new DeviceData { ID = id };
                dev.LLC_Output_OverCurrent_Target_Point = target / 10.0f;
                dev.LLC_Output_OverCurrent_Soft_Point = soft / 10.0f;
                dev.LLC_Output_OverCurrent_Recovery_Point = recovery / 10.0f;
                devicesInfoMap[id] = dev;
            }
            RefreshProtectGrid(id);
        }

        private void ParseOSPProtectData(uint id, byte[] d)
        {
            if (d.Length < 8) return;
            ushort soft = (ushort)(d[2] | (d[3] << 8));
            ushort hard = (ushort)(d[4] | (d[5] << 8));
            lock (devicesInfoMap)
            {
                if (!devicesInfoMap.TryGetValue(id, out DeviceData dev)) dev = new DeviceData { ID = id };
                dev.LLC_Short_Circuit_Soft_Point = soft / 10.0f;
                dev.LLC_Short_Circuit_Hard_Point = hard / 10.0f;
                devicesInfoMap[id] = dev;
            }
            RefreshProtectGrid(id);
        }

        private void ParseLLCOutParaData(uint id, byte[] d)
        {
            if (d.Length < 8) return;
            ushort target = (ushort)(d[2] | (d[3] << 8));
            ushort coef = (ushort)(d[4] | (d[5] << 8));
            ushort refV = (ushort)(d[6] | (d[7] << 8));
            lock (devicesInfoMap)
            {
                if (!devicesInfoMap.TryGetValue(id, out DeviceData dev)) dev = new DeviceData { ID = id };
                dev.LLC_Output_Vbus_Target = target / 1000.0f;
                dev.LLC_Output_Vbus_Coef = coef / 1000.0f;
                dev.LLC_Output_Vbus_Ref = refV / 1000.0f;
                devicesInfoMap[id] = dev;
            }
            RefreshProtectGrid(id);
        }

        private void ParseTempProtectData(uint id, byte[] d)
        {
            // 响应格式：[cnt, 0xBE, ot_low, ot_high, rec_low, rec_high, 0, 0]
            // 温度单位：°C（整数，16位小端）
            if (d.Length < 6) return;
            ushort ot = (ushort)(d[2] | (d[3] << 8));
            ushort rec = (ushort)(d[4] | (d[5] << 8));
            lock (devicesInfoMap)
            {
                if (!devicesInfoMap.TryGetValue(id, out DeviceData dev)) dev = new DeviceData { ID = id };
                dev.LLC_OverTemp_Point = (float)ot;
                dev.LLC_OverTemp_Rec_Point = (float)rec;
                devicesInfoMap[id] = dev;
            }
            RefreshProtectGrid(id);
        }

        // ── PFC 保护/数据帧解析 ───────────────────────────────────────────────

        // 0x87: PFC 输入过压点
        // 帧结构: [cnt, 0x87, ovp_low, ovp_high, rec_low, rec_high, 0, 0]
        private void ParsePfcInputOvpData(uint id, byte[] d)
        {
            if (d.Length < 6) return;
            ushort ovp = (ushort)(d[2] | (d[3] << 8));       // 输入过压阈值 (mV)
            ushort rec = (ushort)(d[4] | (d[5] << 8));       // 输入过压恢复 (mV)
            lock (devicesInfoMap)
            {
                if (!devicesInfoMap.TryGetValue(id, out DeviceData dev)) dev = new DeviceData { ID = id };
                dev.PFC_Input_OVP_Point = ovp / 10.0f;
                dev.PFC_Input_OVP_Rec_Point = rec / 10.0f;
                devicesInfoMap[id] = dev;
            }
            RefreshProtectGrid(id);
        }

        // 0x88: PFC 输入欠压点
        // 帧结构: [cnt, 0x88, uvp_low, uvp_high, rec_low, rec_high, 0, 0]
        private void ParsePfcInputUvpData(uint id, byte[] d)
        {
            if (d.Length < 6) return;
            ushort uvp = (ushort)(d[2] | (d[3] << 8));       // 输入欠压阈值 (mV)
            ushort rec = (ushort)(d[4] | (d[5] << 8));       // 输入欠压恢复 (mV)
            lock (devicesInfoMap)
            {
                if (!devicesInfoMap.TryGetValue(id, out DeviceData dev)) dev = new DeviceData { ID = id };
                dev.PFC_Input_UVP_Point = uvp / 10.0f;
                dev.PFC_Input_UVP_Rec_Point = rec / 10.0f;
                devicesInfoMap[id] = dev;
            }
            RefreshProtectGrid(id);
        }

        // 0x89: PFC 输出过压点
        // 帧结构: [cnt, 0x89, ovp_low, ovp_high, rec_low, rec_high, 0, 0]
        private void ParsePfcOutputOvpData(uint id, byte[] d)
        {
            if (d.Length < 6) return;
            ushort ovp = (ushort)(d[2] | (d[3] << 8));       // 输出过压软件阈值 (mV)
            ushort rec = (ushort)(d[4] | (d[5] << 8));       // 输出过压硬件阈值 (mV)
            lock (devicesInfoMap)
            {
                if (!devicesInfoMap.TryGetValue(id, out DeviceData dev)) dev = new DeviceData { ID = id };
                dev.PFC_Output_OVP_Soft_Point = ovp / 10.0f;
                dev.PFC_Output_OVP_Hard_Point = rec / 10.0f;
                devicesInfoMap[id] = dev;
            }
            RefreshProtectGrid(id);
        }

        // 0x8A: PFC 输出欠压点
        // 帧结构: [cnt, 0x8A, uvp_low, uvp_high, rec_low, rec_high, 0, 0]
        private void ParsePfcOutputUvpData(uint id, byte[] d)
        {
            if (d.Length < 6) return;
            ushort uvp = (ushort)(d[2] | (d[3] << 8));       // 输出欠压阈值 (mV)
            ushort rec = (ushort)(d[4] | (d[5] << 8));       // 输出欠压恢复 (mV)
            lock (devicesInfoMap)
            {
                if (!devicesInfoMap.TryGetValue(id, out DeviceData dev)) dev = new DeviceData { ID = id };
                dev.PFC_Output_UVP_Point = uvp / 10.0f;
                dev.PFC_Output_UVP_Rec_Point = rec / 10.0f;
                devicesInfoMap[id] = dev;
            }
            RefreshProtectGrid(id);
        }

        // 0x8B: PFC 输入过流点
        // 帧结构: [cnt, 0x8B, soft_low, soft_high, dac_low, dac_high, 0, 0]
        private void ParsePfcInputOcpData(uint id, byte[] d)
        {
            if (d.Length < 6) return;
            ushort soft = (ushort)(d[2] | (d[3] << 8));      // 软件过流保护 (0.1A)
            ushort hard = (ushort)(d[4] | (d[5] << 8));      // 硬件过流保护 (0.1A)
            lock (devicesInfoMap)
            {
                if (!devicesInfoMap.TryGetValue(id, out DeviceData dev)) dev = new DeviceData { ID = id };
                dev.PFC_Input_OCP_Soft_Point = soft / 10.0f;
                dev.PFC_Input_OCP_Hard_Point = hard / 10.0f;
                devicesInfoMap[id] = dev;
            }
            RefreshProtectGrid(id);
        }

        // 0x8D: PFC 实时数据1 (live1)
        // 帧结构: [cnt, 0x8D, vin_rel_low, vin_rel_high, iloop_rel_low, iloop_rel_high, ntc_low, ntc_high]
        private void ParsePfcLive1Data(uint id, byte[] d)
        {
            if (d.Length < 8) return;
            ushort vinRel   = (ushort)(d[2] | (d[3] << 8));    // 输入电压实际值 (0.1V)
            ushort iLoopRel = (ushort)(d[4] | (d[5] << 8));    // 电流环参考值 (0.1A)
            short ntc       = (short)(d[6] | (d[7] << 8));     // NTC原始值
            lock (devicesInfoMap)
            {
                if (!devicesInfoMap.TryGetValue(id, out DeviceData dev)) dev = new DeviceData { ID = id };
                dev.PFC_Vin_Rel    = vinRel / 10.0f;
                dev.PFC_ILoop_Rel  = iLoopRel / 10.0f;
                dev.PFC_NTC        = ntc;
                devicesInfoMap[id] = dev;
            }
            RefreshProtectGrid(id);
        }

        // 0x8E: PFC 实时数据2 (live2)
        // 帧结构: [cnt, 0x8E, state, freq_khz, duty_low, duty_high, status_low, status_high]
        private void ParsePfcLive2Data(uint id, byte[] d)
        {
            if (d.Length < 8) return;
            byte state    = d[2];                              // 工作状态
            byte freq     = d[3];                              // 开关频率(kHz)
            ushort duty   = (ushort)(d[4] | (d[5] << 8));      // 占空比 (0.1%)
            ushort status = (ushort)(d[6] | (d[7] << 8));      // 状态标志
            lock (devicesInfoMap)
            {
                if (!devicesInfoMap.TryGetValue(id, out DeviceData dev)) dev = new DeviceData { ID = id };
                dev.PFC_State           = state;
                dev.PFC_Switch_Freq_kHz = freq;
                dev.PFC_Duty_Cycle      = duty / 10.0f;
                dev.PFC_Status_Flags    = status;
                devicesInfoMap[id] = dev;
            }
            RefreshProtectGrid(id);
        }

        // 0x8C: PFC vbus 运行数据
        // 帧结构: [cnt, 0x8C, target_low, target_high, ref_low, ref_high, rel_low, rel_high]
        private void ParsePfcVbusData(uint id, byte[] d)
        {
            if (d.Length < 8) return;
            ushort target = (ushort)(d[2] | (d[3] << 8));    // vbus_target (0.1V)
            ushort vref   = (ushort)(d[4] | (d[5] << 8));    // vbus_ref (0.1V)
            ushort vrel   = (ushort)(d[6] | (d[7] << 8));    // vbus_rel (0.1V)
            lock (devicesInfoMap)
            {
                if (!devicesInfoMap.TryGetValue(id, out DeviceData dev)) dev = new DeviceData { ID = id };
                dev.PFC_Vbus_Target = target / 10.0f;
                dev.PFC_Vbus_Ref    = vref / 10.0f;
                dev.PFC_Vbus_Rel    = vrel / 10.0f;
                devicesInfoMap[id] = dev;
            }
            RefreshProtectGrid(id);
        }

        // 刷新保护点表格中指定设备的数据
        private void RefreshProtectGrid(uint id)
        {
            DeviceData data;
            lock (devicesInfoMap)
            {
                if (!devicesInfoMap.TryGetValue(id, out data)) return;
            }
            this.BeginInvoke(new Action(() =>
            {
                try
                {
                    int colIdx = GetOrAddProtectColumn(id);
                    if (colIdx < 0) return;
                    // LLC 参数
                    SetProtectCell(dataGridView_protect, "LLC软件过压点(V)", colIdx, data.LLC_Output_OverVoltage_Soft_Point > 0 ? data.LLC_Output_OverVoltage_Soft_Point.ToString("F3") : "---");
                    SetProtectCell(dataGridView_protect, "LLC硬件过压点(V)", colIdx, data.LLC_Output_OverVoltage_Hard_Point > 0 ? data.LLC_Output_OverVoltage_Hard_Point.ToString("F3") : "---");
                    SetProtectCell(dataGridView_protect, "LLC欠压点(V)", colIdx, data.LLC_Output_UnderVoltage_Point > 0 ? data.LLC_Output_UnderVoltage_Point.ToString("F3") : "---");
                    SetProtectCell(dataGridView_protect, "LLC欠压恢复点(V)", colIdx, data.LLC_Output_UnderVoltage_Rec_Point > 0 ? data.LLC_Output_UnderVoltage_Rec_Point.ToString("F3") : "---");
                    SetProtectCell(dataGridView_protect, "LLC恒流点(A)", colIdx, data.LLC_Output_OverCurrent_Target_Point > 0 ? data.LLC_Output_OverCurrent_Target_Point.ToString("F1") : "---");
                    SetProtectCell(dataGridView_protect, "LLC软件过流点(A)", colIdx, data.LLC_Output_OverCurrent_Soft_Point > 0 ? data.LLC_Output_OverCurrent_Soft_Point.ToString("F1") : "---");
                    SetProtectCell(dataGridView_protect, "LLC过流恢复点(A)", colIdx, data.LLC_Output_OverCurrent_Recovery_Point > 0 ? data.LLC_Output_OverCurrent_Recovery_Point.ToString("F1") : "---");
                    SetProtectCell(dataGridView_protect, "LLC软件短路点(A)", colIdx, data.LLC_Short_Circuit_Soft_Point > 0 ? data.LLC_Short_Circuit_Soft_Point.ToString("F1") : "---");
                    SetProtectCell(dataGridView_protect, "LLC硬件短路点(A)", colIdx, data.LLC_Short_Circuit_Hard_Point > 0 ? data.LLC_Short_Circuit_Hard_Point.ToString("F1") : "---");
                    SetProtectCell(dataGridView_protect, "LLC过温点(°C)", colIdx, data.LLC_OverTemp_Point > 0 ? data.LLC_OverTemp_Point.ToString("F0") : "---");
                    SetProtectCell(dataGridView_protect, "LLC过温恢复点(°C)", colIdx, data.LLC_OverTemp_Rec_Point > 0 ? data.LLC_OverTemp_Rec_Point.ToString("F0") : "---");
                    SetProtectCell(dataGridView_protect, "LLC目标电压(V)", colIdx, data.LLC_Output_Vbus_Target > 0 ? data.LLC_Output_Vbus_Target.ToString("F3") : "---");
                    SetProtectCell(dataGridView_protect, "LLC校准电压(V)", colIdx, data.LLC_Output_Vbus_Coef > 0 ? data.LLC_Output_Vbus_Coef.ToString("F3") : "---");
                    SetProtectCell(dataGridView_protect, "LLC参考电压(V)", colIdx, data.LLC_Output_Vbus_Ref > 0 ? data.LLC_Output_Vbus_Ref.ToString("F3") : "---");
                    SetProtectCell(dataGridView_protect, "LLC KP参数", colIdx, data.LLC_KP != 0 ? data.LLC_KP.ToString("F3") : "---");
                    SetProtectCell(dataGridView_protect, "LLC KI参数", colIdx, data.LLC_KI != 0 ? data.LLC_KI.ToString("F3") : "---");
                    // PFC 保护/数据参数（映射 MCU 实际上报的字段）
                    // MCU 通过 UART 从 PFC 获取数据后经 CAN 转发，数据项：
                    //   0x87: 输入过压阈值/恢复, 0x88: 输入欠压阈值/恢复
                    //   0x89: 输出过压软件/硬件, 0x8A: 输出欠压阈值/恢复
                    //   0x8B: 输入过流软件/硬件, 0x8C: vbus_target/ref/rel
                    int pfcColIdx = GetOrAddPfcColumn(id);
                    SetProtectCell(dataGridView_pfc, "PFC_VBUS软件过压点(V)",       pfcColIdx, data.PFC_Output_OVP_Soft_Point  > 0 ? data.PFC_Output_OVP_Soft_Point.ToString("F3")  : "---");
                    SetProtectCell(dataGridView_pfc, "PFC_VBUS硬件过压点(V)",       pfcColIdx, data.PFC_Output_OVP_Hard_Point  > 0 ? data.PFC_Output_OVP_Hard_Point.ToString("F3")  : "---");
                    SetProtectCell(dataGridView_pfc, "PFC_VBUS欠压点(V)",           pfcColIdx, data.PFC_Output_UVP_Point       > 0 ? data.PFC_Output_UVP_Point.ToString("F3")       : "---");
                    SetProtectCell(dataGridView_pfc, "PFC_VBUS欠压恢复点(V)",       pfcColIdx, data.PFC_Output_UVP_Rec_Point   > 0 ? data.PFC_Output_UVP_Rec_Point.ToString("F3")   : "---");
                    SetProtectCell(dataGridView_pfc, "PFC_INPUT软件过流点(A)",       pfcColIdx, data.PFC_Input_OCP_Soft_Point   > 0 ? data.PFC_Input_OCP_Soft_Point.ToString("F1")   : "---");
                    SetProtectCell(dataGridView_pfc, "PFC_INPUT硬件过流点(A)",       pfcColIdx, data.PFC_Input_OCP_Hard_Point   > 0 ? data.PFC_Input_OCP_Hard_Point.ToString("F1")   : "---");
                    SetProtectCell(dataGridView_pfc, "PFC_INPUT输入过压点(V)",       pfcColIdx, data.PFC_Input_OVP_Point        > 0 ? data.PFC_Input_OVP_Point.ToString("F3")        : "---");
                    SetProtectCell(dataGridView_pfc, "PFC_INPUT输入过压恢复点(V)",       pfcColIdx, data.PFC_Input_OVP_Rec_Point    > 0 ? data.PFC_Input_OVP_Rec_Point.ToString("F3")    : "---");
                    SetProtectCell(dataGridView_pfc, "PFC_INPUT输入欠压点(V)",          pfcColIdx, data.PFC_Input_UVP_Point        > 0 ? data.PFC_Input_UVP_Point.ToString("F3")        : "---");
                    SetProtectCell(dataGridView_pfc, "PFC_INPUT输入欠压恢复点(V)",      pfcColIdx, data.PFC_Input_UVP_Rec_Point    > 0 ? data.PFC_Input_UVP_Rec_Point.ToString("F3")    : "---");
                    SetProtectCell(dataGridView_pfc, "PFC_VBUS目标电压(V)",         pfcColIdx, data.PFC_Vbus_Target            > 0 ? data.PFC_Vbus_Target.ToString("F3")            : "---");
                    SetProtectCell(dataGridView_pfc, "PFC VBUS参考电压",             pfcColIdx, data.PFC_Vbus_Ref               > 0 ? data.PFC_Vbus_Ref.ToString("F3")               : "---");
                    SetProtectCell(dataGridView_pfc, "PFC VBUS实际电压",             pfcColIdx, data.PFC_Vbus_Rel               > 0 ? data.PFC_Vbus_Rel.ToString("F3")               : "---");
                    SetProtectCell(dataGridView_pfc, "PFC 输入电压实际值(V)",         pfcColIdx, data.PFC_Vin_Rel                > 0 ? data.PFC_Vin_Rel.ToString("F1")                : "---");
                    SetProtectCell(dataGridView_pfc, "PFC 电流环参考值(A)",           pfcColIdx, data.PFC_ILoop_Rel              > 0 ? data.PFC_ILoop_Rel.ToString("F1")              : "---");
                    SetProtectCell(dataGridView_pfc, "PFC NTC温度",                   pfcColIdx, data.PFC_NTC != 0               ? data.PFC_NTC.ToString()                           : "---");
                    SetProtectCell(dataGridView_pfc, "PFC 工作状态",                  pfcColIdx, true                            ? data.PFC_State.ToString()                         : "---");
                    SetProtectCell(dataGridView_pfc, "PFC 开关频率(kHz)",             pfcColIdx, data.PFC_Switch_Freq_kHz > 0    ? data.PFC_Switch_Freq_kHz.ToString()               : "---");
                    SetProtectCell(dataGridView_pfc, "PFC 占空比(%)",                 pfcColIdx, data.PFC_Duty_Cycle > 0         ? data.PFC_Duty_Cycle.ToString("F1")                : "---");
                    SetProtectCell(dataGridView_pfc, "PFC 状态标志(hex)",             pfcColIdx, data.PFC_Status_Flags != 0        ? $"0x{data.PFC_Status_Flags:X4}"                 : "---");
                }
                catch (Exception ex) { Console.WriteLine($"RefreshProtectGrid异常: {ex.Message}"); }
            }));
        }

        private int GetOrAddProtectColumn(uint id)
        {
            uint pureId = GetId(id);
            string colName = $"dev_{pureId:X}";
            if (dataGridView_protect.Columns.Contains(colName))
                return dataGridView_protect.Columns[colName].Index;
            var col = new System.Windows.Forms.DataGridViewTextBoxColumn
            {
                HeaderText = $"0x{pureId:X5}",
                Name = colName,
                Width = 110,
                DefaultCellStyle = new System.Windows.Forms.DataGridViewCellStyle
                {
                    Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleCenter
                }
            };
            return dataGridView_protect.Columns.Add(col);
        }

        private int GetOrAddPfcColumn(uint id)
        {
            uint pureId = GetId(id);
            string colName = $"pfc_{pureId:X}";
            if (dataGridView_pfc.Columns.Contains(colName))
                return dataGridView_pfc.Columns[colName].Index;
            var col = new System.Windows.Forms.DataGridViewTextBoxColumn
            {
                HeaderText = $"0x{pureId:X5}",
                Name = colName,
                Width = 110,
                DefaultCellStyle = new System.Windows.Forms.DataGridViewCellStyle
                {
                    Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleCenter
                }
            };
            return dataGridView_pfc.Columns.Add(col);
        }

        private void SetProtectCell(System.Windows.Forms.DataGridView grid, string paramName, int colIdx, string value)
        {
            foreach (System.Windows.Forms.DataGridViewRow row in grid.Rows)
            {
                if (row.Cells[0].Value?.ToString() == paramName)
                {
                    if (row.Cells[colIdx].Value?.ToString() != value)
                        row.Cells[colIdx].Value = value;
                    return;
                }
            }
        }

        private void Button_queryProtect_Click(object? sender, EventArgs e)
        {
            SendProtectQueryCommands();
        }

        private void UpdateChannelCurrent(uint id, double current)
        {
            lock (_currentSharingLock)
            {
                uint actualId = GetId(id);
                if (actualId >= TARGET_ID_START && actualId <= TARGET_ID_END)
                {
                    _moduleCurrents[actualId] = current;
                    UpdateActiveModulesList();
                }
            }
        }

        private void UpdateActiveModulesList()
        {
            _activeModules.Clear();
            for (uint id = TARGET_ID_START; id <= TARGET_ID_END; id++)
            {
                if (_moduleCurrents.ContainsKey(id) && _moduleCurrents[id] > 0.1)
                {
                    _activeModules.Add(id);
                    if (_activeModules.Count >= CHANNEL_COUNT) break;
                }
            }
        }

        // ── 通讯指示灯 ──────────────────────────────────────────────────────
        // 收到数据：绿色亮 100ms 后变灰；超时 3s 无数据：变红
        private void UpdateCommLed(uint ch, bool isReceiving)
        {
            if (ch < 1 || ch > CHANNEL_COUNT) return;
            string key = $"comm_led_ch{ch}";
            if (!displayLabels.TryGetValue(key, out var ledLabel)) return;

            if (isReceiving)
            {
                _lastCommTime[ch] = DateTime.Now;

                // 取消上一个闪烁定时器，防止叠加
                _commFlashTimers[ch]?.Dispose();
                _commFlashTimers[ch] = null;

                // 立即变绿
                this.BeginInvoke(new Action(() =>
                {
                    ledLabel.ForeColor = Color.Lime;
                }));

                // 100ms 后变灰（表示"刚收到，现在等待"）
                _commFlashTimers[ch] = new System.Threading.Timer(_ =>
                {
                    try
                    {
                        this.BeginInvoke(new Action(() =>
                        {
                            ledLabel.ForeColor = Color.DimGray;
                        }));
                    }
                    catch { }
                    _commFlashTimers[ch]?.Dispose();
                    _commFlashTimers[ch] = null;
                }, null, GREEN_FLASH_MS, System.Threading.Timeout.Infinite);
            }
        }

        // 定时检查各通道通讯超时，超时变红
        private void CheckCommTimeout()
        {
            DateTime now = DateTime.Now;
            for (uint ch = 1; ch <= CHANNEL_COUNT; ch++)
            {
                if (_lastCommTime[ch] == DateTime.MinValue) continue; // 从未收到，保持灰色
                double elapsed = (now - _lastCommTime[ch]).TotalMilliseconds;
                if (elapsed > COMM_RED_TIMEOUT_MS)
                {
                    string key = $"comm_led_ch{ch}";
                    if (displayLabels.TryGetValue(key, out var ledLabel))
                    {
                        this.BeginInvoke(new Action(() =>
                        {
                            ledLabel.ForeColor = Color.Red;
                        }));
                    }
                }
            }
        }

        private void LogDataToExcel(uint id, double voltage, double current, double temperature, uint channel, byte powerStatus)
        {
            if (_excelFilePath == null) return;
            // 只入队，不阻塞 UI 线程
            _excelQueue.Enqueue((DateTime.Now, id, voltage, current, temperature, channel, powerStatus));
        }

        private void FlushExcelQueue()
        {
            if (_excelQueue.IsEmpty) return;
            string? path = _excelFilePath;
            // 即使 _excelFilePath 已被清空（停用时），仍用 savedPath 写完剩余数据
            if (path == null)
            {
                // 停用时 FlushExcelQueue 在 _excelFilePath 清空前已保存 savedPath，此处直接返回
                return;
            }
            lock (_excelWriteLock)
            {
                try
                {
                    string? directory = Path.GetDirectoryName(path);
                    if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
                        Directory.CreateDirectory(directory);

                    using var workbook = File.Exists(path) ? new XLWorkbook(path) : new XLWorkbook();
                    var worksheet = workbook.Worksheets.Count > 0 ? workbook.Worksheet(1) : workbook.Worksheets.Add("Data");

                    if (worksheet.LastRowUsed() == null)
                    {
                        worksheet.Cell(1, 1).Value = "时间戳";
                        worksheet.Cell(1, 2).Value = "通道";
                        worksheet.Cell(1, 3).Value = "设备ID";
                        worksheet.Cell(1, 4).Value = "电压(V)";
                        worksheet.Cell(1, 5).Value = "电流(A)";
                        worksheet.Cell(1, 6).Value = "温度(°C)";
                        worksheet.Cell(1, 7).Value = "电源状态Raw(Hex)";
                        worksheet.Cell(1, 8).Value = "电源开关";
                        worksheet.Cell(1, 9).Value = "过压";
                        worksheet.Cell(1, 10).Value = "欠压";
                        worksheet.Cell(1, 11).Value = "过流";
                        worksheet.Cell(1, 12).Value = "过温";
                        worksheet.Cell(1, 13).Value = "通讯正常";
                        worksheet.Row(1).Style.Font.Bold = true;
                    }

                    int row = (worksheet.LastRowUsed()?.RowNumber() ?? 0) + 1;
                    while (_excelQueue.TryDequeue(out var entry))
                    {
                        if (row > 1048000) break; // 接近 xlsx 行数上限时停止
                        worksheet.Cell(row, 1).Value = entry.ts;
                        worksheet.Cell(row, 1).Style.DateFormat.Format = "yyyy-MM-dd HH:mm:ss";
                        worksheet.Cell(row, 2).Value = entry.channel;
                        worksheet.Cell(row, 3).Value = $"0x{GetId(entry.id):X5}";
                        worksheet.Cell(row, 4).Value = entry.voltage;
                        worksheet.Cell(row, 5).Value = entry.current;
                        worksheet.Cell(row, 6).Value = entry.temperature;
                        // 状态拆解记录
                        byte ps = entry.powerStatus;
                        worksheet.Cell(row, 7).Value = $"0x{ps:X2}";
                        worksheet.Cell(row, 8).Value = (ps & 0x01) != 0 ? "开启" : "关闭";
                        worksheet.Cell(row, 9).Value = (ps & 0x02) != 0 ? "是" : "否";
                        worksheet.Cell(row, 10).Value = (ps & 0x04) != 0 ? "是" : "否";
                        worksheet.Cell(row, 11).Value = (ps & 0x08) != 0 ? "是" : "否";
                        worksheet.Cell(row, 12).Value = (ps & 0x10) != 0 ? "是" : "否";
                        worksheet.Cell(row, 13).Value = (ps & 0x20) != 0 ? "正常" : "异常";
                        row++;
                    }
                    workbook.SaveAs(path);
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Excel刷写异常: {ex.Message}");
                }
            }
        }

        private void AddError()
        {
            ZCAN_CHANNEL_ERROR_INFO pErrInfo = new ZCAN_CHANNEL_ERROR_INFO();
            IntPtr ptr = IntPtr.Zero;

            try
            {
                ptr = Marshal.AllocHGlobal(Marshal.SizeOf(pErrInfo));
                Marshal.StructureToPtr(pErrInfo, ptr, true);

                if (Method.ZCAN_ReadChannelErrInfo(channel_handle_, ptr) != Define.STATUS_OK)
                {
                    MessageBox.Show("获取错误信息失败", "提示", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                    return;
                }

                pErrInfo = Marshal.PtrToStructure<ZCAN_CHANNEL_ERROR_INFO>(ptr);
                string errorInfo = $"错误码：{pErrInfo.error_code:D1}";

                AppendRecvLog(errorInfo);
            }
            finally
            {
                if (ptr != IntPtr.Zero)
                    Marshal.FreeHGlobal(ptr);
            }
        }

        // ── 电源开关指示灯注册──────
        private void CreatePowerLedLabels()
        {
            Label[] powerLabels =
            {
                led_pwr_01, led_pwr_02, led_pwr_03, led_pwr_04, led_pwr_05,
                led_pwr_06, led_pwr_07, led_pwr_08, led_pwr_09, led_pwr_10
            };

            for (int i = 1; i <= powerLabels.Length; i++)
            {
                string suffix = i.ToString("D2");
                Label label = powerLabels[i - 1];
                Control? panel = Controls.Find($"panel_ch{suffix}", true).FirstOrDefault();
                Control? overTemp = Controls.Find($"led_ot_{suffix}", true).FirstOrDefault();

                if (panel == null || overTemp == null)
                    continue;

                label.Name = $"led_pwr_{suffix}";
                label.Text = "ON-OFF";
                label.TextAlign = ContentAlignment.MiddleCenter;
                label.BackColor = Color.Gray;
                label.ForeColor = Color.White;
                label.Size = new Size(62, overTemp.Height);
                label.Location = new Point(overTemp.Right + 8, overTemp.Top);
                label.Visible = true;

                if (label.Parent == null)
                    panel.Controls.Add(label);

                label.BringToFront();
                displayLabels[$"led_pwr_ch{i}"] = label;
            }
        }

        // ── Named event handlers (called from Designer.cs) ────────────────────

        private void IapBrowseButton_Click(object? sender, EventArgs e)
        {
            using var dialog = new OpenFileDialog
            {
                Filter = "固件文件 (*.bin;*.hex)|*.bin;*.hex|BIN文件 (*.bin)|*.bin|HEX文件 (*.hex)|*.hex|所有文件 (*.*)|*.*",
                Title = "选择升级固件",
                CheckFileExists = true
            };

            if (dialog.ShowDialog(this) == DialogResult.OK && _iapFileTextBox != null)
                _iapFileTextBox.Text = dialog.FileName;
        }

        private async void IapStartButton_Click(object? sender, EventArgs e)
        {
            if (!m_bStart)
            {
                MessageBox.Show("请先打开并启动 CAN 通道。", "提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
                return;
            }

            if (_iapFileTextBox == null || string.IsNullOrWhiteSpace(_iapFileTextBox.Text) || !File.Exists(_iapFileTextBox.Text))
            {
                MessageBox.Show("请选择有效的固件文件。", "提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
                return;
            }

            byte targetAddr = _iapTargetComboBox?.SelectedItem?.ToString() == "PFC" ? (byte)1 : (byte)2;
            string targetName = _iapTargetComboBox?.SelectedItem?.ToString() ?? "LLC";
            int canChannel = _iapChannelComboBox?.SelectedIndex ?? 0;
            bool legacyCanIdMode = _iapCanIdModeComboBox?.SelectedIndex == 1;
            var selectedNodes = legacyCanIdMode
                ? new List<int> { 0 }
                : _iapNodesCheckedListBox?.CheckedIndices.Cast<int>().OrderBy(node => node).ToList() ?? new List<int>();
            if (!legacyCanIdMode && selectedNodes.Count == 0)
            {
                MessageBox.Show("请至少选择一个节点。", "提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
                return;
            }

            _iapUpgradeCts = new CancellationTokenSource();
            _iapUpgradeInProgress = true;
            _autoRefreshWasEnabledBeforeIap = timer1.Enabled;
            timer1.Enabled = false;
            AppendIapLog("升级开始，已暂停自动读取和普通控制命令");

            SetIapControlsRunning(true);
            try
            {
                int successCount = 0;
                var failedNodes = new List<int>();
                string idModeText = legacyCanIdMode ? "固定ID 0xAA55" : "节点ID 0xA0000~0xA0007";
                string startText = legacyCanIdMode
                    ? $"开始固定ID升级 {targetName}，CAN ID模式: {idModeText}"
                    : $"开始批量升级 {targetName}，节点: {string.Join(",", selectedNodes)}，CAN ID模式: {idModeText}，采用顺序升级";
                AppendIapLog(startText);

                for (int index = 0; index < selectedNodes.Count; index++)
                {
                    _iapUpgradeCts.Token.ThrowIfCancellationRequested();
                    int nodeId = selectedNodes[index];
                    uint nodeCanId = legacyCanIdMode
                        ? CanIapUpgradeSession.LegacyIapCanId
                        : CanIapUpgradeSession.GetNodeCanId(nodeId);
                    int completedNodes = index;
                    string itemName = legacyCanIdMode ? "固定ID" : $"节点{nodeId}";

                    AppendIapLog($"{itemName}开始，CAN ID=0x{nodeCanId:X5}，目标={targetName}");
                    _iapUpgradeSession = new CanIapUpgradeSession(
                        nodeCanId,
                        (packet, token) => SendIapPacketByCanAsync(packet, canChannel, nodeCanId, token),
                        text => AppendIapLog($"{itemName}: {text}"),
                        value => UpdateIapProgress((completedNodes * 100 + value) / selectedNodes.Count),
                        $"{targetName}_{nodeCanId:X5}_{canChannel}",
                        token => RecoverIapCanChannelAsync(canChannel, token));

                    bool nodeOk;
                    try
                    {
                        nodeOk = await _iapUpgradeSession.UpgradeAsync(targetAddr, _iapFileTextBox.Text, _iapUpgradeCts.Token);
                    }
                    catch (OperationCanceledException)
                    {
                        throw;
                    }
                    catch (Exception ex)
                    {
                        AppendIapLog($"{itemName}异常: {ex.Message}");
                        nodeOk = false;
                    }

                    if (nodeOk)
                    {
                        successCount++;
                        AppendIapLog($"{itemName}升级成功");
                    }
                    else
                    {
                        failedNodes.Add(nodeId);
                        AppendIapLog(legacyCanIdMode ? $"{itemName}升级失败" : $"{itemName}升级失败，继续下一节点");
                    }

                    UpdateIapProgress((index + 1) * 100 / selectedNodes.Count);
                }

                bool ok = failedNodes.Count == 0;
                string summary = ok
                    ? (legacyCanIdMode ? "固定ID升级完成" : $"批量升级完成，成功 {successCount}/{selectedNodes.Count}")
                    : (legacyCanIdMode
                        ? "固定ID升级失败"
                        : $"批量升级完成，成功 {successCount}/{selectedNodes.Count}，失败节点: {string.Join(",", failedNodes)}");
                AppendIapLog(summary);
                if (_iapStatusLabel != null)
                {
                    _iapStatusLabel.Text = summary;
                    _iapStatusLabel.ForeColor = ok ? Color.Green : Color.Red;
                }
            }
            catch (OperationCanceledException)
            {
                AppendIapLog("升级已取消");
                if (_iapStatusLabel != null)
                {
                    _iapStatusLabel.Text = "升级已取消";
                    _iapStatusLabel.ForeColor = Color.DarkOrange;
                }
            }
            catch (Exception ex)
            {
                AppendIapLog($"升级异常: {ex.Message}");
                if (_iapStatusLabel != null)
                {
                    _iapStatusLabel.Text = "升级异常";
                    _iapStatusLabel.ForeColor = Color.Red;
                }
            }
            finally
            {
                _iapUpgradeSession = null;
                _iapUpgradeInProgress = false;
                if (_autoRefreshWasEnabledBeforeIap && checkBox_autoRefresh.Checked)
                {
                    UpdateTimerInterval();
                    timer1.Enabled = true;
                    AppendIapLog("升级结束，已恢复自动读取");
                }
                SetIapControlsRunning(false);
                _iapUpgradeCts?.Dispose();
                _iapUpgradeCts = null;
            }
        }

        private void IapStopButton_Click(object? sender, EventArgs e)
        {
            _iapUpgradeCts?.Cancel();
        }

        private async Task SendIapPacketByCanAsync(byte[] packet, int channel, uint canId, CancellationToken token)
        {
            const int iapCanFrameDelayMs = 1;
            const int iapPacketGapMs = 3;

            for (int offset = 0; offset < packet.Length; offset += 8)
            {
                token.ThrowIfCancellationRequested();
                int len = Math.Min(8, packet.Length - offset);
                var frame = new byte[len];
                Array.Copy(packet, offset, frame, 0, len);
                try
                {
                    SendIapCanFrame(frame, channel, canId);
                }
                catch
                {
                    await RecoverIapCanChannelAsync(channel, token);
                    SendIapCanFrame(frame, channel, canId);
                }
                if (iapCanFrameDelayMs > 0)
                    await Task.Delay(iapCanFrameDelayMs, token);
            }

            if (iapPacketGapMs > 0)
                await Task.Delay(iapPacketGapMs, token);
        }

        private async Task RecoverIapCanChannelAsync(int channel, CancellationToken token)
        {
            token.ThrowIfCancellationRequested();

            IntPtr handle = channel == 0 ? channel_handle_ : channel_handle2_;
            if (handle == IntPtr.Zero)
                throw new InvalidOperationException($"CAN通道{channel + 1}未初始化");

            if (recv_data_thread_ != null)
                recv_data_thread_.SetStart(false);

            await Task.Delay(100, token);

            if (Method.ZCAN_ResetCAN(handle) != Define.STATUS_OK)
                throw new InvalidOperationException($"复位CAN通道{channel + 1}失败");

            await Task.Delay(200, token);

            if (Method.ZCAN_StartCAN(handle) != Define.STATUS_OK)
                throw new InvalidOperationException($"启动CAN通道{channel + 1}失败");

            m_bStart = true;
            if (recv_data_thread_ == null)
            {
                recv_data_thread_ = new RecvDataThread();
                recv_data_thread_.RecvCANData += this.AddCANData;
                recv_data_thread_.RecvFDData += this.AddFDData;
            }

            recv_data_thread_.SetChannelHandle(channel_handle_, channel_handle2_);
            recv_data_thread_.SetStart(true);
            await Task.Delay(300, token);
        }

        private void SendIapCanFrame(byte[] data, int channel, uint canId)
        {
            ZCAN_Transmit_Data canData = new ZCAN_Transmit_Data
            {
                frame = new can_frame
                {
                    can_id = MakeCanId(canId, 1, 0, 0),
                    data = new byte[8],
                    can_dlc = (byte)data.Length
                },
                transmit_type = 0
            };
            Array.Copy(data, canData.frame.data, data.Length);

            IntPtr ptr = IntPtr.Zero;
            try
            {
                ptr = Marshal.AllocHGlobal(Marshal.SizeOf(canData));
                Marshal.StructureToPtr(canData, ptr, true);
                uint result = Method.ZCAN_Transmit(channel == 0 ? channel_handle_ : channel_handle2_, ptr, 1);
                if (result != 1)
                    throw new InvalidOperationException("CAN IAP 帧发送失败");
            }
            finally
            {
                if (ptr != IntPtr.Zero) Marshal.FreeHGlobal(ptr);
            }
        }

        private void AppendIapLog(string text)
        {
            string line = $"[IAP] {text}";
            AppendRecvLog(line);

            if (_iapLogTextBox == null || !_iapLogTextBox.IsHandleCreated)
                return;

            _iapLogTextBox.BeginInvoke(new Action(() =>
            {
                _iapLogTextBox.AppendText($"{DateTime.Now:HH:mm:ss}  {text}{Environment.NewLine}");
                if (_iapLogTextBox.Lines.Length > 800)
                {
                    int removeEnd = _iapLogTextBox.GetFirstCharIndexFromLine(200);
                    if (removeEnd > 0)
                    {
                        _iapLogTextBox.Select(0, removeEnd);
                        _iapLogTextBox.SelectedText = "";
                    }
                }
                _iapLogTextBox.SelectionStart = _iapLogTextBox.TextLength;
                _iapLogTextBox.ScrollToCaret();
            }));
        }

        private void UpdateIapProgress(int value)
        {
            if (_iapProgressBar == null) return;
            int progress = Math.Max(0, Math.Min(100, value));
            BeginInvoke(new Action(() =>
            {
                _iapProgressBar.Value = progress;
                if (_iapProgressValueLabel != null)
                    _iapProgressValueLabel.Text = $"{progress}%";
            }));
        }

        private void SetIapControlsRunning(bool running)
        {
            BeginInvoke(new Action(() =>
            {
                if (_iapStartButton != null) _iapStartButton.Enabled = !running;
                if (_iapStopButton != null) _iapStopButton.Enabled = running;
                if (_iapTargetComboBox != null) _iapTargetComboBox.Enabled = !running;
                if (_iapChannelComboBox != null) _iapChannelComboBox.Enabled = !running;
                if (_iapCanIdModeComboBox != null) _iapCanIdModeComboBox.Enabled = !running;
                if (_iapNodesCheckedListBox != null)
                    _iapNodesCheckedListBox.Enabled = !running && _iapCanIdModeComboBox?.SelectedIndex != 1;
                checkBox_autoRefresh.Enabled = !running;
                if (_iapStatusLabel != null)
                {
                    _iapStatusLabel.Text = running ? "升级中..." : _iapStatusLabel.Text;
                    _iapStatusLabel.ForeColor = running ? Color.Blue : _iapStatusLabel.ForeColor;
                }
            }));
        }

        private void Button_clear_Click(object? sender, EventArgs e)
        {
            this.richTextBox_recv.Clear();
        }

        // 统一的日志追加方法：线程安全，自动限制行数，支持自动滚动
        private void AppendRecvLog(string text)
        {
            this.BeginInvoke(new Action(() =>
            {
                richTextBox_recv.AppendText(text + Environment.NewLine);
                // 超过 500 行时删除前 100 行，避免内存无限增长
                if (richTextBox_recv.Lines.Length > 500)
                {
                    int removeEnd = richTextBox_recv.GetFirstCharIndexFromLine(100);
                    if (removeEnd > 0)
                    {
                        richTextBox_recv.Select(0, removeEnd);
                        richTextBox_recv.SelectedText = "";
                    }
                }
                if (checkBox_autoScroll.Checked)
                {
                    richTextBox_recv.SelectionStart = richTextBox_recv.TextLength;
                    richTextBox_recv.ScrollToCaret();
                }
            }));
        }

        private void Button_startSystem_Click(object? sender, EventArgs e)
        {
            SendSimpleCommand(CMD_START);
        }

        private void Button_stopSystem_Click(object? sender, EventArgs e)
        {
            SendSimpleCommand(CMD_STOP);
        }

        private void Button_readStatus_Click(object? sender, EventArgs e)
        {
            SendSimpleCommand(CMD_QUERY);
        }

        private void Button_storeFlash_Click(object? sender, EventArgs e)
        {
            SendSimpleCommand(CMD_STORE_FLASH);
        }

        private void Button_loadFlash_Click(object? sender, EventArgs e)
        {
            SendSimpleCommand(CMD_LOAD_FLASH);
        }

        private void Button_readVersion_Click(object? sender, EventArgs e)
        {
            SendSimpleCommand(CMD_VERSION);
        }

        private void Button_scanDevice_Click(object? sender, EventArgs e)
        {
            SendCANData(0x08, 0x00000000);
        }

        private void TextBox_masterAddr1_TextChanged(object? sender, EventArgs e)
        {
            ValidateAndEnableCheckbox(this.textBox_masterAddr1, this.checkBox_ch1Enable);
        }

        private void TextBox_masterAddr2_TextChanged(object? sender, EventArgs e)
        {
            ValidateAndEnableCheckbox(this.textBox_masterAddr2, this.checkBox_ch2Enable);
        }

        private void Btn_addr1_20_Click(object? sender, EventArgs e)
        {
            this.textBox_masterAddr1.Text = "20";
            ValidateAndEnableCheckbox(this.textBox_masterAddr1, this.checkBox_ch1Enable);
        }

        private void Btn_addr1_B0000_Click(object? sender, EventArgs e)
        {
            this.textBox_masterAddr1.Text = "B0000";
            ValidateAndEnableCheckbox(this.textBox_masterAddr1, this.checkBox_ch1Enable);
        }

        private void Btn_addr2_20_Click(object? sender, EventArgs e)
        {
            this.textBox_masterAddr2.Text = "20";
            ValidateAndEnableCheckbox(this.textBox_masterAddr2, this.checkBox_ch2Enable);
        }

        private void Btn_addr2_B0000_Click(object? sender, EventArgs e)
        {
            this.textBox_masterAddr2.Text = "B0000";
            ValidateAndEnableCheckbox(this.textBox_masterAddr2, this.checkBox_ch2Enable);
        }

        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            try
            {
                if (timer1 != null)
                {
                    timer1.Enabled = false;
                    timer1.Stop();
                }

                // 停止 Excel 定时器并刷写剩余数据
                _excelFlushTimer?.Dispose();
                _excelFlushTimer = null;
                if (_excelFilePath != null) FlushExcelQueue();

                // 释放所有通讯指示灯定时器
                for (int i = 1; i <= CHANNEL_COUNT; i++)
                {
                    _commFlashTimers[i]?.Dispose();
                    _commFlashTimers[i] = null;
                }

                if (recv_data_thread_ != null)
                {
                    recv_data_thread_.SetStart(false);
                }

                if (m_bOpen)
                {
                    Method.ZCAN_CloseDevice(device_handle_);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"关闭窗口时发生异常: {ex.Message}");
            }

            base.OnFormClosed(e);
        }

        private void panel_channels_Paint(object sender, PaintEventArgs e)
        {

        }
    }

    // 设备数据结构体
    public struct DeviceData
    {
        public uint ID;                                 //  通信ID
        public float Target_Voltage;                    //  目标电压
        public float Voltage;                           //  当前输出电压
        public float Current;                           //  当前输出电流
        public float Load_Current;                      //  当前母线电流
        public int Temperature;                         //  当前温度
        public int Channel;                             //  当前通道

        public float Under_OutPut_Voltage_Point;        //  欠压点
        public float Under_Rec_OutPut_Voltage_Point;    //  欠压恢复点
        public bool Under_OutPut_Voltage_flag;          //  欠压标志位
        public float Over_OutPut_Voltage_Point;         //  过压点
        public float Over_Rec_OutPut_Voltage_Point;     //  过压恢复点
        public bool Over_OutPut_Voltage_flag;           //  过压标志位
        public float Over_Temp_Point;                   //  过温压点
        public float Over_Rec_Temp_Point;               //  过温恢复点
        public bool Over_Temp_flag;                     //  过温标志位
        public bool Over_Temp_mode;                     //  过温设置： 0：过温后打嗝  1：过温后锁死
        public bool PFC_OK_flag;                        //  PFC_OK 信号
        public bool ON_OFF_OK_flag;                     //  ON_OFF 信号
        public bool Pre_Driver_flag;                    //  预启管 信号
        public uint LLC_State;                          //  当前状态

        // LLC保护点参数
        public float LLC_Output_OverVoltage_Soft_Point;
        public float LLC_Output_OverVoltage_Hard_Point;
        public float LLC_Output_UnderVoltage_Point;
        public float LLC_Output_UnderVoltage_Rec_Point;
        public float LLC_Output_OverCurrent_Target_Point;
        public float LLC_Output_OverCurrent_Soft_Point;
        public float LLC_Output_OverCurrent_Recovery_Point;
        public float LLC_Short_Circuit_Soft_Point;
        public float LLC_Short_Circuit_Hard_Point;
        public float LLC_OverTemp_Point;
        public float LLC_OverTemp_Rec_Point;
        public float LLC_Output_Vbus_Target;
        public float LLC_Output_Vbus_Coef;
        public float LLC_Output_Vbus_Ref;
        public float LLC_KP;
        public float LLC_KI;

        // PFC保护点参数
        public float PFC_Input_OVP_Point;
        public float PFC_Input_OVP_Rec_Point;
        public float PFC_Input_UVP_Point;
        public float PFC_Input_UVP_Rec_Point;
        public float PFC_Output_OVP_Soft_Point;
        public float PFC_Output_OVP_Hard_Point;
        public float PFC_Output_UVP_Point;
        public float PFC_Output_UVP_Rec_Point;
        public float PFC_Input_OCP_Soft_Point;
        public float PFC_Input_OCP_Hard_Point;
        public float PFC_Vbus_Target;
        public float PFC_Vbus_Ref;
        public float PFC_Vbus_Rel;

        // PFC 实时数据 live1
        public float PFC_Vin_Rel;
        public float PFC_ILoop_Rel;
        public int PFC_NTC;

        // PFC 实时数据 live2
        public byte PFC_State;
        public byte PFC_Switch_Freq_kHz;
        public float PFC_Duty_Cycle;
        public ushort PFC_Status_Flags;
    }
}
