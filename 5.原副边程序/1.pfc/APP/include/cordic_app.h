#ifndef _CORDIC_APP_H
#define _CORDIC_APP_H
#ifdef __cplusplus
 extern "C" {
#endif
#include "main.h"

void cordic_init_app();
void cordic_config_app();
void User_CORDIC_TestLoop();
void cordic_calculate();
extern void cordic_init_app();
extern void cordic_config_app();
extern void User_CORDIC_TestLoop();
extern void cordic_calculate();
extern int32_t cordic_input[1],cordic_output[1];
extern int32_t out_buf[1];

#ifdef __cplusplus
}
#endif

#endif 
/******************************** END OF FILE ********************/


