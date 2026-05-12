# PFC在线升级设计文档

## 1. 概述

在原有LLC在线升级功能的基础上，增加对原边PFC的在线升级支持。升级链路为：

```
PC (iap_tools) → USB/UART → 转接板 → CAN (0xAA55) → LLC Bootloader → UART0 (115200) → PFC Bootloader
```

PFC本身没有对外通信接口，只能通过UART0与LLC通信。因此PFC的在线升级需要LLC bootloader作为中间透传节点。

---

## 2. 系统架构

### 2.1 硬件连接

```
┌──────┐    USB/UART     ┌──────────┐     CAN (125Kbps)     ┌──────────────────────────────┐
│  PC  │ ──────────────→ │  转接板  │ ──────────────────────→│        电源模块              │
│      │    115200        │CommKit   │     ID: 0xAA55        │                              │
└──────┘                  └──────────┘                       │  ┌─────┐  UART0   ┌─────┐   │
                                                             │  │ LLC │ ───────→ │ PFC │   │
                                                             │  │5800 │  115200  │5800 │   │
                                                             │  └─────┘  PB6/PB7 └─────┘   │
                                                             └──────────────────────────────┘
```

### 2.2 地址分配

| 设备 | Modbus地址 | CAN ID | 通信接口 |
|------|-----------|--------|----------|
| 转接板 | 0xFE | 0xAA55 | USB-UART(PC侧) + CAN(目标侧) |
| LLC | 2 | 0xAA55 | CAN(对外) + UART0(对PFC) |
| PFC | 1 | 无 | UART0(对LLC) |

### 2.3 Flash地址规划（5800芯片，LLC和PFC相同）

| 区域 | 起始地址 | 大小 | 说明 |
|------|----------|------|------|
| Bootloader | 0x08000000 | 32KB | 固定不可升级 |
| ARG(校验区) | 0x08007000 | 4KB | 存储APP大小+CRC32 |
| APP | 0x08008000 | ~224KB | 可在线升级区域 |

---

## 3. 通信协议

### 3.1 Modbus IAP帧格式

```
|addr(1)|fno(1)|cmd[8]|size(2)|data[N]|crc16(2)|
```

其中 cmd[8] 映射为 iap_pkt_t 结构：
```c
typedef struct {
    ushort cmd;    // IAP命令 (0~4)
    ulong  addr;   // Flash地址
    ushort len;    // 请求数据长度
    ushort size;   // 实际数据长度
    uchar  data[256];
} iap_pkt_t;
```

### 3.2 IAP命令定义

| cmd | 功能 | Master发送 | Slave回复 |
|-----|------|-----------|-----------|
| 0 | 退出IAP | cmd:0, addr:0, len:0, size:0 | 同上（验证CRC32后跳转APP） |
| 1 | 进入IAP | cmd:1, addr:0, len:1, size:1, data:who | 同上 |
| 2 | 读Flash | cmd:2, addr, len, size:0 | cmd:2, addr, len, size, data |
| 3 | 写Flash | cmd:3, addr, len, size, data | cmd:3, addr, len, size:0 |
| 4 | 写校验码 | cmd:4, addr, len:8, size:8, data | cmd:4, addr, len:8, size:0 |

### 3.3 LLC→PFC 进入IAP的8字节帧

LLC bootloader通过UART0发送8字节帧通知PFC APP复位进入bootloader：

```
[0xAA][0xFF][0x00][0x00][0x00][0x00][CRC8][0x55]
```

- 帧头: 0xAA（LLC帧格式）
- 命令: 0xFF（进入IAP）
- 数据: 4字节全0
- CRC8: 对byte[1]~byte[5]计算
- 帧尾: 0x55

---

## 4. 升级流程

### 4.1 升级LLC（原有流程，不变）

```
1. iap_tools发送 EnterIAP (addr=2, who=1)
2. 转接板通过CAN转发
3. LLC APP收到 → NVIC_SystemReset()
4. LLC bootloader处理IAP命令（写Flash、校验等）
5. 升级完成 → 跳转LLC APP
```

### 4.2 升级PFC（新增流程）

