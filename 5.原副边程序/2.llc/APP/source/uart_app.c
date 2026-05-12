/**
  ******************************************************************************
  * @file    uart_app.c
  * @brief   UART底层驱动（副边UART通讯，接收原边PFC数据）
  *          使用DMA方式收发，UART0，波特率115200
  ******************************************************************************
  */

#include "tae32g58xx_ll.h"
#include <string.h>
#include "uart_app.h"
#include "pri_sec_commun.h"

#define DBG_TAG     "User UART"
#define DBG_LVL     DBG_LOG
#include "dbg/tae_dbg.h"

/* DMA接收缓冲区，63字节对应PFC完整数据帧 */
volatile uint8_t uart_rx_buf[63];

/* 函数前向声明 */
void UART_RX_DMA_init(void);
void UART_TX_DMA_init(void);

/**
  * @brief  UART初始化（115200, 8N1, DMA收发）
  * @param  Instance: UART外设指针
  */
void user_uart_init(UART_TypeDef *Instance)
{
    UART_InitTypeDef uart_init;

    memset((void *)&uart_init, 0, sizeof(uart_init));

    uart_init.baudrate = 115200;
    uart_init.dat_len  = UART_DAT_LEN_8b;
    uart_init.parity   = UART_PARITY_NO;
    uart_init.stop_len = UART_STOP_LEN_1b;
    uart_init.user_callback.TxCpltCallback = User_Uart_TxCpltCallback;
    uart_init.user_callback.RxCpltCallback = User_Uart_RxCpltCallback;

    LL_UART_Init(Instance, &uart_init);

    __LL_UART_TxFIFOEmptyThres_Set(UART0, 12);
    __LL_UART_RxFIFOFullThres_Set(UART0, 1);

    __LL_UART_TxDMA_En(UART0);
    __LL_UART_RxDMA_En(UART0);

    UART_RX_DMA_init();
    UART_TX_DMA_init();

    __LL_UART_RxFull_INT_En(USER_UART);
    LL_NVIC_SetPriority(UART0_IRQn, 2, 0);
}

/**
  * @brief  DMA接收初始化（Channel 0，P2M，连续模式，63字节）
  */
void UART_RX_DMA_init(void)
{
    DMA_UserCfgTypeDef dma_user_cfg;

    memset(&dma_user_cfg, 0, sizeof(dma_user_cfg));

    dma_user_cfg.trans_type     = DMA_TRANS_TYPE_P2M;
    dma_user_cfg.trans_mode     = DMA_TRANS_MODE_CONTINUE;
    dma_user_cfg.src_addr_mode  = DMA_ADDR_MODE_FIX;
    dma_user_cfg.dst_addr_mode  = DMA_ADDR_MODE_INC;
    dma_user_cfg.src_data_width = DMA_TRANS_WIDTH_8b;
    dma_user_cfg.dst_data_width = DMA_TRANS_WIDTH_8b;
    dma_user_cfg.src_hs_ifc     = DMA_HANDSHAKE_IFC_UART0_RX;
    dma_user_cfg.dst_hs_ifc     = DMA_HANDSHAKE_IFC_MEMORY;
    LL_DMA_Init(DMA, DMA_CHANNEL_0, &dma_user_cfg);

    __LL_DMA_SrcAddr_Set(DMA, DMA_CHANNEL_0, (uint32_t)&(UART0->RDR));
    __LL_DMA_SrcBurstLen_Set(DMA, DMA_CHANNEL_0, 0);

    __LL_DMA_DstAddr_Set(DMA, DMA_CHANNEL_0, (uint32_t)uart_rx_buf);
    __LL_DMA_DstBurstLen_Set(DMA, DMA_CHANNEL_0, 0);

    __LL_DMA_BlockTransCnt_Set(DMA, DMA_CHANNEL_0, 63);

    LL_DMA_ChReqSpecific(DMA_CHANNEL_0);
    __LL_DMA_Ch_En(DMA, DMA_CHANNEL_0);
}

/**
  * @brief  DMA发送初始化（Channel 1，M2P，单次模式）
  */
void UART_TX_DMA_init(void)
{
    DMA_UserCfgTypeDef dma_user_cfg;

    memset(&dma_user_cfg, 0, sizeof(dma_user_cfg));

    dma_user_cfg.trans_type     = DMA_TRANS_TYPE_M2P;
    dma_user_cfg.trans_mode     = DMA_TRANS_MODE_SINGLE;
    dma_user_cfg.src_addr_mode  = DMA_ADDR_MODE_INC;
    dma_user_cfg.dst_addr_mode  = DMA_ADDR_MODE_FIX;
    dma_user_cfg.src_data_width = DMA_TRANS_WIDTH_8b;
    dma_user_cfg.dst_data_width = DMA_TRANS_WIDTH_8b;
    dma_user_cfg.src_hs_ifc     = DMA_HANDSHAKE_IFC_MEMORY;
    dma_user_cfg.dst_hs_ifc     = DMA_HANDSHAKE_IFC_UART0_TX;
    LL_DMA_Init(DMA, DMA_CHANNEL_1, &dma_user_cfg);

    __LL_DMA_SrcBurstLen_Set(DMA, DMA_CHANNEL_1, 0);
    __LL_DMA_DstAddr_Set(DMA, DMA_CHANNEL_1, (uint32_t)&(UART0->TDR));
    __LL_DMA_DstBurstLen_Set(DMA, DMA_CHANNEL_1, 0);
}

/**
  * @brief  通过DMA发送8字节数据给PFC原边
  * @param  buf: 指向8字节发送缓冲区的指针
  */
void uart_send_u8data(uint8_t *buf)
{
    __LL_DMA_SrcAddr_Set(DMA, DMA_CHANNEL_1, (uint32_t)buf);
    __LL_DMA_BlockTransCnt_Set(DMA, DMA_CHANNEL_1, 8);
    LL_DMA_ChReqSpecific(DMA_CHANNEL_1);
    __LL_DMA_Ch_En(DMA, DMA_CHANNEL_1);
}

/**
  * @brief  轮询检查DMA接收完成并解析PFC数据
  *         在主循环中调用，检查DMA Channel 0完成标志
  */
void uart_receive_data(void)
{
    if (DMA0->STR & (1 << 2)) /* DMA Channel 0 传输完成 */
    {
        __LL_DMA_TransCpltIntPnd_Clr(DMA, DMA_CHANNEL_0);
        parse_pfc_data_from_buffer();
    }
}

/**
  * @brief  UART反初始化
  */
void User_UART_DeInit(void)
{
    LL_UART_DeInit(USER_UART);
}
