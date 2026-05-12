using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Management;
using System.Reflection;
using System.Threading.Tasks;
using System.Windows.Forms;
using Newtonsoft.Json;

namespace IapUpgradeTool
{
    public partial class MainForm : Form
    {
        private ComboBox _portComboBox;

        // === 新增简化模式控件 ===
        private GroupBox _simpleGroupBox;
        private ComboBox _chipTypeComboBox;
        private ComboBox _targetComboBox;
        private NumericUpDown _modbusAddrUpDown;
        private NumericUpDown _baudrateUpDown;
        private TextBox _firmwareFileTextBox;
        private Button _selectFirmwareButton;
        private Button _upgradeSimpleButton;

        // === 原有控件保持不变 ===
        private Button _loadConfigButton;
        private Button _upgradeButton;
        private Button _stopButton;
        private RichTextBox _configTextBox;
        private RichTextBox _logTextBox;
        private ProgressBar _progressBar;
        private Label _statusLabel;
        private System.Windows.Forms.Timer _portUpdateTimer;
        private System.Windows.Forms.Timer _uartCloseTimer;
        private DateTime _lastOperationTime;
        private SerialPort _serialPort;
        private IapMaster _iapMaster;
        private IapConfig _currentConfig;
        private bool _isUpgrading = false;
        public enum LogLevel { Info, Warning, Error, Debug }
        private DateTime _lastProgressTime = DateTime.Now;
        private int _lastProgressBytes = 0;

        // === 新增：模式标识 ===
        private bool _isSimpleMode = false;

        public MainForm()
        {
            try
            {
                this.Icon = new Icon("resources\\zhld.ico");
            }
            catch { }

            InitializeComponent();
            InitializeIapMaster();
            InitializeTimers();
            UpdatePortList();
        }