```
 ┌─────────┐         ┌──────────┐         ┌──────────────┐         ┌──────────────┐
 │iap_tools│         │  转接板  │         │LLC Bootloader│         │PFC Bootloader│
 └────┬────┘         └────┬─────┘         └──────┬───────┘         └──────┬───────┘
      │                    │                      │                        │
      │ 1.EnterIAP(addr=1)│                      │                        │
      │───────────────────→│  2.CAN转发           │                        │
      │                    │─────────────────────→│                        │
      │                    │                      │                        │
      │                    │                      │ 3.发送8字节帧(cmd=0xFF) │
      │                    │                      │───────────────────────→│
      │                    │                      │                        │
      │                    │                      │        4.PFC复位       │
      │                    │                      │        进入bootloader  │
      │                    │                      │                        │
      │ 5.重试EnterIAP     │                      │                        │
      │───────────────────→│─────────────────────→│  6.UART0转发给PFC      │
      │                    │                      │───────────────────────→│
      │                    │                      │                        │
      │                    │                      │  7.PFC回复ACK          │
      │                    │                      │←───────────────────────│
      │                    │  8.CAN转发ACK        │                        │
      │←──────────────────│←─────────────────────│                        │
      │                    │                      │                        │
      │ 9.WriteFlash循环   │                      │                        │
      │═══════════════════►│═════════════════════►│═══════════════════════►│
      │◄═══════════════════│◄═════════════════════│◄═══════════════════════│
      │                    │                      │                        │
      │10.WriteChecksum    │                      │                        │
      │───────────────────→│─────────────────────→│───────────────────────→│
      │←──────────────────│←─────────────────────│←───────────────────────│
      │                    │                      │                        │
      │11.ExitIAP          │                      │                        │
      │───────────────────→│─────────────────────→│───────────────────────→│
      │                    │                      │        12.验证CRC32    │
      │                    │                      │        跳转PFC APP     │
      │←──────────────────│←─────────────────────│←───────────────────────│
      │                    │                      │                        │
      │                    │                      │ 13.LLC bootloader      │
      │                    │                      │    跳转LLC APP         │
```

### 4.3 详细步骤说明

1. 用户在iap_tools选择"5800-PFC"，Modbus地址自动设为1
2. iap_tools发送EnterIAP包（addr=1, who=1）
3. 转接板通过CAN转发到LLC
4. LLC APP收到CAN ID=0xAA55, data[0]=1（PFC目标）→ `NVIC_SystemReset()`
5. LLC进入bootloader，初始化CAN + UART0
6. LLC bootloader收到addr=1的IAP包，进入PFC透传模式
7. LLC bootloader通过UART0发送8字节帧(cmd=0xFF)给PFC APP
8. PFC APP的`parse_llc_frame()`识别cmd=0xFF → `NVIC_SystemReset()`
9. PFC进入bootloader（UART0通信, Modbus地址=1）
10. iap_tools重试EnterIAP，LLC bootloader透传给PFC bootloader
11. PFC bootloader回复ACK，LLC bootloader通过CAN转发回上位机
12. 后续WriteFlash/WriteChecksum/ExitIAP均通过LLC透传
13. PFC升级完成后验证CRC32，跳转PFC APP
14. LLC bootloader检测到不再有PFC通信，超时后跳转LLC APP

---

## 5. 代码修改清单

### 5.1 PFC Bootloader (`3. bootloader\BootLoader-5800_pfc`)

| 文件 | 修改内容 |
|------|----------|
| `main/vars.h` | `USE_UART=1, USE_CAN=0`；`MODBUS_LOCAL_ADDR=1` |
| `main/main.c` | 主循环使用 `uart0_tx_poll()` |
| `modbus/modbus_iap.c` | 初始化使用 `uart0_init(115200)` + `uart0_send_data` + `uart0_rx_poll` |

**关键配置：**
```c
#define USE_UART            1
#define USE_CAN             0
#define MODBUS_LOCAL_ADDR   1    // PFC地址
#define APP_BASE_ADDR       0x08008000
#define ARG_BASE_ADDR       (APP_BASE_ADDR - 4*1024)  // 0x08007000
```

### 5.2 LLC Bootloader (`3. bootloader\BootLoader-5800_llc`)

| 文件 | 修改内容 |
|------|----------|
| `main/vars.h` | 保持 `USE_CAN=1, MODBUS_LOCAL_ADDR=2`，增加注释 |
| `main/main.c` | 增加PFC透传功能说明 |
| `main/iap.c` | `iap_task()` 增加PFC透传模式检查，防止误跳转APP |
| `modbus/modbuss.c` | 地址过滤增加 `MODBUS_PFC_ADDR`，接受addr=1的包 |
| `modbus/modbus_iap.c` | **核心**：完整的PFC透传逻辑 |

**LLC bootloader透传核心逻辑：**
```c
static void on_iap_cmd(modbus_iap_t *pkt)
{
    if(pkt->addr == MODBUS_PFC_ADDR)  // addr=1，PFC的包
    {
        pfc_forward_mode = true;
        
        if(!pfc_entered_iap)
        {
            // PFC还没进入bootloader，发送8字节帧让PFC复位
            send_pfc_enter_iap();
            // 不回复ACK，上位机会重试
            return;
        }
        
        // PFC已进入bootloader，透传Modbus包
        uart0_forward_to_pfc(fwd_buff, fwd_size);
        // 等PFC回复后通过CAN转发回上位机
        return;
    }
    
    // addr=2，LLC自己的IAP处理
    iap_pkt_decode((iap_pkt_t*)pkt->cmd);
}
```

### 5.3 LLC APP (`5.原副边程序\2.llc`)

| 文件 | 修改内容 |
|------|----------|
| `APP/source/can_app.c` | IAP_CAN_ID处理：data[0]==1(PFC)时也执行`NVIC_SystemReset()` |

