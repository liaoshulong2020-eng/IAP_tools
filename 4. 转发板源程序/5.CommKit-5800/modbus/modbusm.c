/*
 * modbusm.c
 *
 *  Created on: 2024年4月4日
 *      Author: Liang Jinfeng
 */

#include "modbusm.h"
#include "crc16.h"

/*******************************************************************************
 * 静态变量
 ******************************************************************************/

static uchar slave_addr; //正在通信的从机地址
static ulong timeout_cfg;

static uchar send_buff[sizeof(modbus_pkt_t)]; //发送缓冲区
static uchar recv_buff[sizeof(modbus_pkt_t)]; //接收缓冲区
static ushort recv_size;
static uchar recv_state;
static bool pkt_flag; //是否有数据包未处理

//发送回调函数
static void (*on_send)(const void *buff,ushort size);

/*
 * 接收回调函数
 */
static void (*on_read_reg_ack)(modbus_reg_data_t *pkt);
static void (*on_write_reg_ack)(modbus_reg_cmd_t *pkt);
static void (*on_iap_cmd_ack)(modbus_iap_t *pkt);

static ushort idle_cnt;

//待处理命令
//static ushort pkt_size;
//static uchar pkt_buff[sizeof(modbus_pkt_t)];

/*******************************************************************************
 * 静态函数
 ******************************************************************************/

/*
 * 接收读寄存器ACK
 */
static void recv_read_reg_ack()
{
	modbus_reg_data_t *pkt;

	if(recv_size<=8)return;
	pkt=(modbus_reg_data_t*)recv_buff;
	//接收完成数据包
	if(recv_size>=pkt->size+10)recv_state=2;
}

/*
 * 接收到写寄存器ACK
 */
static void recv_write_reg_ack()
{
	//接收完成数据包
	if(recv_size>=8)
	{
		recv_state=2;
	}
}

/*
 * 接收到IAP命令ACK
 */
static void recv_iap_cmd_ack()
{
	modbus_iap_t *pkt;

	if(recv_size<=12)return;
	pkt=(modbus_iap_t*)recv_buff;
	//接收完成数据包
	if(recv_size>=pkt->size+14)recv_state=2;
}

/*
 * 超时管理，定期执行
 */
static void timeout_check()
{
	idle_cnt++;
	if(idle_cnt>=timeout_cfg)
	{
		idle_cnt=0;
		recv_size=0;
		recv_state=0;
	}
}

/*
 * 数据包解码
 */
static void pkt_decode()
{
	ushort crc0,crc=0;

	if(!pkt_flag)return;

	//crc校验
	crc0=((ushort)recv_buff[recv_size-1]<<8)+recv_buff[recv_size-2];
	crc16_update(&crc,recv_buff,recv_size-2);
	if(crc0!=crc)
	{
		recv_size=0;
		recv_state=0;
		pkt_flag=false;
		return;
	}

	//执行回调函数
	switch(recv_buff[1])
	{
	case FNO_RD_REG:
		if(on_read_reg_ack)on_read_reg_ack((modbus_reg_data_t*)recv_buff);
		break;
	case FNO_WR_REG:
		if(on_write_reg_ack)on_write_reg_ack((modbus_reg_cmd_t*)recv_buff);
		break;
	case FNO_IAP:
		if(on_iap_cmd_ack)on_iap_cmd_ack((modbus_iap_t*)recv_buff);
		break;
	}

	recv_size=0;
	recv_state=0;
	pkt_flag=false;
}

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void modbusm_init()
{
	slave_addr=0;
	timeout_cfg=1000;
	recv_size=0;
	recv_state=0;
	pkt_flag=false;
	on_send=NULL;
	on_read_reg_ack=NULL;
	on_write_reg_ack=NULL;
	idle_cnt=0;
}

/*
 * 设置通信超时计数值
 */
void modbusm_set_timeout(ulong timeout)
{
	 timeout_cfg=timeout;
}

/*
 * 设置发送数据回调函数
 */
void modbusm_set_send_callback(void (*callback)(const void *buff,ushort size))
{
	on_send=callback;
}

