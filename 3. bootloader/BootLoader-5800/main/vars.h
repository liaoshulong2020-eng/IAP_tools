/*
 * vars.h
 *
 *  Created on: 2024年3月20日
 *      Author: Liang Jinfeng
 */

#ifndef VARS_H
#define VARS_H

#include "main.h"

/*******************************************************************************
 * 参数配置
 ******************************************************************************/

/*
 * 通信接口配置：多选一
 */
#define USE_UART						0
#define USE_CAN							1
#define USE_PMBUS						0

//本机Modbus地址
#define MODBUS_LOCAL_ADDR				2
//PFC Modbus地址
#define MODBUS_PFC_ADDR					1
//本机PMBus地址
#define PMBUS_LOCAL_ADDR				0x55
//本机CANID
#define CAN_LOCAL_ID					0xAA55

//APP基址
#define APP_BASE_ADDR					0x08008000 //32K

//参数区基址
#define ARG_BASE_ADDR					(APP_BASE_ADDR-4*1024)

//boot区域容量
#define BOOT_MAX_SIZE					(APP_BASE_ADDR-FLASH_BASE_ADDR)
//APP区域容量
#define APP_MAX_SIZE					(FLASH_MAX_SIZE-BOOT_MAX_SIZE)

/*******************************************************************************
 * 系统变量
 ******************************************************************************/



#endif /* VARS_H */
