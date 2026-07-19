#include "pri_sec_commun_app.h"
#include "variables_define_app.h"
#include "vofa_app.h"
#include <string.h>

static volatile PFC_REPORT_DATA_TypeDef pfc_report_cache;
static volatile uint8_t pfc_comm_rx_buf[PFC_COMM_FRAME_TOTAL_SIZE];
static volatile uint8_t pfc_comm_rx_byte = 0;
static volatile uint16_t pfc_comm_rx_index = 0;
static volatile uint8_t pfc_comm_inited = 0;
static volatile uint8_t pfc_comm_valid = 0;
static volatile uint32_t pfc_comm_rx_ok_count = 0;
static volatile uint32_t pfc_comm_rx_err_count = 0;

static volatile uint8_t pfc_iap_active = 0;
static volatile uint8_t pfc_iap_entered = 0;
static uint16_t pfc_iap_enter_tick = 0;
static uint8_t pfc_iap_enter_try = 0;

static uint8_t pfc_iap_ack_buf[PFC_IAP_MAX_PACKET_SIZE];
static uint16_t pfc_iap_ack_size = 0;
static uint16_t pfc_iap_ack_total = 0;
static uint8_t pfc_iap_ack_state = 0;
static uint16_t pfc_iap_timeout_tick = 0;

static uint8_t pfc_iap_drop_first_packet = 0;
static uint8_t pfc_iap_drop_hdr[12];
static uint16_t pfc_iap_drop_count = 0;
static uint16_t pfc_iap_drop_total = 0;

static uint8_t pfc_comm_checksum(const uint8_t *data, uint16_t len)
{
    uint8_t checksum = 0;
    for(uint16_t i = 0; i < len; i++)
    {
        checksum ^= data[i];
    }
    return checksum;
}

static uint8_t pfc_iap_crc8(const uint8_t *data, uint16_t len)
{
    uint8_t crc = 0;

    for(uint16_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for(uint8_t j = 0; j < 8; j++)
        {
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x07) : (uint8_t)(crc << 1);
        }
    }

    return crc;
}

static uint16_t pfc_iap_crc16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0;

    for(uint16_t i = 0; i < len; i++)
    {
        crc ^= (uint16_t)data[i] << 8;
        for(uint8_t bit = 0; bit < 8; bit++)
        {
            crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
        }
    }

    return crc;
}

static void pfc_iap_reset_ack_parser(void)
{
    pfc_iap_ack_size = 0;
    pfc_iap_ack_total = 0;
    pfc_iap_ack_state = 0;
}

static void pfc_iap_send_can_ack(const uint8_t *data, uint16_t len)
{
    CAN_TxBufFormatTypeDef tx_buf_fmt;
    uint8_t can_data[8];
    uint16_t offset = 0;

    tx_buf_fmt.id_extension = 1;
    tx_buf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(PFC_IAP_CAN_ID);
    tx_buf_fmt.remote_tx_req = 0;

    while(offset < len)
    {
        uint8_t chunk = (uint8_t)((len - offset) > 8 ? 8 : (len - offset));
        memset(can_data, 0, sizeof(can_data));
        memcpy(can_data, &data[offset], chunk);
        tx_buf_fmt.data_len_code = chunk;
        LL_CAN_TransmitPTB_CPU(CAN1, &tx_buf_fmt, (uint32_t *)can_data);
        offset += chunk;
    }
}

static void pfc_iap_send_enter_cmd(void)
{
    uint8_t frame[8];

    frame[0] = 0xAA;
    frame[1] = 0xFF;
    frame[2] = 0x00;
    frame[3] = 0x00;
    frame[4] = 0x00;
    frame[5] = 0x00;
    frame[6] = pfc_iap_crc8(&frame[1], 5);
    frame[7] = 0x55;

    LL_UART_Transmit_CPU(PFC_COMM_UART, frame, sizeof(frame), 100000);
}

