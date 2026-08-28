#include "dualbank_boot.h"

iap_bank_t dualbank_choose_boot(iap_config_t *c, const dualbank_boot_ops_t *ops)
{
    iap_bank_t candidate;
    if (!c || !ops || !ops->image_valid) return IAP_BANK_NONE;
    candidate = (iap_bank_t)c->active_bank;
    if (c->pending_bank <= IAP_BANK_B) {
        candidate = (iap_bank_t)c->pending_bank;
        if (!ops->image_valid(candidate, c->app_crc32)) {
            c->image_state = IAP_IMAGE_BAD;
            c->pending_bank = IAP_BANK_NONE;
            return (iap_bank_t)c->active_bank;
        }
        c->image_state = IAP_IMAGE_TRIAL;
        c->boot_attempts++;
        if (c->boot_attempts > DB_MAX_TRIAL_BOOTS) {
            c->image_state = IAP_IMAGE_BAD;
            c->pending_bank = IAP_BANK_NONE;
            return (iap_bank_t)c->active_bank;
        }
    }
    return candidate;
}

bool dualbank_mark_pending(iap_config_t *c, iap_bank_t bank,
                           uint32_t app_version, uint32_t app_crc32)
{
    if (!c || bank > IAP_BANK_B || bank == c->active_bank) return false;
    c->pending_bank = bank;
    c->app_version = app_version;
    c->app_crc32 = app_crc32;
    c->boot_attempts = 0U;
    c->image_state = IAP_IMAGE_PENDING;
    return true;
}

void dualbank_confirm_running(iap_config_t *c)
{
    if (!c || c->pending_bank > IAP_BANK_B) return;
    c->active_bank = c->pending_bank;
    c->pending_bank = IAP_BANK_NONE;
    c->boot_attempts = 0U;
    c->image_state = IAP_IMAGE_CONFIRMED;
}
