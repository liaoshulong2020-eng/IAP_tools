#ifndef ZHLD_IAP_PROTOCOL_H
#define ZHLD_IAP_PROTOCOL_H

#include <stdint.h>

#define IAP_LEGACY_CAN_ID         UINT32_C(0x0000AA55)
#define IAP_DISCOVERY_CAN_ID      UINT32_C(0x18FF50E5)
#define IAP_CAN_EXT_ID_MAX        UINT32_C(0x1FFFFFFF)
#define IAP_CONFIG_MAGIC          UINT32_C(0x49415043) /* IAPC */
#define IAP_CONFIG_FORMAT         UINT16_C(1)
#define IAP_CONFIG_COMMITTED      UINT64_C(0x434F4D4D49545445) /* COMMITTE */

enum {
    IAP_CMD_DISCOVER       = 0x50,
    IAP_CMD_READ_CONFIG    = 0x51,
    IAP_CMD_WRITE_CONFIG   = 0x52,
    IAP_CMD_CONFIG_ACK     = 0x53,
    IAP_CMD_REBOOT_TO_IAP  = 0x54
};

typedef enum {
    IAP_BANK_A = 0,
    IAP_BANK_B = 1,
    IAP_BANK_NONE = 0xFF
} iap_bank_t;

typedef enum {
    IAP_IMAGE_EMPTY = 0,
    IAP_IMAGE_WRITING,
    IAP_IMAGE_READY,
    IAP_IMAGE_PENDING,
    IAP_IMAGE_TRIAL,
    IAP_IMAGE_CONFIRMED,
    IAP_IMAGE_BAD
} iap_image_state_t;

/* The UID must match before a remote address write is accepted. */
typedef struct {
    uint8_t command;
    uint8_t protocol_version;
    uint16_t reserved;
    uint32_t uid[3];
    uint32_t new_iap_can_id;
    uint32_t nonce;
    uint32_t crc32;
} iap_address_command_t;

#endif
