#ifndef __INIT_APP_H_
#define __INIT_APP_H_

void flash_data_init(void);
void load_data_app(void);
void save_data_flash(void);

extern void flash_data_init(void);
extern void load_data_app(void);
extern void save_data_flash(void);

extern void init_all_app(void);
extern void gpio_init_config(void);
extern void hrpwm_init_app_config(void);
extern void adc_init_config_app(void);

extern void hrpwm_outen_app(void);
extern void hrpwm_outdis_app(void);
extern void tmr_app_init(void);
extern void hrpwm_start_count(void);
extern void reset_value(void);
#endif

