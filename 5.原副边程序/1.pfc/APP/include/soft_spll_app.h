#ifndef __SOFT_SPLL_H__
#define __SOFT_SPLL_H__
#include "main.h"

#define PI 3.14159265358979323846
#define TWO_PI (2.0 * PI)
#define NOMINAL_FREQUENCY 50
#define SAMPLING_FREQUENCY 10000
#define SAMPLES_PER_CYCLE (SAMPLING_FREQUENCY / NOMINAL_FREQUENCY)
//void SPLL_Init(SPLL *spll, float alpha, float beta);
//void SPLL_Update(SPLL *spll, float sample);

// SPLL structure
typedef struct {
    float phase_angle;
    float omega;
    float sin_output;
    float cos_output;
    float phase_increment;
} SPLL;



extern SPLL spll;


#endif
