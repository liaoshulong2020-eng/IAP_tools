/*
 * iap.c
 *
 *  Created on: 2024��3��26��
 *      Author: Liang Jinfeng
 */

#include "iap.h"
#include "vars.h"
#include "crc32.h"
#include "modbuss.h"

/*******************************************************************************
 * ��̬����
 ******************************************************************************/

//�Ƿ����IAPģʽ
static bool iap_flag;

static ulong time_cnt;

//�Ƿ��յ���λ��֪ͨ��ת��APP
static bool jump_flag;

/*******************************************************************************
 * ��̬����
 ******************************************************************************/

//���庯��ָ��
typedef void (*app_main_t)(void);

/*
 * ��ת��APP
 */
static void jump_to_app()
{
	ulong app;
	app_main_t appmain;

	__disable_irq();
	app=*((ulong*)(APP_BASE_ADDR+4));
	appmain=(app_main_t)app;
	//����ջ����ַ
	__set_MSP(*((ulong*)APP_BASE_ADDR));
	//����APP
	appmain();
}

/*
 * @brief ���Flash
 * @param addr ��̿�ʼ��ַ�����밴8�ֽڶ���
 * @param buff ��д��Flash�����ݻ�����
 * @param size ���������ȣ�������8��������
 */
static bool flash_program(ulong addr,const void *buff,ulong size)
{
	ulong offset,rsize,sector_size,remainder;
	uchar sector_index;

	offset=addr-FLASH_BASE_ADDR;
	//��ȡ������С
	sector_size=LL_EFLASH_SectorSize_Get(EFLASH);
	//������������
	sector_index=offset/sector_size;
	remainder=offset%sector_size;
	//����պ�Ϊ������ʼ��ַ�����Ȳ�������
	if(remainder==0)
	{
		if(LL_EFLASH_EraseSector(EFLASH,sector_index)!=LL_OK)return false;
	}

	//д��
	rsize=LL_EFLASH_Program(EFLASH,addr,(uchar*)buff,size);
	if(rsize!=size)return false;

	//У��
	rsize=LL_EFLASH_Verify(EFLASH,addr,(uchar*)buff,size);
	if(rsize!=size)return false;

	return true;
}

/*
 * cmd=0���˳�IAPģʽ
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
 * cmd=1������IAPģʽ
 * master:	|cmd:1|addr:0|len:1|size:1|data:who|
 * slave:	|cmd:1|addr:0|len:1|size:1|data:who|
 */
static void cmd_enter_iap(iap_pkt_t *pkt)
{
	uchar who;

	iap_flag=true;
	jump_flag=false;
	who=pkt->data[0];
	//������Լ����͵��������ٻظ�ACK
	if(who==0)pkt->cmd=0xffff;
}

/*
 * cmd=2����Flash����
 * master:	|cmd:2|addr|len|size:0|
 * slave:	|cmd:2|addr|len|size|data|
 */
static void cmd_read_flash(iap_pkt_t *pkt)
{
	memmove(pkt->data,(void*)pkt->addr,pkt->len);
	pkt->size=pkt->len;
}

/*
 * cmd=3��дFlash����
 * master:	|cmd:3|addr|len|size|data|
 * slave:	|cmd:3|addr|len|size:0|
 */
static void cmd_write_flash(iap_pkt_t *pkt)
{
	if(!flash_program(pkt->addr,pkt->data,pkt->len))pkt->len=0;
	pkt->size=0;
}

/*
 * cmd=4��дУ����
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
 * ���ͽ���IAPģʽ���������
 */
static void request_enter_iap()
{
	static volatile ulong cnt=0;
	modbus_iap_t mpkt;
	iap_pkt_t *pkt;

	//����յ���λ��֪ͨ��ת��APP��֪ͨ�����ٷ�������
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
	pkt->data[0]=0; //who=0: MCU��������
	modbuss_send_iap(mpkt.cmd,(const void*)mpkt.data,mpkt.size);
}
#endif

/*******************************************************************************
 * �ӿں���
 ******************************************************************************/

/*
 * ��ʼ��
 */
void iap_init()
{
	iap_flag=false;
	time_cnt=0;
	jump_flag=false;
}

/*
 * У��Flash
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
 * IAP���ݰ�����
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

	//如果正在进行PFC透传，不跳转到APP
	extern bool iap_pfc_forward_active(void);
	if(iap_pfc_forward_active())return;

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
