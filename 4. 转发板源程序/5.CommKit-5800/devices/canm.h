/*
 * canm.h
 *
 *  Created on: 2024年7月24日
 *      Author: Liang Jinfeng
 */

#ifndef CANM_H_
#define CANM_H_

#include "main.h"

/*
 * 初始化
 */
void canm_init(ulong baudrate);

/*
 * 设置接收回调函数
 */
void canm_set_recv_callback(void (*callback)(ulong id,const void *data,uchar size));

/*
 * 检查发送器是否在忙
 */
bool canm_tx_is_busy();

/*
 * 发送数据
 */
void canm_send_data(ulong id,const void *data,uchar size);

/*
 * 接收轮询数据
 */
void canm_rx_poll();

#endif /* CANM_H_ */
