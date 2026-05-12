/*
 * sys_mgr.c
 *
 *  Created on: 2024年3月20日
 *      Author: Liang Jinfeng
 */

#include "sys_mgr.h"
#include "uart.h"
#include "iwdg.h"
#include "gpio.h"
#include "delay.h"
#include "timer.h"
#include "vars.h"
#include "led.h"
#include "modbus_iap.h"
#include "iap.h"

/*******************************************************************************
 * 静态函数
 ******************************************************************************/

/*
 * UART0测试
 */
//static void uart_test()
//{
//	static ulong cnt=0;
//	uchar buff[128];

//	cnt++;
//	if(cnt<2*10000)return;
//	cnt=0;

//	memmove(buff,(uchar*)APP_BASE_ADDR,sizeof(buff));
//	uart1_send_data(buff,sizeof(buff));
//}

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void sys_init()
{
	iwdg_init(1000);
	led_init();
	iap_init();
	modbus_iap_init();

	timer8_init();
}

/*
 * 定时器回调函数
 * 时基：20us
 */
void sys_timer_isr()
{
	static uchar state=0;

	state++;

	if(state==5)
	{
		led_task();
		state=0;
	}
}

/*
 * 系统管理任务
 */
void sys_task()
{
	//uart_test();
	iwdg_feed(); //喂狗
}
