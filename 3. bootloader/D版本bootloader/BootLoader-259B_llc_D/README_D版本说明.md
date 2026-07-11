# BootLoader-259B_llc_D 说明

这个工程是给 1.259B_LLC_A4_V_1_2_0 使用的 TAE32G5800 D 版本芯片 BootLoader 工程。

## 来源

- 基础工程：原 `BootLoader-5800_llc`
- D 版本官方库：`TAE32G58xx_FW_ReleaseV1.3.1`
- 目标芯片：`TAE32G5800`
- Keil Pack：`Tai-Action.TAE32G58xx_DFP.1.3.1`

## 已替换为 V1.3.1 的关键文件

- `Drivers/TAE32G58xx_Driver/tae32g58xx_ll_eflash.c`
- `Drivers/TAE32G58xx_Driver/tae32g58xx_ll_eflash.h`
- `Drivers/TAE32G58xx_Driver/tae32g58xx_ll_can.c`
- `Drivers/TAE32G58xx_Driver/tae32g58xx_ll_can.h`
- `Drivers/TAE32G58xx_Device/startup_tae32g58xx.c`
- `Drivers/TAE32G58xx_Device/tae32g58xx.h`
- `Drivers/TAE32G58xx_Device/myChip.h`

这些文件已经和官方 V1.3.1 SDK 做过哈希校验，内容一致。

## 当前地址配置

- BootLoader 起始地址：`0x08000000`
- APP 起始地址：`0x08008000`
- IAP 参数区：`0x08007000` 到 `0x08007FFF`
- 固定 CAN 扩展 ID：`0xAA55`
- LLC Modbus 地址：`2`
- PFC Modbus 地址：`1`

## 编译输出

Keil 输出名已改为：

```text
BootLoader-259B-LLC-D
```

编译后重点使用：

```text
Keil/Execute/BootLoader-259B-LLC-D.bin
Keil/Execute/BootLoader-259B-LLC-D.hex
```

## 注意

259B 使用 D 版本芯片时，不建议继续使用 C 版本 BootLoader 或旧版底层库。尤其是 Flash 擦写和 CAN 底层库，D 版本官方 V1.3.1 与旧工程差异很大，旧 BootLoader 可能导致 IAP 写入失败、复位异常或升级成功率低。
