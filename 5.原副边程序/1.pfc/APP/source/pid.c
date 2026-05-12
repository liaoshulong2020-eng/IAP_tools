#include "pid.h"

/**
  ******************************************************************************
  * @file    APP/pid_app.c
  * @author  004
  * @version V1.0.0
  * @date    17-04-2020
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 Tai-Micro</center></h2>
  *
  *
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "pid.h"
/**
* @function int16_t pid_calculation_app(TW_PID_TypeDef * pid)
* @brief    Description: app pid calculation
* @return   return pi_out
*/
RAMCODE
void pfc_PI_iloop()
{
	pfc.iloop.data_error = pfc.iloop.ref - pfc.iloop.rel;
  
	pfc.iloop.p_out = (pfc.iloop.data_error * pfc.iloop.kp);
	
	pfc.iloop.integral +=(pfc.iloop.data_error * pfc.iloop.ki);
	

#if FEED_FORWARD_ENABLE		//前馈
	
	if(pfc.iloop.integral > (pfc.iloop.integral_max))
    {
      pfc.iloop.integral = (pfc.iloop.integral_max);
    }
  else if(pfc.iloop.integral < (pfc.iloop.integral_min))
    {
      pfc.iloop.integral = pfc.iloop.integral_min;
    }
		
		pfc.iloop.loop_out = (pfc.iloop.p_out + pfc.iloop.integral) / 4.0f;
		
		if(pfc.iloop.loop_out > (pfc.iloop.out_max_limit))
    {
      pfc.iloop.loop_out = (pfc.iloop.out_max_limit);
    }
  else if(pfc.iloop.loop_out < (pfc.iloop.out_min_limit))
    {
      pfc.iloop.loop_out = pfc.iloop.out_min_limit;
    }
		
		pfc.current_output = pfc.iloop.loop_out + pfc.k_forward;
		
		if(pfc.current_output > pfc.iloop.out_max)
		{
			pfc.current_output = pfc.iloop.out_max;
		}
		else if(pfc.current_output < pfc.iloop.out_min)
		{
			pfc.current_output = pfc.iloop.out_min;
		}
		
#else	
		if(pfc.iloop.integral > (pfc.iloop.integral_max)) // 1<<8 
    {
      pfc.iloop.integral = (pfc.iloop.integral_max); // 
    }
  else if(pfc.iloop.integral < (pfc.iloop.integral_min))//- 1<<8
    {
      pfc.iloop.integral = pfc.iloop.integral_min;//
    }
		
		pfc.iloop.loop_out = (pfc.iloop.p_out + pfc.iloop.integral) / 8.0f ;
		
		if(pfc.iloop.loop_out > (pfc.iloop.out_max))
    {
      pfc.iloop.loop_out = (pfc.iloop.out_max);
    }
		else if(pfc.iloop.loop_out < (pfc.iloop.out_min))
    {
      pfc.iloop.loop_out = pfc.iloop.out_min;
    }
		
		pfc.current_output = pfc.iloop.loop_out;
		
#endif

}

//RAMCODE
//void pfc_PI_iloop(void)
//{
//	  uint32_t temp;

//		pfc.iloop.data_error = pfc.iloop.ref - pfc.iloop.rel;
//	

//	
//		pfc.iloop.loop_out += (pfc.iloop.k1  * pfc.iloop.data_error);								//Q27
//    pfc.iloop.loop_out += (pfc.iloop.k2  * pfc.iloop.data_error_n1);              //Q27	
//	
//    if(pfc.iloop.loop_out > pfc.iloop.out_max_limit)  																//Q15
//    {
//        pfc.iloop.loop_out = pfc.iloop.out_max_limit;
//    }
//    else if(pfc.iloop.loop_out < pfc.iloop.out_min_limit) 
//    {
//        pfc.iloop.loop_out = pfc.iloop.out_min_limit;

//    }
//		
//		pfc.iloop.data_error_n1 = pfc.iloop.data_error ;
//		

