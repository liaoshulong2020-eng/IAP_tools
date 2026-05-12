/*
 * pmbus_iap.c
 *
 *  Created on: 2024年7月26日
 *      Author: Liang Jinfeng
 */

#include "pmbus_iap.h"
#include "pmbusm.h"
#include "vars.h"
#include "delay.h"

/*******************************************************************************
 * 静态变量
 ******************************************************************************/

//发送缓冲区
static uchar txbuff[sizeof(modbus_iap_t)];

//待处理的modbus IAP数据包指针
static modbus_iap_t *handle_pkt;

//是否有IAP数据包要处理
static volatile bool *handle_flag;

//数据包处理完成回调函数指针
static void (*on_pkt_finish)(modbus_iap_t *pkt);

//待处理的IAP数据包
static iap_pkt_t *iap_pkt;

//超时计数器
static volatile ushort time_cnt;

/*******************************************************************************
 * 静态函数
 ******************************************************************************/

/*
 * 进入/退出IAP模式
 */
static void enter_exit_iap(bool enter)
{
	uchar buff[3],buff2[3];

	//cmd=0x6C：|bytecount|maddr|enter|
	buff[0]=2;
	buff[1]=handle_pkt->addr;
	buff[2]=enter;
	//write cmd
	if(pmbusm_write_cmd_data(pmbus_addr,0x6C,buff,sizeof(buff))==false)return;
	delayms(20);
	//read cmd
	memset(buff2,0,sizeof(buff2));
	if(pmbusm_read_cmd_data(pmbus_addr,0x6C,buff2,sizeof(buff2))==false)return;
	//校验
	if(buff[1]!=buff2[1])return;
	if(buff[2]!=buff2[2])return;
	delayms(10);

	if(!enter)
	{
		pmbus_addr=0;
	}

	//命令处理完成
	*handle_flag=false;
}

/*
 * cmd=0：退出IAP模式
 * master:	|cmd:0|addr:0|len:0|size:0|
 * slave:	|cmd:0|addr:0|len:0|size:0|
 */
static void cmd_exit_iap()
{
	enter_exit_iap(false);
}

/*
 * cmd=1：进入IAP模式
 * master:	|cmd:1|addr:0|len:1|size:1|data:who|
 * slave:	|cmd:1|addr:0|len:1|size:1|data:who|
 */
static void cmd_enter_iap()
{
//	//如果没有从机在线，则探测
//	if(online_num==0)
//	{
//		pmbusm_probe_slave();
//		if(online_num==0)return;
//		delayms(10);
//	}

	if(pmbus_addr!=0x55)
	{
		pmbusm_probe_slave();
		delayms(10);
	}
	//获取第一个在线从机地址
	pmbus_addr=pmbusm_get_first_slave();
	if(pmbus_addr==0xff)return;

	enter_exit_iap(true);
}

/*
 * cmd=2：读Flash数据
 * master:	|cmd:2|addr|len|size:0|
 * slave:	|cmd:2|addr|len|size|data|
 */
static void cmd_read_flash()
{
//	memmove(iap_pkt->data,(void*)iap_pkt->addr,iap_pkt->len);
	iap_pkt->size=iap_pkt->len;

	//命令处理完成
	*handle_flag=false;
}

/*
 * 写Flash
 */
static void write_flash()
{
	uchar size,buff[8];
	static volatile ulong last_faddr=0;

	if(last_faddr!=iap_pkt->addr)
	{
		/*
		 * cmd=0x6D：|bytecount|faddr|len|data|
		 */
		//组包
		size=1;
		memmove(txbuff+size,&iap_pkt->addr,4);
		size+=4;
		memmove(txbuff+size,&iap_pkt->len,2);
		size+=2;
		memmove(txbuff+size,iap_pkt->data,iap_pkt->len);
		size+=iap_pkt->len;
//		memmove(txbuff+size,iap_pkt->data,240); //确保240字节
//		size+=240;
		txbuff[0]=size;

		//write cmd
		if(pmbusm_write_cmd_data(pmbus_addr,0x6D,txbuff,size)==false)return;
		//保存Flash地址
		last_faddr=iap_pkt->addr;
		delayms(10);
	}

	/*
	 * cmd=0x6E：|bytecount|faddr|len|status|
	 */
	//read cmd
	if(pmbusm_read_cmd_data(pmbus_addr,0x6E,buff,sizeof(buff))==false)return;
	delayms(10);
	//校验
	if(memcmp(txbuff+1,buff+1,6)!=0)return;
	if(buff[7]!=0)return;

	iap_pkt->size=0;
	//命令处理完成
	*handle_flag=false;
}

/*
 * cmd=3：写Flash数据
 * master:	|cmd:3|addr|len|size|data|
 * slave:	|cmd:3|addr|len|size:0|
 */
static void cmd_write_flash()
{
	write_flash();
}

/*
 * cmd=4：写校验码
 * master:	|cmd:4|addr|len:8|size:8|data|
 * slave:	|cmd:4|addr|len:8|size:0|
 */
static void cmd_write_checksum()
{
	write_flash();
}

/*
 * 数据包处理超时检测
 */
static void check_timeout()
{
	if(!*handle_flag)return;
	time_cnt++;
	if(time_cnt<100)return;
	time_cnt=0;
	*handle_flag=false;
	pmbus_addr=0;
}

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void pmbus_iap_init(modbus_iap_t *pkt,bool *flag)
{
	handle_pkt=pkt;
	handle_flag=flag;
	on_pkt_finish=NULL;
	iap_pkt=(iap_pkt_t*)pkt->cmd;
	time_cnt=0;
}

/*
 * 设置数据包处理完成回调函数指针
 */
void pmbus_iap_set_finish_callback(void (*callback)(modbus_iap_t *pkt))
{
	on_pkt_finish=callback;
}

/*
 * pmbus IAP 任务
 */
void pmbus_iap_task()
{
	if(iap_interface!=1)return;
	//没有接收到数据包则退出
	if(!*handle_flag)return;

	switch(iap_pkt->cmd)
	{
	case 0:cmd_exit_iap();break;
	case 1:cmd_enter_iap();break;
	case 2:cmd_read_flash();break;
	case 3:cmd_write_flash();break;
	case 4:cmd_write_checksum();break;
	}
	//如果命令处理完成
	if(!*handle_flag)
	{
		time_cnt=0;
		if(on_pkt_finish)on_pkt_finish(handle_pkt);
//		//判断是否需要发送ACK
//		if(iap_pkt->cmd==0xffff)return;
//		//发送ACK
//		modbuss_send_iap(modbus_pkt.cmd,(const void*)modbus_pkt.data,modbus_pkt.size);
	}
	//如果命令处理未完成则等待一下
	else delayms(10);

	check_timeout();
}
