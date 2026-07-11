# 官方SDK目录说明

本目录存放 TAE32G5800 官方SDK，按芯片版本区分。

## 目录结构

- `C版本_TAE32G58xx_FW_ReleaseV1.2.3`
  - TAE32G5800 C版本参考SDK。
  - C版本 227B bootloader 和 APP 底层参考此版本。

- `D版本_TAE32G58xx_FW_ReleaseV1.3.1`
  - TAE32G5800 D版本参考SDK。
  - D版本 259B bootloader 和 APP 底层参考此版本。

- `TAE32G58xx_FW_ReleaseV1.3.1.zip`
  - D版本官方SDK压缩包备份。

## 注意事项

- C版本和D版本底层库、时钟初始化、外设配置不能混用。
- D版本 bootloader 不应继续沿用 C版本 PLL1 配置。
