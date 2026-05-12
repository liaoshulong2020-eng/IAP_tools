/*
 * delay.c
 *
 *  Created on: 2023Äê12ÔÂ2ÈÕ
 *      Author: Liang Jinfeng
 */

#include "delay.h"
#include "iwdg.h"

void delayms(ulong time)
{
	volatile ulong i,j;

	for(i=0;i<time;i++)
	{
		for(j=0;j<20000;j++);
		iwdg_feed();
	}
}
