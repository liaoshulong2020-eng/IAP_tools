/*
 * vars.h
 *
 *  Created on: 2024年4月25日
 *      Author: Liang Jinfeng
 */

#ifndef VARS_H
#define VARS_H

#include "main.h"

/*******************************************************************************
 * 参数配置
 ******************************************************************************/

//本机Modbus地址
#define MODBUS_LOCAL_ADDR				0xFE
//LLC Modbus地址
#define MODBUS_LLC_ADDR					2
//IAP CANID
#define CAN_IAP_ID						0xAA55

/*******************************************************************************
 * 数据结构
 ******************************************************************************/

//IAP数据包最大载荷长度
#define IAP_MAX_PAYLOAD_SIZE			256

/*
 * IAP数据包
 */
typedef struct
{
	ushort cmd; //IAP命令
	ulong addr; //Flash地址
	ushort len; //请求数据长度
	ushort size; //实际数据长度
	uchar data[IAP_MAX_PAYLOAD_SIZE];
}__attribute__((packed))iap_pkt_t;

/*******************************************************************************
 * 系统变量
 ******************************************************************************/
 
//是否IAP工作模式
extern bool iap_mode_flag;

/*
 * IAP通信接口
 * 0: CAN
 * 1: PMBus
 */
extern uchar iap_interface;

//正在通信的PMBus从机地址
extern uchar pmbus_addr;

//在线的从机数量
extern uchar online_num;

//从机在线状态位图：128bit
extern uchar online_bitmap[16];

//正在通信的CAN设备ID
extern ulong can_id;

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void vars_init();

#endif /* VARS_H */
