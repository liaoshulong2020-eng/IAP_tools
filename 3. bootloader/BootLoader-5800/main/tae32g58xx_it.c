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
//test
#include "gpio.h"

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
  * @brief  This function handles GPIOA interrupt request.
  * @param  None
  * @retval None
  */
void GPIOA_IRQHandler(void)
{
#ifdef LL_GPIO_MODULE_ENABLED
    LL_GPIO_IRQHandler(GPIOA);
#endif
}

/**
  * @brief  This function handles GPIOB interrupt request.
  * @param  None
  * @retval None
  */
void GPIOB_IRQHandler(void)
{
#ifdef LL_GPIO_MODULE_ENABLED
    LL_GPIO_IRQHandler(GPIOB);
#endif
}

/**
  * @brief  This function handles GPIOC interrupt request.
  * @param  None
  * @retval None
  */
void GPIOC_IRQHandler(void)
{
#ifdef LL_GPIO_MODULE_ENABLED
    LL_GPIO_IRQHandler(GPIOC);
#endif
}

/**
  * @brief  This function handles GPIOD interrupt request.
  * @param  None
  * @retval None
  */
void GPIOD_IRQHandler(void)
{
#ifdef LL_GPIO_MODULE_ENABLED
    LL_GPIO_IRQHandler(GPIOD);
#endif
}

/**
  * @brief  This function handles GPIOE interrupt request.
  * @param  None
  * @retval None
  */
void GPIOE_IRQHandler(void)
{
#ifdef LL_GPIO_MODULE_ENABLED
    LL_GPIO_IRQHandler(GPIOE);
#endif
}

/**
  * @brief  This function handles GPIOF interrupt request.
  * @param  None
  * @retval None
  */
void GPIOF_IRQHandler(void)
{
#ifdef LL_GPIO_MODULE_ENABLED
    LL_GPIO_IRQHandler(GPIOF);
#endif
}

/**
  * @brief  This function handles UART0 Handler.
  * @param  None
  * @return None
  */
void UART0_IRQHandler(void)
{
#ifdef LL_UART_MODULE_ENABLED
    LL_UART_IRQHandler(UART0);
#endif
}

/**
  * @brief  This function handles UART1 Handler.
  * @param  None
  * @return None
  */
void UART1_IRQHandler(void)
{
#ifdef LL_UART_MODULE_ENABLED
    LL_UART_IRQHandler(UART1);
#endif
}

/**
  * @brief  This function handles UART2 Handler.
  * @param  None
  * @return None
  */
void UART2_IRQHandler(void)
{
#ifdef LL_UART_MODULE_ENABLED
    LL_UART_IRQHandler(UART2);
#endif
}

/**
  * @brief  This function handles UART3 Handler.
  * @param  None
  * @return None
  */
void UART3_IRQHandler(void)
{
#ifdef LL_UART_MODULE_ENABLED
    LL_UART_IRQHandler(UART3);
#endif
}

/**
  * @brief  This function handles UART4 Handler.
  * @param  None
  * @return None
  */
void UART4_IRQHandler(void)
{
#ifdef LL_UART_MODULE_ENABLED
    LL_UART_IRQHandler(UART4);
#endif
}

/**
  * @brief  This function handles DMA channel 0 interrupt request.
  * @param  None
  * @retval None
  */
void DMA_CH0_IRQHandler(void)
{
#ifdef LL_DMA_MODULE_ENABLED
    LL_DMA_IRQHandler(DMA, DMA_CHANNEL_0);
#endif
}

/**
  * @brief  This function handles DMA channel 1 interrupt request.
  * @param  None
  * @retval None
  */
void DMA_CH1_IRQHandler(void)
{
#ifdef LL_DMA_MODULE_ENABLED
    LL_DMA_IRQHandler(DMA, DMA_CHANNEL_1);
#endif
}

/**
  * @brief  This function handles DMA channel 2 interrupt request.
  * @param  None
  * @retval None
  */
void DMA_CH2_IRQHandler(void)
{
#ifdef LL_DMA_MODULE_ENABLED
    LL_DMA_IRQHandler(DMA, DMA_CHANNEL_2);
#endif
}

/**
  * @brief  This function handles DMA channel 3 interrupt request.
  * @param  None
  * @retval None
  */
void DMA_CH3_IRQHandler(void)
{
#ifdef LL_DMA_MODULE_ENABLED
    LL_DMA_IRQHandler(DMA, DMA_CHANNEL_3);
#endif
}

/**
  * @brief  This function handles DMA channel 4 interrupt request.
  * @param  None
  * @retval None
  */
void DMA_CH4_IRQHandler(void)
{
#ifdef LL_DMA_MODULE_ENABLED
    LL_DMA_IRQHandler(DMA, DMA_CHANNEL_4);
#endif
}

/**
  * @brief  This function handles DMA channel 5 interrupt request.
  * @param  None
  * @retval None
  */
void DMA_CH5_IRQHandler(void)
{
#ifdef LL_DMA_MODULE_ENABLED
    LL_DMA_IRQHandler(DMA, DMA_CHANNEL_5);
#endif
}

/**
  * @brief  This function handles I2C0 interrupt request.
  * @param  None
  * @retval None
  */
void I2C0_IRQHandler(void)
{
#ifdef LL_I2C_MODULE_ENABLED
    LL_I2C_IRQHandler(I2C0);
#endif
}

