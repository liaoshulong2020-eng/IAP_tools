#include "main.h"
#include "llc_app.h"

RAMCODE
void vout_over_protect(void)
{
  if(llc.vbus_target > 32.0f)
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
  float vout_set;

  if(llc.state == state_on)
  {
    llc.target_delta = llc.coef_target - llc.vbus_target;

    if(llc.vbus_target <= 22.0f)
    {
      llc.vbus_target = 22.0f;
    }

    if(llc.vbus_target >= 30.0f)
    {
      llc.vbus_target = 30.0f;
    }

    vout_set = llc.vbus_target + llc.vout_hw_trim_delta;

    if(vout_set < 22.0f)
    {
      vout_set = 22.0f;
    }
    else if(vout_set > 30.0f)
    {
      vout_set = 30.0f;
    }

    llc.vbus_ref = vout_set;

    if(vout_set > VOUT_UP_VOLTAGE)
    {
      llc.trim_direct = trim_up;

      // 上调区：1596W / 实时电压
      llc.protection_point.over_current_point =
          (VOUT_VOLTAGE * IOUT_OCP_CURRENT) / vout_set;

      llc.protection_point.over_current_rec_point =
          llc.protection_point.over_current_point -
          (IOUT_OCP_CURRENT - IOUT_REC_TARGET_CURRENT);

      llc.protection_point.short_current_point =
          llc.protection_point.over_current_point +
          (IOUT_SHORT_CURRENT - IOUT_OCP_CURRENT);

      llc.ibus_rec_target =
          llc.protection_point.over_current_rec_point;

      llc.ibus_target =
          llc.protection_point.over_current_point -
          (IOUT_OCP_CURRENT - IOUT_TARGET_CURRENT);

      llc.ibus_ref =
          llc.ibus_target;

      llc.R_Out_ratio_max = 2.0f;
    }
    else if(vout_set < VOUT_DOWM_VOLTAGE)
    {
      llc.trim_direct = trim_down;

      // 下调区：700W / 实时电压
      llc.protection_point.over_current_point =
          DOWM_OCP_WATT / vout_set;

      llc.protection_point.over_current_rec_point =
          IOUT_REC_TARGET_CURRENT;

      llc.protection_point.short_current_point =
          IOUT_SHORT_CURRENT;

      llc.ibus_rec_target =
          llc.protection_point.over_current_rec_point;

      llc.ibus_target =
          IOUT_TARGET_CURRENT;

      llc.ibus_ref =
          llc.ibus_target;

      {
        float x = vout_set;
        float result;

        result = 0.2679f * x * x - 12.2321f * x + 140.0f;

        if(result < 0.0f)
        {
          result = 0.0f;
        }
        else if(result > 10.0f)
        {
          result = 10.0f;
        }

        llc.R_Out_ratio_max = result;
      }
    }
    else
    {
      llc.trim_direct = trim_normal;

      llc.protection_point.over_current_point =
          IOUT_OCP_CURRENT;

      llc.protection_point.over_current_rec_point =
          IOUT_REC_TARGET_CURRENT;

      llc.protection_point.short_current_point =
          IOUT_SHORT_CURRENT;

      llc.ibus_rec_target =
          IOUT_REC_TARGET_CURRENT;

      llc.ibus_target =
          IOUT_TARGET_CURRENT;

      llc.ibus_ref =
          llc.ibus_target;

      llc.R_Out_ratio_max = 8.0f;
    }

			llc.protection_point.under_voltage_point =
					VOUT_UNDER_VOLTAGE +
					llc.vout_trim_delta +
					llc.vout_hw_trim_delta;

			llc.protection_point.under_voltage_point =
					limit_float(llc.protection_point.under_voltage_point, 18.0f, 24.0f);

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
      if(llc.current_mode_cnt > 200)		//切换电流环滤波
        {
//					if(!llc.current_loop_latch)
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
//  else if(llc.iloop.loop_out > llc.vloop.loop_out	|| llc.current_mode_flag )
  else if(llc.iloop.loop_out > llc.vloop.loop_out	 )
    {
      llc_R_vloop(&Volt_Rctrl,llc.vloop.data_error);

      if(llc.iout_rel > IOUT_SR_ON_CURRENT)
        {
          if(llc.pr_open_cnt > 10)
            {
              if(llc.PR_output_cnt > 1000)
                {
                  if(llc.trim_direct == trim_down)
                    {
                      llc.R_Out_ratio += 0.1f;
                    }
                  else
                    {
                      llc.R_Out_ratio++;
                    }

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

          if(llc.R_Out_ratio > llc.R_Out_ratio_max)
            {
              llc.R_Out_ratio = llc.R_Out_ratio_max;
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

//			if(llc.vbus_target < VOUT_TEST_VOLTAGE)
//			{
//				llc.R_Out_ratio = 0;
//			}

      llc.loop_out = llc.vloop.loop_out + (llc.R_Out_ratio * Volt_Rctrl.R_Out);
      if(llc.loop_out < 0)llc.loop_out = 0;
			llc.output_mode = voltage_mode;
    }





//  if(llc.output_mode == voltage_mode)		//恒压模式
//    {
//			llc_R_vloop(&Volt_Rctrl,llc.vloop.data_error);
////      llc.loop_out = llc.vloop.loop_out;// + Volt_Rctrl.R_Out;
//
//			if(llc.iout_rel > 10.0)
//			{
//				if(llc.PR_output_cnt > 1000)
//				{
//					llc.R_Out_ratio++;
//					llc.PR_output_cnt = 0;
//				}
//				else
//				{
//					llc.PR_output_cnt++;
//				}
//
//				if(llc.R_Out_ratio > 8)
//					{
//						llc.R_Out_ratio = 8;
//					}
//					llc.loop_out = llc.vloop.loop_out + (llc.R_Out_ratio * Volt_Rctrl.R_Out);
//			}
//			else
//			{
//			 llc.loop_out = llc.vloop.loop_out;// + Volt_Rctrl.R_Out;
//				llc.PR_output_cnt = 0;
//				llc.R_Out_ratio = 0;
//			}

//      if((llc.iloop.loop_out < llc.vloop.loop_out) &&  (llc.iout_rel > (IOUT_TARGET_CURRENT)))	//电流环输出小于电压环
////			if(llc.iloop.loop_out < llc.vloop.loop_out  || (llc.iout_rel > 86 && (llc.iloop.data_error > 0.1)))	//电流环输出小于电压环
//        {
//          if(llc.current_mode_cnt > 500)		//切换电流环滤波
//            {
//              {
//                llc.output_mode = current_mode;
//                llc.current_mode_cnt = 0 ;
//              }
//            }
//          else
//            {
//              llc.current_mode_cnt ++;
//            }
//        }
//      else
//        {
//          llc.current_mode_cnt = 0;
//        }
//    }
//  else if(llc.output_mode == current_mode)
//    {
//      llc_R_vrest(&Volt_Rctrl);
//			 llc.loop_out = llc.iloop.loop_out;

//      if(llc.vloop.loop_out < llc.iloop.loop_out )	//电压环输出小于电流环
//        {
//          if(llc.voltage_mode_cnt > 100)		//切换电压环滤波
//            {
////							if(!llc.current_loop_latch)
//              {
//                llc.output_mode = voltage_mode;
//                llc.voltage_mode_cnt = 0 ;
//              }
//            }
//          else
//            {
//              llc.voltage_mode_cnt ++;
//            }
//        }
//      else
//        {
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
          llc.loop_out_test++;
        }
    }
  llc.loop_out =	10000;
#endif

  if((llc.iout_rel) >= IOUT_SR_ON_CURRENT && (llc.vbus_ref > 0.95f*llc.vbus_target))
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
  else if((llc.iout_rel) < IOUT_SR_OFF_CURRENT )
    {
      if(llc.sr_close_cnt > 200)
        {
          llc.sr_pwm = 0;		//输出电流大 < 5A	 , SR 关
        }
      else
        {
          llc.sr_close_cnt++;
        }
      llc.sr_open_cnt = 0;
    }
}

uint8_t burst_period;
RAMCODE
void llc_pwm_value_ctrl(void)
{
	if(llc.loop_out < 0)llc.loop_out = 0;
#if PWM_MODE
  if(llc.loop_out > LLC_SW_PERIOD_MAX)		//最低频率  150k
    {
//			llc.loop_out = LLC_SW_PERIOD_MAX;
      llc.period = LLC_SW_PERIOD_MAX;//150k
      llc.duty = (llc.period >> 1) - LLC_DEADTIME_COUNT;			//50% duty
      llc.switch_mode = max_pfm_mode;
    }
  else if(llc.loop_out > LLC_SW_PERIOD_MIN)		//最低频率  300k		9600
    {
      llc.period = llc.loop_out; 					//300k-150k
      llc.duty = (llc.period >> 1) - LLC_DEADTIME_COUNT;			//50%
      llc.switch_mode = pfm_mode;
    }
  else if(llc.loop_out > LLC_DUTY_MIN_COUNT)	//2200
    {
      llc.period = LLC_SW_PERIOD_MIN; 	//300k
      llc.duty = (llc.loop_out >> 1) - LLC_DEADTIME_COUNT;					//1%-47%duty
      llc.switch_mode = pwm_mode;
    }
  else // if(llc.loop_out < LLC_SW_PERIOD_MIN)	//最高频率 300k 打嗝		9600
    {
//		llc.loop_out = LLC_SW_PERIOD_MIN;	//300k
      llc.period = LLC_SW_PERIOD_MIN;
      llc.duty = ( LLC_DUTY_MIN_COUNT / 2) - LLC_DEADTIME_COUNT;//(llc.period>>1) - LLC_DEADTIME_COUNT;		//1% duty
      llc.switch_mode = burst_mode;
      burst_cnt_temp = (llc.loop_out * 10 * 65536 / (( LLC_DUTY_MIN_COUNT / 2) - LLC_DEADTIME_COUNT)) / 65536;	//计算每30个波的能量  相当于10个环路周期打嗝个数  llc.loop_out*30/LLC_DUTY_MIN_COUNT/3
    }
#else
  {
    if(llc.output_mode == voltage_mode)
      {
        if(llc.loop_out > (float)LLC_SW_PERIOD_MAX)												//最低频率  150k
          {
//			llc.loop_out = LLC_SW_PERIOD_MAX;
            llc.period = LLC_SW_PERIOD_MAX;													//150k
            llc.duty = (llc.period >> 1) - LLC_DEADTIME_COUNT;			//50% duty
            llc.switch_mode = max_pfm_mode;
          }
        else if(llc.loop_out > (float)LLC_SW_PERIOD_MIN*1.0f)								//最低频率  350k  //8228
          {
						if(llc.loop_out < LLC_SW_PERIOD_MIN){llc.loop_out = LLC_SW_PERIOD_MIN;}
            llc.period = llc.loop_out; 															//300k-150k
            llc.duty = (llc.period >> 1) - LLC_DEADTIME_COUNT;			//50%
            if(llc.duty < 100) llc.duty = 100;
						llc.switch_mode = pfm_mode;
          }
        else if(llc.loop_out > (float)(LLC_DUTY_MAX_COUNT))		//50%duty 打嗝		//3250
          {
            llc.period = LLC_SW_PERIOD_MIN; 					//350k
            llc.duty = (llc.period >> 1) - LLC_DEADTIME_COUNT;			//50%
						if(llc.duty < 100) llc.duty = 100;
            llc.burst_cnt_temp = (llc.loop_out * 3.0f  / llc.duty);	//计算每8个波的能量  相当于2个环路周期打嗝个数  llc.loop_out*6/LLC_DUTY_MIN_COUNT/3
            llc.switch_mode = duty_pfm_mode;
          }
        else //if(llc.loop_out < ((float)(LLC_BURST_DUTY_MAX_COUNT*0.605f)))	// <1个10%波 脉冲变频发波  //643
          {
            llc.period = LLC_SW_PERIOD_MIN; 					//350k
            llc.duty = ( LLC_BURST_DUTY_COUNT / 2.0f) - LLC_DEADTIME_COUNT;			//50%
						if(llc.duty < 100) llc.duty = 100;
						llc.burst_cnt_temp = (uint32_t)((float)llc.loop_out * 10.0f  / llc.duty);	//计算每20个波的能量  相当于5个环路周期打嗝个数  llc.loop_out*20/LLC_DUTY_MIN_COUNT/4
            llc.switch_mode = burst_mode;
          }
//		else
//		{
//			llc.period = (uint32_t)((llc.loop_out + (float)LLC_DEADTIME_COUNT) * 2.0f);
//			  if(llc.period > LLC_SW_PERIOD_MAX)
//				{
//					llc.period = LLC_SW_PERIOD_MAX	;
//				}
//				else if(llc.period < LLC_SW_PERIOD_MIN)
//				{
//				llc.period = LLC_SW_PERIOD_MIN	;
//				}
//
//      llc.duty = (uint32_t)((float)llc.period * 0.1f);//( LLC_BURST_DUTY_COUNT / 2) - LLC_DEADTIME_COUNT;
//
//      llc.switch_mode = burst_pfm_mode;
//		}
      }
    else
      {
        if(llc.loop_out > (float)LLC_SW_PERIOD_MAX)												//最低频率  150k
          {
//			llc.loop_out = LLC_SW_PERIOD_MAX;
            llc.period = LLC_SW_PERIOD_MAX;													//150k
            llc.duty = (llc.period >> 1) - LLC_DEADTIME_COUNT;			//50% duty
            llc.switch_mode = max_pfm_mode;
          }
        else if(llc.loop_out > (float)LLC_SW_PERIOD_MIN)								//最低频率  350k
          {
            llc.period = (uint32_t)llc.loop_out; 															//300k-150k
            llc.duty = (llc.period >> 1) - LLC_DEADTIME_COUNT;			//50%
            llc.switch_mode = pfm_mode;
          }
        else 		//50%duty 打嗝
          {
            llc.period = LLC_SW_PERIOD_MIN; 					//350k
            llc.duty = (llc.period >> 1) - LLC_DEADTIME_COUNT;			//50%
            llc.burst_cnt_temp = (uint32_t)(llc.loop_out * 20.0f  / llc.duty);	//计算每8个波的能量  相当于2个环路周期打嗝个数  llc.loop_out*6/LLC_DUTY_MIN_COUNT/3
            llc.switch_mode = duty_pfm_mode;
          }
      }
  }
#endif



//  else if(llc.loop_out > LLC_DUTY_MIN_COUNT)
//    {
//      llc.period = LLC_SW_PERIOD_MIN; 	//300k
//      llc.duty = (llc.loop_out >> 1) - LLC_DEADTIME_COUNT;					//1%-47%duty
//      llc.switch_mode = pwm_mode;
//    }
//  else if(llc.loop_out < LLC_SW_PERIOD_MIN)	//最高频率 300k 打嗝
//    {
////		llc.loop_out = LLC_SW_PERIOD_MIN;	//300k
//      llc.period = LLC_SW_PERIOD_MIN;
//      llc.duty = (LLC_SW_PERIOD_MIN >> 1) - LLC_DEADTIME_COUNT;//(llc.period>>1) - LLC_DEADTIME_COUNT;		//1% duty
//      llc.switch_mode = burst_mode;
//    }

  if(llc.switch_mode != burst_mode && llc.switch_mode !=duty_pfm_mode)
    {
      llc.burst_cnt = 5;
    }
  else
    {
      if(llc.burst_period > 5)
        {

          llc.burst_period = 0;
          llc.burst_cnt = llc.burst_cnt_temp;
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

  if(llc.on_off_ctrl_ok && llc.idel_delay_ok &&  llc.pfc_is_ok && llc.protec_set_flag && (!llc.fault_state.all) &&llc.start_contr )
    {
      llc.vbus_ref = llc.vbus_rel;
      enable_current_sharing();		//发出同步信号

      llc.state = state_rampup;
      hrpwm_outen_app();
      ShareCurrDutySet_Max();
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
              if(llc.ramup_delay_cnt > 1000)
                {

                  llc.state = state_on;						//进入下一状态
                  llc.ramup_delay_cnt = 0;
                }
            }
          else
            {
//							if(llc.vbus_ref < 3.0f)
//							{
//							 llc.vbus_ref = llc.vbus_ref + (float)0.0001;
//							}
//							else
              {
                llc.vbus_ref = llc.vbus_ref + (float)0.001;//未达到设定的目标值则继续增加
              }
            }

          if(llc.vbus_ref > 5.0f)
            {
              LL_CMP_Start(CMP3);
            }
          else
            {
              LL_CMP_Stop(CMP3);
            }
						ShareCurrDutySet_FullLoad();
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

      if(llc.test_cnt > 5000)
        {
          LL_DAC_ValueSet(DAC0,IOUT_OCP_CURRENT_DAC_DAC_VALUE);
          llc.ibus_ref = llc.ibus_target;
//				LL_CMP_Start(CMP3);

//				llc.state = state_fault;
        }
      if(llc.test_cnt > 10000)
        {
          llc.ocp_time = 0;
        }

//      if(llc.output_mode == voltage_mode)
//				if(llc.shareloop.data_error + 2.0f < 0 )
//				{
//					ShareCurrDutySet_Max();
//				}
//				else
//				{
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

  llc.state_fault_cnt++;

  if(llc.state_fault_cnt > 100000)//20000)//32767)		//延迟、放电
    {
      llc.state_fault_cnt = 0;
      clear_fault_state();
    }
		
  if(llc.on_off_ctrl_fault)		
    {
      llc.state_fault_cnt = 0;
      clear_fault_state();
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
  llc.share_duty = (uint32_t)((45.0f)/((2.9f-IOUT_OFFSET_VAL)/IOUT_GAIN_VAL)*(4500.0f-1.0f));	
  __LL_TMR_CC2_CmpVal_Set(TMR3,llc.share_duty);

}


