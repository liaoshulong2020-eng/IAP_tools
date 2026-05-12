#include "main.h"



void init_all_app(void)
{
		DEM_CR |= (uint32_t)DEM_CR_TRCENA; //那1?邦?迆℅迄??
		DWT_CYCCNT = (uint32_t)0u; //?? CYCCNT ??那y?? 0
		DWT_CR |= (uint32_t)DWT_CR_CYCCNTENA; //那1?邦 CYCCNT ??
	
		//Set NVIC GROUP
		LL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3);
		adc_init_app();
		hrpwm_init_app();
    gpio_init_app();					// GPIO 場宎趙
		dac_init_app();
		cmp_init_app();
		iir_init_app();
		cordic_init_app();
		tmr_init_app();
//		user_uart_init();
//		llc_comm_init();
	
#if(!UART_FUNC)
    User_VOFA_Init();
#else
    user_uart_init();
#endif
		__LL_ADC_REG_Conv_Start(ADC0);
		__LL_ADC_REG_Conv_Start(ADC1);
    hrpwm_start_count();
		LL_TMR_Start_IT(TMR7);
		LL_TMR_Start_IT(TMR8);
//		LL_TMR_Start_IT(TMR3);
		hrpwm_pre_outen();
 //   hrpwm_outen_app();
}


