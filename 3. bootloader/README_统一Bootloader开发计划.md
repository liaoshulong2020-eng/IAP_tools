# 统一Bootloader开发计划

> 当前先不实施。本文件用于今天验证 C/D 版本 PFC、LLC 在线升级后，下一阶段继续开发“同一套 bootloader 源码”的依据。

## 当前状态

现阶段 bootloader 按芯片版本和板卡角色分开：

- `C版本bootloader/BootLoader-5800_llc_C`
- `C版本bootloader/BootLoader-5800_pfc_C`
- `D版本bootloader/BootLoader-259B_llc_D`
- `D版本bootloader/BootLoader-259B_pfc_D`

当前升级链路：

```text
外部CAN上位机
  -> LLC APP / LLC bootloader
    -> 升级LLC自身：CAN直接写LLC Flash
    -> 升级PFC：LLC bootloader通过UART0转发到PFC bootloader
```

当前原则：

- LLC bootloader 是 CAN 入口和 PFC 转发网关。
- PFC bootloader 主要通过 UART0 接收升级数据。
- C/D 版本底层库和时钟配置不同，不能混用。

## 统一目标

后续目标不是马上做一个完全相同的 bin，而是做成：

```text
同一套源码
  + 不同芯片版本配置
  + 不同板卡角色配置
  + 不同传输能力开关
```

最终希望同一套 bootloader 源码支持：

- CAN 升级本机
- UART 升级本机
- CAN -> UART 转发升级远端
- UART -> CAN 转发预留
- LLC / PFC 不同本地地址配置
- C / D 芯片版本底层适配

## 推荐源码结构

建议下一步新建一个统一工程目录，例如：

```text
3. bootloader/
  统一bootloader/
    Core/
      bl_iap_core.c/h          # 擦除、写入、CRC、跳转APP、升级状态机
      bl_protocol.c/h          # IAP帧解析、应答、超时处理
      bl_app_jump.c/h          # APP有效性检查、跳转
      bl_flash_layout.h        # Flash地址、APP地址、参数区地址

    Transport/
      bl_can.c/h               # CAN收发适配
      bl_uart.c/h              # UART收发适配
      bl_bridge.c/h            # CAN/UART透明转发

    Board/
      C_5800/
        board_clock.c/h
        board_can.c/h
        board_uart.c/h
      D_259B/
        board_clock.c/h
        board_can.c/h
        board_uart.c/h

    Config/
      bl_config_llc_c.h
      bl_config_pfc_c.h
      bl_config_llc_d.h
      bl_config_pfc_d.h

    Projects/
      Keil_LLC_C/
      Keil_PFC_C/
      Keil_LLC_D/
      Keil_PFC_D/
```

## 推荐配置宏

统一源码通过配置宏选择角色和能力：

```c
#define BL_CHIP_C_5800              1
#define BL_CHIP_D_259B              2

#define BL_ROLE_LLC_GATEWAY         1
#define BL_ROLE_PFC_NODE            2

#define BL_LOCAL_ADDR_LLC           2
#define BL_LOCAL_ADDR_PFC           1

#define BL_ENABLE_CAN_IAP           1
#define BL_ENABLE_UART_IAP          1
#define BL_ENABLE_CAN_UART_BRIDGE   1
#define BL_ENABLE_UART_CAN_BRIDGE   0
```

建议默认配置：

| 工程 | CAN本机升级 | UART本机升级 | CAN转UART | 本机地址 |
| --- | --- | --- | --- | --- |
| LLC C | 开 | 可开 | 开 | 2 |
| PFC C | 可预留 | 开 | 关 | 1 |
| LLC D | 开 | 可开 | 开 | 2 |
| PFC D | 可预留 | 开 | 关 | 1 |

## 分阶段实施计划

### 第1阶段：冻结现有可用功能

目标：今天验证通过后，先不要再大改现有 C/D 分离 bootloader。

要做：

- 记录 C/D LLC、PFC 当前可升级现象。
- 记录上位机版本、IAP包、CAN速率、UART速率。
- 确认 LLC 自升、PFC 转发升级分别是否稳定。
- 标记当前四个 bootloader 为“验证基线版本”。

交付物：

- `README_C_D在线升级链路说明.md` 更新验证结果。
- 每个可用 bootloader 的 bin/hex/axf 保留在对应 `Keil/Execute`。

### 第2阶段：抽离公共IAP核心

目标：把四个 bootloader 里相同的擦写 Flash、CRC、APP跳转、IAP状态机抽成公共模块。

要做：

- 对比 C/D、LLC/PFC 的 `modbus_iap.c`、flash 操作、APP跳转代码。
- 抽出 `Core/bl_iap_core.c/h`。
- 保持外部协议不变。
- 先让 LLC C 和 PFC C 使用同一个核心模块编译通过。

验收：

- LLC C CAN升级功能不变。
- PFC C UART升级功能不变。

### 第3阶段：抽离传输层

目标：把 CAN、UART、CAN/UART 转发拆成独立传输层。

要做：

- 抽出 `Transport/bl_can.c/h`。
- 抽出 `Transport/bl_uart.c/h`。
- 抽出 `Transport/bl_bridge.c/h`。
- 明确本机包和转发包的判断规则。

验收：

- LLC 仍可 CAN 升级自身。
- LLC 仍可 CAN -> UART 升级 PFC。
- PFC 仍可 UART 升级自身。

### 第4阶段：抽离板级适配

目标：C/D 芯片差异只留在 `Board/`。

要做：

- C版本使用官方 V1.2.3 底层库和 C 版时钟配置。
- D版本使用官方 V1.3.1 底层库和 D 版时钟配置。
- CAN/UART 引脚配置按板卡实际硬件放到 board 层。
- 禁止 D 版继续沿用 C 版 PLL1 配置。

验收：

- C/D 四个工程都能从统一源码编译。
- 生成的四个 bin 名称能清楚区分 C/D、LLC/PFC。

### 第5阶段：增强可靠性

目标：解决升级成功率和反复重启风险。

要做：

- 进入 IAP 命令只触发一次，避免 APP 和 bootloader 重复复位。
- bootloader 增加保持态/握手确认。
- 写 Flash 包增加序号、地址、长度、CRC 校验。
- 上位机超时后优先重发当前包，不立刻重新发送进入 IAP。
- 转发模式增加 UART 发送间隔和接收窗口。

验收：

- CAN 直升 LLC 稳定。
- CAN 经 LLC 转发升级 PFC 稳定。
- 断电/中断后重新升级不会卡死。

## 下次继续开发时的入口

下次开始时，先读取这几个文件：

1. `IAP_tools-main/3. bootloader/README_版本目录说明.md`
2. `IAP_tools-main/3. bootloader/README_C_D在线升级链路说明.md`
3. `IAP_tools-main/3. bootloader/README_统一Bootloader开发计划.md`

然后先做两件事：

1. 根据你实测结果更新“第1阶段：冻结现有可用功能”。
2. 从四个 bootloader 中对比 `modbus_iap.c`，开始抽离公共 IAP 核心。

## 暂不实施内容

以下内容等现有 C/D、LLC/PFC 在线升级验证稳定后再做：

- 不立刻合并四个 bootloader 工程。
- 不立刻改成一个通用 bin。
- 不立刻改变上位机协议。
- 不立刻改变 APP 和 bootloader 的 Flash 地址布局。
