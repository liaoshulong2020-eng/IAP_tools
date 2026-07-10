namespace CAN_TOOLS;

internal sealed class CanIapUpgradeSession
{
    public const uint IapCanIdBase = 0xA0000;
    public const uint LegacyIapCanId = 0xAA55;
    private const byte FnoIap = 0x41;
    private const uint AppBaseAddr = 0x08008000;
    private const uint ArgBaseAddr = 0x08007000;
    private const int AppMaxSize = 224 * 1024;
    private const int WriteSize = 96;
    private const int MaxMainRetries = 3;
    private const int MainRetryDelayMs = 5000;
    private const int WriteRetryDelayMs = 300;
    private const int EnterProbeCount = 18;
    private const int EnterProbeDelayMs = 120;
    private const int EnterProbeAckTimeoutMs = 260;

    private readonly Func<byte[], CancellationToken, Task> _sendPacketAsync;
    private readonly Func<CancellationToken, Task>? _recoverTransportAsync;
    private readonly Action<string> _log;
    private readonly Action<int> _progress;
    private readonly uint _iapCanId;
    private readonly string _resumeKey;
    private readonly object _rxLock = new();
    private readonly List<byte> _rxBuffer = new();
    private TaskCompletionSource<byte[]>? _ackTcs;
    private byte _targetAddr;
    private ResumeState _resumeState = new();

    public CanIapUpgradeSession(
        uint iapCanId,
        Func<byte[], CancellationToken, Task> sendPacketAsync,
        Action<string> log,
        Action<int> progress,
        string? resumeKey = null,
        Func<CancellationToken, Task>? recoverTransportAsync = null)
    {
        _iapCanId = iapCanId;
        _sendPacketAsync = sendPacketAsync;
        _log = log;
        _progress = progress;
        _resumeKey = string.IsNullOrWhiteSpace(resumeKey) ? $"can_{iapCanId:X5}" : resumeKey;
        _recoverTransportAsync = recoverTransportAsync;
    }

    public bool IsRunning { get; private set; }

    public void HandleCanFrame(uint id, byte[] data, int length)
    {
        if (id != _iapCanId || length <= 0) return;
        HandleBytes(data, length);
    }

    public static uint GetNodeCanId(int nodeId)
    {
        if (nodeId < 0 || nodeId > 7)
            throw new ArgumentOutOfRangeException(nameof(nodeId), "节点号必须在0到7之间");
        return IapCanIdBase + (uint)nodeId;
    }

    public void HandleBytes(byte[] data, int length)
    {
        if (length <= 0) return;
        lock (_rxLock)
        {
            for (int i = 0; i < length; i++)
                _rxBuffer.Add(data[i]);

            while (_rxBuffer.Count > 0 && _rxBuffer[0] != _targetAddr)
                _rxBuffer.RemoveAt(0);

            if (_rxBuffer.Count <= 12) return;

            int payloadSize = _rxBuffer[10] | (_rxBuffer[11] << 8);
            if (payloadSize < 0 || payloadSize > 256)
            {
                _log($"IAP接收缓存长度异常 size={payloadSize}，已清空");
                _rxBuffer.Clear();
                return;
            }

            int packetSize = payloadSize + 14;
            if (_rxBuffer.Count < packetSize) return;

            byte[] packet = _rxBuffer.Take(packetSize).ToArray();
            _rxBuffer.RemoveRange(0, packetSize);

            ushort crcRx = (ushort)(packet[^2] | (packet[^1] << 8));
            ushort crcCalc = IapCrc.Crc16(packet, packet.Length - 2);
            if (crcRx == crcCalc)
            {
                _ackTcs?.TrySetResult(packet);
            }
            else
            {
                _log($"IAP包CRC错误 RX=0x{crcRx:X4}, CALC=0x{crcCalc:X4}, LEN={packet.Length}");
            }
        }
    }

