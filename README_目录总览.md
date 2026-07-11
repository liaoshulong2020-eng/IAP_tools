# IAP_tools-main 目录总览

本目录按工具、bootloader、APP工程、官方SDK和构建产物重新整理。

## 主要目录

- `2. 上位机`
  - 旧版上位机资料。

- `3. bootloader`
  - C/D版本、LLC/PFC bootloader工程。
  - 当前有效入口：
    - `C版本bootloader/BootLoader-5800_llc_C`
    - `C版本bootloader/BootLoader-5800_pfc_C`
    - `D版本bootloader/BootLoader-259B_llc_D`
    - `D版本bootloader/BootLoader-259B_pfc_D`
  - 旧散落 bootloader 已归档到 `历史备份_未整理`。

- `4. 转发板源程序`
  - 串口IAP转发板/CommKit源程序。

- `4.boot_config`
  - bootloader + APP 合并生成 jlink 文件的工具。
  - 注意：该工具后续需要更新为支持 C/D、LLC/PFC 四套 bootloader。

- `5.原副边程序`
  - LLC/PFC APP工程。
  - 已按 C版本、D版本、项目型号整理。

- `6.官方SDK`
  - TAE32G5800 C/D芯片官方SDK。

- `7.构建日志`
  - 编译验证过程产生的日志。

- `11.iap_tools`
  - 旧串口IAP上位机。

- `CAN_TOOLS_NET8.0`
  - 新CAN上位机工程。

- `IapFileCreator`
  - IAP包/合并文件生成辅助工具。

## 当前验证优先级

1. 验证新CAN上位机升级 LLC。
2. 验证新CAN上位机经 LLC UART 转发升级 PFC。
3. 验证通过后，再更新 `4.boot_config` 支持 C/D、LLC/PFC 四套 bootloader。
4. 最后再统一 bootloader 源码。
