/*
 * led.c
 *
 *  Created on: 2024年3月21日
 *      Author: Liang Jinfeng
 */

#include "led.h"
#include "gpio.h"

/*
 * 初始化
 */
void led_init()
{
	//呼吸灯
	gpio_set_pin_output(GPIOC,13);
	pc_write_pin(13,0); //LED on
}

/*
 * led任务
 * 时基：100us
 */
void led_task()
{
	static ulong cnt=0;

	cnt++;
	if(cnt<5000)return;
	cnt=0;
	pc_toggle_pin(13);
}
