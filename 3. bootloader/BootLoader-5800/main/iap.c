/*
 * iap.c
 *
 *  Created on: 2024年3月26日
 *      Author: Liang Jinfeng
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

//是否收到上位机通知跳转到APP
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
	//运行APP
	appmain();
}

/*
 * @brief 编程Flash
 * @param addr 编程开始地址，必须按8字节对齐
 * @param buff 待写入Flash的数据缓冲区
 * @param size 缓冲区长度，必须是8的整数倍
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
	//如果刚好为扇区开始地址，则先擦除扇区
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
 * master:	|cmd:0|addr:0|len:0|size:0|
 * slave:	|cmd:0|addr:0|len:0|size:0|
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
 * master:	|cmd:1|addr:0|len:1|size:1|data:who|
 * slave:	|cmd:1|addr:0|len:1|size:1|data:who|
 */
static void cmd_enter_iap(iap_pkt_t *pkt)
{
	uchar who;

	iap_flag=true;
	jump_flag=false;
	who=pkt->data[0];
	//如果是自己发送的请求，则不再回复ACK
	if(who==0)pkt->cmd=0xffff;
}

/*
 * cmd=2：读Flash数据
 * master:	|cmd:2|addr|len|size:0|
 * slave:	|cmd:2|addr|len|size|data|
 */
static void cmd_read_flash(iap_pkt_t *pkt)
{
	memmove(pkt->data,(void*)pkt->addr,pkt->len);
	pkt->size=pkt->len;
}

/*
 * cmd=3：写Flash数据
 * master:	|cmd:3|addr|len|size|data|
 * slave:	|cmd:3|addr|len|size:0|
 */
static void cmd_write_flash(iap_pkt_t *pkt)
{
	if(!flash_program(pkt->addr,pkt->data,pkt->len))pkt->len=0;
	pkt->size=0;
}

/*
 * cmd=4：写校验码
 * master:	|cmd:4|addr|len:8|size:8|data|
 * slave:	|cmd:4|addr|len:8|size:0|
 */
static void cmd_write_checksum(iap_pkt_t *pkt)
{
	if(!flash_program(ARG_BASE_ADDR,pkt->data,pkt->len))pkt->len=0;
	pkt->size=0;
}

#if(USE_UART)
/*
 * 发送进入IAP模式命令给主机
 */
static void request_enter_iap()
{
	static volatile ulong cnt=0;
	modbus_iap_t mpkt;
	iap_pkt_t *pkt;

	//如果收到上位机通知跳转到APP的通知，则不再发送请求
	if(jump_flag)return;

	cnt++;
	if(cnt<2500)return;
	cnt=0;

	mpkt.addr=MODBUS_LOCAL_ADDR;
	mpkt.fno=FNO_IAP;

	//master:	|cmd:1|addr:0|len:1|size:1|data:who|
	pkt=(iap_pkt_t*)mpkt.cmd;
	pkt->cmd=1;
	pkt->addr=0;
	pkt->len=1;
	pkt->size=1;
	pkt->data[0]=0; //who=0: MCU发出请求
	modbuss_send_iap(mpkt.cmd,(const void*)mpkt.data,mpkt.size);
}
#endif

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
 */
void iap_task()
{
	//如果已经进入IAP模式，则退出
	if(iap_flag)return;

#if(USE_UART)
	request_enter_iap();
#endif

	time_cnt++;
	if(time_cnt<20000)return;
	time_cnt=0;

	if(iap_flash_verify())
	{
		LL_IWDG_DeInit(IWDG);
		LL_TMR_DeInit(TMR8);

//#if(USE_UART)
//		LL_UART_DeInit(UART1);
//#elif(USE_CAN)
//		LL_CAN_DeInit(CAN1);
//#elif(USE_PMBUS)
//		LL_I2C_DeInit(I2C0);
//#endif
		jump_to_app();
	}
}
