/*
 * modbus_var.c
 *
 *  Created on: 2024年4月3日
 *      Author: Liang Jinfeng
 */

#include "modbus_var.h"
#include "modbuss.h"
#include "uart.h"
#include "vars.h"
#include "led.h"
#include "gpio.h"
#include "iap_transfer.h"

/*******************************************************************************
 * 静态变量
 ******************************************************************************/

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
	//本机升级
	if(pkt->addr==MODBUS_LOCAL_ADDR)sys_reset();
	//其它机器
	else
	{
		iap_mode_flag=true;
		iap_transfer_pkt(pkt);
		modbuss_set_ack_addr(pkt->addr);
	}
}

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void modbus_var_init()
{
	idle_cnt=0;
	modbuss_init(0);
	modbuss_set_timeout(1000);
	modbuss_set_send_callback(uart0_send_data);
	modbuss_set_iap_callback(on_iap_cmd);
	uart0_init(115200);
}

/*
 * modbus变量任务
 */
void modbus_var_task()
{
	uchar data;
	while(uart0_rx_poll(&data))
	{
		modbuss_recv_byte(data);
	}
	modbuss_task();
}
