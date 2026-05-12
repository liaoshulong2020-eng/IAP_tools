#include "main.h"
#include "protection_app.h"

RAMCODE
void over_output_voltage_check_app(void)		//输出过压
{
  if(llc.vbus_rel > VOUT_OVER_VOLTAGE)
    {
      llc.fault_cnt.over_voltage_cnt++;
    }
  else
    {
      llc.fault_cnt.over_voltage_cnt = 0;
    }

  if(llc.fault_cnt.over_voltage_cnt > 10)
    {
      llc.fault_state.bit.over_voltage = 1 ;
    }
}

RAMCODE
void under_output_voltage_check_app(void)		//输出欠压
{
  if(llc.state == state_on)									//完成稳压后才开始检测欠压
    {
      llc.state_on_cnt++;
      if((llc.vbus_rel < llc.protection_point.under_voltage_point) && (llc.state_on_cnt > 100))
        {
          llc.fault_cnt.under_voltage_cnt++;
        }
      else
        {
          llc.fault_cnt.under_voltage_cnt = 0;
        }

				if((llc.output_mode == current_mode) && (llc.vbus_rel < (llc.protection_point.under_voltage_point + 2.0f)))
				{
						llc.fault_cnt.under_voltage_cnt+=50;
				}
				else
        {
          llc.fault_cnt.under_voltage_cnt = 0;
        }
		
      if(llc.fault_cnt.under_voltage_cnt > 100)
        {
          llc.state = state_fault;
          llc.fault_state.bit.under_voltage = 1 ;
					llc.ibus_ref = llc.ibus_rec_target;
					LL_DAC_ValueSet(DAC0,IOUT_OCP_REC_CURRENT_DAC_DAC_VALUE);
        }
    }
		

}

RAMCODE
void over_current_check_app(void)
{
  if(!llc.fault_state.bit.over_current)
    {
      if(llc.iout_rel > llc.protection_point.over_current_point)
        {
          if(llc.iout_rel > llc.protection_point.short_current_point)
            {
              llc.short_current_cnt++;
              if(llc.short_current_cnt > 2)
                {
                  llc.short_current_flag = 1;
                }
            }
          else
            {
              llc.short_current_cnt = 0;
            }

          llc.fault_cnt.over_current_cnt++;

          if((llc.fault_cnt.over_current_cnt > 1) && (llc.fault_cnt.over_current_cnt < 200))
            {
//              llc.current_loop_latch = 1;
//              llc.output_mode = current_mode;
//              if(llc.switch_mode == burst_mode)
//                {
                  llc.loop_out = 0;
//                }
//              else
//                {
//                  llc.period = LLC_SW_PERIOD_MIN;
//                }
            }
//          else
//            {
//              llc.current_loop_latch = 0;
//            }
        }
      else // if(llc.iout_rel < IOUT_REC_CURRENT)
        {
          llc.fault_cnt.over_current_cnt = 0;
        }

      if((llc.fault_cnt.over_current_cnt > 10) || (llc.short_current_cnt > 2))
        {
          {
						hrpwm_outdis_app();
						hrpwm_srdis_app();
            llc.fault_state.bit.over_current = 1 ;//	过流
						llc.state = state_fault;
						LL_DAC_ValueSet(DAC0,IOUT_OCP_REC_CURRENT_DAC_DAC_VALUE);
						llc.ibus_ref = llc.ibus_rec_target;
						
          }

          llc.fault_cnt.over_current_cnt = 0;
          llc.short_current_cnt = 0;
        }
    }
//  else	//过流 打嗝
//    {
//      llc.fault_cnt.over_current_cnt++;
//      if(llc.fault_cnt.over_current_cnt > 360000)
//        {
//          llc.ocp_burst_cnt++;
//          if(llc.ocp_burst_cnt >= 1)
//            {
//              {
////                llc.ocp_time++;
//              }
//              
//              
//              llc.ocp_burst_cnt = 0;
//            }

//          llc.fault_cnt.over_current_cnt = 0;
//        }
//    }
}

