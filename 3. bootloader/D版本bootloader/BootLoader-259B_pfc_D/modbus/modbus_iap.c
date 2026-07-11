/*
 * modbus_iap.c - PFC Bootloader IAP通信模块
 *
 * PFC通过UART0与LLC通信，接收IAP命令并执行Flash编程
 *
 *  Created on: 2024年3月25日
 *      Author: Liang Jinfeng
 *  Modified: 2025 - 改为UART0通信，支持PFC在线升级
 */

#include "modbus_iap.h"
#include "modbuss.h"
#include "uart.h"
#include "vars.h"
#include "iap.h"

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
 * IAP命令处理
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
 * 初始化 - PFC使用UART0与LLC通信
 */
void modbus_iap_init()
{
	time_cnt=0;
	idle_cnt=0;
	modbuss_init(MODBUS_LOCAL_ADDR);
	modbuss_set_timeout(1000);
	modbuss_set_iap_callback(on_iap_cmd);

	//PFC bootloader固定使用UART0与LLC通信
	modbuss_set_send_callback(uart0_send_data);
	uart0_init(115200);
}

/*
 * modbus IAP 任务
 */
void modbus_iap_task()
{
	uchar data;
	//从UART0接收数据
	while(uart0_rx_poll(&data))
	{
		modbuss_recv_byte(data);
	}
	modbuss_task();
}
