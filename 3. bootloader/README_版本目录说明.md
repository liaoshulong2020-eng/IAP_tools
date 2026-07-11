# Bootloader 版本目录说明

本目录按芯片版本拆分为 C 版本和 D 版本，避免 TAE32G5800 C/D 版底层库、时钟配置和外设初始化混用。

## 目录结构

- `历史备份_未整理`
  - 旧的散落 bootloader 目录备份，仅用于对比，不作为当前开发入口

- `C版本bootloader/BootLoader-5800_llc_C`
  - 原 C 版本 LLC Bootloader
  - CAN IAP 通信

- `C版本bootloader/BootLoader-5800_pfc_C`
  - 原 C 版本 PFC Bootloader
  - UART0 与 LLC 通信

- `D版本bootloader/BootLoader-259B_llc_D`
  - 259B / TAE32G5800 D 版本 LLC Bootloader
  - 使用 V1.3.1 官方 D 版底层库
  - CAN1 引脚按 259B APP 工程配置为 PB8/PB9
  - 输出文件名: `BootLoader-259B-LLC-D`

- `D版本bootloader/BootLoader-259B_pfc_D`
  - 259B / TAE32G5800 D 版本 PFC Bootloader
  - 使用 V1.3.1 官方 D 版底层库
  - 参考官方 UART 例程保留 UART0 轮询收发方式，UART0 引脚沿用原 PFC Bootloader 的 PB6/PB7
  - 输出文件名: `BootLoader-259B-PFC-D`

## 公共地址约定

- Bootloader 区: `0x08000000 ~ 0x08007FFF`
- APP 起始地址: `0x08008000`
- IAP 参数区: `APP_BASE_ADDR - 0x1000`

APP 工程使能 IAP 时，ROM 起始地址应与 Bootloader 的 APP 起始地址保持一致，例如:

```c
#define __ROM_BASE      0x08008000
#define __ROM_SIZE      0x38000
```

## 注意事项

- C 版本工程继续保留原来的 C 版底层库和配置。
- D 版本工程已替换为 V1.3.1 官方 D 版底层库。
- D 版本时钟初始化不要继续沿用 C 版里的 PLL1 配置。
- Keil 工程路径中包含空格或中文目录时，使用工程内的 `AfterBuildHandler.bat` 生成并复制 bin/hex/axf 到 `Keil/Execute`。
