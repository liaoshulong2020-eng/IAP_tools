#include "iap_runtime.h"
#include "tae32_iap_port.h"

static iap_config_t g_iap_config;

bool iap_runtime_init(uint32_t product_default_id)
{
    if (iap_config_load(tae32_iap_flash_ops(), &g_iap_config,
                        TAE32_IAP_META_A, TAE32_IAP_META_B)) return true;
    iap_config_defaults(&g_iap_config, product_default_id);
    /* A factory-new device still works if the first persistent write fails. */
    return iap_config_store(tae32_iap_flash_ops(), &g_iap_config,
                            TAE32_IAP_META_A, TAE32_IAP_META_B);
}

const iap_config_t *iap_runtime_config(void) { return &g_iap_config; }
uint32_t iap_runtime_can_id(void) { return g_iap_config.iap_can_id; }
bool iap_runtime_accept_id(uint32_t rx_id) { return iap_accept_can_id(rx_id, &g_iap_config); }

bool iap_runtime_change_address(uint32_t new_id)
{
    return iap_config_set_address(tae32_iap_flash_ops(), new_id, &g_iap_config,
                                  TAE32_IAP_META_A, TAE32_IAP_META_B);
}

bool iap_runtime_save_bank_state(uint32_t active_bank, uint32_t pending_bank,
                                 uint32_t image_state, uint32_t boot_attempts)
{
    if (active_bank > IAP_BANK_B ||
        (pending_bank > IAP_BANK_B && pending_bank != IAP_BANK_NONE)) return false;
    g_iap_config.active_bank = active_bank;
    g_iap_config.pending_bank = pending_bank;
    g_iap_config.image_state = image_state;
    g_iap_config.boot_attempts = boot_attempts;
    return iap_config_store(tae32_iap_flash_ops(), &g_iap_config,
                            TAE32_IAP_META_A, TAE32_IAP_META_B);
}
