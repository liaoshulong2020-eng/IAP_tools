# CAN IAP Tools 交接说明

更新时间：2026-07-19

本仓库用于维护 TAE32G5800 C/D 版本芯片的 CAN 上位机、串口 IAP 工具、LLC/PFC bootloader、LLC/PFC APP 在线升级相关代码。本文档是给后续开发对话或接手人员优先读取的总览，目标是不用翻历史聊天记录，也能继续开发和验证。

## 总目标

形成 C 版本和 D 版本都可用的一套 LLC/PFC 在线升级方案：

| 目标 | 说明 |
| --- | --- |
| LLC 自升级 | 外部 CAN 盒直接通过 CAN 升级 LLC APP。 |
| PFC 间接升级 | 外部 CAN 盒发给 LLC，LLC 通过 UART 转发到 PFC bootloader，完成 PFC APP 升级。 |
| 普通监控 | CAN 上位机能读取 LLC 数据、保护点、版本号等。 |
| 原副边通讯 | LLC 通过 UART 接收 PFC 数据，并转换后通过 CAN 上报给上位机。 |
| 调试/通讯模式切换 | LLC UART 需要能在 VOFA 调试模式和原副边通讯模式之间切换。 |
| 后续统一 bootloader | 远期规划为同一套 bootloader 支持 UART 升级、CAN 升级、CAN-UART 转发能力。当前先不重构统一版。 |

## 当前真实发布位置

桌面当前可运行上位机目录：

```text
C:\Users\10412\Desktop\ZHLD_CAN_IAP_v1.0
```

运行入口：

```text
C:\Users\10412\Desktop\ZHLD_CAN_IAP_v1.0\CAN_TOOLS_IAP.exe
```

GitHub 当前发布目录：

```text
release/CAN_TOOLS_IAP
```

当前 259B / TAE32G5800D 完整测试发布包：

```text
C:\Users\10412\Desktop\ZHLD_259B_5800D_IAP_当前发布
```

发布规则：这个目录是固定目录。每次生成新版本时，先删除旧内容，再把最新上位机和最新烧录码覆盖进去；不要再新建 `ZHLD_5800D_IAP_当前烧录码`、`当前使用_xxx` 之类临时目录。

该发布包必须包含：

| 目录 | 必须文件 | 用途 |
| --- | --- | --- |
| `01_上位机` | `CAN_TOOLS_IAP.exe` 及同目录 DLL/JSON 依赖 | CAN 上位机运行包。 |
| `02_JLink合并烧录码` | `01_LLC_JLINK合并Bootloader_259B_5800D.bin` | LLC APP + LLC bootloader 合并文件，JLink 首次烧录用。 |
| `02_JLink合并烧录码` | `02_PFC_JLINK合并Bootloader_259B_5800D.bin` | PFC APP + PFC bootloader 合并文件，JLink 首次烧录用。 |
| `03_在线升级APP` | `LLC_IAP在线升级APP_259B_5800D.bin` | LLC 在线升级时选择的 APP 文件。 |
| `03_在线升级APP` | `PFC_IAP在线升级APP_259B_5800D.bin` | PFC 经 LLC UART 转发在线升级时选择的 APP 文件。 |

注意：

- 上位机运行包不是烧录码，不再放到 `烧录码\当前使用_259B_D版本_CAN升级PFC` 这种目录。
- 上位机目录不绑定单一项目，不再使用 `259B`、`D版本`、`PFC` 作为上位机发布目录名。
- `iap_resume` 是上位机运行时生成的 IAP 断点状态目录，不是烧录码，不提交到 GitHub。

## 主要目录

