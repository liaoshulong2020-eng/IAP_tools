/**
  ******************************************************************************
  * @file    vofa_app.c
  * @author  lzf / Gemini
  * @brief   VOFA+ 调试接口实现 - 4通道 Hex 解析版
  ******************************************************************************
  */


#include "vofa_app.h"
#include <string.h>  /* 修复 memset 警告 */
#include <stdlib.h>  /* 修复 atof 警告 */

#if(!UART_FUNC)

/* ---------------- 全局变量定义 ---------------- */
tx_fcunion tx_vofa_data; 
float rx_vofa_data[rec_data_num]; // 对应索引: 0:vp, 1:vi, 2:ip, 3:ii

/* 接收缓冲区 */
#define VOFA_RX_BUF_SIZE 64
static uint8_t rx_buf[VOFA_RX_BUF_SIZE];
static uint16_t rx_index = 0;

/**
 * @brief 纯 Hex 解析函数：识别名称并提取数值
 * 协议格式示例：vp:0.0388\n -> 76 70 3A 30 2E 30 33 38 38 0A
 */
void user_vofa_parse_hex(uint8_t* buf, uint16_t len) 
{
    // 检查长度：至少要有 "xx:" 3个字节 + 数字 + "\n"
    if (len < 5) return;

    // 检查冒号位置 (索引2必须是 0x3A ':')
    if (buf[2] != 0x3A) return;

    // 将末尾换行符 0x0A 临时改为字符串结束符 \0，确保 atof 安全
    buf[len - 1] = '\0';
    float val = (float)atof((char*)&buf[3]);

    /* --- 分支判断：根据 Hex 码匹配名称 --- */
    
    // 1. vp (76 70) -> 通道 0
    if (buf[0] == 0x76 && buf[1] == 0x70) {
        rx_vofa_data[0] = val;
			pfc.vloop.kp = rx_vofa_data[0];
    }
    // 2. vi (76 69) -> 通道 1
    else if (buf[0] == 0x76 && buf[1] == 0x69) {
        rx_vofa_data[1] = val;
			pfc.vloop.ki = rx_vofa_data[1];
    }
    // 3. ip (69 70) -> 通道 2
    else if (buf[0] == 0x69 && buf[1] == 0x70) {
        rx_vofa_data[2] = val;
			pfc.iloop.kp = rx_vofa_data[2];
    }
    // 4. ii (69 69) -> 通道 3
    else if (buf[0] == 0x69 && buf[1] == 0x69) {
			rx_vofa_data[3] = val;
      pfc.iloop.ki = rx_vofa_data[3];
    }
}

/**
 * @brief 库函数弱回调重写：硬件 RxFIFO 收到数据时触发
 */
void LL_UART_RxFIFOFullCallback(UART_TypeDef *Instance) 
{
    if (Instance == VOFA_UART) 
    {
        uint8_t rxdata = (uint8_t)__LL_UART_RxDat8bits_Read(Instance);

        if (rx_index < VOFA_RX_BUF_SIZE - 1) {
            rx_buf[rx_index++] = rxdata;
        }

        // 检测到 \n (0x0A) 触发解析
        if (rxdata == 0x0A) {
            user_vofa_parse_hex(rx_buf, rx_index);
            rx_index = 0;
        }
    }
}

/**
 * @brief VOFA 初始化
 */
void User_VOFA_Init(void) 
{
    UART_InitTypeDef uartinitstruct;
    
    // 显式清理结构体，修复隐式声明警告
    memset((void *)&uartinitstruct, 0, sizeof(uartinitstruct));
    
    uartinitstruct.baudrate    = USER_UART_BAUDRATE;
    uartinitstruct.dat_len     = UART_DAT_LEN_8b;
    uartinitstruct.parity      = UART_PARITY_NO;
    uartinitstruct.stop_len    = UART_STOP_LEN_1b;

    // 配置 JustFloat 帧尾 (0x7F800000)
    uint16_t tail_start = 4 * tx_data_num;
    tx_vofa_data.c[tail_start]     = 0x00;
    tx_vofa_data.c[tail_start + 1] = 0x00;
    tx_vofa_data.c[tail_start + 2] = 0x80;
    tx_vofa_data.c[tail_start + 3] = 0x7f;

    LL_UART_Init(VOFA_UART, &uartinitstruct);

    // 开启 UART 接收中断
    __LL_UART_RxFull_INT_En(VOFA_UART);

    // 预启动 DMA 发送 (注意 DBL 长度包含帧尾)
    LL_UART_Transmit_DMA(VOFA_UART, tx_vofa_data.c, 4 * (tx_data_num + 1));
}

/**
 * @brief 发送驱动：由定时器 TMR 任务调用
 */
void user_vofa_tx(void) 
{
    // 检查 DMA 状态，如果当前传输已完成则启动下一次
    if (DMA->CH[0].REG.STR & (1 << 2)) 
    {
        DMA->CH[0].REG.STR |= (1 << 2); // 清除完成标志
        DMA->CH[0].REG.SAR = (uint32_t)&tx_vofa_data.c;
        DMA->CH[0].REG.DAR = (uint32_t)&(VOFA_UART->TDR);
        DMA->CH[0].REG.DBL = 4 * (tx_data_num + 1);
        DMA->CH[0].REG.CER = 1;         // 启动传输
    }
}
#endif
