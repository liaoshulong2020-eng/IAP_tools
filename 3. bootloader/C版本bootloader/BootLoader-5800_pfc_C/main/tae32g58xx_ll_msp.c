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
#include "../main/main.h"
#include "gpio.h" //test

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

    //GPIO Interrupt Enable
	LL_NVIC_EnableIRQ(GPIOA_IRQn);
	LL_NVIC_EnableIRQ(GPIOB_IRQn);
	LL_NVIC_EnableIRQ(GPIOC_IRQn);
	LL_NVIC_EnableIRQ(GPIOD_IRQn);
	LL_NVIC_EnableIRQ(GPIOE_IRQn);
	LL_NVIC_EnableIRQ(GPIOF_IRQn);
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

    //GPIO Interrupt Disable
	LL_NVIC_DisableIRQ(GPIOA_IRQn);
	LL_NVIC_DisableIRQ(GPIOB_IRQn);
	LL_NVIC_DisableIRQ(GPIOC_IRQn);
	LL_NVIC_DisableIRQ(GPIOD_IRQn);
	LL_NVIC_DisableIRQ(GPIOE_IRQn);
	LL_NVIC_DisableIRQ(GPIOF_IRQn);
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
        //UART0 Pinmux Config: PB6 & PB7
        UART_GPIO_Init.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        UART_GPIO_Init.Alternate = GPIO_AF8_UART0;
        LL_GPIO_Init(GPIOB, &UART_GPIO_Init);

        //UART0 Bus Clock Enable and Soft Reset Release
        LL_RCU_UART0_ClkEnRstRelease();

        //NVIC UART0 Interrupt Enable
//        LL_NVIC_EnableIRQ(UART0_IRQn);
    }
    else if(Instance == UART1)
    {
        //UART1 Pinmux Config: PA2-->Tx & PA3--Rx
        UART_GPIO_Init.Pin = GPIO_PIN_2 | GPIO_PIN_3;
        UART_GPIO_Init.Alternate = GPIO_AF8_UART1;
        LL_GPIO_Init(GPIOA, &UART_GPIO_Init);

        //UART1 Bus Clock Enable and Soft Reset Release
        LL_RCU_UART1_ClkEnRstRelease();

        //NVIC UART1 Interrupt Enable
//        LL_NVIC_EnableIRQ(UART1_IRQn);
    }
    else if(Instance == UART2)
    {

        //UART2 Pinmux Config: PD0 & PD1
        UART_GPIO_Init.Pin = GPIO_PIN_0 | GPIO_PIN_1;
        UART_GPIO_Init.Alternate = GPIO_AF8_UART2;
        LL_GPIO_Init(GPIOD, &UART_GPIO_Init);

        //UART2 Bus Clock Enable and Soft Reset Release
        LL_RCU_UART2_ClkEnRstRelease();

        //NVIC UART2 Interrupt Enable
        LL_NVIC_EnableIRQ(UART2_IRQn);

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
        LL_NVIC_EnableIRQ(UART3_IRQn);

    }
    else if(Instance == UART4)
    {

        //UART4 Pinmux Config: PD2 & PD3
        UART_GPIO_Init.Pin = GPIO_PIN_2 | GPIO_PIN_3;
        UART_GPIO_Init.Alternate = GPIO_AF6_UART4;
        LL_GPIO_Init(GPIOD, &UART_GPIO_Init);

        //UART4 Bus Clock Enable and Soft Reset Release
        LL_RCU_UART4_ClkEnRstRelease();

        //NVIC UART4 Interrupt Enable
        LL_NVIC_EnableIRQ(UART4_IRQn);
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
        LL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);

    }
    else if(Instance == UART1)
    {

        //NVIC UART1 Interrupt Disable
        LL_NVIC_DisableIRQ(UART1_IRQn);

        //UART1 Bus Clock Disable and Soft Reset Assert
        LL_RCU_UART1_ClkDisRstAssert();

        //UART1 Pinmux DeInit
        LL_GPIO_DeInit(GPIOB, GPIO_PIN_3 | GPIO_PIN_4);

    }
    else if(Instance == UART2)
    {

        //NVIC UART2 Interrupt Disable
        LL_NVIC_DisableIRQ(UART2_IRQn);

        //UART2 Bus Clock Disable and Soft Reset Assert
        LL_RCU_UART2_ClkDisRstAssert();

        //UART2 Pinmux DeInit
        LL_GPIO_DeInit(GPIOD, GPIO_PIN_0 | GPIO_PIN_1);

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
        LL_GPIO_DeInit(GPIOD, GPIO_PIN_2 | GPIO_PIN_3);
    }
}

