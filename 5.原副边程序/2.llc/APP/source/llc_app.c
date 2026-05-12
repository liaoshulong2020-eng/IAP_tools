#include "main.h"
#include "llc_app.h"

RAMCODE
void vout_over_protect()
{
  if(llc.vbus_target > 14.5)
    {
      if(llc.over_voltage_check_cnt > 10)
        {
          llc.check_ovp_flag = 1 ;
          llc.state = state_fault;
        }
      else
        {
          llc.over_voltage_check_cnt++;
        }
    }
}

RAMCODE
void vout_config(void)
{
if ((llc.state == state_on && abs_f(llc.vbus_target - llc.vbus_ref) > 0.01f) || llc.first_load)
    {
			if(llc.first_load){llc.first_load = false;}
			
      if(llc.vbus_target <= 9.0f)
        {
          llc.vbus_target = 9.0f;
        }
				else if(llc.vbus_target >= 14.0f)
				{
					llc.vbus_target = 14.0f;
				}
				
      llc.vbus_ref = llc.vbus_target;
			
 
      if(llc.vbus_target > VOUT_UP_VOLTAGE)	//	>	12.5v	上调
        {
          llc.trim_direct = trim_up;

          user_data.over_current_point =  RATED_POUT_WATT/llc.vbus_target * 1.2f;// 13.2V->60.0A
          llc.protection_point.over_current_point = user_data.over_current_point;

          user_data.ibus_rec_target = user_data.over_current_point - (IOUT_OCP_CURRENT-IOUT_REC_TARGET_CURRENT);
					llc.protection_point.over_current_rec_point = user_data.ibus_rec_target;
          llc.ibus_rec_target = llc.protection_point.over_current_rec_point;

          user_data.short_current_point = user_data.over_current_point + (IOUT_SHORT_CURRENT - IOUT_OCP_CURRENT);
          llc.protection_point.short_current_point = user_data.short_current_point;

          user_data.ibus_target = user_data.over_current_point - (IOUT_OCP_CURRENT-IOUT_TARGET_CURRENT) ;
          llc.ibus_target = user_data.ibus_target;
          llc.ibus_ref = llc.ibus_target;
					user_data.under_voltage_point = VOUT_UNDER_VOLTAGE + llc.vout_trim_delta;
					llc.protection_point.under_voltage_point = user_data.under_voltage_point;
					llc.R_Out_ratio_max = 5.0f;
        }
      else if(llc.vbus_target < VOUT_DOWM_VOLTAGE)	//	< 11.5v	下调 
        {
					llc.trim_direct = trim_down;
					
          user_data.over_current_point =  IOUT_OCP_CURRENT;
          llc.protection_point.over_current_point = user_data.over_current_point;

          user_data.ibus_rec_target = IOUT_REC_TARGET_CURRENT-3.0f;
          llc.protection_point.over_current_rec_point = user_data.ibus_rec_target;
          llc.ibus_rec_target = llc.protection_point.over_current_rec_point;

          user_data.short_current_point = IOUT_SHORT_CURRENT;
          llc.protection_point.short_current_point = user_data.short_current_point;

          user_data.ibus_target = IOUT_TARGET_CURRENT-5.0f;
          llc.ibus_target = user_data.ibus_target;

					user_data.under_voltage_point = 8.0f;
					llc.protection_point.under_voltage_point = user_data.under_voltage_point;
					
          llc.ibus_ref = llc.ibus_target;

          llc.R_Out_ratio_max = 5.0f;
        }
      else	//12V
        {
					llc.trim_direct = trim_normal;
					
          user_data.over_current_point =  IOUT_OCP_CURRENT;
          llc.protection_point.over_current_point = user_data.over_current_point;

          user_data.ibus_rec_target = IOUT_REC_TARGET_CURRENT;
          llc.protection_point.over_current_rec_point = user_data.ibus_rec_target;
          llc.ibus_rec_target = llc.protection_point.over_current_rec_point;

          user_data.short_current_point = IOUT_SHORT_CURRENT;
          llc.protection_point.short_current_point = user_data.short_current_point;

          user_data.ibus_target = IOUT_TARGET_CURRENT;
          llc.ibus_target = user_data.ibus_target;

					user_data.under_voltage_point = VOUT_UNDER_VOLTAGE + llc.vout_trim_delta;
					llc.protection_point.under_voltage_point = user_data.under_voltage_point;
					
          llc.ibus_ref = llc.ibus_target;
          llc.R_Out_ratio_max = 5.0f;
        }



			if(llc.protection_point.under_voltage_point < 7.0f)
				{
					llc.protection_point.under_voltage_point = 7.0f;
				}
			else if(llc.protection_point.under_voltage_point > 11.0f)
				{
					llc.protection_point.under_voltage_point = 11.0f;
				}
    }
}


