#include "main.h"


RAMCODE
void rectify_vac(void)
{
  if(pfc.acn_value > pfc.acl_value)
    {
      pfc.vin_voltage = pfc.acn_value - pfc.acl_value;	//零线-火线 定义为负半周
      pfc.positive_period = 0;
      pfc.negative_period = 1; 	//负半周标志位
    }
  else
    {
      pfc.vin_voltage = pfc.acl_value - pfc.acn_value;  //火线-零线 定义为正半周
      pfc.positive_period = 1;
      pfc.negative_period = 0; 	//正半周标志位
    }

//  pfc.vin_sum = pfc.vin_voltage + pfc.vin_sum - (pfc.vin_sum >> 3);
//  pfc.vin_filtered = pfc.vin_sum >> 3;
  pfc.vin_filtered = pfc.vin_voltage;
  pfc.vdc_filtered = pfc.vbus_value + pfc.vdc_filtered - (pfc.vdc_filtered / 32.0f);
  pfc.vdc_bus_square = (pfc.vdc_filtered / 32.0f) * (pfc.vdc_filtered / 32.0f) / 2048.0f;
}

RAMCODE
void calculate_cur_target(void)
{
  pfc.ac_current_ref = ((pfc.ibus_target_average * pfc.vin_filtered) / 65536.0f) + pfc.ibus_target_offset ;

  if(pfc.ac_current_ref > 8192.0f)
    {
      pfc.ac_current_ref = 8192.0f;
    }
  else if(pfc.ac_current_ref <= 0.0f)
    {
      pfc.ac_current_ref = 0.0f;
    }
}

uint16_t cnt;
RAMCODE
void half_cycle_processing(void)
{
  pfc.vin_squared = (pfc.vin_filtered * pfc.vin_filtered) ;

  pfc.move_vin_sum += pfc.vin_squared - pfc.move_calcu_vin[(cnt + 1) % 200];
  pfc.move_calcu_vin[cnt] = pfc.vin_squared;
  cnt = (cnt + 1) % 200; // 更新cnt
  pfc.move_vin_average = pfc.move_vin_sum / 200; // 计算最近100个样本的平均电压平方
  pfc.vin_rel = FastInverseSqrt(pfc.move_vin_average);

  if(pfc.positive_period)					//正半周
    {
      pfc.positive_cnt++;	//正半周期位置

      pfc.positive_squares_sum = pfc.vin_squared + pfc.positive_squares_sum;	//计算N次平方和
      pfc.positive_average_sum += pfc.vin_filtered;
      if((8 == pfc.positive_cnt) && (pfc.negative_cnt >= 50))
        {
          pfc.vin_squares_average = 	pfc.negative_squares_sum / pfc.negative_cnt;	//平方和的平均值  均方值
          pfc.ac_voltage_avg = pfc.negative_average_sum / pfc.negative_cnt;
          pfc.negative_cnt = 0;						//清除负半周期计数值
          pfc.negative_squares_sum = 0;   //清除负半周期累加值
          pfc.negative_average_sum = 0;
          pfc.zero_point = NEGA_TO_POSI;										//过零点
        }
      else if(pfc.positive_cnt >= 110)
        {
          pfc.negative_cnt = 0;						//清除负半周期计数值
          pfc.negative_squares_sum = 0;   //清除负半周期累加值
          pfc.vin_squares_average = 	pfc.positive_squares_sum / pfc.positive_cnt;	//平方和的平均值  均方值
          pfc.ac_voltage_avg = pfc.positive_average_sum / pfc.positive_cnt;
          pfc.positive_average_sum = 0;
          pfc.positive_squares_sum = 0;
          pfc.positive_cnt = 0;
          pfc.zero_point = NO_ZERO_POINT;
        }

      pfc.last_cycle = POSITIVE;

    }

  else //if(pfc.negative_period )								//负半周
    {

      pfc.negative_cnt++;	//负半周期位置
      pfc.negative_squares_sum = pfc.vin_squared + pfc.negative_squares_sum;	//计算N次平方和
      pfc.negative_average_sum += pfc.vin_filtered;

      //  if(pfc.last_cycle == POSITIVE )		//正变负，过零点
      if((8 == pfc.negative_cnt) && (pfc.positive_cnt >= 50))
        {
          pfc.vin_squares_average = 	pfc.positive_squares_sum / pfc.positive_cnt;	//平方和的平均值  均方值
          pfc.ac_voltage_avg = pfc.positive_average_sum / pfc.positive_cnt;
          pfc.positive_cnt = 0;						//清除正半周期计数值
          pfc.positive_squares_sum = 0;		//清除正半周期累加值
          pfc.positive_average_sum = 0;
          pfc.zero_point = POSI_TO_NEGA;
        }
      else if(pfc.negative_cnt >= 110)
        {
          pfc.positive_cnt = 0;						//清除正半周期计数值
          pfc.positive_squares_sum = 0;		//清除正半周期累加值
          pfc.vin_squares_average = 	pfc.negative_squares_sum / pfc.negative_cnt;	//平方和的平均值  均方值
          pfc.ac_voltage_avg = pfc.negative_average_sum / pfc.negative_cnt;
          pfc.negative_cnt = 0;						//清除负半周期计数值
          pfc.negative_squares_sum = 0;   //清除负半周期累加值
          pfc.negative_average_sum = 0;
          pfc.zero_point = NO_ZERO_POINT;
        }

      pfc.last_cycle = NEGATIVE;
    }

  pfc.vin_squared_for_ac_drop = pfc.vin_squares_average;
  //	pfc.ac_voltage_rms = pfc.ac_voltage_avg * 10/9;
}

