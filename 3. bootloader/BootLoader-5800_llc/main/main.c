/*
 * main.c - LLC Bootloader主程序（含PFC透传支持）
 *
 * LLC通过CAN接收上位机IAP命令：
 * - 升级自己时，直接处理
 * - 升级PFC时，通过UART0透传给PFC
 *
 *  Created on: 2024年3月20日
 *      Author: Liang Jinfeng
 *  Modified: 2025 - 增加PFC透传支持
 */

#include "main.h"
#include "sys_mgr.h"
#include "delay.h"
#include "uart.h"
#include "vars.h"
#include "iwdg.h"
#include "modbus_iap.h"
#include "iap.h"
#include "cans.h"

static void SystemClock_Config(void);

/*
 * 主函数
 */
int main(void)
{
    SystemClock_Config();
    LL_Init();

    delayms(20);
    sys_init();

    while(1)
    {
    	modbus_iap_task();
    	iap_task();
		sys_task();

		//CAN接收轮询
		cans_rx_poll();
    }
}

/**
  * @brief  SYSCLK Config
  * @param  None
  * @return None
  */
static void SystemClock_Config(void)
{
    LL_StatusETypeDef ret;
    RCU_PLLUserCfgTypeDef    pll1_cfg;
    RCU_SysclkUserCfgTypeDef sysclk_cfg;

    //SYSCLK Clock Config
    sysclk_cfg.sysclk_src  = SYSCLK_SRC_PLL0DivClk;
    sysclk_cfg.sysclk_freq = Fcpu;
    sysclk_cfg.pll0clk_src = PLLCLK_SRC_HSI;
    sysclk_cfg.pll0clk_src_freq = HSI_VALUE;
    sysclk_cfg.apb0_clk_div = RCU_CLK_DIV_2;
    sysclk_cfg.apb1_clk_div = RCU_CLK_DIV_2;
    sysclk_cfg.ahb_clk_div  = RCU_CLK_DIV_1;
    ret = LL_RCU_SysclkInit(RCU, &sysclk_cfg);

    if (ret == LL_OK) {
        SystemCoreClockUpdate(sysclk_cfg.sysclk_freq);
    }

    //PLL1 Config
    pll1_cfg.pll_clk_src = PLLCLK_SRC_HSI;
    pll1_cfg.pll_in_freq = HSI_VALUE;
    pll1_cfg.pll_user_freq = 360000000UL;
    ret = LL_RCU_Pll1Cfg(RCU, &pll1_cfg);

    if (ret == LL_OK) {
        LL_RCU_HRPWM_ClkCfg(RCU_CLK_SRC_PLL1, RCU_CLK_DIV_2);
        LL_RCU_ADC_ClkCfg(RCU_CLK_SRC_PLL1, RCU_CLK_DIV_6);
		LL_RCU_CAN_ClkCfg(RCU_CLK_SRC_PLL1, RCU_CLK_DIV_3);
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
    while(1)
    {
        __NOP();
    }
}
#endif