/**
  * @brief  Initializes the I2C MSP
  * @param  Instance Specifies I2C peripheral
  * @retval None
  */
void LL_I2C_MspInit(I2C_TypeDef *Instance)
{
    GPIO_InitTypeDef I2C_GPIO_Init;

    //Assert param
    assert_param(IS_I2C_ALL_INSTANCE(Instance));

    if (Instance == I2C0) {
        //I2C0 Pinmux Config: PA0(I2C_CLK) & PA1(I2C_SDA)
        I2C_GPIO_Init.Pin       = GPIO_PIN_0 | GPIO_PIN_1;
        I2C_GPIO_Init.IntMode   = GPIO_INT_MODE_CLOSE;
        I2C_GPIO_Init.OType     = GPIO_OTYPE_OD;
        I2C_GPIO_Init.Pull      = GPIO_PULLUP;
        I2C_GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
        I2C_GPIO_Init.Alternate = GPIO_AF12_I2C0;
        LL_GPIO_Init(GPIOA, &I2C_GPIO_Init);

        //I2C0 Bus Clock Enable and Soft Reset Release
        LL_RCU_I2C0_ClkEnRstRelease();

        //NVIC I2C0 Interrupt Enable
        LL_NVIC_EnableIRQ(I2C0_IRQn);

    } else if (Instance == I2C1) {

        //I2C1 Pinmux Config: PA9(I2C_CLK) & PA8(I2C_SDA)
        I2C_GPIO_Init.Pin       = GPIO_PIN_8 | GPIO_PIN_9;
        I2C_GPIO_Init.IntMode   = GPIO_INT_MODE_CLOSE;
        I2C_GPIO_Init.OType     = GPIO_OTYPE_OD;
        I2C_GPIO_Init.Pull      = GPIO_PULLUP;
        I2C_GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
        I2C_GPIO_Init.Alternate = GPIO_AF5_I2C;
        LL_GPIO_Init(GPIOA, &I2C_GPIO_Init);

        //I2C1 Bus Clock Enable and Soft Reset Release
        LL_RCU_I2C1_ClkEnRstRelease();

        //NVIC I2C1 Interrupt Enable
        LL_NVIC_EnableIRQ(I2C1_IRQn);

    } else if (Instance == I2C2) {

        //I2C2 Pinmux Config: PC8(I2C_CLK) & PC9(I2C_SDA)
        I2C_GPIO_Init.Pin       = GPIO_PIN_8 | GPIO_PIN_9;
        I2C_GPIO_Init.IntMode   = GPIO_INT_MODE_CLOSE;
        I2C_GPIO_Init.OType     = GPIO_OTYPE_OD;
        I2C_GPIO_Init.Pull      = GPIO_PULLUP;
        I2C_GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
        I2C_GPIO_Init.Alternate = GPIO_AF9_I2C2;
        LL_GPIO_Init(GPIOC, &I2C_GPIO_Init);

        //I2C2 Bus Clock Enable and Soft Reset Release
        LL_RCU_I2C2_ClkEnRstRelease();

        //NVIC I2C2 Interrupt Enable
        LL_NVIC_EnableIRQ(I2C2_IRQn);
    }
}

/**
  * @brief  DeInitializes the I2C MSP
  * @param  Instance Specifies I2C peripheral
  * @retval None
  */
void LL_I2C_MspDeInit(I2C_TypeDef *Instance)
{
    //Assert param
    assert_param(IS_I2C_ALL_INSTANCE(Instance));

    if (Instance == I2C0) {
        //NVIC I2C0 Interrupt Disable
        LL_NVIC_DisableIRQ(I2C0_IRQn);

        //I2C0 Bus Clock Disable and Soft Reset Assert
        LL_RCU_I2C0_ClkDisRstAssert();

        //I2C0 Pinmux DeInit
        LL_GPIO_DeInit(GPIOA, GPIO_PIN_0 | GPIO_PIN_1);

    } else if (Instance == I2C1) {

        //NVIC I2C1 Interrupt Disable
        LL_NVIC_DisableIRQ(I2C1_IRQn);

        //I2C1 Bus Clock Disable and Soft Reset Assert
        LL_RCU_I2C1_ClkDisRstAssert();

        //I2C1 Pinmux DeInit
        LL_GPIO_DeInit(GPIOA, GPIO_PIN_8 | GPIO_PIN_9);

    } else if (Instance == I2C2) {

        //NVIC I2C2 Interrupt Disable
        LL_NVIC_DisableIRQ(I2C2_IRQn);

        //I2C2 Bus Clock Disable and Soft Reset Assert
        LL_RCU_I2C2_ClkDisRstAssert();

        //I2C2 Pinmux DeInit
        LL_GPIO_DeInit(GPIOC, GPIO_PIN_8 | GPIO_PIN_9);
    }
}