RAMCODE
void input_mode_check(void)
{
  if(pfc.input_check_is_ok == 0)
    {
      pfc.vin_on_voltage = VAC_IN_ON_VOLTAGE;
      pfc.vin_under_voltage = VAC_IN_UNDER_VOLTAGE;
      pfc.vin_max_voltage = VAC_IN_MAX_VOLTAGE;
      pfc.vin_over_voltage = VAC_IN_OVER_VOLTAGE;
      pfc.set_protect_is_ok = 0;

      if(pfc.vin_rel > 100)  // 大于该电压才进行判断 AC/DC输入
        {
          pfc.check_input_mode_cnt++;

          if(pfc.zero_point == NO_ZERO_POINT)
            {
              pfc.ac_check_cnt++;
            }
          else
            {
              pfc.ac_check_cnt = 0;
            }

          if(pfc.check_input_mode_cnt >= 1000)
            {
              if(pfc.ac_check_cnt >= 1000)
                {
                  pfc.input_mode = DC_MODE;
                }
              else
                {
                  pfc.input_mode = AC_MODE;
                }

              pfc.input_check_is_ok = 1;

              // 判断完成后重置计数器
              pfc.check_input_mode_cnt = 0;
              pfc.ac_check_cnt = 0;
            }
          else
            {
              pfc.input_check_is_ok = 0;
              pfc.input_mode = UNKNOW_MODE;
            }
        }
      else
        {
          // 如果输入电压不足，重置计数器和模式
          pfc.check_input_mode_cnt = 0;
          pfc.ac_check_cnt = 0;
          pfc.input_check_is_ok = 0;
          pfc.input_mode = UNKNOW_MODE;
        }
    }
}

//RAMCODE
//void set_vbus_voltage()
//{
//	if((llc_send_vbus_target > 385.0f) && (llc_send_vbus_target< 420.0f))
//	{
//		pfc.vbus_target = llc_send_vbus_target;
//	}
//	else
//	{
//	  pfc.vbus_target = VOUT_VOLTAGE ;
//	}
//	if(pfc.state == State_On)
//	{
//		pfc.vbus_ref = pfc.vbus_target;
//	}
//}
RAMCODE
void set_protect_point(void)
{
  if(pfc.set_protect_is_ok == 0 && pfc.input_check_is_ok == 1)
    {
      switch(pfc.input_mode)
        {
        case DC_MODE:
          pfc.vin_on_voltage = VDC_IN_ON_VOLTAGE;
          pfc.vin_under_voltage = VDC_IN_UNDER_VOLTAGE;
          pfc.vin_max_voltage = VDC_IN_MAX_VOLTAGE;
          pfc.vin_over_voltage = VDC_IN_OVER_VOLTAGE;
          pfc.set_protect_is_ok = 1;
          break;

        case AC_MODE:
          pfc.vin_on_voltage = VAC_IN_ON_VOLTAGE;
          pfc.vin_under_voltage = VAC_IN_UNDER_VOLTAGE;
          pfc.vin_max_voltage = VAC_IN_MAX_VOLTAGE;
          pfc.vin_over_voltage = VAC_IN_OVER_VOLTAGE;
          pfc.set_protect_is_ok = 1;
          break;

        default:
          pfc.vin_on_voltage = VAC_IN_ON_VOLTAGE;
          pfc.vin_under_voltage = VAC_IN_UNDER_VOLTAGE;
          pfc.vin_max_voltage = VAC_IN_MAX_VOLTAGE;
          pfc.vin_over_voltage = VAC_IN_OVER_VOLTAGE;
          pfc.set_protect_is_ok = 0;
          break;
        }
    }
}

