#include "main.h"

void task_1s(void);
void main_loop(void);
extern void main_loop(void);
volatile uint8_t  flag_1s   = 0;
extern int32_t theta_index;
void tmr_init_app(void)
{
  LL_TMR_Init(TMR7);
  LL_TMR_Init(TMR8);

  TMR_BaseInitTypeDef tmr_base_init;

  memset((void*)&tmr_base_init, 0x00, sizeof(tmr_base_init));

  tmr_base_init.auto_preload_en = true;
  tmr_base_init.work_mode = TMR_WORK_MODE_CONTINUE;
  tmr_base_init.period = 200 - 1;		//50k
  tmr_base_init.prescaler = 9 - 1; // 10M
  tmr_base_init.update_evt_en = false;
  tmr_base_init.update_evt_src = TMR_UPDATE_EVT_SRC_OV;

  LL_TMR_Base_Cfg(TMR7, &tmr_base_init);


  tmr_base_init.period = 1000 - 1;		//10k
  LL_TMR_Base_Cfg(TMR8, &tmr_base_init);

  LL_NVIC_SetPriority(TMR8_IRQn, 2, 0);
  LL_NVIC_SetPriority(TMR7_IRQn, 1, 0);

}

//50k
RAMCODE
void TMR7_IRQHandler(void)
{
  if((TMR7->IER & (1 << 9)) && (TMR7->SR & (1 << 9)))
    {
      __LL_TMR_OverflowIntPnd_Clr(TMR7);

//			GPIOA->DOR |= (1 << 10);	//TEST_PIN_HIGH
		
			if(!pfc.time.test_1)
			{
				DWT_CYCCNT = 0;
				pfc.time.tik = DWT_CYCCNT;
			}

      get_adc_value();
			K_forward_calcula();
      if((pfc.vbus_rel) > VOUT_OVER_VOLTAGE)
        {
          hrpwm_outdis_app();
          pfc.close_pwm_flag |= (1 << 1);
          pfc.state = State_Fault;
        }
      state_switch();

//		pfc.iloop.ref = 	(pfc.vloop.loop_out * pfc.vin_voltage / (pfc.vin_squares_average))	 ;

      if(pfc.state == State_Rampup || pfc.state == State_On)
        {
          pfc_iloop_handle();
        }
      else
        {
          pfc_iloop_init();
        }
      hrpwm_update_app();
    }
		
#if(!UART_FUNC)
			tx_vofa_data.f[0] = pfc.vbus_ref;
			tx_vofa_data.f[1] = pfc.vbus_rel;
			tx_vofa_data.f[2] = pfc.vloop.loop_out;
			tx_vofa_data.f[3] = pfc.iloop.ref;
			tx_vofa_data.f[4] = pfc.iloop.rel;
			tx_vofa_data.f[5] = pfc.iloop.loop_out;
			tx_vofa_data.f[6] = pfc.vbus_move_filter_value;
			tx_vofa_data.f[7] = pfc.vbus_value;
			tx_vofa_data.f[8] = pfc.vin_voltage;
			tx_vofa_data.f[9] = pfc.vbus_notch_rel;
      user_vofa_tx();
#endif
		if(!pfc.time.test_1)
			{
				pfc.time.tok = DWT_CYCCNT;
				pfc.time.cycle_100k = (pfc.time.tok - pfc.time.tik);
				pfc.time.cycle_100k = pfc.time.cycle_100k*5/1000;
				pfc.time.test_1 = 1;
			}
// 		 GPIOA->DOR &= ~(1 << 10);	//TEST_PIN_LOW
}

//10k
RAMCODE
void TMR8_IRQHandler(void)
{
  if((TMR8->IER & (1 << 9)) && (TMR8->SR & (1 << 9)))
    {

      __LL_TMR_OverflowIntPnd_Clr(TMR8);
						if(pfc.time.test_1)
			{
				DWT_CYCCNT = 0;
				pfc.time.tik = DWT_CYCCNT;
			}
			pfc.vbus_notch_rel = notch_tick(&pfc.notch_100hz, pfc.vbus_value);
		// -----------------------------
    // VBUS 滤波输出
    // -----------------------------
			pfc.vbus_rel = pfc.vbus_notch_rel;
			
      check_current_zero_offset();//零电流漂移校准

      led_breath();							//呼吸灯
			pfc_fast_input_under_protect();
      rectify_vac();						//正负半周判断
      half_cycle_processing();	//正负半周数据处理
      input_mode_check();				//输入模式判断
			set_protect_point();			//设置保护点
      vbus_filter();						//母线滤波
      fault_check_app();				//故障检测
      pfc_handle();							//PFC状态机
				
	if(pfc.vloop.loop_out < 100)
		{
			pfc.vbus_value_error_th = 1;
		}
		else if(pfc.vloop.loop_out > 120)
		{
			pfc.vbus_value_error_th	= 1 + (float)0.005*pfc.vloop.loop_out;
		}
		else
		{
			pfc.vbus_value_error_th	= 1;
		}


		static uint16_t div_1s = 0;
		if (++div_1s >= 1000)
		{
			div_1s = 0;
			flag_1s = 1;
		}
	
		
		if(pfc.time.test_1)
			{
			pfc.time.tok = DWT_CYCCNT;
			pfc.time.cycle_10k = (pfc.time.tok - pfc.time.tik);		//us
			pfc.time.cycle_10k = pfc.time.cycle_10k*5/1000;//
			pfc.time.test_1 = 0;
			}

    }
}
RAMCODE
void main_loop(void)
{
  if (flag_1s)
    {
      flag_1s = 0;
      task_1s();
    }
}

RAMCODE
void task_1s(void)
{
#if(UART_FUNC)
  	uart_send_pfc_detail_info();
		DMA->CH[0].REG.CER = 1;
#endif
}