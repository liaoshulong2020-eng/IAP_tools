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
#include "iap_runtime.h"

/*******************************************************************************
 * ��̬����
 ******************************************************************************/

//�Ƿ����IAPģʽ
static bool iap_flag;

static ulong time_cnt;

//�Ƿ��յ���λ��֪ͨ��ת��APP
static bool jump_flag;

#define DB_INACTIVE_BASE  0x08020000UL
#define DB_IMAGE_MAX      0x0001E000UL
static bool db_mode;
static ulong db_image_size;
static ulong db_image_crc;

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
	ulong offset,rsize,sector_size,remainder,end_offset,write_addr,write_size,remain,sector_remain,buff_offset;
	uchar sector_index,end_sector_index,index;

	offset=addr-FLASH_BASE_ADDR;
	//��ȡ������С
	sector_size=LL_EFLASH_SectorSize_Get(EFLASH);
	//������������
	sector_index=offset/sector_size;
	end_offset=offset+size-1;
	end_sector_index=end_offset/sector_size;
	remainder=offset%sector_size;
	//����պ�Ϊ������ʼ��ַ�����Ȳ�������
	if(remainder==0)
	{
		if(LL_EFLASH_EraseSector(EFLASH,sector_index)!=LL_OK)return false;
	}
	for(index=sector_index+1;index<=end_sector_index;index++)
	{
		if(LL_EFLASH_EraseSector(EFLASH,index)!=LL_OK)return false;
	}

	write_addr=addr;
	buff_offset=0;
	remain=size;
	while(remain>0)
	{
		sector_remain=sector_size-((write_addr-FLASH_BASE_ADDR)%sector_size);
		write_size=(remain<sector_remain)?remain:sector_remain;

		//д��
		rsize=LL_EFLASH_Program(EFLASH,write_addr,((uchar*)buff)+buff_offset,write_size);
		if(rsize!=write_size)return false;

		//У��
		rsize=LL_EFLASH_Verify(EFLASH,write_addr,((uchar*)buff)+buff_offset,write_size);
		if(rsize!=write_size)return false;

		write_addr+=write_size;
		buff_offset+=write_size;
		remain-=write_size;
	}

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
	ulong write_addr=pkt->addr;
	if(db_mode)
	{
		if(pkt->addr<FLASH_BASE_ADDR || pkt->addr+pkt->len>FLASH_BASE_ADDR+db_image_size)
		{pkt->len=0; pkt->size=0; return;}
		write_addr=DB_INACTIVE_BASE+(pkt->addr-FLASH_BASE_ADDR);
	}
	if(!flash_program(write_addr,pkt->data,pkt->len))pkt->len=0;
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

/* cmd=5: persist a new 29-bit IAP CAN ID. Applied after reset. */
static void cmd_write_iap_address(iap_pkt_t *pkt)
{
	if(!iap_runtime_change_address(pkt->addr))
	{
		pkt->cmd=0xffff;
		return;
	}
	pkt->len=0;
	pkt->size=0;
}

static void cmd_db_begin(iap_pkt_t *pkt)
{
	if(pkt->size<8){pkt->cmd=0xffff;return;}
	memmove(&db_image_size,pkt->data,4); memmove(&db_image_crc,pkt->data+4,4);
	if(db_image_size<APP_BASE_ADDR-FLASH_BASE_ADDR || db_image_size>DB_IMAGE_MAX || (db_image_size&7))
	{pkt->cmd=0xffff;return;}
	db_mode=true; iap_flag=true; jump_flag=false; pkt->len=0; pkt->size=0;
}
static void cmd_db_verify(iap_pkt_t *pkt)
{
	ulong crc=0,sp,reset;
	if(!db_mode){pkt->cmd=0xffff;return;}
	memmove(&sp,(void*)DB_INACTIVE_BASE,4); memmove(&reset,(void*)(DB_INACTIVE_BASE+4),4);
	crc32_update(&crc,(void*)DB_INACTIVE_BASE,db_image_size);
	if((sp&0x2FFE0000UL)!=0x20000000UL || reset<FLASH_BASE_ADDR || reset>=FLASH_BASE_ADDR+db_image_size || crc!=db_image_crc)
	{pkt->cmd=0xffff;return;}
	pkt->len=0; pkt->size=0;
}
static void cmd_db_commit(iap_pkt_t *pkt)
{
	EFLASH_BankAddrMapETypeDef next; ulong crc=0; uchar current;
	if(!db_mode){pkt->cmd=0xffff;return;}
	crc32_update(&crc,(void*)DB_INACTIVE_BASE,db_image_size);
	if(crc!=db_image_crc){pkt->cmd=0xffff;return;}
	next=(__LL_EFLASH_BankAddrMap_Get(EFLASH)==EFLASH_BANK_ADDR_MAP_BANK1)?EFLASH_BANK_ADDR_MAP_BANK0:EFLASH_BANK_ADDR_MAP_BANK1;
	current=(__LL_EFLASH_BankAddrMap_Get(EFLASH)==EFLASH_BANK_ADDR_MAP_BANK1)?1:0;
	if(!iap_runtime_save_bank_state(current,current?0:1,IAP_IMAGE_TRIAL,0)){pkt->cmd=0xffff;return;}
	__LL_SYSCTRL_EFLASHDblBankSwitch_En(SYSCTRL);
	if(LL_EFLASH_BankMapCfg(EFLASH,next)!=LL_OK){pkt->cmd=0xffff;return;}
	pkt->len=0; pkt->size=0; db_mode=false;
}

