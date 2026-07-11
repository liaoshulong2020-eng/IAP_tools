#ifndef __PR_CONTROL_APP_H_
#define __PR_CONTROL_APP_H_
#include "main.h"

// 定义PR控制器结构体
typedef struct {
    float kp;       // 比例增益
    float kr;       // 谐振增益
    float omega;    // 谐振频率
    float T;        // 采样周期
    float x1, x2;   // 延迟元素
    float y1, y2;   // 输出延迟元素
	
	  float Coff_a0;
	  float Coff_a2;
	  float Coff_b1;
	  float Coff_b2;
	
	  float R_Out;
	  float R_PreOut;
	  float R_PrePreOut;
	  float R_Pre_Err;
	  float R_PrePre_Err;
	  
} PRController;


extern PRController Volt_Rctrl;
void initPRController(PRController* c) ;
float updatePRController(PRController* c, float error);

extern void initPRController(PRController *c)  ;
extern float updatePRController(PRController* c, float error);
extern void llc_R_vloop(PRController* c,float error);
extern void llc_R_vrest(PRController* c);

#endif
