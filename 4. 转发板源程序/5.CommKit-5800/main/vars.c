/*
 * vars.c
 *
 *  Created on: 2024年4月25日
 *      Author: Liang Jinfeng
 */

#include "vars.h"

/*******************************************************************************
 * 系统变量
 ******************************************************************************/

//是否IAP工作模式
bool iap_mode_flag;

/*
 * IAP通信接口
 * 0: CAN
 * 1: PMBus
 */
uchar iap_interface;

//正在通信的PMBus从机地址
uchar pmbus_addr;

//在线的从机数量
uchar online_num;

//从机在线状态位图：128bit
uchar online_bitmap[16];

//正在通信的CAN设备ID
ulong can_id;

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void vars_init()
{
	pmbus_addr=0;
	online_num=0;
	
	//test
	can_id=CAN_IAP_ID;
}
