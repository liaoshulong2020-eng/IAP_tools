/*
 * modbus_iap.c - LLC Bootloader IAP通信模块（含PFC透传）
 *
 * LLC通过CAN接收上位机IAP命令：
 * - 如果目标地址是自己(addr=2)，则自己处理IAP
 * - 如果目标地址是PFC(addr=1)，则通过UART0透传给PFC
 *
 *  Created on: 2024年3月25日
 *      Author: Liang Jinfeng
 *  Modified: 2025 - 增加PFC透传支持
 */

#include "modbus_iap.h"
#include "modbuss.h"
#include "uart.h"
#include "vars.h"
#include "iap.h"
#include "cans_modbus.h"
#include "crc16.h"

/*******************************************************************************
 * 静态变量
 ******************************************************************************/

static ulong time_cnt;

//空闲计数器
static ushort idle_cnt;

//PFC透传模式标志
static bool pfc_forward_mode;

//PFC透传接收缓冲区
static uchar pfc_recv_buff[sizeof(modbus_pkt_t)];
static ushort pfc_recv_size;
static uchar pfc_recv_state;

//PFC透传超时计数器
static ushort pfc_timeout_cnt;
#define PFC_FORWARD_TIMEOUT		5000  //透传超时（约5秒）

//PFC进入IAP请求计数器
static ulong pfc_enter_iap_cnt;
static bool pfc_entered_iap;

/*******************************************************************************
 * 静态函数
 ******************************************************************************/

/*
 * 通过UART0发送数据给PFC
 */
static void uart0_forward_to_pfc(const void *buff, ushort size)
{
	uart0_send_data(buff, size);
}

/*
 * 向PFC发送进入IAP命令（使用8字节LLC帧格式）
 * 帧格式: [0xAA][0xFF][0x00][0x00][0x00][0x00][CRC8][0x55]
 * PFC APP会识别cmd=0xFF并执行NVIC_SystemReset()
 */
static void send_pfc_enter_iap()
{
	uchar frame[8];
	uchar crc;

	frame[0]=0xAA;  //帧头（LLC帧格式）
	frame[1]=0xFF;  //命令：进入IAP
	frame[2]=0x00;
	frame[3]=0x00;
	frame[4]=0x00;
	frame[5]=0x00;

	//计算CRC8（对frame[1]~frame[5]共5字节）
	crc=0;
	for(int i=1;i<=5;i++)
	{
		crc^=frame[i];
		for(int j=0;j<8;j++)
		{
			if(crc&0x80) crc=(crc<<1)^0x07;
			else crc=crc<<1;
		}
	}
	frame[6]=crc;
	frame[7]=0x55;  //帧尾

	uart0_forward_to_pfc(frame, 8);
}

/*
 * 处理从PFC收到的UART0数据（透传模式下的ACK）
 * 将PFC的回复转发回CAN给上位机
 */
static void pfc_recv_byte(uchar byte)
{
	modbus_iap_t *pkt;

	pfc_timeout_cnt=0; //收到数据，重置超时

	if(pfc_recv_state==0)
	{
		//等待地址字节
		if(byte==MODBUS_PFC_ADDR)
		{
			pfc_recv_buff[0]=byte;
			pfc_recv_size=1;
			pfc_recv_state=1;
		}
		return;
	}

	pfc_recv_buff[pfc_recv_size++]=byte;

	if(pfc_recv_state==1)
	{
		//等待功能码
		if(pfc_recv_size>=2)
		{
			if(pfc_recv_buff[1]==FNO_IAP)
			{
				pfc_recv_state=2;
			}
			else
			{
				//不是IAP包，重置
				pfc_recv_size=0;
				pfc_recv_state=0;
			}
		}
		return;
	}

	if(pfc_recv_state==2)
	{
		//等待IAP包完整（至少14字节头+CRC）
		if(pfc_recv_size>12)
		{
			pkt=(modbus_iap_t*)pfc_recv_buff;
			//检查是否接收完整
			if(pfc_recv_size>=pkt->size+14)
			{
				pfc_recv_state=3; //接收完成
			}
		}
		return;
	}
}

/*
 * 处理PFC透传完成的数据包
 * 验证CRC后通过CAN转发回上位机
 */
static void pfc_forward_ack()
{
	ushort crc0, crc=0;
	modbus_iap_t *pkt;

	if(pfc_recv_state!=3)return;

	//CRC校验
	crc0=((ushort)pfc_recv_buff[pfc_recv_size-1]<<8)+pfc_recv_buff[pfc_recv_size-2];
	crc16_update(&crc, pfc_recv_buff, pfc_recv_size-2);

	if(crc0==crc)
	{
		pkt=(modbus_iap_t*)pfc_recv_buff;

		//检查是否是进入IAP的ACK
		if(pkt->cmd[0]==1 && pkt->cmd[1]==0)
		{
			pfc_entered_iap=true;
		}

		//通过CAN转发PFC的ACK给上位机
		//注意：保持PFC的地址，上位机通过地址区分是LLC还是PFC的回复
		cansmb_send_data(pfc_recv_buff, pfc_recv_size);
	}

	//重置接收状态
	pfc_recv_size=0;
	pfc_recv_state=0;
}