    public async Task<bool> UpgradeAsync(byte targetAddr, string firmwarePath, CancellationToken token)
    {
        _targetAddr = targetAddr;
        IsRunning = true;
        try
        {
            byte[] app = await LoadFirmwareAsync(firmwarePath, token);
            if (app.Length == 0 || app.Length > AppMaxSize)
            {
                _log($"固件大小异常: {app.Length} bytes，最大 {AppMaxSize} bytes");
                return false;
            }

            uint appCrc = IapCrc.Crc32(app, app.Length);
            _log($"固件加载完成: {app.Length} bytes, CRC32=0x{appCrc:X8}");
            LoadResumeState(firmwarePath, app.Length, appCrc);

            for (int attempt = 1; attempt <= MaxMainRetries; attempt++)
            {
                token.ThrowIfCancellationRequested();
                _log($"=== 第 {attempt} 次升级尝试 ===");

                try
                {
                    if (await ExecuteUpgradeOnceAsync(app, appCrc, token))
                    {
                        ClearResumeState();
                        _progress(100);
                        _log($"升级完成，总尝试次数: {attempt}");
                        return true;
                    }
                }
                catch (OperationCanceledException)
                {
                    throw;
                }
                catch (Exception ex)
                {
                    _log($"第 {attempt} 次升级异常: {ex.Message}");
                }

                SaveResumeState(firmwarePath, app.Length, appCrc);
                if (attempt >= MaxMainRetries)
                    break;

                if (_recoverTransportAsync != null)
                {
                    try
                    {
                        _log("尝试恢复 CAN 通道...");
                        await _recoverTransportAsync(token);
                    }
                    catch (Exception ex)
                    {
                        _log($"CAN 通道恢复失败: {ex.Message}");
                    }
                }

                _log($"第 {attempt} 次尝试失败，{MainRetryDelayMs / 1000}秒后自动重试");
                await Task.Delay(MainRetryDelayMs, token);
            }

            _log($"升级失败，已达到最大尝试次数 {MaxMainRetries}");
            return false;
        }
        finally
        {
            IsRunning = false;
            lock (_rxLock)
            {
                _rxBuffer.Clear();
                _ackTcs = null;
            }
        }
    }

    private async Task<bool> ExecuteUpgradeOnceAsync(byte[] app, uint appCrc, CancellationToken token)
    {
        if (!await EnterIapAsync(token)) return false;

        if (!await WriteFlashAsync(app, token)) return false;
        if (!await WriteChecksumAsync(app.Length, appCrc, token)) return false;
        if (!await ExitIapAsync(token)) return false;
        return true;
    }

    private static async Task<byte[]> LoadFirmwareAsync(string path, CancellationToken token)
    {
        if (path.EndsWith(".hex", StringComparison.OrdinalIgnoreCase))
            return IapHexFileLoader.Load(path);

        return await File.ReadAllBytesAsync(path, token);
    }

    private async Task<bool> EnterIapAsync(CancellationToken token)
    {
        _log("进入 IAP...");
        byte[] data = { 1 };
        byte[] packet = BuildPacket(1, 0, 1, data);

        _log("发送进入 IAP 命令，触发 APP 复位...");
        byte[]? ack = await SendAndWaitAsync(packet, TimeSpan.FromMilliseconds(EnterProbeAckTimeoutMs), token);
        if (IsEnterAck(ack))
        {
            _log("已进入 IAP");
            await Task.Delay(500, token);
            return true;
        }

        _log("等待 bootloader 启动并确认 IAP...");
        for (int probe = 1; probe <= EnterProbeCount; probe++)
        {
            await Task.Delay(EnterProbeDelayMs, token);
            ack = await SendAndWaitAsync(packet, TimeSpan.FromMilliseconds(EnterProbeAckTimeoutMs), token);
            if (IsEnterAck(ack))
            {
                _log($"bootloader 已进入 IAP，握手次数 {probe}");
                await Task.Delay(500, token);
                return true;
            }
        }

        _log("未收到 bootloader 进入确认，停止本次尝试，避免继续反复复位");
        return false;
    }

    private static bool IsEnterAck(byte[]? ack)
    {
        return ack != null && GetCmd(ack) == 1 && ack.Length > 12 && ack[12] == 1;
    }

