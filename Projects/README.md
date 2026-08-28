# CAN IAP 项目入口

本目录把仓库中的 IAP 方案划分为两个相互独立、地位对等的项目：

| 项目 | 用途 | 项目入口 |
| --- | --- | --- |
| 单 Bank IAP | 已有产品兼容、原量产升级流程 | `01.SingleBank_IAP` |
| 双 Bank IAP | 双 Bank 在线升级、回滚及 Bank 管理测试 | `02.DualBank_IAP` |

两个项目不存在父子或包含关系，分别维护自己的说明、协议、构建方式和发布产物。双 Bank 项目不是单 Bank 项目的子模块；单 Bank 项目也不依赖双 Bank 工程。

为保证旧产品工程路径、Keil 工程引用及已有脚本继续有效，单 Bank 源码仍保留在仓库原位置，由 `01.SingleBank_IAP` 提供统一入口。双 Bank 项目则完整收纳在 `02.DualBank_IAP` 下。
