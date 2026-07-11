#ifndef __PID__H_
#define __PID__H_

#include "main.h"
#include "variables_define_app.h"

void pfc_PI_vloop(void);
void pfc_PI_iloop(void);
void pfc_vloop_init(void);
void pfc_iloop_init(void);
void BusOverVoltPWMDisable();
void current_pid_calculation_app(void);

extern void pfc_PI_vloop(void);
extern void pfc_PI_iloop(void);
extern void pfc_vloop_init(void);
extern void pfc_iloop_init(void);

#endif