static void pfc_iap_drop_byte(uint8_t data)
{
    if(pfc_iap_drop_count < sizeof(pfc_iap_drop_hdr))
    {
        pfc_iap_drop_hdr[pfc_iap_drop_count] = data;
    }

    pfc_iap_drop_count++;

    if(pfc_iap_drop_count == 12)
    {
        uint16_t payload_size = (uint16_t)pfc_iap_drop_hdr[10] | ((uint16_t)pfc_iap_drop_hdr[11] << 8);
        pfc_iap_drop_total = (uint16_t)(payload_size + 14);
        if(pfc_iap_drop_total > PFC_IAP_MAX_PACKET_SIZE || pfc_iap_drop_hdr[0] != PFC_IAP_TARGET_ADDR || pfc_iap_drop_hdr[1] != PFC_IAP_FNO)
        {
            pfc_iap_drop_first_packet = 0;
            pfc_iap_drop_count = 0;
            pfc_iap_drop_total = 0;
            return;
        }
    }

    if(pfc_iap_drop_total != 0 && pfc_iap_drop_count >= pfc_iap_drop_total)
    {
        pfc_iap_drop_first_packet = 0;
        pfc_iap_drop_count = 0;
        pfc_iap_drop_total = 0;
    }
}

static void pfc_iap_recv_byte(uint8_t data)
{
    uint16_t crc_recv;
    uint16_t crc_calc;

    pfc_iap_timeout_tick = 0;

    if(pfc_iap_ack_state == 0)
    {
        if(data != PFC_IAP_TARGET_ADDR)
        {
            return;
        }

        pfc_iap_ack_buf[0] = data;
        pfc_iap_ack_size = 1;
        pfc_iap_ack_state = 1;
        return;
    }

    if(pfc_iap_ack_size >= PFC_IAP_MAX_PACKET_SIZE)
    {
        pfc_iap_reset_ack_parser();
        return;
    }

    pfc_iap_ack_buf[pfc_iap_ack_size++] = data;

    if(pfc_iap_ack_state == 1)
    {
        if(pfc_iap_ack_size >= 2)
        {
            pfc_iap_ack_state = (pfc_iap_ack_buf[1] == PFC_IAP_FNO) ? 2 : 0;
            if(pfc_iap_ack_state == 0)
            {
                pfc_iap_ack_size = 0;
            }
        }
        return;
    }

    if(pfc_iap_ack_state == 2 && pfc_iap_ack_size >= 12 && pfc_iap_ack_total == 0)
    {
        uint16_t payload_size = (uint16_t)pfc_iap_ack_buf[10] | ((uint16_t)pfc_iap_ack_buf[11] << 8);
        pfc_iap_ack_total = (uint16_t)(payload_size + 14);
        if(pfc_iap_ack_total > PFC_IAP_MAX_PACKET_SIZE)
        {
            pfc_iap_reset_ack_parser();
            return;
        }
    }

    if(pfc_iap_ack_total != 0 && pfc_iap_ack_size >= pfc_iap_ack_total)
    {
        crc_recv = (uint16_t)pfc_iap_ack_buf[pfc_iap_ack_total - 2] |
                   ((uint16_t)pfc_iap_ack_buf[pfc_iap_ack_total - 1] << 8);
        crc_calc = pfc_iap_crc16(pfc_iap_ack_buf, (uint16_t)(pfc_iap_ack_total - 2));

        if(crc_recv == crc_calc)
        {
            if(pfc_iap_ack_buf[2] == 1 && pfc_iap_ack_buf[3] == 0)
            {
                pfc_iap_entered = 1;
            }
            pfc_iap_send_can_ack(pfc_iap_ack_buf, pfc_iap_ack_total);
        }

        pfc_iap_reset_ack_parser();
    }
}

static uint16_t pfc_scale_u16(float value, float scale)
{
    if(value <= 0.0f)
    {
        return 0;
    }

    float scaled = value * scale;
    if(scaled > 65535.0f)
    {
        return 65535;
    }

    return (uint16_t)(scaled + 0.5f);
}

static int16_t pfc_scale_i16(float value, float scale)
{
    float scaled = value * scale;

    if(scaled > 32767.0f)
    {
        return 32767;
    }

    if(scaled < -32768.0f)
    {
        return -32768;
    }

    return (int16_t)(scaled >= 0.0f ? scaled + 0.5f : scaled - 0.5f);
}

