#ifndef ZHLD_DUALBANK_LAYOUT_H
#define ZHLD_DUALBANK_LAYOUT_H

#include <stdint.h>

#define DB_FLASH_BASE             UINT32_C(0x08000000)
#define DB_FLASH_SIZE             UINT32_C(0x00040000)
#define DB_BANK_SIZE              UINT32_C(0x00020000)
#define DB_SECTOR_SIZE            UINT32_C(0x00001000)

#define DB_BOOT_OFFSET            UINT32_C(0x00000000)
#define DB_BOOT_SIZE              UINT32_C(0x00008000)
#define DB_APP_OFFSET             UINT32_C(0x00008000)
#define DB_APP_SIZE               UINT32_C(0x00016000)
#define DB_META_OFFSET            UINT32_C(0x0001E000)
#define DB_CAL_OFFSET             UINT32_C(0x0001F000)

#define DB_BANK_A_BASE            (DB_FLASH_BASE)
#define DB_BANK_B_BASE            (DB_FLASH_BASE + DB_BANK_SIZE)
#define DB_BANK_A_BOOT            (DB_BANK_A_BASE + DB_BOOT_OFFSET)
#define DB_BANK_B_BOOT            (DB_BANK_B_BASE + DB_BOOT_OFFSET)
#define DB_BANK_A_APP             (DB_BANK_A_BASE + DB_APP_OFFSET)
#define DB_BANK_B_APP             (DB_BANK_B_BASE + DB_APP_OFFSET)
#define DB_BANK_A_META            (DB_BANK_A_BASE + DB_META_OFFSET)
#define DB_BANK_B_META            (DB_BANK_B_BASE + DB_META_OFFSET)
#define DB_BANK_A_CAL             (DB_BANK_A_BASE + DB_CAL_OFFSET)
#define DB_BANK_B_CAL             (DB_BANK_B_BASE + DB_CAL_OFFSET)

#if DB_BOOT_SIZE != DB_APP_OFFSET
#error Boot layout mismatch
#endif
#if (DB_APP_OFFSET + DB_APP_SIZE) != DB_META_OFFSET
#error APP layout mismatch
#endif

#endif
