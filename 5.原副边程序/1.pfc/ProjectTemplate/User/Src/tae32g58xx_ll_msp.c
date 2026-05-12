/**
  ******************************************************************************
  * @file    tae32g58xx_ll_msp.c
  * @author  MCD Application Team
  * @brief   LL MSP module.
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

#define DBG_TAG             "MSP LL"
#define DBG_LVL             DBG_ERROR
#include "dbg/tae_dbg.h"

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
/* Private Functions ---------------------------------------------------------*/
/** @defgroup MSP_LL_Private_Functions MSP LL Private Functions
  * @brief    MSP LL Private Functions
  * @{
  */

/**
  * @brief  Initializes the Global MSP
  * @param  None
  * @retval None
  */
void LL_MspInit(void)
{
#ifdef LL_GPIO_MODULE_ENABLED
  //GPIO Msp Init
  LL_RCU_GPIOA_ClkEnRstRelease();
  LL_RCU_GPIOB_ClkEnRstRelease();
  LL_RCU_GPIOC_ClkEnRstRelease();
  LL_RCU_GPIOD_ClkEnRstRelease();
  LL_RCU_GPIOE_ClkEnRstRelease();
  LL_RCU_GPIOF_ClkEnRstRelease();
#endif

#ifdef LL_DMA_MODULE_ENABLED
  //DMA Msp Init
  LL_RCU_DMA_ClkEnRstRelease();

  //DMA Channel Interrupt Enable
//    LL_NVIC_EnableIRQ(DMA_CH0_IRQn);
//    LL_NVIC_EnableIRQ(DMA_CH1_IRQn);
//    LL_NVIC_EnableIRQ(DMA_CH2_IRQn);
//    LL_NVIC_EnableIRQ(DMA_CH3_IRQn);
//    LL_NVIC_EnableIRQ(DMA_CH4_IRQn);
//    LL_NVIC_EnableIRQ(DMA_CH5_IRQn);
#endif

#ifdef LL_EFLASH_MODULE_ENABLED
  //EFLASH Msp Init
  LL_RCU_EFLASH_ClkEnRstRelease();
#endif
}

/**
  * @brief  DeInitializes the Global MSP
  * @param  None
  * @retval None
  */
void LL_MspDeInit(void)
{
#ifdef LL_GPIO_MODULE_ENABLED
  //GPIO Msp DeInit
  LL_RCU_GPIOA_ClkDisRstAssert();
  LL_RCU_GPIOB_ClkDisRstAssert();
  LL_RCU_GPIOC_ClkDisRstAssert();
  LL_RCU_GPIOD_ClkDisRstAssert();
  LL_RCU_GPIOE_ClkDisRstAssert();
  LL_RCU_GPIOF_ClkDisRstAssert();
#endif

#ifdef LL_DMA_MODULE_ENABLED
  //DMA Msp DeInit
  LL_RCU_DMA_ClkDisRstAssert();

  //DMA Channel Interrupt Disable
  LL_NVIC_DisableIRQ(DMA_CH0_IRQn);
  LL_NVIC_DisableIRQ(DMA_CH1_IRQn);
  LL_NVIC_DisableIRQ(DMA_CH2_IRQn);
  LL_NVIC_DisableIRQ(DMA_CH3_IRQn);
  LL_NVIC_DisableIRQ(DMA_CH4_IRQn);
  LL_NVIC_DisableIRQ(DMA_CH5_IRQn);
#endif

#ifdef LL_EFLASH_MODULE_ENABLED
  //EFLASH Msp DeInit
  LL_RCU_EFLASH_ClkDisRstAssert();
#endif
}

/**
  * @brief  Initializes the UART MSP
  * @param  Instance Specifies UART peripheral
  * @retval None
  */
