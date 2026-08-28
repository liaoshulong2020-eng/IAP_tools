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
	gpio_set_pin_output(GPIOC,13);
	pc_write_pin(13,0); //LED on
}

/*
 * led on
 */
void led_on()
{
	pc_write_pin(13,0); //LED on
}

/*
 * led off
 */
void led_off()
{
	pc_write_pin(13,1); //LED off
}

/*
 * led任务
 * 时基：100us
 */
void led_task()
{
	static ulong cnt=0;

	cnt++;
	if(cnt<500*10)return;
	cnt=0;
	pc_toggle_pin(13);
}