/*
 * 设置读寄存器ACK回调函数
 */
void modbusm_set_read_reg_ack_callback(void (*callback)(modbus_reg_data_t *pkt))
{
	on_read_reg_ack=callback;
}

/*
 * 设置写寄存器ACK回调函数
 */
void modbusm_set_write_reg_ack_callback(void (*callback)(modbus_reg_cmd_t *pkt))
{
	on_write_reg_ack=callback;
}

/*
 * 设置IAP命令回调函数
 */
void modbusm_set_iap_cmd_ack_callback(void (*callback)(modbus_iap_t *pkt))
{
	on_iap_cmd_ack=callback;
}

/*
 * 读寄存器
 */
void modbusm_read_reg(uchar addr,ushort reg,ushort num)
{
	ushort crc=0;
	modbus_reg_cmd_t *pkt;

	//复位接收状态机
	recv_size=0;
	recv_state=0;

	slave_addr=addr;

	pkt=(modbus_reg_cmd_t*)send_buff;
	pkt->addr=addr;
	pkt->fno=FNO_RD_REG;
	pkt->reg=reg;
	pkt->num=num;
	crc16_update(&crc,pkt,6);
	pkt->crc=crc;

	if(on_send)on_send(pkt,sizeof(modbus_reg_cmd_t));
}

/*
 * 写寄存器
 */
void modbusm_write_reg(uchar addr,ushort reg,ushort num,const void *data,ushort size)
{
	ushort crc=0;
	modbus_reg_data_t *pkt;

	//复位接收状态机
	recv_size=0;
	recv_state=0;

	slave_addr=addr;

	pkt->addr=addr;
	pkt->fno=FNO_WR_REG;
	pkt->reg=reg;
	pkt->num=num;
	pkt->size=size;
	memmove(pkt->data,data,size);
	crc16_update(&crc,pkt,size+8);
	memmove(pkt->data+size,&crc,2);

	if(on_send)on_send(pkt,size+10);
}

/*
 * 发送IAP命令
 */
void modbusm_iap_cmd(uchar addr,uchar *cmd,const void *data,ushort size)
{
	ushort crc=0;
	modbus_iap_t *pkt;

	//复位接收状态机
	recv_size=0;
	recv_state=0;

	slave_addr=addr;

	pkt=(modbus_iap_t*)send_buff;
	pkt->addr=addr;
	pkt->fno=FNO_IAP;
	memmove(pkt->cmd,cmd,8);
	pkt->size=size;
	memmove(pkt->data,data,size);
	crc16_update(&crc,pkt,size+12);
	memmove(pkt->data+size,&crc,2);

	if(on_send)on_send(pkt,size+14);
}

/*
 * 接收一个字节
 */
void modbusm_recv_byte(uchar data)
{
	//如果有数据包未处理则退出
	if(pkt_flag)return;

	idle_cnt=0;
	recv_buff[recv_size]=data;
	recv_size++;

	//接收地址和功能码
	if(recv_state==0)
	{
		if(recv_size==2)recv_state=1;
	}
	//根据功能码接收不同的数据包
	else if(recv_state==1)
	{
		switch(recv_buff[1])
		{
		case FNO_RD_REG:recv_read_reg_ack();break;
		case FNO_WR_REG:recv_write_reg_ack();break;
		case FNO_IAP:recv_iap_cmd_ack();break;
		//不支持的功能码
		default:
			recv_size=0;
			recv_state=0;
		}
	}
	//接收完成数据包
	if(recv_state==2)
	{
		//只处理指定从机的数据包
		if(recv_buff[0]==slave_addr)pkt_flag=true;
		//不是指定从机的数据包
		else
		{
			recv_size=0;
			recv_state=0;
		}

		//数据包解码
//		pkt_decode(bus);
	}

	//防止溢出
	if(recv_size>=sizeof(modbus_pkt_t))
	{
		recv_size=0;
		recv_state=0;
	}
}

/*
 * MODBUS主机任务
 */
void modbusm_task()
{
	timeout_check();

	//数据包解码
	pkt_decode();
}
