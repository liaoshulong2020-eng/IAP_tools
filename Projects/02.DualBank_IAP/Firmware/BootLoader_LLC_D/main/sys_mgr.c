/*
 * sys_mgr.c
 *
 *  Created on: 2024骞?3鏈?20鏃?
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
#include "iap_runtime.h"

static volatile ulong system_millis;

/*******************************************************************************
 * 闈欐?佸嚱鏁?
 ******************************************************************************/

/*
 * UART0娴嬭瘯
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
 * 鎺ュ彛鍑芥暟
 ******************************************************************************/

/*
 * 鍒濆鍖?
 */
void sys_init()
{
	system_millis=0;
	iwdg_init(1000);
	led_init();
	iap_runtime_init(CAN_LOCAL_ID);
	iap_init();
	modbus_iap_init();

	timer8_init();
}

/*
 * 瀹氭椂鍣ㄥ洖璋冨嚱鏁?
 * 鏃跺熀锛?20us
 */
void sys_timer_isr()
{
	static uchar state=0;
	static uchar ms_divider=0;

	/* TMR8 runs every 20 us: 50 interrupts are exactly 1 ms. */
	ms_divider++;
	if(ms_divider>=50)
	{
		ms_divider=0;
		system_millis++;
	}

	state++;

	if(state==5)
	{
		led_task();
		state=0;
	}
}

ulong sys_millis(void)
{
	return system_millis;
}

/*
 * 绯荤粺绠＄悊浠诲姟
 */
void sys_task()
{
	//uart_test();
	iwdg_feed(); //鍠傜嫍
}
