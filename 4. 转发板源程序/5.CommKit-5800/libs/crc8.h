#ifndef CRC8_H
#define CRC8_H

#include "main.h"

/*
 * 静态查表法：CRC8多项式 ATM: x^8+x^2+x^1+1 ----- 0x07
 */

/*
 * 生成CRC校验码
 */
void crc8_update(uchar *crc,const void *buff,uchar size);

uint8_t crc8(uint8_t* data, uint8_t len);

#endif // CRC8_H