void LL_UART_MspInit(UART_TypeDef* Instance)
{
  GPIO_InitTypeDef UART_GPIO_Init;

  //Assert param
  assert_param(IS_UART_ALL_INSTANCE(Instance));

  //UART GPIO Common Config
  UART_GPIO_Init.IntMode = GPIO_INT_MODE_CLOSE;
  UART_GPIO_Init.OType   = GPIO_OTYPE_PP;
  UART_GPIO_Init.Pull    = GPIO_NOPULL;
  UART_GPIO_Init.Speed   = GPIO_SPEED_FREQ_LOW;

  if(Instance == UART0)
    {
      //UART0 Pinmux Config: PA9
      UART_GPIO_Init.Pin = GPIO_PIN_9|GPIO_PIN_10;
      UART_GPIO_Init.Alternate = GPIO_AF8_UART0;
      LL_GPIO_Init(GPIOA, &UART_GPIO_Init);

      //UART0 Bus Clock Enable and Soft Reset Release
      LL_RCU_UART0_ClkEnRstRelease();

      //NVIC UART0 Interrupt Enable
      LL_NVIC_EnableIRQ(UART0_IRQn);

    }
  else if(Instance == UART1)
    {

      //UART1 Pinmux Config: PA2 & PA3
      UART_GPIO_Init.Pin = GPIO_PIN_2 | GPIO_PIN_3;
      UART_GPIO_Init.Alternate = GPIO_AF8_UART1;
      LL_GPIO_Init(GPIOA, &UART_GPIO_Init);

      //UART1 Bus Clock Enable and Soft Reset Release
      LL_RCU_UART1_ClkEnRstRelease();

      //NVIC UART1 Interrupt Enable
//      LL_NVIC_EnableIRQ(UART1_IRQn);

    }
  else if(Instance == UART2)
    {

      //UART2 Pinmux Config: PC10 & PC11
      UART_GPIO_Init.Pin = GPIO_PIN_10 | GPIO_PIN_11;
      UART_GPIO_Init.Alternate = GPIO_AF6_UART2;
      LL_GPIO_Init(GPIOC, &UART_GPIO_Init);

      //UART2 Bus Clock Enable and Soft Reset Release
      LL_RCU_UART2_ClkEnRstRelease();

      //NVIC UART2 Interrupt Enable
//      LL_NVIC_EnableIRQ(UART2_IRQn);

    }
  else if(Instance == UART3)
    {

      //UART3 Pinmux Config: PC10 & PC11
      UART_GPIO_Init.Pin = GPIO_PIN_10 | GPIO_PIN_11;
      UART_GPIO_Init.Alternate = GPIO_AF8_UART3;
      LL_GPIO_Init(GPIOC, &UART_GPIO_Init);

      //UART3 Bus Clock Enable and Soft Reset Release
      LL_RCU_UART3_ClkEnRstRelease();

      //NVIC UART3 Interrupt Enable
//      LL_NVIC_EnableIRQ(UART3_IRQn);

    }
  else if(Instance == UART4)
    {

      //UART4 Pinmux Config: PC12
      UART_GPIO_Init.Pin = GPIO_PIN_12;
      UART_GPIO_Init.Alternate = GPIO_AF6_UART4;
      LL_GPIO_Init(GPIOC, &UART_GPIO_Init);

      //UART4 Pinmux Config: PD2
      UART_GPIO_Init.Pin = GPIO_PIN_2;
      UART_GPIO_Init.Alternate = GPIO_AF6_UART4;
      LL_GPIO_Init(GPIOD, &UART_GPIO_Init);

      //UART4 Bus Clock Enable and Soft Reset Release
      LL_RCU_UART4_ClkEnRstRelease();

      //NVIC UART4 Interrupt Enable
//      LL_NVIC_EnableIRQ(UART4_IRQn);
    }
}

/**
  * @brief  DeInitializes the UART MSP
  * @param  Instance Specifies UART peripheral
  * @retval None
  */
void LL_UART_MspDeInit(UART_TypeDef* Instance)
{
  //Assert param
  assert_param(IS_UART_ALL_INSTANCE(Instance));

  if(Instance == UART0)
    {
      //NVIC UART0 Interrupt Disable
      LL_NVIC_DisableIRQ(UART0_IRQn);

      //UART0 Bus Clock Disable and Soft Reset Assert
      LL_RCU_UART0_ClkDisRstAssert();

      //UART0 Pinmux DeInit
      LL_GPIO_DeInit(GPIOC, GPIO_PIN_4 | GPIO_PIN_5);

    }
  else if(Instance == UART1)
    {

      //NVIC UART1 Interrupt Disable
      LL_NVIC_DisableIRQ(UART1_IRQn);

      //UART1 Bus Clock Disable and Soft Reset Assert
      LL_RCU_UART1_ClkDisRstAssert();

      //UART1 Pinmux DeInit
      LL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);

    }
  else if(Instance == UART2)
    {

      //NVIC UART2 Interrupt Disable
      LL_NVIC_DisableIRQ(UART2_IRQn);

      //UART2 Bus Clock Disable and Soft Reset Assert
      LL_RCU_UART2_ClkDisRstAssert();

      //UART2 Pinmux DeInit
      LL_GPIO_DeInit(GPIOC, GPIO_PIN_10 | GPIO_PIN_11);

    }
  else if(Instance == UART3)
    {

      //NVIC UART3 Interrupt Disable
      LL_NVIC_DisableIRQ(UART3_IRQn);

      //UART3 Bus Clock Disable and Soft Reset Assert
      LL_RCU_UART3_ClkDisRstAssert();

      //UART3 Pinmux DeInit
      LL_GPIO_DeInit(GPIOC, GPIO_PIN_10 | GPIO_PIN_11);

    }
  else if(Instance == UART4)
    {

      //NVIC UART4 Interrupt Disable
      LL_NVIC_DisableIRQ(UART4_IRQn);

      //UART4 Bus Clock Disable and Soft Reset Assert
      LL_RCU_UART4_ClkDisRstAssert();

      //UART4 Pinmux DeInit
      LL_GPIO_DeInit(GPIOC, GPIO_PIN_12);
      LL_GPIO_DeInit(GPIOD, GPIO_PIN_2);
    }
}
/**
  * @}
  */
