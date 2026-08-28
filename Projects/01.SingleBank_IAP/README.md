# 中瀚蓝盾 TAE32G5800 单 Bank IAP 项目

这是现有产品使用的单 Bank IAP 项目入口，与 `02.DualBank_IAP` 为两个相互独立、地位对等的项目。

## 项目组成

- Bootloader：[`../../3. bootloader`](../../3.%20bootloader)
- LLC / PFC APP：[`../../5.原副边程序`](../../5.原副边程序)
- Pro 上位机：[`../../12.HLD_CANFDToolPro`](../../12.HLD_CANFDToolPro)
- IAP 文件生成工具：[`../../IapFileCreator`](../../IapFileCreator)
- Boot + APP 配置及合并工具：[`../../4.boot_config`](../../4.boot_config)
- 单 Bank 发布产物：[`../../release`](../../release)
- 协议和设计文档：[`../../docs`](../../docs)

## 兼容原则

- 保留原有 Bootloader、APP、IAP 文件格式和升级命令，继续兼容已有旧产品。
- 保留原目录结构，避免改变 Keil 工程的相对路径、构建脚本和发布流程。
- 单 Bank 项目不引用双 Bank 的启动状态、Bank 切换或回滚实现。
- 后续对旧产品的维护应从本入口进入，不应把双 Bank 测试代码合并回单 Bank 量产工程。

## 使用说明

单 Bank 的具体型号、LLC/PFC 工程和烧录文件以原目录及 `release` 中对应产品版本为准。烧录或升级前，应核对芯片型号、APP 起始地址、IAP 通讯 ID 和目标产品版本。
