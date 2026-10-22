#include <stdio.h>
#include "pr_control_app.h"

PRController Volt_Rctrl;

// ????Tustin¡À?????????PR?????¡Â
RAMCODE
void initPRController(PRController* c)
{
//  c->kp = kp;
//  c->kr = kr;
//  c->omega = omega;
//  c->T = T;
//  c->x1 = c->x2 = 0.0;
//  c->y1 = c->y2 = 0.0;
	
	c->Coff_a0 = 0.599958081166212;
	c->Coff_a2 = -0.599958081166212;
	c->Coff_b1 = -1.999840532724311;
	c->Coff_b2 = 0.999880008383767;
	
	c->R_PreOut = 0;
	c->R_PrePreOut = 0;
	c->R_Pre_Err = 0;
	c->R_PrePre_Err = 0;
	
}

// ?¨¹??PR?????¡Â¡Á???
float updatePRController(PRController* c, float error)
{
  float a = c->omega * c->T / 2.0;
  float b = c->kr / (1.0 + a);
  float y = b * (error - 2 * c->x1 + c->x2) + (2.0 + a) * c->y1 - c->y2;

  // ?¨¹??????????
  c->x2 = c->x1;
  c->x1 = error;
  c->y2 = c->y1;
  c->y1 = y;

  return y + c->kp * error; // ????¡Á?????
}


RAMCODE
void llc_R_vloop(PRController* c,float error)
{
		
	  c->R_Out =  c->Coff_a0*error + c->Coff_a2*c->R_PrePre_Err - c->Coff_b1*c->R_PreOut - c->Coff_b2*c->R_PrePreOut;
	  c->R_PrePre_Err = c->R_Pre_Err;
	  c->R_Pre_Err = error;
	  c->R_PrePreOut = c->R_PreOut;
	  c->R_PreOut = c->R_Out;
	
	
}

RAMCODE
void llc_R_vrest(PRController* c)
{
   	c->R_PreOut = 0;
	  c->R_PrePreOut = 0;
	  c->R_Pre_Err = 0;
	  c->R_PrePre_Err = 0;
}
