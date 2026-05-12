/*
 * sys_mgr.h
 *
 *  Created on: 2024年1月8日
 *      Author: Liang Jinfeng
 */

#ifndef SYS_MGR_H
#define SYS_MGR_H

#include "main.h"

/*
 * 初始化
 */
void sys_init();

/*
 * 定时器回调函数
 * 时基20us
 */
void sys_timer_isr();

/*
 * 系统管理任务
 */
void sys_task();

#endif /* SYS_MGR_H */
