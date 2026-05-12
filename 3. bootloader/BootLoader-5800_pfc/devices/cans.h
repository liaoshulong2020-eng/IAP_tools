/*
 * cans.h
 *
 *  Created on: 2024年7月22日
 *      Author: Liang Jinfeng
 */

#ifndef CANS_H_
#define CANS_H_

#include "main.h"

/*
 * 初始化
 */
void cans_init(ulong id,ulong baudrate);

/*
 * 设置接收回调函数
 */
void cans_set_recv_callback(void (*callback)(ulong id,const void *data,uchar size));

/*
 * 检查发送器是否在忙
 */
bool cans_tx_is_busy();

/*
 * 发送数据
 */
void cans_send_data(const void *data,uchar size);

/*
 * 接收轮询数据
 */
void cans_rx_poll();

#endif /* CANS_H_ */