/*
 * LLC自己的IAP命令处理
 * master:	|addr|fno|cmd|size|data|crc|
 * slave:	|addr|fno|cmd|size|data|crc|
 */
static void on_iap_cmd(modbus_iap_t *pkt)
{
	idle_cnt=0;

	//判断目标地址：如果是PFC地址，则透传
	if(pkt->addr==MODBUS_PFC_ADDR)
	{
		//进入PFC透传模式
		pfc_forward_mode=true;
		pfc_timeout_cnt=0;
		pfc_recv_size=0;
		pfc_recv_state=0;

		//如果PFC还没进入bootloader，先不转发，等PFC准备好
		if(!pfc_entered_iap)
		{
			//发送8字节帧让PFC APP复位
			send_pfc_enter_iap();
			//暂时不回复ACK，上位机会重试
			pkt->cmd[0]=0xff;
			pkt->cmd[1]=0xff;
			return;
		}

		//PFC已进入bootloader，将完整的Modbus包通过UART0转发给PFC
		{
			uchar fwd_buff[sizeof(modbus_pkt_t)];
			ushort fwd_size;
			ushort crc=0;

			fwd_buff[0]=pkt->addr;
			fwd_buff[1]=pkt->fno;
			memmove(&fwd_buff[2], pkt->cmd, 8);    //cmd[8]
			memmove(&fwd_buff[10], &pkt->size, 2); //size
			memmove(&fwd_buff[12], pkt->data, pkt->size); //data

			fwd_size=12+pkt->size;
			crc16_update(&crc, fwd_buff, fwd_size);
			fwd_buff[fwd_size]=(uchar)(crc&0xff);
			fwd_buff[fwd_size+1]=(uchar)(crc>>8);
			fwd_size+=2;

			uart0_forward_to_pfc(fwd_buff, fwd_size);
		}

		//不回复ACK给上位机，等PFC回复后再转发
		pkt->cmd[0]=0xff;
		pkt->cmd[1]=0xff;
		return;
	}

	//LLC自己的IAP处理
	iap_pkt_decode((iap_pkt_t*)pkt->cmd);
	//判断是否需要发送ACK
	if(pkt->cmd[0]==0xff&&pkt->cmd[1]==0xff)return;
	modbuss_send_iap(pkt->cmd,(const void*)pkt->data,pkt->size);
}

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void modbus_iap_init()
{
	time_cnt=0;
	idle_cnt=0;
	pfc_forward_mode=false;
	pfc_recv_size=0;
	pfc_recv_state=0;
	pfc_timeout_cnt=0;
	pfc_enter_iap_cnt=0;
	pfc_entered_iap=false;

	modbuss_init(MODBUS_LOCAL_ADDR);
	modbuss_set_timeout(1000);
	modbuss_set_iap_callback(on_iap_cmd);

	//CAN通信（与上位机/转接板通信）
	modbuss_set_send_callback(cansmb_send_data);
	cansmb_init(CAN_LOCAL_ID, 125000);

	//初始化UART0（与PFC通信）
	uart0_init(115200);
}

/*
 * 查询PFC透传模式是否激活（供iap.c调用）
 */
bool iap_pfc_forward_active(void)
{
	return pfc_forward_mode;
}

/*
 * modbus IAP 任务
 */
void modbus_iap_task()
{
	uchar data;

	//CAN接收处理（主通信）
	modbuss_task();

	//UART0接收处理（PFC回复）
	while(uart0_rx_poll(&data))
	{
		if(pfc_forward_mode)
		{
			pfc_recv_byte(data);
		}
	}

	//UART0发送轮询
	uart0_tx_poll();

	//PFC透传模式下的处理
	if(pfc_forward_mode)
	{
		//检查PFC是否回复完成
		pfc_forward_ack();

		//透传超时处理
		pfc_timeout_cnt++;
		if(pfc_timeout_cnt>=PFC_FORWARD_TIMEOUT)
		{
			pfc_timeout_cnt=0;
			pfc_recv_size=0;
			pfc_recv_state=0;
			//超时不退出透传模式，等待上位机重试
		}
	}

	//如果PFC还没进入IAP模式，周期性发送进入IAP命令（8字节LLC帧格式）
	if(pfc_forward_mode && !pfc_entered_iap)
	{
		pfc_enter_iap_cnt++;
		if(pfc_enter_iap_cnt>=2500)
		{
			pfc_enter_iap_cnt=0;
			send_pfc_enter_iap();
		}

		//发送一段时间后（约2秒），认为PFC已经复位进入bootloader
		//因为PFC收到0xFF命令后会立即复位
		if(pfc_enter_iap_cnt==0)
		{
			static uchar enter_try_cnt=0;
			enter_try_cnt++;
			if(enter_try_cnt>=3) //发送3次后认为PFC已进入bootloader
			{
				pfc_entered_iap=true;
				enter_try_cnt=0;
			}
		}
	}
}
