# HLD_CANFDToolPro

USB CAN / CAN-FD 双通道分析仪上位机。原始 Qt5 源码已丢失，本工程是**基于原版可执行程序的逆向分析结果**用 Qt 6.8.3 从零重建的**功能等价**版本。

> 重要说明：编译产物不会保留原始源码的变量名、注释与排版，因此本工程不是「逐字还原」，而是根据原版 exe 的类结构/槽函数元数据、官方 `ControlCANFD.h` 接口、接口函数库手册、`config/setdb.txt` 配置格式重建的可编译、可运行、行为对齐的源码。个别无法 100% 确定的细节（界面布局、部分文案）为合理推断，可按需微调。

## 功能

- 双通道 CAN / CAN-FD（ISO / BOSCH）收发
- 标准帧、扩展帧、远程帧、CAN-FD 加速（BRS）
- 单次 / 周期 / 连续发送，ID 自增、数据自增、发送间隔
- 列表发送、CSV 文件发送
- 实时接收列表（11 列，发送=白、接收=淡灰）、暂停显示、合并列表数据、显示缓存帧数限制
- 按列值筛选（右键 → 筛选 / 显示全部 / 删除全部筛选条件）
- 收发计数与实时帧率统计（指数平滑）、复位计数
- 「打开设备」一键打开（默认 125k）＋ 设备状态指示（未插入 / 已插入 / 已打开）
- 设备插拔自动检测（低频轮询）
- 实时保存为 CSV（后台写线程，逐帧不丢数据）
- 设备信息查询、参数设置（仲裁/数据波特率、CANFD 标准、工作模式、终端电阻、自定义波特率）、滤波设置（标准/扩展多组）
- 固件升级（BIN/HEX）
- 实时监控：自动识别设备 ID，解析电压、电流、功率、温度、保护和开关状态
- 参数调试、保护参数读取、单设备控制、电压校准与在线升级
- 监控数据记录：原始 TX/RX 与解析数据同步保存为 CSV

## 当前版本：v1.0

- 修复 `ControlCANFD.dll` 在额外工作线程读取时持续返回空接收缓冲区的问题；改为主线程精确定时器每 10 ms 非阻塞批量读取，已连接实物验证 TX/RX 与监控解析正常。
- 通用列表支持 RX 整行阴影、合并相同 ID、筛选、缓存限制和高频批量刷新。
- 数据记录 ID 统一为 `0xA0007` 格式，移除 CNT，增加过压、欠压、过流、过温及设备状态。
- 监控自动刷新支持运行中修改周期，收发与 CSV 写入互不阻塞。

## 目录结构

```text
HLD_CANFDToolPro/
├── HLD_CANFDToolPro.pro    qmake 工程
├── CMakeLists.txt          CMake 工程（可选）
├── build.ps1               一键构建脚本
├── resources/              资源（logo.png 占位，可替换）
├── docs/official/          官方协议与分析仪资料（本地，不入库）
├── vendor/                 厂商工具与运行依赖（本地，不入库）
├── dist/win64/             唯一发布运行目录（本地，不入库）
├── packages/               交付压缩包（本地，不入库）
└── src/
    ├── main.cpp            入口 + 多语言加载
    ├── mainwindow.*        主窗口（工具栏/双通道面板/收发表/统计/日志）
    ├── modelitem.h         行数据结构
    ├── mytablemodel.*      11 列表格模型（含筛选、方向着色）
    ├── candevice.*         ControlCANFD.dll 封装
    ├── csvwriter.*         后台 CSV 写线程
    ├── configmanager.*     config/setdb.txt 读写
    ├── recvthread.*        接收批次类型（当前稳定版由主定时器直接读取厂商 DLL）
    ├── sendthread.*        发送线程
    ├── baudrate.h          波特率档位表
    ├── inc/controlcanfd.h  官方结构体/函数声明
    └── dialogs/            12 个对话框
```

## 环境要求

- Windows 10 / 11
- Qt 6.8.3 MinGW 64 位（`qmake` + `mingw32-make`）
- 设备厂商提供的 **64 位** `ControlCANFD.dll`（原版 exe 自带的为 32 位，与本工程不兼容）

本仓库不含设备驱动、固件与厂商二进制库，请从 SDK 获取。

## 构建

直接执行（路径可按需修改）：

```powershell
.\build.ps1
```

或手动（从工程根目录执行）：

```powershell
$env:QT_ROOT = (Resolve-Path '.\.qt\6.8.3\mingw_64').Path
$env:MINGW_ROOT = (Resolve-Path '.\.qt\Tools\mingw1310_64').Path
$env:PATH = "$MINGW_ROOT\bin;$QT_ROOT\bin;$env:PATH"
qmake HLD_CANFDToolPro.pro CONFIG+=release
mingw32-make -j4
```

产物在 `build\release\HLD_CANFDToolPro.exe`。

## 运行

编译完成后，把以下内容放到 exe 同目录（或用 `windeployqt`）：

1. Qt 运行库：`windeployqt build\release\HLD_CANFDToolPro.exe`
2. 64 位 `ControlCANFD.dll`
3. `config\`（`setdb.txt`、`company.txt`）、`language\`（`*.qm`）、`help\`、`firmware\firmware.bin`

目录结构应与原版安装目录一致：

```text
HLD_CANFDToolPro.exe
ControlCANFD.dll
config\setdb.txt
config\company.txt
language\*.qm
help\Chinese\...
firmware\firmware.bin
```

## 与原版的已知差异

- Qt 5 → Qt 6（仅 API 现代化，功能一致）
- 菜单栏简化为单行工具栏，界面布局为推断值，可微调
- `Firmware_Update` 的函数签名以厂商 SDK 为准（`src/inc/controlcanfd.h` 已标注推断），如不一致请按 SDK 修正
- `resources/logo.png` 为占位图，可替换为原版 logo
