# 使用 CAN 经 LLC—UART 可靠升级 PFC：详细实施规划

## 1. 文档目的

本文规划一条可量产、可恢复、兼容旧产品的 PFC 在线升级链路：

```text
Pro 上位机
   │ CAN 2.0，29 位扩展帧，IAP CAN ID
   ▼
LLC APP / LLC Bootloader（升级网关）
   │ UART0，115200，8N1
   ▼
PFC APP / PFC Bootloader（升级目标）
```

目标不是简单地转发字节，而是保证以下结果：

1. 上位机只连接 CAN，即可选择 LLC 或 PFC 作为升级目标。
2. 升级 PFC 时，LLC 作为可靠网关，不擦写 LLC 自己的 Flash。
3. PFC APP 能可靠进入 PFC Bootloader，不能依靠固定延时猜测状态。
4. 每个写块必须由 PFC 最终确认，LLC 不能伪造成功应答。
5. CAN 丢帧、UART 错帧、PFC 重启、LLC 重启、上位机中断或掉电后，系统可安全重试或续传。
6. 单 Bank 旧产品和现有 LLC 本机升级协议继续兼容。
7. 双 Bank PFC 支持整 Bank 下载、校验、试启动、确认和回滚。

本文是实施基线。代码、协议常量、测试用例和上位机日志应以本文定义为准，并在实现后同步更新。

---

## 2. 当前工程基线与问题

### 2.1 已具备的基础

当前双 Bank 工程已有以下基础能力：

- 上位机 IAP 页面可以选择 `LLC` 或 `PFC`，包内地址分别为 `2` 和 `1`。
- 上位机把完整 IAP 包按 8 字节拆成经典 CAN 帧发送，并按相同 CAN ID 接收应答。
- LLC Bootloader 使用 CAN，地址为 `2`，并已预留目标地址 `1` 的 PFC 转发分支。
- LLC Bootloader 已初始化 UART0，计划把 PFC IAP 包转发到 UART。
- PFC Bootloader 使用 UART0、115200、8N1，能够解析现有 `0x41` IAP 包。
- PFC APP 已有 LLC/PFC 原副边 UART 通讯，并能识别进入 IAP 命令后复位。
- LLC/PFC Bootloader 均已有单 Bank 命令 `0～5` 和双 Bank 命令 `6～8、0x20～0x25`。
- IAP 包已经具有 CRC16；固件整体具有 CRC32。

### 2.2 当前实现不能直接量产的原因

现有透传骨架存在以下关键缺口：

1. **PFC 进入状态是推测而非确认**
   LLC 重复发送三次进入命令后直接把 `pfc_entered_iap` 置为真，没有收到 PFC Bootloader 的有效握手。

2. **首个进入 IAP 请求可能丢失语义**
   上位机发送目标地址 `1` 的 `cmd=1` 时，LLC 先发送 APP 私有复位帧，但不保存并在 PFC 就绪后重放该请求。

3. **没有明确的网关会话状态机**
   当前仅有布尔型 `pfc_forward_mode`，不能区分请求进入、等待 Bootloader、转发请求、等待应答、提交切换、复位后重连等阶段。

4. **没有事务序号和重复包规则**
   上位机重试后，PFC 无法可靠判断是同一个写块重发还是新的命令；LLC 也无法识别迟到应答。

5. **UART 接收边界保护不足**
   当前 PFC 应答缓冲在写入前缺少严格的最大长度检查；错误的 `size` 可能导致越界或长时间等待。

6. **超时基于循环次数**
   `5000`、`2500` 等计数依赖主循环频率，不是确定的毫秒时间，在编译优化或任务负载变化后会漂移。

7. **CAN 多帧重组缺少分段层保护**
   IAP CRC16只能在完整包到齐后发现错误，无法判断丢的是哪一个 CAN 分片，也不能避免不同请求的分片粘连。

