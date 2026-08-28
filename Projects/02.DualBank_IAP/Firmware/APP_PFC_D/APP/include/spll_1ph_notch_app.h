#ifndef _SPLL_1PH_NOTCH_APP_H
#define _SPLL_1PH_NOTCH_APP_H
#include "main.h"

#ifdef __cplusplus
 extern "C" {
#endif
#include "stdint.h"
#include "stdbool.h"
#ifdef __cplusplus
}
#endif
typedef struct{
  float b2;
  float b1;
  float b0;
  float a2;
  float a1;
} SPLL_1PH_NOTCH_COEFF;

typedef struct{
    int32_t   y_notch1;  //!< Notch filter1 data storage
    int32_t   theta;        //!< Angle output (0-2*pi)
    int16_t   cosine;       //!< Cosine value of the PLL angle
    int16_t   sine;         //!< Sine value of the PLL angle
		int16_t   cosine_prev;       //!< Cosine value of the PLL angle
    int16_t   sine_prev;         //!< Sine value of the PLL angle
		int32_t   wn;
		int32_t   spll_erro_sum;
		int32_t   dwn;
    SPLL_1PH_NOTCH_COEFF notch_coeff; //!< Notch filter coeffcient structure
} SPLL_1PH_NOTCH;


extern void SPLL_1PH_NOTCH_RUN(SPLL_1PH_NOTCH *spll_obj, int32_t acValue);
extern 	SPLL_1PH_NOTCH spll_1ph_notch;

#endif 

/******************************** END OF FILE ********************/











































