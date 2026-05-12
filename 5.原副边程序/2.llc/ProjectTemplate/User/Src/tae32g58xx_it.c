/**
  ******************************************************************************
  * @file    tae32g58xx_it.c
  * @author  MCD Application Team
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
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
#include "tae32g58xx_it.h"


/** @addtogroup TAE32G58xx_Examples
  * @{
  */

/** @addtogroup TAE32G58xx_Template
  * @{
  */


/* Private Constants ---------------------------------------------------------*/
/* Private Macros ------------------------------------------------------------*/
/* Private Types -------------------------------------------------------------*/
/* Private Variables ---------------------------------------------------------*/
/* Private Function Prototypes -----------------------------------------------*/
/* Exported Variables --------------------------------------------------------*/
/* Exported Functions --------------------------------------------------------*/
/** @defgroup TAE32G58xx_IT_Exported_Functions TAE32G58xx IT Exported Functions
  * @brief    TAE32G58xx IT Exported Functions
  * @{
  */

/** @defgroup TAE32G58xx_IT_Exported_Functions_Group1 Cortex-M4 Processor Exceptions Handlers
  * @brief    Cortex-M4 Processor Exceptions Handlers
  * @{
  */

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1) {
    }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1) {
    }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1) {
    }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1) {
    }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
#ifdef LL_MODULE_ENABLED
    LL_IncTick();
#endif

#ifdef LL_CORTEX_MODULE_ENABLED
    LL_SYSTICK_IRQHandler();
#endif
}

/**
  * @}
  */


/******************************************************************************/
/*                  TAE32G58xx Peripherals Interrupt Handlers                 */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_tae32g58xx.c).                                              */
/******************************************************************************/

/** @defgroup TAE32G58xx_IT_Exported_Functions_Group2 XXX Peripherals Interrupt Handlers
  * @brief    XXX Peripherals Interrupt Handlers
  * @{
  */

/**
  * @brief  This function handles XXX interrupt request.
  * @param  None
  * @retval None
  */
/*void XXX_IRQHandler(void)
{
}*/

/**
  * @}
  */

/**
  * @}
  */


/* Private Functions ---------------------------------------------------------*/
//void HRPWM_COMM_IRQHandler(void)
//{
//	LL_HRPWM_Comm_IRQHandler(HRPWM);
//}


/**
  * @}
  */
void CAN1_IRQHandler(void)
{
	LL_CAN_IRQHandler(CAN1);
}
/**
  * @}
  */

void UART0_IRQHandler(void)
{
	LL_UART_IRQHandler(UART0);
}

/************************* (C) COPYRIGHT Tai-Action *****END OF FILE***********/

