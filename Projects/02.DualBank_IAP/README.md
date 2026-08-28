# 中瀚蓝盾 TAE32G5800D 双 Bank IAP 项目

这是独立的双 Bank 在线升级、试启动、确认及回滚项目，与 `01.SingleBank_IAP` 为两个相互独立、地位对等的项目。它有自己的 Bootloader、APP、链接布局、上位机、协议、测试固件和发布产物，不依赖单 Bank 工程运行。

## 目录说明

- `Firmware/BootLoader_LLC_D`、`Firmware/BootLoader_PFC_D`：双 Bank Bootloader 工程。
- `Firmware/APP_LLC_D`、`Firmware/APP_PFC_D`：双 Bank APP 工程。
- `Common`：动态 IAP 地址、CRC、双副本配置和启动状态机等共用代码。
- `HostTool/HLD_CANFDToolPro`：带 IAP 与 Bank 管理功能的 Pro 上位机。
- `Linker`：双 Bank 固件使用的 AC6 链接布局。
- `Docs`：内存布局、协议、迁移说明和上板验证清单。
- `成品_可烧录`：量产基线烧录码、IAP 文件和配套上位机。
- `开发板_Bank通讯验证`：Bank A 固定 12 V、Bank B 固定 48 V 的切换验证固件。
- `中瀚蓝盾_DualBank_IAP_成品.zip`：可直接下载的完整成品包。

## 方案要点

- 每个 Bank 都保留 32 KB Bootloader 区，并拥有各自的 APP 区。
- Bank 尾部保留启动状态/IAP 配置及校准参数备份区。
- IAP 地址支持由 APP 或上位机配置并持久化，Bootloader 在重启后读取。
- 保留旧单 Bank 命令兼容处理，同时增加 Bank 查询、试启动、确认、回滚和切换管理。
- 上位机的 Bank 管理窗口采用非模态方式，打开时仍可操作主 CAN 收发界面。

## 开发板验证

`开发板_Bank通讯验证` 中的测试 APP 不额外增加周期上报，只修改原电压回复字段：

- Bank A 回复固定为 12 V；
- Bank B 回复固定为 48 V。

这样可以通过现有 CAN 查询流程确认当前运行 Bank，而不会改变原通讯节奏。

## 已完成验证

- LLC/PFC 双 Bank Bootloader 已完成 Keil 构建检查。
- APP 量产基线及 12 V/48 V 测试版本已完成构建和二进制差异检查。
- 共用配置与状态机主机单元测试通过。
- Pro 上位机已完成 Qt 构建及成品打包。

首次上板仍应使用可恢复的开发板，先保存 J-Link 全片备份，并按 `Docs` 中的验证清单逐项确认芯片双 Bank 选项、链接地址、升级、试启动、确认与掉电回滚行为。
