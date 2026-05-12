/*
 * soft_i2c.h
 *
 *  Created on: 2024年5月22日
 *      Author: Liang Jinfeng
 */

#ifndef SOFT_I2C_H
#define SOFT_I2C_H

#include "main.h"

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void i2c_init();

/*
 * 设置延时
 */
void i2c_set_delay(ulong wdelay,ulong rdelay);

/*
 * start
 */
void i2c_start();

/*
 * stop
 */
void i2c_stop();

/*
 * write byte
 */
bool i2c_write(uchar data);

/*
 * read byte
 */
uchar i2c_read(bool ack);

#endif /* SOFT_I2C_H */