//		pfc.current_output = pfc.iloop.loop_out +  pfc.k_forward;
//		
//		 if(pfc.current_output > pfc.iloop.out_max)  																//Q15
//    {
//        pfc.current_output = pfc.iloop.out_max ;
//    }
//    else if(pfc.current_output < pfc.iloop.out_min)
//    {
//        pfc.current_output = pfc.iloop.out_min ;

//    }
//}


RAMCODE
void pfc_PI_vloop(void)
{
	const float ERROR_LOW = 1.0f;
	const float ERROR_HIGH = 40.0f;
	const float KP_LOW = 10.0f, KP_HIGH = 50.0f;
	const float KI_LOW = 0.05f, KI_HIGH = 1.0f;
	float error_abs, transition_factor;
	
  float  i_mul = 0.0f, i_out = 0.0f; //p_out,
	
	if(pfc.vloop.out_max < VLOOP_OUTPUT_MAX)
		{
			  pfc.vloop.out_max+= 0.2f;
		} 
		else
		{
				pfc.vloop.out_max = VLOOP_OUTPUT_MAX;
	
		}
  pfc.vloop.data_error = pfc.vbus_ref - pfc.vbus_rel;	//误差
		
	error_abs = abs_f(pfc.vloop.data_error);  // 使用 fabsf 代替自定义的 abs_f
		
	
		
		
 		
if (error_abs <= ERROR_LOW)
{
	transition_factor = 0.0f;
}
else if (error_abs >= ERROR_HIGH)
{
	transition_factor = 1.0f;
}
else
{
	transition_factor = (error_abs - ERROR_LOW) / (ERROR_HIGH - ERROR_LOW);
}

// 使用线性插值计算 kp 和 ki
//pfc.vloop.kp = (KP_LOW + transition_factor * (KP_HIGH - KP_LOW));
//pfc.vloop.ki = (KI_LOW + transition_factor * (KI_HIGH - KI_LOW));
pfc.vloop.kp = 35.0f;//20;//35;//30;
pfc.vloop.ki = 0.12f;//0.08;//0.15;//0.08;


//if (error_abs > 30)
//{
//	pfc.vloop.kp = 150;
//	pfc.vloop.ki = 1;
//}
//else if(error_abs > 20)
//{
//	pfc.vloop.kp = 100;
//	pfc.vloop.ki = 0.6;

//}else if(error_abs > 10)
//{
//	pfc.vloop.kp = 50;
//	pfc.vloop.ki = 0.5;
//}else if(error_abs > 5)
//{
//	pfc.vloop.kp = 30;
//	pfc.vloop.ki = 0.05;
//}
//else
//{
//	pfc.vloop.kp = 15;
//	pfc.vloop.ki = 0.001;
//}


//if (error_abs > 100)
//{
//	pfc.vloop.kp = 150;
//	pfc.vloop.ki = 1;
//}
//else if(error_abs < 5)
//{
//	if(pfc.vloop.kp > 15)
//	{
//		pfc.vloop.kp--;
//	}
//	else
//	{
//		pfc.vloop.kp  = 15;
//	}

//}




  pfc.vloop.p_out = (pfc.vloop.data_error * pfc.vloop.kp);		//计算p_out

  i_mul = (pfc.vloop.data_error * pfc.vloop.ki ) * (pfc.vloop.integral_en_k);	//计算 i_out

  i_out = i_mul + pfc.vloop.integral;					//计算 i_out

  if(i_out > (pfc.vloop.integral_max))
    {
			i_out = (pfc.vloop.integral_max);
      pfc.vloop.integral_inside_flag = false;		//饱和
    }
  else if(i_out < (pfc.vloop.integral_min))
    {
			i_out = (pfc.vloop.integral_min);
      pfc.vloop.integral_inside_flag = false;
    }
  else
    {
      pfc.vloop.integral_inside_flag = true;			//未饱和
    }
  pfc.vloop.integral = i_out;						//赋值 i_out
//  pfc.vloop.integral_storage	= i_out;		//储存 i_out
  if(((pfc.vloop.p_out + pfc.vloop.integral) ) > pfc.vloop.out_max)
    {
      pfc.vloop.loop_out = (pfc.vloop.out_max);
      pfc.vloop.out_inside_flag = false;
    }
  else if(((pfc.vloop.p_out + pfc.vloop.integral)) < (pfc.vloop.out_min))
    {
      pfc.vloop.loop_out = (pfc.vloop.out_min);
      pfc.vloop.out_inside_flag = false;
    }
  else
    {
      pfc.vloop.loop_out = (pfc.vloop.p_out + pfc.vloop.integral) ;
      pfc.vloop.out_inside_flag = true;	//计算 pid_out 并限制幅度
    }
		
  if(((pfc.vloop.p_out >= 0) && (i_out >= 0)) || (pfc.vloop.p_out < 0) && (i_out < 0))
    {
      pfc.vloop.twopart_no_same_flag = false;
    }
  else
    {
      pfc.vloop.twopart_no_same_flag = true;
    }
  if((pfc.vloop.out_inside_flag == true) && ((pfc.vloop.integral_inside_flag) || (pfc.vloop.twopart_no_same_flag)))
    {
      pfc.vloop.integral_en_k = 1;
    }
  else
    {
      pfc.vloop.integral_en_k = 0;
    }
		
}
//RAMCODE
//void pfc_PI_vloop(void)
//{
//    
//	  pfc.vloop.error   = pfc.vbus_ref   -  pfc.vbus_notch_filer_data;		//当前误差					  Q13
//	
//    pfc.vloop.p_out  = ( pfc.vloop.kp  *  pfc.vloop.error ) ;								//Q30
//		pfc.vloop.integral += ( pfc.vloop.ki  *  pfc.vloop.error ) ;								//Q30
//    if( pfc.vloop.integral > PI_I_HIGH_LIMIT)
//    {
//        pfc.vloop.integral = PI_I_HIGH_LIMIT ;																//Q30
//    }
//    else if( pfc.vloop.integral < PI_I_LOW_LIMIT )
//    {
//        pfc.vloop.integral = PI_I_LOW_LIMIT ;															//Q30
//    }

