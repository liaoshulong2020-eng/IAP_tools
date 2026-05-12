/*
 * cans_modbus.c
 *
 *  Created on: 2024年7月25日
 *      Author: Liang Jinfeng
 */

#include "cans_modbus.h"
#include "cans.h"
#include "iwdg.h"
#include "vars.h"
#include "modbuss.h"

/*******************************************************************************
 * 静态函数
 ******************************************************************************/

/*
 * can接收回调函数
 */
static void on_can_recv(ulong id,const void *data,uchar size)
{
	uchar i;
	if(id!=CAN_LOCAL_ID)return;
	for(i=0;i<size;i++)modbuss_recv_byte(((uchar*)data)[i]);
}

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void cansmb_init(ulong id,ulong baudrate)
{
	cans_init(id,baudrate);
	cans_set_recv_callback(on_can_recv);
}

/*
 * 发送数据
 */
void cansmb_send_data(const void *buff,ushort size)
{
	ushort index=0,len;
	while(index<size)
	{
		len=8;
		if(index+len>=size)len=size-index;
		cans_send_data(buff+index,len);
		while(cans_tx_is_busy())iwdg_feed();
		index+=len;
	}
}
