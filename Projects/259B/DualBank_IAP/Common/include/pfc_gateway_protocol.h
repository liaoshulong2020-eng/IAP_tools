#ifndef PFC_GATEWAY_PROTOCOL_H
#define PFC_GATEWAY_PROTOCOL_H

#include <stdint.h>

#define IAP_TARGET_PFC             1U
#define IAP_TARGET_LLC             2U
#define IAP_FUNCTION_CODE          0x41U
#define IAP_MAX_PAYLOAD            256U
#define IAP_PACKET_OVERHEAD        14U
#define IAP_PROTOCOL_MAJOR         1U
#define IAP_PROTOCOL_MINOR         0U

enum iap_gateway_command {
    IAP_CMD_EXIT = 0x00,
    IAP_CMD_ENTER = 0x01,
    IAP_CMD_WRITE = 0x03,
    IAP_CMD_DUAL_BEGIN = 0x06,
    IAP_CMD_DUAL_VERIFY = 0x07,
    IAP_CMD_DUAL_COMMIT = 0x08,
    IAP_CMD_BANK_STATUS = 0x20,
    IAP_CMD_BANK_RESET = 0x25,
    IAP_CMD_CAPABILITY = 0x30
};

enum iap_gateway_error {
    IAP_GW_OK = 0,
    IAP_GW_BUSY = 0x0101,
    IAP_GW_CAN_CRC = 0x0102,
    IAP_GW_CAN_TIMEOUT = 0x0103,
    IAP_GW_PFC_NOT_READY = 0x0202,
    IAP_GW_UART_TIMEOUT = 0x0203,
    IAP_GW_UART_CRC = 0x0204,
    IAP_GW_UART_OVERFLOW = 0x0205,
    IAP_GW_PROTOCOL_MISMATCH = 0x0501
};

#endif
