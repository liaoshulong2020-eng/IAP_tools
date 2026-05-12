#include "main.h"
#include "pid_app.h"

RAMCODE
void llc_PI_iloop(void)
{
  int32_t  i_mul, i_out, pi_out, data_error; //p_out,

  llc.iloop.data_error = llc.ibus_ref - llc.iout_rel;	//误差

  llc.iloop.p_out = (llc.iloop.data_error * llc.iloop.kp);		//??p_out

  i_mul = (llc.iloop.data_error * llc.iloop.ki) * (llc.iloop.integral_en_k);	//?? i_out

  i_out = i_mul + llc.iloop.integral;					//?? i_out

  if(i_out > (llc.iloop.integral_max))//积分最大限幅
    {
      i_out = (llc.iloop.integral_max);
      llc.iloop.integral_inside_flag = false;		//??
    }
  else if(i_out < (llc.iloop.integral_min))//积分最小限幅
    {
      i_out = (llc.iloop.integral_min);
      llc.iloop.integral_inside_flag = false;
    }
  else
    {
      llc.iloop.integral_inside_flag = true;			//???
    }
  llc.iloop.integral = i_out;						//?? i_out  积分
//  llc.iloop.integral_storage	= i_out;		//?? i_out
  if(((llc.iloop.p_out + llc.iloop.integral)) > llc.iloop.out_max)//电流环输出最大限幅
    {
      pi_out = (llc.iloop.out_max);
      llc.iloop.out_inside_flag = false;
    }
  else if(((llc.iloop.p_out + llc.iloop.integral)) < (llc.iloop.out_min))//电流环输出最小限幅
    {
      pi_out = (llc.iloop.out_min);
      llc.iloop.out_inside_flag = false;
    }
  else
    {
      pi_out = (llc.iloop.p_out + llc.iloop.integral) ;//输出等于比例加积分
      llc.iloop.out_inside_flag = true;	//?? pid_out ?????
    }
  if(((llc.iloop.p_out >= 0) && (i_out >= 0)) || (llc.iloop.p_out < 0) && (i_out < 0))
    {
      llc.iloop.twopart_no_same_flag = false;
    }
  else
    {
      llc.iloop.twopart_no_same_flag = true;
    }
  if((llc.iloop.out_inside_flag == true) && ((llc.iloop.integral_inside_flag) || (llc.iloop.twopart_no_same_flag)))
    {
      llc.iloop.integral_en_k = 1;
    }
  else
    {
      llc.iloop.integral_en_k = 0;
    }
  llc.iloop.loop_out = pi_out;

}


RAMCODE
void llc_PI_vloop(void)
{
  const float ERROR_LOW = 0.1f;
  const float ERROR_HIGH = 2.0f;
  const float KP_LOW = 100.0f, KP_HIGH = 2000.0f;
  const float KI_LOW = 60.0f, KI_HIGH = 100.0f;
  float transition_factor;

  float  i_mul, i_out, pi_out; //p_out,


  llc.vloop.data_error = llc.vbus_ref + llc.delta_voltage - llc.vbus_rel;	//误差

  llc.error_abs = abs_f(llc.vloop.data_error);//误差加绝对值

  if(llc.state == state_rampup)
    {
			  llc.vloop.kp = 300.0f;
				llc.vloop.ki = 40.0f;
    }
  else if(llc.state == state_on)
    {
				if (llc.error_abs <= ERROR_LOW)
						{
							transition_factor = 0.0f;
						}
						else if (llc.error_abs >= ERROR_HIGH)
						{
							transition_factor = 1.0f;
						}
						else
						{
							transition_factor = (llc.error_abs - ERROR_LOW) / (ERROR_HIGH - ERROR_LOW);
						}
						
						// ???????? kp ? ki
						llc.vloop.kp = (KP_LOW + transition_factor * (KP_HIGH - KP_LOW));
						llc.vloop.ki = (KI_LOW + transition_factor * (KI_HIGH - KI_LOW));
    }
  llc.vloop.p_out = (llc.vloop.data_error * llc.vloop.kp);		//??p_out  比例输出

  i_mul = (llc.vloop.data_error * llc.vloop.ki) * (llc.vloop.integral_en_k);	//?? i_out   积分输出

  i_out = i_mul + llc.vloop.integral;					//?? i_out  积分输出

  if(i_out > (llc.vloop.integral_max))//积分最大限幅
    {
      i_out = (llc.vloop.integral_max);
      llc.vloop.integral_inside_flag = false;		//??
    }
  else if(i_out < (llc.vloop.integral_min))//积分最小限幅
    {
      i_out = (llc.vloop.integral_min);
      llc.vloop.integral_inside_flag = false;
    }
  else
    {
      llc.vloop.integral_inside_flag = true;			//???
    }
  llc.vloop.integral = i_out;						//?? i_out  积分输出
//  llc.vloop.integral_storage	= i_out;		//?? i_out
  if(((llc.vloop.p_out + llc.vloop.integral)) > llc.vloop.out_max)//输出最大限幅
    {
      pi_out = (llc.vloop.out_max);
      llc.vloop.out_inside_flag = false;
    }
  else if(((llc.vloop.p_out + llc.vloop.integral)) < (llc.vloop.out_min))//输出最小限幅
    {
      pi_out = (llc.vloop.out_min);
      llc.vloop.out_inside_flag = false;
    }
  else
    {
      pi_out = (llc.vloop.p_out + llc.vloop.integral) ;//电压环输出
      llc.vloop.out_inside_flag = true;	//?? pid_out ?????
    }
  if(((llc.vloop.p_out >= 0) && (i_out >= 0)) || (llc.vloop.p_out < 0) && (i_out < 0))
    {
      llc.vloop.twopart_no_same_flag = false;
    }
  else
    {
      llc.vloop.twopart_no_same_flag = true;
    }
  if((llc.vloop.out_inside_flag == true) && ((llc.vloop.integral_inside_flag) || (llc.vloop.twopart_no_same_flag)))
    {
      llc.vloop.integral_en_k = 1;
    }
  else
    {
      llc.vloop.integral_en_k = 0;
    }
  llc.vloop.loop_out = pi_out;

}



