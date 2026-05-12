#include "main.h"
#include "init_app.h"

void flash_data_init(void)
{
	user_data.under_voltage_point			=	VOUT_UNDER_VOLTAGE;
	user_data.under_voltage_rec_point	= 0;
	user_data.over_voltage_point				= VOUT_OVER_VOLTAGE;
	user_data.over_voltage_rec_point		= 0;
	user_data.over_current_point				= IOUT_OCP_CURRENT;
	user_data.over_current_rec_point		= 0;
	user_data.over_temp_point 					= TEMP_MAX_VALUE;
	user_data.over_temp_rec_point 			= TEMP_REC_VALUE;
	user_data.temp_recover_mode				= 1;
	user_data.vbus_target							= VOUT_VOLTAGE;
	user_data.coef_target							= user_data.vbus_target;
	user_data.vout_can_delta					= 0;
	
}

void load_data_app(void)
{
	load_data_from_flash(&user_data);
	
	llc.protection_point.under_voltage_point			=		 user_data.under_voltage_point;					//Ƿѹ������
	llc.protection_point.under_voltage_rec_point 	=		 user_data.under_voltage_rec_point;			//Ƿѹ�ָ���
		
	llc.protection_point.over_voltage_point 			=		 user_data.over_voltage_point;					//��ѹ������
	llc.protection_point.over_voltage_rec_point 	=		 user_data.over_voltage_rec_point;			//��ѹ�ָ���
		
	llc.protection_point.over_current_point 			=		 user_data.over_current_point;					//����������
	llc.protection_point.over_current_rec_point 	=		 user_data.over_current_rec_point;			//�����ָ���	
			
	llc.protection_point.over_temp_point 					=		 user_data.over_temp_point;							//���±�����
	llc.protection_point.over_temp_rec_point 			=		 user_data.over_temp_rec_point;					//���»ָ���	
	llc.temp_recover_mode 												=		 user_data.temp_recover_mode;						//���»ָ�ģʽ
	
	llc.coef_target																=		 user_data.coef_target;
	llc.vbus_target 															=		 llc.coef_target;									//���Ŀ���ѹ
	llc.can_com_voltag_delta											=		 user_data.vout_can_delta;							//�ϱ�ͨ�����
	
}

void save_data_flash(void)
{
//	 memcpy(&flash_data, &user_data, sizeof(flash_data));
	
//	flash_data.under_voltage_point			=		user_data.under_voltage_point;
//	flash_data.under_voltage_rec_point 	= 	user_data.under_voltage_rec_point;
//	flash_data.over_voltage_point				= 	user_data.over_voltage_point;
//	flash_data.over_voltage_rec_point		= 	user_data.over_voltage_rec_point;
//	flash_data.over_current_point				= 	user_data.over_current_point;
//	flash_data.over_current_rec_point		= 	user_data.over_current_rec_point;
//	flash_data.over_temp_point 					= 	user_data.over_temp_point;
//	flash_data.over_temp_rec_point 			= 	user_data.over_temp_rec_point;
//	flash_data.temp_recover_mode				= 	user_data.temp_recover_mode;
//	flash_data.vbus_target							= 	user_data.vbus_target;
}


void init_all_app(void)
{
//	 if(load_data_from_flash(&flash_data))
//	 {
//		load_data_app();
//	 }
//	 else
//	 {
//		flash_data_init();
//	 }
	 
	DEM_CR |= (uint32_t)DEM_CR_TRCENA; //��1?��?������??
	DWT_CYCCNT = (uint32_t)0u; //?? CYCCNT ??��y?? 0
	DWT_CR |= (uint32_t)DWT_CR_CYCCNTENA; //��1?�� CYCCNT ??
  adc_init_app();
  hrpwm_init_app();
	gpio_init_config();
	dac_init_app();
	cmp_init_app();
//		iir_init_app();
//		cordic_init_app();
  tmr_init_app();

	loadshare_pwm_init();
#if(!UART_FUNC)
  User_VOFA_Init();
#else
  User_VOFA_Init();
  can_init_app();
  user_uart_init(USER_UART);  /* 初始化UART，用于接收原边PFC保护点数据 */
#endif
	can_send_data_init();
  initPRController(&Volt_Rctrl);
	reset_value();
	llc.vloop_kp_init = 100;//800
	llc.vloop_ki_init = 5;//80
	llc.shareloop_kp_init = 100.0f;
	llc.shareloop_ki_init = 50.0f;
//	llc.vloop.kp = llc.vloop_kp_init;//12000 ;
//	llc.vloop.ki = llc.vloop_ki_init;//1000 ;
	llc.last_error_abs = 12.0f;
	llc.addr_set_flag = 0;
	llc.R_Out_ratio = 0;
	llc.R_Out_ratio_max = 5;
	llc.start_contr = 1;
	llc.first_load = true;
	llc.ibus_ref = IOUT_TARGET_CURRENT;
  __LL_ADC_REG_Conv_Start(ADC0);
  __LL_ADC_REG_Conv_Start(ADC1);
	__LL_ADC_REG_Conv_Start(ADC2);
	LL_CMP_Start(OVP_CMP);

  LL_TMR_Start_IT(TMR7);
  hrpwm_start_count();
  LL_TMR_Start_IT(TMR8);
	LL_TMR_Start_IT(TMR9);
	LL_TMR_Start_IT(TMR3);
		
}