        private void InitializeComponent()
        {
            this.Size = new Size(900, 800);  // 稍微增加高度
            this.FormBorderStyle = FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.StartPosition = FormStartPosition.CenterScreen;
            this.Text = $"IAP升级工具 {GetVersionString()} - 深圳中瀚蓝盾电源有限公司";

            int yPos = 20;

            // === 串口选择（保持原有位置） ===
            var portLabel = new Label
            {
                Text = "串口:",
                Location = new Point(20, yPos + 5),
                Size = new Size(50, 20)
            };
            this.Controls.Add(portLabel);

            _portComboBox = new ComboBox
            {
                Location = new Point(80, yPos),
                Size = new Size(200, 25),
                DropDownStyle = ComboBoxStyle.DropDown
            };
            this.Controls.Add(_portComboBox);

            yPos += 40;

            // === 新增：简化模式区域 ===
            _simpleGroupBox = new GroupBox
            {
                Text = "简化模式 - 直接升级（无需JSON配置文件）",
                Location = new Point(20, yPos),
                Size = new Size(840, 120),
                ForeColor = Color.DarkGreen
            };
            this.Controls.Add(_simpleGroupBox);

            // 简化模式内的控件
            int simpleY = 25;

            var chipLabel = new Label
            {
                Text = "芯片:",
                Location = new Point(15, simpleY),
                Size = new Size(50, 20)
            };
            _simpleGroupBox.Controls.Add(chipLabel);

            _chipTypeComboBox = new ComboBox
            {
                Location = new Point(70, simpleY - 3),
                Size = new Size(80, 25),
                DropDownStyle = ComboBoxStyle.DropDownList
            };
            _chipTypeComboBox.Items.AddRange(new[] { "5300", "5800", "自定义" });
            _chipTypeComboBox.SelectedIndex = 0; // 默认5300
            _chipTypeComboBox.SelectedIndexChanged += ChipTypeComboBox_SelectedIndexChanged;
            _simpleGroupBox.Controls.Add(_chipTypeComboBox);

            var targetLabel = new Label
            {
                Text = "目标:",
                Location = new Point(170, simpleY),
                Size = new Size(40, 20)
            };
            _simpleGroupBox.Controls.Add(targetLabel);

            _targetComboBox = new ComboBox
            {
                Location = new Point(215, simpleY - 3),
                Size = new Size(70, 25),
                DropDownStyle = ComboBoxStyle.DropDownList
            };
            _targetComboBox.Items.AddRange(new[] { "LLC", "PFC" });
            _targetComboBox.SelectedIndex = 0; // 默认LLC
            _targetComboBox.SelectedIndexChanged += TargetComboBox_SelectedIndexChanged;
            _simpleGroupBox.Controls.Add(_targetComboBox);

            var addrLabel = new Label
            {
                Text = "地址:",
                Location = new Point(305, simpleY),
                Size = new Size(40, 20),
                ForeColor = Color.Gray
            };
            _simpleGroupBox.Controls.Add(addrLabel);

            _modbusAddrUpDown = new NumericUpDown
            {
                Location = new Point(350, simpleY - 3),
                Size = new Size(60, 25),
                Minimum = 1,
                Maximum = 247,
                Value = 2,
                Enabled = false
            };
            _simpleGroupBox.Controls.Add(_modbusAddrUpDown);

            var baudLabel = new Label
            {
                Text = "波特率:",
                Location = new Point(430, simpleY),
                Size = new Size(50, 20)
            };
            _simpleGroupBox.Controls.Add(baudLabel);

            _baudrateUpDown = new NumericUpDown
            {
                Location = new Point(485, simpleY - 3),
                Size = new Size(80, 25),
                Minimum = 9600,
                Maximum = 460800,
                Value = 115200,
                Increment = 9600
            };
            _simpleGroupBox.Controls.Add(_baudrateUpDown);

            simpleY += 35;

            var firmwareLabel = new Label
            {
                Text = "固件:",
                Location = new Point(15, simpleY),
                Size = new Size(50, 20)
            };
            _simpleGroupBox.Controls.Add(firmwareLabel);

            _firmwareFileTextBox = new TextBox
            {
                Location = new Point(70, simpleY - 3),
                Size = new Size(400, 25),
                ReadOnly = true,
                BackColor = Color.White
            };
            _simpleGroupBox.Controls.Add(_firmwareFileTextBox);

            _selectFirmwareButton = new Button
            {
                Text = "选择",
                Location = new Point(480, simpleY - 5),
                Size = new Size(60, 30)
            };
            _selectFirmwareButton.Click += SelectFirmwareButton_Click;
            _simpleGroupBox.Controls.Add(_selectFirmwareButton);

            _upgradeSimpleButton = new Button
            {
                Text = "执行升级",
                Location = new Point(560, simpleY - 5),
                Size = new Size(100, 30),
                Enabled = false,
                BackColor = Color.LightGreen,
                Font = new Font("Microsoft YaHei", 9, FontStyle.Bold)
            };
            _upgradeSimpleButton.Click += UpgradeSimpleButton_Click;
            _simpleGroupBox.Controls.Add(_upgradeSimpleButton);

            yPos += 140;

            // === 原有功能区域（完全保持不变） ===
            var originalLabel = new Label
            {
                Text = "原有模式 - 使用JSON配置文件（保持原有功能不变）",
                Location = new Point(20, yPos),
                Size = new Size(400, 20),
                ForeColor = Color.DarkBlue,
                Font = new Font("Microsoft YaHei", 9, FontStyle.Bold)
            };
            this.Controls.Add(originalLabel);

            yPos += 30;

            // 原有的加载配置按钮（完全不变）
            _loadConfigButton = new Button
            {
                Text = "加载配置",
                Location = new Point(20, yPos),
                Size = new Size(100, 30)
            };
            _loadConfigButton.Click += LoadConfigButton_Click; // 保持原有方法
            this.Controls.Add(_loadConfigButton);

            // 原有的升级按钮（完全不变）
            _upgradeButton = new Button
            {
                Text = "执行升级",
                Location = new Point(140, yPos),
                Size = new Size(100, 30),
                Enabled = false
            };
            _upgradeButton.Click += UpgradeButton_Click; // 保持原有方法
            this.Controls.Add(_upgradeButton);

            // 停止按钮
            _stopButton = new Button
            {
                Text = "停止升级",
                Location = new Point(260, yPos),
                Size = new Size(100, 30),
                Enabled = false,
                BackColor = Color.Red,
                ForeColor = Color.White
            };
            _stopButton.Click += StopButton_Click;
            this.Controls.Add(_stopButton);

            // 关于按钮
            var aboutButton = new Button
            {
                Text = "关于",
                Location = new Point(380, yPos),
                Size = new Size(60, 30)
            };
            aboutButton.Click += (s, e) => ShowAboutDialog();
            this.Controls.Add(aboutButton);

            yPos += 50;

            // === 以下所有控件保持原有设计不变 ===

            // 配置信息显示
            var configLabel = new Label
            {
                Text = "配置信息:",
                Location = new Point(20, yPos),
                Size = new Size(80, 20)
            };
            this.Controls.Add(configLabel);

            _configTextBox = new RichTextBox
            {
                Location = new Point(20, yPos + 25),
                Size = new Size(840, 150), // 增加宽度
                ReadOnly = true,
                Font = new Font("Consolas", 9)
            };
            this.Controls.Add(_configTextBox);

            yPos += 185;

            // 运行日志显示
            var logLabel = new Label
            {
                Text = "运行日志:",
                Location = new Point(20, yPos),
                Size = new Size(80, 20)
            };
            this.Controls.Add(logLabel);

            _logTextBox = new RichTextBox
            {
                Location = new Point(20, yPos + 25),
                Size = new Size(840, 200), // 增加宽度
                ReadOnly = true,
                Font = new Font("Consolas", 9)
            };
            this.Controls.Add(_logTextBox);

            yPos += 235;

            // 进度条
            _progressBar = new ProgressBar
            {
                Location = new Point(20, yPos),
                Size = new Size(840, 25), // 增加宽度
                Style = ProgressBarStyle.Continuous
            };
            this.Controls.Add(_progressBar);

            // 状态标签
            _statusLabel = new Label
            {
                Text = "就绪 - 可使用简化模式或加载JSON配置文件",
                Location = new Point(20, yPos + 35),
                Size = new Size(400, 20),
                ForeColor = Color.Blue
            };
            this.Controls.Add(_statusLabel);
        }

