#ifndef _GPIO_APP_H
#define _GPIO_APP_H
#ifdef __cplusplus
 extern "C" {
#endif
#include "main.h"


void 	gpio_init_config(void);
void  gpio_init_app(void);
void 	gpio_output_init(void);
void 	gpio_input_init(void);
void  gpio_state_init(void);
void  breathing_light(void);
void pfc_ok_signal(void);
void pfc_nok_signal(void);
void  led_breath(void);
void relay_ctrl_enable(void);
void relay_ctrl_disable_low(void);

extern void 	gpio_init_config(void);
extern void 	gpio_init_app(void);
extern void 	gpio_output_init(void);
extern void 	gpio_input_init(void);
extern void   gpio_state_init(void);
extern void   breathing_light(void);
extern void 	pfc_ok_signal(void);
extern void 	pfc_nok_signal(void);
extern void   led_breath(void);
extern void relay_ctrl_enable(void);
extern void relay_ctrl_disable_low(void);
#ifdef __cplusplus
}
#endif

#endif 
/******************************** END OF FILE ********************/




