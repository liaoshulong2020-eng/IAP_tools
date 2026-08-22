# 5800D Boot 配置工具

## 直接运行

运行 `release/ZHLD_boot_config_5800D.exe`。

工具默认选择 `5800D` 和 `259B-LLC`，也可以将目标切换为 `PFC`。选择 LLC/PFC 时，项目名称会在 `259B-LLC` 与 `259B-PFC` 之间同步切换。

## 内嵌 Bootloader

- LLC：`BootLoader-259B-LLC-D.bin/.hex`
- PFC：`BootLoader-259B-PFC-D.bin/.hex`

生成到目标 Keil 工程时仍使用 `BootLoader-5800.bin/.hex` 文件名，以兼容已有的 `AfterBuildHandler.bat`、`MergeIapFiles.ps1` 和 JSON 配置。

## 地址配置

- Bootloader 最大空间：32 KB
- APP 起始地址：`0x08008000`
- APP 最大空间：224 KB
- Scatter 文件：`tae32g58xx_ac6_flash.sct`

工具会配置或生成 `AfterBuildHandler.bat`、`MergeIapFiles.ps1`、IAP JSON、Bootloader BIN/HEX，并调整在线/离线模式的 Scatter 地址。

## 兼容性

原有 `5800` 和 `5300` 选项仍然保留。只有 `5800D` 选项启用独立的 LLC/PFC 目标选择。