static void pfc_put_u16(uint8_t *frame, uint8_t offset, uint16_t value)
{
    frame[offset] = (uint8_t)(value & 0xFF);
    frame[offset + 1] = (uint8_t)((value >> 8) & 0xFF);
}

static void pfc_put_i16(uint8_t *frame, uint8_t offset, int16_t value)
{
    pfc_put_u16(frame, offset, (uint16_t)value);
}

static bool pfc_comm_parse_frame(const uint8_t *frame)
{
    if(frame[0] != PFC_COMM_FRAME_HEADER ||
       frame[1] != PFC_COMM_CMD_DETAIL_INFO ||
       frame[2] != (uint8_t)PFC_COMM_FRAME_DATA_SIZE ||
       frame[PFC_COMM_TAIL_OFFSET] != PFC_COMM_FRAME_TAIL)
    {
        return false;
    }

    uint8_t checksum = pfc_comm_checksum(&frame[1], (uint16_t)(2 + PFC_COMM_FRAME_DATA_SIZE));
    if(checksum != frame[PFC_COMM_CHECKSUM_OFFSET])
    {
        return false;
    }

    memcpy((void *)&pfc_report_cache, &frame[3], sizeof(PFC_REPORT_DATA_TypeDef));
    pfc_comm_valid = 1;
    pfc_comm_rx_ok_count++;
    return true;
}

static void pfc_comm_reset_parser(void)
{
    pfc_comm_rx_index = 0;
}

static void pfc_comm_parse_byte(uint8_t data)
{
    if(pfc_comm_rx_index == 0)
    {
        if(data != PFC_COMM_FRAME_HEADER)
        {
            return;
        }

        pfc_comm_rx_buf[pfc_comm_rx_index++] = data;
        return;
    }

    if(pfc_comm_rx_index == 1)
    {
        if(data != PFC_COMM_CMD_DETAIL_INFO)
        {
            pfc_comm_reset_parser();
            if(data == PFC_COMM_FRAME_HEADER)
            {
                pfc_comm_rx_buf[pfc_comm_rx_index++] = data;
            }
            return;
        }

        pfc_comm_rx_buf[pfc_comm_rx_index++] = data;
        return;
    }

    if(pfc_comm_rx_index == 2)
    {
        if(data != (uint8_t)PFC_COMM_FRAME_DATA_SIZE)
        {
            pfc_comm_rx_err_count++;
            pfc_comm_reset_parser();
            if(data == PFC_COMM_FRAME_HEADER)
            {
                pfc_comm_rx_buf[pfc_comm_rx_index++] = data;
            }
            return;
        }

        pfc_comm_rx_buf[pfc_comm_rx_index++] = data;
        return;
    }

    if(pfc_comm_rx_index >= PFC_COMM_FRAME_TOTAL_SIZE)
    {
        pfc_comm_rx_err_count++;
        pfc_comm_reset_parser();
        return;
    }

    pfc_comm_rx_buf[pfc_comm_rx_index++] = data;

    if(pfc_comm_rx_index >= PFC_COMM_FRAME_TOTAL_SIZE)
    {
        if(!pfc_comm_parse_frame((const uint8_t *)pfc_comm_rx_buf))
        {
            pfc_comm_rx_err_count++;
        }
        pfc_comm_reset_parser();
    }
}

void pfc_comm_init(void)
{
    if(pfc_comm_inited)
    {
        return;
    }

    UART_InitTypeDef uart_init;
    memset((void *)&uart_init, 0, sizeof(uart_init));
    uart_init.baudrate = PFC_COMM_UART_BAUDRATE;
    uart_init.dat_len = UART_DAT_LEN_8b;
    uart_init.parity = UART_PARITY_NO;
    uart_init.stop_len = UART_STOP_LEN_1b;
    uart_init.user_callback.TxCpltCallback = User_PfcComm_Uart_TxCpltCallback;
    uart_init.user_callback.RxCpltCallback = User_PfcComm_Uart_RxCpltCallback;

    LL_UART_Init(PFC_COMM_UART, &uart_init);
    __LL_UART_RxFull_INT_En(PFC_COMM_UART);
    LL_NVIC_SetPriority(UART0_IRQn, 4, 0);
    pfc_comm_reset_parser();
    LL_UART_Receive_IT(PFC_COMM_UART, (uint8_t *)&pfc_comm_rx_byte, 1);
    pfc_comm_inited = 1;
}

