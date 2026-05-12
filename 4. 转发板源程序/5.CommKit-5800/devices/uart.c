/*
 * uart.c
 *
 *  Created on: 2024年1月30日
 *      Author: Liang Jinfeng
 */

#include "uart.h"
#include "ring_buff.h"

/*******************************************************************************
 * UART0静态变量
 ******************************************************************************/

//发送缓冲区
static ring_buff_t txbuff0;

/*******************************************************************************
 * UART0 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void uart0_init(ulong baudrate)
{
	UART_InitTypeDef init;

	memset((void *)&init,0,sizeof(init));

	init.baudrate=baudrate;
	init.dat_len=UART_DAT_LEN_8b;
	init.parity=UART_PARITY_NO;
	init.stop_len=UART_STOP_LEN_1b;

	LL_UART_Init(UART0,&init);

	__LL_UART_TxFIFO_Reset(UART0);
	__LL_UART_TxDoneIntPnd_Clr(UART0);
	__LL_UART_Tx_En(UART0);
	__LL_UART_Tx_En(UART0);

	__LL_UART_RxFIFO_Reset(UART0);
	__LL_UART_Rx_En(UART0);

	ring_buff_init(&txbuff0);
}

/*
 * 发送数据
 */
void uart0_send_data(const void *data,ushort size)
{
	ring_buff_save_data(&txbuff0,data,size);
}

/*
 * 发送轮询数据
 */
void uart0_tx_poll()
{
	uchar byte;

	//如果数据未发送完成
//	if(!__LL_UART_IsTxDoneIntPnd(UART0))return;
//	__LL_UART_TxDoneIntPnd_Clr(UART0);

	//如果TxFIFO已满，则退出
	if(__LL_UART_IsTxFIFOFull(UART0))return;

	if(!ring_buff_take_byte(&txbuff0,&byte))return;

	//写入数据
	__LL_UART_TxDat8bits_Write(UART0,byte);
}

/*
 * 接收轮询数据
 */
bool uart0_rx_poll(uchar *data)
{
	if(__LL_UART_IsRxFIFOEmpty(UART0))return false;
	*data=(uchar)(__LL_UART_RxDat8bits_Read(UART0));
	return true;
}

/*
 * 发送缓冲区是否为空
 */
bool uart0_is_tx_empty()
{
	return ring_buff_is_empty(&txbuff0);
}
