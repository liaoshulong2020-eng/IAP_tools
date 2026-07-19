# ZHLD 259B / TAE32G5800D IAP 当前发布包

生成日期：2026-07-19

## 目录说明

- 01_上位机：直接运行 CAN_TOOLS_IAP.exe，DLL/JSON 文件必须和 exe 放在同一目录。
- 02_JLink合并烧录码：PFC/LLC 的 APP + bootloader 合并烧录文件，JLink 首次烧录用这个。
- 03_在线升级APP：只包含 APP 区域，使用 CAN IAP 在线升级时选择这个。

## 推荐使用顺序

1. 首次烧录 LLC：02_JLink合并烧录码\01_LLC_JLINK合并Bootloader_259B_5800D.bin
2. 首次烧录 PFC：02_JLink合并烧录码\02_PFC_JLINK合并Bootloader_259B_5800D.bin
3. 打开上位机：01_上位机\CAN_TOOLS_IAP.exe
4. LLC 在线升级时选择：03_在线升级APP\LLC_IAP在线升级APP_259B_5800D.bin
5. PFC 在线升级后续走 CAN上位机 -> LLC -> UART -> PFC bootloader 链路，选择：03_在线升级APP\PFC_IAP在线升级APP_259B_5800D.bin

## 本次 LLC 修改

LLC 接收 PFC 上报帧由整帧固定长度接收改为单字节状态机同步解析，用于修复切换原副边通讯后 PFC 参数一直显示 --- 的问题。
