#ifndef _ADC_APP_H
#define _ADC_APP_H
#ifdef __cplusplus
 extern "C" {
#endif
#include "main.h"

void adc_init_app(void);
void get_adc_value(void);
void check_current_zero_offset(void);
	
extern void adc_init_app(void);
extern void get_adc_value(void);
extern void check_current_zero_offset(void);
#ifdef __cplusplus
}
#endif

#endif 
/******************************** END OF FILE ********************/