/**
  * @brief  Initializes the CAN MSP
  * @param  Instance Specifies CAN peripheral
  * @retval None
  */
void LL_CAN_MspInit(CAN_TypeDef *Instance)
{
    GPIO_InitTypeDef CAN_GPIO_Init;

    //Assert param
    assert_param(IS_CAN_ALL_INSTANCE(Instance));

    //CAN GPIO Common Config
    memset((void *)&CAN_GPIO_Init, 0x00, sizeof(CAN_GPIO_Init));
    CAN_GPIO_Init.IntMode   = GPIO_INT_MODE_CLOSE;
    CAN_GPIO_Init.OType     = GPIO_OTYPE_PP;
    CAN_GPIO_Init.Pull      = GPIO_NOPULL;   //external hardware Pull Up
    CAN_GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
    CAN_GPIO_Init.Alternate = GPIO_AF10_CAN;

    if (Instance == CAN0) {
        //CAN0 Pinmux Config: CAN0_RX->PB3 & CAN0_TX->PB4
        CAN_GPIO_Init.Pin = GPIO_PIN_3 | GPIO_PIN_4;
        LL_GPIO_Init(GPIOB, &CAN_GPIO_Init);

        //CAN0 Bus Clock Enable and Soft Reset Release
        LL_RCU_CAN0_ClkEnRstRelease();

        //NVIC CAN0 Interrupt Enable
//        LL_NVIC_EnableIRQ(CAN0_IRQn);

    } else if (Instance == CAN1) {

        //CAN1 Pinmux Config: CAN1_RX->PB12 & CAN1_TX->PB13
    	//CAN1 Pinmux Config: CAN1_RX->PB5 & CAN1_TX->PB6
        CAN_GPIO_Init.Pin = GPIO_PIN_5 | GPIO_PIN_6;
        LL_GPIO_Init(GPIOB, &CAN_GPIO_Init);

        //CAN1 Bus Clock Enable and Soft Reset Release
        LL_RCU_CAN1_ClkEnRstRelease();

        //NVIC CAN1 Interrupt Enable
//        LL_NVIC_EnableIRQ(CAN1_IRQn);
    }
}

/**
  * @brief  DeInitializes the CAN MSP
  * @param  Instance Specifies CAN peripheral
  * @retval None
  */
void LL_CAN_MspDeInit(CAN_TypeDef *Instance)
{
    //Assert param
    assert_param(IS_CAN_ALL_INSTANCE(Instance));

    if (Instance == CAN0) {
        //NVIC CAN0 Interrupt Disable
//        LL_NVIC_DisableIRQ(CAN0_IRQn);

        //CAN0 Bus Clock Disable and Soft Reset Assert
        LL_RCU_CAN0_ClkDisRstAssert();

        //CAN0 Pinmux DeInit
        LL_GPIO_DeInit(GPIOB, GPIO_PIN_3 | GPIO_PIN_4);

    }  else if (Instance == CAN1) {

        //NVIC CAN1 Interrupt Disable
//        LL_NVIC_DisableIRQ(CAN1_IRQn);

        //CAN1 Bus Clock Disable and Soft Reset Assert
        LL_RCU_CAN1_ClkDisRstAssert();

        //CAN1 Pinmux DeInit
        LL_GPIO_DeInit(GPIOB, GPIO_PIN_5 | GPIO_PIN_6);
    }
}

/**
  * @brief  Initializes the ADC MSP
  * @param  Instance Specifies ADC peripheral
  * @retval None
  */