        // === 新增：简化模式相关方法 ===

        private void ChipTypeComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            var chipType = _chipTypeComboBox.SelectedItem?.ToString();

            if (chipType == "5300")
            {
                // 5300只有LLC，禁用PFC选项
                _targetComboBox.SelectedIndex = 0; // LLC
                _targetComboBox.Enabled = false;
                _baudrateUpDown.Value = 115200;
            }
            else if (chipType == "5800")
            {
                // 5800支持LLC和PFC
                _targetComboBox.Enabled = true;
                _baudrateUpDown.Value = 115200;
            }
            else
            {
                _targetComboBox.Enabled = true;
            }

            UpdateModbusAddr();
            UpdateSimpleConfigDisplay();
        }

        private void TargetComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            UpdateModbusAddr();
            UpdateSimpleConfigDisplay();
        }

        private void UpdateModbusAddr()
        {
            var chipType = _chipTypeComboBox.SelectedItem?.ToString();
            var target = _targetComboBox.SelectedItem?.ToString();

            if (chipType == "5300")
            {
                _modbusAddrUpDown.Value = 2; // 5300只有LLC
            }
            else if (chipType == "5800")
            {
                _modbusAddrUpDown.Value = (target == "PFC") ? 1 : 2;
            }
        }

        private void SelectFirmwareButton_Click(object sender, EventArgs e)
        {
            using (var openFileDialog = new OpenFileDialog())
            {
                openFileDialog.Filter = "固件文件 (*.bin;*.hex)|*.bin;*.hex|BIN文件 (*.bin)|*.bin|HEX文件 (*.hex)|*.hex|所有文件 (*.*)|*.*";
                openFileDialog.Title = "选择固件文件";
                openFileDialog.CheckFileExists = true;

                if (openFileDialog.ShowDialog() == DialogResult.OK)
                {
                    _firmwareFileTextBox.Text = openFileDialog.FileName;

                    var fileInfo = new FileInfo(openFileDialog.FileName);
                    LogMessage(LogLevel.Info, $"已选择固件文件: {fileInfo.Name} ({fileInfo.Length / 1024.0:F1} KB)");

                    UpdateSimpleConfigDisplay();
                    CheckSimpleUpgradeReady();
                }
            }
        }

        private void UpdateSimpleConfigDisplay()
        {
            if (_chipTypeComboBox.SelectedItem == null) return;

            var chipType = _chipTypeComboBox.SelectedItem.ToString();

            // 显示简化模式的配置信息
            _configTextBox.Clear();

            var configInfo = new[]
            {
                "=== 简化模式配置 ===",
                $"芯片类型: {chipType}",
                $"通信波特率: {_baudrateUpDown.Value}",
                $"Modbus地址: {_modbusAddrUpDown.Value}",
                $"固件文件: {(_firmwareFileTextBox.Text == "" ? "未选择" : Path.GetFileName(_firmwareFileTextBox.Text))}",
                "",
                chipType == "自定义" ? "注意: 自定义模式请使用'加载配置'功能" :
                    $"使用{chipType}芯片的标准内存布局"
            };

            foreach (var info in configInfo)
            {
                var color = info.StartsWith("===") ? Color.DarkGreen :
                           info.Contains("注意") ? Color.Red : Color.Black;
                AppendColoredText(_configTextBox, info + Environment.NewLine, color);
            }
        }

        private void CheckSimpleUpgradeReady()
        {
            bool canUpgrade = !string.IsNullOrEmpty(_firmwareFileTextBox.Text) &&
                            File.Exists(_firmwareFileTextBox.Text) &&
                            _chipTypeComboBox.SelectedItem?.ToString() != "自定义" &&
                            !_isUpgrading;

            _upgradeSimpleButton.Enabled = canUpgrade;

            if (canUpgrade)
            {
                _statusLabel.Text = "简化模式配置完成，可以开始升级";
                _statusLabel.ForeColor = Color.Green;
            }
            else if (_chipTypeComboBox.SelectedItem?.ToString() == "自定义")
            {
                _statusLabel.Text = "自定义配置请使用'加载配置'功能";
                _statusLabel.ForeColor = Color.Orange;
            }
            else if (string.IsNullOrEmpty(_firmwareFileTextBox.Text))
            {
                _statusLabel.Text = "请选择固件文件";
                _statusLabel.ForeColor = Color.Orange;
            }
        }

        private async void UpgradeSimpleButton_Click(object sender, EventArgs e)
        {
            try
            {
                _isSimpleMode = true;

                // 根据选择的芯片类型生成配置
                var chipType = _chipTypeComboBox.SelectedItem?.ToString();

                if (chipType == "5300")
                {
                    // 使用您原有的5300配置作为模板
                    _currentConfig = new IapConfig
                    {
                        ModbusAddr = (byte)_modbusAddrUpDown.Value,
                        AppFile = _firmwareFileTextBox.Text,
                        Baudrate = (int)_baudrateUpDown.Value,
                        FlashBaseAddr = 134217728,    // 0x08000000
                        FlashMaxSize = 76800,         // 您原有的配置
                        AppBaseAddr = 134248448,      // 您原有的配置 0x08028000
                        AppMaxSize = 46080,           // 您原有的配置
                        ArgBaseAddr = 134247968,      // 您原有的配置 0x08027E00
                        ReadWriteSize = 240,          // 您原有的配置
                        EnableResume = false,
                        LastWrittenAddr = 0,
                        LastWrittenProgress = 0
                    };
                }
                else if (chipType == "5800")
                {
                    // 5800芯片（LLC和PFC地址布局相同，通过Modbus地址区分）
                    _currentConfig = new IapConfig
                    {
                        ModbusAddr = (byte)_modbusAddrUpDown.Value,
                        AppFile = _firmwareFileTextBox.Text,
                        Baudrate = (int)_baudrateUpDown.Value,
                        FlashBaseAddr = 134217728,    // 0x08000000
                        FlashMaxSize = 262144,        // 256KB
                        AppBaseAddr = 134250496,      // 0x08008000 (32KB偏移)
                        AppMaxSize = 229376,          // ~224KB
                        ArgBaseAddr = 134246400,      // 0x08007000
                        ReadWriteSize = 128,
                        EnableResume = false,
                        LastWrittenAddr = 0,
                        LastWrittenProgress = 0
                    };
                }
                else
                {
                    LogMessage(LogLevel.Error, "请选择有效的芯片类型");
                    return;
                }

                // 使用原有的升级逻辑
                await ExecuteUpgrade();
            }
            catch (Exception ex)
            {
                LogMessage(LogLevel.Error, $"简化模式升级失败: {ex.Message}");
                ResetUpgradeState();
            }
        }

        // === 原有方法完全保持不变 ===

        private async void LoadConfigButton_Click(object sender, EventArgs e)
        {
            try
            {
                _isSimpleMode = false;

                using (var openFileDialog = new OpenFileDialog())
                {
                    openFileDialog.Filter = "JSON配置文件 (*.json)|*.json|所有文件 (*.*)|*.*";
                    openFileDialog.Title = "选择配置文件";

                    if (openFileDialog.ShowDialog() == DialogResult.OK)
                    {
                        if (await LoadConfigAsync(openFileDialog.FileName))
                        {
                            _upgradeButton.Enabled = true;
                            _statusLabel.Text = "配置已加载，可以开始升级";
                            _statusLabel.ForeColor = Color.Green;
                            LogMessage(LogLevel.Info, "配置文件加载成功，可以开始升级");
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                LogMessage(LogLevel.Error, $"加载配置文件时发生异常: {ex.Message}");
            }
        }

        private async Task<bool> LoadConfigAsync(string configPath)
        {
            try
            {
                if (!File.Exists(configPath))
                {
                    LogMessage(LogLevel.Error, $"配置文件不存在: {configPath}");
                    return false;
                }

                var json = await File.ReadAllTextAsync(configPath);
                _currentConfig = JsonConvert.DeserializeObject<IapConfig>(json);

                if (_currentConfig == null)
                {
                    LogMessage(LogLevel.Error, "配置文件格式错误，无法解析JSON");
                    return false;
                }

                // 先验证其他配置参数（不包括文件存在性）
                if (!ValidateConfig(_currentConfig))
                    return false;

                // 单独检查APP文件是否存在
                if (!File.Exists(_currentConfig.AppFile))
                {
                    LogMessage(LogLevel.Error, $"APP文件不存在: {_currentConfig.AppFile}");
                    LogMessage(LogLevel.Info, "请选择正确的APP固件文件...");

                    using (var appFileDialog = new OpenFileDialog())
                    {
                        appFileDialog.Filter = "固件文件 (*.bin;*.hex)|*.bin;*.hex|所有文件 (*.*)|*.*";
                        appFileDialog.Title = "选择APP固件文件";

                        try
                        {
                            var configDir = Path.GetDirectoryName(configPath);
                            if (Directory.Exists(configDir))
                                appFileDialog.InitialDirectory = configDir;
                        }
                        catch { }

                        if (appFileDialog.ShowDialog() == DialogResult.OK)
                        {
                            _currentConfig.AppFile = appFileDialog.FileName;
                            LogMessage(LogLevel.Info, $"已选择APP文件: {_currentConfig.AppFile}");

                            if (!File.Exists(_currentConfig.AppFile))
                            {
                                LogMessage(LogLevel.Error, "选择的文件不存在");
                                return false;
                            }
                        }
                        else
                        {
                            LogMessage(LogLevel.Error, "未选择APP文件，加载配置失败");
                            return false;
                        }
                    }
                }

                // 显示配置信息
                DisplayConfigInfo(_currentConfig);

                // 设置IAP配置
                if (!_iapMaster.LoadConfig(_currentConfig, configPath))
                {
                    LogMessage(LogLevel.Error, "将配置传递给IapMaster失败");
                    return false;
                }

                LogMessage(LogLevel.Info, "配置已成功传递给IapMaster");
                return true;
            }
            catch (Newtonsoft.Json.JsonException ex)
            {
                LogMessage(LogLevel.Error, $"JSON格式错误: {ex.Message}");
                return false;
            }
            catch (Exception ex)
            {
                LogMessage(LogLevel.Error, $"加载配置失败: {ex.Message}");
                return false;
            }
        }

        private bool ValidateConfig(IapConfig config)
        {
            var errors = new List<string>();

            if (config.AppBaseAddr % 4 != 0)
                errors.Add("APP基址必须4字节对齐");

            if (config.ReadWriteSize % 4 != 0)
                errors.Add("读写块大小必须4字节对齐");

            if (config.Baudrate != 9600 && config.Baudrate != 19200 &&
                config.Baudrate != 38400 && config.Baudrate != 57600 &&
                config.Baudrate != 115200)
                errors.Add("建议使用标准波特率");

            if (config.ModbusAddr == 0)
                errors.Add("Modbus地址不能为0");

            if (string.IsNullOrWhiteSpace(config.AppFile))
                errors.Add("APP文件路径不能为空");

            if (config.Baudrate <= 0)
                errors.Add("波特率必须大于0");

            if (config.FlashMaxSize == 0)
                errors.Add("Flash容量不能为0");

            if (config.AppMaxSize == 0)
                errors.Add("APP区域容量不能为0");

            if (config.ReadWriteSize <= 0 || config.ReadWriteSize > 1024)
                errors.Add("读写大小必须在1-1024字节之间");

            if (config.AppBaseAddr < config.FlashBaseAddr)
                errors.Add("APP基址不能小于Flash基址");

            if (config.AppBaseAddr + config.AppMaxSize > config.FlashBaseAddr + config.FlashMaxSize)
                errors.Add("APP区域超出Flash范围");

            if (errors.Any())
            {
                foreach (var error in errors)
                    LogMessage(LogLevel.Error, $"配置验证失败: {error}");
                return false;
            }

            return true;
        }

        private void DisplayConfigInfo(IapConfig config)
        {
            _configTextBox.Clear();

            var configInfo = new[]
            {
                "=== JSON配置文件模式 ===",
                $"传输波特: {config.Baudrate}",
                $"APP文件: {config.AppFile}",
                $"Modbus地址: 0x{config.ModbusAddr:X2}",
                $"Flash基址: 0x{config.FlashBaseAddr:X8}, 容量: {config.FlashMaxSize / 1024} KB",
                $"APP基址: 0x{config.AppBaseAddr:X8}, 容量: {config.AppMaxSize / 1024} KB",
                $"参数基址: 0x{config.ArgBaseAddr:X8}",
                $"读写块大小: {config.ReadWriteSize} 字节",
                "升级模式: 每次重新开始（已禁用断点续传)"
            };

            foreach (var info in configInfo)
            {
                AppendColoredText(_configTextBox, info + Environment.NewLine, Color.Black);
            }
        }

        private async void UpgradeButton_Click(object sender, EventArgs e)
        {
            _isSimpleMode = false;
            await ExecuteUpgrade();
        }

        // === 通用升级执行方法 ===
        private async Task ExecuteUpgrade()
        {
            if (_currentConfig == null)
            {
                LogMessage(LogLevel.Error, "请先配置参数");
                return;
            }

            var selectedDisplayText = _portComboBox.Text?.Trim();

            // ===== 新增：检查是否为CH340设备 =====
            if (selectedDisplayText == "未检测到CH340设备" || string.IsNullOrEmpty(selectedDisplayText))
            {
                LogMessage(LogLevel.Error, "请连接CH340串口设备");
                MessageBox.Show("未检测到CH340串口设备，请检查设备连接", "提示",
                    MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }
            // =====================================

            var portName = ExtractPortName(selectedDisplayText);

            if (string.IsNullOrEmpty(portName))
            {
                LogMessage(LogLevel.Error, "请选择串口");
                return;
            }

            var modeText = _isSimpleMode ? "简化模式" : "JSON配置模式";
            LogMessage(LogLevel.Info, $"使用{modeText}开始升级 - 串口: {portName}");

            try
            {
                _isUpgrading = true;

                // 禁用所有相关按钮
                _upgradeButton.Enabled = false;
                _upgradeSimpleButton.Enabled = false;
                _loadConfigButton.Enabled = false;
                _stopButton.Enabled = true;
                _portComboBox.Enabled = false;
                _selectFirmwareButton.Enabled = false;
                _chipTypeComboBox.Enabled = false;

                _progressBar.Value = 0;
                _statusLabel.Text = "升级进行中...";
                _statusLabel.ForeColor = Color.Orange;
                _lastOperationTime = DateTime.Now;

                _uartCloseTimer.Stop();

                // 如果是简化模式，需要加载配置到IapMaster
                if (_isSimpleMode)
                {
                    if (!_iapMaster.LoadConfig(_currentConfig))
                    {
                        LogMessage(LogLevel.Error, "加载简化配置到IAP模块失败");
                        ResetUpgradeState();
                        return;
                    }
                }

                // 强制关闭现有连接
                if (_serialPort != null)
                {
                    try
                    {
                        if (_serialPort.IsOpen)
                            _serialPort.Close();
                        _serialPort.Dispose();
                    }
                    catch { }
                    _serialPort = null;
                    await Task.Delay(1000);
                }

                // 创建新的串口连接
                _serialPort = new SerialPort(portName, _currentConfig.Baudrate)
                {
                    DataBits = 8,
                    StopBits = StopBits.One,
                    Parity = Parity.None,
                    ReadTimeout = 3000,
                    WriteTimeout = 3000,
                    DtrEnable = true,
                    RtsEnable = true
                };

                LogMessage(LogLevel.Info, $"正在打开串口 {portName}...");
                _serialPort.Open();

                if (!_serialPort.IsOpen)
                {
                    LogMessage(LogLevel.Error, $"无法打开串口 {portName}");
                    ResetUpgradeState();
                    return;
                }

                LogMessage(LogLevel.Info, $"串口 {portName} 打开成功");

                _logTextBox.Clear();
                await Task.Delay(1000);

                LogMessage(LogLevel.Info, "开始IAP升级流程...");
                var success = await _iapMaster.StartUpgradeAsync(_serialPort);

                _lastOperationTime = DateTime.Now;

                if (!success)
                {
                    LogMessage(LogLevel.Error, "IAP升级失败");
                    _statusLabel.Text = "升级失败";
                    _statusLabel.ForeColor = Color.Red;
                }
            }
            catch (Exception ex)
            {
                LogMessage(LogLevel.Error, $"升级过程发生错误: {ex.Message}");
                _statusLabel.Text = "升级错误";
                _statusLabel.ForeColor = Color.Red;
                ResetUpgradeState();
            }
        }

        // === 以下所有方法保持原有逻辑不变 ===

        private void ResetUpgradeState()
        {
            _isUpgrading = false;

            // 根据模式恢复按钮状态
            if (_isSimpleMode)
            {
                _upgradeSimpleButton.Enabled = !string.IsNullOrEmpty(_firmwareFileTextBox.Text) &&
                                             File.Exists(_firmwareFileTextBox.Text) &&
                                             _chipTypeComboBox.SelectedItem?.ToString() != "自定义";
            }
            else
            {
                _upgradeButton.Enabled = _currentConfig != null;
            }

            _loadConfigButton.Enabled = true;
            _stopButton.Enabled = false;
            _portComboBox.Enabled = true;
            _selectFirmwareButton.Enabled = true;
            _chipTypeComboBox.Enabled = true;
            _lastOperationTime = DateTime.Now;

            if (!_isUpgrading && !_uartCloseTimer.Enabled)
            {
                _uartCloseTimer.Start();
            }
        }

        private string GetVersionString()
        {
            var assembly = Assembly.GetExecutingAssembly();
            var version = assembly.GetName().Version;
            return version != null ? $"v{version.Major}.{version.Minor}.{version.Build}" : "v1.0.0";
        }

        private void ShowAboutDialog()
        {
            var assembly = Assembly.GetExecutingAssembly();
            var title = assembly.GetCustomAttribute<AssemblyTitleAttribute>()?.Title ?? "IAP升级工具";
            var version = assembly.GetName().Version?.ToString() ?? "1.0.0.0";
            var company = assembly.GetCustomAttribute<AssemblyCompanyAttribute>()?.Company ?? "";
            var copyright = assembly.GetCustomAttribute<AssemblyCopyrightAttribute>()?.Copyright ?? "";

            var aboutText = $"{title}\n" +
                           $"版本: {version}\n" +
                           $"{company}\n" +
                           $"{copyright}\n\n" +
                           $"专业的IAP固件升级工具\n" +
                           $"支持5300/5800芯片和Modbus协议\n\n" +
                           $"功能特点:\n" +
                           $"• 简化模式：无需JSON配置文件\n" +
                           $"• 兼容模式：完全支持原有JSON配置\n" +
                           $"• 支持.bin和.hex固件文件\n" +
                           $"• 智能串口重连机制\n" +
                           $"• 实时升级进度显示\n" +
                           $"• 完善的错误处理机制";

            MessageBox.Show(aboutText, "关于", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        private void InitializeIapMaster()
        {
            _iapMaster = new IapMaster();
            _iapMaster.MessageReceived += OnIapMessageReceived;
            _iapMaster.ProgressUpdated += OnIapProgressUpdated;
            _iapMaster.UpgradeCompleted += OnIapUpgradeCompleted;
        }

        private void InitializeTimers()
        {
            _portUpdateTimer = new System.Windows.Forms.Timer { Interval = 2000 };
            _portUpdateTimer.Tick += (s, e) =>
            {
                if (!_isUpgrading)
                {
                    UpdatePortList();
                }
            };
            _portUpdateTimer.Start();

            _uartCloseTimer = new System.Windows.Forms.Timer { Interval = 1000 };
            _uartCloseTimer.Tick += UartCloseTimer_Tick;
            _uartCloseTimer.Start();
        }

        private Dictionary<string, string> GetSerialPortDescriptions()
        {
            var portDescriptions = new Dictionary<string, string>();

            try
            {
                using (var searcher = new ManagementObjectSearcher("SELECT * FROM Win32_PnPEntity WHERE Caption like '%(COM%'"))
                {
                    foreach (ManagementObject obj in searcher.Get())
                    {
                        var caption = obj["Caption"]?.ToString();
                        if (!string.IsNullOrEmpty(caption))
                        {
                            // ===== 新增：只处理CH340设备 =====
                            if (!caption.Contains("CH340"))
                                continue;
                            // ===================================

                            var startIndex = caption.LastIndexOf("(COM");
                            var endIndex = caption.LastIndexOf(")");

                            if (startIndex != -1 && endIndex != -1)
                            {
                                var portName = caption.Substring(startIndex + 1, endIndex - startIndex - 1);
                                var description = caption.Substring(0, startIndex).Trim();

                                // 简化描述为CH340
                                description = "CH340";

                                portDescriptions[portName] = description;
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                LogMessage(LogLevel.Error, $"获取串口设备信息失败: {ex.Message}");
            }

            return portDescriptions;
        }

        private void UpdatePortList()
        {
            try
            {
                var availablePorts = SerialPort.GetPortNames().OrderBy(p => p).ToArray();
                var portDescriptions = GetSerialPortDescriptions();

                // ===== 新增：只保留CH340设备 =====
                var ch340Ports = availablePorts
                    .Where(port => portDescriptions.ContainsKey(port))
                    .ToArray();
                // ===================================

                var displayItems = ch340Ports.Select(port =>
                    $"{port} - {portDescriptions[port]}"
                ).ToArray();

                var currentItems = _portComboBox.Items.Cast<string>().ToArray();

                if (!displayItems.SequenceEqual(currentItems))
                {
                    var selectedPort = ExtractPortName(_portComboBox.Text);
                    _portComboBox.Items.Clear();

                    // ===== 新增：如果没有CH340设备，显示提示 =====
                    if (displayItems.Length == 0)
                    {
                        _portComboBox.Items.Add("未检测到CH340设备");
                        _portComboBox.SelectedIndex = 0;
                        _portComboBox.Enabled = false;
                    }
                    else
                    {
                        _portComboBox.Items.AddRange(displayItems);
                        _portComboBox.Enabled = true;

                        var matchingItem = displayItems.FirstOrDefault(item => item.StartsWith(selectedPort));
                        if (matchingItem != null)
                            _portComboBox.Text = matchingItem;
                        else
                            _portComboBox.SelectedIndex = 0;
                    }
                    // =============================================
                }
            }
            catch (Exception ex)
            {
                LogMessage(LogLevel.Error, $"更新端口列表失败: {ex.Message}");
            }
        }

        private string ExtractPortName(string displayText)
        {
            if (string.IsNullOrEmpty(displayText))
                return string.Empty;

            var dashIndex = displayText.IndexOf(" - ");
            return dashIndex > 0 ? displayText.Substring(0, dashIndex) : displayText;
        }

        private void StopButton_Click(object sender, EventArgs e)
        {
            if (_isUpgrading)
            {
                LogMessage(LogLevel.Info, "用户请求停止升级...");
                _iapMaster.StopUpgrade();

                ResetUpgradeState();
                _statusLabel.Text = "升级已停止";
                _statusLabel.ForeColor = Color.Red;
            }
        }

        private void UartCloseTimer_Tick(object sender, EventArgs e)
        {
            try
            {
                if (!_isUpgrading &&
                    (_upgradeButton.Enabled || _upgradeSimpleButton.Enabled) &&
                    _serialPort?.IsOpen == true)
                {
                    var timeSinceLastOperation = DateTime.Now.Subtract(_lastOperationTime);
                    if (timeSinceLastOperation.TotalSeconds > 5)
                    {
                        try
                        {
                            _serialPort.Close();
                            _serialPort?.Dispose();
                            _serialPort = null;
                            LogMessage(LogLevel.Debug, "串口已安全关闭");
                        }
                        catch (Exception ex)
                        {
                            LogMessage(LogLevel.Error, $"关闭串口时发生错误: {ex.Message}");
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                LogMessage(LogLevel.Error, $"定时器错误: {ex.Message}");
            }
        }

        private void OnIapMessageReceived(bool isSuccess, string message)
        {
            if (InvokeRequired)
            {
                Invoke(new Action<bool, string>(OnIapMessageReceived), isSuccess, message);
                return;
            }

            LogMessage(isSuccess ? LogLevel.Info : LogLevel.Error, message);
        }

        private void OnIapProgressUpdated(int progress)
        {
            if (InvokeRequired)
            {
                Invoke(new Action<int>(OnIapProgressUpdated), progress);
                return;
            }

            _progressBar.Value = Math.Max(0, Math.Min(100, progress));

            var now = DateTime.Now;
            if (_currentConfig != null)
            {
                var timeDiff = (now - _lastProgressTime).TotalSeconds;
                if (timeDiff >= 1.0)
                {
                    var estimatedBytes = (_currentConfig.AppMaxSize * progress) / 100;
                    var bytesDiff = estimatedBytes - _lastProgressBytes;
                    var speed = bytesDiff / timeDiff;

                    _statusLabel.Text = $"升级进行中... {progress}% (速度: {speed:F0} B/s)";

                    _lastProgressTime = now;
                    _lastProgressBytes = (int)estimatedBytes;
                }
            }
        }

        private void OnIapUpgradeCompleted(bool success)
        {
            if (InvokeRequired)
            {
                Invoke(new Action<bool>(OnIapUpgradeCompleted), success);
                return;
            }

            ResetUpgradeState();

            if (success)
            {
                _progressBar.Value = 100;
                _statusLabel.Text = "升级成功完成";
                _statusLabel.ForeColor = Color.Green;
                LogMessage(LogLevel.Info, "=== IAP升级成功完成 ===");
            }
            else
            {
                _progressBar.Value = 0;
                _statusLabel.Text = "升级失败";
                _statusLabel.ForeColor = Color.Red;
                LogMessage(LogLevel.Error, "=== IAP升级失败 ===");
            }
        }

        private void LogMessage(LogLevel level, string message)
        {
            var timestamp = DateTime.Now.ToString("HH:mm:ss.fff");
            var levelText = level.ToString().ToUpper();
            var logMessage = $"[{timestamp}] [{levelText}] {message}{Environment.NewLine}";

            var color = level switch
            {
                LogLevel.Info => Color.Blue,
                LogLevel.Warning => Color.Orange,
                LogLevel.Error => Color.Red,
                LogLevel.Debug => Color.Gray,
                _ => Color.Black
            };

            AppendColoredText(_logTextBox, logMessage, color);

            if (_logTextBox.Lines.Length > 1000)
            {
                var lines = _logTextBox.Lines.Skip(500).ToArray();
                _logTextBox.Lines = lines;
            }
        }

        private void AppendColoredText(RichTextBox rtb, string text, Color color)
        {
            rtb.SelectionStart = rtb.TextLength;
            rtb.SelectionLength = 0;
            rtb.SelectionColor = color;
            rtb.AppendText(text);
            rtb.SelectionColor = rtb.ForeColor;
            rtb.ScrollToCaret();
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            try
            {
                if (_isUpgrading)
                {
                    var result = MessageBox.Show("IAP升级正在进行中，确定要强制关闭程序吗？", "警告",
                        MessageBoxButtons.YesNo, MessageBoxIcon.Warning);

                    if (result == DialogResult.No)
                    {
                        e.Cancel = true;

                        return;
                    }

                    _iapMaster.StopUpgrade();
                }

                _portUpdateTimer?.Stop();
                _portUpdateTimer?.Dispose();

                _uartCloseTimer?.Stop();
                _uartCloseTimer?.Dispose();

                try
                {
                    if (_serialPort?.IsOpen == true)
                    {
                        _serialPort.Close();
                    }
                    _serialPort?.Dispose();
                }
                catch { }
            }
            catch (Exception ex)
            {
                LogMessage(LogLevel.Error, $"关闭程序时发生错误: {ex.Message}");
            }

            base.OnFormClosing(e);
        }
    }
}