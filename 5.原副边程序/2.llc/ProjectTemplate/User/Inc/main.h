/**
  ******************************************************************************
  * @file    main.h
  * @author  MCD Application Team
  * @brief   Header for main.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "tae32g58xx_ll.h"
#include "dbg/user_debug.h"
#include <stdio.h>
#include "adc_app.h"
#include "hrpwm_app.h"
#include "dac_app.h"
#include "gpio_app.h"
#include "tmr_app.h"
#include "init_app.h"
#include "vofa_app.h"
#include "can_app.h"
#include "protection_app.h"
#include "communication_app.h"
#include "llc_app.h"
#include "pid_app.h"
#include "variables_define_app.h"
#include "pr_control_app.h"
#include "save_data.h"
#include "uart_app.h"
#include "pri_sec_commun.h"

#define RAMCODE __SECTION(RAMCODE)
extern float abs_f(float num) ;
/** @addtogroup TAE32G58xx_Examples
  * @{
  */

/** @addtogroup TAE32G58xx_Template
  * @{
  */


/* Exported Constants --------------------------------------------------------*/
/* Exported Macros -----------------------------------------------------------*/
/* Exported Types ------------------------------------------------------------*/
/* Exported Variables --------------------------------------------------------*/
/* Exported Functions --------------------------------------------------------*/


/**
  * @}
  */

/**
  * @}
  */


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* _MAIN_H_ */


/************************* (C) COPYRIGHT Tai-Action *****END OF FILE***********/

