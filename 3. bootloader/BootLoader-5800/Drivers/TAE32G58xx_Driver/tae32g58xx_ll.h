/**
  ******************************************************************************
  * @file    tae32g58xx_ll.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the LL
  *          module driver.
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
#ifndef _TAE32G58XX_LL_H_
#define _TAE32G58XX_LL_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** @addtogroup TAE32G58xx_LL_Driver
  * @{
  */

/** @addtogroup TAE32G58xx_LL
  * @{
  */


/* Exported Constants --------------------------------------------------------*/
/** @defgroup TAE32G58xx_LL_Exported_Constants TAE32G58xx LL Exported Constants
  * @brief    TAE32G58xx LL Exported Constants
  * @{
  */

/**
  * @brief TAE32G58xx LL Driver version number V1.2.3
  */
#define __TAE32G58xx_LL_VERSION_MAIN    (0x01U)     /*!< [31:24] main version       */
#define __TAE32G58xx_LL_VERSION_SUB1    (0x02U)     /*!< [23:16] sub1 version       */
#define __TAE32G58xx_LL_VERSION_SUB2    (0x03U)     /*!< [15:8]  sub2 version       */
#define __TAE32G58xx_LL_VERSION_RC      (0x00U)     /*!< [7:0]  release candidate   */
#define __TAE32G58xx_LL_VERSION         ((__TAE32G58xx_LL_VERSION_MAIN << 24) |\
                                         (__TAE32G58xx_LL_VERSION_SUB1 << 16) |\
                                         (__TAE32G58xx_LL_VERSION_SUB2 << 8 ) |\
                                         (__TAE32G58xx_LL_VERSION_RC))

/**
  * @brief TAE32G58xx SDK Stage String definition
  * @note  Value range: "Alpha" "Beta" "RC" "Trial" "Release"
  */
#define SDK_STAGE_STR                   "Release"


/**
  * @brief Internal High Speed oscillator (HSI) value.
  *        This value is used by the PLL module to compute the system frequency
  *        (when HSI is used as system clock source, directly or through the PLL).
  */
#define HSI_VALUE                       (__LL_SYSCTRL_RC8M_Real_Get(SYSCTRL) == 0xffffffUL ? 8000000U : __LL_SYSCTRL_RC8M_Real_Get(SYSCTRL))

/**
  * @brief Internal Low Speed oscillator (LSI) value.
  *        Defines the value of the Internal Low Speed oscillator in Hz.
  * @note  The real value may vary depending on the variations in voltage and temperature.
  */
#define LSI_VALUE                       (__LL_SYSCTRL_RC32K_Real_Get(SYSCTRL) == 0xffffUL ? 32000U : __LL_SYSCTRL_RC32K_Real_Get(SYSCTRL))


/**
  * @}
  */


/* Includes ------------------------------------------------------------------*/
#include "tae32g58xx_ll_conf.h"


/* Exported Types ------------------------------------------------------------*/
/** @defgroup TAE32G58xx_LL_Exported_Types TAE32G58xx LL Exported Types
  * @brief    TAE32G58xx LL Exported Types
  * @{
  */

/**
  * LL Tick Freq Enum Type Definition
  */
typedef enum {
    LL_TICK_FREQ_10HZ    = 100U,                /*!< Tick Frequency 10Hz    */
    LL_TICK_FREQ_100HZ   = 10U,                 /*!< Tick Frequency 100Hz   */
    LL_TICK_FREQ_1KHZ    = 1U,                  /*!< Tick Frequency 1KHz    */
    LL_TICK_FREQ_DEFAULT = LL_TICK_FREQ_1KHZ,   /*!< Tick Frequency default */
} LL_TickFreqETypeDef;

/**
  * @}
  */


/* Exported Macros -----------------------------------------------------------*/
/* Exported Variables --------------------------------------------------------*/
/* Exported Functions --------------------------------------------------------*/
/** @addtogroup TAE32G58xx_LL_Exported_Functions
  * @{
  */

/** @addtogroup TAE32G58xx_LL_Exported_Functions_Group1
  * @{
  */
LL_StatusETypeDef LL_Init(void);
LL_StatusETypeDef LL_DeInit(void);
void LL_MspInit(void);
void LL_MspDeInit(void);

LL_StatusETypeDef LL_InitTick(uint32_t TickPriority);
/**
  * @}
  */


/** @addtogroup TAE32G58xx_LL_Exported_Functions_Group2
  * @{
  */
void LL_IncTick(void);
uint32_t LL_GetTick(void);
uint32_t LL_GetTickPrio(void);
LL_StatusETypeDef LL_SetTickFreq(LL_TickFreqETypeDef Freq);
LL_TickFreqETypeDef LL_GetTickFreq(void);
void LL_SuspendTick(void);
void LL_ResumeTick(void);
void LL_Delay(uint32_t Delay);
uint32_t LL_GetHalVersion(void);
LL_StatusETypeDef LL_GetUID(uint32_t UID[]);
/**
  * @}
  */


/** @addtogroup TAE32G58xx_LL_Exported_Functions_Group3
  * @{
  */
void LL_ShowInfo(void);
void delay_ms(uint32_t ms);
void printf_array(void *ptr, uint32_t len);
/**
  * @}
  */

/**
  * @}
  */


/**
  * @}
  */

/**
  * @}
  */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _TAE32G58XX_LL_H_ */


/************************* (C) COPYRIGHT Tai-Action *****END OF FILE***********/

