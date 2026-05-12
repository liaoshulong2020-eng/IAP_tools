#ifndef _LLC_APP_H
#define _LLC_APP_H

#include "main.h"

void llc_handle(void);
void llc_state_idle(void);
void llc_state_rampup(void);
void llc_state_on(void);
void llc_state_fault(void);
void llc_loop_ctrl(void);
void  llc_loop_output_ctrl(void);
void  llc_loop_switch_ctrl(void);
void  llc_pwm_value_ctrl(void);
void llc_loop_init(void);
void vout_config(void);
void ShareCurrDutySet(void);
void ShareCurrDutySet_Max(void);
void vout_over_protect(void);
void ShareCurrDutySet_FullLoad(void);
#endif 
/******************************** END OF FILE ********************/




