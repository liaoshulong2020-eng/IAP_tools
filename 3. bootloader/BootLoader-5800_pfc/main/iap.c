/*
 * iap.c - PFC Bootloader IAP核心逻辑
 *
 * PFC bootloader被动接收LLC转发的IAP命令，不主动发送任何请求
 *
 *  Created on: 2024年3月26日
 *      Author: Liang Jinfeng
 *  Modified: 2025 - 适配PFC bootloader，移除主动请求逻辑
 */

#include "iap.h"
#include "vars.h"
#include "crc32.h"
#include "modbuss.h"

/*******************************************************************************
 * 静态变量
 ******************************************************************************/

//是否进入IAP模式
static bool iap_flag;

static ulong time_cnt;

//是否收到退出通知，准备跳转APP
static bool jump_flag;

/*******************************************************************************
 * 静态函数
 ******************************************************************************/

//定义函数指针
typedef void (*app_main_t)(void);

/*
 * 跳转到APP
 */
static void jump_to_app()
{
	ulong app;
	app_main_t appmain;

	__disable_irq();
	app=*((ulong*)(APP_BASE_ADDR+4));
	appmain=(app_main_t)app;
	//设置栈顶地址
	__set_MSP(*((ulong*)APP_BASE_ADDR));
	//启动APP
	appmain();
}

/*
 * @brief 编程Flash
 * @param addr 编程开始地址，必须按8字节对齐
 * @param buff 待写入Flash的数据缓冲区
 * @param size 数据长度，必须是8的倍数
 */
static bool flash_program(ulong addr,const void *buff,ulong size)
{
	ulong offset,rsize,sector_size,remainder;
	uchar sector_index;

	offset=addr-FLASH_BASE_ADDR;
	//获取扇区大小
	sector_size=LL_EFLASH_SectorSize_Get(EFLASH);
	//计算扇区索引
	sector_index=offset/sector_size;
	remainder=offset%sector_size;
	//如果刚好为扇区起始地址，则先擦除扇区
	if(remainder==0)
	{
		if(LL_EFLASH_EraseSector(EFLASH,sector_index)!=LL_OK)return false;
	}

	//写入
	rsize=LL_EFLASH_Program(EFLASH,addr,(uchar*)buff,size);
	if(rsize!=size)return false;

	//校验
	rsize=LL_EFLASH_Verify(EFLASH,addr,(uchar*)buff,size);
	if(rsize!=size)return false;

	return true;
}

/*
 * cmd=0：退出IAP模式
 */
static void cmd_exit_iap(iap_pkt_t *pkt)
{
	if(!iap_flash_verify())
	{
		pkt->cmd=0xffff;
		return;
	}
	time_cnt=0;
	iap_flag=false;
	jump_flag=true;
}

/*
 * cmd=1：进入IAP模式
 */
static void cmd_enter_iap(iap_pkt_t *pkt)
{
	uchar who;

	iap_flag=true;
	jump_flag=false;
	who=pkt->data[0];
	//如果是自己发送的请求，则不回复ACK
	if(who==0)pkt->cmd=0xffff;
}

/*
 * cmd=2：读Flash命令
 */
static void cmd_read_flash(iap_pkt_t *pkt)
{
	memmove(pkt->data,(void*)pkt->addr,pkt->len);
	pkt->size=pkt->len;
}

/*
 * cmd=3：写Flash命令
 */
static void cmd_write_flash(iap_pkt_t *pkt)
{
	if(!flash_program(pkt->addr,pkt->data,pkt->len))pkt->len=0;
	pkt->size=0;
}

/*
 * cmd=4：写校验码
 */
static void cmd_write_checksum(iap_pkt_t *pkt)
{
	if(!flash_program(ARG_BASE_ADDR,pkt->data,pkt->len))pkt->len=0;
	pkt->size=0;
}

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void iap_init()
{
	iap_flag=false;
	time_cnt=0;
	jump_flag=false;
}

/*
 * 校验Flash
 */
bool iap_flash_verify()
{
	ulong appsize,appcrc,crc=0;

	appsize=*((ulong*)ARG_BASE_ADDR);
	appcrc=*((ulong*)(ARG_BASE_ADDR+4));
	if(appsize>APP_MAX_SIZE)return false;

	crc32_update(&crc,(void*)APP_BASE_ADDR,appsize);
	if(crc==appcrc)return true;

	return false;
}

/*
 * IAP数据包解码
 */
void iap_pkt_decode(iap_pkt_t *pkt)
{
	switch(pkt->cmd)
	{
	case 0:cmd_exit_iap(pkt);break;
	case 1:cmd_enter_iap(pkt);break;
	case 2:cmd_read_flash(pkt);break;
	case 3:cmd_write_flash(pkt);break;
	case 4:cmd_write_checksum(pkt);break;
	}
}

/*
 * IAP任务
 * PFC bootloader被动等待LLC发来的命令，不主动请求
 */
void iap_task()
{
	//如果已经进入IAP模式，则退出（等待IAP命令处理）
	if(iap_flag)return;

	//超时后尝试跳转APP
	time_cnt++;
	if(time_cnt<20000)return;
	time_cnt=0;

	if(iap_flash_verify())
	{
		LL_IWDG_DeInit(IWDG);
		LL_TMR_DeInit(TMR8);
		jump_to_app();
	}
}