RAMCODE
void llc_loop_output_ctrl(void)
{
  if(llc.vloop.out_max < VOLT_LOOP_MAX)	//电压环输出最大值软启动
    {
      llc.vloop.out_max += 10;
    }
  else
    {
      llc.vloop.out_max = VOLT_LOOP_MAX;
    }

  if(llc.iloop.out_max < CURR_LOOP_MAX)	//电流环输出最大值软启动
    {
      llc.iloop.out_max += 10;		//电流环输出最大值软启动
    }
  else
    {
      llc.iloop.out_max = CURR_LOOP_MAX;
    }
}

uint16_t test_cnt;
RAMCODE
void llc_loop_switch_ctrl(void)
{
  llc_PI_Shareloop();
  llc_PI_vloop();
  llc_PI_iloop();

  if((llc.iout_rel < (IOUT_TARGET_CURRENT)))	//电流环输出小于电压环
    {
      if(llc.current_mode_cnt > 200)
        {
          {
            llc.current_mode_flag = 1;	//电流较小
            llc.current_mode_cnt = 0 ;
          }
        }
      else
        {
          llc.current_mode_flag = 0;
          llc.current_mode_cnt ++;
        }
    }
  else
    {
      llc.current_mode_cnt = 0;
      llc.current_mode_flag = 0;
    }
  if(llc.iloop.loop_out < llc.vloop.loop_out)
    {
      llc_R_vrest(&Volt_Rctrl);
      llc.loop_out = llc.iloop.loop_out;
      llc.output_mode = current_mode;
    }
  else if(llc.iloop.loop_out > llc.vloop.loop_out	 )
    {
      llc_R_vloop(&Volt_Rctrl,llc.vloop.data_error);
//      llc.loop_out = llc.vloop.loop_out;// + Volt_Rctrl.R_Out;
//
      if(llc.iout_rel > IOUT_SR_ON_CURRENT)
        {
          if(llc.pr_open_cnt > 10)
            {
              if(llc.PR_output_cnt > 10000)
                {
									llc.R_Out_ratio += 0.1f;
                  llc.PR_output_cnt = 0;
                }
              else
                {
                  llc.PR_output_cnt++;
                }
              llc.pr_close_cnt = 0;
            }
          else
            {
              llc.pr_open_cnt++;
            }


          if(llc.R_Out_ratio > llc.R_Out_ratio_max )
            {
              llc.R_Out_ratio = llc.R_Out_ratio_max ;
            }

        }
      else if(llc.iout_rel < IOUT_SR_OFF_CURRENT)
        {
          if(llc.pr_close_cnt > 100)
            {
              llc.R_Out_ratio = 0;
              llc.pr_open_cnt = 0;
              llc.PR_output_cnt = 0;
            }
          else
            {
              llc.pr_close_cnt++;
            }

        }
				
      llc.loop_out = llc.vloop.loop_out + (llc.R_Out_ratio * Volt_Rctrl.R_Out);
			if(llc.loop_out < 0)llc.loop_out = 0;
      llc.output_mode = voltage_mode;
    }
//			else

//          llc.output_mode = current_mode;
//          llc.voltage_mode_cnt = 0;
//        }
//    }



#if OPEN_LOOP
  if(llc.loop_out_test >LLC_SW_PERIOD_MAX)
    {
      llc.loop_out_test = LLC_SW_PERIOD_MAX;
    }
  else
    {
      test_cnt++;
      if(test_cnt > 1000)
        {
//          llc.loop_out_test++;
        }
    }
  llc.loop_out = 5000;//llc.loop_out_test;
#endif

  if((llc.iout_rel) >= IOUT_SR_ON_CURRENT)
    {

      if(llc.sr_open_cnt > 200)
        {
					llc.sr_pwm = 1;		// 输出电流大 > 10A , SR 开
        }
      else
        {
          llc.sr_open_cnt++;
        }
      llc.sr_close_cnt = 0;
    }
  else if((llc.iout_rel) < IOUT_SR_OFF_CURRENT)
    {
      if(llc.sr_close_cnt > 200)
        {
          llc.sr_pwm = 0;		//输出电流大 < 2A	 , SR 关
        }
      else
        {
          llc.sr_close_cnt++;
        }
      llc.sr_open_cnt = 0;
    }
}
#define PWM_DUTY_SLOPE      (2724.0f / 8253.0f)   // ≈ 0.33f
#define PWM_DUTY_OFFSET     (623.0f)
uint8_t burst_period;
RAMCODE
void llc_pwm_value_ctrl(void)
{
	if(llc.loop_out < 0)llc.loop_out = 0;
if((llc.state == state_rampup) && (llc.output_mode == voltage_mode))
    {
        if(llc.loop_out > (float)LLC_SW_PERIOD_MAX)         // > 13714, 低于210k
        {
            llc.period = LLC_SW_PERIOD_MAX;
            llc.duty   = (llc.period >> 1) - LLC_DEADTIME_COUNT;
            llc.switch_mode = max_pfm_mode;
        }
        else if(llc.loop_out > (float)LLC_SW_PERIOD_PFM_MODE)  // 9600~13714, 210k~300k
        {
            llc.period = (uint32_t)llc.loop_out;
            llc.duty   = (llc.period >> 1) - LLC_DEADTIME_COUNT;   // 50%占空比
            llc.switch_mode = pfm_mode;
        }
        else if(llc.loop_out > (float)LLC_DUTY_MIN_COUNT_RAMPUP * 0.23f)  // 1347~9600
        {
            llc.period = LLC_SW_PERIOD_PWM_MODE;    // 固定300k

            // 线性映射，保证两端边界duty连续
            int32_t duty_calc = (int32_t)(PWM_DUTY_SLOPE * llc.loop_out + PWM_DUTY_OFFSET);
            if(duty_calc < 100) duty_calc = 100;
            llc.duty = (uint32_t)duty_calc;

            llc.switch_mode = pwm_mode;
        }
        else    // < 1347, burst模式
        {
            llc.period = LLC_SW_PERIOD_BURST;
            llc.duty   = (LLC_DUTY_MIN_COUNT / 2) - LLC_DEADTIME_COUNT - LLC_DEADTIME_DUTY_COUNT;
            llc.switch_mode = burst_mode;
            llc.burst_cnt_temp = (uint16_t)(llc.loop_out * 15.0f
                                  / ((float)(LLC_DUTY_MIN_COUNT / 2)
                                  - LLC_DEADTIME_COUNT + LLC_DEADTIME_DUTY_COUNT));
        }
    }
  else
{
				if(llc.trim_direct == trim_down)
				{
								if(llc.loop_out > (float)LLC_SW_PERIOD_MAX)      // ×?μí?μ?ê??êy?μ 150k
							{
									llc.period = LLC_SW_PERIOD_MAX;
									llc.duty = (llc.period >> 1) - LLC_DEADTIME_COUNT ;   // 50% duty
									if(llc.duty < 100) llc.duty = 100;
									llc.switch_mode = max_pfm_mode;
							}
							else if(llc.loop_out > LLC_SW_PERIOD_MIN)
							{
									llc.period = (uint32_t)llc.loop_out;
									if(llc.period < LLC_SW_PERIOD_MIN) llc.period = LLC_SW_PERIOD_MIN;
									if(llc.period > LLC_SW_PERIOD_MAX) llc.period = LLC_SW_PERIOD_MAX;

									llc.duty = (llc.period >> 1) - LLC_DEADTIME_COUNT ;
									if(llc.duty < 100) llc.duty = 100;

									llc.switch_mode = pfm_mode;
							}
							else if(llc.loop_out < 200.0f)
							{
									float target_ratio = 0.1f;
									uint32_t target_duty;
									uint32_t max_duty;

									llc.period = LLC_SW_PERIOD_MIN;   // 1ì?¨350k 8228

									target_duty = (uint32_t)((float)llc.period * target_ratio); // 822
									max_duty = (llc.period / 2) - LLC_DEADTIME_COUNT;
									if(target_duty > max_duty) target_duty = max_duty;
									llc.duty = target_duty;

									if(llc.duty < 100) llc.duty = 100;

									llc.burst_cnt_temp = (uint16_t)(llc.loop_out * 10.0f / (float)llc.duty);
									if(llc.burst_cnt_temp > 15) llc.burst_cnt_temp = 15;

									llc.switch_mode = burst_mode;
							}
								else //if(llc.loop_out > LLC_DUTY_MIN_COUNT)
							{
								llc.period = LLC_SW_PERIOD_MIN; 	//300k
								llc.duty = (llc.loop_out /2.0f) - LLC_DEADTIME_COUNT ;					//1%-47%duty
								if(llc.duty < 100) llc.duty = 100;
								llc.switch_mode = pwm_mode;
							}
				
				}
				else
				{
							if(llc.loop_out > (float)LLC_SW_PERIOD_MAX)      // 最低频率计数值 150k
							{
									llc.period = LLC_SW_PERIOD_MAX;
									llc.duty = (llc.period >> 1) - LLC_DEADTIME_COUNT;   // 50% duty
									llc.switch_mode = max_pfm_mode;
							}
							else if(llc.loop_out > 4000.0f)
							{
									llc.period = (uint32_t)llc.loop_out;
									if(llc.period < LLC_SW_PERIOD_MIN) llc.period = LLC_SW_PERIOD_MIN;
									if(llc.period > LLC_SW_PERIOD_MAX) llc.period = LLC_SW_PERIOD_MAX;

									llc.duty = (llc.period >> 1) - LLC_DEADTIME_COUNT;
									if(llc.duty < 100) llc.duty = 100;

									llc.switch_mode = pfm_mode;
							}
							else
							{
									float target_ratio = 0.1f;
									uint32_t target_duty;
									uint32_t max_duty;

									llc.period = LLC_SW_PERIOD_MIN;   // 固定350k 8228

									target_duty = (uint32_t)((float)llc.period * target_ratio); // 822
									max_duty = (llc.period / 2) - LLC_DEADTIME_COUNT;
									if(target_duty > max_duty) target_duty = max_duty;
									llc.duty = target_duty;

									if(llc.duty < 100) llc.duty = 100;

									llc.burst_cnt_temp = (uint16_t)(llc.loop_out * 10.0f / (float)llc.duty);
									if(llc.burst_cnt_temp > 15) llc.burst_cnt_temp = 15;

									llc.switch_mode = burst_mode;
							}
				}
			}

	
  if(llc.switch_mode != burst_mode && llc.switch_mode !=duty_pfm_mode)
    {
      llc.burst_cnt = 5;
    }
  else
    {
      if(llc.burst_period > 5)
        {

					uint16_t temp = llc.burst_cnt_temp;
					if(temp > 15) temp = 15;
					llc.burst_cnt = temp;
          llc.burst_period = 0;
        }
      else
        {
          llc.burst_period ++;
        }
    }

//    if((HRPWM->PWM[2].REG.PWMISR & (1 << 3)))
  {
//        (HRPWM->PWM[2].REG.PWMISR |= (1 << 3));
    if(llc.burst_cnt > 0)
      {
        if((llc.state != state_idel) && (llc.state != state_fault))
          {
            hrpwm_outen_app();
            llc.burst_cnt--;
          }

      }
    else
      {
        llc.burst_cnt = 0;
        hrpwm_outdis_app();
      }
  }
}