/**
  * @brief  This function handles I2C1 interrupt request.
  * @param  None
  * @retval None
  */
void I2C1_IRQHandler(void)
{
#ifdef LL_I2C_MODULE_ENABLED
    LL_I2C_IRQHandler(I2C1);
#endif
}

/**
  * @brief  This function handles I2C2 interrupt request.
  * @param  None
  * @retval None
  */
void I2C2_IRQHandler(void)
{
#ifdef LL_I2C_MODULE_ENABLED
    LL_I2C_IRQHandler(I2C2);
#endif
}

/**
  * @}
  */
void ADC1_NORM_IRQHandler(void)
{
	LL_ADC_Norm_IRQHandler(ADC1);
}
/**
  * @}
  */

/**
  * @brief  This function handles TMR0 interrupt request.
  * @param  None
  * @retval None
  */
void TMR0_IRQHandler(void)
{
#ifdef LL_TMR_MODULE_ENABLED
    LL_TMR_IRQHandler(TMR0);
#endif
}

/**
  * @brief  This function handles TMR1 interrupt request.
  * @param  None
  * @retval None
  */
void TMR1_IRQHandler(void)
{
#ifdef LL_TMR_MODULE_ENABLED
    LL_TMR_IRQHandler(TMR1);
#endif
}

/**
  * @brief  This function handles TMR2 interrupt request.
  * @param  None
  * @retval None
  */
void TMR2_IRQHandler(void)
{
#ifdef LL_TMR_MODULE_ENABLED
    LL_TMR_IRQHandler(TMR2);
#endif
}

/**
  * @brief  This function handles TMR3 interrupt request.
  * @param  None
  * @retval None
  */
void TMR3_IRQHandler(void)
{
#ifdef LL_TMR_MODULE_ENABLED
    LL_TMR_IRQHandler(TMR3);
#endif
}

/**
  * @brief  This function handles TMR4 interrupt request.
  * @param  None
  * @retval None
  */
void TMR4_IRQHandler(void)
{
#ifdef LL_TMR_MODULE_ENABLED
    LL_TMR_IRQHandler(TMR4);
#endif
}

/**
  * @brief  This function handles TMR7 interrupt request.
  * @param  None
  * @retval None
  */
void TMR7_IRQHandler(void)
{
#ifdef LL_TMR_MODULE_ENABLED
    LL_TMR_IRQHandler(TMR7);
#endif
}

/**
  * @brief  This function handles TMR8 interrupt request.
  * @param  None
  * @retval None
  */
void TMR8_IRQHandler(void)
{
#ifdef LL_TMR_MODULE_ENABLED
    LL_TMR_IRQHandler(TMR8);
#endif
}

/**
  * @brief  This function handles TMR9 Break interrupt request.
  * @param  None
  * @retval None
  */
void TMR9_BRK_IRQHandler(void)
{
#ifdef LL_TMR_MODULE_ENABLED
    LL_TMR_Brk_IRQHandler(TMR9);
#endif
}

/**
  * @brief  This function handles TMR9 Update interrupt request.
  * @param  None
  * @retval None
  */
void TMR9_UPD_IRQHandler(void)
{
#ifdef LL_TMR_MODULE_ENABLED
    LL_TMR_Upd_IRQHandler(TMR9);
#endif
}

/**
  * @brief  This function handles TMR9 Trigger interrupt request.
  * @param  None
  * @retval None
  */
void TMR9_TRG_IRQHandler(void)
{
#ifdef LL_TMR_MODULE_ENABLED
    LL_TMR_Trg_IRQHandler(TMR9);
#endif
}

/**
  * @brief  This function handles TMR9 Capture/Compare interrupt request.
  * @param  None
  * @retval None
  */
void TMR9_CC_IRQHandler(void)
{
#ifdef LL_TMR_MODULE_ENABLED
    LL_TMR_CC_IRQHandler(TMR9);
#endif
}

/**
  * @brief  This function handles TMR10 Break interrupt request.
  * @param  None
  * @retval None
  */
void TMR10_BRK_IRQHandler(void)
{
#ifdef LL_TMR_MODULE_ENABLED
    LL_TMR_Brk_IRQHandler(TMR10);
#endif
}

/**
  * @brief  This function handles TMR10 Update interrupt request.
  * @param  None
  * @retval None
  */
void TMR10_UPD_IRQHandler(void)
{
#ifdef LL_TMR_MODULE_ENABLED
    LL_TMR_Upd_IRQHandler(TMR10);
#endif
}

/**
  * @brief  This function handles TMR10 Trigger interrupt request.
  * @param  None
  * @retval None
  */
void TMR10_TRG_IRQHandler(void)
{
#ifdef LL_TMR_MODULE_ENABLED
    LL_TMR_Trg_IRQHandler(TMR10);
#endif
}

/**
  * @brief  This function handles TMR10 Capture/Compare interrupt request.
  * @param  None
  * @retval None
  */
void TMR10_CC_IRQHandler(void)
{
#ifdef LL_TMR_MODULE_ENABLED
    LL_TMR_CC_IRQHandler(TMR10);
#endif
}

/* Private Functions ---------------------------------------------------------*/

void HRPWM_COMM_IRQHandler(void)
{
	LL_HRPWM_Comm_IRQHandler(HRPWM);
}
/**
  * @}
  */

/**
  * @}
  */


/************************* (C) COPYRIGHT Tai-Action *****END OF FILE***********/

