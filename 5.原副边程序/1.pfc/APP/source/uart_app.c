///**
//  ******************************************************************************
//  * @file    user_uart.c
//  * @author  MCD Application Team
//  * @brief   This file provides the UART Config Method.
//  *
//  ******************************************************************************
//  * @attention
//  *
//  * <h2><center>&copy; Copyright (c) 2020 Tai-Action.
//  * All rights reserved.</center></h2>
//  *
//  * This software is licensed by Tai-Action under BSD 3-Clause license,
//  * the "License"; You may not use this file except in compliance with the
//  * License. You may obtain a copy of the License at:
//  *                        opensource.org/licenses/BSD-3-Clause
//  *
//  ******************************************************************************
//  */

///* Includes ------------------------------------------------------------------*/
//#include "tae32g58xx_ll.h"
//#include <string.h>
//#include "uart_app.h"
//#include "main.h"
//#define DBG_TAG     "User UART"
//#define DBG_LVL     DBG_LOG
//#include "dbg/tae_dbg.h"


///** @addtogroup TAE32G58xx_Examples
//  * @{
//  */

///** @addtogroup TAE32G58xx_UART_TxRx_IT_Example
//  * @{
//  */


///* Private Constants ---------------------------------------------------------*/
///** @defgroup User_UART_TxRx_IT_Private_Constants User UART_TxRx_IT Private Constants
//  * @brief    User UART_TxRx_IT Private Constants
//  * @{
//  */

///**
//  * @brief User UART Baud Rate Definition
//  */

///**
//  * @}
//  */

///* Private Macros ------------------------------------------------------------*/
///* Private Function Prototypes -----------------------------------------------*/
///** @defgroup User_UART_TxRx_IT_Private_Functions User UART_TxRx_IT Private Functions
//  * @brief    User UART_TxRx_IT Private Functions
//  * @{
//  */


///**
//  * @brief  User UART Init
//  * @param  Instance Specifies UART peripheral
//  * @return None
//  */
//void user_uart_init()
//{
//    UART_InitTypeDef uart_init;

//    memset((void *)&uart_init, 0, sizeof(uart_init));

//    //User UART Init
//    uart_init.baudrate = USER_UART_COM_BAUDRATE;
//    uart_init.dat_len  = UART_DAT_LEN_8b;
//    uart_init.parity   = UART_PARITY_NO;
//    uart_init.stop_len = UART_STOP_LEN_1b;
//	  uart_init.user_callback.TxCpltCallback = User_Uart_TxCpltCallback;
//		uart_init.user_callback.RxCpltCallback = User_Uart_RxCpltCallback;
//    LL_UART_Init(USER_UART, &uart_init);
//	
//		__LL_UART_RxFull_INT_En(USER_UART);
//	  LL_NVIC_SetPriority(UART0_IRQn, 4, 0);
//}

///**
//  * @brief  User UART DeInit
//  * @param  Instance Specifies UART peripheral
//  * @return None
//  */
//void User_UART_DeInit()
//{
//    LL_UART_DeInit(USER_UART);
//}



///************************* (C) COPYRIGHT Tai-Action *****END OF FILE***********/