RAMCODE
void over_temp_check_app(void)
{
  if(llc.temp_c > llc.protection_point.over_temp_point)	//过温
    {
      if(llc.state == state_on)
        {
					if(llc.fault_cnt.over_temp_cnt > 100)
					{
						llc.fault_state.bit.over_temp = 1;
					}
					else
					{
						llc.fault_cnt.over_temp_cnt++;
					}
        }
    }
		else
		{
			llc.fault_cnt.over_temp_cnt = 0;
		}
		
		if(llc.fault_state.bit.over_temp)
		{ 
			if(llc.temp_c <  llc.protection_point.over_temp_rec_point && llc.temp_recover_mode)
			{
				if(llc.fault_cnt.over_temp_rec_cnt > 100)
				{
					llc.fault_state.bit.over_temp = 0;
					llc.fault_cnt.over_temp_cnt = 0;
					llc.fault_cnt.over_temp_rec_cnt = 0;
				}
				else
				{
					llc.fault_cnt.over_temp_rec_cnt++;
				}
			}
			else
			{
					llc.fault_cnt.over_temp_rec_cnt = 0;
			}
		}
}


RAMCODE
void Hw_over_Curr_check_app(void)			//硬件过流
{
//  static uint16_t s_u16EnterCnt = 0;
//  static uint16_t s_u16ExitCnt = 0;
//  if(!llc.fault_state.bit.over_current)
//    {

//      if(llc.HwOcp == 1)
//        {
////          llc.HwOcp = 0;
//          s_u16EnterCnt++;
//          if(s_u16EnterCnt > 10)
//            {
//              s_u16EnterCnt = 0;
//              llc.fault_state.bit.over_current = 1;
//              llc.state = state_fault;
//              LL_CMP_Stop(OCP_CMP);
//              llc.fault1_cnt = 0;
//            }
//        }
//      s_u16ExitCnt = 0;
//    }
//  else
//    {
//      s_u16ExitCnt++;
//      if(s_u16ExitCnt > 10)
//        {
//          s_u16ExitCnt = 0;
//          llc.ocp_time++;
//          llc.fault_state.bit.over_current = 0;
//        }
//      s_u16EnterCnt = 0;
//    }
}


RAMCODE
void fault_check_app(void)
{
#if TEST_MODE
  llc.pfc_is_ok = 1;
//  llc.on_off_ctrl_ok = 1 ;
  on_off_ctrl_check_app();
  over_output_voltage_check_app();
//	under_output_voltage_check_app();
//  over_current_check_app();
#else
  Hw_over_Curr_check_app();
  over_output_voltage_check_app();
  under_output_voltage_check_app();
  over_current_check_app();
  over_temp_check_app();
  on_off_ctrl_check_app();
//	llc.pfc_is_ok = 1;
//	llc.on_off_ctrl_ok = 1 ;
  pfc_state_check_app();
#endif
}

RAMCODE
void clear_fault_state(void)
{
  if(!llc.fault_state.bit.over_temp)
    {
      if(llc.fault_state.bit.under_voltage )
			{
				llc.fault_state.bit.under_voltage = 0;
			}			
			if(llc.fault_state.bit.over_current)
			{
				llc.fault_state.bit.over_current = 0 ;
			}
			
			llc.ocp_time++;
    }
  if((!llc.fault_state.all) && (llc.ocp_time <= 5))
    {
			llc.HwOcp = 0;
				
      reset_value();
    }
}

RAMCODE
int no_fault(void)
{
  if(llc.fault_state.bit.over_voltage |  llc.fault_state.bit.under_voltage | llc.fault_state.bit.over_current | llc.fault_state.bit.over_temp)
    {
      return 0;
    }
  else
    {
      return 1;
    }

}