void LL_ADC_MspInit(ADC_TypeDef* Instance)
{
  GPIO_InitTypeDef ADC_GPIO_Init;

  //Assert param
  assert_param(IS_ADC_ALL_INSTANCE(Instance));

  memset((void*)&ADC_GPIO_Init, 0x00, sizeof(ADC_GPIO_Init));

  //ADC GPIO Common Config
  ADC_GPIO_Init.IntMode   = GPIO_INT_MODE_CLOSE;
  ADC_GPIO_Init.OType     = GPIO_OTYPE_PP;
  ADC_GPIO_Init.Pull      = GPIO_NOPULL;
  ADC_GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
  ADC_GPIO_Init.Alternate = GPIO_AF15_ADC;

  if(Instance == ADC0)
    {
      //ADC0 Pinmux Config: PA0->ADC0_IN1
      ADC_GPIO_Init.Pin = GPIO_PIN_1;
      LL_GPIO_Init(GPIOA, &ADC_GPIO_Init);

      ADC_GPIO_Init.Pin = GPIO_PIN_2;
      LL_GPIO_Init(GPIOA, &ADC_GPIO_Init);

      ADC_GPIO_Init.Pin = GPIO_PIN_3;
      LL_GPIO_Init(GPIOA, &ADC_GPIO_Init);

      ADC_GPIO_Init.Pin = GPIO_PIN_11;
      LL_GPIO_Init(GPIOB, &ADC_GPIO_Init);

      //ADC0 Bus Clock Enable and Soft Reset Release
      LL_RCU_ADC0_ClkEnRstRelease();

      //NVIC ADC0 Interrupt Enable
      //LL_NVIC_EnableIRQ(ADC0_NORM_IRQn);
      //LL_NVIC_EnableIRQ(ADC0_SAMP_IRQn);
//            LL_NVIC_EnableIRQ(ADC0_HALF_IRQn);
//            LL_NVIC_EnableIRQ(ADC0_FULL_IRQn);

    }
  else if(Instance == ADC1)
    {

      //ADC1 Pinmux Config: PA0->ADC1_IN1
      ADC_GPIO_Init.Pin = GPIO_PIN_4;
      LL_GPIO_Init(GPIOA, &ADC_GPIO_Init);

      ADC_GPIO_Init.Pin = GPIO_PIN_5;
      LL_GPIO_Init(GPIOA, &ADC_GPIO_Init);

      ADC_GPIO_Init.Pin = GPIO_PIN_6;
      LL_GPIO_Init(GPIOA, &ADC_GPIO_Init);

      ADC_GPIO_Init.Pin = GPIO_PIN_2;
      LL_GPIO_Init(GPIOB, &ADC_GPIO_Init);

      //ADC1 Bus Clock Enable and Soft Reset Release
      LL_RCU_ADC1_ClkEnRstRelease();

      //NVIC ADC1 Interrupt Enable
//            LL_NVIC_EnableIRQ(ADC1_NORM_IRQn);
//            LL_NVIC_EnableIRQ(ADC1_SAMP_IRQn);
//            LL_NVIC_EnableIRQ(ADC1_HALF_IRQn);
//            LL_NVIC_EnableIRQ(ADC1_FULL_IRQn);

    }
  else if(Instance == ADC2)
    {

      //ADC2 Pinmux Config: PB1->ADC2_IN1
      ADC_GPIO_Init.Pin = GPIO_PIN_1;
      LL_GPIO_Init(GPIOB, &ADC_GPIO_Init);

      //ADC2 Bus Clock Enable and Soft Reset Release
      LL_RCU_ADC2_ClkEnRstRelease();

      //NVIC ADC2 Interrupt Enable
      //LL_NVIC_EnableIRQ(ADC2_NORM_IRQn);
      //LL_NVIC_EnableIRQ(ADC2_SAMP_IRQn);
//            LL_NVIC_EnableIRQ(ADC2_HALF_IRQn);
//            LL_NVIC_EnableIRQ(ADC2_FULL_IRQn);

    }
  else if(Instance == ADC3)
    {

      //ADC3 Pinmux Config: PE14->ADC3_IN1
      ADC_GPIO_Init.Pin = GPIO_PIN_14;
      LL_GPIO_Init(GPIOE, &ADC_GPIO_Init);

      //ADC3 Bus Clock Enable and Soft Reset Release
      LL_RCU_ADC3_ClkEnRstRelease();

      //NVIC ADC3 Interrupt Enable
      //LL_NVIC_EnableIRQ(ADC3_NORM_IRQn);
      //LL_NVIC_EnableIRQ(ADC3_SAMP_IRQn);
//            LL_NVIC_EnableIRQ(ADC3_HALF_IRQn);
//            LL_NVIC_EnableIRQ(ADC3_FULL_IRQn);
    }
}

