#ifndef ZHLD_DUALBANK_BOOT_H
#define ZHLD_DUALBANK_BOOT_H

#include <stdbool.h>
#include <stdint.h>
#include "iap_config.h"

#define DB_MAX_TRIAL_BOOTS 3U

typedef struct {
    bool (*image_valid)(iap_bank_t bank, uint32_t expected_crc);
    bool (*select_bank_and_reset)(iap_bank_t bank);
} dualbank_boot_ops_t;

iap_bank_t dualbank_choose_boot(iap_config_t *config,
                                const dualbank_boot_ops_t *ops);
bool dualbank_mark_pending(iap_config_t *config, iap_bank_t bank,
                           uint32_t app_version, uint32_t app_crc32);
void dualbank_confirm_running(iap_config_t *config);

#endif
