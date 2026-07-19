# CAN IAP Tools

本仓库用于维护 TAE32G5800 C/D 版本芯片的在线升级工具链，目标是形成两套完整 IAP 方案：

- C 版本芯片：LLC bootloader、PFC bootloader、LLC/PFC APP 配套 IAP 包。
- D 版本芯片：LLC bootloader、PFC bootloader、LLC/PFC APP 配套 IAP 包。

当前优先验证对象是 `D版本_TAE32G5800D / 259B_A4 / LLC`，上位机使用 `CAN_TOOLS_NET8.0`。

## 当前状态

| 项目 | 状态 | 说明 |
| --- | --- | --- |
| D 版 LLC bootloader | 已整理 | 当前路径为 `3. bootloader/D版本bootloader/BootLoader-259B_llc_D` |
| D 版 LLC APP IAP | 已整理 | 当前路径为 `5.原副边程序/D版本_TAE32G5800D/259B_A4/LLC_1.259B_LLC_A4_V_1_2_0` |
| D 版 PFC bootloader | 待验证 | 目录已预留，后续按 LLC 方式完善 |
| C 版 LLC/PFC bootloader | 待回归 | 保留原始 C 版工程，后续统一检查 UART/CAN/IAP 行为 |
| CAN 上位机 | 已整理 | 当前路径为 `CAN_TOOLS_NET8.0` |
| 串口 IAP 工具 | 保留 | 当前路径为 `11.iap_tools`，用于对照旧升级流程 |

## 主要目录

- `CAN_TOOLS_NET8.0`
  - 新 CAN 上位机工程，支持固定 ID `0xAA55` 和节点 ID `0xA0000~0xA0007` 在线升级。
  - 当前默认 CAN 仲裁波特率已还原为 `125kbps`。

- `3. bootloader`
  - `C版本bootloader`
    - `BootLoader-5800_llc_C`
    - `BootLoader-5800_pfc_C`
  - `D版本bootloader`
    - `BootLoader-259B_llc_D`
    - `BootLoader-259B_pfc_D`
  - `历史备份_未整理`
    - 原始散落 bootloader 备份，只作为追溯用。

- `5.原副边程序`
  - `C版本_TAE32G5800C`
    - 227B-T A03 LLC/PFC APP 工程。
  - `D版本_TAE32G5800D`
    - 259B A4 LLC/PFC APP 工程。
  - `历史旧版_原目录`
    - 未整理前的原始目录备份，只作为追溯用。

- `6.官方SDK`
  - C/D 芯片官方 SDK，用于对照底层外设初始化。

- `docs`
  - 版本矩阵、开发计划、工具共用性分析、验证记录。

- `release`
  - `current_259B_D_CAN_IAP` 为当前已验证的 259B D 版本 CAN IAP 上位机运行包。
  - 当前版本优化了固定 ID `0xAA55` 进入 LLC IAP 的握手过程，APP 复位后会连续补发进入 IAP 命令，提高个别模块进入 bootloader 的成功率。

## 本地发布规则

桌面固定导出位置：

- 上位机入口：`C:\Users\10412\Desktop\CAN_IAP_发布\烧录码\当前使用_259B_D版本_CAN升级PFC\CAN_TOOLS_IAP.exe`
- GitHub 运行包：`release/current_259B_D_CAN_IAP`

后续生成新的烧录码时，只保留最新一套文件。

## 当前验证建议

1. 先确认普通 CAN 通讯可以读取 LLC 数据。
2. LLC 在线升级优先选择固定 ID `0xAA55`。
3. 如果日志显示“等待 bootloader 启动并确认 IAP，期间连续补发进入命令”，说明正在使用当前稳定版。
4. LLC 稳定后，再验证 LLC 经 UART 转发升级 PFC。
5. PFC 验证通过后，再推进 C/D 两版本统一 bootloader 方案。

更多细节见：

- `docs/版本矩阵.md`
- `docs/开发计划_C_D版本LLC_PFC_IAP.md`
- `docs/工具共用性分析.md`
