/*
 * modbus_iap.c
 *
 *  Created on: 2024年7月25日
 *      Author: Liang Jinfeng
 */

#include "modbus_iap.h"
#include "vars.h"
#include "canm_modbus.h"

/*******************************************************************************
 * 静态变量
 ******************************************************************************/

//待处理的modbus IAP数据包指针
static modbus_iap_t *handle_pkt;

//是否有IAP数据包要处理
static volatile bool *handle_flag;

//数据包处理完成回调函数指针
static void (*on_pkt_finish)(modbus_iap_t *pkt);

/*******************************************************************************
 * 静态函数
 ******************************************************************************/

/*
 * IAP命令
 * master:	|addr|fno|cmd|size|data|crc|
 * slave:	|addr|fno|cmd|size|data|crc|
 */
static void on_iap_cmd_ack(modbus_iap_t *pkt)
{
	if(!on_pkt_finish)return;
	on_pkt_finish(pkt);
}

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void modbus_iap_init(modbus_iap_t *pkt,bool *flag)
{
	handle_pkt=pkt;
	handle_flag=flag;
	on_pkt_finish=NULL;

	modbusm_init();
	modbusm_set_timeout(1000);
	modbusm_set_send_callback(canmmb_send_data);
	modbusm_set_iap_cmd_ack_callback(on_iap_cmd_ack);
	canmmb_init(125000);
}

/*
 * 设置数据包处理完成回调函数指针
 */
void modbus_iap_set_finish_callback(void (*callback)(modbus_iap_t *pkt))
{
	on_pkt_finish=callback;
}

/*
 * modbus IAP 任务
 */
void modbus_iap_task()
{
	if(iap_interface!=0)return;
	//如果有数据包要处理
	if(*handle_flag)
	{
		modbusm_iap_cmd(handle_pkt->addr,handle_pkt->cmd,handle_pkt->data,handle_pkt->size);
		*handle_flag=false;
	}
	modbusm_task();
}
