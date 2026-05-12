/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @brief   main source file for template project
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 Tai-Action.
  * All rights reserved.</center></h2>
  *
  * This software is licensed by Tai-Action under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"


#define DBG_TAG     "APP"
#define DBG_LVL     DBG_LOG
#include "dbg/tae_dbg.h"


/** @defgroup TAE32G58xx_Examples TAE32G58xx Examples
  * @brief    TAE32G58xx Examples
  * @{
  */

/** @defgroup TAE32G58xx_Template TAE32G58xx Template
  * @brief    TAE32G58xx Template
  * @{
  */


/* Private Constants ---------------------------------------------------------*/
/* Private Macros ------------------------------------------------------------*/
/* Private Types -------------------------------------------------------------*/
/* Private Variables ---------------------------------------------------------*/
/* Private Function Prototypes -----------------------------------------------*/
/** @addtogroup MAIN_Private_Functions MAIN Private Functions
  * @{
  */
static void SystemClock_Config(void);
/**
  * @}
  */


/* Exported Variables --------------------------------------------------------*/
/* Exported Functions --------------------------------------------------------*/
/* Private Functions ---------------------------------------------------------*/
/** @defgroup MAIN_Private_Functions MAIN Private Functions
  * @brief    MAIN Private Functions
  * @{
  */

/**
  * @brief  main entry
  * @param  None
  * @return status
  */
int main(void)
{
    SystemClock_Config();
    LL_Init();
	for(int cossin_cnt=0; cossin_cnt<LEN_COSSIN_BUF; cossin_cnt++) {
		cosdata[cossin_cnt] = 4096*cos(0.0030679615*cossin_cnt);	// ²½³¤ = 0.0030679615 = 2pi / 2048 
		sindata[cossin_cnt] = 4096*sin(0.0030679615*cossin_cnt);
	}
    LOG_D("App start\n");
		init_all_app();
		Reset_value();
    while (1) {
			main_loop();
			for(int i=0;i<LEN_COSSIN_BUF;i++)
			{
//				cordic_input[1] = 40000;
//				cordic_calculate();
			}
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
    sysclk_cfg.sysclk_freq = 180000000UL;
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
        LL_RCU_ADC_ClkCfg(RCU_CLK_SRC_PLL1, RCU_CLK_DIV_6);
        LL_RCU_HRPWM_ClkCfg(RCU_CLK_SRC_PLL1, RCU_CLK_DIV_2);
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
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    if (file != NULL) {
        LOG_E("Assert failed in %s line %d", file, line);
    }

    /* Infinite loop */
    while (1) {
        __NOP();
    }
}
#endif

/**
  * @}
  */


/**
  * @}
  */

/**
  * @}
  */


/************************* (C) COPYRIGHT Tai-Action *****END OF FILE***********/

