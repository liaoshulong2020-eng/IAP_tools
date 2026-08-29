/*
 * iap.c - PFC Bootloader IAP閺嶇ǹ绺鹃柅鏄忕帆
 *
 * PFC bootloader鐞氼偄濮╅幒銉︽暪LLC鏉烆剙褰傞惃鍑P閸涙垝鎶ら敍灞肩瑝娑撹濮╅崣鎴︹偓浣锋崲娴ｆ洝顕Ч? *
 *  Created on: 2024楠??閺??6閺?? *      Author: Liang Jinfeng
 *  Modified: 2025 - 闁倿鍘FC bootloader閿涘瞼些闂勩倓瀵岄崝銊嚞濮瑰倿鈧槒绶?
 */

#include "iap.h"
#include "vars.h"
#include "crc32.h"
#include "modbuss.h"
#include "iap_runtime.h"
#include "sys_mgr.h"

#define RAM_BASE_ADDR      0x20000000UL
#define RAM_END_ADDR       0x20020000UL
#define APP_END_ADDR       (FLASH_BASE_ADDR+FLASH_MAX_SIZE)
#define APP_INFO_SIZE      8UL
#define PFC_IAP_BOOT_MAGIC_ADDR     0x2001FFF0UL
#define PFC_IAP_BOOT_MAGIC_VALUE    0x50464349UL

/*******************************************************************************
 * 闂堟瑦鈧礁褰夐柌? ******************************************************************************/

//閺勵垰鎯佹潻娑樺弳IAP濡?崇础
static bool iap_flag;

static ulong time_cnt;
static ulong boot_wait_started;

//閺勵垰鎯侀弨璺哄煂闁偓閸戞椽鈧氨鐓￠敍灞藉櫙婢跺洩鐑︽潪鐝筆P
static bool jump_flag;

#define DB_INACTIVE_BASE  0x08020000UL
#define DB_IMAGE_MAX      0x0001E000UL
static bool db_mode;
static ulong db_image_size;
static ulong db_image_crc;

/*******************************************************************************
 * 闂堟瑦鈧礁鍤遍弫? ******************************************************************************/

//鐎规矮绠熼崙鑺ユ殶閹稿洭鎷?
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
 * 鐠哄疇娴嗛崚鐧哖P
 */
static void jump_to_app()
{
	ulong app;
	app_main_t appmain;

	if(!iap_flash_verify())return;

	__disable_irq();
	app=*((ulong*)(APP_BASE_ADDR+4));
	appmain=(app_main_t)app;
	//鐠佸墽鐤嗛弽鍫ャ?婇崷鏉挎絻
	__set_MSP(*((ulong*)APP_BASE_ADDR));
	//閸氼垰濮〢PP
	appmain();
}

/*
 * @brief 缂傛牜鈻糉lash
 * @param addr 缂傛牜鈻煎鈧慨瀣勾閸р偓閿涘苯绻?妞ょ粯瀵?8鐎涙濡?靛綊缍?
 * @param buff 瀵板懎鍟撻崗顧宭ash閻ㄥ嫭鏆熼幑顔剧处閸愭彃灏?
 * @param size 閺佺増宓侀梹鍨閿涘苯绻?妞ょ粯妲?8閻ㄥ嫬鈧秵鏆?
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
	//閼惧嘲褰囬幍鍥у隘婢堆冪毈
	sector_size=LL_EFLASH_SectorSize_Get(EFLASH);
	//鐠侊紕鐣婚幍鍥у隘缁便垹绱?
	sector_index=offset/sector_size;
	remainder=offset%sector_size;
	//婵″倹鐏夐崚姘偨娑撶儤澧栭崠楦挎崳婵婀撮崸鈧敍灞藉灟閸忓牊鎽濋梽銈嗗閸??	if(remainder==0)
	{
		if(LL_EFLASH_EraseSector(EFLASH,sector_index)!=LL_OK)return false;
	}

	//閸愭瑥鍙?
	rsize=LL_EFLASH_Program(EFLASH,addr,(uchar*)buff,size);
	if(rsize!=size)return false;

	//閺嶏繝鐛?
	rsize=LL_EFLASH_Verify(EFLASH,addr,(uchar*)buff,size);
	if(rsize!=size)return false;

	return true;
}

/*
 * cmd=0閿涙岸鈧偓閸戠瘨AP濡?崇础
 */