| 路径 | 用途 | 当前说明 |
| --- | --- | --- |
| `CAN_TOOLS_NET8.0` | 新 CAN 上位机源码 | WinForms / .NET 8，当前主要使用这个。 |
| `release/CAN_TOOLS_IAP` | 当前可运行上位机发布包 | 只保留一个 `CAN_TOOLS_IAP.exe`，其它 DLL/JSON 是运行依赖。 |
| `11.iap_tools` | 旧串口 IAP 工具 | 用于对照串口转发板升级流程。 |
| `3. bootloader/C版本bootloader` | C 版本 bootloader | 包含 LLC/PFC C 版 bootloader。 |
| `3. bootloader/D版本bootloader` | D 版本 bootloader | 包含 259B LLC/PFC D 版 bootloader。 |
| `5.原副边程序/C版本_TAE32G5800C` | C 版本 APP 工程 | 227B-T A03 LLC/PFC APP。 |
| `5.原副边程序/D版本_TAE32G5800D` | D 版本 APP 工程 | 259B A4 LLC/PFC APP。 |
| `6.官方SDK` | 官方 SDK | C/D 芯片底层外设初始化对照。 |
| `docs` | 设计和计划文档 | 版本矩阵、开发计划、工具共用性分析。 |

## 已完成内容

### 1. 目录整理

- 将 C 版本和 D 版本 bootloader 分开：
  - `3. bootloader/C版本bootloader/BootLoader-5800_llc_C`
  - `3. bootloader/C版本bootloader/BootLoader-5800_pfc_C`
  - `3. bootloader/D版本bootloader/BootLoader-259B_llc_D`
  - `3. bootloader/D版本bootloader/BootLoader-259B_pfc_D`
- 将 APP 原副边程序分开：
  - C 版：`5.原副边程序/C版本_TAE32G5800C/227B-T_A03_新版本`
  - D 版：`5.原副边程序/D版本_TAE32G5800D/259B_A4`
- 227B 和 227B-T 已按“同一类项目”处理，保留 227B-T 目录作为 C 版新版本入口。
- 上位机发布包改为通用目录 `release/CAN_TOOLS_IAP`。

### 2. CAN 上位机

- 保留固件升级页 `IAP` 标签，并移动回原来的页签位置。
- 优化 IAP 页面布局，保留独立 IAP 日志区域、升级进度、全局日志。
- 固定 ID 模式下使用 `0xAA55`，不再要求选择节点号。
- 节点 ID 模式保留 `0xA0000~0xA0007`。
- 自动读取期间执行升级时，会暂停自动读取和普通控制命令，升级结束后恢复，避免读数命令干扰 IAP。
- IAP 进入 bootloader 阶段做过优化：
  - APP 复位后等待 bootloader 启动；
  - 等待期间可连续补发进入 IAP 命令；
  - 未收到 bootloader 确认时停止本次尝试，避免继续盲目写入；
  - 达到最大尝试次数后失败退出。
- CAN 自动读取已修改：
  - 不再固定使用 `0x20`；
  - 改为使用 CAN通道1/2 输入框中的通讯 ID。
- CAN 主机地址快捷按钮已修改：
  - 通道1快捷：`0x20`、`0xB0000`
  - 通道2快捷：`0x21`、`0xBB208`
  - 点击任意快捷按钮时，CAN通道1和CAN通道2输入框会同时切换到该地址。
- CAN通道1、CAN通道2默认勾选。
- 节点 0~节点 7 默认全不选；点击“全选”按钮可在全选/全不选之间切换，按钮文字不变。

### 3. LLC/PFC 程序和 bootloader

- D 版本 LLC APP 已按 IAP 地址方式处理过，并能通过 JLink 合并烧录后运行。
- 2026-07-19 继续优化 D 版 LLC 原副边通讯：PFC 上报帧接收由“整帧固定长度接收”改为“单字节状态机找帧头/帧尾”，避免切换模式或丢字节后一直错位，目标是修复 PFC 参数显示 `---`。
- D 版本 PFC APP 已确认 UART 功能选择为原副边通讯时，PFC 程序本身可以 UART 收发。
- LLC UART 硬件管脚确认：`PA9 / PA10`。
- LLC 通过 CAN 读取 LLC 自身数据、保护点已经可用。
- PFC 保护点/数据经 LLC 转换上报到上位机仍有问题，见“当前问题”。
- 旧 LLC bootloader 中已保留 CAN-UART 转发功能，因为后续 PFC 在线升级依赖它。
- PFC bootloader 后续可以比 LLC bootloader 做得更严格，增加更可靠的校验和进入条件。

