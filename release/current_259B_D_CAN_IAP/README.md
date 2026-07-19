# 259B D版本 CAN IAP 当前稳定版

更新时间：2026-07-19

## 使用文件

| 文件 | 用途 |
|---|---|
| `CAN_TOOLS.exe` | 推荐直接运行的上位机 |
| `CAN_TOOLS_IAP.exe` | 同一份上位机的 IAP 命名副本 |
| `CAN_TOOLS.dll` / `*.json` / `dll` | 运行依赖，需与 exe 放在同一目录 |

## 本次更新

- 保留 CAN 上位机固件升级页，页签位置恢复到原来的最后。
- 优化 LLC 固定 ID `0xAA55` 进入 IAP 的握手策略。
- APP 触发复位后，上位机会在等待 bootloader 启动期间连续补发进入 IAP 命令，提高个别模块复位后进入 bootloader 的命中率。
- 不修改写 Flash 协议和固件包格式。

## 当前使用建议

- LLC 自升级选择固定 ID `0xAA55`。
- 单机验证异常模块时，先确认普通 CAN 通讯正常，再执行 IAP。
- 如果仍出现“未收到 bootloader 进入确认”，下一步应在 bootloader/APP 中增加 IAP 标志位机制，避免完全依赖启动窗口。
