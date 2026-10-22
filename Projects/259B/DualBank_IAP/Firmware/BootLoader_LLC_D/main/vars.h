/*
 * vars.h
 *
 *  Created on: 2024��3��20��
 *      Author: Liang Jinfeng
 */

#ifndef VARS_H
#define VARS_H

#include "main.h"

/*******************************************************************************
 * ��������
 ******************************************************************************/

/*
 * 通信接口配置：三选一
 * LLC bootloader通过CAN与上位机通信，同时通过UART0透传给PFC
 */
#define USE_UART						0
#define USE_CAN							1
#define USE_PMBUS						0

//本机Modbus地址（LLC地址为2）
#define MODBUS_LOCAL_ADDR				2
//PFC Modbus地址
#define MODBUS_PFC_ADDR					1
//本机PMBus地址
#define PMBUS_LOCAL_ADDR				0x55
//本机CANID
#define CAN_LOCAL_ID					0xAA55

//APP��ַ
#define APP_BASE_ADDR					0x08008000 //32K

//��������ַ
#define ARG_BASE_ADDR					(APP_BASE_ADDR-4*1024)

//boot��������
#define BOOT_MAX_SIZE					(APP_BASE_ADDR-FLASH_BASE_ADDR)
//APP��������
#define APP_MAX_SIZE					(FLASH_MAX_SIZE-BOOT_MAX_SIZE)

/*******************************************************************************
 * ϵͳ����
 ******************************************************************************/



#endif /* VARS_H */
