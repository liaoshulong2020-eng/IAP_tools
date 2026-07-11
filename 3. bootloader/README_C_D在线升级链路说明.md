# C/D版本在线升级链路说明

## 版本目录

- C版本APP：
  - LLC：`2.227B_llc_A03_V_1_1_26_0115_release`
  - PFC：`2.227B_pfc_A03_v_1_1_10_20260610_release`
- D版本APP：
  - LLC：`1.259B_LLC_A4_V_1_2_0`
  - PFC：`1.259B_PFC_A4_V_1_2_0`
- Bootloader：
  - `C版本bootloader/BootLoader-5800_llc_C`
  - `C版本bootloader/BootLoader-5800_pfc_C`
  - `D版本bootloader/BootLoader-259B_llc_D`
  - `D版本bootloader/BootLoader-259B_pfc_D`

## 通讯路径

外部上位机通过CAN连接LLC。升级LLC时，LLC APP收到IAP命令后复位进入LLC bootloader，由LLC bootloader直接处理CAN升级数据。

升级PFC时，上位机仍然通过CAN发送PFC目标地址。LLC APP收到目标为PFC的IAP进入命令后，也会复位进入LLC bootloader。随后LLC bootloader进入PFC转发模式，通过LLC UART0把IAP命令转发给PFC UART0，PFC APP收到UART命令后复位进入PFC bootloader，再由LLC bootloader透明转发CAN/UART数据完成PFC升级。

## 本次关键修改

- C/D LLC APP的IAP入口支持目标`2=LLC`和`1=PFC`，目标为PFC时也会让LLC进入bootloader，以便启动PFC UART透明转发。
- D版本PFC APP开启`UART_FUNC=1`，使用原副边UART通讯。
- C版本PFC APP移植259B的`pri_sec_commun_app.c/h`，并开启`UART_FUNC=1`。
- C/D PFC APP新增LLC命令`CMD_LLC_ENTER_IAP = 0xFF`，收到后执行`NVIC_SystemReset()`进入PFC bootloader。
- C版本PFC底层UART0按259B方式补齐为`PA9|PA10`，并启用`UART0_IRQn`和`UART0_IRQHandler()`。
- 227B PFC Keil工程已加入`pri_sec_commun_app.c`。

## 已验证

- `2.227B_pfc_A03_v_1_1_10_20260610_release` APP编译通过：0 Error。
- `1.259B_PFC_A4_V_1_2_0/1.pfc_A04_v1.2.0` APP编译通过：0 Error。
- `1.259B_LLC_A4_V_1_2_0` APP源码编译通过：0 Error；后处理阶段提示缺少`Patcher.exe/BootLoader-5800.bin`，属于合并打包脚本依赖，不是C代码编译错误。
- `2.227B_llc_A03_V_1_1_26_0115_release` APP当前链接失败：缺少既有符号`send_massage_share_get`，与本次IAP入口改动无关。

## 注意事项

- PFC升级依赖PFC APP处于可收UART命令状态，因此PFC APP必须启用原副边UART通讯。
- LLC bootloader和PFC bootloader的本地地址需要保持一致：LLC=2，PFC=1。
- IAP包内APP起始地址仍需与串口工具一致。
