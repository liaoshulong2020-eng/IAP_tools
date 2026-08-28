/*
 * iap.c - PFC Bootloader IAP鏍稿績閫昏緫
 *
 * PFC bootloader琚姩鎺ユ敹LLC杞彂鐨処AP鍛戒护锛屼笉涓诲姩鍙戦€佷换浣曡姹? *
 *  Created on: 2024骞?鏈?6鏃? *      Author: Liang Jinfeng
 *  Modified: 2025 - 閫傞厤PFC bootloader锛岀Щ闄や富鍔ㄨ姹傞€昏緫
 */

#include "iap.h"
#include "vars.h"
#include "crc32.h"
#include "modbuss.h"
#include "iap_runtime.h"

#define RAM_BASE_ADDR      0x20000000UL
#define RAM_END_ADDR       0x20020000UL
#define APP_END_ADDR       (FLASH_BASE_ADDR+FLASH_MAX_SIZE)
#define APP_INFO_SIZE      8UL
#define PFC_IAP_BOOT_MAGIC_ADDR     0x2001FFF0UL
#define PFC_IAP_BOOT_MAGIC_VALUE    0x50464349UL

/*******************************************************************************
 * 闈欐€佸彉閲? ******************************************************************************/

//鏄惁杩涘叆IAP妯″紡
static bool iap_flag;

static ulong time_cnt;

//鏄惁鏀跺埌閫€鍑洪€氱煡锛屽噯澶囪烦杞珹PP
static bool jump_flag;

#define DB_INACTIVE_BASE  0x08020000UL
#define DB_IMAGE_MAX      0x0001E000UL
static bool db_mode;
static ulong db_image_size;
static ulong db_image_crc;

/*******************************************************************************
 * 闈欐€佸嚱鏁? ******************************************************************************/

//瀹氫箟鍑芥暟鎸囬拡
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
 * 璺宠浆鍒癆PP
 */
static void jump_to_app()
{
	ulong app;
	app_main_t appmain;

	if(!iap_flash_verify())return;

	__disable_irq();
	app=*((ulong*)(APP_BASE_ADDR+4));
	appmain=(app_main_t)app;
	//璁剧疆鏍堥《鍦板潃
	__set_MSP(*((ulong*)APP_BASE_ADDR));
	//鍚姩APP
	appmain();
}

/*
 * @brief 缂栫▼Flash
 * @param addr 缂栫▼寮€濮嬪湴鍧€锛屽繀椤绘寜8瀛楄妭瀵归綈
 * @param buff 寰呭啓鍏lash鐨勬暟鎹紦鍐插尯
 * @param size 鏁版嵁闀垮害锛屽繀椤绘槸8鐨勫€嶆暟
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
	//鑾峰彇鎵囧尯澶у皬
	sector_size=LL_EFLASH_SectorSize_Get(EFLASH);
	//璁＄畻鎵囧尯绱㈠紩
	sector_index=offset/sector_size;
	remainder=offset%sector_size;
	//濡傛灉鍒氬ソ涓烘墖鍖鸿捣濮嬪湴鍧€锛屽垯鍏堟摝闄ゆ墖鍖?	if(remainder==0)
	{
		if(LL_EFLASH_EraseSector(EFLASH,sector_index)!=LL_OK)return false;
	}

	//鍐欏叆
	rsize=LL_EFLASH_Program(EFLASH,addr,(uchar*)buff,size);
	if(rsize!=size)return false;

	//鏍￠獙
	rsize=LL_EFLASH_Verify(EFLASH,addr,(uchar*)buff,size);
	if(rsize!=size)return false;

	return true;
}

/*
 * cmd=0锛氶€€鍑篒AP妯″紡
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
 * cmd=1锛氳繘鍏AP妯″紡
 */
static void cmd_enter_iap(iap_pkt_t *pkt)
{
	uchar who;

	iap_flag=true;
	jump_flag=false;
	who=pkt->data[0];
	//濡傛灉鏄嚜宸卞彂閫佺殑璇锋眰锛屽垯涓嶅洖澶岮CK
	if(who==0)pkt->cmd=0xffff;
}

/*
 * cmd=2锛氳Flash鍛戒护
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
 * cmd=3锛氬啓Flash鍛戒护
 */
static void cmd_write_flash(iap_pkt_t *pkt)
{
	ulong write_addr=pkt->addr;
	bool valid=db_mode?range_is_valid(pkt->addr,pkt->len,FLASH_BASE_ADDR,FLASH_BASE_ADDR+db_image_size):range_is_valid(pkt->addr,pkt->len,APP_BASE_ADDR,APP_END_ADDR);
	if(!iap_flag || pkt->len>IAP_MAX_PAYLOAD_SIZE || !valid)
	{
		pkt->len=0;
		pkt->size=0;
		return;
	}
	if(db_mode)write_addr=DB_INACTIVE_BASE+(pkt->addr-FLASH_BASE_ADDR);
	if(!flash_program(write_addr,pkt->data,pkt->len))pkt->len=0;
	pkt->size=0;
}