8. **普通业务 UART 与 IAP UART 缺少互斥切换**
   APP 模式下 UART 正在传输 PFC 实时数据和控制数据，进入升级前必须停业务流、清 DMA/FIFO，再切换为 IAP 会话。

9. **PFC 复位和 Bank 切换期间的短暂无应答没有明确处理**
   双 Bank `commit/reset` 可能在 ACK 发完前复位，不能简单把“无 ACK”一律当成功。

10. **上位机目前把 LLC 直连和 PFC 转发使用同一套固定超时**
    PFC 路径多了一次 CAN 重组和一次 UART 往返，需要独立的阶段超时、日志和错误码。

结论：现有代码可作为原型，但需要升级为“有状态、逐事务确认、可恢复”的网关协议。

---

## 3. 总体设计原则

### 3.1 LLC 是代理，不是 PFC 升级结果的裁判

LLC 只完成：

- 接收和校验上位机请求；
- 控制 PFC 从 APP 进入 Bootloader；
- CAN 与 UART 之间的完整包转发；
- 流控、超时、错误映射和状态报告；
- 缓存当前未完成事务，支持上位机重试。

PFC 才负责：

- Flash 地址合法性判断；
- 擦除、写入和读回验证；
- 镜像 CRC32 验证；
- Bank 有效性、试启动、确认和回滚；
- 返回最终成功或明确错误。

LLC 只有收到 PFC 的合法应答后，才能向上位机报告该事务成功。

### 3.2 一次只允许一个 IAP 事务

第一版采用 Stop-and-Wait：上位机发送一个完整请求，收到对应 ACK 后再发送下一请求。暂不做流水线并发。

好处是 RAM 占用小、调试简单、断点边界清晰，115200 UART 下升级速度仍能满足当前 128 字节写块方案。

### 3.3 保留原 IAP 应用层，增加可协商的网关能力

现有 IAP 主包继续保留：

```text
addr(1) | fno=0x41(1) | cmd(2) | flash_addr(4) |
length(2) | payload_size(2) | payload(N) | CRC16(2)
```

- `addr=2`：LLC 本机处理。
- `addr=1`：LLC 转发给 PFC。
- 旧命令 `0～5` 保持语义不变。
- 双 Bank 命令保持 `6～8、0x20～0x25`。

新增能力通过版本查询和网关控制命令协商，旧上位机仍可升级旧设备，新上位机发现不支持网关能力时应禁止 PFC 转发升级并给出明确提示。

---

## 4. 推荐协议设计

## 4.1 设备角色与寻址

| 层级 | LLC | PFC |
| --- | --- | --- |
| 上位机 CAN ID | 使用产品动态 IAP ID，如 `0xA0000～0xA0007` | 与 LLC 共用外部 CAN ID，由包内地址区分 |
| IAP 包内地址 | `2` | `1` |
| LLC—PFC UART 地址 | `2` | `1` |
| 救援 CAN ID | `0xAA55` | 仍经 LLC，由包内地址 `1` 转发 |

PFC 不需要自己的 CAN 收发器。上位机始终向 LLC 所在节点的 CAN ID 发帧，选择 PFC 时只改变包内地址。

## 4.2 新增能力查询命令

建议增加 `cmd=0x30 CAPABILITY_QUERY`，LLC 和 PFC 均可响应。

请求：

```text
cmd=0x30, addr=0, len=0, size=0
```

建议应答 payload：

| 偏移 | 长度 | 含义 |
| --- | ---: | --- |
| 0 | 1 | 协议主版本 |
| 1 | 1 | 协议次版本 |
| 2 | 2 | 最大 payload |
| 4 | 4 | 能力位：单 Bank、双 Bank、网关、续传、Bank 管理等 |
| 8 | 4 | Bootloader 版本 |
| 12 | 4 | 当前 IAP CAN ID |
| 16 | 1 | 设备角色：1=PFC，2=LLC |
| 17 | 1 | 当前运行区：APP/Bootloader |

上位机必须先查询 LLC 网关能力，再查询 PFC Bootloader 能力，禁止仅凭产品型号猜测。

