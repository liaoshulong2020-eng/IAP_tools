/*
 * delay.c
 *
 *  Created on: 2024年4月26日
 *      Author: Liang Jinfeng
 */

#include "delay.h"
#include "iwdg.h"

void delayms(ulong time)
{
	volatile ulong i,j;

	for(i=0;i<time;i++)
	{
		for(j=0;j<15000;j++);
		iwdg_feed();
	}
}