//    pfc.vloop.loop_out = ( pfc.vloop.p_out + pfc.vloop.integral ) >> 10;				//Q30>>15 = Q15 =32768
//		
//		if( pfc.vloop.loop_out > pfc.vloop.out_max)
//		{
//			pfc.vloop.loop_out = pfc.vloop.out_max;
//		}
//		else if(pfc.vloop.loop_out < pfc.vloop.out_min)
//		{
//			pfc.vloop.loop_out = pfc.vloop.out_min;
//		}
//   
//}

void pfc_vloop_init()
{
	pfc.vloop.kp = 15.0f;
  pfc.vloop.ki = 0.05f ;
	pfc.vloop.integral = 0.0f;
	pfc.vloop.p_out = 0.0f;
	pfc.vloop.integral_en_k = 0;
	
	pfc.vloop.integral_inside_flag = false;
	pfc.vloop.integral_max	= VLOOP_INTEGRA_MAX;
	pfc.vloop.integral_min	= VLOOP_INTEGRA_MIN;
	pfc.vloop.out_max = VLOOP_OUTPUT_MIN;
	pfc.vloop.out_min = VLOOP_OUTPUT_MIN;	
}

void pfc_iloop_init()
{
	pfc.iloop.k1 = 0.01155f;
  pfc.iloop.k2 = -0.00845f;
		
	pfc.iloop.kp = 0.07f;//0.096;//0.1;//0.12;
  pfc.iloop.ki = 0.004f;//0.0028;//0.0040;//0.01;			
	pfc.iloop.integral = 0.0f;
	pfc.iloop.p_out = 0.0f;
	
	pfc_current_integral_keep_en = false;
	pfc.iloop.integral_max	= ILOOP_INTEGRA_MAX;//(int32_t)1<<8; 
	pfc.iloop.integral_min	= ILOOP_INTEGRA_MIN;//(int32_t)-(1<<8);
	pfc.iloop.out_max_limit = ILOOP_OUTPUT_MAX_LIMIT;
	pfc.iloop.out_min_limit = ILOOP_OUTPUT_MIN_LIMIT;
	pfc.iloop.out_max = ILOOP_OUTPUT_MAX;
	pfc.iloop.out_min = ILOOP_OUTPUT_MIN;
}
