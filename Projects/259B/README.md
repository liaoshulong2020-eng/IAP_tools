# 259B IAP 工具链

本目录统一收纳259B的IAP方案。

| 方案 | 入口 | 说明 |
| --- | --- | --- |
| 单 Bank | `SingleBank_IAP` | 保留既有产品协议和原工程路径，用于旧产品兼容及量产维护 |
| 双 Bank | `DualBank_IAP` | 包含259B双 Bank Bootloader、LLC/PFC APP、Pro上位机、协议文档、构建工具和可烧录成品 |

两套方案彼此独立。选择固件前必须确认目标板是单 Bank还是双 Bank布局，禁止交叉烧录。
