259B / TAE32G5800D 当前发布包

已验证基线（2026-07-19）：
- LLC CAN 上报正常。
- LLC 在线升级正常。
- 当前 LLC UART 仅验证 VOFA 模式正常。
- 当前版本暂不支持通过 CAN 设置 LLC UART 模式；后续原副边通讯/PFC 转发升级改动必须实测通过后再上传 GitHub。

使用顺序：
1. 首次/救砖烧录 LLC：02_JLink合并烧录码\01_LLC_JLINK合并Bootloader_259B_5800D.bin
2. 首次/救砖烧录 PFC：02_JLink合并烧录码\02_PFC_JLINK合并Bootloader_259B_5800D.bin
3. 在线升级 LLC：03_在线升级APP\LLC_IAP在线升级APP_259B_5800D.bin
4. 在线升级 PFC：03_在线升级APP\PFC_IAP在线升级APP_259B_5800D.bin

UART 接线：LLC PA9(TX) -> PFC RX，LLC PA10(RX) <- PFC TX，GND 共地。
上位机使用：当前验证基线先按 VOFA 模式使用；不要把未验证的原副边通讯模式作为量产基线。
