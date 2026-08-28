/*
 * modbuss.c
 *
 *  Created on: 2024��3��28��
 *      Author: Liang Jinfeng
 */

#include "modbuss.h"
#include "crc16.h"
#include "vars.h"

/*******************************************************************************
 * ��̬����
 ******************************************************************************/

static uchar myaddr; //����ͨ�ŵ�ַ
static ulong timeout_cfg;

static uchar send_buff[sizeof(modbus_pkt_t)]; //���ͻ�����
static uchar recv_buff[sizeof(modbus_pkt_t)]; //���ջ�����
static ushort recv_size;
static uchar recv_state;
static bool pkt_flag; //�Ƿ������ݰ�δ����

//���ͻص�����
static void (*on_send)(const void *buff,ushort size);

/*
 * ���ջص�����
 */
static void (*on_read_reg)(modbus_reg_cmd_t *pkt);
static void (*on_write_reg)(modbus_reg_data_t *pkt);
static void (*on_iap)(modbus_iap_t *pkt);

static ushort idle_cnt; //���м�����

/*******************************************************************************
 * ��̬����
 ******************************************************************************/

/*
 * ���յ����Ĵ�������
 */
static void recv_read_reg()
{
	//����������ݰ�
	if(recv_size>=8)
	{
		recv_state=2;
	}
}

/*
 * ���յ�д�Ĵ�������
 */
static void recv_write_reg()
{
	modbus_reg_data_t *pkt;

	if(recv_size<=8)return;
	pkt=(modbus_reg_data_t*)recv_buff;
	//����������ݰ�
	if(recv_size>=pkt->size+10)recv_state=2;
}

/*
 * ���յ�IAP����
 */
static void recv_iap_cmd()
{
	modbus_iap_t *pkt;

	if(recv_size<=12)return;
	pkt=(modbus_iap_t*)recv_buff;
	//����������ݰ�
	if(recv_size>=pkt->size+14)recv_state=2;
}

/*
 * ��ʱ����������ִ��
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
 * ���ݰ�����
 */
static void pkt_decode()
{
	ushort crc0,crc=0;

	if(!pkt_flag)return;

	//crcУ��
	crc0=((ushort)recv_buff[recv_size-1]<<8)+recv_buff[recv_size-2];
	crc16_update(&crc,recv_buff,recv_size-2);
	if(crc0!=crc)
	{
		recv_size=0;
		recv_state=0;
		pkt_flag=false;
		return;
	}

	//ִ�лص�����
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
 * �ӿں���
 ******************************************************************************/

/*
 * ��ʼ��
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
 * ����ͨ�ų�ʱ����ֵ
 */
void modbuss_set_timeout(ulong timeout)
{
	timeout_cfg=timeout;
}

/*
 * ���÷������ݻص�����
 */
void modbuss_set_send_callback(void (*callback)(const void *buff,ushort size))
{
	on_send=callback;
}

/*
 * ���ö��Ĵ����ص�����
 */
void modbuss_set_read_reg_callback(void (*callback)(modbus_reg_cmd_t *pkt))
{
	on_read_reg=callback;
}

/*
 * ����д�Ĵ����ص�����
 */
void modbuss_set_write_reg_callback(void (*callback)(modbus_reg_data_t *pkt))
{
	on_write_reg=callback;
}

/*
 * ����IAP����ص�����
 */
void modbuss_set_iap_callback(void (*callback)(modbus_iap_t *pkt))
{
	on_iap=callback;
}

/*
 * ��Ӧ���Ĵ�������
 */
void modbuss_ack_read_reg(ushort reg,ushort num,const void *data,ushort size)
{
	ushort crc=0;
	modbus_reg_data_t *pkt;

	//��λ����״̬��
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
 * ��Ӧд�Ĵ�������
 */
void modbuss_ack_write_reg(ushort reg,ushort num)
{
	ushort crc=0;
	modbus_reg_cmd_t *pkt;

	//��λ����״̬��
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
 * ����IAP����
 */
void modbuss_send_iap(uchar *cmd,const void *data,ushort size)
{
	ushort crc=0;
	modbus_iap_t *pkt;

	//��λ����״̬��
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
 * ����һ���ֽ�
 */
void modbuss_recv_byte(uchar data)
{
	//��������ݰ�δ�������˳�
	if(pkt_flag)return;

	idle_cnt=0;
	recv_buff[recv_size]=data;
	recv_size++;
	//���յ�ַ�͹�����
	if(recv_state==0)
	{
		if(recv_size==2)recv_state=1;
	}
	//���ݹ�������ղ�ͬ�����ݰ�
	else if(recv_state==1)
	{
		switch(recv_buff[1])
		{
		case FNO_RD_REG:recv_read_reg();break;
		case FNO_WR_REG:recv_write_reg();break;
		case FNO_IAP:recv_iap_cmd();break;
		//��֧�ֵĹ�����
		default:
			recv_size=0;
			recv_state=0;
			break;
		}
	}
	//接收完成数据包
	if(recv_state==2)
	{
		//接受本机地址和PFC地址的数据包（PFC包需要透传）
		if(recv_buff[0]==myaddr || recv_buff[0]==MODBUS_PFC_ADDR)pkt_flag=true;
		//其它非本机地址的数据包丢弃
		else
		{
			recv_size=0;
			recv_state=0;
		}

		//数据包解码
//		pkt_decode();
	}

	//��ֹ���
	if(recv_size>=sizeof(modbus_pkt_t))
	{
		recv_size=0;
		recv_state=0;
	}
}

/*
 * MODBUS�ӻ�����
 */
void modbuss_task()
{
	uchar i;

	timeout_check();

	//���ݰ�����
	pkt_decode();
}