    private async Task<bool> WriteFlashAsync(byte[] app, CancellationToken token)
    {
        _log("开始写 Flash...");
        int index = GetResumeIndex(app.Length);
        if (index > 0)
        {
            uint resumeAddr = AppBaseAddr + (uint)index;
            int resumePercent = Math.Min(99, index * 100 / app.Length);
            _progress(resumePercent);
            _log($"从断点续传: {resumePercent}%, 地址 0x{resumeAddr:X8}");
        }

        while (index < app.Length)
        {
            token.ThrowIfCancellationRequested();

            var block = new byte[WriteSize];
            int copy = Math.Min(WriteSize, app.Length - index);
            Array.Copy(app, index, block, 0, copy);

            uint addr = AppBaseAddr + (uint)index;
            bool ok = false;
            for (int retry = 0; retry < 5; retry++)
            {
                byte[]? ack = await SendAndWaitAsync(BuildPacket(3, addr, WriteSize, block), TimeSpan.FromMilliseconds(3000), token);
                if (ack != null && GetCmd(ack) == 3 && GetAddr(ack) == addr && GetLen(ack) == WriteSize)
                {
                    ok = true;
                    break;
                }

                await Task.Delay(WriteRetryDelayMs, token);
            }

            if (!ok)
            {
                _resumeState.LastWrittenIndex = index;
                SaveResumeState();
                _log($"写 Flash 失败: 0x{addr:X8}");
                return false;
            }

            index += WriteSize;
            _resumeState.LastWrittenIndex = Math.Min(index, app.Length);
            SaveResumeState();

            int percent = Math.Min(99, index * 100 / app.Length);
            _progress(percent);
            if (percent % 10 == 0)
                _log($"写入进度 {percent}%");
        }

        return true;
    }

    private async Task<bool> WriteChecksumAsync(int appSize, uint appCrc, CancellationToken token)
    {
        _log("写入校验信息...");
        var data = new byte[WriteSize];
        WriteUInt32(data, 0, (uint)appSize);
        WriteUInt32(data, 4, appCrc);

        for (int retry = 0; retry < 10; retry++)
        {
            byte[]? ack = await SendAndWaitAsync(BuildPacket(4, ArgBaseAddr, WriteSize, data), TimeSpan.FromMilliseconds(3000), token);
            if (ack != null && GetCmd(ack) == 4 && GetLen(ack) == WriteSize)
            {
                _log("校验信息写入完成");
                return true;
            }
        }

        _log("校验信息写入失败");
        return false;
    }

    private async Task<bool> ExitIapAsync(CancellationToken token)
    {
        _log("退出 IAP...");
        for (int retry = 0; retry < 5; retry++)
        {
            byte[]? ack = await SendAndWaitAsync(BuildPacket(0, 0, 0, Array.Empty<byte>()), TimeSpan.FromMilliseconds(1500), token);
            if (ack != null && GetCmd(ack) == 0)
                return true;
        }

        _log("退出 IAP 未收到确认");
        return false;
    }

    private async Task<byte[]?> SendAndWaitAsync(byte[] packet, TimeSpan timeout, CancellationToken token)
    {
        TaskCompletionSource<byte[]> tcs;
        lock (_rxLock)
        {
            _rxBuffer.Clear();
            _ackTcs = tcs = new TaskCompletionSource<byte[]>(TaskCreationOptions.RunContinuationsAsynchronously);
        }

        await _sendPacketAsync(packet, token);
        Task completed = await Task.WhenAny(tcs.Task, Task.Delay(timeout, token));
        return completed == tcs.Task ? await tcs.Task : null;
    }

    private int GetResumeIndex(int appLength)
    {
        int index = Math.Max(0, Math.Min(_resumeState.LastWrittenIndex, appLength));
        return index / WriteSize * WriteSize;
    }

    private string ResumeStatePath
    {
        get
        {
            string dir = Path.Combine(AppContext.BaseDirectory, "iap_resume");
            Directory.CreateDirectory(dir);
            string safeName = string.Concat(_resumeKey.Select(ch =>
                char.IsLetterOrDigit(ch) || ch == '_' || ch == '-' ? ch : '_'));
            return Path.Combine(dir, safeName + ".json");
        }
    }