/*
 * cmd=4锛氬啓鏍￠獙鐮? */
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

static uchar bank_current_physical(void){return (__LL_EFLASH_BankAddrMap_Get(EFLASH)==EFLASH_BANK_ADDR_MAP_BANK1)?1:0;}
static ulong bank_logical_base(uchar physical){return (physical==bank_current_physical())?FLASH_BASE_ADDR:DB_INACTIVE_BASE;}
static bool bank_image_valid(uchar physical)
{
	ulong base,size,expected,actual=0,sp,reset;
	if(physical>1)return false; base=bank_logical_base(physical);
	memmove(&sp,(void*)base,4); memmove(&reset,(void*)(base+4),4);
	memmove(&size,(void*)(base+0x7000UL),4); memmove(&expected,(void*)(base+0x7004UL),4);
	if((sp&0x2FFE0000UL)!=0x20000000UL || reset<FLASH_BASE_ADDR || reset>=FLASH_BASE_ADDR+0x8000UL || size==0 || size>APP_MAX_SIZE)return false;
	crc32_update(&actual,(void*)(base+0x8000UL),size); return actual==expected;
}
static void cmd_bank_status(iap_pkt_t *pkt)
{
	const iap_config_t *cfg=iap_runtime_config(); ulong map=__LL_EFLASH_BankAddrMap_Get(EFLASH),mode=__LL_EFLASH_BankMode_Get(EFLASH),id=iap_runtime_can_id();
	pkt->data[0]=bank_current_physical(); pkt->data[1]=(uchar)map; pkt->data[2]=(uchar)mode;
	pkt->data[3]=bank_image_valid(0)?1:0; pkt->data[4]=bank_image_valid(1)?1:0; pkt->data[5]=(uchar)cfg->active_bank;
	pkt->data[6]=(uchar)cfg->pending_bank; pkt->data[7]=(uchar)cfg->image_state; pkt->data[8]=(uchar)cfg->boot_attempts;
	memmove(pkt->data+9,&id,4); pkt->len=13; pkt->size=13;
}
static void cmd_bank_verify(iap_pkt_t *pkt){uchar bank=(uchar)pkt->addr;pkt->data[0]=bank_image_valid(bank)?1:0;pkt->len=1;pkt->size=1;}
static void cmd_bank_switch(iap_pkt_t *pkt)
{
	uchar target=(uchar)pkt->addr; EFLASH_BankAddrMapETypeDef map=target?EFLASH_BANK_ADDR_MAP_BANK1:EFLASH_BANK_ADDR_MAP_BANK0;
	if(target>1 || !bank_image_valid(target)){pkt->cmd=0xffff;return;}
	if(target==bank_current_physical()){pkt->len=0;pkt->size=0;return;}
	if(!iap_runtime_save_bank_state(bank_current_physical(),target,IAP_IMAGE_TRIAL,0)){pkt->cmd=0xffff;return;}
	__LL_SYSCTRL_EFLASHDblBankSwitch_En(SYSCTRL); if(LL_EFLASH_BankMapCfg(EFLASH,map)!=LL_OK){pkt->cmd=0xffff;return;}
	pkt->len=0;pkt->size=0;
}
static void cmd_bank_confirm(iap_pkt_t *pkt)
{
	uchar current=bank_current_physical(); if(!bank_image_valid(current)||!iap_runtime_save_bank_state(current,IAP_BANK_NONE,IAP_IMAGE_CONFIRMED,0)){pkt->cmd=0xffff;return;}pkt->len=0;pkt->size=0;
}
static void cmd_bank_rollback(iap_pkt_t *pkt){pkt->addr=bank_current_physical()?0:1;cmd_bank_switch(pkt);}
static void cmd_bank_reset(iap_pkt_t *pkt){(void)pkt;NVIC_SystemReset();}

/*******************************************************************************
 * 鎺ュ彛鍑芥暟
 ******************************************************************************/

/*
 * 鍒濆鍖? */
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
 * 鏍￠獙Flash
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
 * IAP鏁版嵁鍖呰В鐮? */
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
	default:
		pkt->cmd=0xffff;
		pkt->size=0;
		break;
	}
}

/*
 * IAP浠诲姟
 * PFC bootloader琚姩绛夊緟LLC鍙戞潵鐨勫懡浠わ紝涓嶄富鍔ㄨ姹? */
void iap_task()
{
	//濡傛灉宸茬粡杩涘叆IAP妯″紡锛屽垯閫€鍑猴紙绛夊緟IAP鍛戒护澶勭悊锛?	if(iap_flag)return;

	//瓒呮椂鍚庡皾璇曡烦杞珹PP
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
