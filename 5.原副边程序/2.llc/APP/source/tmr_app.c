#include "main.h"
#include "tmr_app.h"

volatile uint32_t tick_10k   = 0;
volatile uint8_t  flag_1ms   = 0;
volatile uint8_t  flag_10ms  = 0;
volatile uint8_t  flag_100ms = 0;
volatile uint8_t  flag_1s    = 0;

void loadshare_pwm_init(void)
{

  GPIO_InitTypeDef User_GPIO_Init;

  User_GPIO_Init.Alternate = GPIO_AF3_TMR3;
  User_GPIO_Init.Pull      = GPIO_NOPULL;
  User_GPIO_Init.IntMode   = GPIO_INT_MODE_CLOSE;
  User_GPIO_Init.OType     = GPIO_OTYPE_PP;
  User_GPIO_Init.Speed     = GPIO_SPEED_FREQ_HIGH;

  User_GPIO_Init.Pin = GPIO_PIN_2;
  LL_GPIO_Init(GPIOA, &User_GPIO_Init);		//LED

  LL_TMR_Init(TMR3);

  TMR_BaseInitTypeDef tmr_base_init;
  TMR_OutputCmpInitTypeDef	tmr_cmp_init;

  memset((void*)&tmr_base_init, 0x00, sizeof(tmr_base_init));
  memset((void*)&tmr_cmp_init, 0x00, sizeof(tmr_cmp_init));

  tmr_base_init.auto_preload_en = true;
  tmr_base_init.work_mode = TMR_WORK_MODE_CONTINUE;
  tmr_base_init.period = 4500 - 1;		//40k
  tmr_base_init.prescaler = 1 - 1; // 180M
  tmr_base_init.update_evt_en = false;
  tmr_base_init.update_evt_src = TMR_UPDATE_EVT_SRC_OV_UG;

  LL_TMR_Base_Cfg(TMR3, &tmr_base_init);


  tmr_cmp_init.auto_preload_en = true;
  tmr_cmp_init.mode = TMR_CMP_MODE_PWM1;
  tmr_cmp_init.OC_en = true;
  tmr_cmp_init.OC_idle_sta = TMR_OUTPUT_IDLE_STA_LOW;
  tmr_cmp_init.match_val = 100;

  LL_TMR_OutputCmp_Cfg(TMR3, TMR_CAP_CMP_CH_2, &tmr_cmp_init);
}


void tmr_init_app(void)
{
  LL_TMR_Init(TMR7);
  LL_TMR_Init(TMR8);
  LL_TMR_Init(TMR9);

  TMR_BaseInitTypeDef tmr_base_init;

  memset((void*)&tmr_base_init, 0x00, sizeof(tmr_base_init));

  tmr_base_init.auto_preload_en = true;
  tmr_base_init.work_mode = TMR_WORK_MODE_CONTINUE;
  tmr_base_init.period = 100 - 1;		//100k
  tmr_base_init.prescaler = 9 - 1; // 10M
  tmr_base_init.update_evt_en = false;
  tmr_base_init.update_evt_src = TMR_UPDATE_EVT_SRC_OV;

  LL_TMR_Base_Cfg(TMR7, &tmr_base_init);

  LL_TMR_Base_Cfg(TMR9, &tmr_base_init);

  tmr_base_init.period = 1000 - 1;		//10k
  LL_TMR_Base_Cfg(TMR8, &tmr_base_init);


  TMR_SlvInitTypeDef 	TMR_SlvInitTypeStruct;

//	TMR_SlvInitTypeStruct.slv_mode =
//	TMR_SlvInitTypeStruct.etr_edge_mode =
//	TMR_SlvInitTypeStruct.etr_filter =
//	TMR_SlvInitTypeStruct.etr_input_src =
//	TMR_SlvInitTypeStruct.etr_mode =
//	TMR_SlvInitTypeStruct.fast_sync_en =
//	TMR_SlvInitTypeStruct.trig =
//	LL_TMR_Slv_Cfg(TMR0,&TMR_SlvInitTypeStruct);

  TMR7->CR1 |= (1);
  TMR9->CR1 |= (1);
}

