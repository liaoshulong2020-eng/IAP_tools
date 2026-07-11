/*
 * led.h
 *
 *  Created on: 2024年3月21日
 *      Author: Liang Jinfeng
 */

#ifndef LED_H
#define LED_H

#include "main.h"

/*
 * 初始化
 */
void led_init();

/*
 * led on
 */
void led_on();

/*
 * led off
 */
void led_off();

/*
 * led任务
 * 时基：100us
 */
void led_task();

#endif /* LED_H */
