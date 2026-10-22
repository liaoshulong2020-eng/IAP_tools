# 双 Bank 实施与上板清单

## 内存布局

| 区域 | Bank A | Bank B | 大小 |
|---|---:|---:|---:|
| Stage0 | 0x08000000 | 0x08020000 | 8KB |
| Stage1 | 0x08002000 | 0x08022000 | 24KB |
| APP | 0x08008000 | 0x08028000 | 88KB |
| IAP/启动状态 | 0x0801E000 | 0x0803E000 | 4KB |
| 校准参数 | 0x0801F000 | 0x0803F000 | 4KB |

Bank 重映射后 Stage0、Stage1、APP 均使用同一套逻辑链接地址。`Linker` 中提供了三个 AC6 scatter 文件。

## 升级状态

`EMPTY -> WRITING -> READY -> PENDING -> TRIAL -> CONFIRMED`。镜像校验失败或试启动超过 3 次进入 `BAD`，回到上一个 `CONFIRMED` Bank。

提交顺序必须是：写非活动 Bank、读回、CRC32、写 PENDING 元数据、最后修改 Bank Map 选项字节、复位。APP 完成关键外设自检后调用确认接口。

## 首次迁移

1. 用 J-Link 备份现有整片 Flash、配置扇区和校准扇区。
2. 将芯片选项字节设为 256KB Double Bank。
3. 将同一份 Stage0 烧入两个 Bank，写保护各自前 8KB。
4. Bank A 烧入 Stage1 和当前 APP，Bank B 烧入可回退镜像。
5. 初始化两份元数据，当前 Bank 标记 CONFIRMED。
6. 验证动态地址、`0xAA55` 救援地址、升级断电、CRC 错误和三次启动回滚。

## 必须实机确认

SDK 已提供 `LL_EFLASH_UserOptCfg(...BANK_MODE...)` 和 `...BANK_MAP...`，但在确认芯片手册前，不应直接在线执行 Bank Map 切换。必须确认：切换代码是否要求在 RAM 运行、切换立即生效还是复位生效、选项字节掉电行为，以及映射后物理配置区地址的访问规则。

本目录的状态机、地址持久化和上位机已经完成；Bank Map 最后一跳保持为硬件适配边界，确认上述四点后再接入，避免首板变砖。
