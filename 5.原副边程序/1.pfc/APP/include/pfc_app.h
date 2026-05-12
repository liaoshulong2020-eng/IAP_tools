#ifndef _PFC_APP_H
#define _PFC_APP_H
#ifdef __cplusplus
 extern "C" {
#endif
#include "main.h"
void rectify_vac(void);
void half_cycle_processing(void);
void pfc_state_idle(void);	
void vbus_filter();
void pfc_handle(void); //100k
void pfc_state_idle(void);	
void pfc_state_rampup(void);
void pfc_state_on(void);
void pfc_state_fault(void);
void state_switch(void);
extern void rectify_vac(void);
extern void half_cycle_processing(void);
void pfc_iloop_handle(void);
void K_forward_calcula();
void input_mode_check(void);
void set_protect_point(void);
void set_vbus_voltage(void);
#ifdef __cplusplus
}
#endif

#endif 
/******************************** END OF FILE ********************/




