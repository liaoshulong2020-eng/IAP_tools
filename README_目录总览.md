# 30.CAN_IAP_tools-main 目录总览

本仓库同时维护单 Bank IAP 和双 Bank IAP 两套独立方案。统一项目入口位于 `Projects`，两者是对等项目，不存在包含关系。

## IAP 项目入口

- `Projects/01.SingleBank_IAP`
  - 已有产品使用的单 Bank IAP 项目入口。
  - 为兼容旧工程，Bootloader、APP、上位机及发布产物继续保留原路径。
- `Projects/02.DualBank_IAP`
  - 独立的双 Bank 在线升级、回滚、Bank 管理及开发板验证项目。
  - 包含自己的固件、上位机、协议文档、可烧录成品和测试包。

## 主要目录分类

### 1. 上位机工具 (Host Tools)
- `12.HLD_CANFDToolPro`
  - **最新主上位机工程**（C++ / Qt 6）。
  - **已集成完整 IAP 在线升级功能**（包含二进制/HEX解析、CRC32校验、断点续传、跨机/转发升级），**正式替代了旧版 `11.iap_tools` 及旧 CAN 上位机**。
- `CAN_TOOLS_NET8.0`
  - CAN 上位机工程（.NET 8.0 历史开发版本，已被 `HLD_CANFDToolPro` 替代）。
- `11.iap_tools`
  - 旧串口 IAP 上位机工程（已被 `HLD_CANFDToolPro` 替代）。
- `2. 上位机`
  - 早期旧版上位机资料归档。

### 2. 嵌入式工程与 SDK (MCU Firmware & SDK)
- `3. bootloader`
  - C/D 版本、LLC/PFC Bootloader 芯片工程。
  - 当前有效入口：
    - `C版本bootloader/BootLoader-5800_llc_C`
    - `C版本bootloader/BootLoader-5800_pfc_C`
    - `D版本bootloader/BootLoader-259B_llc_D`
    - `D版本bootloader/BootLoader-259B_pfc_D`
  - 旧散落 Bootloader 已归档到 `历史备份_未整理`。
- `4. 转发板源程序`
  - 串口 IAP 转发板 / CommKit 嵌入式源程序。
- `5.原副边程序`
  - LLC / PFC APP 应用程序工程。已按 `C版本_TAE32G5800C` 和 `D版本_TAE32G5800D` 整理分类。
- `6.官方SDK`
  - TAE32G5800 C/D 芯片官方 SDK 驱动及例程。

### 3. 辅助打包与配置工具 (Packers & Config Tools)
- `4.boot_config`
  - Bootloader + APP 打包配置工具（已整理分类）：
    - `ZHLD_boot_config.exe` (主程序直接运行)
    - `1.应用程序/` (归档)
    - `2.源代码/` (C# 源码及数据)
    - `3.生成脚本/` (Python/Bat)
    - `4.文档/`
- `IapFileCreator`
  - IAP 固件打包 / 文件合并辅助工具。

### 4. 产物与日志 (Releases & Logs)
- `release`
  - 统一编译发布的固件、JLink 合并烧录码及在线升级包。
- `7.构建日志`
  - 归档编译与测试过程产生的全部 log 日志。
- `docs`
  - 各种系统设计文档与协议说明。