void pfc_comm_deinit(void)
{
    if(!pfc_comm_inited)
    {
        return;
    }

    LL_UART_AbortReceive_IT(PFC_COMM_UART);
    __LL_UART_RxFull_INT_Dis(PFC_COMM_UART);
    LL_UART_DeInit(PFC_COMM_UART);
    pfc_comm_inited = 0;
}

void pfc_iap_forward_start(void)
{
    UART_InitTypeDef uart_init;

    if(pfc_iap_active)
    {
        return;
    }

    pfc_comm_deinit();
    User_VOFA_UART_DeInit();
    llc_uart_work_mode = LLC_UART_MODE_IAP;

    memset((void *)&uart_init, 0, sizeof(uart_init));
    uart_init.baudrate = PFC_COMM_UART_BAUDRATE;
    uart_init.dat_len = UART_DAT_LEN_8b;
    uart_init.parity = UART_PARITY_NO;
    uart_init.stop_len = UART_STOP_LEN_1b;
    uart_init.user_callback.TxCpltCallback = User_PfcComm_Uart_TxCpltCallback;
    uart_init.user_callback.RxCpltCallback = User_PfcComm_Uart_RxCpltCallback;

    LL_UART_DeInit(PFC_COMM_UART);
    LL_UART_Init(PFC_COMM_UART, &uart_init);
    __LL_UART_RxFull_INT_Dis(PFC_COMM_UART);
    __LL_UART_RxFIFO_Reset(PFC_COMM_UART);

    pfc_iap_active = 1;
    pfc_iap_entered = 0;
    pfc_iap_enter_tick = 0;
    pfc_iap_enter_try = 0;
    pfc_iap_timeout_tick = 0;
    pfc_iap_drop_first_packet = 1;
    pfc_iap_drop_count = 0;
    pfc_iap_drop_total = 0;
    pfc_iap_reset_ack_parser();

    pfc_iap_send_enter_cmd();
}

bool pfc_iap_forward_is_active(void)
{
    return pfc_iap_active != 0;
}

void pfc_iap_forward_can_frame(const uint8_t *data, uint16_t len)
{
    if(data == NULL || len == 0)
    {
        return;
    }

    if(!pfc_iap_active)
    {
        pfc_iap_forward_start();
    }

    if(pfc_iap_drop_first_packet)
    {
        for(uint16_t i = 0; i < len; i++)
        {
            pfc_iap_drop_byte(data[i]);
        }
        return;
    }

    LL_UART_Transmit_CPU(PFC_COMM_UART, (uint8_t *)data, len, 100000);
}

void pfc_iap_forward_task(void)
{
    uint16_t rx_guard = 0;

    if(!pfc_iap_active)
    {
        return;
    }

    while(!__LL_UART_IsRxFIFOEmpty(PFC_COMM_UART) && rx_guard < 128)
    {
        uint8_t data = (uint8_t)__LL_UART_RxDat8bits_Read(PFC_COMM_UART);
        pfc_iap_recv_byte(data);
        rx_guard++;
    }

    if(pfc_iap_ack_size > 0)
    {
        pfc_iap_timeout_tick++;
        if(pfc_iap_timeout_tick >= 5000)
        {
            pfc_iap_timeout_tick = 0;
            pfc_iap_reset_ack_parser();
        }
    }

    if(!pfc_iap_entered)
    {
        pfc_iap_enter_tick++;
        if(pfc_iap_enter_tick >= 2500)
        {
            pfc_iap_enter_tick = 0;
            pfc_iap_enter_try++;
            pfc_iap_send_enter_cmd();

            if(pfc_iap_enter_try >= 3)
            {
                pfc_iap_entered = 1;
            }
        }
    }
}

bool pfc_comm_is_ready(void)
{
    return pfc_comm_valid != 0;
}

