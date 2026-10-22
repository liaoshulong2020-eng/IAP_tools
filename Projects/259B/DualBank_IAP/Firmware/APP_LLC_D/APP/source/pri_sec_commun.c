#include "main.h"
#include "pri_sec_commun.h"
#include "uart_app.h"
#include "can_app.h"
#include <string.h>

extern volatile uint8_t uart_rx_buf[PFC_UART_FRAME_LENGTH];

#define CMD_LLC_VBUS_TARGET     0x11

typedef struct {
    uint8_t header;
    uint8_t command;
    union {
        float float_val;
        uint8_t bytes[4];
        uint32_t uint_val;
    } data;
    uint8_t checksum;
    uint8_t tail;
} __attribute__((packed)) comm_frame_8byte_t;

float llc_data_array[DATA_ARRAY_SIZE] = {0};
PFC_RECEIVED_DATA_TypeDef pfc_received_data = {0};
float pfc_vout_uvp_point = 0.0f;
float pfc_vout_uvp_recovery = 0.0f;
volatile uint8_t pfc_uart_data_valid = 0U;
volatile uint8_t pfc_uart_protocol_version = 0U;
volatile uint8_t pfc_uart_report_sequence = 0U;
static comm_frame_8byte_t llc_vbus_target_frame;

static uint8_t calculate_crc8(uint8_t *data, uint16_t len)
{
    uint8_t crc = 0x00;

    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

static uint8_t calculate_xor_checksum(const uint8_t *data, uint16_t len)
{
    uint8_t checksum = 0;

    for (uint16_t i = 0; i < len; i++) {
        checksum ^= data[i];
    }

    return checksum;
}

static void build_8byte_frame(comm_frame_8byte_t *frame, uint8_t cmd, float value)
{
    frame->header = COMM_FRAME_HEADER;
    frame->command = cmd;
    frame->data.float_val = value;
    frame->checksum = calculate_crc8(&frame->command, 5);
    frame->tail = COMM_FRAME_TAIL;
}

bool parse_pfc_data_from_buffer(void)
{
    uint8_t local_buf[PFC_UART_FRAME_LENGTH];
    uint8_t data_len;
    uint8_t received_checksum;
    uint8_t calculated_checksum;

    memcpy(local_buf, (const void*)uart_rx_buf, PFC_UART_FRAME_LENGTH);

    if (local_buf[0] != PFC_UART_FRAME_HEADER) {
        return false;
    }

    if (local_buf[1] != PFC_UART_CMD_DETAIL_INFO &&
        local_buf[1] != PFC_UART_CMD_PROTECT_EXT) {
        return false;
    }

    data_len = local_buf[2];
    if (data_len != sizeof(PFC_RECEIVED_DATA_TypeDef)) {
        return false;
    }

    if (local_buf[PFC_UART_FRAME_LENGTH - 1] != PFC_UART_FRAME_TAIL) {
        return false;
    }

    received_checksum = local_buf[PFC_UART_CHECKSUM_INDEX];
    calculated_checksum = calculate_xor_checksum(&local_buf[PFC_UART_CHECKSUM_OFFSET], PFC_UART_CHECKSUM_LENGTH);
    if (received_checksum != calculated_checksum) {
        return false;
    }

    if (local_buf[1] == PFC_UART_CMD_DETAIL_INFO) {
        memcpy(&pfc_received_data, &local_buf[PFC_UART_DATA_OFFSET], sizeof(PFC_RECEIVED_DATA_TypeDef));
        pfc_uart_data_valid = 1U;
    } else {
        float_union_t value;
        pfc_uart_protocol_version = local_buf[PFC_UART_DATA_OFFSET];
        pfc_uart_report_sequence = local_buf[PFC_UART_DATA_OFFSET + 1U];
        memcpy(value.b, &local_buf[PFC_UART_DATA_OFFSET + 2U], sizeof(value.b));
        pfc_vout_uvp_point = value.f;
        memcpy(value.b, &local_buf[PFC_UART_DATA_OFFSET + 6U], sizeof(value.b));
        pfc_vout_uvp_recovery = value.f;
    }
    return true;
}

static LL_StatusETypeDef uart_send_llc_vbus_target(void)
{
    llc_data_array[DATA_VBUS_TARGET] = 390.0f;
    build_8byte_frame(&llc_vbus_target_frame, CMD_LLC_VBUS_TARGET, llc_data_array[DATA_VBUS_TARGET]);
    uart_send_u8data((uint8_t*)&llc_vbus_target_frame);

    return LL_OK;
}

void uart_send_info(void)
{
    uart_send_llc_vbus_target();
}

void User_Uart_RxCpltCallback(void)
{
}

void User_Uart_TxCpltCallback(void)
{
}
