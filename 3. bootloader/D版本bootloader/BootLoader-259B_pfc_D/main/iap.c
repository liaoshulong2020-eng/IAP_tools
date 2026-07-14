/*
 * iap.c - PFC Bootloader IAP核心逻辑
 *
 * PFC bootloader被动接收LLC转发的IAP命令，不主动发送任何请�? *
 *  Created on: 2024�?�?6�? *      Author: Liang Jinfeng
 *  Modified: 2025 - 适配PFC bootloader，移除主动请求逻辑
 */

#include "iap.h"
#include "vars.h"
#include "crc32.h"
#include "modbuss.h"

#define RAM_BASE_ADDR      0x20000000UL
#define RAM_END_ADDR       0x20020000UL
#define APP_END_ADDR       (FLASH_BASE_ADDR+FLASH_MAX_SIZE)
#define APP_INFO_SIZE      8UL
#define PFC_IAP_BOOT_MAGIC_ADDR     0x2001FFF0UL
#define PFC_IAP_BOOT_MAGIC_VALUE    0x50464349UL

/*******************************************************************************
 * 静态变�? ******************************************************************************/

//是否进入IAP模式
static bool iap_flag;

static ulong time_cnt;

//是否收到退出通知，准备跳转APP
static bool jump_flag;

/*******************************************************************************
 * 静态函�? ******************************************************************************/

//定义函数指针
typedef void (*app_main_t)(void);

static bool range_is_valid(ulong addr,ulong size,ulong start,ulong end)
{
	if(size==0)return false;
	if(addr<start || addr>=end)return false;
	if(size>(end-addr))return false;
	return true;
}

static bool flash_write_is_aligned(ulong addr,ulong size)
{
	if((addr&0x07UL)!=0)return false;
	if((size&0x07UL)!=0)return false;
	return true;
}

static bool app_info_is_valid(ulong appsize)
{
	if(appsize==0)return false;
	if(appsize>APP_MAX_SIZE)return false;
	if(appsize>(APP_END_ADDR-APP_BASE_ADDR))return false;
	return true;
}

static bool app_vector_is_valid()
{
	ulong sp,reset,reset_addr;

	sp=*((ulong*)APP_BASE_ADDR);
	reset=*((ulong*)(APP_BASE_ADDR+4));
	reset_addr=reset&(~1UL);

	if(sp<RAM_BASE_ADDR || sp>RAM_END_ADDR)return false;
	if((sp&0x03UL)!=0)return false;
	if((reset&0x01UL)==0)return false;
	if(reset_addr<APP_BASE_ADDR || reset_addr>=APP_END_ADDR)return false;
	return true;
}

static bool iap_boot_magic_is_set()
{
	ulong magic=*((volatile ulong*)PFC_IAP_BOOT_MAGIC_ADDR);
	if(magic!=PFC_IAP_BOOT_MAGIC_VALUE)return false;
	*((volatile ulong*)PFC_IAP_BOOT_MAGIC_ADDR)=0;
	return true;
}

/*
 * 跳转到APP
 */
static void jump_to_app()
{
	ulong app;
	app_main_t appmain;

	if(!iap_flash_verify())return;

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

	if(buff==0)return false;
	if(!range_is_valid(addr,size,ARG_BASE_ADDR,APP_END_ADDR))return false;
	if(addr<APP_BASE_ADDR && addr!=ARG_BASE_ADDR)return false;
	if(!flash_write_is_aligned(addr,size))return false;

	offset=addr-FLASH_BASE_ADDR;
	//获取扇区大小
	sector_size=LL_EFLASH_SectorSize_Get(EFLASH);
	//计算扇区索引
	sector_index=offset/sector_size;
	remainder=offset%sector_size;
	//如果刚好为扇区起始地址，则先擦除扇�?	if(remainder==0)
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
	if(pkt->len>IAP_MAX_PAYLOAD_SIZE || !range_is_valid(pkt->addr,pkt->len,ARG_BASE_ADDR,APP_END_ADDR))
	{
		pkt->len=0;
		pkt->size=0;
		return;
	}
	memmove(pkt->data,(void*)pkt->addr,pkt->len);
	pkt->size=pkt->len;
}

/*
 * cmd=3：写Flash命令
 */
static void cmd_write_flash(iap_pkt_t *pkt)
{
	if(!iap_flag || pkt->len>IAP_MAX_PAYLOAD_SIZE || !range_is_valid(pkt->addr,pkt->len,APP_BASE_ADDR,APP_END_ADDR))
	{
		pkt->len=0;
		pkt->size=0;
		return;
	}
	if(!flash_program(pkt->addr,pkt->data,pkt->len))pkt->len=0;
	pkt->size=0;
}

/*
 * cmd=4：写校验�? */
static void cmd_write_checksum(iap_pkt_t *pkt)
{
	ulong appsize;

	if(!iap_flag || pkt->len<APP_INFO_SIZE || pkt->len>IAP_MAX_PAYLOAD_SIZE || !flash_write_is_aligned(ARG_BASE_ADDR,pkt->len))
	{
		pkt->len=0;
		pkt->size=0;
		return;
	}
	memmove(&appsize,pkt->data,sizeof(appsize));
	if(!app_info_is_valid(appsize))
	{
		pkt->len=0;
		pkt->size=0;
		return;
	}
	if(!flash_program(ARG_BASE_ADDR,pkt->data,pkt->len))pkt->len=0;
	pkt->size=0;
}

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始�? */
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
	if(!app_info_is_valid(appsize))return false;
	if(!app_vector_is_valid())return false;

	crc32_update(&crc,(void*)APP_BASE_ADDR,appsize);
	if(crc==appcrc)return true;

	return false;
}

/*
 * IAP数据包解�? */
void iap_pkt_decode(iap_pkt_t *pkt)
{
	if(pkt==0)return;
	switch(pkt->cmd)
	{
	case 0:cmd_exit_iap(pkt);break;
	case 1:cmd_enter_iap(pkt);break;
	case 2:cmd_read_flash(pkt);break;
	case 3:cmd_write_flash(pkt);break;
	case 4:cmd_write_checksum(pkt);break;
	default:
		pkt->cmd=0xffff;
		pkt->size=0;
		break;
	}
}

/*
 * IAP任务
 * PFC bootloader被动等待LLC发来的命令，不主动请�? */
void iap_task()
{
	//如果已经进入IAP模式，则退出（等待IAP命令处理�?	if(iap_flag)return;

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