## 4.3 网关控制命令

这些命令由 LLC 处理，不转发给 PFC；建议使用 `addr=2`。

| 命令 | 名称 | 功能 |
| --- | --- | --- |
| `0x31` | `GW_OPEN_PFC` | 请求 LLC 建立 PFC IAP 会话 |
| `0x32` | `GW_STATUS` | 查询 LLC 网关和 PFC 在线状态 |
| `0x33` | `GW_CLOSE` | 正常结束转发，恢复或复位 LLC |
| `0x34` | `GW_ABORT` | 终止本次事务、清缓冲，不提交镜像 |
| `0x35` | `GW_PING_PFC` | 经 UART 查询 PFC Bootloader 能力 |

`GW_OPEN_PFC` 建议携带：会话随机数 `session_id`、期望 PFC 地址、上位机协议版本和操作类型。LLC 返回相同 `session_id`、当前状态和错误码。

## 4.4 事务标识与幂等

现有 IAP 包的 8 字节命令头已经全部占用。为兼容旧格式，第一阶段不改变包长度，事务唯一性使用：

```text
transaction_key = cmd + flash_addr + length + payload_crc32
```

LLC 缓存最近一次请求键和 PFC 应答：

- 收到完全相同的请求且上次已有应答：直接重发缓存 ACK，不再次写 Flash。
- 收到完全相同的请求且仍在等待 PFC：返回 `BUSY` 或保持等待，不重复塞入 UART。
- 收到不同请求但前一事务未结束：返回 `GW_BUSY`。

第二阶段可升级协议，把 `session_id + sequence` 放入扩展头，以更严格地区分迟到应答。新旧版本通过能力查询协商。

## 4.5 统一错误码

建议在失败应答 payload 中返回 16 位错误码：

| 错误码 | 含义 |
| --- | --- |
| `0x0000` | 成功 |
| `0x0101` | LLC 网关忙 |
| `0x0102` | CAN 请求 CRC 错误 |
| `0x0103` | CAN 重组超时 |
| `0x0201` | PFC APP 无应答 |
| `0x0202` | PFC Bootloader 未就绪 |
| `0x0203` | UART 接收超时 |
| `0x0204` | UART CRC 错误 |
| `0x0205` | UART 缓冲溢出 |
| `0x0301` | PFC Flash 地址非法 |
| `0x0302` | PFC 擦除失败 |
| `0x0303` | PFC 编程失败 |
| `0x0304` | PFC 读回验证失败 |
| `0x0305` | PFC 整体 CRC32 错误 |
| `0x0401` | Bank 不可用 |
| `0x0402` | Bank 切换失败 |
| `0x0403` | 试启动未确认并已回滚 |
| `0x0501` | 协议版本不兼容 |
| `0x05FF` | 未知错误 |

上位机日志必须显示阶段、命令、地址、重试次数和具体错误，不只显示“升级失败”。

---

## 5. 完整升级时序

## 5.1 建立连接

1. 上位机打开 CAN，确认目标 LLC CAN ID 可通信。
2. 上位机对 `addr=2` 发送 `CAPABILITY_QUERY`。
3. LLC APP 若在运行，可直接返回 APP/网关能力；若旧版本不支持，则提示需要先更新 LLC。
4. 上位机发送 `GW_OPEN_PFC(session_id)`。
5. LLC APP 停止普通 PFC UART业务，等待当前 DMA TX 完成，关闭普通收发中断，清 FIFO 和错误标志。
6. LLC APP 向 PFC APP 发送带确认的“准备进入 IAP”控制帧。
7. PFC APP 校验命令、检查电源状态并返回 `READY_TO_RESET`。
8. LLC 收到确认后向上位机返回“PFC 正在重启”，随后 PFC APP 复位。
9. LLC 保持网关状态并进入 LLC Bootloader，或者使用保留 RAM/Flash 标志让 LLC 重启后继续网关会话。
10. LLC Bootloader 初始化 CAN 和 UART，周期发送 PFC Bootloader 探测帧。
11. PFC Bootloader 返回能力信息和当前 Bank 状态。
12. LLC 将真实 PFC 响应转回上位机；只有此时上位机才开始发送固件。

