# 发布规则

本目录只保存发布规则说明，不建议把大量 `.bin/.hex/.exe` 编译产物提交到 GitHub。

本机固定发布位置：

- 上位机：`C:\Users\10412\Desktop\CAN_TOOLS_IAP.exe`
- 烧录码：`C:\Users\10412\Desktop\CAN_IAP_发布\烧录码`

发布原则：

1. 每次只保留最新一套烧录码。
2. 文件名必须标出芯片/项目/目标/波特率/用途。
3. JLink 合并烧录文件和在线升级 APP 文件分开命名。
4. 发布前先确认 APP 与 bootloader 的 CAN 波特率一致。
5. GitHub 只保存源码、配置、说明和必要依赖。

当前建议命名：

- `259B_LLC_V_1_2_0_20260711_125k_JLink烧录.bin`
- `259B_LLC_V_1_2_0_20260711_125k_在线升级.bin`

