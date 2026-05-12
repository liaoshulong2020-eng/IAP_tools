/*
 * pmbus.c
 *
 *  Created on: 2024年5月21日
 *      Author: Liang Jinfeng
 */

#include "pmbusm.h"
#include "crc8.h"
#include "soft_i2c.h"
#include "vars.h"
#include "delay.h"

/*******************************************************************************
 * 静态函数
 ******************************************************************************/

/*
 * 设置在线状态
 */
static void set_online(uchar addr)
{
	uchar byte,bit;
	byte=addr/8;
	bit=addr%8;
	set_bit(online_bitmap[byte],bit);
}

/*
 * 清除在线状态
 */
static void clear_online(uchar addr)
{
	uchar byte,bit;
	byte=addr/8;
	bit=addr%8;
	clear_bit(online_bitmap[byte],bit);
}

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void pmbusm_init()
{
	i2c_init();
	i2c_set_delay(100,2000);
}

/*
 * 探测从机
 */
void pmbusm_probe_slave()
{
	uchar addr;

	online_num=0;
	memset(online_bitmap,0,sizeof(online_bitmap));
	for(addr=PMBUS_ADDR_MIN;addr<=PMBUS_ADDR_MAX;addr++)
	{
		i2c_start();
		//探测到从机
		if(i2c_write(addr<<1))
		{
			set_online(addr);
			online_num++;
		}
		else clear_online(addr);
		i2c_stop();
		delayms(2);
	}
}

/*
 * 获取第一个在线从机地址
 */
uchar pmbusm_get_first_slave()
{
	uchar addr,byte,bit;
	if(online_num==0)return 0xff;
	for(addr=PMBUS_ADDR_MIN;addr<=PMBUS_ADDR_MAX;addr++)
	{
		byte=addr/8;
		bit=addr%8;
		if(online_bitmap[byte]&(1<<bit))return addr;
	}
	return 0xff;
}

/*
 * 写命令数据
 */
bool pmbusm_write_cmd_data(uchar addr,uchar cmd,const void *data,uchar size)
{
	uchar waddr,i,crc=0;

	waddr=addr<<1;
	//计算CRC
	crc8_update(&crc,&waddr,1);
	crc8_update(&crc,&cmd,1);
	crc8_update(&crc,data,size);

	//start
	i2c_start();
	//write addr|w
	if(!i2c_write(waddr))goto error;
	//write cmd
	if(!i2c_write(cmd))goto error;
	//write data
	for(i=0;i<size;i++)
	{
		if(i2c_write(((uchar*)data)[i])==false)goto error;
	}
	//write pec
	if(i2c_write(crc)==false)goto error;

	i2c_stop();
	return true;

error:
	i2c_stop();
	return false;
}

/*
 * 读命令数据
 */
bool pmbusm_read_cmd_data(uchar addr,uchar cmd,void *data,uchar size)
{
	uchar waddr,raddr,i,crc,crc2=0;

	waddr=addr<<1;
	raddr=(addr<<1)|1;

	//start
	i2c_start();
	//write addr|w
	if(!i2c_write(waddr))goto error;
	//write cmd
	if(!i2c_write(cmd))goto error;
	//re-start
	i2c_start();
	//write addr|r
	if(!i2c_write(raddr))goto error;
	//read data
	for(i=0;i<size;i++)
	{
		((uchar*)data)[i]=i2c_read(true);
	}
	//read pec
	crc=i2c_read(false);
	//crc校验
	crc8_update(&crc2,&waddr,1);
	crc8_update(&crc2,&cmd,1);
	crc8_update(&crc2,&raddr,1);
	crc8_update(&crc2,data,size);
	crc8_update(&crc2,&crc,1);
	if(crc2!=0)goto error;

	i2c_stop();
	return true;

error:
	i2c_stop();
	return false;
}

/*
 * PMBus任务
 */
void pmbusm_task()
{
//	probe_slave();
//	test_write();
//	test_read();
}
