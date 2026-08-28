#ifndef ZHLD_IAP_RUNTIME_H
#define ZHLD_IAP_RUNTIME_H

#include "iap_config.h"

/* Shared by APP and Stage1. Falls back to 0xAA55 if both records are invalid. */
bool iap_runtime_init(uint32_t product_default_id);
const iap_config_t *iap_runtime_config(void);
uint32_t iap_runtime_can_id(void);
bool iap_runtime_accept_id(uint32_t rx_id);
bool iap_runtime_change_address(uint32_t new_id);

#endif
