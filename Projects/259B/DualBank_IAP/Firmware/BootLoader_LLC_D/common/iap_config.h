#ifndef ZHLD_IAP_CONFIG_H
#define ZHLD_IAP_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "iap_protocol.h"

typedef struct {
    uint32_t magic;
    uint16_t format_version;
    uint16_t record_size;
    uint32_t sequence;
    uint32_t iap_can_id;
    uint32_t app_can_id;
    uint32_t can_id_mask;
    uint32_t product_type;
    uint32_t flags;
    uint32_t active_bank;
    uint32_t pending_bank;
    uint32_t image_state;
    uint32_t boot_attempts;
    uint32_t stage1_version;
    uint32_t app_version;
    uint32_t stage1_crc32;
    uint32_t app_crc32;
    uint32_t crc32;
    uint64_t commit;
} iap_config_t;

/* Stored in flags so the existing configuration record layout remains
 * compatible. Zero keeps the bootloader's legacy/default pin mapping. */
#define IAP_CONFIG_UART_PINMAP_MASK UINT32_C(0x00000003)
typedef enum {
    IAP_UART_PINMAP_DEFAULT = 0,
    IAP_UART0_PA9_PA10 = 1,
    IAP_UART0_PB9_PB10 = 2,
    IAP_UART0_PB6_PB7 = 3
} iap_uart_pinmap_t;

typedef struct {
    bool (*erase_sector)(uint32_t address);
    bool (*program)(uint32_t address, const void *data, size_t size);
    bool (*read)(uint32_t address, void *data, size_t size);
} iap_flash_ops_t;

uint32_t iap_crc32(const void *data, size_t size);
bool iap_can_id_valid(uint32_t can_id);
bool iap_uart_pinmap_valid(uint32_t pinmap);
bool iap_config_valid(const iap_config_t *record);
void iap_config_defaults(iap_config_t *record, uint32_t fallback_iap_id);
bool iap_config_load(const iap_flash_ops_t *ops, iap_config_t *record,
                     uint32_t address_a, uint32_t address_b);
bool iap_config_store(const iap_flash_ops_t *ops, iap_config_t *record,
                      uint32_t address_a, uint32_t address_b);
bool iap_config_set_address(const iap_flash_ops_t *ops, uint32_t new_iap_id,
                            iap_config_t *record, uint32_t address_a, uint32_t address_b);
bool iap_accept_can_id(uint32_t rx_id, const iap_config_t *record);

#endif
