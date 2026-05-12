// IapMaster.cs - 基于原版优化的持续重连版本
using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Threading;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace IapUpgradeTool
{
    public class IapMaster
    {
        private IapConfig _config;
        private ModbusMaster _modbusMaster;
        private SerialPort _serialPort;
        private byte[] _appData;
        private int _appSize;
        private uint _appCrc;
        private bool _isWorking;
        private bool _ackReceived;
        private byte[] _ackData;
        private bool _stopRequested = false;
        private readonly ManualResetEventSlim _ackEvent = new ManualResetEventSlim(false);

        // 串口配置信息 - 用于重新创建串口对象
        private string _portName;
        private int _baudRate;
        private Parity _parity;
        private int _dataBits;
        private StopBits _stopBits;

        // 持续重连配置
        private const int MAX_MAIN_RETRIES = 999;
        private const int MAIN_RETRY_DELAY_MS = 5000;

        // 续传相关
        private string _configFilePath;
        private uint _lastSuccessAddr = 0;
        private int _lastSuccessProgress = 0;

        // 统计信息
        private int _totalMainRetries = 0;

        public event Action<bool, string> MessageReceived;
        public event Action<int> ProgressUpdated;
        public event Action<bool> UpgradeCompleted;

        public IapMaster()
        {
            _modbusMaster = new ModbusMaster();
            _modbusMaster.DataReceived += OnDataReceived;
            _modbusMaster.ErrorOccurred += OnErrorOccurred;
        }

        public void StopUpgrade()
        {
            _stopRequested = true;
            _modbusMaster?.StopReceiving();
            SendMessage(true, "正在停止升级任务...");
        }

        public bool LoadConfig(IapConfig config, string configPath = null)
        {
            try
            {
                if (config == null)
                {
                    SendMessage(false, "配置对象为空");
                    return false;
                }

                _config = config;
                _configFilePath = configPath;

                // 启用断点续传
                _config.EnableResume = true;
                CheckResumeState();

                SendMessage(true, $"成功加载配置 - ModbusAddr: {_config.ModbusAddr}, AppFile: {_config.AppFile}");
                SendMessage(true, "持续重连模式已启用，支持断点续传");

                return true;
            }
            catch (Exception ex)
            {
                SendMessage(false, $"加载配置失败: {ex.Message}");
                return false;
            }
        }

        public async Task<bool> StartUpgradeAsync(SerialPort serialPort)
        {
            if (_isWorking)
            {
                SendMessage(false, "IAP任务正在运行中");
                return false;
            }

            if (_config == null)
            {
                SendMessage(false, "请先加载配置文件");
                return false;
            }

            // 保存串口配置信息
            _portName = serialPort.PortName;
            _baudRate = serialPort.BaudRate;
            _parity = serialPort.Parity;
            _dataBits = serialPort.DataBits;
            _stopBits = serialPort.StopBits;

            _serialPort = serialPort;
            _isWorking = true;
            _stopRequested = false;
            _totalMainRetries = 0;

            SendMessage(true, "开始IAP升级任务 - 持续重连模式");
            SendMessage(true, $"串口: {_portName}, 波特率: {_baudRate}");

            var startTime = DateTime.Now;
            bool upgradeSuccess = false;

            // 主重试循环
            while (!upgradeSuccess && !_stopRequested && _totalMainRetries < MAX_MAIN_RETRIES)
            {
                _totalMainRetries++;

                try
                {
                    SendMessage(true, $"=== 第 {_totalMainRetries} 次升级尝试 ===");

                    // 使用原版的升级逻辑，但加入串口重建功能
                    upgradeSuccess = await ExecuteOriginalUpgradeLogicAsync();

                    if (upgradeSuccess)
                    {
                        SendMessage(true, $"=== 第 {_totalMainRetries} 次尝试成功 ===");
                        break;
                    }
                    else if (!_stopRequested)
                    {
                        SendMessage(false, $"第 {_totalMainRetries} 次尝试失败，{MAIN_RETRY_DELAY_MS / 1000}秒后重试");
                        SaveResumeState();
                        await Task.Delay(MAIN_RETRY_DELAY_MS);
                    }
                }
                catch (Exception ex)
                {
                    SendMessage(false, $"第 {_totalMainRetries} 次尝试异常: {ex.Message}");
                    SaveResumeState();
                    if (!_stopRequested)
                    {
                        await Task.Delay(MAIN_RETRY_DELAY_MS);
                    }
                }
            }

            // 处理结果
            try
            {
                if (_stopRequested)
                {
                    SendMessage(false, "升级已被用户停止");
                    SaveResumeState();
                    UpgradeCompleted?.Invoke(false);
                    return false;
                }

                if (upgradeSuccess)
                {
                    ClearResumeState();
                    var duration = DateTime.Now - startTime;
                    SendMessage(true, $"IAP升级成功完成，总尝试次数: {_totalMainRetries}，耗时: {duration.TotalSeconds:F1} 秒");
                    UpgradeCompleted?.Invoke(true);
                    return true;
                }
                else
                {
                    SendMessage(false, $"经过 {_totalMainRetries} 次尝试仍未成功");
                    SaveResumeState();
                    UpgradeCompleted?.Invoke(false);
                    return false;
                }
            }
            finally
            {
                _modbusMaster?.StopReceiving();
                CleanupSerialPort();
                _isWorking = false;
                _stopRequested = false;
            }
        }

        /// <summary>
        /// 执行原版的升级逻辑，保持兼容性
        /// </summary>
        private async Task<bool> ExecuteOriginalUpgradeLogicAsync()
        {
            try
            {
                // 确保串口连接正常
                if (!await EnsureSerialPortAsync())
                {
                    return false;
                }

                // 启动Modbus接收
                _modbusMaster.StartReceiving(_serialPort);
                await Task.Delay(500);

                // 执行原版升级步骤
                if (_stopRequested || !await LoadAppFileAsync()) return false;
                if (_stopRequested || !await EnterIapModeAsync()) return false;
                if (_stopRequested || !await WriteFlashAsync()) return false;
                if (_stopRequested || !await WriteChecksumAsync()) return false;
                if (_stopRequested || !await ExitIapModeAsync()) return false;

                return true;
            }
            catch (Exception ex)
            {
                SendMessage(false, $"升级逻辑执行异常: {ex.Message}");
                return false;
            }
        }

        /// <summary>
        /// 确保串口连接 - 关键修复：重新创建串口对象
        /// </summary>
        private async Task<bool> EnsureSerialPortAsync()
        {
            try
            {
                // 检查串口状态
                bool needReconnect = false;

                if (_serialPort == null)
                {
                    needReconnect = true;
                    SendMessage(true, "串口对象为空，需要重新创建");
                }
                else
                {
                    try
                    {
                        // 测试串口是否有效
                        var _ = _serialPort.IsOpen;
                        var __ = _serialPort.PortName;

                        if (!_serialPort.IsOpen)
                        {
                            needReconnect = true;
                            SendMessage(true, "串口未打开，需要重新创建");
                        }
                    }
                    catch
                    {
                        needReconnect = true;
                        SendMessage(true, "串口对象失效，需要重新创建");
                    }
                }

                if (needReconnect)
                {
                    return await RecreateSerialPortAsync();
                }

                return true;
            }
            catch (Exception ex)
            {
                SendMessage(false, $"检查串口状态异常: {ex.Message}");
                return false;
            }
        }

        /// <summary>
        /// 重新创建串口对象 - 核心修复
        /// </summary>
        private async Task<bool> RecreateSerialPortAsync()
        {
            try
            {
                SendMessage(true, "正在重新创建串口对象...");

                // 清理旧的串口对象
                CleanupSerialPort();
                await Task.Delay(2000); // 等待系统释放资源

                // 创建全新的串口对象
                _serialPort = new SerialPort()
                {
                    PortName = _portName,
                    BaudRate = _baudRate,
                    Parity = _parity,
                    DataBits = _dataBits,
                    StopBits = _stopBits,
                    Handshake = Handshake.None,
                    WriteTimeout = 5000,
                    ReadTimeout = 5000
                };

                // 尝试打开串口
                for (int retry = 0; retry < 3; retry++)
                {
                    try
                    {
                        _serialPort.Open();
                        await Task.Delay(1000);

                        // 配置串口参数
                        _serialPort.DtrEnable = true;
                        _serialPort.RtsEnable = true;
                        _serialPort.DiscardInBuffer();
                        _serialPort.DiscardOutBuffer();

                        SendMessage(true, "串口对象重新创建成功");
                        return true;
                    }
                    catch (Exception ex)
                    {
                        SendMessage(false, $"打开串口失败 (尝试 {retry + 1}/3): {ex.Message}");
                        if (retry < 2)
                        {
                            await Task.Delay(2000);
                        }
                    }
                }

                return false;
            }
            catch (Exception ex)
            {
                SendMessage(false, $"重新创建串口对象失败: {ex.Message}");
                return false;
            }
        }

        /// <summary>
        /// 清理串口对象
        /// </summary>
        private void CleanupSerialPort()
        {
            try
            {
                _modbusMaster?.StopReceiving();

                if (_serialPort != null)
                {
                    try
                    {
                        if (_serialPort.IsOpen)
                        {
                            _serialPort.Close();
                        }
                    }
                    catch { }

                    try
                    {
                        _serialPort.Dispose();
                    }
                    catch { }

                    _serialPort = null;
                }
            }
            catch { }
        }

        /// <summary>
        /// 原版写Flash逻辑，加入断点续传和重连处理
        /// </summary>
        private async Task<bool> WriteFlashAsync()
        {
            SendMessage(true, "开始烧写Flash...");

            var flashAddr = _config.AppBaseAddr;
            var txIndex = 0;
            var txBuffer = new byte[_config.ReadWriteSize];

            // 断点续传逻辑
            if (_config.EnableResume && _config.LastWrittenProgress > 0)
            {
                var resumeBytes = (_config.LastWrittenProgress * _appSize) / 100;
                txIndex = (resumeBytes / _config.ReadWriteSize) * _config.ReadWriteSize;
                flashAddr = _config.AppBaseAddr + (uint)txIndex;

                SendMessage(true, $"从断点续传: 进度 {_config.LastWrittenProgress}%, 地址: 0x{flashAddr:X8}");
                ProgressUpdated?.Invoke(_config.LastWrittenProgress);
            }
            else
            {
                SendMessage(true, $"从起始地址开始烧写: 0x{flashAddr:X8}");
                ProgressUpdated?.Invoke(0);
            }

            while (txIndex < _appSize && !_stopRequested)
            {
                if (_stopRequested)
                {
                    SendMessage(false, "用户请求停止，正在停止Flash烧写...");
                    return false;
                }

                // 检查串口状态 - 关键修复点
                if (_serialPort == null || !_serialPort.IsOpen)
                {
                    SendMessage(false, "串口连接丢失，尝试重新连接...");

                    if (!await RecreateSerialPortAsync())
                    {
                        SendMessage(false, "重新连接失败，停止烧写");
                        return false;
                    }

                    // 重新启动接收和重新进入IAP模式
                    _modbusMaster.StartReceiving(_serialPort);
                    await Task.Delay(500);

                    SendMessage(true, "重新连接成功，重新进入IAP模式...");
                    if (!await EnterIapModeAsync())
                    {
                        SendMessage(false, "重新进入IAP模式失败");
                        return false;
                    }
                }

                try
                {
                    var txSize = Math.Min(_config.ReadWriteSize, _appSize - txIndex);

                    Array.Clear(txBuffer, 0, txBuffer.Length);
                    Array.Copy(_appData, txIndex, txBuffer, 0, txSize);

                    bool success = false;
                    for (int retry = 0; retry < 3; retry++)
                    {
                        if (_stopRequested || !_serialPort.IsOpen) return false;

                        _ackReceived = false;
                        _ackEvent.Reset();

                        _modbusMaster.SendWriteFlashPacket(_config.ModbusAddr, flashAddr, txBuffer, _config.ReadWriteSize);

                        if (await WaitForAckAsync(3000)) // 保持原版的3秒超时
                        {
                            if (_ackData.Length >= 10)
                            {
                                var cmd = (_ackData[3] << 8) + _ackData[2];
                                var ackAddr = ((uint)_ackData[7] << 24) + ((uint)_ackData[6] << 16) +
                                             ((uint)_ackData[5] << 8) + _ackData[4];
                                var ackLen = (_ackData[9] << 8) + _ackData[8];

                                if (cmd == 3 && ackAddr == flashAddr && ackLen == _config.ReadWriteSize)
                                {
                                    success = true;
                                    break;
                                }
                            }
                        }
                        else
                        {
                            SendMessage(false, $"写Flash超时，地址: 0x{flashAddr:X8}, 重试 {retry + 1}/3");

                            if (retry >= 1) // 连续2次失败后认为需要重连
                            {
                                SendMessage(true, "多次超时，可能连接不稳定，将在下次循环时检查连接状态");
                                break;
                            }
                        }

                        await Task.Delay(200);
                    }

                    if (!success)
                    {
                        SendMessage(false, $"写Flash失败，地址: 0x{flashAddr:X8}，保存断点");
                        SaveCurrentProgress(txIndex);
                        return false; // 返回失败，让主循环重试
                    }

                    // 更新进度
                    txIndex += _config.ReadWriteSize;
                    flashAddr += (uint)_config.ReadWriteSize;

                    var progress = Math.Min(100, (txIndex * 100) / _appSize);
                    ProgressUpdated?.Invoke(progress);

                    // 保存断点续传状态
                    if (progress % 10 == 0 && progress > _lastSuccessProgress)
                    {
                        SaveCurrentProgress(txIndex);
                        SendMessage(true, $"烧写进度: {progress}%, 地址: 0x{flashAddr:X8} (已保存断点)");
                    }
                    else if (progress % 10 == 0)
                    {
                        SendMessage(true, $"烧写进度: {progress}%, 地址: 0x{flashAddr:X8}");
                    }
                }
                catch (Exception ex)
                {
                    SendMessage(false, $"写Flash时发生错误: {ex.Message}");
                    return false;
                }
            }

            if (_stopRequested)
            {
                SendMessage(false, "Flash烧写被用户停止");
                return false;
            }

            SendMessage(true, "Flash烧写完成");
            ProgressUpdated?.Invoke(100);
            return true;
        }

        // 保留原版的其他方法
        private async Task<bool> LoadAppFileAsync()
        {
            try
            {
                SendMessage(true, "正在加载APP文件...");

                if (string.IsNullOrEmpty(_config.AppFile) || !File.Exists(_config.AppFile))
                {
                    SendMessage(false, $"APP文件不存在: {_config.AppFile}");
                    return false;
                }

                if (_config.AppFile.ToLower().EndsWith(".hex"))
                {
                    var hexLoader = new HexFileLoader();
                    _appData = hexLoader.LoadHexFile(_config.AppFile);
                }
                else
                {
                    _appData = await File.ReadAllBytesAsync(_config.AppFile);
                }

                _appSize = _appData.Length;

                if (_appSize <= 0 || _appSize > _config.AppMaxSize)
                {
                    SendMessage(false, $"APP文件大小异常: {_appSize} bytes (最大: {_config.AppMaxSize} bytes)");
                    return false;
                }

                _appCrc = CrcCalculator.Crc32Update(0, _appData, _appSize);
                SendMessage(true, $"APP文件加载成功: {_appSize} bytes, CRC32=0x{_appCrc:X8}");
                return true;
            }
            catch (Exception ex)
            {
                SendMessage(false, $"加载APP文件失败: {ex.Message}");
                return false;
            }
        }

        private async Task<bool> EnterIapModeAsync()
        {
            SendMessage(true, "正在进入IAP模式...");

            for (int retry = 0; retry < 20; retry++)
            {
                if (_stopRequested) return false;

                try
                {
                    _ackReceived = false;
                    _ackEvent.Reset();

                    _modbusMaster.SendEnterPacket(_config.ModbusAddr, 1);

                    if (await WaitForAckAsync(1000))
                    {
                        if (_ackData.Length >= 13)
                        {
                            var cmd = (_ackData[3] << 8) + _ackData[2];
                            var who = _ackData[12];

                            if (cmd == 1 && who == 1)
                            {
                                SendMessage(true, "成功进入IAP模式");
                                return true;
                            }
                        }
                    }

                    await Task.Delay(100);
                }
                catch (Exception ex)
                {
                    SendMessage(false, $"进入IAP模式时发生错误: {ex.Message}");
                }
            }

            SendMessage(false, "进入IAP模式失败，请检查设备连接和通信参数");
            return false;
        }

        private async Task<bool> WriteChecksumAsync()
        {
            if (_stopRequested) return false;

            SendMessage(true, "正在写入校验码...");

            try
            {
                var txBuffer = new byte[_config.ReadWriteSize];

                txBuffer[0] = (byte)(_appSize & 0xFF);
                txBuffer[1] = (byte)((_appSize >> 8) & 0xFF);
                txBuffer[2] = (byte)((_appSize >> 16) & 0xFF);
                txBuffer[3] = (byte)((_appSize >> 24) & 0xFF);

                txBuffer[4] = (byte)(_appCrc & 0xFF);
                txBuffer[5] = (byte)((_appCrc >> 8) & 0xFF);
                txBuffer[6] = (byte)((_appCrc >> 16) & 0xFF);
                txBuffer[7] = (byte)((_appCrc >> 24) & 0xFF);

                for (int retry = 0; retry < 10; retry++)
                {
                    if (_stopRequested) return false;

                    _ackReceived = false;
                    _ackEvent.Reset();

                    _modbusMaster.SendWriteChecksumPacket(_config.ModbusAddr, _config.ArgBaseAddr, txBuffer, _config.ReadWriteSize);

                    if (await WaitForAckAsync(2000))
                    {
                        if (_ackData.Length >= 10)
                        {
                            var cmd = (_ackData[3] << 8) + _ackData[2];
                            var ackLen = (_ackData[9] << 8) + _ackData[8];

                            if (cmd == 4 && ackLen == _config.ReadWriteSize)
                            {
                                SendMessage(true, "校验码写入成功");
                                return true;
                            }
                        }
                    }

                    await Task.Delay(100);
                }

                SendMessage(false, "写入校验码失败");
                return false;
            }
            catch (Exception ex)
            {
                SendMessage(false, $"写入校验码时发生错误: {ex.Message}");
                return false;
            }
        }

        private async Task<bool> ExitIapModeAsync()
        {
            if (_stopRequested) return false;

            SendMessage(true, "正在退出IAP模式...");

            for (int retry = 0; retry < 5; retry++)
            {
                if (_stopRequested) return false;

                try
                {
                    _ackReceived = false;
                    _ackEvent.Reset();

                    _modbusMaster.SendExitPacket(_config.ModbusAddr);

                    if (await WaitForAckAsync(1000))
                    {
                        if (_ackData.Length >= 4)
                        {
                            var cmd = (_ackData[3] << 8) + _ackData[2];
                            if (cmd == 0)
                            {
                                SendMessage(true, "成功退出IAP模式");
                                return true;
                            }
                        }
                    }

                    await Task.Delay(200);
                }
                catch (Exception ex)
                {
                    SendMessage(false, $"退出IAP模式时发生错误: {ex.Message}");
                }
            }

            SendMessage(false, "退出IAP模式失败");
            return false;
        }

        private void SaveCurrentProgress(int txIndex)
        {
            var progress = Math.Min(100, (txIndex * 100) / _appSize);
            var currentAddr = _config.AppBaseAddr + (uint)txIndex;

            _lastSuccessAddr = currentAddr;
            _lastSuccessProgress = progress;

            SaveResumeState();
        }

        private void SaveResumeState()
        {
            try
            {
                if (_config != null && !string.IsNullOrEmpty(_configFilePath))
                {
                    _config.LastWrittenAddr = _lastSuccessAddr;
                    _config.LastWrittenProgress = _lastSuccessProgress;

                    var json = JsonConvert.SerializeObject(_config, Formatting.Indented);
                    File.WriteAllText(_configFilePath, json);

                    SendMessage(true, $"已保存续传状态: {_lastSuccessProgress}%, 地址: 0x{_lastSuccessAddr:X8}");
                }
            }
            catch (Exception ex)
            {
                SendMessage(false, $"保存续传状态失败: {ex.Message}");
            }
        }

        private void CheckResumeState()
        {
            if (_config.EnableResume && _config.LastWrittenProgress > 0)
            {
                _lastSuccessAddr = _config.LastWrittenAddr;
                _lastSuccessProgress = _config.LastWrittenProgress;
                SendMessage(true, $"发现续传状态: 进度 {_config.LastWrittenProgress}%, 地址 0x{_config.LastWrittenAddr:X8}");
            }
        }

        private void ClearResumeState()
        {
            try
            {
                if (_config != null)
                {
                    _config.LastWrittenAddr = 0;
                    _config.LastWrittenProgress = 0;
                    _lastSuccessAddr = 0;
                    _lastSuccessProgress = 0;

                    if (!string.IsNullOrEmpty(_configFilePath))
                    {
                        var json = JsonConvert.SerializeObject(_config, Formatting.Indented);
                        File.WriteAllText(_configFilePath, json);
                        SendMessage(true, "已清除续传状态");
                    }
                }
            }
            catch (Exception ex)
            {
                SendMessage(false, $"清除续传状态失败: {ex.Message}");
            }
        }

        private async Task<bool> WaitForAckAsync(int timeoutMs)
        {
            return await Task.Run(() => _ackEvent.Wait(timeoutMs));
        }

        private void OnDataReceived(byte[] data)
        {
            _ackData = data;
            _ackReceived = true;
            _ackEvent.Set();
        }

        private void OnErrorOccurred(string error)
        {
            SendMessage(false, error);
        }

        private void SendMessage(bool isSuccess, string message)
        {
            MessageReceived?.Invoke(isSuccess, message);
        }

        public string GetConnectionStats()
        {
            return $"总主循环重试: {_totalMainRetries}";
        }
    }

    // 注意：IapConfig 类已在其他文件中定义，这里不重复定义
}