// ModbusMaster.cs
using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Threading;
using System.Threading.Tasks;

namespace IapUpgradeTool
{
	public class ModbusMaster
	{
		private SerialPort _serialPort;
		private bool _isReceiving;
		private CancellationTokenSource _cancellationTokenSource;

		public event Action<byte[]> DataReceived;
		public event Action<string> ErrorOccurred;

		public void StartReceiving(SerialPort serialPort)
		{
			try
			{
				_serialPort = serialPort ?? throw new ArgumentNullException(nameof(serialPort));

				if (!_serialPort.IsOpen)
					throw new InvalidOperationException("串口未打开");

				_cancellationTokenSource = new CancellationTokenSource();
				_isReceiving = true;

				Task.Run(() => ReceiveData(_cancellationTokenSource.Token));
			}
			catch (Exception ex)
			{
				ErrorOccurred?.Invoke($"启动接收失败: {ex.Message}");
			}
		}

		public void StopReceiving()
		{
			try
			{
				_isReceiving = false;
				_cancellationTokenSource?.Cancel();
			}
			catch (Exception ex)
			{
				ErrorOccurred?.Invoke($"停止接收失败: {ex.Message}");
			}
		}

        private void ReceiveData(CancellationToken cancellationToken)
        {
            var rxBuffer = new List<byte>();

            while (_isReceiving && !cancellationToken.IsCancellationRequested)
            {
                try
                {
                    // 检查串口状态
                    if (_serialPort == null || !_serialPort.IsOpen)
                    {
                        ErrorOccurred?.Invoke("串口连接丢失，停止接收数据");
                        break;
                    }

                    if (_serialPort.BytesToRead > 0)
                    {
                        var buffer = new byte[_serialPort.BytesToRead];
                        var bytesRead = _serialPort.Read(buffer, 0, buffer.Length);

                        for (int i = 0; i < bytesRead; i++)
                        {
                            rxBuffer.Add(buffer[i]);
                        }

                        if (rxBuffer.Count > 12)
                        {
                            int size = (rxBuffer[11] << 8) + rxBuffer[10];
                            if (rxBuffer.Count >= size + 14)
                            {
                                var packet = rxBuffer.ToArray();
                                var crcReceived = (ushort)((packet[packet.Length - 1] << 8) + packet[packet.Length - 2]);
                                var crcCalculated = CrcCalculator.Crc16Update(0, packet, packet.Length - 2);

                                if (crcReceived == crcCalculated)
                                {
                                    DataReceived?.Invoke(packet);
                                }
                                else
                                {
                                    ErrorOccurred?.Invoke($"CRC校验失败: 期望={crcCalculated:X4}, 接收={crcReceived:X4}");
                                }

                                rxBuffer.Clear();
                            }
                        }
                    }

                    Thread.Sleep(50);
                }
                catch (InvalidOperationException ex) when (ex.Message.Contains("port is closed") ||
                                                          ex.Message.Contains("设备没有发挥作用"))
                {
                    ErrorOccurred?.Invoke("串口已关闭，停止接收数据");
                    break;
                }
                catch (UnauthorizedAccessException ex)
                {
                    ErrorOccurred?.Invoke($"串口访问权限丢失: {ex.Message}");
                    break;
                }
                catch (System.IO.IOException ex)
                {
                    ErrorOccurred?.Invoke($"串口IO错误: {ex.Message}");
                    break;
                }
                catch (Exception ex)
                {
                    if (_isReceiving)
                    {
                        ErrorOccurred?.Invoke($"接收数据时发生错误: {ex.Message}");
                    }
                    Thread.Sleep(200);
                }
            }
        }

        public void SendExitPacket(byte modbusAddr)
        {
            try
            {
                if (_serialPort == null || !_serialPort.IsOpen)
                {
                    ErrorOccurred?.Invoke("串口未打开，无法发送退出命令");
                    return;
                }

                var buffer = new byte[14];
                buffer[0] = modbusAddr;
                buffer[1] = 0x41;
                buffer[2] = 0;
                buffer[3] = 0;

                var crc = CrcCalculator.Crc16Update(0, buffer, 12);
                buffer[12] = (byte)(crc & 0xFF);
                buffer[13] = (byte)((crc >> 8) & 0xFF);

                _serialPort.Write(buffer, 0, buffer.Length);
            }
            catch (InvalidOperationException ex) when (ex.Message.Contains("port is closed"))
            {
                ErrorOccurred?.Invoke("串口已关闭，无法发送退出命令");
            }
            catch (Exception ex)
            {
                ErrorOccurred?.Invoke($"发送退出命令失败: {ex.Message}");
            }
        }