RAMCODE
void llc_filter()
{
  llc.vout_value 	= (llc.vout_value_store[0]) / VOUT_FILTER_ORDER *1.0f+0.0f;
  llc.iout_value_1 	= llc.iout_value/ IOUT_FILTER_ORDER;//(llc.iout_value_store[0] + llc.iout_value_store[1] + llc.iout_value_store[2] + llc.iout_value_store[3]) / 16;
  llc.vbus_rel		= llc.vout_value;
//  llc.iout_rel 		= (llc.iout_value / IOUT_FILTER_ORDER);


//  if((llc.iout_value / IOUT_FILTER_ORDER) < 14.0f)
//    {
//      llc.iout_rel 		= ((llc.iout_value / IOUT_FILTER_ORDER) * ((float)(0.9232)) + ((float)(-2.9917)));
//    }
//  else 
//		if((llc.iout_value_1 ) > 90.0f)
//    {
//      llc.iout_rel 		= ((llc.iout_value_1 ) - 2.0) ;
//    }
//		else
//		{
//		 	llc.iout_rel 		= (llc.iout_value_1 )  ;
//		}
	llc.iout_rel 		= (llc.iout_value_1 )  ;
	
  if(llc.iout_rel < 0)
    {
      llc.iout_rel = 0;
    }

}



//100k
RAMCODE
void TMR7_IRQHandler(void)
{
  if((TMR7->IER & (1 << 9)) && (TMR7->SR & (1 << 9)))
    {
      __LL_TMR_OverflowIntPnd_Clr(TMR7);

      if(!llc.time.test_1)
        {
          DWT_CYCCNT = 0;
          llc.time.tik = DWT_CYCCNT;
        }

      get_adc_value();

      llc_filter();

      if(llc.start_delay_cnt < START_DELAY_VALUE)
        {
          llc.start_delay_cnt++;
        }
      else
        {
          fault_check_app();

          llc_handle();

          hrpwm_update_app();
        }


				
      if(!llc.time.test_1)
        {
          llc.time.tok = DWT_CYCCNT;
          llc.time.cycle_100k = (llc.time.tok - llc.time.tik);
          llc.time.cycle_100k = llc.time.cycle_100k * 5 / 1000;
          llc.time.test_1 = 1;
        }
    }
}