RAMCODE
void llc_PI_Shareloop(void)
{
		if(llc.state == state_rampup)
	{
		llc.shareloop.kp = 100.0f;//llc.shareloop_kp_init;
		llc.shareloop.ki = 50.0f;//llc.shareloop_ki_init;
		llc.shareloop.out_max = 0.5f;//llc.shareloop_out_max_init;
		llc.shareloop.out_min = -llc.shareloop.out_max;
	}	
	else
	{
//		llc.shareloop.kp = llc.shareloop_kp_init;
//		llc.shareloop.ki = llc.shareloop_ki_init;
		
		//10k
		llc.shareloop.kp = llc.shareloop_kp_init;
		llc.shareloop.ki = llc.shareloop_ki_init;
		
		//100k
//		llc.shareloop.kp = 20.0f;//llc.shareloop_kp_init;
//		llc.shareloop.ki = 5.0f;//llc.shareloop_ki_init;
		
		llc.shareloop.out_max = 0.5f;
		
		if(llc.shareloop.out_max > 0.5f)
		{
			llc.shareloop.out_max = 0.5f;
		}
		llc.shareloop.out_min = -llc.shareloop.out_max;
		
//		if(llc.shareloop.data_error + 2.0f < 0 )
//		{
//			llc.shareloop.kp = 20.0f;
//			llc.shareloop.ki = 800.0f;
//		}
//		else
//		{
//			llc.shareloop.kp = 20.0f;
//			llc.shareloop.ki = 70.0f;
//		}
	}

  llc.shareloop.data_error = 0;

  llc.shareloop.data_error = llc.shareloop.ref - llc.shareloop.rel;	//????-????

  llc.shareloop.p_out = llc.shareloop.kp * llc.shareloop.data_error;
	
//  if(llc.shareloop.p_out > llc.shareloop.out_max)
//    {
//      llc.shareloop.p_out = llc.shareloop.out_max;

//    }
//  else if(llc.shareloop.p_out < llc.shareloop.out_min)
//    {
//      llc.shareloop.p_out = llc.shareloop.out_min;
//    }

  llc.shareloop.integral += llc.shareloop.ki * llc.shareloop.data_error;
	
  if(llc.shareloop.integral > 5.0f)
    {
      llc.shareloop.integral = 5.0f;

    }
  else if(llc.shareloop.integral < -5.0f)
    {
      llc.shareloop.integral = -5.0f;
    }

  llc.shareloop.loop_out = (llc.shareloop.p_out + llc.shareloop.integral)/1000.0f;
		
  if(llc.shareloop.loop_out > llc.shareloop.out_max)
    {
      llc.shareloop.loop_out = llc.shareloop.out_max;

    }
  else if(llc.shareloop.loop_out < llc.shareloop.out_min)
    {
      llc.shareloop.loop_out = llc.shareloop.out_min;
    }
  llc.delta_voltage = llc.shareloop.loop_out ;
	llc.delta_voltage = 0;



}

RAMCODE
void llc_loop_para_init(void)
{
//	llc.vloop.kp = 1000 ;
//  llc.vloop.ki = 20 ;
  llc.vloop.data_move = 12;
  llc.vloop.integral_max	= (int32_t)1 << 25;
  llc.vloop.integral_min	= (int32_t) - (1 << 25);

  llc.vloop.integral 			= 0 ;
  llc.vloop.data_error 		= 0 ;
  llc.vloop.loop_out 			= 0 ;
  llc.vloop.out_max 			= VOLT_LOOP_MIN;
  llc.vloop.out_min 			= VOLT_LOOP_MIN;


  llc.iloop.kp = 100 ;
  llc.iloop.ki = 10;
  llc.iloop.data_move = 12;
  llc.iloop.integral_max	= (int32_t)1 << 25;
  llc.iloop.integral_min	= (int32_t) - (1 << 25);

  llc.iloop.integral 			= 0 ;
  llc.iloop.data_error 		= 0 ;
  llc.iloop.loop_out 			= 0 ;
  llc.iloop.out_max 			= CURR_LOOP_MIN;
  llc.iloop.out_min 			= CURR_LOOP_MIN;



	
//	    llc.r_vloop.out = 0;
//	    llc.r_vloop.pre_out = 0;
//	    llc.r_vloop.prepre_out = 0;
//	    llc.r_vloop.err = 0;
//	    llc.r_vloop.pre_err = 0;
//      llc.r_vloop.prepre_err = 0;


}
