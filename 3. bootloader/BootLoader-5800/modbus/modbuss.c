/*
 * modbuss.c
 *
 *  Created on: 2024年3月28日
 *      Author: Liang Jinfeng
 */

#include "modbuss.h"
#include "crc16.h"

/*******************************************************************************
 * 静态变量
 ******************************************************************************/

static uchar myaddr; //本机通信地址
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
static void (*on_read_reg)(modbus_reg_cmd_t *pkt);
static void (*on_write_reg)(modbus_reg_data_t *pkt);
static void (*on_iap)(modbus_iap_t *pkt);

static ushort idle_cnt; //空闲计数器

/*******************************************************************************
 * 静态函数
 ******************************************************************************/

/*
 * 接收到读寄存器请求
 */
static void recv_read_reg()
{
	//接收完成数据包
	if(recv_size>=8)
	{
		recv_state=2;
	}
}

/*
 * 接收到写寄存器请求
 */
static void recv_write_reg()
{
	modbus_reg_data_t *pkt;

	if(recv_size<=8)return;
	pkt=(modbus_reg_data_t*)recv_buff;
	//接收完成数据包
	if(recv_size>=pkt->size+10)recv_state=2;
}

/*
 * 接收到IAP命令
 */
static void recv_iap_cmd()
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
		if(on_read_reg)on_read_reg((modbus_reg_cmd_t*)recv_buff);
		break;
	case FNO_WR_REG:
		if(on_write_reg)on_write_reg((modbus_reg_data_t*)recv_buff);
		break;
	case FNO_IAP:
		if(on_iap)on_iap((modbus_iap_t*)recv_buff);
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
void modbuss_init(uchar addr)
{
	timeout_cfg=1000;
	recv_size=0;
	recv_state=0;
	pkt_flag=false;
	on_send=NULL;
	on_read_reg=NULL;
	on_write_reg=NULL;
	on_iap=NULL;
	idle_cnt=0;
	myaddr=addr;
}

/*
 * 设置通信超时计数值
 */
void modbuss_set_timeout(ulong timeout)
{
	timeout_cfg=timeout;
}

/*
 * 设置发送数据回调函数
 */
void modbuss_set_send_callback(void (*callback)(const void *buff,ushort size))
{
	on_send=callback;
}

/*
 * 设置读寄存器回调函数
 */
void modbuss_set_read_reg_callback(void (*callback)(modbus_reg_cmd_t *pkt))
{
	on_read_reg=callback;
}

/*
 * 设置写寄存器回调函数
 */
void modbuss_set_write_reg_callback(void (*callback)(modbus_reg_data_t *pkt))
{
	on_write_reg=callback;
}

/*
 * 设置IAP命令回调函数
 */
void modbuss_set_iap_callback(void (*callback)(modbus_iap_t *pkt))
{
	on_iap=callback;
}

/*
 * 回应读寄存器数据
 */
void modbuss_ack_read_reg(ushort reg,ushort num,const void *data,ushort size)
{
	ushort crc=0;
	modbus_reg_data_t *pkt;

	//复位接收状态机
	recv_size=0;
	recv_state=0;

	pkt=(modbus_reg_data_t*)send_buff;
	pkt->addr=myaddr;
	pkt->fno=FNO_RD_REG;
	pkt->reg=reg;
	pkt->num=num;
	pkt->size=size;
	memmove(pkt->data,data,size);
	crc16_update(&crc,pkt,size+8);
	memmove(pkt->data+size,&crc,2);

	if(on_send)on_send(pkt,size+10);
}

/*
 * 回应写寄存器数据
 */
void modbuss_ack_write_reg(ushort reg,ushort num)
{
	ushort crc=0;
	modbus_reg_cmd_t *pkt;

	//复位接收状态机
	recv_size=0;
	recv_state=0;

	pkt=(modbus_reg_cmd_t*)send_buff;
	pkt->addr=myaddr;
	pkt->fno=FNO_WR_REG;
	pkt->reg=reg;
	pkt->num=num;
	crc16_update(&crc,pkt,6);
	pkt->crc=crc;

	if(on_send)on_send(pkt,sizeof(modbus_reg_cmd_t));
}

/*
 * 发送IAP命令
 */
void modbuss_send_iap(uchar *cmd,const void *data,ushort size)
{
	ushort crc=0;
	modbus_iap_t *pkt;

	//复位接收状态机
	recv_size=0;
	recv_state=0;

	pkt=(modbus_iap_t*)send_buff;
	pkt->addr=myaddr;
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
void modbuss_recv_byte(uchar data)
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
		case FNO_RD_REG:recv_read_reg();break;
		case FNO_WR_REG:recv_write_reg();break;
		case FNO_IAP:recv_iap_cmd();break;
		//不支持的功能码
		default:
			recv_size=0;
			recv_state=0;
			break;
		}
	}
	//接收完成数据包
	if(recv_state==2)
	{
		//只处理发给本机的数据包
		if(recv_buff[0]==myaddr)pkt_flag=true;
		//不是发给本机的数据包
		else
		{
			recv_size=0;
			recv_state=0;
		}

		//数据包解码
//		pkt_decode();
	}

	//防止溢出
	if(recv_size>=sizeof(modbus_pkt_t))
	{
		recv_size=0;
		recv_state=0;
	}
}

/*
 * MODBUS从机任务
 */
void modbuss_task()
{
	uchar i;

	timeout_check();

	//数据包解码
	pkt_decode();
}
