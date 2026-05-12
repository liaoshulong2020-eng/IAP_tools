/*
 * sys_mgr.c
 *
 *  Created on: 2024年4月25日
 *      Author: Liang Jinfeng
 */

#include "sys_mgr.h"
#include "uart.h"
#include "iwdg.h"
#include "gpio.h"
#include "delay.h"
#include "vars.h"
#include "led.h"
#include "timer.h"
#include "modbus_var.h"
#include "pmbusm.h"
#include "iap_transfer.h"
#include "canm_modbus.h"
#include "dac.h" //test

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void sys_init()
{
	vars_init();

	iwdg_init(1000);

    //VREFBUF Enable：采用2.9V
    __LL_SYSCTRL_SpRegWrite_Unlock(SYSCTRL);
    __LL_SYSCTRL_VREFBUF_En(SYSCTRL);
    __LL_SYSCTRL_SpRegWrite_Lock(SYSCTRL);
	
	//PE12为IAP通信接口配置
    gpio_set_pin_input(GPIOE,12,GPIO_PULLUP);
    iap_interface=pe_read_pin(12);

	led_init();
	modbus_var_init();
	pmbusm_init();
	iap_transfer_init();

	timer8_init();

	//test
	dac_init(DAC0);
	dac_output(DAC0,2200);
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
	iwdg_feed(); //喂狗
}
