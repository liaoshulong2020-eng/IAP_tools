#include "iap_config.h"
#include <string.h>
#include <stddef.h>

static bool read_record(const iap_flash_ops_t *ops, uint32_t address, iap_config_t *out)
{
    return ops && ops->read && ops->read(address, out, sizeof(*out));
}

uint32_t iap_crc32(const void *data, size_t size)
{
    const uint8_t *p = (const uint8_t *)data;
    uint32_t crc = UINT32_C(0xFFFFFFFF);
    size_t i;
    unsigned bit;
    for (i = 0; i < size; ++i) {
        crc ^= p[i];
        for (bit = 0; bit < 8; ++bit) {
            crc = (crc >> 1) ^ (UINT32_C(0xEDB88320) & (uint32_t)-(int32_t)(crc & 1U));
        }
    }
    return crc ^ UINT32_C(0xFFFFFFFF);
}

bool iap_can_id_valid(uint32_t can_id)
{
    return can_id > 0U && can_id <= IAP_CAN_EXT_ID_MAX &&
           can_id != IAP_DISCOVERY_CAN_ID;
}

bool iap_config_valid(const iap_config_t *r)
{
    uint32_t crc;
    if (!r || r->magic != IAP_CONFIG_MAGIC ||
        r->format_version != IAP_CONFIG_FORMAT ||
        r->record_size != sizeof(*r) || r->commit != IAP_CONFIG_COMMITTED ||
        !iap_can_id_valid(r->iap_can_id)) {
        return false;
    }
    crc = iap_crc32(r, offsetof(iap_config_t, crc32));
    return crc == r->crc32;
}

void iap_config_defaults(iap_config_t *r, uint32_t fallback_iap_id)
{
    memset(r, 0, sizeof(*r));
    r->magic = IAP_CONFIG_MAGIC;
    r->format_version = IAP_CONFIG_FORMAT;
    r->record_size = (uint16_t)sizeof(*r);
    r->sequence = 1U;
    r->iap_can_id = iap_can_id_valid(fallback_iap_id) ? fallback_iap_id : IAP_LEGACY_CAN_ID;
    r->can_id_mask = IAP_CAN_EXT_ID_MAX;
    r->active_bank = IAP_BANK_A;
    r->pending_bank = IAP_BANK_NONE;
    r->image_state = IAP_IMAGE_CONFIRMED;
    r->crc32 = iap_crc32(r, offsetof(iap_config_t, crc32));
    r->commit = IAP_CONFIG_COMMITTED;
}

bool iap_config_load(const iap_flash_ops_t *ops, iap_config_t *out,
                     uint32_t address_a, uint32_t address_b)
{
    iap_config_t a, b;
    bool va = read_record(ops, address_a, &a) && iap_config_valid(&a);
    bool vb = read_record(ops, address_b, &b) && iap_config_valid(&b);
    if (!va && !vb) return false;
    *out = (!vb || (va && (int32_t)(a.sequence - b.sequence) > 0)) ? a : b;
    return true;
}

bool iap_config_store(const iap_flash_ops_t *ops, iap_config_t *r,
                      uint32_t address_a, uint32_t address_b)
{
    iap_config_t current, verify;
    uint32_t target = address_a;
    uint64_t commit = IAP_CONFIG_COMMITTED;
    if (!ops || !ops->erase_sector || !ops->program || !ops->read || !r) return false;
    if (iap_config_load(ops, &current, address_a, address_b)) {
        iap_config_t a;
        bool a_is_current = read_record(ops, address_a, &a) && iap_config_valid(&a) &&
                            a.sequence == current.sequence;
        target = a_is_current ? address_b : address_a;
        r->sequence = current.sequence + 1U;
    } else if (r->sequence == 0U) {
        r->sequence = 1U;
    }
    r->magic = IAP_CONFIG_MAGIC;
    r->format_version = IAP_CONFIG_FORMAT;
    r->record_size = (uint16_t)sizeof(*r);
    r->commit = UINT64_MAX;
    r->crc32 = iap_crc32(r, offsetof(iap_config_t, crc32));
    if (!ops->erase_sector(target) ||
        !ops->program(target, r, offsetof(iap_config_t, commit)) ||
        !ops->program(target + offsetof(iap_config_t, commit), &commit, sizeof(commit)) ||
        !ops->read(target, &verify, sizeof(verify)) || !iap_config_valid(&verify)) {
        return false;
    }
    *r = verify;
    return true;
}

bool iap_config_set_address(const iap_flash_ops_t *ops, uint32_t new_iap_id,
                            iap_config_t *record, uint32_t address_a, uint32_t address_b)
{
    if (!record || !iap_can_id_valid(new_iap_id)) return false;
    record->iap_can_id = new_iap_id;
    return iap_config_store(ops, record, address_a, address_b);
}

bool iap_accept_can_id(uint32_t rx_id, const iap_config_t *record)
{
    return rx_id == IAP_LEGACY_CAN_ID || rx_id == IAP_DISCOVERY_CAN_ID ||
           (record && iap_config_valid(record) && rx_id == record->iap_can_id);
}
