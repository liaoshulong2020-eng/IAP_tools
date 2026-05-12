/*
 * pmbus_iap.h
 *
 *  Created on: 2024年7月26日
 *      Author: Liang Jinfeng
 */

#ifndef PMBUS_IAP_H_
#define PMBUS_IAP_H_

#include "main.h"
#include "modbus.h"

/*
 * 初始化
 */
void pmbus_iap_init(modbus_iap_t *pkt,bool *flag);

/*
 * 设置数据包处理完成回调函数指针
 */
void pmbus_iap_set_finish_callback(void (*callback)(modbus_iap_t *pkt));

/*
 * pmbus IAP 任务
 */
void pmbus_iap_task();

#endif /* PMBUS_IAP_H_ */
