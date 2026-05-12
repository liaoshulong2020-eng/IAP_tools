/**
  ******************************************************************************
  * @file    user_cordic.c
  * @author  MCD Application Team
  * @brief   This file provides the CORDIC User Config Method.
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
#include "tae32g58xx_ll.h"
#include <string.h>
#include "main.h"


#define DBG_TAG     "User CORDIC"
#define DBG_LVL     DBG_LOG
#include "dbg/tae_dbg.h"


/** @addtogroup TAE32G58xx_Examples
  * @{
  */

/** @addtogroup TAE32G58xx_CORDIC_Base_Example
  * @{
  */


/* Private Constants ---------------------------------------------------------*/
/** @defgroup User_CORDIC_Base_Private_Constants User CORDIC_Base Private Constants
  * @brief    User CORDIC_Base Private Constants
  * @{
  */

/**
  * @brief User CORDIC test Calculate mode definition
  */
#define USER_CORDIC_CALC_ONE_BY_ONE_16              (0)     /*!< One by one 16bit                   */
#define USER_CORDIC_CALC_ONE_BY_ONE_32              (1)     /*!< One by one 32bit                   */
#define USER_CORDIC_CALC_MULTI_ONETIME_SINGLE_CH    (2)     /*!< Multi one-time with single channel */
#define USER_CORDIC_CALC_MULTI_ONETIME_MIXED_CH     (3)     /*!< Multi one-time with mixed channel  */

#define USER_CORDIC_CALC_MODE                       USER_CORDIC_CALC_MULTI_ONETIME_MIXED_CH

/**
  * @brief User CORDIC test Channel macro definition
  */
#define USER_CORDIC_TEST_CHANNEL_NUM                (CORDIC_CHANNEL_0)

/**
  * @brief User CORDIC test Default Timeout macro definition
  */
#define USER_CORDIC_TEST_TIMEOUT_DEF                (300)

/**
  * @}
  */


/* Private Macros ------------------------------------------------------------*/
/* Private Types -------------------------------------------------------------*/
/* Private Variables ---------------------------------------------------------*/
/** @defgroup User_CORDIC_Base_Private_Variables User CORDIC_Base Private Variables
  * @brief    User CORDIC_Base Private Variables
  * @{
  */

/**
  * @brief User CORDIC Test Input Data
  */
static const int32_t user_cordic_input_dat[] = {
    0xffffb546, 0xffffc086, 0xffffb56f, 0xffffe921, 0x000065ef, 0xffff8b41, 0x0000208c, 0xfffff355, 0xffffd6d6, 0x000020a5,
};

/**
  * @brief User CORDIC Test Output Data
  */
static const int32_t user_cordic_output_dat[] = {
    0x8469deb1, 0x800301a5, 0x8448df2d, 0xbbdd6c5b, 0x4c6b9950, 0xdd1884da, 0x5bb75949, 0xd8d779dd, 0x9395440b, 0x5bee5911,
};

/**
  * @}
  */

int32_t out_buf[1],cordic_output[1];
int32_t cordic_input[1]={100};

/**
  * @}
  */
/**
  * @brief  User CORDIC Init
  * @param  CORDIC Specifies CORDIC peripheral
  * @return None
  */
void cordic_init_app()
{
    //CORDIC LL Init
    LL_CORDIC_Init(CORDIC);
		cordic_config_app();
}
void cordic_config_app()
{
    int ret;
    uint32_t i = 0;
    CORDIC_UserCfgTypeDef cordic_user_cfg;

    LOG_D("/*** CORDIC_Base test Start ***/\n\n");

    //User CORDIC Config
    cordic_user_cfg.arg_width = CORDIC_DAT_WIDTH_16bit;
    cordic_user_cfg.res_width = CORDIC_DAT_WIDTH_16bit;
    cordic_user_cfg.arg_num = CORDIC_DAT_NUM_TWO_16bit;
    cordic_user_cfg.res_num = CORDIC_DAT_NUM_TWO_16bit;
    cordic_user_cfg.func  = CORDIC_FUNC_SQRT;
    cordic_user_cfg.scale = 0;

		LL_CORDIC_Config(CORDIC, CORDIC_CHANNEL_0, &cordic_user_cfg);

}
/**
  * @}
  */

void cordic_calculate()
{
    LL_CORDIC_Calculate_MixCh_ZO(CORDIC, (int32_t *)cordic_input, out_buf, 1,
                                 USER_CORDIC_TEST_TIMEOUT_DEF);
		LL_CORDIC_Reset(CORDIC,  CORDIC_CHANNEL_0 );
	
	
		
		cordic_output[1] = CORDIC->ARX0;
}
/**
  * @}
  */

/**
  * @}
  */


/************************* (C) COPYRIGHT Tai-Action *****END OF FILE***********/

