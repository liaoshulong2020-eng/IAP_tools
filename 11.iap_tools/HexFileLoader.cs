// HexFileLoader.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Globalization;

namespace IapUpgradeTool
{
	public class HexFileLoader
	{
		private class HexRecord
		{
			public byte ByteCount { get; set; }
			public ushort Address { get; set; }
			public byte RecordType { get; set; }
			public byte[] Data { get; set; }
			public byte Checksum { get; set; }
		}

        public byte[] LoadHexFile(string filePath)
        {
            try
            {
                if (!File.Exists(filePath))
                    throw new FileNotFoundException($"文件不存在: {filePath}");

                var lines = File.ReadAllLines(filePath);
                var memoryMap = new Dictionary<uint, byte>();
                uint baseAddress = 0;
                uint minAddress = uint.MaxValue;
                uint maxAddress = 0;

                foreach (var line in lines)
                {
                    if (string.IsNullOrWhiteSpace(line) || !line.StartsWith(":"))
                        continue;

                    var record = ParseHexRecord(line);
                    if (record == null) continue;

                    switch (record.RecordType)
                    {
                        case 0x00: // Data Record
                            uint address = baseAddress + record.Address;
                            for (int i = 0; i < record.ByteCount; i++)
                            {
                                memoryMap[address + (uint)i] = record.Data[i];
                                minAddress = Math.Min(minAddress, address + (uint)i);
                                maxAddress = Math.Max(maxAddress, address + (uint)i);
                            }
                            break;

                        case 0x01: // End of File Record
                            break;

                        case 0x02: // Extended Segment Address Record
                            if (record.ByteCount == 2)
                            {
                                baseAddress = ((uint)record.Data[0] << 12) | ((uint)record.Data[1] << 4);
                            }
                            break;

                        case 0x04: // Extended Linear Address Record
                            if (record.ByteCount == 2)
                            {
                                baseAddress = ((uint)record.Data[0] << 24) | ((uint)record.Data[1] << 16);
                            }
                            break;

                        case 0x05: // Start Linear Address Record
                            break;
                    }
                }

                if (memoryMap.Count == 0)
                    throw new InvalidOperationException("HEX文件中没有找到有效数据");

                uint totalSize = maxAddress - minAddress + 1;
                var result = new byte[totalSize];

                for (uint addr = minAddress; addr <= maxAddress; addr++)
                {
                    if (memoryMap.ContainsKey(addr))
                        result[addr - minAddress] = memoryMap[addr];
                    else
                        result[addr - minAddress] = 0xFF;
                }

                return result;
            }
            catch (Exception ex)
            {
                throw new InvalidOperationException($"加载HEX文件失败: {ex.Message}", ex);
            }
        }

        private HexRecord ParseHexRecord(string line)
        {
            try
            {
                if (line.Length < 11) return null;

                var record = new HexRecord();
                record.ByteCount = byte.Parse(line.Substring(1, 2), NumberStyles.HexNumber);
                record.Address = ushort.Parse(line.Substring(3, 4), NumberStyles.HexNumber);
                record.RecordType = byte.Parse(line.Substring(7, 2), NumberStyles.HexNumber);

                record.Data = new byte[record.ByteCount];
                for (int i = 0; i < record.ByteCount; i++)
                {
                    record.Data[i] = byte.Parse(line.Substring(9 + i * 2, 2), NumberStyles.HexNumber);
                }

                record.Checksum = byte.Parse(line.Substring(9 + record.ByteCount * 2, 2), NumberStyles.HexNumber);

                // 验证校验和
                byte calculatedChecksum = 0;
                calculatedChecksum += record.ByteCount;
                calculatedChecksum += (byte)(record.Address >> 8);
                calculatedChecksum += (byte)(record.Address & 0xFF);
                calculatedChecksum += record.RecordType;
                foreach (var b in record.Data)
                    calculatedChecksum += b;
                calculatedChecksum = (byte)(0x100 - calculatedChecksum);

                if (calculatedChecksum != record.Checksum)
                    throw new InvalidOperationException($"HEX记录校验和错误: {line}");

                return record;
            }
            catch (Exception ex)
            {
                throw new InvalidOperationException($"解析HEX记录失败: {line}, 错误: {ex.Message}");
            }
        }
    }
}