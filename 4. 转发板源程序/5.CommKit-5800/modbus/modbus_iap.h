/*
 * modbus_iap.h
 *
 *  Created on: 2024年7月25日
 *      Author: Liang Jinfeng
 */

#ifndef MODBUS_IAP_H_
#define MODBUS_IAP_H_

#include "main.h"
#include "modbusm.h"

/*
 * 初始化
 */
void modbus_iap_init(modbus_iap_t *pkt,bool *flag);

/*
 * 设置数据包处理完成回调函数指针
 */
void modbus_iap_set_finish_callback(void (*callback)(modbus_iap_t *pkt));

/*
 * modbus IAP 任务
 */
void modbus_iap_task();

#endif /* MODBUS_IAP_H_ */
