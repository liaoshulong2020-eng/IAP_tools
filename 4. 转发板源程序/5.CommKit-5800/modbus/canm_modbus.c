/*
 * canm_modbus.c
 *
 *  Created on: 2024年7月25日
 *      Author: Liang Jinfeng
 */

#include "canm_modbus.h"
#include "iwdg.h"
#include "vars.h"
#include "canm.h"
#include "uart.h"
#include "modbusm.h"

/*******************************************************************************
 * 静态函数
 ******************************************************************************/

/*
 * can接收回调函数
 */
static void on_can_recv(ulong id,const void *data,uchar size)
{
	uchar i;
	if(id!=can_id)return;
	for(i=0;i<size;i++)modbusm_recv_byte(((uchar*)data)[i]);
}

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void canmmb_init(ulong baudrate)
{
	canm_init(baudrate);
	canm_set_recv_callback(on_can_recv);
}

/*
 * 发送数据
 */
void canmmb_send_data(const void *buff,ushort size)
{
	ushort index=0,len;
	while(index<size)
	{
		len=8;
		if(index+len>=size)len=size-index;
		canm_send_data(can_id,buff+index,len);
		while(canm_tx_is_busy())iwdg_feed();
		index+=len;
	}
}
