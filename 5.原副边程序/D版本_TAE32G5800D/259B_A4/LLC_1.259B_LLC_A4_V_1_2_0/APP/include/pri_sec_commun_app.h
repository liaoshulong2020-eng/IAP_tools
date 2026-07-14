#ifndef PRI_SEC_COMMUN_APP_H
#define PRI_SEC_COMMUN_APP_H

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

#define PFC_COMM_UART                 UART0
#define PFC_COMM_UART_BAUDRATE        115200

#define PFC_COMM_FRAME_HEADER         0x55
#define PFC_COMM_FRAME_TAIL           0xAA
#define PFC_COMM_CMD_DETAIL_INFO      0x02

#define PFC_IAP_TARGET_ADDR           1
#define LLC_IAP_TARGET_ADDR           2
#define PFC_IAP_FNO                   0x41
#define PFC_IAP_CAN_ID                0xAA55
#define PFC_IAP_MAX_PACKET_SIZE       300

#define CMD_PFC_INPUT_OVP             0x30
#define CMD_PFC_INPUT_UVP             0x31
#define CMD_PFC_OUTPUT_OVP            0x32
#define CMD_PFC_OUTPUT_UVP            0x33
#define CMD_PFC_INPUT_OCP             0x34
#define CMD_PFC_DATA                  0x35
#define CMD_PFC_DATA_LIVE1            0x36
#define CMD_PFC_DATA_LIVE2            0x37

#define RETURN_BIT_PFC_INPUT_OVP      0x87
#define RETURN_BIT_PFC_INPUT_UVP      0x88
#define RETURN_BIT_PFC_OUTPUT_OVP     0x89
#define RETURN_BIT_PFC_OUTPUT_UVP     0x8A
#define RETURN_BIT_PFC_INPUT_OCP      0x8B
#define RETURN_BIT_PFC_DATA           0x8C
#define RETURN_BIT_PFC_LIVE1          0x8D
#define RETURN_BIT_PFC_LIVE2          0x8E

typedef union {
    float f;
    uint32_t u32;
    uint8_t b[4];
} pfc_float_union_t;

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
} pfc_status_flags_union_t;

typedef struct {
    pfc_float_union_t vbus_target;
    pfc_float_union_t vbus_rel;
    pfc_float_union_t iloop_rel;
    pfc_float_union_t vin_rel;
    int16_t r_ntc_raw;
    pfc_float_union_t vin_on_voltage_set;
    pfc_float_union_t vin_under_voltage_set;
    pfc_float_union_t vin_over_voltage_set;
    pfc_float_union_t vin_max_voltage_set;
    pfc_float_union_t vout_over_voltage_sw;
    pfc_float_union_t bus_ovp_point_hw;
    pfc_float_union_t ipfc_ocp_current_sw;
    pfc_float_union_t pfc_i_ocp_dac_point_hw;
    uint8_t state;
    uint8_t switch_frequency;
    pfc_float_union_t duty_cycle;
    pfc_status_flags_union_t status_flags;
} __attribute__((packed)) PFC_REPORT_DATA_TypeDef;

#define PFC_COMM_FRAME_DATA_SIZE       sizeof(PFC_REPORT_DATA_TypeDef)
#define PFC_COMM_FRAME_OVERHEAD        5
#define PFC_COMM_FRAME_TOTAL_SIZE      (PFC_COMM_FRAME_DATA_SIZE + PFC_COMM_FRAME_OVERHEAD)
#define PFC_COMM_CHECKSUM_OFFSET       (3 + PFC_COMM_FRAME_DATA_SIZE)
#define PFC_COMM_TAIL_OFFSET           (PFC_COMM_CHECKSUM_OFFSET + 1)

void pfc_comm_init(void);
void pfc_comm_deinit(void);
bool pfc_comm_is_ready(void);
bool pfc_comm_fill_can_response(uint8_t query_cmd, uint8_t out_frame[8]);
void pfc_iap_forward_start(void);
bool pfc_iap_forward_is_active(void);
void pfc_iap_forward_can_frame(const uint8_t *data, uint16_t len);
void pfc_iap_forward_task(void);
void User_PfcComm_Uart_RxCpltCallback(void);
void User_PfcComm_Uart_TxCpltCallback(void);

#endif