不再使用“延时两秒后认为 PFC 已进入”的逻辑。

## 5.2 数据下载

每个 128 字节块执行：

1. 上位机生成完整 IAP `cmd=3` 请求，记录 `transaction_key`。
2. 上位机按 8 字节 CAN 帧发送给 LLC。
3. LLC CAN 重组器检查：CAN ID、目标地址、功能码、声明长度、最大长度、CRC16。
4. LLC 把完整包一次性放入 UART TX 队列；未发送完成前拒绝下一事务。
5. PFC UART 重组器检查长度和 CRC16，调用 PFC IAP 写 Flash。
6. PFC 执行写入并读回验证，然后返回 ACK。
7. LLC UART 重组器检查应答地址、命令、地址、长度、CRC16。
8. LLC 将完整应答拆为 CAN 帧返回上位机，并缓存该应答。
9. 上位机确认 ACK 与请求命令、Flash 地址、写入长度一致后推进进度。

任何一层失败都不得推进断点位置。

## 5.3 双 Bank提交

PFC `.dbiap` 的推荐顺序：

1. `cmd=6 DB_BEGIN`：声明镜像大小和 CRC32，PFC 选择非活动 Bank。
2. 多次 `cmd=3`：写入非活动 Bank。
3. `cmd=7 DB_VERIFY`：PFC 对整镜像重新计算 CRC32，并检查向量表。
4. `cmd=8 DB_COMMIT`：PFC写入试启动状态并准备 Bank 切换。
5. PFC 必须尽可能先返回“提交已持久化”ACK，再执行复位。
6. LLC 检测 UART 静默和 PFC 重启，不把断线立即判为失败。
7. PFC 新 Bank 启动后，APP 或 Bootloader 返回 `TRIAL_RUNNING`。
8. 上位机读取 PFC 的 Bank 状态和测试数据，确认新镜像工作。
9. 上位机发送 `cmd=0x23 BANK_CONFIRM`。
10. 若规定时间/启动次数内未确认，PFC Bootloader 自动回滚。

## 5.4 正常结束

1. 上位机确认 PFC 新 Bank 状态为 `CONFIRMED`。
2. 上位机发送 `GW_CLOSE`。
3. LLC 清除网关会话和缓存。
4. LLC 恢复 APP 普通 UART通讯，或复位进入 APP。
5. 上位机恢复普通参数查询，确认 LLC 与 PFC 均正常。

---

## 6. 各模块实施任务

## 6.1 PFC APP

涉及原副边 UART 通讯模块，建议修改：

- `Firmware/APP_PFC_D/APP/source/pri_sec_commun_app.c`
- 对应头文件、UART 驱动和主循环/中断调用点。

任务：

1. 保留现有普通上报和电压目标命令。
2. 把进入 IAP 从单向 `cmd=0xFF` 改为两阶段握手：
   - `PREPARE_IAP(session_id, reason)`；
   - PFC 返回 `READY_TO_RESET(session_id, app_version, boot_version)`；
   - LLC 返回 `RESET_NOW(session_id)`；
   - PFC 设置 Bootloader 请求标志后复位。
3. Bootloader 请求标志放在保持 RAM 或有 CRC 的持久配置中，不能破坏校准参数。
4. 进入升级前将功率控制切到安全状态，停止会影响 UART 时序的大任务。
5. 对 APP UART 帧严格检查帧头、长度、命令、CRC、帧尾。
6. 重复收到相同 `PREPARE_IAP` 时重复返回确认，保持幂等。
7. 若 LLC 未发 `RESET_NOW`，超时后恢复普通通讯，不应自行卡死。

## 6.2 LLC APP

建议新增独立模块，而不是把网关逻辑继续塞进普通 CAN 命令分支：

