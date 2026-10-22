#include "main.h"


RAMCODE
float FastInverseSqrt(float number) {
    float x = number;
    float xhalf = 0.5f * x;
    int32_t i = *(int32_t*)&x;            // get bits for floating value
    i = 0x5f3759df - (i >> 1);            // gives initial guess y0
    x = *(float*)&i;                      // convert bits back to float
    x = x * (1.5f - xhalf * x * x);       // Newton step, repeating increases accuracy
    return 1/x;
}


