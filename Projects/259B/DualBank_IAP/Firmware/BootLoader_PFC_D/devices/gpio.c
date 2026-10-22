/*
 * gpio.c
 *
 *  Created on: 2023年12月2日
 *      Author: Liang Jinfeng
 */

#include "gpio.h"
//#include "vars.h"

/*******************************************************************************
 * 中断回调函数
 ******************************************************************************/

/*
 * GPIO中断回调函数
 */
void LL_GPIO_ExtTrigCallback(GPIO_TypeDef *port, uint32_t pin)
{
	//PD7
	if(port==GPIOD&&pin==GPIO_PIN_7)
	{
		pe_toggle_pin(1); //LED
	}
}

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 配置管脚为输出模式
 */
void gpio_set_pin_output(GPIO_TypeDef *GPIOx,uchar pin)
{
	GPIO_InitTypeDef init={0};

	LL_GPIO_DeInit(GPIOx,1<<pin);

	init.Pin       = 1<<pin;
	init.Alternate = GPIO_AF1_OUTPUT;
	init.Pull      = GPIO_NOPULL;
	init.IntMode   = GPIO_INT_MODE_CLOSE;
	init.OType     = GPIO_OTYPE_PP; //推挽输出
	init.Speed     = GPIO_SPEED_FREQ_HIGH;
	LL_GPIO_Init(GPIOx,&init);
}

/*
 * 设置管脚为输入模式
 */
void gpio_set_pin_input(GPIO_TypeDef *GPIOx,uchar pin,uchar pull)
{
	GPIO_InitTypeDef init={0};

	init.Pin       = 1<<pin;
	init.IntMode   = GPIO_INT_MODE_CLOSE;	//无中断
//	init.OType     = GPIO_OTYPE_PP;			//推挽输出
	init.Pull      = pull;
	init.Speed     = GPIO_SPEED_FREQ_HIGH;	//高速模式
	init.Alternate = GPIO_AF0_INPUT;
	LL_GPIO_Init(GPIOx,&init);
}

/*
 * 设置管脚为外部中断模式
 */
void gpio_set_pin_exti(GPIO_TypeDef *GPIOx,uchar pin,uchar pull,uchar mode)
{
	GPIO_InitTypeDef init={0};

	init.Pin       = 1<<pin;
	init.Alternate = GPIO_AF0_INPUT;
	init.Pull      = pull;
	init.IntMode   = mode;
//	init.OType     = GPIO_OTYPE_PP;
	init.Speed     = GPIO_SPEED_FREQ_LOW;
	LL_GPIO_Init(GPIOx,&init);
}