static uchar bank_current_physical(void)
{
	return (__LL_EFLASH_BankAddrMap_Get(EFLASH)==EFLASH_BANK_ADDR_MAP_BANK1)?1:0;
}

static ulong bank_logical_base(uchar physical)
{
	return (physical==bank_current_physical())?FLASH_BASE_ADDR:DB_INACTIVE_BASE;
}

static bool bank_image_valid(uchar physical)
{
	ulong base,size,expected,actual=0,sp,reset;
	if(physical>1)return false;
	base=bank_logical_base(physical);
	memmove(&sp,(void*)base,4); memmove(&reset,(void*)(base+4),4);
	memmove(&size,(void*)(base+0x7000UL),4); memmove(&expected,(void*)(base+0x7004UL),4);
	if((sp&0x2FFE0000UL)!=0x20000000UL || reset<FLASH_BASE_ADDR || reset>=FLASH_BASE_ADDR+0x8000UL || size==0 || size>APP_MAX_SIZE)return false;
	crc32_update(&actual,(void*)(base+0x8000UL),size);
	return actual==expected;
}

static void cmd_bank_status(iap_pkt_t *pkt)
{
	const iap_config_t *cfg=iap_runtime_config();
	ulong map=__LL_EFLASH_BankAddrMap_Get(EFLASH),mode=__LL_EFLASH_BankMode_Get(EFLASH),id=iap_runtime_can_id();
	pkt->data[0]=bank_current_physical(); pkt->data[1]=(uchar)map; pkt->data[2]=(uchar)mode;
	pkt->data[3]=bank_image_valid(0)?1:0; pkt->data[4]=bank_image_valid(1)?1:0;
	pkt->data[5]=(uchar)cfg->active_bank; pkt->data[6]=(uchar)cfg->pending_bank;
	pkt->data[7]=(uchar)cfg->image_state; pkt->data[8]=(uchar)cfg->boot_attempts;
	memmove(pkt->data+9,&id,4); pkt->len=13; pkt->size=13;
}

static void cmd_bank_verify(iap_pkt_t *pkt)
{
	uchar bank=(uchar)pkt->addr; pkt->data[0]=bank_image_valid(bank)?1:0; pkt->len=1; pkt->size=1;
}

static void cmd_bank_switch(iap_pkt_t *pkt)
{
	uchar target=(uchar)pkt->addr;
	EFLASH_BankAddrMapETypeDef map=target?EFLASH_BANK_ADDR_MAP_BANK1:EFLASH_BANK_ADDR_MAP_BANK0;
	if(target>1 || !bank_image_valid(target)){pkt->cmd=0xffff;return;}
	if(target==bank_current_physical()){pkt->len=0;pkt->size=0;return;}
	if(!iap_runtime_save_bank_state(bank_current_physical(),target,IAP_IMAGE_TRIAL,0)){pkt->cmd=0xffff;return;}
	__LL_SYSCTRL_EFLASHDblBankSwitch_En(SYSCTRL);
	if(LL_EFLASH_BankMapCfg(EFLASH,map)!=LL_OK){pkt->cmd=0xffff;return;}
	pkt->len=0; pkt->size=0;
}

static void cmd_bank_confirm(iap_pkt_t *pkt)
{
	uchar current=bank_current_physical();
	if(!bank_image_valid(current) || !iap_runtime_save_bank_state(current,IAP_BANK_NONE,IAP_IMAGE_CONFIRMED,0)){pkt->cmd=0xffff;return;}
	pkt->len=0; pkt->size=0;
}

static void cmd_bank_rollback(iap_pkt_t *pkt)
{
	pkt->addr=bank_current_physical()?0:1; cmd_bank_switch(pkt);
}
static void cmd_bank_reset(iap_pkt_t *pkt){(void)pkt;NVIC_SystemReset();}

static void cmd_capability_query(iap_pkt_t *pkt)
{
	ulong capabilities=0x0000001FUL,id=iap_runtime_can_id();
	pkt->data[0]=1;pkt->data[1]=0;
	pkt->data[2]=(uchar)(MODBUS_MAX_REG_PAYLOAD_SIZE&0xff);
	pkt->data[3]=(uchar)(MODBUS_MAX_REG_PAYLOAD_SIZE>>8);
	memmove(pkt->data+4,&capabilities,4);
	pkt->data[8]=1;pkt->data[9]=1;pkt->data[10]=0;pkt->data[11]=0;
	memmove(pkt->data+12,&id,4);pkt->data[16]=2;pkt->data[17]=1;
	pkt->len=18;pkt->size=18;
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
	if(__LL_EFLASH_BankMode_Get(EFLASH)!=EFLASH_BANK_MODE_DOUBLE)
	{
		__LL_SYSCTRL_EFLASHDblBankSwitch_En(SYSCTRL);
		(void)LL_EFLASH_BankModeCfg(EFLASH,EFLASH_BANK_MODE_DOUBLE);
	}
	iap_flag=false;
	time_cnt=0;
	jump_flag=false;
	db_mode=false; db_image_size=0; db_image_crc=0;
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
	case 5:cmd_write_iap_address(pkt);break;
	case 6:cmd_db_begin(pkt);break;
	case 7:cmd_db_verify(pkt);break;
	case 8:cmd_db_commit(pkt);break;
	case 0x20:cmd_bank_status(pkt);break;
	case 0x21:cmd_bank_verify(pkt);break;
	case 0x22:cmd_bank_switch(pkt);break;
	case 0x23:cmd_bank_confirm(pkt);break;
	case 0x24:cmd_bank_rollback(pkt);break;
	case 0x25:cmd_bank_reset(pkt);break;
	case 0x30:cmd_capability_query(pkt);break;
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