### 4. 发布和 GitHub

- 当前 GitHub 分支：`main`。
- 最近与上位机相关的提交：
  - `8fdf62e Use configured CAN master IDs for polling`
  - `4f2f071 Rename generic CAN IAP release package`
  - `6d1e46e Restore CAN IAP release binaries`
- 当前上位机发布包已经上传到 `release/CAN_TOOLS_IAP`。

## 当前协议整理

### CAN ID 分类

| CAN ID | 类型 | 用途 | 支持命令 |
| --- | --- | --- | --- |
| `0x20` | 标准主机/广播 ID | 常用 CAN 主机地址/广播查询入口 | 与 `0xB0000` 支持的普通命令相同。 |
| `0xB0000` | 扩展主机/广播 ID | 常用扩展帧主机地址/广播查询入口 | 与 `0x20` 支持的普通命令相同。 |
| `0x21` | 通道2标准主机/广播 ID | CAN通道2快捷地址 | 与普通查询/控制命令一致，需按实际设备协议验证。 |
| `0xBB208` | 通道2扩展主机/广播 ID | CAN通道2快捷地址 | 与普通查询/控制命令一致，需按实际设备协议验证。 |
| `0xA0000~0xA0007` | 节点扩展 ID | 节点 0~7 独立通讯 ID | 支持普通查询/控制命令，也用于节点 ID 模式 IAP。 |
| `0xAA55` | 固定扩展 ID | 固定 ID IAP 升级入口 | 支持 LLC 固定 ID IAP 升级命令。 |

### 普通命令范围

普通 CAN 命令包括但不限于：

| 命令 | 用途 |
| --- | --- |
| 查询命令 | 自动读取电压、电流、状态、版本等。 |
| 启动/停止命令 | 普通控制。 |
| 保存/读取 Flash 参数 | 参数保存和加载。 |
| 查询保护点 | 读取 LLC/PFC 保护点参数。 |
| UART 模式选择 | 切换 VOFA 调试模式和原副边通讯模式。 |

说明：

- `0x20` 和 `0xB0000` 当前作为普通通讯入口，支持的普通命令应保持一致。
- `0xA0000~0xA0007` 是设备节点自己的 ID，支持普通命令和节点 ID 模式 IAP。
- `0xAA55` 是固定 ID IAP 入口，升级时不需要选择节点。
- 当前上位机自动读取已经改为按输入框 ID 发送，不再强制 `0x20`。

### IAP 基本流程

| 阶段 | 行为 |
| --- | --- |
| 进入 IAP | 上位机发送进入 IAP 命令，APP 触发复位。 |
| 等待 bootloader | 上位机等待 bootloader 启动并回确认。 |
| 写 Flash | 上位机按块发送固件数据，bootloader 擦写 APP 区。 |
| 校验 | 固件包包含 CRC32，上位机日志显示加载 CRC；bootloader 侧校验能力需继续确认/增强。 |
| 结束/跳转 | 写入完成后进入 APP。 |

## 当前状态

| 模块 | 当前状态 | 说明 |
| --- | --- | --- |
| CAN 上位机普通读取 | 可用 | LLC 数据读取已验证。 |
| CAN 上位机 IAP 页 | 可用 | 固定 ID 和节点 ID 界面都保留。 |
| LLC 固定 ID 在线升级 | 基本可用 | 10 个模块中大部分可升级，但有个别模块仍进入 bootloader 成功率低。 |
| LLC 保护点读取 | 可用 | 已能显示 LLC 保护点。 |
| PFC UART 本体收发 | 已确认 | 断开 LLC 单独看 PFC，PFC 有 UART 数据。 |
| PFC 数据经 LLC 上报 | 待复测 | 2026-07-19 已将 LLC 侧 PFC UART 接收改为单字节同步解析，已生成测试烧录码，需实物验证 PFC 区域是否从 `---` 变为数据。 |
| CAN 升级 PFC | 未完成 | 需要 LLC CAN-UART 转发 + PFC bootloader 共同验证。 |
| C 版本原副边通讯 | 待回归 | 计划参考 259B 的 UART 应用层方式，底层参考 C 版官方 SDK。 |
| D 版本 PFC bootloader | 待完整验证 | 后续重点。 |

