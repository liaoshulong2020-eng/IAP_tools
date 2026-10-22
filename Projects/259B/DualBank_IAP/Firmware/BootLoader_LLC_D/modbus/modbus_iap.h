/*
 * modbus_iap.h
 *
 *  Created on: 2024年3月25日
 *      Author: Liang Jinfeng
 */

#ifndef MODBUS_IAP_H
#define MODBUS_IAP_H

#include "main.h"

/*
 * 初始化
 */
void modbus_iap_init();

/*
 * modbus IAP 任务
 */
void modbus_iap_task();

/* True while LLC is acting as the CAN-to-UART PFC upgrade gateway. */
bool iap_pfc_forward_active(void);

#endif /* MODBUS_IAP_H */