```c
case IAP_CAN_ID:
    if(user_can_ctrl.rx_buf[...][1] == 0x41) {
        if(user_can_ctrl.rx_buf[...][0] == 2 ||  // LLC升级
           user_can_ctrl.rx_buf[...][0] == 1) {   // PFC升级
            NVIC_SystemReset();
        }
    }
    break;
```

### 5.4 PFC APP (`5.原副边程序\1.pfc`)

| 文件 | 修改内容 |
|------|----------|
| `APP/source/pri_sec_commun.c` | `parse_llc_frame()` 增加 cmd=0xFF 处理 |

```c
static void parse_llc_frame(const uint8_t *frame)
{
    // ... 帧头帧尾校验、CRC8校验 ...
    
    uint8_t cmd = frame[1];
    
    // IAP进入命令：cmd=0xFF表示要求PFC进入bootloader模式
    if (cmd == 0xFF) {
        NVIC_SystemReset();
        return;
    }
    
    // ... 原有命令处理 ...
}
```

### 5.5 iap_tools (`11.iap_tools`)

| 文件 | 修改内容 |
|------|----------|
| `MainForm.cs` | 芯片类型增加"5800-PFC"选项，自动设置Modbus地址为1 |

**配置参数（5800-PFC）：**
```json
{
    "ModbusAddr": 1,
    "Baudrate": 115200,
    "FlashBaseAddr": "0x08000000",
    "FlashMaxSize": 262144,
    "AppBaseAddr": "0x08008000",
    "AppMaxSize": 229376,
    "ArgBaseAddr": "0x08007000",
    "ReadWriteSize": 128
}
```

### 5.6 转接板代码 (`4. 转发板源程序`)

**无需修改。** 转接板只做Modbus地址透传，addr=1的包照样通过CAN发出。

---

## 6. 编译和烧录操作

### 6.1 首次烧录（JLink直接烧录）

操作流程与原来一致：

```
1. 编译 BootLoader-5800_pfc → 生成 bootloader hex/bin
2. 编译 PFC APP → 生成 iap.hex/iap.bin
3. boot_config 合并 → 生成 jlink.hex/jlink.bin
4. JLink烧录 jlink.hex 到PFC芯片

5. 编译 BootLoader-5800_llc → 生成 bootloader hex/bin
6. 编译 LLC APP → 生成 iap.hex/iap.bin
7. boot_config 合并 → 生成 jlink.hex/jlink.bin
8. JLink烧录 jlink.hex 到LLC芯片
```

### 6.2 在线升级PFC

```
1. 编译 PFC APP → 生成 iap.bin
2. 打开 iap_tools
3. 选择串口（连接转接板的USB口）
4. 芯片类型选择"5800-PFC"（地址自动设为1）
5. 选择 PFC 的 iap.bin 文件
6. 点击"执行升级"
7. 等待升级完成
```

### 6.3 在线升级LLC（原有流程不变）

```
1. 编译 LLC APP → 生成 iap.bin
2. 打开 iap_tools
3. 选择串口
4. 芯片类型选择"5800"（地址自动设为2）
5. 选择 LLC 的 iap.bin 文件
6. 点击"执行升级"
```

---

## 7. 可靠性设计

### 7.1 CRC校验

- **通信层**：Modbus CRC16校验每个数据包
- **固件层**：CRC32校验整个APP镜像（写入ARG区域）
- **退出IAP时验证**：bootloader在跳转APP前验证CRC32

### 7.2 超时处理

- **LLC透传超时**：5秒无PFC回复则重置接收状态，等待上位机重试
- **Modbus通信超时**：1000个tick无数据则重置接收状态机
- **iap_tools重试**：最多999次主循环重试，每次间隔5秒

### 7.3 断点续传

- iap_tools记录最后成功写入的Flash地址
- 通信中断后重连时从上次位置继续
- 状态保存在PC端（配置文件中）

### 7.4 异常处理

- PFC升级失败：PFC bootloader不跳转APP，等待重新升级
- LLC透传异常：LLC bootloader超时后可跳转LLC APP，不影响LLC正常运行
- 通信中断：iap_tools自动重建串口连接并重试

---

## 8. 注意事项

1. **PFC bootloader编译后**需要运行 `4.boot_config\generate_pfc_config.py` 更新数据文件
2. **UART0引脚**：LLC和PFC的UART0都是 PB6(TX) / PB7(RB)，确保硬件连接正确
3. **升级PFC时LLC也会复位**：因为LLC需要进入bootloader才能透传，升级PFC期间LLC APP不运行
4. **PWM安全**：LLC APP在收到PFC升级命令后会先关闭PWM再复位（与升级自己的行为一致）
5. **首次使用**：PFC芯片必须先通过JLink烧录包含bootloader的完整固件
6. **升级顺序建议**：如果LLC和PFC都需要升级，建议先升级PFC再升级LLC
