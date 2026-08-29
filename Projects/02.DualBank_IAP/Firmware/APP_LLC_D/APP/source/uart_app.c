#include "tae32g58xx_ll.h"
#include <string.h>
#include "uart_app.h"
#include "can_app.h"
#include "pri_sec_commun.h"

void UART_RX_DMA_init(void);
void UART_TX_DMA_init(void);

volatile uint8_t uart_rx_buf[PFC_UART_FRAME_LENGTH];

#define PFC_IAP_PREPARE       0xABU
#define PFC_IAP_READY         0xBAU
#define PFC_IAP_RESET         0x5AU
#define PFC_IAP_LEGACY        0xFFU
#define PFC_IAP_FRAME_HEADER  0xAAU
#define PFC_IAP_FRAME_TAIL    0x55U
#define PFC_ACK_FRAME_HEADER  0x55U
#define PFC_ACK_FRAME_TAIL    0xAAU
#define PFC_IAP_RETRIES       3U

static uint8_t iap_tx_frame[8];
static uint8_t iap_ack_frame[8];
static uint8_t iap_sequence;

static uint8_t uart_crc8(const uint8_t *data, uint16_t len)
{
    uint8_t crc = 0;
    uint16_t i;
    uint8_t bit;
    for (i = 0; i < len; ++i) {
        crc ^= data[i];
        for (bit = 0; bit < 8; ++bit)
            crc = (crc & 0x80U) ? (uint8_t)((crc << 1) ^ 0x07U) : (uint8_t)(crc << 1);
    }
    return crc;
}

static void uart_iap_build(uint8_t cmd, uint32_t id, uint8_t sequence)
{
    iap_tx_frame[0] = PFC_IAP_FRAME_HEADER;
    iap_tx_frame[1] = cmd;
    iap_tx_frame[2] = (uint8_t)id;
    iap_tx_frame[3] = (uint8_t)(id >> 8);
    iap_tx_frame[4] = (uint8_t)(id >> 16);
    iap_tx_frame[5] = sequence;
    iap_tx_frame[6] = uart_crc8(&iap_tx_frame[1], 5);
    iap_tx_frame[7] = PFC_IAP_FRAME_TAIL;
}

static void uart_iap_arm_ack(void)
{
    __LL_DMA_Ch_Dis(DMA, DMA_CHANNEL_0);
    __LL_DMA_TransCpltIntPnd_Clr(DMA, DMA_CHANNEL_0);
    memset(iap_ack_frame, 0, sizeof(iap_ack_frame));
    __LL_DMA_DstAddr_Set(DMA, DMA_CHANNEL_0, (uint32_t)iap_ack_frame);
    __LL_DMA_BlockTransCnt_Set(DMA, DMA_CHANNEL_0, sizeof(iap_ack_frame));
    __LL_UART_RxFIFO_Reset(USER_UART);
    __LL_DMA_Ch_En(DMA, DMA_CHANNEL_0);
}

static bool uart_iap_send_and_wait(uint8_t cmd, uint32_t id, uint8_t sequence)
{
    uint16_t wait;
    uart_iap_arm_ack();
    uart_iap_build(cmd, id, sequence);
    for (wait = 0; wait < 10U && __LL_DMA_ChEnSta_Get(DMA, DMA_CHANNEL_1); ++wait) delay_ms(1);
    if (__LL_DMA_ChEnSta_Get(DMA, DMA_CHANNEL_1)) return false;
    uart_send_u8data(iap_tx_frame);
    for (wait = 0; wait < 100U; ++wait) {
        if (__LL_DMA_IsTransCpltIntPnd(DMA, DMA_CHANNEL_0)) {
            __LL_DMA_TransCpltIntPnd_Clr(DMA, DMA_CHANNEL_0);
            return iap_ack_frame[0] == PFC_ACK_FRAME_HEADER &&
                   iap_ack_frame[1] == PFC_IAP_READY &&
                   iap_ack_frame[2] == (uint8_t)id &&
                   iap_ack_frame[3] == (uint8_t)(id >> 8) &&
                   iap_ack_frame[4] == (uint8_t)(id >> 16) &&
                   iap_ack_frame[5] == sequence &&
                   iap_ack_frame[6] == uart_crc8(&iap_ack_frame[1], 5) &&
                   iap_ack_frame[7] == PFC_ACK_FRAME_TAIL;
        }
        delay_ms(1);
    }
    return false;
}

bool uart_enter_pfc_iap(uint32_t device_id)
{
    uint8_t retry;
    uint32_t id = device_id & 0x00FFFFFFUL;
    uint8_t sequence = (uint8_t)(++iap_sequence ^ (uint8_t)DWT_CYCCNT ^
                                 (uint8_t)device_id ^ (uint8_t)(device_id >> 8));
    if (sequence == 0U) sequence = 1U;

    for (retry = 0; retry < PFC_IAP_RETRIES; ++retry) {
        if (uart_iap_send_and_wait(PFC_IAP_PREPARE, id, sequence)) {
            for (retry = 0; retry < PFC_IAP_RETRIES; ++retry) {
                if (uart_iap_send_and_wait(PFC_IAP_RESET, id, sequence)) return true;
            }
            break;
        }
    }

    /* Compatibility path for PFC products that only understand command 0xFF. */
    uart_iap_build(PFC_IAP_LEGACY, 0, 0);
    for (retry = 0; retry < PFC_IAP_RETRIES; ++retry) {
        while (__LL_DMA_ChEnSta_Get(DMA, DMA_CHANNEL_1)) delay_ms(1);
        uart_send_u8data(iap_tx_frame);
        delay_ms(20);
    }
    return false;
}

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