void LL_ADC_MspInit(ADC_TypeDef *Instance)
{
    GPIO_InitTypeDef ADC_GPIO_Init;

    //Assert param
    assert_param(IS_ADC_ALL_INSTANCE(Instance));

    memset((void *)&ADC_GPIO_Init, 0x00, sizeof(ADC_GPIO_Init));

    //ADC GPIO Common Config
    ADC_GPIO_Init.IntMode   = GPIO_INT_MODE_CLOSE;
    ADC_GPIO_Init.OType     = GPIO_OTYPE_PP;
    ADC_GPIO_Init.Pull      = GPIO_NOPULL;
    ADC_GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
    ADC_GPIO_Init.Alternate = GPIO_AF15_ADC;

    if (Instance == ADC0) {
        /*
         * ADC0 Pinmux Config:
         * PA0->ADC0_IN1, PA2->ADC0-IN3, PA3->ADC0-IN4
         * PB0->ADC0-IN15
         */
        ADC_GPIO_Init.Pin = GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3;
        LL_GPIO_Init(GPIOA, &ADC_GPIO_Init);
        ADC_GPIO_Init.Pin = GPIO_PIN_0;
		LL_GPIO_Init(GPIOB, &ADC_GPIO_Init);

        //ADC0 Bus Clock Enable and Soft Reset Release
        LL_RCU_ADC0_ClkEnRstRelease();

        //NVIC ADC0 Interrupt Enable
        //LL_NVIC_EnableIRQ(ADC0_NORM_IRQn);
        //LL_NVIC_EnableIRQ(ADC0_SAMP_IRQn);
//        LL_NVIC_EnableIRQ(ADC0_HALF_IRQn);
//        LL_NVIC_EnableIRQ(ADC0_FULL_IRQn);

    } else if (Instance == ADC1) {

        /*
         * ADC1 Pinmux Config:
         * PA7->ADC1_IN4, PA5->ADC1-IN13, PA1->ADC1-IN2
         * PB2->ADC1-IN12
         */
        ADC_GPIO_Init.Pin = GPIO_PIN_7|GPIO_PIN_5|GPIO_PIN_1;
        LL_GPIO_Init(GPIOA, &ADC_GPIO_Init);
        ADC_GPIO_Init.Pin=GPIO_PIN_2;
        LL_GPIO_Init(GPIOB, &ADC_GPIO_Init);

        //ADC1 Bus Clock Enable and Soft Reset Release
        LL_RCU_ADC1_ClkEnRstRelease();

        //NVIC ADC1 Interrupt Enable
        //LL_NVIC_EnableIRQ(ADC1_NORM_IRQn);
        //LL_NVIC_EnableIRQ(ADC1_SAMP_IRQn);
//        LL_NVIC_EnableIRQ(ADC1_HALF_IRQn);
//        LL_NVIC_EnableIRQ(ADC1_FULL_IRQn);

    } else if (Instance == ADC2) {

        //ADC2 Pinmux Config: PB1->ADC2_IN1
        ADC_GPIO_Init.Pin = GPIO_PIN_1;
        LL_GPIO_Init(GPIOB, &ADC_GPIO_Init);

        //ADC2 Bus Clock Enable and Soft Reset Release
        LL_RCU_ADC2_ClkEnRstRelease();

        //NVIC ADC2 Interrupt Enable
        //LL_NVIC_EnableIRQ(ADC2_NORM_IRQn);
        ///LL_NVIC_EnableIRQ(ADC2_SAMP_IRQn);
//        LL_NVIC_EnableIRQ(ADC2_HALF_IRQn);
//        LL_NVIC_EnableIRQ(ADC2_FULL_IRQn);

    } else if (Instance == ADC3) {

        //ADC3 Pinmux Config: PE14->ADC3_IN1
        ADC_GPIO_Init.Pin = GPIO_PIN_14;
        LL_GPIO_Init(GPIOE, &ADC_GPIO_Init);

        //ADC3 Bus Clock Enable and Soft Reset Release
        LL_RCU_ADC3_ClkEnRstRelease();

        //NVIC ADC3 Interrupt Enable
        //LL_NVIC_EnableIRQ(ADC3_NORM_IRQn);
        //LL_NVIC_EnableIRQ(ADC3_SAMP_IRQn);
//        LL_NVIC_EnableIRQ(ADC3_HALF_IRQn);
//        LL_NVIC_EnableIRQ(ADC3_FULL_IRQn);
    }
}

/**
  * @brief  DeInitializes the ADC MSP
  * @param  Instance Specifies ADC peripheral
  * @retval None
  */
