#ifndef __INIT_APP_H_
#define __INIT_APP_H_


extern void init_all_app(void);
extern void gpio_init_app(void);
extern void hrpwm_init_app_config(void);
extern void adc_init_config_app(void);

extern void hrpwm_outen_app(void);
extern void hrpwm_outdis_app(void);
extern void tmr_app_init(void);
extern void hrpwm_start_count(void);
#endif