        public void SendEnterPacket(byte modbusAddr, byte who)
        {
            try
            {
                if (_serialPort == null || !_serialPort.IsOpen)
                {
                    ErrorOccurred?.Invoke("串口未打开，无法发送进入IAP命令");
                    return;
                }

                var buffer = new byte[15];
                buffer[0] = modbusAddr;
                buffer[1] = 0x41;
                buffer[2] = 1;
                buffer[3] = 0;
                buffer[8] = 1;
                buffer[9] = 0;
                buffer[10] = 1;
                buffer[11] = 0;
                buffer[12] = who;

                var crc = CrcCalculator.Crc16Update(0, buffer, 13);
                buffer[13] = (byte)(crc & 0xFF);
                buffer[14] = (byte)((crc >> 8) & 0xFF);

                _serialPort.Write(buffer, 0, buffer.Length);
            }
            catch (InvalidOperationException ex) when (ex.Message.Contains("port is closed"))
            {
                ErrorOccurred?.Invoke("串口已关闭，无法发送进入IAP命令");
            }
            catch (Exception ex)
            {
                ErrorOccurred?.Invoke($"发送进入IAP命令失败: {ex.Message}");
            }
        }


        public void SendWriteFlashPacket(byte modbusAddr, uint flashAddr, byte[] data, int length)
        {
            try
            {
                // 【关键修复】发送前检查串口状态
                if (_serialPort == null || !_serialPort.IsOpen)
                {
                    ErrorOccurred?.Invoke("串口未打开，无法发送写Flash命令");
                    return;
                }

                var buffer = new byte[length + 14];
                buffer[0] = modbusAddr;
                buffer[1] = 0x41;
                buffer[2] = 3;
                buffer[3] = 0;

                buffer[4] = (byte)(flashAddr & 0xFF);
                buffer[5] = (byte)((flashAddr >> 8) & 0xFF);
                buffer[6] = (byte)((flashAddr >> 16) & 0xFF);
                buffer[7] = (byte)((flashAddr >> 24) & 0xFF);

                buffer[8] = (byte)(length & 0xFF);
                buffer[9] = (byte)((length >> 8) & 0xFF);
                buffer[10] = (byte)(length & 0xFF);
                buffer[11] = (byte)((length >> 8) & 0xFF);

                Array.Copy(data, 0, buffer, 12, length);

                var crc = CrcCalculator.Crc16Update(0, buffer, 12 + length);
                buffer[12 + length] = (byte)(crc & 0xFF);
                buffer[12 + length + 1] = (byte)((crc >> 8) & 0xFF);

                _serialPort.Write(buffer, 0, buffer.Length);
            }
            catch (InvalidOperationException ex) when (ex.Message.Contains("port is closed") ||
                                                      ex.Message.Contains("设备没有发挥作用"))
            {
                ErrorOccurred?.Invoke($"串口连接丢失: {ex.Message}");
            }
            catch (UnauthorizedAccessException ex)
            {
                ErrorOccurred?.Invoke($"串口访问被拒绝，可能被其他程序占用: {ex.Message}");
            }
            catch (System.IO.IOException ex)
            {
                ErrorOccurred?.Invoke($"串口IO错误，设备可能已断开: {ex.Message}");
            }
            catch (Exception ex)
            {
                ErrorOccurred?.Invoke($"发送写Flash命令失败: {ex.Message}");
            }
        }

        public void SendWriteChecksumPacket(byte modbusAddr, uint flashAddr, byte[] data, int length)
		{
			try
			{
				var buffer = new byte[length + 14];
				buffer[0] = modbusAddr;
				buffer[1] = 0x41;
				buffer[2] = 4;
				buffer[3] = 0;

				buffer[4] = (byte)(flashAddr & 0xFF);
				buffer[5] = (byte)((flashAddr >> 8) & 0xFF);
				buffer[6] = (byte)((flashAddr >> 16) & 0xFF);
				buffer[7] = (byte)((flashAddr >> 24) & 0xFF);

				buffer[8] = (byte)(length & 0xFF);
				buffer[9] = (byte)((length >> 8) & 0xFF);
				buffer[10] = (byte)(length & 0xFF);
				buffer[11] = (byte)((length >> 8) & 0xFF);

				Array.Copy(data, 0, buffer, 12, length);

				var crc = CrcCalculator.Crc16Update(0, buffer, 12 + length);
				buffer[12 + length] = (byte)(crc & 0xFF);
				buffer[12 + length + 1] = (byte)((crc >> 8) & 0xFF);

				_serialPort.Write(buffer, 0, buffer.Length);
			}
			catch (Exception ex)
			{
				ErrorOccurred?.Invoke($"发送写校验码命令失败: {ex.Message}");
			}
		}
	}
}