RAMCODE
void llc_loop_ctrl(void)
{

  llc_loop_output_ctrl();
  llc_loop_switch_ctrl();
  llc_pwm_value_ctrl();
}

RAMCODE
void llc_loop_init(void)
{
  llc_loop_para_init();
  llc_R_vrest(&Volt_Rctrl);
  llc.output_mode = voltage_mode;
  llc.loop_out = 0;
  llc.duty = 50;
  llc.period = LLC_SW_PERIOD_MIN;		//300k
  llc.switch_mode = burst_mode;
}

RAMCODE
void llc_handle(void)
{
  if(llc.state == state_rampup || llc.state == state_on)
    {
      llc_loop_ctrl();
    }
  else
    {
      llc_loop_init();
    }

  switch(llc.state)
    {

    case state_idel:
      llc_state_idle();
      break;

    case state_rampup:
      llc_state_rampup();
      break;

    case state_on:
      llc_state_on();
      break;

    case state_fault:
      llc_state_fault();
      break;

    default:
      llc_state_fault();
      break;
    }
}

RAMCODE
void llc_state_idle(void)
{

  if(llc.idel_delay_cnt > IDEL_DELAY_VALUE)
    {
      llc.idel_delay_ok = 1;
      llc.idel_delay_cnt = 0 ;
    }
  else
    {
      llc.idel_delay_cnt ++;
    }

  if(llc.on_off_ctrl_ok && llc.idel_delay_ok &&  llc.pfc_is_ok && (!llc.fault_state.all) &&llc.start_contr )
    {
      llc.vbus_ref = llc.vbus_rel;
			enable_current_sharing();		//使能均流	
			enable_sync_start();//发出同步信号
	
			if(read_sync_sign())
			{
			    llc.state = state_rampup;
					hrpwm_outen_app();

//					ShareCurrDutySet();
					ShareCurrDutySet_FullLoad();
			}

    }
}