```text
APP/include/pfc_iap_gateway.h
APP/source/pfc_iap_gateway.c
```

任务：

1. CAN 收到包内 `addr=1` 时，不再直接复位 LLC。
2. 实现 LLC APP 网关前置状态机：
   - `IDLE`
   - `QUIESCE_NORMAL_UART`
   - `WAIT_PFC_APP_READY`
   - `REQUEST_PFC_RESET`
   - `ENTER_LLC_BOOTLOADER`
   - `ERROR`
3. 保存 `gateway_pending + session_id + target_can_id`，使 LLC 重启到 Bootloader 后知道要继续 PFC 会话。
4. 保存操作应使用双副本、序号和 CRC；断电后若记录不完整则回到正常 APP，不能误入永久升级模式。
5. 普通 UART 通讯与 IAP 网关通过唯一的 UART 所有权管理器切换，禁止两个模块同时访问 UART0/DMA。
6. LLC 进入 Bootloader 前先等待 CAN 确认帧发完，再复位。
7. 老上位机发送 LLC 本机 `addr=2` 时沿用原行为。

## 6.3 LLC Bootloader

这是网关可靠性的核心。建议把当前 `modbus_iap.c` 中的透传逻辑拆为：

```text
modbus/can_iap_transport.c     CAN 完整包重组/拆分
modbus/uart_iap_transport.c    UART 完整包重组/发送
gateway/pfc_iap_gateway.c      会话和事务状态机
gateway/pfc_iap_gateway.h
```

任务：

1. CAN 重组与 LLC 本机 IAP 解码分离。
2. 只有完整包通过 CRC16 后，才能交给 LLC 本机或 PFC 网关。
3. PFC 网关状态至少包含：
   - `GW_IDLE`
   - `GW_PROBE_BOOTLOADER`
   - `GW_READY`
   - `GW_UART_TX`
   - `GW_WAIT_PFC_ACK`
   - `GW_CAN_TX_ACK`
   - `GW_WAIT_PFC_REBOOT`
   - `GW_FAULT`
4. 使用硬件毫秒 Tick，禁止用主循环次数定义超时。
5. UART RX 在写缓冲前检查最大长度；`payload_size` 必须不大于协议上限。
6. UART 出现 overrun、framing、parity 错误时清错误并丢弃当前包，然后返回明确错误。
7. 维护最近请求和最近 ACK 缓存，实现重复请求幂等。
8. 看门狗喂狗点覆盖 CAN 等待、UART发送、Flash 等待和长 CRC 计算，但不能用死循环永久喂狗掩盖故障。
9. `GW_ABORT` 只清除未提交会话，不得把不完整镜像标记为有效。
10. PFC Bank 提交后进入“等待重启”状态，通过能力探测确认 PFC 已重新上线。
11. LLC 自己的 IAP 命令和 PFC 转发命令必须互斥；网关忙时拒绝升级 LLC。
12. 动态 IAP CAN ID 与 `0xAA55` 救援地址继续同时接受。

## 6.4 PFC Bootloader

任务：

1. 保持 UART0、115200、8N1，并在主循环持续执行 UART TX/RX 轮询。
2. 增加 `CAPABILITY_QUERY` 和带版本的状态响应。
3. 每个命令先检查目标地址必须为 `1`。
4. 所有地址运算使用无符号溢出安全检查：
   - `addr >= allowed_start`；
   - `length <= allowed_end - addr`；
   - 写入长度满足 8 字节编程约束。
5. 双 Bank 下载只能写非活动 Bank；启动状态和校准区禁止被固件块覆盖。
6. 每个写块执行 Flash 写入后读回比较，再返回 ACK。
7. 记录下载镜像大小、CRC、最后完成阶段；断电后不自动启动未验证镜像。
8. `DB_VERIFY` 必须校验：栈顶、Reset Handler、镜像范围、整体 CRC32、产品类型和版本信息。
9. `DB_COMMIT` 采用掉电安全状态顺序：先写 pending/trial 状态，校验写入成功，再切 Bank。
10. 新 Bank APP 在系统自检通过后才确认；上位机手工确认和 APP 自动确认策略要明确，建议开发阶段仅手工确认。
11. 重复写相同地址和相同数据必须安全；重复 `DB_BEGIN/VERIFY/COMMIT` 应返回当前真实状态。
12. 返回结构化错误码，不能仅用 `cmd=0xFFFF` 表示所有错误。

