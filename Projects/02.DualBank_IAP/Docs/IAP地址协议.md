# IAP 地址配置协议

## 兼容规则

- 配置地址：任意 `1..0x1FFFFFFF` 的扩展帧 ID。
- 旧产品救援地址：`0xAA55`，永久保留。
- 发现地址：`0x18FF50E5`，为后续 UID 发现预留。
- LLC 包内目标地址为 `2`，PFC 为 `1`，保持旧协议。
- PFC没有独立的外部CAN ID。升级PFC时仍向LLC的动态CAN ID发送，LLC依据包内目标地址`1`通过UART0转发。

## 修改地址

沿用原 IAP 数据包，新增命令 `cmd=5`：

```text
发送 CAN ID = 当前动态地址，忘记时使用 0xAA55
payload: target | 0x41 | cmd=0x0005 | new_can_id(u32 LE) | len=0 | size=0 | CRC16
```

Bootloader 写入另一 Bank 的配置扇区，读回并验证 CRC，最后写 64 位提交标记。ACK 在旧 CAN ID 上返回；新地址复位后生效。

LLC APP 在任务上下文调用：

```c
if (iap_runtime_change_address(new_id)) {
    NVIC_SystemReset();
}
```

不要在 CAN 中断里擦写 Flash。APP 和 Bootloader 必须使用同一份 `Common` 代码。

PFC经LLC网关升级时，“修改IAP地址”只对LLC的外部CAN ID有意义，因此Pro上位机在目标选择PFC时会禁用地址写入按钮。

## 断电策略

配置分别保存在 `0x0801E000` 和 `0x0803E000`。读取时选择 CRC 正确且 sequence 最新的记录；写入时只擦除较旧记录所在扇区。任何时刻至少保留一份旧的有效记录。

校准区 `0x0801F000/0x0803F000` 不属于升级镜像，地址配置写入不会擦除校准数据。