## 当前问题和风险

### 1. 个别 LLC 模块升级时反复重启

现象：

- 大多数模块可以升级；
- 约 2/10 模块在固定 ID `0xAA55` 升级时，进入 IAP 后未收到 bootloader 确认；
- 日志类似：

```text
固定ID: 发送进入 IAP 命令，触发 APP 复位...
固定ID: 等待 bootloader 启动并确认 IAP...
固定ID: 未收到 bootloader 进入确认，停止本次尝试，避免继续反复复位
```

当前判断：

- 烧录码同一批，单机固定 ID，问题不一定是 ID 冲突。
- 可能与 bootloader 启动窗口、APP 复位时序、电源掉电恢复、CAN 初始化时间、或进入 IAP 标志位机制有关。
- 上位机只能改善握手和重试策略，根因很可能需要 bootloader/APP 增加可靠进入标志。

后续建议：

- 在 APP 收到进入 IAP 后写入一处可靠标志位，复位后 bootloader 优先检查该标志。
- bootloader 进入 IAP 后持续周期回应握手，不只依赖短窗口。
- 上位机在进入 IAP 后先等待确认，确认不到就停止，不盲写。

### 2. PFC 数据没有经 LLC 显示到上位机

现象：

- PFC 单独断开 UART 查看时有数据输出。
- LLC 通过 CAN 可读取自身数据和保护点。
- 上位机 PFC 参数区域仍显示 `---`，说明 PFC->LLC->CAN->上位机没有闭环。

需要检查链路：

| 环节 | 需要确认 |
| --- | --- |
| PFC UART TX/RX | PFC 是否按原副边协议周期发送。 |
| LLC UART PA9/PA10 | 是否正确初始化 RX/TX、中断/DMA、波特率、校验位。 |
| LLC UART 模式 | 是否确实切到原副边通讯，而不是 VOFA。 |
| LLC 协议解析 | 是否按 259B 原副边协议解析 PFC 帧。 |
| LLC 数据映射 | PFC 数据是否写入 CAN 上报结构。 |
| CAN 上位机解析 | 上位机是否按正确命令/字段刷新 PFC 表格。 |

2026-07-19 已完成的针对性修改：

- 修改文件：`5.原副边程序/D版本_TAE32G5800D/259B_A4/LLC_1.259B_LLC_A4_V_1_2_0/APP/source/pri_sec_commun_app.c`
- 原逻辑：`LL_UART_Receive_IT(PFC_COMM_UART, pfc_comm_rx_buf, PFC_COMM_FRAME_TOTAL_SIZE)` 收满一整帧后直接解析。
- 新逻辑：`LL_UART_Receive_IT(..., &pfc_comm_rx_byte, 1)` 每次收 1 字节，通过 `0x55 / 0x02 / LEN / ... / 0xAA` 状态机自动重新同步。
- 验证文件已导出到：`C:\Users\10412\Desktop\ZHLD_5800D_IAP_当前烧录码`
- 先烧 `01_LLC_JLINK先烧这个_5800D_PFC_UART解析修复.bin`，或使用 `02_LLC_IAP在线升级_5800D_PFC_UART解析修复.bin` 在线升级后复测 PFC 参数。

### 3. UART 模式切换仍需实测

需求：

- 调试模式：使能 VOFA。
- 原副边通讯模式：LLC 接收 PFC 输入电压、电流、保护点等数据。

当前已有上位机切换入口，但需要继续确认：

