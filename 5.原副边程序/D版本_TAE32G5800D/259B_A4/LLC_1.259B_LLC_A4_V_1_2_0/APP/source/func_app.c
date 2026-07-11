#include "main.h"
#include "func_app.h"



RAMCODE
float abs_f(float num) {
    if (num < 0) {
        return -num;
    } else {
        return num;
    }
}
//RAMCODE
//float abs_f(float num) {
//    uint32_t *i = (uint32_t*)&num;
//    *i &= 0x7fffffff;
//    return num;
//}
#define CRC8_POLYNOMIAL 0x07

uint8_t crc8(uint8_t* data, uint8_t len)
{
    uint8_t crc = 0;
    uint8_t i;

    while(len--)
    {
        crc ^= *data++;

        for(i = 0; i < 8; i++)
        {
            if(crc & 0x80)
            {
                crc = (crc << 1) ^ CRC8_POLYNOMIAL;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}


