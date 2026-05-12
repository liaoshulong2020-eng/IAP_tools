/*
 * iwdg.c
 *
 *  Created on: 2024年4月26日
 *      Author: Liang Jinfeng
 */

#include "iwdg.h"

/*
 * 初始化
 */
void iwdg_init(ushort ms)
{
    IWDG_InitTypeDef init={0};

    //When the program runs to here, it means that the system has been reset last time
    if(__LL_RCU_IsIWDGRst(RCU))
	{
        //IWDG Reset Pending Clear
        __LL_RCU_RegWrite_Unlock(RCU);
        __LL_RCU_IWDGRst_Clr(RCU);
        __LL_RCU_RegWrite_Lock(RCU);
    }

    //IWDG RCU Reset Enable
    __LL_RCU_RegWrite_Unlock(RCU);
    __LL_RCU_IWDG_Rst_En(RCU);
    __LL_RCU_RegWrite_Lock(RCU);

    init.mode=IWDG_MODE_RESET;
    init.pre_div=IWDG_PRE_DIV_32;
    init.reload_val=ms;
    LL_IWDG_Init(IWDG,&init);

    //启动看门狗定时器
    LL_IWDG_Start(IWDG);
}

/*
 * 喂狗
 */
void iwdg_feed()
{
	LL_IWDG_Refresh(IWDG);
}
