# CAN经LLC—UART升级PFC：实现说明与验证

## 已实现链路

```text
Pro上位机
  └─ 经典CAN扩展帧，LLC动态IAP ID/救援ID AA55
      └─ LLC APP先通过UART通知PFC APP进入Bootloader，然后LLC复位
          └─ LLC Bootloader重组CAN IAP包并通过UART0转发
              └─ PFC Bootloader执行Flash/双Bank操作并返回真实ACK
                  └─ LLC按原CAN ID把ACK分片返回上位机
```

外部CAN ID始终属于LLC节点；包内地址`2`表示LLC，地址`1`表示PFC。

## 可靠性实现

- LLC APP只有在确认收到目标地址`1`的IAP头后才启动PFC进入握手。新协议使用`PREPARE(0xAB) → READY(0xBA) → RESET(0x5A) → READY(0xBA)`，每一步校验24位CAN/IAP ID、事务序号、CRC8、帧头和帧尾，最多重试3次。
- 新握手失败时才发送三次旧`0xFF`帧，兼容已有PFC产品；新PFC不会仅凭`0x5A`复位，必须先收到相同ID和序号的`0xAB`。
- PFC Bootloader上电后保留15秒网关连接窗口；收到`cmd=1`后进入IAP保持状态。
- LLC Bootloader使用TMR8产生的真实1ms时基，不再用主循环次数推算UART超时。
- 网关采用Stop-and-Wait，同一时间最多一个PFC请求在途。
- LLC只在完整包长度合法、CRC16正确且应答命令与请求相符时转发PFC ACK。
- 重复CAN请求在等待期不会重复进入UART队列；已完成请求使用缓存的真实PFC ACK应答，保证Flash写入幂等。
- UART解析在写缓冲区前检查最大长度，拒绝超过256字节的payload。
- PFC超时后不伪造成功，上位机重试同一事务。
- 双Bank PFC提交并复位后，上位机会重新进入IAP并执行`cmd=0x30`能力探测；探测不到真实PFC响应则整次升级判失败。
- PFC重新上线后，上位机发送退出IAP命令，使PFC和LLC返回APP运行。
- LLC网关空闲60秒后自动退出，避免永久停留在Bootloader。

## 协议扩展

### APP进入Bootloader握手

LLC请求帧固定8字节：

```text
AA | CMD | ID_L | ID_M | ID_H | SEQ | CRC8 | 55
```

PFC应答帧固定8字节：

```text
55 | CMD | ID_L | ID_M | ID_H | SEQ | CRC8 | AA
```

`ID`取触发升级的LLC CAN/IAP ID低24位，当前`0xA0000～0xA0007`、`0xB0001～0xB0008`和救援地址`0xAA55`均可表示。CRC8覆盖`CMD、ID_L、ID_M、ID_H、SEQ`，初值`0x00`，多项式`0x07`。命令定义为：`0xAB=PREPARE`、`0xBA=READY`、`0x5A=RESET`、`0xA5=REJECT`、`0xFF=旧协议进入`。

PFC把第一次合法`PREPARE`中的ID和SEQ锁定为当前事务；后续`RESET`必须完全一致。应答DMA真正发送完成后PFC才复位，避免ACK尚未出线就中断。

### Bootloader能力查询

新增`cmd=0x30`能力查询。应答payload为18字节：

| 偏移 | 长度 | 内容 |
| --- | ---: | --- |
| 0 | 1 | 协议主版本 |
| 1 | 1 | 协议次版本 |
| 2 | 2 | 最大payload，当前256 |
| 4 | 4 | 能力位 |
| 8 | 4 | Bootloader版本 |
| 12 | 4 | IAP配置ID |
| 16 | 1 | 角色：1=PFC，2=LLC |
| 17 | 1 | 运行区：1=Bootloader |

上位机选择PFC时，在任何Flash写入之前必须收到角色为1的能力应答，并在日志显示：

```text
PFC Bootloader 已就绪：协议 1.0，能力=...，路径=CAN→LLC→UART→PFC
```

## 涉及模块

- LLC APP：`Firmware/APP_LLC_D/APP/source/can_app.c`
- LLC Bootloader网关：`Firmware/BootLoader_LLC_D/modbus/modbus_iap.c`
- LLC 1ms时基：`Firmware/BootLoader_LLC_D/main/sys_mgr.c`
- PFC Bootloader等待窗口及能力查询：`Firmware/BootLoader_PFC_D/main/iap.c`
- PFC 1ms时基：`Firmware/BootLoader_PFC_D/main/sys_mgr.c`
- Pro上位机流程：`HostTool/HLD_CANFDToolPro/src/iapupgrade.cpp`
- 公共协议定义：`Common/include/pfc_gateway_protocol.h`
- 协议测试：`Common/tests/test_pfc_gateway_protocol.c`

## 构建验证结果

| 项目 | 结果 |
| --- | --- |
| LLC Bootloader | Keil AC6，0错误、0警告 |
| PFC Bootloader | Keil AC6，0错误、0警告 |
| LLC APP量产版 | Keil AC6，0错误；原工程历史警告58个 |
| PFC APP | Keil AC6，0错误；原工程历史警告7个 |
| Pro上位机 | Qt 6.8.3 / MinGW Release构建通过 |
| IAP配置测试 | 通过 |
| PFC网关协议分片/CRC测试 | 通过 |
| 量产包SHA-256 | 全部通过 |
| 12V/48V开发板验证包SHA-256 | 全部通过 |

## 上板验证步骤

1. LLC和PFC分别烧录最新的`*.factory.256k.bin`。
2. 连接LLC—PFC UART0，双方保持115200、8N1并共地。
3. CAN只连接LLC，启动成品上位机并打开CAN。
4. 在IAP页面选择目标“PFC”，CAN ID填写LLC节点地址。
5. 选择`PFC/PFC.dbiap`并开始升级。
6. 确认日志依次出现：进入IAP、PFC能力探测成功、写入、整体校验、提交、复位后重新探测成功、退出IAP。
7. 打开Bank管理并选择PFC，查询当前Bank、镜像有效性和试启动状态。
8. 验证新程序运行正常后确认当前Bank；异常时执行回滚。

## 上板验收边界

软件构建和主机协议测试已完成。实际UART引脚复用、电气连接、PFC功率安全停机、掉电注入及100次连续升级属于硬件验收，必须按规划文档的测试矩阵在目标板完成并保存记录。
