/*
 * save.c
 *
 *  Created on: 2024年8月6日
 *      Author: Liang Jinfeng
 */

#include "save_data.h"
#include "string.h"

__attribute__((section("FLASH_DATA_ZONE")))
save_data_t flash_data;

save_data_t	user_data;
//数据缓冲区，大小暂定256字节，且必须为8的整数倍，可根据实际数据量修改，最大不超过4KB
static uint8_t data_buff[256];

/*
 * 烧写Flash
 * 注意：size必须为8的整数倍
 */
static bool flash_program(uint32_t addr,const void *buff,uint32_t size)
{

	uint32_t rsize;
	uint8_t sector_index;

	sector_index=SAVE_OFFSET/4096;
	//擦除
	if(LL_EFLASH_EraseSector(EFLASH,sector_index)!=LL_OK)return false;
	//写入
	rsize=LL_EFLASH_Program(EFLASH,addr,(uint8_t*)buff,size);
	if(rsize!=size)return false;

	//校验
	rsize=LL_EFLASH_Verify(EFLASH,addr,(uint8_t*)buff,size);
	if(rsize!=size)return false;

	return true;
}

void flash_load_init()
{
	


}


/*
 * 保存数据
 */
bool save_data_to_flash(save_data_t *data)
{
	const uint8_t flag[]={0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55};

	if(sizeof(save_data_t)>sizeof(data_buff))return false;
	//设置标志
	memmove(data->save_flag,flag,8);
	//复制数据
	memmove(data_buff,data,sizeof(save_data_t));
	//写入Flash
	if(!flash_program(SAVE_ADDR,data_buff,sizeof(data_buff)))return false;

	return true;
}

/*
 * 加载数据
 */
bool load_data_from_flash(save_data_t *data)
{
	const uint8_t flag[]={0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55};

	memmove(data,(void*)SAVE_ADDR,sizeof(save_data_t));
	
	//检查标志
	if(memcmp(data->save_flag,flag,8)!=0)return false;

	

	return true;
}

void user_flash_check()
{
	const uint8_t flag[]={0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55};
	
	if(load_data_from_flash(&flash_data) == false)
	{
		flash_data_init();
		save_data_to_flash(&user_data);
	}
}



