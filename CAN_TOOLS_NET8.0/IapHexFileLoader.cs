namespace CAN_TOOLS;

internal static class IapHexFileLoader
{
    public static byte[] Load(string path)
    {
        var memory = new SortedDictionary<uint, byte>();
        uint baseAddress = 0;
        uint minAddress = uint.MaxValue;
        uint maxAddress = 0;

        foreach (string rawLine in File.ReadLines(path))
        {
            string line = rawLine.Trim();
            if (line.Length == 0) continue;
            if (!line.StartsWith(':')) throw new InvalidDataException("HEX record must start with ':'");

            byte length = ParseByte(line, 1);
            ushort address = (ushort)((ParseByte(line, 3) << 8) | ParseByte(line, 5));
            byte type = ParseByte(line, 7);

            byte sum = length;
            sum += (byte)(address >> 8);
            sum += (byte)address;
            sum += type;
            var data = new byte[length];
            for (int i = 0; i < length; i++)
            {
                data[i] = ParseByte(line, 9 + i * 2);
                sum += data[i];
            }

            byte checksum = ParseByte(line, 9 + length * 2);
            if ((byte)(sum + checksum) != 0)
                throw new InvalidDataException($"HEX checksum error: {line}");

            if (type == 0x00)
            {
                uint absolute = baseAddress + address;
                for (uint i = 0; i < length; i++)
                {
                    uint addr = absolute + i;
                    memory[addr] = data[i];
                    minAddress = Math.Min(minAddress, addr);
                    maxAddress = Math.Max(maxAddress, addr);
                }
            }
            else if (type == 0x01)
            {
                break;
            }
            else if (type == 0x04)
            {
                baseAddress = (uint)((data[0] << 8) | data[1]) << 16;
            }
        }

        if (memory.Count == 0) return Array.Empty<byte>();

        var result = new byte[maxAddress - minAddress + 1];
        Array.Fill(result, (byte)0xFF);
        foreach (var item in memory)
            result[item.Key - minAddress] = item.Value;

        return result;
    }

    private static byte ParseByte(string line, int index)
    {
        return Convert.ToByte(line.Substring(index, 2), 16);
    }
}
