/*
 * iap.h
 *
 *  Created on: 2024年3月26日
 *      Author: Liang Jinfeng
 */

#ifndef IAP_H
#define IAP_H

#include "main.h"

/*******************************************************************************
 * 参数配置
 ******************************************************************************/

//IAP数据包最大载荷长度
#define IAP_MAX_PAYLOAD_SIZE			256

/*******************************************************************************
 * 数据结构
 ******************************************************************************/

/*
 * IAP数据包
 */
typedef struct
{
	ushort cmd; //IAP命令
	ulong addr; //Flash地址
	ushort len; //请求数据长度
	ushort size; //实际数据长度
	uchar data[IAP_MAX_PAYLOAD_SIZE];
}__attribute__((packed))iap_pkt_t;

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void iap_init();

/*
 * 校验Flash
 */
bool iap_flash_verify();

/*
 * IAP数据包解码
 */
void iap_pkt_decode(iap_pkt_t *pkt);

/*
 * IAP任务
 */
void iap_task();

#endif /* IAP_H */
