#ifndef _HRPWM_APP_H
#define _HRPWM_APP_H
#ifdef __cplusplus
 extern "C" {
#endif
#include "main.h"
void hrpwm_data_init(void);
void user_hrpwm_config(HRPWM_TypeDef* Instance);
void hrpwm_init_app(void);
void hrpwm_update_app(void);
void hrpwm_pre_outdis();
void hrpwm_pre_outen();
extern void hrpwm_data_init(void);
extern void user_hrpwm_config(HRPWM_TypeDef* Instance);
extern void hrpwm_init_app(void);
extern void hrpwm_update_app(void);
extern void hrpwm_pre_outdis();
extern void hrpwm_pre_outen();
#ifdef __cplusplus
}
#endif

#endif 
/******************************** END OF FILE ********************/