//10k
RAMCODE
void TMR8_IRQHandler(void)
{
  if((TMR8->IER & (1 << 9)) && (TMR8->SR & (1 << 9)))
    {
      __LL_TMR_OverflowIntPnd_Clr(TMR8);
      if(llc.time.test_1)
        {
          DWT_CYCCNT = 0;
          llc.time.tik = DWT_CYCCNT;
        }

				send_massage_get();
//			 llc.pr_output = updatePRController(&ctrl, llc.vbus_ref - llc.vbus_rel);
//				llc.loadshare_value =llc.loadshare_value_store[0]+llc.loadshare_value_store[1] >> 10;
     
				llc.div_freq_1k++;
				llc.div_freq_1s++;
				
      if(llc.div_freq_1k == 1)
        {
          vout_config();
        }

      if(llc.div_freq_1k == 2)
        {
          data_slow_filter();
        }
      if(llc.div_freq_1k == 3)
        {
					read_ac_alarm_app();
        }
      if(llc.div_freq_1k == 4)
        {
				vout_over_protect();
        }
      if(llc.div_freq_1k == 5)
        {
					
        }
      if(llc.div_freq_1k == 6)
        {

        }
      if(llc.div_freq_1k == 7)
        {

        }
      if(llc.div_freq_1k == 8)
        {

        }
      if(llc.div_freq_1k == 9)
        {

        }
      if(llc.div_freq_1k == 10)
        {
          llc.div_freq_1k = 0;
        }
				
			if(llc.div_freq_1s == 10000)
			{
				llc.div_freq_1s  = 0;
			}
				
			static uint16_t div_1ms = 0, div_10ms = 0, div_100ms = 0, div_1s = 0;
      if (++div_1ms >= 10)
        {
          div_1ms = 0;
          flag_1ms = 1;
        }
      if (++div_10ms >= 100)
        {
          div_10ms = 0;
          flag_10ms = 1;
        }
      if (++div_100ms >= 1000)
        {
          div_100ms = 0;
          flag_100ms = 1;
        }
      if (++div_1s >= 10000)
        {
          div_1s = 0;
          flag_1s = 1;
        }

      if(llc.time.test_1)
        {
          llc.time.tok = DWT_CYCCNT;
          llc.time.cycle_10k = (llc.time.tok - llc.time.tik);		//us
          llc.time.cycle_10k = llc.time.cycle_10k * 5 / 1000; //
          llc.time.test_1 = 0;
        }
    }
}
RAMCODE
void task_1ms(void)
{
      tx_vofa_data.f[0] =	llc.ibus_target;
      tx_vofa_data.f[1] = llc.vbus_rel;
      tx_vofa_data.f[2] =	llc.iout_rel;
      tx_vofa_data.f[3] = llc.fault_state.all; 
      tx_vofa_data.f[4] = llc.ibus_ref;//llc.vloop.p_out;
      tx_vofa_data.f[5] = llc.ocp_time;//llc.switch_mode;
      tx_vofa_data.f[6] = llc.protection_point.over_current_rec_point;
      tx_vofa_data.f[7] =	llc.HwOcp;//llc.vloop.integral_en_k;
			tx_vofa_data.f[8] = llc.output_mode;
			tx_vofa_data.f[9] = llc.state_fault_cnt;

//			tx_vofa_data.f[0] =	llc.vbus_rel;
//      tx_vofa_data.f[1] = llc.share_duty;
//      tx_vofa_data.f[2] =	llc.shareloop.ref	;
//      tx_vofa_data.f[3] = llc.shareloop.rel ; 
//      tx_vofa_data.f[4] = llc.delta_voltage ;
//      tx_vofa_data.f[5] = llc.shareloop.out_max;
//      tx_vofa_data.f[6] = llc.shareloop.kp;
//      tx_vofa_data.f[7] = llc.shareloop.ki;
//			tx_vofa_data.f[8] = llc.shareloop_kp_init;
//			tx_vofa_data.f[9] = llc.shareloop_ki_init;

//			tx_vofa_data.f[0] =	llc.vbus_ref;
//			tx_vofa_data.f[1] = llc.vbus_rel;
//			tx_vofa_data.f[2] =	llc.vloop.loop_out ;
//			tx_vofa_data.f[3] = llc.ibus_ref;
//			tx_vofa_data.f[4] = llc.iout_rel;
//			tx_vofa_data.f[5] = llc.iloop.loop_out;
//			tx_vofa_data.f[6] = llc.R_Out_ratio;
//			tx_vofa_data.f[7] =	llc.protection_point.over_current_point;//llc.fault_state.all;
//			tx_vofa_data.f[8] = llc.ibus_rec_target;
//			tx_vofa_data.f[9] = llc.protection_point.under_voltage_point;

				user_vofa_tx();

}
RAMCODE	
void task_10ms(void)
{
	send_massage_get();
}
RAMCODE	
void task_100ms(void)
{
	data_slow_filter();
	if((!llc.addr_set_flag))
		{
			llc.addr_cnt++; 
			if(llc.addr_cnt > 500)
				{
					can_addr_set();
				}
		}
}
RAMCODE
void task_1s(void)
{

}
RAMCODE
void main_loop(void)
{
  if (flag_1ms)
    {
      flag_1ms = 0;
      task_1ms();
    }
  if (flag_10ms)
    {
      flag_10ms = 0;
      task_10ms();
    }
  if (flag_100ms)
    {
      flag_100ms = 0;
      task_100ms();
    }
  if (flag_1s)
    {
      flag_1s = 0;
      task_1s();
    }
}