## 6.5 原副边普通 UART 通讯

普通通讯必须与 IAP 共存但不能并发。建议定义 UART 所有权：

| 模式 | LLC端 | PFC端 | 允许内容 |
| --- | --- | --- | --- |
| `NORMAL` | LLC APP | PFC APP | 参数、状态、控制 |
| `ENTER_IAP` | LLC APP | PFC APP | 只允许 IAP 切换握手 |
| `IAP` | LLC Bootloader | PFC Bootloader | 只允许 `0x41` IAP 完整包 |
| `RECOVERY` | LLC Bootloader | PFC Bootloader/APP | 探测、终止、重新进入 |

要求：

- 模式切换前等待 TX 完成并清 RX 残留。
- APP 普通帧与 Bootloader IAP 帧使用不同帧头或可靠的长度/CRC识别。
- UART 引脚、复用功能、时钟、波特率在 APP 和 Bootloader 中保持一致。
- 明确 LLC TX→PFC RX、LLC RX←PFC TX 的硬件方向和共地要求。
- VOFA 与 PFC 通讯若共用引脚，升级时必须强制切到 PFC 通讯模式，并在上位机提示。

## 6.6 LLC 与上位机 CAN 通讯

任务：

1. IAP期间只接受当前目标 CAN ID 和救援 ID，屏蔽无关业务负载。
2. CAN 多帧重组必须有：最大长度、包间超时、接收状态复位、来源 ID 固定。
3. 第一阶段保持连续 8 字节分片，以兼容现有上位机。
4. 第二阶段建议增加 CAN 传输层小头：`session/sequence/fragment_index/flags`，支持明确检测丢片和乱序。
5. CAN发送 ACK 时等待邮箱可用，但必须有硬件超时；超时返回网关错误而不是无限阻塞。
6. 批量升级多个 LLC 节点时仍按节点串行；禁止多个节点同时应答同一救援 ID。
7. 使用 `0xAA55` 救援地址时，上位机一次只能连接一个物理节点，界面应二次提醒。

## 6.7 Pro 上位机

IAP 页面建议增加“通讯路径”显示：

```text
目标：PFC
路径：CAN 通道1 → LLC 0xA0003 → UART0 → PFC 地址1
```

实施任务：

1. 选择 PFC 后先执行：LLC能力查询 → 打开网关 → PFC能力查询。
2. 只有探测到 PFC Bootloader 后才允许选择固件并开始写入。
3. 校验固件目标类型，禁止把 LLC 包写入 PFC；`.dbiap` 应增加产品角色、硬件型号、布局版本和镜像版本元数据。
4. 为 PFC 路径设置分阶段超时：
   - CAN包间：建议 100～300 ms；
   - PFC普通命令应答：建议 500 ms；
   - Flash写块：建议 3 s；
   - 整体 CRC：建议 10～30 s；
   - Bank切换重启：建议 10 s；
   具体值以实测最大值乘安全系数确定。
5. 重试只重发当前事务，不回退或越过未确认写块。
6. 断点文件加入：设备 CAN ID、目标角色=PFC、固件 SHA-256、镜像大小、协议版本、最后确认块地址、会话时间。
7. 恢复升级前查询 PFC 当前镜像状态；若 PFC 已重启或擦除上下文变化，从安全扇区边界重传。
8. 日志按链路分层：`CAN`、`LLC网关`、`UART`、`PFC Bootloader`、`Bank状态`。
9. 显示实时统计：当前块、重试次数、UART超时数、总耗时和平均速度。
10. 停止按钮发送 `GW_ABORT`，等待确认后结束；若无响应，提示“已停止上位机发送，但目标状态未知”。
11. 升级成功后自动读取 PFC Bank 状态和版本，不以进度达到100%作为唯一成功条件。
12. Bank 管理选择 PFC 时也必须经同一网关，不应复制另一套临时转发代码。

