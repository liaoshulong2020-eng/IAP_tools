#ifndef PRI_SEC_COMMUN_H
#define PRI_SEC_COMMUN_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "main.h"

#define COMM_FRAME_HEADER    0xAA
#define COMM_FRAME_TAIL      0x55
#define COMM_FRAME_LENGTH    8

#define DATA_VBUS_TARGET    0
#define DATA_ARRAY_SIZE     8

typedef union {
    float f;
    uint32_t u32;
    uint8_t b[4];
} float_union_t;

typedef union {
    uint16_t all;
    struct {
        uint16_t input_ok           : 1;
        uint16_t input_under_v      : 1;
        uint16_t input_over_v       : 1;
        uint16_t output_under_v     : 1;
        uint16_t output_over_v      : 1;
        uint16_t output_over_i      : 1;
        uint16_t pfc_normal         : 1;
        uint16_t pre_finish         : 1;
        uint16_t start_delay        : 1;
        uint16_t current_calib      : 1;
        uint16_t protect_set        : 1;
        uint16_t input_mode_dc      : 1;
        uint16_t reserved           : 4;
    } bits;
} status_flags_union_t;

typedef struct {
    float_union_t vbus_target;
    float_union_t vbus_ref;
    float_union_t vbus_rel;
    float_union_t iloop_rel;
    float_union_t vin_rel;
    int16_t r_ntc_raw;
    float_union_t vin_on_voltage_set;
    float_union_t vin_under_voltage_set;
    float_union_t vin_over_voltage_set;
    float_union_t vin_max_voltage_set;
    float_union_t vout_over_voltage_sw;
    float_union_t bus_ovp_point_hw;
    float_union_t ipfc_ocp_current_sw;
    float_union_t pfc_i_ocp_dac_point_hw;
    uint8_t state;
    uint8_t switch_frequency;
    float_union_t duty_cycle;
    status_flags_union_t status_flags;
} __attribute__((packed)) PFC_RECEIVED_DATA_TypeDef;

#define PFC_UART_FRAME_HEADER       0x55
#define PFC_UART_FRAME_TAIL         0xAA
#define PFC_UART_CMD_DETAIL_INFO    0x02
#define PFC_UART_DATA_OFFSET        3
#define PFC_UART_FRAME_OVERHEAD     5
#define PFC_UART_FRAME_LENGTH       (sizeof(PFC_RECEIVED_DATA_TypeDef) + PFC_UART_FRAME_OVERHEAD)
#define PFC_UART_CHECKSUM_OFFSET    1
#define PFC_UART_CHECKSUM_INDEX     (PFC_UART_DATA_OFFSET + sizeof(PFC_RECEIVED_DATA_TypeDef))
#define PFC_UART_CHECKSUM_LENGTH    (2 + sizeof(PFC_RECEIVED_DATA_TypeDef))

extern float llc_data_array[DATA_ARRAY_SIZE];
extern PFC_RECEIVED_DATA_TypeDef pfc_received_data;

bool parse_pfc_data_from_buffer(void);
void uart_send_info(void);

void User_Uart_RxCpltCallback(void);
void User_Uart_TxCpltCallback(void);

#endif
