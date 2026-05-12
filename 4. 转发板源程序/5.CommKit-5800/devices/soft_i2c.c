/*
 * soft_i2c.c
 *
 *  Created on: 2024年5月22日
 *      Author: Liang Jinfeng
 */

#include "soft_i2c.h"
#include "gpio.h"

/*******************************************************************************
 * 静态变量
 ******************************************************************************/

//延时设置：写延时
static ulong write_delay=20;
//延时设置：读延时
static ulong read_delay=1000;

/*******************************************************************************
 * 静态函数
 ******************************************************************************/

/*
 * SCL设置为输出模式
 */
static void scl_out()
{
	gpio_set_pin_output(GPIOA,0);
}

/*
 * SDA设置为输入模式
 */
static void sda_in()
{
	gpio_set_pin_input(GPIOA,1,GPIO_PULLUP);
}

/*
 * SDA设置为输出模式
 */
static void sda_out()
{
	gpio_set_pin_output(GPIOA,1);
}

/*
 * SCL输出高电平
 */
static void scl_high()
{
	pa_write_pin(0,1);
}

/*
 * SCL输出低电平
 */
static void scl_low()
{
	pa_write_pin(0,0);
}

/*
 * SDA输出高电平
 */
static void sda_high()
{
	pa_write_pin(1,1);
}

/*
 * SDA输出低电平
 */
static void sda_low()
{
	pa_write_pin(1,0);
}

/*
 * 读SDA
 */
static uchar sda_read()
{
	return pa_read_pin(1);
}

/*
 * 延时
 */
static void delay(ulong cycles)
{
	volatile ulong i;
	for(i=0;i<cycles;i++);
}

/*
 * 检测ACK
 */
static bool check_ack()
{
	volatile ulong i;

	sda_in();
	delay(write_delay);
	scl_high();
	delay(write_delay);
	while(sda_read())
	{
		i++;
		if(i>read_delay)
		{
			scl_low();
			return false;
		}
	}
	scl_low();
	return true;
}

/*
 * do ack/nack
 */
void do_ack(bool ack)
{
	scl_low();
	sda_out();
	if(ack)sda_low();
	else sda_high();
	delay(write_delay);
	scl_high();
	delay(write_delay);
	scl_low();
}

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void i2c_init()
{
	write_delay=20;
	read_delay=1000;
	scl_out();
	sda_out();
	scl_high();
	sda_high();
}

/*
 * 设置延时
 */
void i2c_set_delay(ulong wdelay,ulong rdelay)
{
	write_delay=wdelay;
	read_delay=rdelay;
}

/*
 * start
 */
void i2c_start()
{
	sda_out();
	scl_high();
	sda_high();
	delay(write_delay);
	sda_low();
	delay(write_delay);
	scl_low();
}

/*
 * stop
 */
void i2c_stop()
{
	sda_out();
	sda_low();
	scl_low();
	delay(write_delay);
	scl_high();
	delay(write_delay);
	sda_high();
	delay(write_delay);
}

/*
 * write byte
 */
bool i2c_write(uchar data)
{
	uchar i;
	sda_out();
	scl_low();
	for(i=0;i<8;i++)
	{
		if((data<<i)&0x80)sda_high();
		else sda_low();
		delay(write_delay);
		scl_high();
		delay(write_delay);
		scl_low();
		delay(write_delay);
	}
	if(check_ack())return true;
	return false;
}

/*
 * read byte
 */
uchar i2c_read(bool ack)
{
	uchar i,data=0;
	sda_in();
	for(i=0;i<8;i++)
	{
		scl_low();
		delay(write_delay);
		scl_high();
		if(sda_read())set_bit(data,7-i);
		delay(write_delay);
	}
	do_ack(ack);
	return data;
}