---

## 7. 超时、重试与恢复策略

### 7.1 推荐默认值

| 阶段 | 初始超时 | 最大重试 | 失败动作 |
| --- | ---: | ---: | --- |
| LLC CAN 探测 | 300 ms | 5 | 检查 CAN ID/通道 |
| PFC APP 准备握手 | 500 ms | 5 | 恢复普通 UART或重试 |
| PFC Bootloader 上线 | 10 s | 持续探测 | 不开始擦写 |
| 普通 IAP 命令 | 1 s | 5 | 重发同一事务 |
| 128字节 Flash 写 | 3 s | 5 | 从同一块重发 |
| 整体 CRC32 | 30 s | 2 | 保留旧 Bank |
| Bank切换重启 | 10 s | 3次探测 | 查询是否回滚 |

这些值必须由上板压力测试得到最终值，而不是直接固化为循环次数。

### 7.2 掉电场景

- 下载中 PFC 掉电：非活动 Bank 不完整，旧 Bank 继续有效；恢复后重新查询并从安全边界续传。
- 提交状态写入前掉电：仍启动旧 Bank。
- pending 状态写入后、切换前掉电：Bootloader按完整状态和CRC决定继续切换或回旧 Bank。
- 新 Bank 试启动时掉电：启动次数累加，超过阈值回滚。
- LLC 掉电：恢复后默认不伪造成功；上位机重新建立网关并查询 PFC真实状态。
- 上位机退出：LLC/PFC 会话超时后保留安全镜像状态并允许重新连接。

---

## 8. 安全与产品防错

1. `.dbiap` 增加受 CRC/SHA 覆盖的头部：magic、格式版本、设备角色、MCU型号、硬件型号、Bank布局版本、镜像大小、镜像 CRC32、软件版本。
2. PFC Bootloader 校验角色必须为 PFC，LLC 包必须拒绝。
3. 降级策略可配置：开发模式允许，量产模式默认禁止版本回退。
4. IAP地址修改、Bank切换、回滚等高风险命令必须只在已建立会话中接受。
5. 校准区和 IAP 配置区设独立范围保护，固件下载命令永远不能覆盖。
6. 若以后增加签名验证，应由 PFC Bootloader验证签名，LLC 只透传，不能只在上位机验证。

---

## 9. 分阶段实施顺序

### 阶段 A：冻结协议和建立主机仿真

- 定义新命令、状态、错误码和超时单位。
- 把 CAN/UART 完整包解析器做成无硬件依赖模块。
- 为正常包、拆包、粘包、CRC错误、超长包、随机字节和重复包编写单元测试。
- 建立“虚拟 LLC 网关 + 虚拟 PFC”测试程序，让上位机可在无开发板时跑完整流程。

验收：10万次随机分片/噪声测试无越界、无死锁，重复请求不重复提交。

### 阶段 B：PFC APP → Bootloader 可靠切换

- 完成两阶段进入握手。
- 完成 UART 所有权切换和安全停机。
- 上板验证 PFC 每次都能从 APP 进入 Bootloader并返回能力帧。

验收：连续1000次进入/退出，无一次假成功或卡死。

### 阶段 C：LLC Bootloader 网关

- 重构当前透传骨架为明确状态机。
- 加入真实毫秒超时、边界检查、事务缓存、错误码和恢复。
- 先只实现小命令：能力查询、读状态、Bank状态。

验收：CAN侧查询1000次，PFC响应内容和CRC全部一致。

### 阶段 D：单 Bank PFC APP 下载

- 实现 `cmd=3/4/0` 经网关传输。
- 进行中断、重试和断点续传测试。

