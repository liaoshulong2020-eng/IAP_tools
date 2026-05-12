/*
 * modbus_iap.c
 *
 *  Created on: 2024年3月25日
 *      Author: Liang Jinfeng
 */

#include "modbus_iap.h"
#include "modbuss.h"
#include "uart.h"
#include "vars.h"
#include "iap.h"
#include "cans_modbus.h"

/*******************************************************************************
 * 静态变量
 ******************************************************************************/

static ulong time_cnt;

//空闲计数器
static ushort idle_cnt;

/*******************************************************************************
 * 静态函数
 ******************************************************************************/

/*
 * IAP命令
 * master:	|addr|fno|cmd|size|data|crc|
 * slave:	|addr|fno|cmd|size|data|crc|
 */
static void on_iap_cmd(modbus_iap_t *pkt)
{
	idle_cnt=0;
	iap_pkt_decode((iap_pkt_t*)pkt->cmd);
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
void modbus_iap_init()
{
	time_cnt=0;
	idle_cnt=0;
	modbuss_init(MODBUS_LOCAL_ADDR);
	modbuss_set_timeout(1000);
	modbuss_set_iap_callback(on_iap_cmd);
#if(USE_UART)
	modbuss_set_send_callback(uart1_send_data);
	uart1_init(115200);
#elif(USE_CAN)
	modbuss_set_send_callback(cansmb_send_data);
	cansmb_init(CAN_LOCAL_ID,125000);
#endif
}

/*
 * modbus IAP 任务
 */
void modbus_iap_task()
{
#if(USE_UART)
	uchar data;
	while(uart1_rx_poll(&data))
	{
		modbuss_recv_byte(data);
	}
#endif
	modbuss_task();
}
