# 原副边程序目录说明

本目录只放 LLC/PFC APP 工程，不放 bootloader 和上位机。

## 目录结构

```text
5.原副边程序/
  C版本_TAE32G5800C/
    227B-T_A03_新版本/
      LLC/
        1.227B-T_llc_A03_V_1_2_0_20260624_release/
        2.227B-T_llc_A03_V_1_2_1_20260709_release/
      PFC/
        PFC_2.227B_pfc_A03_v_1_1_10_20260610_release/

  D版本_TAE32G5800D/
    259B_A4/
      LLC_1.259B_LLC_A4_V_1_2_0/
      PFC_1.259B_PFC_A4_V_1_2_0/

  历史旧版_原目录/
    1.pfc/
    2.llc/
```

## 当前改动状态

- C版本 227B-T LLC APP：
  - IAP目标支持 `LLC=2` 和 `PFC=1`，用于 PFC 升级时让 LLC 进入 bootloader 转发模式。

- C版本 227B-T PFC APP：
  - 已移植 259B 的原副边 UART 通讯模块。
  - 已开启 `UART_FUNC=1`。
  - 已支持 LLC 通过 UART 发送 `0xFF` 命令后复位进入 PFC bootloader。

- D版本 259B LLC APP：
  - IAP目标支持 `LLC=2` 和 `PFC=1`。

- D版本 259B PFC APP：
  - 已开启 `UART_FUNC=1`。
  - 已支持 LLC 通过 UART 发送 `0xFF` 命令后复位进入 PFC bootloader。

## 编译记录

编译日志已移动到：

```text
../7.构建日志/
```

已验证：

- `227B-T PFC APP`：0 Error。
- `259B PFC APP v1.2.0`：0 Error。
- `259B LLC APP`：C代码 0 Error，后处理缺少打包依赖。
- `227B-T LLC APP`：链接缺少原有符号 `send_massage_share_get`，与 IAP入口改动无关。
