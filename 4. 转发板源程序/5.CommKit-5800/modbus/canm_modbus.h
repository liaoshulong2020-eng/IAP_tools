/*
 * canm_modbus.h
 *
 *  Created on: 2024年7月25日
 *      Author: Liang Jinfeng
 */

#ifndef CANM_MODBUS_H_
#define CANM_MODBUS_H_

#include "main.h"

/*
 * 初始化
 */
void canmmb_init(ulong baudrate);

/*
 * 发送数据
 */
void canmmb_send_data(const void *buff,ushort size);

#endif /* CANM_MODBUS_H_ */