void LL_ADC_MspDeInit(ADC_TypeDef *Instance)
{
    //Assert param
    assert_param(IS_ADC_ALL_INSTANCE(Instance));

    if (Instance == ADC0) {
        //NVIC ADC0 Interrupt Disable
        LL_NVIC_DisableIRQ(ADC0_NORM_IRQn);
        LL_NVIC_DisableIRQ(ADC0_SAMP_IRQn);
        LL_NVIC_DisableIRQ(ADC0_HALF_IRQn);
        LL_NVIC_DisableIRQ(ADC0_FULL_IRQn);

        //ADC0 Bus Clock Disable and Soft Reset Assert
        LL_RCU_ADC0_ClkDisRstAssert();

        //ADC0 Pinmux DeInit
        LL_GPIO_DeInit(GPIOA, GPIO_PIN_0);

    } else if (Instance == ADC1) {

        //NVIC ADC1 Interrupt Disable
        LL_NVIC_DisableIRQ(ADC1_NORM_IRQn);
        LL_NVIC_DisableIRQ(ADC1_SAMP_IRQn);
        LL_NVIC_DisableIRQ(ADC1_HALF_IRQn);
        LL_NVIC_DisableIRQ(ADC1_FULL_IRQn);

        //ADC1 Bus Clock Disable and Soft Reset Assert
        LL_RCU_ADC1_ClkDisRstAssert();

        //ADC1 Pinmux DeInit
        LL_GPIO_DeInit(GPIOA, GPIO_PIN_0);

    } else if (Instance == ADC2) {

        //NVIC ADC2 Interrupt Disable
        LL_NVIC_DisableIRQ(ADC2_NORM_IRQn);
        LL_NVIC_DisableIRQ(ADC2_SAMP_IRQn);
        LL_NVIC_DisableIRQ(ADC2_HALF_IRQn);
        LL_NVIC_DisableIRQ(ADC2_FULL_IRQn);

        //ADC2 Bus Clock Disable and Soft Reset Assert
        LL_RCU_ADC2_ClkDisRstAssert();

        //ADC2 Pinmux DeInit
        LL_GPIO_DeInit(GPIOB, GPIO_PIN_1);

    } else if (Instance == ADC3) {

        //NVIC ADC3 Interrupt Disable
        LL_NVIC_DisableIRQ(ADC3_NORM_IRQn);
        LL_NVIC_DisableIRQ(ADC3_SAMP_IRQn);
        LL_NVIC_DisableIRQ(ADC3_HALF_IRQn);
        LL_NVIC_DisableIRQ(ADC3_FULL_IRQn);

        //ADC3 Bus Clock Disable and Soft Reset Assert
        LL_RCU_ADC3_ClkDisRstAssert();

        //ADC3 Pinmux DeInit
        LL_GPIO_DeInit(GPIOE, GPIO_PIN_14);

    }
}

/**
  * @brief  Initializes the DAC MSP
  * @param  Instance Specifies DAC peripheral
  * @return None
  */
void LL_DAC_MspInit(DAC_TypeDef *Instance)
{
    GPIO_InitTypeDef DAC_GPIO_Init;

    //Assert param
    assert_param(IS_DAC_ALL_INSTANCE(Instance));

    //DAC Bus Clock Enable and Soft Reset Release
    LL_RCU_DAC_ClkEnRstRelease();

    memset((void *)&DAC_GPIO_Init, 0x00, sizeof(DAC_GPIO_Init));

    //DAC GPIO Common Config
    DAC_GPIO_Init.IntMode   = GPIO_INT_MODE_CLOSE;
    DAC_GPIO_Init.OType     = GPIO_OTYPE_PP;
    DAC_GPIO_Init.Pull      = GPIO_NOPULL;
    DAC_GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
    DAC_GPIO_Init.Alternate = GPIO_AF15_DAC;

    if (Instance == DAC0) {
        //DAC0 Pinmux Config: PA4->DAC0_OUT
        DAC_GPIO_Init.Pin = GPIO_PIN_4;
        LL_GPIO_Init(GPIOA, &DAC_GPIO_Init);

    } else if (Instance == DAC1) {

        //DAC1 Pinmux Config: PA5->DAC1_OUT
        DAC_GPIO_Init.Pin = GPIO_PIN_5;
        LL_GPIO_Init(GPIOA, &DAC_GPIO_Init);

    } else if (Instance == DAC2) {

        //DAC2 Pinmux Config: PA6->DAC2_OUT
        DAC_GPIO_Init.Pin = GPIO_PIN_6;
        LL_GPIO_Init(GPIOA, &DAC_GPIO_Init);
    }
}

