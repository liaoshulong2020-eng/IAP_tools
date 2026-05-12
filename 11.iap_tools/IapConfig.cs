// IapConfig.cs - 添加续传配置
using System;

namespace IapUpgradeTool
{
    public class IapConfig
    {
        public byte ModbusAddr { get; set; }
        public string AppFile { get; set; }
        public int Baudrate { get; set; }
        public uint FlashBaseAddr { get; set; }
        public uint FlashMaxSize { get; set; }
        public uint AppBaseAddr { get; set; }
        public uint AppMaxSize { get; set; }
        public uint ArgBaseAddr { get; set; }
        public int ReadWriteSize { get; set; }

        // 新增续传功能相关字段
        public bool EnableResume { get; set; } = true;  // 是否启用续传
        public uint LastWrittenAddr { get; set; } = 0;  // 上次写入的地址
        public int LastWrittenProgress { get; set; } = 0; // 上次的进度
    }
}