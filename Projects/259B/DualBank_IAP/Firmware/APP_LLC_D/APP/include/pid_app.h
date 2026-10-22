#ifndef __PID_APP_H_
#define __PID_APP_H_

#include "main.h"



void llc_PI_vloop(void);
void llc_PI_iloop(void);
void llc_PI_Shareloop(void);
void llc_loop_para_init(void);

extern void llc_PI_vloop(void);
extern void llc_PI_iloop(void);
extern void llc_PI_Shareloop(void);
extern void llc_loop_para_init(void);

#endif



