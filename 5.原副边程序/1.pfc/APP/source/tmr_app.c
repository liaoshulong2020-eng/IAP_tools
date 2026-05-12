#include "main.h"

extern int32_t theta_index;
void tmr_init_app(void)
{
  LL_TMR_Init(TMR7);
  LL_TMR_Init(TMR8);
	LL_TMR_Init(TMR3);
	
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

//  tmr_base_init.prescaler = 9000 - 1; // 20k
//	tmr_base_init.period = 4000*5 - 1;		//1Hz
//  LL_TMR_Base_Cfg(TMR3, &tmr_base_init);

  LL_NVIC_SetPriority(TMR8_IRQn, 2, 0);
  LL_NVIC_SetPriority(TMR7_IRQn, 1, 0);
//  LL_NVIC_SetPriority(TMR3_IRQn, 5, 0);
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
      rectify_vac();						//正负半周判断
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

		
		if(!pfc.time.test_1)
			{
				pfc.time.tok = DWT_CYCCNT;
				pfc.time.cycle_100k = (pfc.time.tok - pfc.time.tik);
				pfc.time.cycle_100k = pfc.time.cycle_100k*5/1000;
				pfc.time.test_1 = 1;
			}
// 		 GPIOA->DOR &= ~(1 << 10);	//TEST_PIN_LOW
}
void task_1s(void);
void main_loop(void);
extern void main_loop(void);
volatile uint8_t  flag_1s   = 0;
extern int32_t theta_index;
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
				
      check_current_zero_offset();//零电流漂移校准

      led_breath();							//呼吸灯

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

			pfc.vbus_notch_filer_data = applyFilter(pfc.vbus_rel);
//			pfc.vin_rel = sqrt(pfc.vin_squares_average) ;
//			pfc.vout_rel_value = pfc.vbus_rel*0.052;
//			 collectData(pfc.vbus_rel);
//			analyzeHarmonics((float)pfc.vin_voltage);
//			SPLL_1PH_NOTCH_RUN(&spll_1ph_notch,pfc.acl_value);
//			LL_IIR_Calc_Multi(IIR1, &vbus_notch_in, 1, IIR_COEF_GRP_AUTO, (int16_t*)&vbus_notch_out,300);
//			SPLL_Update(&spll, pfc.vin_voltage); // Update SPLL with new sample

#if(!UART_FUNC)
			tx_vofa_data.f[0] =	pfc.vbus_ref;//pfc.k_forward;//pfc.iloop.integral;//(float)pfc.duty*100/PFC_DRIVER_PERIOD_VALUE;;//;//;//pfc.vloop.loop_out;//pfc.state;//pfc.vout_rel_value;//pfc.vin_rel_value;//pfc.vbus_rel;//pfc.duty;
      tx_vofa_data.f[1] =	pfc.vbus_rel;////;//(pfc.vin_squares_average>>11);
      tx_vofa_data.f[2] =	pfc.iloop.ref;//;////pfc.iloop.ref;//pfc.vbus_notch_filer_data;//pfc.iloop.ref;
      tx_vofa_data.f[3] =	pfc.iloop.rel;//pfc.vloop.kp;//pfc.iloop.p_out;//pfc.vbus_ref;//pfc.state;//pfc.vloop.loop_out;//pfc.iloop.loop_out;//pfc.iloop.rel;//pfc.k_forward;//pfc.vin_rel ;//pfc.fault_num ;//pfc.vin_rel_value;//
      tx_vofa_data.f[4] = pfc.acl_value; ;//pfc.k_forward;//check_current_is_ok;//(float)pfc.duty*100/PFC_DRIVER_PERIOD_VALUE;;//pfc.vbus_target;//(2 * 3.14159 * 2608 * 50 * 4);
      tx_vofa_data.f[5] =	pfc.acn_value;
			tx_vofa_data.f[6] =	pfc.vin_voltage;
			tx_vofa_data.f[7] =	pfc.iloop.loop_out;
			tx_vofa_data.f[8] =	pfc.vloop.loop_out;
      user_vofa_tx();
#endif
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