RAMCODE
void vbus_filter()
{

}


RAMCODE
void check_ac_drop(void)
{

//  if(pfc.vin_filtered > AC_DROP_V_RECT_THRESHOLD)
//    {
//      pfc.ac_drop_count = 0;
//    }
//  else
//    {
//      pfc.ac_drop_count ++ ;
//      if(pfc.ac_drop_count > AC_DROP_COUNT_MAX)
//        {
//          pfc.ac_drop = 1; // can add the io control
//          pfc.vin_squared_for_ac_drop = 0;
//        }
//    }
//  if(pfc.vin_squared_for_ac_drop > VAC_MIN_OFF_SQ_AVG)
//    {
//      pfc.ac_drop = 0; // can add the io control
//    }
}


RAMCODE
void pfc_vloop_para_manage()
{
//	pfc.dc_voltage_error = pfc.vbus_ref - pfc.vbus_value;
//	if(abs(pfc.dc_voltage_error) > DC_VOLTAGE_ERROR_THR1)
//	{
//
//	}
//  pfc.vloop.kp = pfc_vloop_kp1;
//  pfc.vloop.ki = pfc_vloop_ki1;

}
RAMCODE
void  pfc_iloop_para_manage()
{


}

//RAMCODE
//void current_ref_culculation()
//{
//  pfc.ibus_target_average = (pfc.vloop.loop_out * pfc.vff_multiplier);
//  pfc.iloop.ref = pfc.ibus_target_average * pfc.vin_filtered;

//  if(pfc.iloop.ref > 8192)
//    {
//      pfc.iloop.ref  = 8192;
//    }
//  else if(pfc.iloop.ref  <= 0)
//    {
//      pfc.iloop.ref = 0;
//    }

//}

RAMCODE
void pfc_iloop_handle(void)
{
  //								1									311(Q9)															220*220(Q15)
  pfc.iloop.ref =	(pfc.vloop.loop_out * pfc.vin_voltage / ((float)pfc.vin_squares_average))	;
  //vloop_out*Vac/Rms^2

  if(pfc.iloop.ref > ILOOP_INPUT_MAX)		//10
    {
      pfc.iloop.ref = ILOOP_INPUT_MAX;
    }
  else if(pfc.iloop.ref < ILOOP_INPUT_MIN)
    {
      pfc.iloop.ref = ILOOP_INPUT_MIN;
    }

  pfc_PI_iloop();
}

RAMCODE
void K_forward_calcula()
{
  //0.7*(1-vac/vbus)*duty

  if(pfc.vbus_rel < 200.0f)
    {
      pfc.k_forward = 0.8f * (1.0f - (pfc.vin_voltage / 200.0f)) ;
    }
  else
    {
      pfc.k_forward = 0.8f * (1.0f - (pfc.vin_voltage / pfc.vbus_rel));
    }
  if(pfc.k_forward > 0.8f)
    {
      pfc.k_forward = 0.8f;
    }
}

