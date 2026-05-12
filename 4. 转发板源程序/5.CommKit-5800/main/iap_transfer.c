/*
 * iap_transfer.c
 *
 *  Created on: 2024年5月23日
 *      Author: Liang Jinfeng
 */

#include "iap_transfer.h"
#include "pmbus_iap.h"
#include "modbus_iap.h"

/*******************************************************************************
 * 静态变量
 ******************************************************************************/

//待处理的modbus IAP数据包
static modbus_iap_t modbus_pkt;

//是否有IAP数据包要处理
static volatile bool iap_pkt_flag;

/*******************************************************************************
 * 静态函数
 ******************************************************************************/

/*
 * 数据包处理完成回调函数
 */
static void on_pkt_finish(modbus_iap_t *pkt)
{
	//判断是否需要发送ACK
	if(pkt->cmd[0]==0xff&&pkt->cmd[1]==0xff)return;
	modbuss_send_iap(pkt->cmd,(const void*)pkt->data,pkt->size);
}

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void iap_transfer_init()
{
	iap_pkt_flag=false;
	pmbus_iap_init(&modbus_pkt,(bool*)(&iap_pkt_flag));
	pmbus_iap_set_finish_callback(on_pkt_finish);
	modbus_iap_init(&modbus_pkt,(bool*)(&iap_pkt_flag));
	modbus_iap_set_finish_callback(on_pkt_finish);
}

/*
 * 处理数据包
 */
void iap_transfer_pkt(modbus_iap_t *pkt)
{
	memmove(&modbus_pkt,pkt,sizeof(modbus_iap_t));
	iap_pkt_flag=true;
}

/*
 * IAP转发任务
 */
void iap_transfer_task()
{
	pmbus_iap_task();
	modbus_iap_task();
}
