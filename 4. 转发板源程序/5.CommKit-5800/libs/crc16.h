/*
 * crc16.h
 *
 *  Created on: 2024年3月21日
 *      Author: Liang Jinfeng
 */

#ifndef CRC16_H
#define CRC16_H

#include "main.h"

/*
 * 静态查表法：CRC-16/CCITT，多项式：x16 + x12 + x5 + 1 ----- 0x1021
 */

/*
 * 生成CRC校验码
 */
void crc16_update(ushort *crc,const void *buff,ushort size);

#endif // CRC16_H
