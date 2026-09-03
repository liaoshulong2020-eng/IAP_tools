# APP 配置 Bootloader UART0 引脚

同一份 Bootloader 支持以下白名单映射：

| 配置值 | UART0 引脚 |
|---|---|
| `IAP_UART_PINMAP_DEFAULT` | 保持旧产品默认映射 |
| `IAP_UART0_PA9_PA10` | PA9 / PA10 |
| `IAP_UART0_PB9_PB10` | PB9 / PB10 |
| `IAP_UART0_PB6_PB7` | PB6 / PB7（兼容现有 LLC） |

APP 启动并完成 Flash 初始化后调用：

```c
#include "iap_runtime.h"

iap_runtime_init(IAP_LEGACY_CAN_ID);
iap_runtime_change_uart_pinmap(IAP_UART0_PA9_PA10);
```

新产品使用 PB9/PB10 时改为：

```c
iap_runtime_change_uart_pinmap(IAP_UART0_PB9_PB10);
```

该函数会比较当前有效配置；映射没有变化时直接返回，不擦写 Flash。映射变化时使用现有的双副本配置机制写入 `0x0801E000`/`0x0803E000`，先写未使用副本、回读校验，最后提交。Bootloader 复位后在初始化 UART0 前读取该映射。

旧配置的 `flags` 为 0，因此继续采用原 Bootloader 默认引脚，不影响旧产品。非法或损坏记录无法通过 magic、长度、CRC32 和 commit 校验，Bootloader 回退默认映射。

注意：APP 传入的是白名单枚举，不允许直接提交任意 GPIO 地址或复用功能。PA9/PA10、PB9/PB10 的 UART0 复用能力仍须与具体 MCU 封装的数据手册一致。