/**
  * @}
  */
void LL_HRPWM_MspInit(HRPWM_TypeDef* Instance)
{
  //Assert param
  assert_param(IS_HRPWM_ALL_INSTANCE(Instance));

  //HRPWM Bus Clock Disable and Soft Reset Assert
  LL_RCU_HRPWM_ClkEnRstRelease();
  LL_GPIO_AF_Config(GPIOB, GPIO_PIN_14, GPIO_AF14_HRPWM);
  LL_GPIO_AF_Config(GPIOA, GPIO_PIN_8, GPIO_AF14_HRPWM);	
  //NVIC HRPAM Interrupt Enable
  //LL_NVIC_EnableIRQ(HRPWM_MST_IRQn);
//    LL_NVIC_EnableIRQ(HRPWM_SLV0_IRQn);
//    LL_NVIC_EnableIRQ(HRPWM_SLV1_IRQn);
//    LL_NVIC_EnableIRQ(HRPWM_SLV2_IRQn);
//    LL_NVIC_EnableIRQ(HRPWM_SLV3_IRQn);
//    LL_NVIC_EnableIRQ(HRPWM_SLV4_IRQn);
//    LL_NVIC_EnableIRQ(HRPWM_SLV5_IRQn);
//    LL_NVIC_EnableIRQ(HRPWM_SLV6_IRQn);
//    LL_NVIC_EnableIRQ(HRPWM_SLV7_IRQn);
  //LL_NVIC_EnableIRQ(HRPWM_COM_IRQn);
}



/**
  * @}
  */
void LL_DAC_MspInit(DAC_TypeDef* Instance)
{
  //Assert param
  assert_param(IS_DAC_ALL_INSTANCE(Instance));

  //DAC Bus Clock Enable and Soft Reset Release
  LL_RCU_DAC_ClkEnRstRelease();
}



/**
  * @}
  */
void LL_CMP_MspInit(CMP_TypeDef* Instance)
{
  //Assert param
  assert_param(IS_CMP_ALL_INSTANCE(Instance));

  //DAC Bus Clock Enable and Soft Reset Release
  LL_RCU_CMP_ClkEnRstRelease();
}

void LL_TMR_MspInit(TMR_TypeDef* Instance)
{
  //TMR7 Bus Clock Enable and Soft Reset Release
  LL_RCU_TMR7_ClkEnRstRelease();
  LL_RCU_TMR8_ClkEnRstRelease();
  LL_RCU_TMR3_ClkEnRstRelease();

  //NVIC TMR7 Interrupt Enable
  LL_NVIC_EnableIRQ(TMR7_IRQn);
  LL_NVIC_EnableIRQ(TMR8_IRQn);
  LL_NVIC_EnableIRQ(TMR3_IRQn);
}

void LL_IIR_MspInit(IIR_TypeDef* Instance)
{
  //Assert param
  assert_param(IS_IIR_ALL_INSTANCE(Instance));

  //NVIC IIR Interrupt Enable
  //LL_NVIC_EnableIRQ(IIR_IRQn);

  //IIR Bus Clock Enable and Soft Reset Release
  if(Instance == IIR0)
    {
      LL_RCU_IIR0_ClkEnRstRelease();
    }
  else if(Instance == IIR1)
    {
      LL_RCU_IIR1_ClkEnRstRelease();
    }
  else if(Instance == IIR2)
    {
      LL_RCU_IIR2_ClkEnRstRelease();
    }
  else if(Instance == IIR3)
    {
      LL_RCU_IIR3_ClkEnRstRelease();
    }
  else if(Instance == IIR4)
    {
      LL_RCU_IIR4_ClkEnRstRelease();
    }
  else if(Instance == IIR5)
    {
      LL_RCU_IIR5_ClkEnRstRelease();
    }
}


void LL_CORDIC_MspInit(CORDIC_TypeDef* Instance)
{
  //Assert param
  assert_param(IS_CORDIC_ALL_INSTANCE(Instance));

  //CORDIC Bus Clock Enable and Soft Reset Release
  LL_RCU_CORDIC_ClkEnRstRelease();
}
/************************* (C) COPYRIGHT Tai-Action *****END OF FILE***********/