    private void LoadResumeState(string firmwarePath, int appLength, uint appCrc)
    {
        _resumeState = new ResumeState
        {
            FirmwarePath = firmwarePath,
            AppLength = appLength,
            AppCrc = appCrc,
            LastWrittenIndex = 0
        };

        try
        {
            string path = ResumeStatePath;
            if (!File.Exists(path))
                return;

            string json = File.ReadAllText(path);
            ResumeState? saved = System.Text.Json.JsonSerializer.Deserialize<ResumeState>(json);
            if (saved == null ||
                !string.Equals(Path.GetFullPath(saved.FirmwarePath ?? string.Empty), Path.GetFullPath(firmwarePath), StringComparison.OrdinalIgnoreCase) ||
                saved.AppLength != appLength ||
                saved.AppCrc != appCrc ||
                saved.LastWrittenIndex <= 0)
            {
                return;
            }

            _resumeState = saved;
            _log($"发现断点续传状态: {Math.Min(99, saved.LastWrittenIndex * 100 / appLength)}%, 地址 0x{AppBaseAddr + (uint)saved.LastWrittenIndex:X8}");
        }
        catch (Exception ex)
        {
            _log($"读取断点续传状态失败: {ex.Message}");
        }
    }

    private void SaveResumeState(string? firmwarePath = null, int? appLength = null, uint? appCrc = null)
    {
        try
        {
            if (firmwarePath != null) _resumeState.FirmwarePath = firmwarePath;
            if (appLength.HasValue) _resumeState.AppLength = appLength.Value;
            if (appCrc.HasValue) _resumeState.AppCrc = appCrc.Value;

            string json = System.Text.Json.JsonSerializer.Serialize(_resumeState, new System.Text.Json.JsonSerializerOptions
            {
                WriteIndented = true
            });
            File.WriteAllText(ResumeStatePath, json);
        }
        catch (Exception ex)
        {
            _log($"保存断点续传状态失败: {ex.Message}");
        }
    }

    private void ClearResumeState()
    {
        _resumeState.LastWrittenIndex = 0;
        try
        {
            string path = ResumeStatePath;
            if (File.Exists(path))
                File.Delete(path);
        }
        catch (Exception ex)
        {
            _log($"清除断点续传状态失败: {ex.Message}");
        }
    }

    private byte[] BuildPacket(ushort cmd, uint addr, ushort len, byte[] data)
    {
        ushort size = (ushort)data.Length;
        var packet = new byte[12 + size + 2];
        packet[0] = _targetAddr;
        packet[1] = FnoIap;
        WriteUInt16(packet, 2, cmd);
        WriteUInt32(packet, 4, addr);
        WriteUInt16(packet, 8, len);
        WriteUInt16(packet, 10, size);
        Array.Copy(data, 0, packet, 12, size);
        ushort crc = IapCrc.Crc16(packet, packet.Length - 2);
        WriteUInt16(packet, packet.Length - 2, crc);
        return packet;
    }

    private static ushort GetCmd(byte[] packet) => (ushort)(packet[2] | (packet[3] << 8));
    private static uint GetAddr(byte[] packet) => (uint)(packet[4] | (packet[5] << 8) | (packet[6] << 16) | (packet[7] << 24));
    private static ushort GetLen(byte[] packet) => (ushort)(packet[8] | (packet[9] << 8));

    private static void WriteUInt16(byte[] data, int offset, ushort value)
    {
        data[offset] = (byte)value;
        data[offset + 1] = (byte)(value >> 8);
    }

    private static void WriteUInt32(byte[] data, int offset, uint value)
    {
        data[offset] = (byte)value;
        data[offset + 1] = (byte)(value >> 8);
        data[offset + 2] = (byte)(value >> 16);
        data[offset + 3] = (byte)(value >> 24);
    }

    private sealed class ResumeState
    {
        public string? FirmwarePath { get; set; }
        public int AppLength { get; set; }
        public uint AppCrc { get; set; }
        public int LastWrittenIndex { get; set; }
    }
}