static void cmd_exit_iap(iap_pkt_t *pkt)
{
	if(!iap_flash_verify())
	{
		pkt->cmd=0xffff;
		return;
	}
	time_cnt=0;
	boot_wait_started=sys_millis();
	iap_flag=false;
	jump_flag=true;
}

/*
 * cmd=1閿涙俺绻橀崗顧廇P濡?崇础
 */
static void cmd_enter_iap(iap_pkt_t *pkt)
{
	uchar who;

	iap_flag=true;
	jump_flag=false;
	who=pkt->data[0];
	//婵″倹鐏夐弰顖濆殰瀹稿崬褰傞柅浣烘畱鐠囬攱鐪伴敍灞藉灟娑撳秴娲栨径宀瓹K
	if(who==0)pkt->cmd=0xffff;
}

/*
 * cmd=2閿涙俺顕癋lash閸涙垝鎶?
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
 * cmd=3閿涙艾鍟揊lash閸涙垝鎶?
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
 * cmd=4閿涙艾鍟撻弽锟犵崣閻?? */
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

static void cmd_capability_query(iap_pkt_t *pkt)
{
	ulong capabilities=0x0000001BUL,id=iap_runtime_can_id();
	pkt->data[0]=1;pkt->data[1]=0;
	pkt->data[2]=(uchar)(MODBUS_MAX_REG_PAYLOAD_SIZE&0xff);
	pkt->data[3]=(uchar)(MODBUS_MAX_REG_PAYLOAD_SIZE>>8);
	memmove(pkt->data+4,&capabilities,4);
	pkt->data[8]=1;pkt->data[9]=1;pkt->data[10]=0;pkt->data[11]=0;
	memmove(pkt->data+12,&id,4);pkt->data[16]=1;pkt->data[17]=1;
	pkt->len=18;pkt->size=18;
}

/*******************************************************************************
 * 閹恒儱褰涢崙鑺ユ殶
 ******************************************************************************/

/*
 * 閸掓繂顫愰崠? */
void iap_init()
{
	if(__LL_EFLASH_BankMode_Get(EFLASH)!=EFLASH_BANK_MODE_DOUBLE)
	{
		__LL_SYSCTRL_EFLASHDblBankSwitch_En(SYSCTRL);
		(void)LL_EFLASH_BankModeCfg(EFLASH,EFLASH_BANK_MODE_DOUBLE);
	}
	iap_flag=false;
	time_cnt=0;
	boot_wait_started=sys_millis();
	jump_flag=false;
	db_mode=false; db_image_size=0; db_image_crc=0;
}

/*
 * 閺嶏繝鐛橣lash
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
 * IAP閺佺増宓侀崠鍛靶掗惍? */
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
	case 0x30:cmd_capability_query(pkt);break;
	default:
		pkt->cmd=0xffff;
		pkt->size=0;
		break;
	}
}

/*
 * IAP娴犺濮?
 * PFC bootloader鐞氼偄濮╃粵澶婄窡LLC閸欐垶娼甸惃鍕嚒娴犮倧绱濇稉宥勫瘜閸斻劏顕Ч? */
void iap_task()
{
	//婵″倹鐏夊鑼病鏉╂稑鍙咺AP濡?崇础閿涘苯鍨柅鈧崙鐚寸礄缁涘绶烮AP閸涙垝鎶ゆ径鍕倞閿??	if(iap_flag)return;

	//鐡掑懏妞傞崥搴＄毦鐠囨洝鐑︽潪鐝筆P
	/* Give the LLC gateway enough time to reconnect after both APPs reset. */
	if((ulong)(sys_millis()-boot_wait_started)<15000UL)return;
	time_cnt=0;

	if(iap_flash_verify())
	{
		LL_IWDG_DeInit(IWDG);
		LL_TMR_DeInit(TMR8);
		jump_to_app();
	}
}