RAMCODE
void pfc_handle(void) //10k
{
  {
    if(pfc.state == State_Rampup || pfc.state == State_On)
      {
        pfc_vloop_para_manage();
        pfc_PI_vloop();
      }
    else
      {
        pfc_vloop_init();
      }

    switch(pfc.state)
      {
      case State_Idel:
        pfc_state_idle();
        break;

      case State_Rampup:
        pfc_state_rampup();
        break;

      case State_On:
        pfc_state_on();
        break;

      case State_Fault:
        pfc_state_fault();
        break;

      default:
        pfc_state_fault();
        break;
      }
  }
}

RAMCODE
void state_switch(void)
{



}


RAMCODE
void pfc_transition_to_idle_state(void)
{
  if((!pfc.under_input_flag) && (!pfc.over_input_flag)
      && (!pfc.V_over_output_flag) && (!pfc.I_over_output_flag))
    {
      Reset_value();
    }
}

RAMCODE
void pfc_state_idle(void)
{
  // Reset_value();
//  uint32_t delta_v, vin_square_temp;
  if(pfc.r_ntc_value < 200)
    {
//			hrpwm_pre_outen();
			
      if(start_cnt < START_COUNT_VAL)
        {
          start_cnt ++ ;
        }
      else
        {
          pfc.start_cnt_flg = 1;
        }
    }
  else
    {
      start_cnt = 0;
			
			if(!pfc.pre_finish_flg)
			{			
					if(pfc.pre_driver_cmpb > pfc.pre_driver_period)
				{
					pfc.pre_driver_cmpb = pfc.pre_driver_period+1;
				}
				else
				{
					pfc.pre_driver_cmpb +=10;   
				}
			}
    }

  if(start_cnt == (START_COUNT_VAL >> 1))
    {
      pfc.pre_finish_flg = 1;
			pfc.pre_driver_cmpb = pfc.pre_driver_period+1;

    }
  if(pfc.start_cnt_flg && pfc.pre_finish_flg && pfc.check_current_is_ok && pfc.is_no_fault && pfc.set_protect_is_ok)
    {
      pfc.vbus_ref = pfc.vbus_rel ;
      pfc.state = State_Rampup;	//进入启动过程
      pfc.Fault_Flag = 0;
    }
}

RAMCODE
void pfc_state_rampup(void)
{
  if(pfc.is_no_fault)
//        if(pfc.zero_point == NEGA_TO_POSI)							//过零点才启动
    {
      hrpwm_outen_app();										//启动PWM(最小占空比)//hyd disable for ac sample 11.01
      if(pfc.vbus_ref >= pfc.vbus_target)	//参考值达到设定的目标值
        {
          pfc.dcdc_on_cnt ++ ;
          pfc.vbus_ref = pfc.vbus_target; //锁定在目标值
          if(pfc.dcdc_on_cnt >= 1000)
            {
              pfc_ok_signal();	//PFC_POWER_DET 光耦信号
              pfc.dcdc_on_cnt = 0;
              pfc.state = State_On;					  //进入保持状态
            }
        }
      else
        {
          pfc.vbus_ref = pfc.vbus_ref + 0.01;//4		//5							//未达到设定的目标值则继续增加
        }
    }
  else
    {
      pfc.state = State_Fault;
    }
}

RAMCODE
void pfc_state_on(void)
{
  if((pfc.vbus_rel) < VOUT_UNDER_VOLTAGE)
    {
      pfc.V_under_output_cnt++;
    }
  else
    {
      pfc.V_under_output_cnt  = 0;
    }
#if TEST_MODE
#else
  if((pfc.V_under_output_cnt) > 100)
    {
      hrpwm_outdis_app();
      pfc_nok_signal();
      pfc.close_pwm_flag |= (1 << 4);
      pfc.state = State_Fault;
    }
#endif
  if(pfc.is_no_fault)
    {
//      fault_check_app();
    }
  else
    {
      hrpwm_outdis_app();
      pfc.close_pwm_flag |= (1 << 5);
      pfc_nok_signal();
      pfc.state = State_Fault;
    }
}

RAMCODE
void pfc_state_fault(void)
{
  hrpwm_outdis_app();
  pfc.close_pwm_flag |= (1 << 6);
  pfc_nok_signal();
  pfc.fault_cnt++;

  if(pfc.fault_cnt > 200)//32767)		//延迟、放电
    {
      pfc.fault_cnt = 0;
      pfc_transition_to_idle_state();
    }
}
