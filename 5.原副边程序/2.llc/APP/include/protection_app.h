#ifndef __PROTECTION_APP_H__
#define __PROTECTION_APP_H__
#include "main.h"
void over_output_voltage_check_app(void);
void under_output_voltage_check_app(void);
void over_current_check_app(void);
void over_tamp_check_app(void);
void fault_check_app(void);
int no_fault(void);
void clear_fault_state(void);

extern void over_output_voltage_check_app(void);
extern void under_output_voltage_check_app(void);
extern void over_current_check_app(void);
extern void over_tamp_check_app(void);
extern void fault_check_app(void);
extern int no_fault(void);
extern void clear_fault_state(void);
#endif