bool pfc_comm_fill_can_response(uint8_t query_cmd, uint8_t out_frame[8])
{
    PFC_REPORT_DATA_TypeDef data;
    memset(out_frame, 0, 8);

    if(!pfc_comm_valid)
    {
        return false;
    }

    memcpy(&data, (const void *)&pfc_report_cache, sizeof(PFC_REPORT_DATA_TypeDef));

    switch(query_cmd)
    {
        case CMD_PFC_INPUT_OVP:
            out_frame[1] = RETURN_BIT_PFC_INPUT_OVP;
            pfc_put_u16(out_frame, 2, pfc_scale_u16(data.vin_over_voltage_set.f, 10.0f));
            pfc_put_u16(out_frame, 4, pfc_scale_u16(data.vin_max_voltage_set.f, 10.0f));
            break;

        case CMD_PFC_INPUT_UVP:
            out_frame[1] = RETURN_BIT_PFC_INPUT_UVP;
            pfc_put_u16(out_frame, 2, pfc_scale_u16(data.vin_under_voltage_set.f, 10.0f));
            pfc_put_u16(out_frame, 4, pfc_scale_u16(data.vin_on_voltage_set.f, 10.0f));
            break;

        case CMD_PFC_OUTPUT_OVP:
            out_frame[1] = RETURN_BIT_PFC_OUTPUT_OVP;
            pfc_put_u16(out_frame, 2, pfc_scale_u16(data.vout_over_voltage_sw.f, 10.0f));
            pfc_put_u16(out_frame, 4, pfc_scale_u16(data.bus_ovp_point_hw.f, 10.0f));
            break;

        case CMD_PFC_OUTPUT_UVP:
            out_frame[1] = RETURN_BIT_PFC_OUTPUT_UVP;
            pfc_put_u16(out_frame, 2, 0);
            pfc_put_u16(out_frame, 4, 0);
            break;

        case CMD_PFC_INPUT_OCP:
            out_frame[1] = RETURN_BIT_PFC_INPUT_OCP;
            pfc_put_u16(out_frame, 2, pfc_scale_u16(data.ipfc_ocp_current_sw.f, 10.0f));
            pfc_put_u16(out_frame, 4, pfc_scale_u16(data.pfc_i_ocp_dac_point_hw.f, 10.0f));
            break;

        case CMD_PFC_DATA:
            out_frame[1] = RETURN_BIT_PFC_DATA;
            pfc_put_u16(out_frame, 2, pfc_scale_u16(data.vbus_target.f, 10.0f));
            pfc_put_u16(out_frame, 4, pfc_scale_u16(data.vbus_target.f, 10.0f));
            pfc_put_u16(out_frame, 6, pfc_scale_u16(data.vbus_rel.f, 10.0f));
            break;

        case CMD_PFC_DATA_LIVE1:
            out_frame[1] = RETURN_BIT_PFC_LIVE1;
            pfc_put_u16(out_frame, 2, pfc_scale_u16(data.vin_rel.f, 10.0f));
            pfc_put_u16(out_frame, 4, pfc_scale_u16(data.iloop_rel.f, 10.0f));
            pfc_put_i16(out_frame, 6, data.r_ntc_raw);
            break;

        case CMD_PFC_DATA_LIVE2:
            out_frame[1] = RETURN_BIT_PFC_LIVE2;
            out_frame[2] = data.state;
            out_frame[3] = data.switch_frequency;
            pfc_put_u16(out_frame, 4, pfc_scale_u16(data.duty_cycle.f, 1000.0f));
            pfc_put_u16(out_frame, 6, data.status_flags.all);
            break;

        default:
            return false;
    }

    return true;
}

void User_PfcComm_Uart_RxCpltCallback(void)
{
    if(llc_uart_work_mode == LLC_UART_MODE_PFC_COMM)
    {
        pfc_comm_parse_byte(pfc_comm_rx_byte);
        LL_UART_Receive_IT(PFC_COMM_UART, (uint8_t *)&pfc_comm_rx_byte, 1);
    }
}

void User_PfcComm_Uart_TxCpltCallback(void)
{
}
