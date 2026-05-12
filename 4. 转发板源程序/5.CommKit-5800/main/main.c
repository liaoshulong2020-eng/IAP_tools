/*
 * main.c
 *
 *  Created on: 2024年3月20日
 *      Author: Liang Jinfeng
 */

#include "main.h"
#include "sys_mgr.h"
#include "delay.h"
#include "uart.h"
#include "iwdg.h"
#include "modbus_var.h"
#include "iap_transfer.h"
#include "canm.h"

static void SystemClock_Config(void);

/*
 * Flash读保护
 */
//static void flash_protect()
//{
//	LL_EFLASH_ReadProtLvlCfg(EFLASH,EFLASH_READ_PROT_LVL_1);
//}

/*
 * 主函数
 */
int main(void)
{
    SystemClock_Config();
    LL_Init();

//    flash_protect();
	__enable_irq();
    sys_init();

    while(1)
    {
		sys_task();
		modbus_var_task();
		iap_transfer_task();
		uart0_tx_poll();
		canm_rx_poll();
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
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
//    LOG_E("Assert failed in %s line %d", file, line);

    /* Infinite loop */
    while(1)
    {
        __NOP();
    }
}
#endif