- 模式命令是否发到正确 CAN ID；
- LLC APP 是否收到并保存/应用；
- 切换后是否需要复位；
- VOFA 和原副边通讯是否完全互斥。

### 4. PFC 间接升级尚未闭环

最终链路：

```text
CAN上位机 -> CAN盒 -> LLC CAN -> LLC UART -> PFC UART -> PFC bootloader
```

当前还没完成完整闭环验证。下一步应先确保 PFC 数据通讯闭环，再验证 PFC IAP。

## 下一步开发计划

### 短期优先级

1. 修复 PFC 数据显示：
   - 从 PFC UART 输出开始；
   - 检查 LLC UART 接收；
   - 检查 LLC 协议解析；
   - 检查 LLC CAN 上报；
   - 检查上位机 PFC 表格刷新。
2. 验证 UART 模式切换：
   - VOFA 调试模式；
   - 原副边通讯模式。
3. 完成 D 版本 PFC 间接升级：
   - PFC bootloader 进入 IAP；
   - LLC 转发 IAP 命令；
   - 上位机 CAN 升级 PFC；
   - 输出 PFC 的 JLink 合并烧录文件和 IAP APP 文件。
4. 提高 LLC 在线升级成功率：
   - 继续优化 bootloader 进入确认机制；
   - 必要时实现 APP->bootloader 的 IAP 标志位。

### 中期目标

1. C 版本 227B-T：
   - 参考 259B UART 应用层协议；
   - 底层参考 C 版本官方 SDK；
   - 打通 LLC/PFC 原副边通讯；
   - 打通 LLC/PFC IAP。
2. D 版本 259B：
   - 完成 LLC/PFC 两侧 JLink 合并烧录码；
   - 完成 LLC/PFC 两侧在线升级 APP 文件；
   - 完成 CAN 上位机一键选择 LLC/PFC 升级目标。

### 长期规划

统一 bootloader 能力，但暂不实施：

| 能力 | 说明 |
| --- | --- |
| UART 直接升级 | 兼容旧串口 IAP 工具。 |
| CAN 直接升级 | LLC 自升级。 |
| CAN-UART 转发升级 | LLC 转发给 PFC bootloader。 |
| 安全进入 IAP | 进入命令不应只靠单字节 `0xFF`，应增加帧头、长度、命令、目标、CRC/校验。 |
| 写入校验 | 每包 ACK、整包 CRC、地址范围检查、写后校验。 |
| 断点续传 | 可保留，但必须先保证地址、CRC、固件版本一致。 |

## 开发注意事项

- 不要随意删除 `3. bootloader/历史备份_未整理` 和 `5.原副边程序/历史旧版_原目录`，它们用于追溯。
- LLC 旧 bootloader 已有转发功能，不能简单还原成最原始版本，否则 PFC 升级链路会断。
- PFC bootloader 可以在不影响旧产品兼容的前提下加强安全性。
- 如果更改 CAN 波特率，必须同时确认 APP、bootloader、上位机、CAN 盒配置一致。当前主要按 `125kbps` 使用。
- 发布上位机时，保留 `CAN_TOOLS_IAP.exe` 和同目录 DLL/JSON 依赖；不要只复制 exe。
- `CAN_TOOLS.pdb` 只是调试符号，不是运行必需文件。

## 新对话继续开发建议

新的 Codex 对话接手时，建议先读取：

1. `README.md`
2. `docs/开发计划_C_D版本LLC_PFC_IAP.md`
3. `docs/工具共用性分析.md`
4. `docs/版本矩阵.md`
5. `CAN_TOOLS_NET8.0/MainForm.cs`
6. D 版 LLC/PFC APP 和 bootloader 的 UART/CAN/IAP 相关文件

推荐第一件事：

```text
检查 PFC -> LLC -> CAN上位机 的数据链路为什么 PFC 参数仍显示 ---
```

其次再继续：

```text
验证 CAN上位机 -> LLC -> PFC bootloader 的间接 IAP 升级闭环
```
