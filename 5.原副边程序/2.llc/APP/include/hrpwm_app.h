#ifndef __HRPWM__APP_H_
#define __HRPWM__APP_H_

#include "main.h"
void hrpwm_data_init(void);
void user_hrpwm_config(HRPWM_TypeDef* Instance);
void hrpwm_init_app(void);
void hrpwm_start_count(void);
void hrpwm_outdis_app(void);
void hrpwm_outen_app(void);
void hrpwm_sren_app(void);
void hrpwm_srdis_app(void);
void hrpwm_update_app(void);

extern void hrpwm_data_init(void);
extern void user_hrpwm_config(HRPWM_TypeDef* Instance);
extern void hrpwm_init_app(void);
extern void hrpwm_start_count(void);
extern void hrpwm_outdis_app(void);
extern void hrpwm_outen_app(void);
extern void hrpwm_sren_app(void);
extern void hrpwm_srdis_app(void);
extern void hrpwm_update_app(void);


#endif


