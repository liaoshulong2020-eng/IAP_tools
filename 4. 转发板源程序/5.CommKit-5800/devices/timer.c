/*
 * timer.c
 *
 *  Created on: 2024年1月7日
 *      Author: Liang Jinfeng
 */

#include "timer.h"
#include "gpio.h"
#include "sys_mgr.h"

/**
  * @brief  TMR Overflow Interrupt Callback Function
  * @param  Instance Specifies TMR peripheral
  * @return None
  */
void LL_TMR_OverflowCallback(TMR_TypeDef *Instance)
{
	sys_timer_isr();
}

/*
 * 初始化
 * 20us(50KHz)定时器
 */
void timer8_init()
{
	ulong apb0clk;
	TMR_BaseInitTypeDef tmr_base_init;

	memset((void *)&tmr_base_init,0x00,sizeof(tmr_base_init));

	apb0clk=LL_RCU_APB0ClkGet();

	LL_TMR_Init(TMR8);

	tmr_base_init.auto_preload_en = true;
	tmr_base_init.work_mode = TMR_WORK_MODE_CONTINUE;
	tmr_base_init.period = apb0clk/50000;
	tmr_base_init.prescaler = 0;
	tmr_base_init.update_evt_en = true;
	tmr_base_init.update_evt_src = TMR_UPDATE_EVT_SRC_OV;

	LL_TMR_Base_Cfg(TMR8,&tmr_base_init);
	LL_TMR_Start_IT(TMR8);
}
