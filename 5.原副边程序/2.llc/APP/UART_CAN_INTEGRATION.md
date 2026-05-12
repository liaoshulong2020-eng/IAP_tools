# UART + CAN 整合说明

## 功能概述

本项目在原有副边CAN上报功能基础上，整合了UART原副边通讯功能：
- 副边(LLC)通过 **UART0** 接收原边(PFC)发来的保护点及状态数据
- 副边(LLC)通过 **CAN** 将原边保护点数据转发给上位机

## 新增文件

| 文件 | 说明 |
|------|------|
| `APP/include/uart_app.h` | UART底层驱动头文件 |
| `APP/source/uart_app.c`  | UART底层驱动（DMA收发，UART0，115200bps） |
| `APP/include/pri_sec_commun.h` | LLC↔PFC通讯协议头文件 |
| `APP/source/pri_sec_commun.c`  | LLC↔PFC通讯协议实现（解析PFC数据帧） |

## 修改文件

| 文件 | 修改内容 |
|------|---------|
| `APP/include/can_app.h` | 新增 `CMD_PFC_PROTECT(0x3E)`、`pfc_vin_protect_TypeDef`、`pfc_vout_protect_TypeDef`、`can_pfc_TypeDef` |
| `APP/source/can_app.c`  | 新增 `CMD_PFC_PROTECT` 命令处理、`can_send_pfc_protect_data()` 函数 |
| `APP/source/init_app.c` | 在 `init_all_app()` 中添加 `user_uart_init(USER_UART)` 调用 |
| `ProjectTemplate/User/Inc/main.h` | 添加 `#include "uart_app.h"` 和 `#include "pri_sec_commun.h"` |

## 通讯协议

### UART（副边接收原边数据）

**PFC → LLC 帧格式（63字节）：**
```
[0]    = 0x55（帧头）
[1]    = 0x02（命令字）
[2]    = 58（数据长度）
[3..60]= PFC_RECEIVED_DATA_TypeDef（58字节）
[61]   = XOR校验（对[1..60]共60字节）
[62]   = 0xAA（帧尾）
```

**PFC_RECEIVED_DATA_TypeDef 包含的保护点：**
- `vin_under_voltage_set` — PFC输入欠压点
- `vin_over_voltage_set`  — PFC输入过压点
- `vin_on_voltage_set`    — PFC输入启动电压
- `vout_over_voltage_sw`  — PFC输出过压软件点
- `bus_ovp_point_hw`      — PFC母线过压硬件点
- `ipfc_ocp_current_sw`   — PFC过流软件点

### CAN（副边上报原边保护点给上位机）

**上位机查询命令：** `0x3E`（`CMD_PFC_PROTECT`）

**副边响应帧1（返回码 0xBE）：PFC输入电压保护点**
```
[0] frame_count
[1] 0xBE
[2] pfc_vin_uvp_low   — PFC输入欠压点低8位 (mV)
[3] pfc_vin_uvp_high  — PFC输入欠压点高8位 (mV)
[4] pfc_vin_ovp_low   — PFC输入过压点低8位 (mV)
[5] pfc_vin_ovp_high  — PFC输入过压点高8位 (mV)
[6] pfc_vin_on_low    — PFC输入启动电压低8位 (mV)
[7] pfc_vin_on_high   — PFC输入启动电压高8位 (mV)
```

**副边响应帧2（返回码 0xBF）：PFC输出电压及过流保护点**
```
[0] frame_count
[1] 0xBF
[2] pfc_vout_ovp_sw_low   — PFC输出过压软件点低8位 (mV)
[3] pfc_vout_ovp_sw_high  — PFC输出过压软件点高8位 (mV)
[4] pfc_bus_ovp_hw_low    — PFC母线过压硬件点低8位 (mV)
[5] pfc_bus_ovp_hw_high   — PFC母线过压硬件点高8位 (mV)
[6] pfc_ocp_sw_low        — PFC过流软件点低8位 (0.1A)
[7] pfc_ocp_sw_high       — PFC过流软件点高8位 (0.1A)
```

## 上位机（CAN_TOOLS_NET8.0）修改

| 修改内容 | 说明 |
|---------|------|
| `DeviceData` 新增字段 | `PFC_Vin_UnderVoltage_Point`、`PFC_Vin_OverVoltage_Point`、`PFC_Vin_On_Voltage`、`PFC_Vout_OverVoltage_Soft_Point`、`PFC_Bus_OVP_Hard_Point`、`PFC_OCP_Soft_Point` |
| `ParseTempProtectData(0xBE)` | 改为解析PFC输入电压保护点 |
| `ParsePFCVoutProtectData(0xBF)` | 新增，解析PFC输出电压及过流保护点 |
| `ParseAndDisplayData` | 新增 `0xBF` 分发 |
| `RefreshProtectGrid` | PFC表格从"---"改为显示实际数据 |

## 使用方法

1. 将 `uart_app.c`、`pri_sec_commun.c` 添加到工程编译列表
2. 确保 `variables_define_app.h` 中 `#define UART_FUNC 1`
3. 硬件连接：UART0 TX/RX 连接到原边PFC的对应引脚
4. 上位机点击"查询保护点"按钮，即可在PFC标签页看到原边保护点数据
