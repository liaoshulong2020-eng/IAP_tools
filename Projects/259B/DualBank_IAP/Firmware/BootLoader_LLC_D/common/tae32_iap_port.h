#ifndef ZHLD_TAE32_IAP_PORT_H
#define ZHLD_TAE32_IAP_PORT_H

#include "iap_config.h"

/* Uses the TAE32G58xx LL EFLASH driver already present in each project. */
const iap_flash_ops_t *tae32_iap_flash_ops(void);

/* These addresses are physical addresses while Bank0 is mapped first. */
#define TAE32_IAP_META_A UINT32_C(0x0801E000)
#define TAE32_IAP_META_B UINT32_C(0x0803E000)

#endif
