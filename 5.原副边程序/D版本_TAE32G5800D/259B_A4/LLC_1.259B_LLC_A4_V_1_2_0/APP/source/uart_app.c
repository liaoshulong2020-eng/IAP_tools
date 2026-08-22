#include "tae32g58xx_ll.h"
#include <string.h>
#include "uart_app.h"
#include "can_app.h"
#include "pri_sec_commun.h"

void UART_RX_DMA_init(void);
void UART_TX_DMA_init(void);

volatile uint8_t uart_rx_buf[PFC_UART_FRAME_LENGTH];

void user_uart_init(UART_TypeDef *Instance)
{
    UART_InitTypeDef uart_init;

    memset((void *)&uart_init, 0, sizeof(uart_init));

    uart_init.baudrate = USER_UART_COM_BAUDRATE;
    uart_init.dat_len  = UART_DAT_LEN_8b;
    uart_init.parity   = UART_PARITY_NO;
    uart_init.stop_len = UART_STOP_LEN_1b;
    uart_init.user_callback.TxCpltCallback = User_Uart_TxCpltCallback;
    uart_init.user_callback.RxCpltCallback = User_Uart_RxCpltCallback;

    LL_UART_Init(Instance, &uart_init);

    __LL_UART_TxFIFOEmptyThres_Set(USER_UART, 12);
    __LL_UART_RxFIFOFullThres_Set(USER_UART, 1);

    __LL_UART_TxDMA_En(USER_UART);
    __LL_UART_RxDMA_En(USER_UART);

    UART_RX_DMA_init();
    UART_TX_DMA_init();

    __LL_UART_RxFull_INT_Dis(USER_UART);
    LL_NVIC_SetPriority(UART0_IRQn, 2, 0);
}

void UART_RX_DMA_init(void)
{
    DMA_UserCfgTypeDef dma_user_cfg;

    memset(&dma_user_cfg, 0, sizeof(dma_user_cfg));

    dma_user_cfg.trans_type = DMA_TRANS_TYPE_P2M;
    dma_user_cfg.trans_mode = DMA_TRANS_MODE_CONTINUE;
    dma_user_cfg.src_addr_mode = DMA_ADDR_MODE_FIX;
    dma_user_cfg.dst_addr_mode = DMA_ADDR_MODE_INC;
    dma_user_cfg.src_data_width = DMA_TRANS_WIDTH_8b;
    dma_user_cfg.dst_data_width = DMA_TRANS_WIDTH_8b;
    dma_user_cfg.src_hs_ifc = DMA_HANDSHAKE_IFC_UART0_RX;
    dma_user_cfg.dst_hs_ifc = DMA_HANDSHAKE_IFC_MEMORY;
    LL_DMA_Init(DMA, DMA_CHANNEL_0, &dma_user_cfg);

    __LL_DMA_SrcAddr_Set(DMA, DMA_CHANNEL_0, (uint32_t)&(USER_UART->RDR));
    __LL_DMA_SrcBurstLen_Set(DMA, DMA_CHANNEL_0, 0);
    __LL_DMA_DstAddr_Set(DMA, DMA_CHANNEL_0, (uint32_t)uart_rx_buf);
    __LL_DMA_DstBurstLen_Set(DMA, DMA_CHANNEL_0, 0);
    __LL_DMA_BlockTransCnt_Set(DMA, DMA_CHANNEL_0, PFC_UART_FRAME_LENGTH);

    LL_DMA_ChReqSpecific(DMA_CHANNEL_0);
    __LL_DMA_Ch_En(DMA, DMA_CHANNEL_0);
}

void UART_TX_DMA_init(void)
{
    DMA_UserCfgTypeDef dma_user_cfg;

    memset(&dma_user_cfg, 0, sizeof(dma_user_cfg));

    dma_user_cfg.trans_type = DMA_TRANS_TYPE_M2P;
    dma_user_cfg.trans_mode = DMA_TRANS_MODE_SINGLE;
    dma_user_cfg.src_addr_mode = DMA_ADDR_MODE_INC;
    dma_user_cfg.dst_addr_mode = DMA_ADDR_MODE_FIX;
    dma_user_cfg.src_data_width = DMA_TRANS_WIDTH_8b;
    dma_user_cfg.dst_data_width = DMA_TRANS_WIDTH_8b;
    dma_user_cfg.src_hs_ifc = DMA_HANDSHAKE_IFC_MEMORY;
    dma_user_cfg.dst_hs_ifc = DMA_HANDSHAKE_IFC_UART0_TX;
    LL_DMA_Init(DMA, DMA_CHANNEL_1, &dma_user_cfg);
    LL_DMA_ChReqSpecific(DMA_CHANNEL_1);

    __LL_DMA_SrcBurstLen_Set(DMA, DMA_CHANNEL_1, 0);
    __LL_DMA_DstAddr_Set(DMA, DMA_CHANNEL_1, (uint32_t)&(USER_UART->TDR));
    __LL_DMA_DstBurstLen_Set(DMA, DMA_CHANNEL_1, 0);
}

void uart_send_u8data(uint8_t *buf)
{
    if (__LL_DMA_ChEnSta_Get(DMA, DMA_CHANNEL_1)) {
        return;
    }

    __LL_DMA_TransCpltIntPnd_Clr(DMA, DMA_CHANNEL_1);
    __LL_DMA_SrcAddr_Set(DMA, DMA_CHANNEL_1, (uint32_t)buf);
    __LL_DMA_BlockTransCnt_Set(DMA, DMA_CHANNEL_1, 8);
    __LL_DMA_Ch_En(DMA, DMA_CHANNEL_1);
}

void uart_receive_data(void)
{
    if (DMA0->STR & (1 << 2)) {
        __LL_DMA_TransCpltIntPnd_Clr(DMA, DMA_CHANNEL_0);
        if (parse_pfc_data_from_buffer()) {
            pfc_uart_to_llc_massage();
        }
    }
}

void User_UART_DeInit(void)
{
    LL_UART_DeInit(USER_UART);
}