/**
  * @brief  DeInitializes the DAC MSP
  * @param  Instance Specifies DAC peripheral
  * @retval None
  */
void LL_DAC_MspDeInit(DAC_TypeDef *Instance)
{
    //Assert param
    assert_param(IS_DAC_ALL_INSTANCE(Instance));

    //DAC Bus Clock Disable and Soft Reset Assert
    LL_RCU_DAC_ClkDisRstAssert();

    if (Instance == DAC0) {
        //DAC0 Pinmux DeInit
        LL_GPIO_DeInit(GPIOA, GPIO_PIN_4);

    } else if (Instance == DAC1) {

        //DAC1 Pinmux DeInit
        LL_GPIO_DeInit(GPIOA, GPIO_PIN_5);

    } else if (Instance == DAC2) {

        //DAC2 Pinmux DeInit
        LL_GPIO_DeInit(GPIOA, GPIO_PIN_6);
    }
}

/**
  * @}
  */
void LL_CMP_MspInit(CMP_TypeDef* Instance)
{
    LL_RCU_CMP_ClkEnRstRelease();
}

/**
  * @}
  */
void LL_HRPWM_MspInit(HRPWM_TypeDef* Instance)
{
    GPIO_InitTypeDef HRPWM_GPIO_Init;

    //Assert param
    assert_param(IS_HRPWM_ALL_INSTANCE(Instance));

    //HRPWM GPIO Common Config
    memset((void*)&HRPWM_GPIO_Init, 0x00, sizeof(HRPWM_GPIO_Init));
    HRPWM_GPIO_Init.IntMode = GPIO_INT_MODE_CLOSE;
    HRPWM_GPIO_Init.OType   = GPIO_OTYPE_PP;
    HRPWM_GPIO_Init.Pull    = GPIO_NOPULL;
    HRPWM_GPIO_Init.Speed   = GPIO_SPEED_FREQ_HIGH;//GPIO_SPEED_FREQ_LOW;
    HRPWM_GPIO_Init.Alternate = GPIO_AF14_HRPWM;

    //HRPWM Slave 0 Output Pinmux Config: PA8->OUT0A, PA9->OUT0B
	HRPWM_GPIO_Init.Pin = GPIO_PIN_8 | GPIO_PIN_9;
	LL_GPIO_Init(GPIOA, &HRPWM_GPIO_Init);

//	//HRPWM Slave 1 Output Pinmux Config: PA10->OUT1A, PA11->OUT1B
//	HRPWM_GPIO_Init.Pin = GPIO_PIN_10 | GPIO_PIN_11;
//	LL_GPIO_Init(GPIOA, &HRPWM_GPIO_Init);

	//HRPWM Slave 2 Output Pinmux Config: PB12->OUT2A, PB13->OUT2B
	HRPWM_GPIO_Init.Pin = GPIO_PIN_12 | GPIO_PIN_13;
	LL_GPIO_Init(GPIOB, &HRPWM_GPIO_Init);

	//HRPWM Slave 3 Output Pinmux Config: PB14->OUT3A, PB15->OUT3B
	HRPWM_GPIO_Init.Pin = GPIO_PIN_14 | GPIO_PIN_15;
	LL_GPIO_Init(GPIOB, &HRPWM_GPIO_Init);

//	//HRPWM Slave 4 Output Pinmux Config: PC8->OUT4A, PC9->OUT4B
//	HRPWM_GPIO_Init.Pin = GPIO_PIN_8 | GPIO_PIN_9;
//	LL_GPIO_Init(GPIOC, &HRPWM_GPIO_Init);
//
//	//HRPWM Slave 5 Output Pinmux Config: PC6->OUT5A, PC7->OUT5B
//	HRPWM_GPIO_Init.Pin = GPIO_PIN_6 | GPIO_PIN_7;
//	LL_GPIO_Init(GPIOC, &HRPWM_GPIO_Init);
//
//	//HRPWM Slave 6 Output Pinmux Config: PD8->OUT6A, PD9->OUT6B
//	HRPWM_GPIO_Init.Pin = GPIO_PIN_8 | GPIO_PIN_9;
//	LL_GPIO_Init(GPIOD, &HRPWM_GPIO_Init);
//
//	//HRPWM Slave 7 Output Pinmux Config: PD10->OUT7A, PD11->OUT7B
//	HRPWM_GPIO_Init.Pin = GPIO_PIN_10 | GPIO_PIN_11;
//	LL_GPIO_Init(GPIOD, &HRPWM_GPIO_Init);

    //HRPWM Bus Clock Enable and Soft Reset Release
    LL_RCU_HRPWM_ClkEnRstRelease();
//    LL_RCU_HRPWM0_ClkEnRstRelease();
//    LL_RCU_HRPWM1_ClkEnRstRelease();
//    LL_RCU_HRPWM2_ClkEnRstRelease();
//    LL_RCU_HRPWM3_ClkEnRstRelease();
//    LL_RCU_HRPWM4_ClkEnRstRelease();
//    LL_RCU_HRPWM5_ClkEnRstRelease();
//    LL_RCU_HRPWM6_ClkEnRstRelease();
//    LL_RCU_HRPWM7_ClkEnRstRelease();

    //NVIC HRPAM Interrupt Enable
    //LL_NVIC_EnableIRQ(HRPWM_MST_IRQn);
    //LL_NVIC_EnableIRQ(HRPWM_SLV0_IRQn);
    //LL_NVIC_EnableIRQ(HRPWM_SLV1_IRQn);
    //LL_NVIC_EnableIRQ(HRPWM_SLV2_IRQn);
    //LL_NVIC_EnableIRQ(HRPWM_SLV3_IRQn);
    //LL_NVIC_EnableIRQ(HRPWM_SLV4_IRQn);
    //LL_NVIC_EnableIRQ(HRPWM_SLV5_IRQn);
    //LL_NVIC_EnableIRQ(HRPWM_SLV6_IRQn);
    //LL_NVIC_EnableIRQ(HRPWM_SLV7_IRQn);
    LL_NVIC_EnableIRQ(HRPWM_COM_IRQn);
}

