/*
 * pmbus.h
 *
 *  Created on: 2024年5月21日
 *      Author: Liang Jinfeng
 */

#ifndef PMBUSM_H
#define PMBUSM_H

#include "main.h"

/*******************************************************************************
 * 参数配置
 ******************************************************************************/

//PMBus地址范围
#define PMBUS_ADDR_MIN				0x50
#define PMBUS_ADDR_MAX				0x7F

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void pmbusm_init();

/*
 * 探测从机
 */
void pmbusm_probe_slave();

/*
 * 获取第一个在线从机地址
 */
uchar pmbusm_get_first_slave();

/*
 * 写命令数据
 */
bool pmbusm_write_cmd_data(uchar addr,uchar cmd,const void *data,uchar size);

/*
 * 读命令数据
 */
bool pmbusm_read_cmd_data(uchar addr,uchar cmd,void *data,uchar size);

/*
 * PMBus任务
 */
void pmbusm_task();

#endif /* PMBUSM_H */
