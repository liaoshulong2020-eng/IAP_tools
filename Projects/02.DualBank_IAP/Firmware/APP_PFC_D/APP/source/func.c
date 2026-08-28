#include "main.h"
#include "func.h"



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
