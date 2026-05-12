/*
 * gpio.h
 *
 *  Created on: 2024年4月26日
 *      Author: Liang Jinfeng
 */

#ifndef GPIO_H
#define GPIO_H

#include "main.h"

/*
 * 写GPIO
 */
#define gpio_write_pin(GPIOx,pin,value)	LL_GPIO_WritePin(GPIOx,1<<(pin),value)

/*
 * 取反GPIO
 */
#define gpio_toggle_pin(GPIOx,pin) LL_GPIO_TogglePin(GPIOx,1<<(pin))

/*
 * 读GPIO
 */
#define gpio_read_pin(GPIOx,pin) LL_GPIO_ReadPin(GPIOx,1<<(pin))

/*
 * 写GPIOA
 */
#define pa_write_pin(pin,value) LL_GPIO_WritePin(GPIOA,1<<(pin),value)

/*
 * 取反GPIOA
 */
#define pa_toggle_pin(pin) LL_GPIO_TogglePin(GPIOA,1<<(pin))

/*
 * 读GPIOA
 */
#define pa_read_pin(pin) LL_GPIO_ReadPin(GPIOA,1<<(pin))

/*
 * 写GPIOB
 */
#define pb_write_pin(pin,value) LL_GPIO_WritePin(GPIOB,1<<(pin),value)

/*
 * 取反GPIOB
 */
#define pb_toggle_pin(pin) LL_GPIO_TogglePin(GPIOB,1<<(pin))

/*
 * 读GPIOB
 */
#define pb_read_pin(pin) LL_GPIO_ReadPin(GPIOB,1<<(pin))

/*
 * 写GPIOC
 */
#define pc_write_pin(pin,value) LL_GPIO_WritePin(GPIOC,1<<(pin),value)

/*
 * 取反GPIOC
 */
#define pc_toggle_pin(pin) LL_GPIO_TogglePin(GPIOC,1<<(pin))

/*
 * 读GPIOC
 */
#define pc_read_pin(pin) LL_GPIO_ReadPin(GPIOC,1<<(pin))

/*
 * 写GPIOD
 */
#define pd_write_pin(pin,value) LL_GPIO_WritePin(GPIOD,1<<(pin),value)

/*
 * 取反GPIOD
 */
#define pd_toggle_pin(pin) LL_GPIO_TogglePin(GPIOD,1<<(pin))

/*
 * 读GPIOD
 */
#define pd_read_pin(pin) LL_GPIO_ReadPin(GPIOD,1<<(pin))

/*
 * 写GPIOE
 */
#define pe_write_pin(pin,value) LL_GPIO_WritePin(GPIOE,1<<(pin),value)

/*
 * 取反GPIOE
 */
#define pe_toggle_pin(pin) LL_GPIO_TogglePin(GPIOE,1<<(pin))

/*
 * 读GPIOE
 */
#define pe_read_pin(pin) LL_GPIO_ReadPin(GPIOE,1<<(pin))

/*
 * 写GPIOF
 */
#define pf_write_pin(pin,value) LL_GPIO_WritePin(GPIOF,1<<(pin),value)

/*
 * 取反GPIOF
 */
#define pf_toggle_pin(pin) LL_GPIO_TogglePin(GPIOF,1<<(pin))

/*
 * 读GPIOF
 */
#define pf_read_pin(pin) LL_GPIO_ReadPin(GPIOF,1<<(pin))

/*
 * 配置管脚为输出模式
 */
void gpio_set_pin_output(GPIO_TypeDef *GPIOx,uchar pin);

/*
 * 设置管脚为输入模式
 */
void gpio_set_pin_input(GPIO_TypeDef *GPIOx,uchar pin,uchar pull);

/*
 * 设置管脚为外部中断模式
 */
void gpio_set_pin_exti(GPIO_TypeDef *GPIOx,uchar pin,uchar pull,uchar mode);

#endif /* GPIO_H */
