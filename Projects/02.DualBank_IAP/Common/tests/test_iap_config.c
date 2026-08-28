#include "iap_config.h"
#include "dualbank_boot.h"
#include "dualbank_layout.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned char flash_mem[DB_FLASH_SIZE];
static bool erase_ok(uint32_t a) { memset(flash_mem + a - DB_FLASH_BASE, 0xff, DB_SECTOR_SIZE); return true; }
static bool program_ok(uint32_t a, const void *p, size_t n) {
    size_t i, off = a - DB_FLASH_BASE; const unsigned char *s = p;
    for (i = 0; i < n; ++i) { if ((flash_mem[off+i] & s[i]) != s[i]) return false; flash_mem[off+i] &= s[i]; }
    return true;
}
static bool read_ok(uint32_t a, void *p, size_t n) { memcpy(p, flash_mem + a - DB_FLASH_BASE, n); return true; }
static bool image_ok(iap_bank_t b, uint32_t crc) { return b <= IAP_BANK_B && crc == 0x12345678U; }

int main(void)
{
    const iap_flash_ops_t f = { erase_ok, program_ok, read_ok };
    const dualbank_boot_ops_t b = { image_ok, 0 };
    iap_config_t c, loaded;
    memset(flash_mem, 0xff, sizeof(flash_mem));
    iap_config_defaults(&c, 0xA0003U);
    assert(iap_config_store(&f, &c, DB_BANK_A_META, DB_BANK_B_META));
    assert(iap_config_load(&f, &loaded, DB_BANK_A_META, DB_BANK_B_META));
    assert(loaded.iap_can_id == 0xA0003U);
    assert(iap_config_set_address(&f, 0xB0008U, &loaded, DB_BANK_A_META, DB_BANK_B_META));
    assert(iap_config_load(&f, &loaded, DB_BANK_A_META, DB_BANK_B_META));
    assert(loaded.iap_can_id == 0xB0008U);
    assert(iap_accept_can_id(0xB0008U, &loaded));
    assert(iap_accept_can_id(IAP_LEGACY_CAN_ID, &loaded));
    assert(iap_accept_can_id(IAP_DISCOVERY_CAN_ID, &loaded));
    assert(!iap_accept_can_id(0xA0003U, &loaded));
    assert(dualbank_mark_pending(&loaded, IAP_BANK_B, 2U, 0x12345678U));
    assert(dualbank_choose_boot(&loaded, &b) == IAP_BANK_B);
    dualbank_confirm_running(&loaded);
    assert(loaded.active_bank == IAP_BANK_B && loaded.image_state == IAP_IMAGE_CONFIRMED);
    puts("iap_config tests passed");
    return 0;
}
