/**
  ******************************************************************************
  * @file    user_vofa.c
  * @author  lzf
  * @version V0.2
  * @date    7-09-2021
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 Tai-Micro</center></h2>
  *
  *
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "vofa_app.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>



/* 宏定义请查看user_vofa.h ------------------------------------------------------------*/



/*数据的帧尾（格式固定）*/
uint8_t tail[4] = {0x00, 0x00, 0x80, 0x7f};
DMA_UserCfgTypeDef  dma_user_cfg;
//UART_DMAHandleTypeDef huart_dma_cfg;
/*串口接收缓冲器：用于接收VOFA发送给下位机的数据*/
char rx_buf[rec_byte_len];
uint16_t rx_index = 0;

/*发送,接收数据缓冲器，可以在其它c文件中调用*/
//float tx_vofa_data[tx_data_num];    //发送的数据
float rx_vofa_data[rec_data_num];   //串口接收到的字符串数据经过处理后得到的浮点数据

tx_fcunion tx_vofa_data;

uint8_t vofa_flog = 0;

/*将串口接收到的字符串数据解析成float数据*/
void rec_buf(char* rxbuf, float* fc_buf)
{
    uint8_t i, j;
    char buf[rec_data_len];
    for(i = 0; i < rec_data_num; i++)
    {
        for(j = 0; j < rec_data_len; j++)
        {
            buf[j] = rxbuf[i * rec_data_len + j];
        }
        fc_buf[i] = atof(buf);
    }
}

void user_vofa_tx(void)
{
    if(DMA->CH[0].REG.STR & (1<<2))
    {
//				LL_GPIO_WritePin(cesu_gpio_port, cesu_gpio_pin,GPIO_PIN_RESET);
        vofa_flog = 1;
        DMA->CH[0].REG.STR |= (1<<2);
        DMA->CH[0].REG.SAR = (uint32_t)&tx_vofa_data.c;
        DMA->CH[0].REG.DAR = (uint32_t) & (VOFA_UART->TDR);
        DMA->CH[0].REG.DBL = tx_data_len;
        DMA->CH[0].REG.CER = 1;
//				LL_GPIO_WritePin(cesu_gpio_port, cesu_gpio_pin,GPIO_PIN_SET);
		
//		LL_UART_Transmit_DMA(VOFA_UART, tx_vofa_data.c, tx_data_len);
    }
}


/*中断回调函数*/
//void LL_UART_RxAvailableCallback(UART_TypeDef* Instance)
//{
//    uint32_t rxsize = __LL_UART_RxFIFOLevel_Get(Instance);
//    uint16_t rxdata;

//    for(uint32_t icnt = 0; icnt < rxsize; icnt ++)
//    {
//        rxdata = __LL_UART_RxBuf8bits_Read(Instance);

//        if(__LL_UART_IsRxFIFOErr(Instance) != SET)
//        {
//            rx_buf[rx_index++] = (rxdata & 0xFF);
//        }
//    }
//    if(rx_index >= rec_byte_len)
//    {
//        rx_index = 0;
//        rec_buf(rx_buf, rx_vofa_data);
//    }
//}


/*重定向printf已经被debug所使用，因此没法做成用printf发送的函数*/
#if 0
int fputc(int ch, FILE* f)
{
    __LL_UART_TxBuf8bits_Write(VOFA_UART, ch);
    while(!__LL_UART_IsTxEmpty(VOFA_UART));

    return (ch);
}
#endif

/* vofa串口初始化 */
void User_VOFA_Init(void)
{
    UART_InitTypeDef uartinitstruct;
#if cesu  //测发送速度使用
    GPIO_InitTypeDef LED_GPIO_Init;

    LED_GPIO_Init.Pin   = cesu_gpio_pin;
    LED_GPIO_Init.Mode  = GPIO_MODE_OUTPUT;
    LED_GPIO_Init.OType = GPIO_OTYPE_PP;
    LED_GPIO_Init.Pull  = GPIO_NOPULL;
    LED_GPIO_Init.Speed = GPIO_SPEED_FREQ_LOW;
    LL_GPIO_Init(cesu_gpio_port, &LED_GPIO_Init);
#endif
	memset((void *)&uartinitstruct, 0, sizeof(uartinitstruct));
    uartinitstruct.baudrate       = USER_UART_BAUDRATE;
    uartinitstruct.dat_len        = UART_DAT_LEN_8b;
    uartinitstruct.parity         = UART_PARITY_NO;
    uartinitstruct.stop_len       = UART_STOP_LEN_1b;
//    uartinitstruct.rx_tl          = UART_RX_AVL_TRI_LVL_1CHAR;
//    uartinitstruct.tx_tl          = UART_TX_EMPTY_TRI_LVL_EMPTY;

//    dma_user_cfg.trans_type     = DMA_TRANS_TYPE_M2P;
//    dma_user_cfg.src_addr_mode  = DMA_SRC_ADDR_MODE_INC;
//    dma_user_cfg.dst_addr_mode  = DMA_DST_ADDR_MODE_FIX;
//    dma_user_cfg.src_data_width = DMA_SRC_TRANS_WIDTH_8b;
//    dma_user_cfg.dst_data_width = DMA_DST_TRANS_WIDTH_8b;
//    dma_user_cfg.src_hs_ifc     = DMA_SRC_HANDSHAKE_IFC_MEMORY;
//    dma_user_cfg.dst_hs_ifc     = DMA_CH_CR3_DHSIF_UART0_TX;

//    huart_dma_cfg.buf           = tx_vofa_data.c;
//    huart_dma_cfg.buf_len       = 4 * (tx_data_num + 1);

    memset(tx_vofa_data.f, 0, sizeof(tx_vofa_data.f));
    tx_vofa_data.c[(4 * tx_data_num)] = 0x00;
    tx_vofa_data.c[(4 * tx_data_num) + 1] = 0x00;
    tx_vofa_data.c[(4 * tx_data_num) + 2] = 0x80;
    tx_vofa_data.c[(4 * tx_data_num) + 3] = 0x7f;

    LL_UART_Init(VOFA_UART, &uartinitstruct);
    LL_UART_Transmit_DMA(VOFA_UART, tx_vofa_data.c, 4 * (tx_data_num + 1));

//    LL_UART_Receive_IT(VOFA_UART);
}

/*vofa串口去除初始化*/
void User_VOFA_UART_DeInit(void)
{
    /* UART LL DeInit */
    LL_UART_DeInit(VOFA_UART);
}




/************************* (C) COPYRIGHT Tai-Action *****END OF FILE***********/