RAMCODE
void llc_state_rampup(void)
{
  if(llc.on_off_ctrl_ok && llc.pfc_is_ok &&llc.start_contr)
    {
      if((!llc.fault_state.all))
        {
          if(llc.vbus_ref >= llc.vbus_target)	//达到目标电压
            {
              llc.vbus_ref = llc.vbus_target; //
              llc.ramup_delay_cnt++;
              if(llc.ramup_delay_cnt > 10)
                {

                  llc.state = state_on;						//进入下一状态
                  llc.ramup_delay_cnt = 0;
                }
            }
          else
            {
//              if(llc.vbus_ref  < 2.0f)
//                {
//                  llc.vbus_ref += (float)0.005;
//                }
//              else
//                {
                  llc.vbus_ref = llc.vbus_ref + (float)0.0005;//未达到设定的目标值则继续增加
//                }
								
								if(llc.vbus_ref > 5.0f)
								{
//									LL_DAC_ValueSet(DAC0,IOUT_OCP_REC_CURRENT_DAC_DAC_VALUE);
								}
            }
//						ShareCurrDutySet_FullLoad();
						ShareCurrDutySet();
        }
      else
        {
          llc.state = state_fault;
        }
    }
  else
    {
      llc.state = state_fault;
    }
}

RAMCODE
void llc_state_on(void)
{
  if(llc.on_off_ctrl_ok && llc.pfc_is_ok && (!llc.fault_state.all) &&llc.start_contr )
    {
      llc.test_cnt ++;

      //欠压检测

      if(llc.test_cnt > 50)
        {
					llc.ibus_ref = llc.ibus_target;
					LL_DAC_ValueSet(DAC0,IOUT_OCP_CURRENT_DAC_DAC_VALUE);					
//				LL_CMP_Start(OCP_CMP);

//				llc.state = state_fault;
//				llc.start_contr = 0;
				llc.test_cnt = 0;
				llc.ocp_time = 0;
        }

				
//      if(llc.output_mode == voltage_mode)
//        {
						ShareCurrDutySet();
//        }
//      else
//        {
//          ShareCurrDutySet_Max();
//        }




    }
  else
    {
      llc.state = state_fault;
    }
}
RAMCODE
void llc_state_fault(void)
{
  hrpwm_outdis_app();
  hrpwm_srdis_app();
  disable_current_sharing();
	disable_sync_start();
  llc.state_fault_cnt++;

  if(llc.state_fault_cnt > 100000)	//延迟、放电  大约1s
    {
      llc.state_fault_cnt = 0;
      clear_fault_state();
//			llc_transition_to_idle_state();
    }

}


RAMCODE
void ShareCurrDutySet(void)
{
  uint32_t u32Tmp;

	    u32Tmp = (uint32_t)((llc.shareloop.rel)/((2.9f-IOUT_OFFSET_VAL)/IOUT_GAIN_VAL)*(4500.0f-1.0f));		//（samp_vol/2.9）* duty
  llc.share_duty = (uint16_t)(u32Tmp);

  if(llc.share_duty >= (4500-1))
    {
      llc.share_duty = (4500-1);
    }
  else if(llc.share_duty <= 2)
    {
      llc.share_duty = 2;
    }

  __LL_TMR_CC2_CmpVal_Set(TMR3,llc.share_duty);


}

RAMCODE
void ShareCurrDutySet_Max(void)
{
  llc.share_duty = (4500-1);
  __LL_TMR_CC2_CmpVal_Set(TMR3,llc.share_duty);

}

RAMCODE
void ShareCurrDutySet_FullLoad()
{
  llc.share_duty = (uint32_t)((50.0f)/((2.9f-IOUT_OFFSET_VAL)/IOUT_GAIN_VAL)*(4500.0f-1.0f));	
  __LL_TMR_CC2_CmpVal_Set(TMR3,llc.share_duty);

}


