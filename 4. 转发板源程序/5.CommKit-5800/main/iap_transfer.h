/*
 * iap_transfer.h
 *
 *  Created on: 2024年5月23日
 *      Author: Liang Jinfeng
 */

#ifndef IAP_TRANSFER_H
#define IAP_TRANSFER_H

#include "main.h"
#include "modbuss.h"

/*
 * 初始化
 */
void iap_transfer_init();

/*
 * 处理数据包
 */
void iap_transfer_pkt(modbus_iap_t *pkt);

/*
 * IAP转发任务
 */
void iap_transfer_task();

#endif /* IAP_TRANSFER_H */
