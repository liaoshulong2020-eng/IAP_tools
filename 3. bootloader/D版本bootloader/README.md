# D 版本 Bootloader

本目录保存 TAE32G5800D 芯片 bootloader 工程。

## 当前目录

- `BootLoader-259B_llc_D`
  - 当前重点验证工程。
  - 外部 CAN 使用固定扩展 ID `0xAA55`。
  - 当前已回退为 `125kbps`。
  - 支持 LLC 自身 IAP。
  - 保留 LLC 到 PFC 的 UART 转发框架。

- `BootLoader-259B_pfc_D`
  - D 版本 PFC bootloader 预留/待完善工程。
  - 后续应强化进入 IAP 命令校验，避免单字节误触发。

## D 版本 LLC 关键文件

- CAN/UART 转发入口：
  - `BootLoader-259B_llc_D/modbus/modbus_iap.c`
- IAP Flash 擦写：
  - `BootLoader-259B_llc_D/main/iap.c`
- CAN 底层：
  - `BootLoader-259B_llc_D/devices/cans.c`
- UART 底层：
  - `BootLoader-259B_llc_D/devices/uart.c`

## 当前验证基线

- CAN 波特率：`125kbps`
- APP 起始地址：`0x08008000`
- IAP 参数区：`0x08007000`
- 固定 CAN ID：`0xAA55`
- LLC 地址：`2`
- PFC 地址：`1`