/**
  * @brief  Initializes the TMR MSP
  * @param  Instance Specifies TMR peripheral
  * @retval None
  */
void LL_TMR_MspInit(TMR_TypeDef *Instance)
{
    //Assert param
    assert_param(IS_TMR_ALL_INSTANCE(Instance));

    if (Instance == TMR0) {
        //TMR0 Bus Clock Enable and Soft Reset Release
        LL_RCU_TMR0_ClkEnRstRelease();

        //NVIC TMR0 Interrupt Enable
        LL_NVIC_EnableIRQ(TMR0_IRQn);
    } else if (Instance == TMR1) {
        //TMR1 Bus Clock Enable and Soft Reset Release
        LL_RCU_TMR1_ClkEnRstRelease();

        //NVIC TMR1 Interrupt Enable
        LL_NVIC_EnableIRQ(TMR1_IRQn);
    } else if (Instance == TMR2) {
        //TMR2 Bus Clock Enable and Soft Reset Release
        LL_RCU_TMR2_ClkEnRstRelease();

        //NVIC TMR2 Interrupt Enable
        LL_NVIC_EnableIRQ(TMR2_IRQn);
    } else if (Instance == TMR3) {
        //TMR3 Bus Clock Enable and Soft Reset Release
        LL_RCU_TMR3_ClkEnRstRelease();

        //NVIC TMR3 Interrupt Enable
        LL_NVIC_EnableIRQ(TMR3_IRQn);
    } else if (Instance == TMR4) {
        //TMR4 Bus Clock Enable and Soft Reset Release
        LL_RCU_TMR4_ClkEnRstRelease();

        //NVIC TMR4 Interrupt Enable
        LL_NVIC_EnableIRQ(TMR4_IRQn);
    } else if (Instance == TMR7) {
        //TMR7 Bus Clock Enable and Soft Reset Release
        LL_RCU_TMR7_ClkEnRstRelease();

        //NVIC TMR7 Interrupt Enable
        LL_NVIC_EnableIRQ(TMR7_IRQn);
    } else if (Instance == TMR8) {
        //TMR8 Bus Clock Enable and Soft Reset Release
        LL_RCU_TMR8_ClkEnRstRelease();

        //NVIC TMR8 Interrupt Enable
        LL_NVIC_EnableIRQ(TMR8_IRQn);
    } else if (Instance == TMR9) {
        //TMR9 Bus Clock Enable and Soft Reset Release
        LL_RCU_TMR9_ClkEnRstRelease();

        //NVIC TMR9 Interrupt Enable
        LL_NVIC_EnableIRQ(TMR9_BRK_IRQn);
        LL_NVIC_EnableIRQ(TMR9_UPD_IRQn);
        LL_NVIC_EnableIRQ(TMR9_TRG_IRQn);
        LL_NVIC_EnableIRQ(TMR9_CC_IRQn);
    } else if (Instance == TMR10) {
        //TMR10 Bus Clock Enable and Soft Reset Release
        LL_RCU_TMR10_ClkEnRstRelease();

        //NVIC TMR10 Interrupt Enable
        LL_NVIC_EnableIRQ(TMR10_BRK_IRQn);
        LL_NVIC_EnableIRQ(TMR10_UPD_IRQn);
        LL_NVIC_EnableIRQ(TMR10_TRG_IRQn);
        LL_NVIC_EnableIRQ(TMR10_CC_IRQn);
    }
}

