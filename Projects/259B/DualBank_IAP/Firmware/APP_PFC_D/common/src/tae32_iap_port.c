#include "tae32_iap_port.h"
#include "tae32g58xx_ll_eflash.h"
#include <string.h>

static bool port_erase(uint32_t address)
{
    uint32_t sector_size = LL_EFLASH_SectorSize_Get(EFLASH);
    uint32_t sector = (address - UINT32_C(0x08000000)) / sector_size;
    return sector_size == UINT32_C(0x1000) &&
           LL_EFLASH_EraseSector(EFLASH, sector) == LL_OK;
}

static bool port_program(uint32_t address, const void *data, size_t size)
{
    if ((address & 7U) != 0U || (size & 7U) != 0U) return false;
    return LL_EFLASH_Program(EFLASH, address, (uint8_t *)(uintptr_t)data,
                             (uint32_t)size) == size &&
           LL_EFLASH_Verify(EFLASH, address, (uint8_t *)(uintptr_t)data,
                            (uint32_t)size) == size;
}

static bool port_read(uint32_t address, void *data, size_t size)
{
    memcpy(data, (const void *)(uintptr_t)address, size);
    return true;
}

const iap_flash_ops_t *tae32_iap_flash_ops(void)
{
    static const iap_flash_ops_t ops = { port_erase, port_program, port_read };
    return &ops;
}