验收：不同大小固件连续升级100次；任意断点掉电后旧程序可运行或可恢复升级。

### 阶段 E：双 Bank PFC

- 实现 `cmd=6/7/8` 和 Bank 管理命令。
- 验证试启动、确认、超时回滚和手工回滚。

验收：下载中、校验中、提交前后、首次启动各注入掉电，均不能导致两个 Bank 同时不可启动。

### 阶段 F：上位机产品化

- 增加路径引导、能力检查、固件类型校验、详细日志和错误提示。
- 支持单节点、批量节点和恢复会话。
- 发布包含版本说明、烧录基线、升级包和验证步骤的成品。

---

## 10. 测试矩阵

### 10.1 功能测试

- LLC 本机单 Bank升级保持兼容。
- LLC 本机双 Bank升级保持兼容。
- PFC 单 Bank升级。
- PFC 双 Bank升级。
- PFC Bank A/B查询、校验、切换、确认、回滚。
- 动态 CAN ID 和 `0xAA55` 救援 ID。
- 不同 LLC 节点地址和批量升级。

### 10.2 故障注入

- 每个 CAN 分片位置丢一帧、重复一帧、延迟一帧。
- UART 任意字节翻转、丢失、重复、插入噪声。
- UART TX/RX FIFO溢出和 framing error。
- PFC 擦除、写入、整体验证时断电。
- LLC 转发请求和转发 ACK 时断电。
- 上位机在 0%～100% 任意进度退出。
- PFC 新 Bank不喂狗、HardFault、无 UART响应。
- 使用错误 LLC/PFC固件、错误硬件型号和超大镜像。

### 10.3 性能与耐久

- 记录 125 kbit/s CAN + 115200 UART 下完整升级耗时。
- 连续升级100次，统计重试率和失败率。
- 高 CAN 总线负载下测试。
- 高低温、低电压和看门狗开启条件下测试。
- Flash 状态区反复提交和回滚耐久测试。

---

## 11. 完成定义

只有同时满足以下条件，才能声明“可以使用 CAN 通过 LLC 的 UART 可靠升级 PFC”：

1. 上位机能先识别 LLC 网关和真实 PFC Bootloader版本。
2. PFC 未进入 Bootloader时，上位机不会开始下载。
3. 每个进度点都对应 PFC 已确认的 Flash 操作。
4. 任意通信错误都有明确错误码、有限重试和可恢复行为。
5. 任意升级阶段掉电不会破坏当前确认可用 Bank。
6. PFC 新 Bank必须经过试启动和确认，失败可自动回滚。
7. LLC 本机升级、旧单 Bank产品和原普通 UART通讯回归测试通过。
8. 通过本文测试矩阵并保存版本化测试记录。
9. 最终成品包含匹配版本的 LLC Bootloader、LLC APP、PFC Bootloader、PFC APP、PFC升级包和 Pro 上位机，不能混用旧版本。

---

## 12. 推荐首轮代码改动清单

按风险和依赖关系，首轮开发建议严格按以下顺序进行：

1. 新建公共网关协议头文件，冻结命令、状态和错误码。
2. 修复 LLC/PFC UART解析器的长度边界和真实毫秒超时。
3. 给 PFC Bootloader增加能力查询和在线探测。
4. 给 PFC APP增加 `PREPARE/READY/RESET_NOW` 握手。
5. 给 LLC APP增加 UART所有权管理及网关进入状态机。
6. 重构 LLC Bootloader透传为独立网关状态机，删除“发送三次即认为成功”。
7. 给 LLC Bootloader增加请求/ACK缓存和幂等重试。
8. 修改 Pro 上位机：打开网关、探测PFC、分阶段超时、详细日志。
9. 先打通只读能力和Bank查询，再开放Flash写入。
10. 完成单 Bank下载故障注入后，再开放双 Bank提交和切换。

该顺序能保证每一步都有可测输出，不会在尚未确认通讯可靠时直接进入 Flash 写入阶段。
