# C/D 版本 LLC/PFC IAP 开发计划

## 最终目标

形成 C 版本、D 版本两套完整在线升级能力：

- LLC 支持通过外部 CAN 直接在线升级。
- PFC 支持通过外部 CAN 到 LLC，再由 LLC UART 转发到 PFC 在线升级。
- CAN 上位机统一管理 LLC/PFC、固定 ID/节点 ID、断点续传、失败重试、日志和发布文件。

## 当前基线

当前先以 D 版本 259B LLC 为基线：

- D 版本 LLC bootloader：`3. bootloader/D版本bootloader/BootLoader-259B_llc_D`
- D 版本 LLC APP：`5.原副边程序/D版本_TAE32G5800D/259B_A4/LLC_1.259B_LLC_A4_V_1_2_0`
- CAN 上位机：`CAN_TOOLS_NET8.0`

当前已回退为 `125kbps`，优先保证可靠升级。

## 阶段 1：D 版本 LLC 在线升级闭环

目标：D 版本 LLC 可通过 CAN 固定 ID `0xAA55` 稳定在线升级。

检查项：

- APP 正常运行时 CAN 波特率与上位机一致。
- bootloader IAP 模式 CAN 波特率与上位机一致。
- APP 能收到进入 IAP 命令并复位。
- bootloader 能回复进入 IAP ACK。
- Flash 写入不跨扇区失败。
- 写入失败时断点回退到可靠扇区边界。
- 升级期间暂停自动读取和普通控制命令。

## 阶段 2：D 版本 PFC 在线升级

目标：外部 CAN 到 LLC，LLC 通过 UART 转发 PFC IAP。

检查项：

- LLC APP/bootloader 都能识别 PFC 目标地址 `1`。
- LLC 到 PFC UART 通讯稳定。
- PFC 进入 IAP 命令需要比单字节 `0xFF` 更安全，建议增加帧头、命令、参数、CRC 和帧尾。
- PFC bootloader 独立处理 IAP 写入和校验。
- 上位机日志能区分 LLC/PFC 升级流程。

## 阶段 3：C 版本 LLC/PFC 回归

目标：C 版本 227B-T 复用同一套上位机流程。

检查项：

- C 版本 LLC/PFC UART 通讯按 259B 的应用层方式打通。
- 底层 CAN/UART 初始化参考 C 版本官方 SDK。
- C 版本 LLC bootloader 保留旧功能并补齐 PFC 转发能力。
- C 版本 PFC bootloader 做安全进入和可靠写入。

## 阶段 4：统一 bootloader 方案

目标：在验证 C/D、LLC/PFC 都能升级后，再考虑统一 bootloader。

建议方向：

- 公共 IAP 协议层统一。
- Flash 擦写、CAN、UART 底层按芯片版本隔离。
- LLC/PFC 的差异用目标角色配置控制。
- 保留编译期配置项，例如：
  - `CHIP_VERSION_C` / `CHIP_VERSION_D`
  - `ROLE_LLC` / `ROLE_PFC`
  - `ENABLE_CAN_IAP`
  - `ENABLE_UART_FORWARD`

## 工具共用策略

先把 PC 端工具和烧录文件生成工具统一起来，bootloader 暂时不强行合并。

- CAN 上位机 `CAN_TOOLS_NET8.0`：C/D、LLC/PFC 共用，通过项目预设区分。
- 串口 IAP 工具 `11.iap_tools`：C/D、LLC/PFC 共用，作为旧流程对照和串口升级入口。
- 合并工具 `IapFileCreator`：C/D、LLC/PFC 共用，后续作为统一烧录码生成入口。
- 串口/CAN 转发板：如果转发板硬件不变，可以共用；如果转发板 MCU 或底层驱动变化，需要分版本。
- bootloader：先保持 C/D、LLC/PFC 分目录验证，等四套路径都跑通后再抽公共协议层。

详细结论见 `docs/工具共用性分析.md`。

## 暂不做的事

- 暂不把 C/D bootloader 强行合并成一个工程。
- 暂不把正常运行 125kbps、升级 500kbps 作为默认方案。
- 暂不删除历史备份目录，等 GitHub 替代版本稳定后再清理。
