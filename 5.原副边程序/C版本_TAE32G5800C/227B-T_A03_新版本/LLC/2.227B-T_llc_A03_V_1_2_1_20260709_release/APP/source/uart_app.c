/**
  ******************************************************************************
  * @file    user_uart.c
  * @author  MCD Application Team
  * @brief   This file provides the UART Config Method.
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
#include "uart_app.h"

#define DBG_TAG     "User UART"
#define DBG_LVL     DBG_LOG
#include "dbg/tae_dbg.h"


/** @addtogroup TAE32G58xx_Examples
  * @{
  */

/** @addtogroup TAE32G58xx_UART_TxRx_IT_Example
  * @{
  */


/* Private Constants ---------------------------------------------------------*/
/** @defgroup User_UART_TxRx_IT_Private_Constants User UART_TxRx_IT Private Constants
  * @brief    User UART_TxRx_IT Private Constants
  * @{
  */

/**
  * @brief User UART Baud Rate Definition
  */

/**
  * @}
  */

/* Private Macros ------------------------------------------------------------*/
/* Private Function Prototypes -----------------------------------------------*/
/** @defgroup User_UART_TxRx_IT_Private_Functions User UART_TxRx_IT Private Functions
  * @brief    User UART_TxRx_IT Private Functions
  * @{
  */
void User_UART_Init(UART_TypeDef *Instance);
void UART_RX_DMA_init(void);
void UART_TX_DMA_init(void);

volatile uint8_t uart_rx_buf[63];

//UART場宎趙
void user_uart_init(UART_TypeDef *Instance)
{
    UART_InitTypeDef uart_init;

    memset((void *)&uart_init, 0, sizeof(uart_init));
    
    //User UART Init
    uart_init.baudrate = 115200;
    uart_init.dat_len  = UART_DAT_LEN_8b;
    uart_init.parity   = UART_PARITY_NO;
    uart_init.stop_len = UART_STOP_LEN_1b;
		uart_init.user_callback.TxCpltCallback = User_Uart_TxCpltCallback;
		uart_init.user_callback.RxCpltCallback = User_Uart_RxCpltCallback;

    LL_UART_Init(Instance, &uart_init);

    __LL_UART_TxFIFOEmptyThres_Set(UART0,12);
    __LL_UART_RxFIFOFullThres_Set(UART0,1);
    
    __LL_UART_TxDMA_En(UART0);
    __LL_UART_RxDMA_En(UART0);
    
    UART_RX_DMA_init();
    UART_TX_DMA_init();
	
		__LL_UART_RxFull_INT_En(USER_UART);
	  LL_NVIC_SetPriority(UART0_IRQn, 2, 0);
}

void UART_RX_DMA_init(void)
{
    DMA_UserCfgTypeDef dma_user_cfg;

    memset(&dma_user_cfg, 0, sizeof(dma_user_cfg));

    //User DAM init
    dma_user_cfg.trans_type = DMA_TRANS_TYPE_P2M;              
    dma_user_cfg.trans_mode = DMA_TRANS_MODE_CONTINUE;       
    dma_user_cfg.src_addr_mode = DMA_ADDR_MODE_FIX;             
    dma_user_cfg.dst_addr_mode = DMA_ADDR_MODE_INC;             
    dma_user_cfg.src_data_width = DMA_TRANS_WIDTH_8b;           
    dma_user_cfg.dst_data_width = DMA_TRANS_WIDTH_8b;          
    dma_user_cfg.src_hs_ifc = DMA_HANDSHAKE_IFC_UART0_RX;        
    dma_user_cfg.dst_hs_ifc = DMA_HANDSHAKE_IFC_MEMORY;         
    LL_DMA_Init(DMA, DMA_CHANNEL_0, &dma_user_cfg);
   
    __LL_DMA_SrcAddr_Set(DMA,DMA_CHANNEL_0,(uint32_t)&(UART0->RDR));
    __LL_DMA_SrcBurstLen_Set(DMA,DMA_CHANNEL_0,0);
    
    __LL_DMA_DstAddr_Set(DMA,DMA_CHANNEL_0,(uint32_t)uart_rx_buf);
    __LL_DMA_DstBurstLen_Set(DMA,DMA_CHANNEL_0,0);
    
    __LL_DMA_BlockTransCnt_Set(DMA, DMA_CHANNEL_0, 63); //set length
    
    LL_DMA_ChReqSpecific(DMA_CHANNEL_0);
    
    __LL_DMA_Ch_En(DMA, DMA_CHANNEL_0);
}

void UART_TX_DMA_init(void)
{
    DMA_UserCfgTypeDef dma_user_cfg;

    memset(&dma_user_cfg, 0, sizeof(dma_user_cfg));

    //User DAM init
    dma_user_cfg.trans_type = DMA_TRANS_TYPE_M2P;          
    dma_user_cfg.trans_mode = DMA_TRANS_MODE_SINGLE;       
    dma_user_cfg.src_addr_mode = DMA_ADDR_MODE_INC;            
    dma_user_cfg.dst_addr_mode = DMA_ADDR_MODE_FIX;            
    dma_user_cfg.src_data_width = DMA_TRANS_WIDTH_8b;          
    dma_user_cfg.dst_data_width = DMA_TRANS_WIDTH_8b;          
    dma_user_cfg.src_hs_ifc = DMA_HANDSHAKE_IFC_MEMORY;         
    dma_user_cfg.dst_hs_ifc = DMA_HANDSHAKE_IFC_UART0_TX;        
    LL_DMA_Init(DMA, DMA_CHANNEL_1, &dma_user_cfg);
   
    __LL_DMA_SrcBurstLen_Set(DMA,DMA_CHANNEL_1,0);
    __LL_DMA_DstAddr_Set(DMA,DMA_CHANNEL_1,(uint32_t)&(UART0->TDR));
    __LL_DMA_DstBurstLen_Set(DMA,DMA_CHANNEL_1,0);
}

void uart_send_u8data(uint8_t * buf)
{
    __LL_DMA_SrcAddr_Set(DMA,DMA_CHANNEL_1,(uint32_t)buf);
    __LL_DMA_BlockTransCnt_Set(DMA, DMA_CHANNEL_1, 8); //set length
    LL_DMA_ChReqSpecific(DMA_CHANNEL_1);
    __LL_DMA_Ch_En(DMA, DMA_CHANNEL_1);
}
void uart_receive_data(void)
{
  if(DMA0->STR & (1<<2)) //RX OK
     {
      __LL_DMA_TransCpltIntPnd_Clr(DMA,DMA_CHANNEL_0);
			parse_pfc_data_from_buffer();
     }
    
}
/**
  * @brief  User UART DeInit
  * @param  Instance Specifies UART peripheral
  * @return None
  */
void User_UART_DeInit()
{
    LL_UART_DeInit(USER_UART);
}

// UART﹞⊿?赤赤那3谷??米¯o‘那y
//void User_Uart_TxCpltCallback(void)
//{

//}

//// UART?車那?赤那3谷??米¯o‘那y
//void User_Uart_RxCpltCallback(void)
//{
//    // ∩|角赤?車那?米?米?車|∩e?辰?邦芍?
//}


/************************* (C) COPYRIGHT Tai-Action *****END OF FILE***********/

