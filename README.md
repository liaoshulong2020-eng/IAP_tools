# IAP_tools

5800 电源项目在线升级工具与配套资料仓库，包含 CAN 上位机、串口 IAP 工具资料、Bootloader、转发板程序、IAP 包生成工具和相关设计文档。

## 主要功能

- CAN 上位机：支持 CAN 设备连接、数据监控、参数设置、参数列表和固件升级。
- CAN 直接 IAP：支持通过 CAN 盒直接升级 LLC/PFC 固件。
- 节点 ID 升级：支持扩展帧 ID `0xA0000~0xA0007`，可选择节点 0~节点 7 顺序升级。
- 固定 ID 升级：支持固定扩展帧 ID `0xAA55`，用于兼容固定 ID 的 IAP 流程。
- 升级保护：升级开始时暂停自动读取和普通控制命令，避免升级期间普通 CAN 指令干扰 IAP。
- 可靠重试：进入 IAP、写 Flash、写校验参数等流程带超时、确认和自动重试。
- IAP 日志：固件升级页内显示最近 IAP 日志，同时保留底部全局日志。
- 串口 IAP 资料：保留旧串口升级工具和转发板相关资料，便于对比 CAN 直接升级与串口转发升级差异。

## 目录说明

```text
CAN_TOOLS_NET8.0/      新版 CAN 上位机源码，目标框架 net8.0-windows
11.iap_tools/          串口 IAP 工具资料
2. 上位机/             原上位机相关资料
3. bootloader/         5800 Bootloader 源码与分析文档
4. 转发板源程序/       串口/CAN 转发板程序
4.boot_config/         Boot 配置与说明
5.原副边程序/          原 PFC/LLC 应用程序资料
IapFileCreator/        IAP 文件生成工具
PFC在线升级设计文档.md  PFC 在线升级设计说明
IAP说明文档            IAP 使用说明
```

## CAN 上位机固件升级说明

1. 打开 `CAN_TOOLS_NET8.0` 工程，或使用已编译的 `CAN_TOOLS.exe`。
2. 在 `CAN设置` 页启动 CAN。
3. 进入 `固件升级` 页。
4. 选择目标：`LLC` 或 `PFC`。
5. 选择 CAN 通道。
6. 选择 CAN ID 模式：
   - `节点ID 0xA0000~0xA0007`：用于节点 0~节点 7 顺序升级。
   - `固定ID 0xAA55`：用于固定 ID 升级，不需要选择节点。
7. 选择 `.bin` 固件文件。
8. 点击 `批量升级` 或 `固定ID升级`。

## 当前 CAN IAP 逻辑

- APP 起始地址：`0x08008000`
- IAP 参数/校验区：`0x08007000`
- 固定 CAN ID：`0xAA55`
- 节点 CAN ID：`0xA0000~0xA0007`
- LLC 包内目标地址：`2`
- PFC 包内目标地址：`1`

升级时，上位机会先发送进入 IAP 命令，确认 Bootloader 响应后再开始写 Flash。写入完成后会写入 APP 大小和 CRC 校验信息，用于 Bootloader 判断 APP 是否有效。

## 最近更新

### 2026-07-10

- 优化 CAN 直接 IAP 可靠性：
  - 写 Flash 前必须等待 Bootloader IAP 确认。
  - 避免未进入 Bootloader 时盲目写入导致设备反复复位。
  - 增加 CAN 通道恢复、超时和自动重试处理。
- 优化 CAN IAP 速度：
  - 写入块大小调整为 96 字节。
  - CAN 帧间隔调整为 2 ms。
  - IAP 包间隔调整为 8 ms。
  - 减少接收帧日志刷屏。
- 优化固件升级页界面：
  - 修复笔记本高 DPI/不同分辨率下节点 0~节点 7 显示不全的问题。
  - 节点默认全不选，避免误操作批量升级。
  - `全选` 按钮文字不变，但点击可在全选/全不选之间切换。
  - 固定 ID 模式下隐藏节点选择，并提示不需要选择节点。
  - 增加升级进度百分比显示。
  - 增加固件升级页内 IAP 日志显示。

## 构建方式

需要安装 .NET 8 SDK。

```powershell
cd CAN_TOOLS_NET8.0
dotnet build CAN_TOOLS_NET8.csproj
```

如需输出到指定目录：

```powershell
dotnet build CAN_TOOLS_NET8.csproj -c Debug -o bin\publish
```

## 注意事项

- 运行 CAN 上位机时需要 `dll/ControlCANFD.dll` 等 CAN 盒驱动文件。
- 如果复制新版 exe 或 dll 失败，请先关闭正在运行的 `CAN_TOOLS.exe`。
- `SeeSharpTools.JY.GUI 1.4.4.533` 是 .NET Framework 包，构建时可能出现兼容性警告；当前项目可正常编译运行。
- `System.IO.Packaging 8.0.0` 可能提示 NuGet 安全警告，后续可单独升级依赖处理。
- 在线升级前请确认 CAN 已启动、固件文件正确、目标 LLC/PFC 选择正确。

## Flash 布局参考

当前单 IAP 方案常见布局：

```text
0x08000000 ~ 0x08007FFF  Bootloader
0x08007000               IAP 参数/校验信息区
0x08008000               APP 起始地址
0x0801F000               参数保存区 SAVE_ADDR
```

参数保存区由应用程序中的 `SAVE_ADDR` 定义，目前为 `0x0801F000`。在线升级 APP 时应避免写入范围覆盖该区域。