/**
  * @brief  DeInitializes the TMR MSP
  * @param  Instance Specifies TMR peripheral
  * @retval None
  */
void LL_TMR_MspDeInit(TMR_TypeDef *Instance)
{
    //Assert param
    assert_param(IS_TMR_ALL_INSTANCE(Instance));

    if (Instance == TMR0) {
        //TMR0 Bus Clock Disable and Soft Reset Assert
        LL_RCU_TMR0_ClkDisRstAssert();

        //NVIC TMR0 Interrupt Disable
        LL_NVIC_DisableIRQ(TMR0_IRQn);
    } else if (Instance == TMR1) {
        //TMR1 Bus Clock Disable and Soft Reset Assert
        LL_RCU_TMR1_ClkDisRstAssert();

        //NVIC TMR1 Interrupt Disable
        LL_NVIC_DisableIRQ(TMR1_IRQn);
    } else if (Instance == TMR2) {
        //TMR2 Bus Clock Disable and Soft Reset Assert
        LL_RCU_TMR2_ClkDisRstAssert();

        //NVIC TMR2 Interrupt Disable
        LL_NVIC_DisableIRQ(TMR2_IRQn);
    } else if (Instance == TMR3) {
        //TMR3 Bus Clock Disable and Soft Reset Assert
        LL_RCU_TMR3_ClkDisRstAssert();

        //NVIC TMR3 Interrupt Disable
        LL_NVIC_DisableIRQ(TMR3_IRQn);
    } else if (Instance == TMR4) {
        //TMR4 Bus Clock Disable and Soft Reset Assert
        LL_RCU_TMR4_ClkDisRstAssert();

        //NVIC TMR4 Interrupt Disable
        LL_NVIC_DisableIRQ(TMR4_IRQn);
    } else if (Instance == TMR7) {
        //TMR7 Bus Clock Disable and Soft Reset Assert
        LL_RCU_TMR7_ClkDisRstAssert();

        //NVIC TMR7 Interrupt Disable
        LL_NVIC_DisableIRQ(TMR7_IRQn);
    } else if (Instance == TMR8) {
        //TMR8 Bus Clock Disable and Soft Reset Assert
        LL_RCU_TMR8_ClkDisRstAssert();

        //NVIC TMR8 Interrupt Disable
        LL_NVIC_DisableIRQ(TMR8_IRQn);
    } else if (Instance == TMR9) {
        //TMR9 Bus Clock Disable and Soft Reset Assert
        LL_RCU_TMR9_ClkDisRstAssert();

        //NVIC TMR9 Interrupt Disable
        LL_NVIC_DisableIRQ(TMR9_BRK_IRQn);
        LL_NVIC_DisableIRQ(TMR9_UPD_IRQn);
        LL_NVIC_DisableIRQ(TMR9_TRG_IRQn);
        LL_NVIC_DisableIRQ(TMR9_CC_IRQn);
    } else if (Instance == TMR10) {
        //TMR10 Bus Clock Disable and Soft Reset Assert
        LL_RCU_TMR10_ClkDisRstAssert();

        //NVIC TMR10 Interrupt Disable
        LL_NVIC_DisableIRQ(TMR10_BRK_IRQn);
        LL_NVIC_DisableIRQ(TMR10_UPD_IRQn);
        LL_NVIC_DisableIRQ(TMR10_TRG_IRQn);
        LL_NVIC_DisableIRQ(TMR10_CC_IRQn);
    }
}

/************************* (C) COPYRIGHT Tai-Action *****END OF FILE***********/

