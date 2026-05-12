/*
 * cans_modbus.h
 *
 *  Created on: 2024年7月25日
 *      Author: Liang Jinfeng
 */

#ifndef CANS_MODBUS_H_
#define CANS_MODBUS_H_

#include "main.h"

/*
 * 初始化
 */
void cansmb_init(ulong id,ulong baudrate);

/*
 * 发送数据
 */
void cansmb_send_data(const void *buff,ushort size);

#endif /* CANS_MODBUS_H_ */
