/*
 * uart.h
 *
 *  Created on: 2024年1月30日
 *      Author: Liang Jinfeng
 */

#ifndef UART_H
#define UART_H

#include "main.h"

/*******************************************************************************
 * 参数配置
 ******************************************************************************/



/*******************************************************************************
 * UART0 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void uart0_init(ulong baudrate);

/*
 * 发送数据
 */
void uart0_send_data(const void *data,ushort size);

/*
 * 发送轮询数据
 */
void uart0_tx_poll();

/*
 * 接收轮询数据
 */
bool uart0_rx_poll(uchar *data);

/*
 * 发送缓冲区是否为空
 */
bool uart0_is_tx_empty();

/*******************************************************************************
 * UART1 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void uart1_init(ulong baudrate);

/*
 * 发送数据
 */
void uart1_send_data(const void *data,ushort size);

/*
 * 发送轮询数据
 */
void uart1_tx_poll();

/*
 * 接收轮询数据
 */
bool uart1_rx_poll(uchar *data);

/*
 * 发送缓冲区是否为空
 */
bool uart1_is_tx_empty();

#endif /* UART_H */
