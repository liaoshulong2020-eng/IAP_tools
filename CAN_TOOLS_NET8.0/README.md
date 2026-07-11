# CAN_TOOLS_NET8.0

新 CAN 上位机工程，用于 5800 平台参数读取、控制和 CAN IAP 在线升级。

## 当前 IAP 配置

- 默认 CAN 仲裁波特率：`125kbps`
- 固定 IAP CAN 扩展 ID：`0xAA55`
- 节点 IAP CAN 扩展 ID：`0xA0000~0xA0007`
- LLC IAP 地址：`2`
- PFC IAP 地址：`1`
- APP 起始地址：`0x08008000`
- IAP 参数区：`0x08007000`
- 当前写包大小：`128 bytes`

## 可靠升级策略

- 升级期间暂停自动读取和普通控制命令。
- 固定 ID 模式不需要选择节点。
- 写入失败时记录断点。
- 断点续传回退到可靠扇区起点，避免从半写入地址继续。
- 上位机接收 ACK 后再发下一包。

## 本地发布

发布后的桌面入口：

`C:\Users\10412\Desktop\CAN_TOOLS_IAP.exe`

运行依赖：

`dll/ControlCANFD.dll`

源码工程中保留 `dll/ControlCANFD.dll`，用于 GitHub 拉取后能直接运行/调试。

