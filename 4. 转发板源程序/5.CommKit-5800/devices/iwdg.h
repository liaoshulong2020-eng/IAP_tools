/*
 * iwdg.h
 *
 *  Created on: 2024年4月26日
 *      Author: Liang Jinfeng
 */

#ifndef IWDG_H
#define IWDG_H

#include "main.h"

/*
 * 初始化
 */
void iwdg_init(ushort ms);

/*
 * 喂狗
 */
void iwdg_feed();

#endif /* IWDG_H */
