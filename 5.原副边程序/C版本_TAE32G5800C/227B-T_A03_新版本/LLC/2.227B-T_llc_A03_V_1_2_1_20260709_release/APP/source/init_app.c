#include "main.h"
#include "init_app.h"

float limit_float(float value, float min, float max)
{
  if(value < min)
  {
    value = min;
  }
  else if(value > max)
  {
    value = max;
  }

  return value;
}

RAMCODE
void user_data_vout_protect_update(void)
{
  float vout_cmd;

  vout_cmd = user_data.coef_target;

  vout_cmd = limit_float(vout_cmd, 22.0f, 30.0f);

  user_data.coef_target = vout_cmd;
  user_data.vbus_target = vout_cmd;

  if(vout_cmd > VOUT_UP_VOLTAGE)
  {
    // 上调区：28V * 57A = 1596W
    user_data.over_current_point =
        (VOUT_VOLTAGE * IOUT_OCP_CURRENT) / vout_cmd;

    user_data.ibus_rec_target =
        user_data.over_current_point -
        (IOUT_OCP_CURRENT - IOUT_REC_TARGET_CURRENT);

    user_data.short_current_point =
        user_data.over_current_point +
        (IOUT_SHORT_CURRENT - IOUT_OCP_CURRENT);

    user_data.ibus_target =
        user_data.over_current_point -
        (IOUT_OCP_CURRENT - IOUT_TARGET_CURRENT);
  }
  else if(vout_cmd < VOUT_DOWM_VOLTAGE)
  {
    // 下调区：700W / 电压
    user_data.over_current_point =
        DOWM_OCP_WATT / vout_cmd;

    user_data.ibus_rec_target =
        IOUT_REC_TARGET_CURRENT;

    user_data.short_current_point =
        IOUT_SHORT_CURRENT;

    user_data.ibus_target =
        IOUT_TARGET_CURRENT;
  }
  else
  {
    // 正常区
    user_data.over_current_point =
        IOUT_OCP_CURRENT;

    user_data.ibus_rec_target =
        IOUT_REC_TARGET_CURRENT;

    user_data.short_current_point =
        IOUT_SHORT_CURRENT;

    user_data.ibus_target =
        IOUT_TARGET_CURRENT;
  }

  user_data.ibus_ref =
      user_data.ibus_target;

  user_data.under_voltage_point =
      VOUT_UNDER_VOLTAGE + user_data.vout_trim_delta;

  user_data.under_voltage_point =
      limit_float(user_data.under_voltage_point, 18.0f, 24.0f);
}

void flash_data_init(void)
{
  user_data.under_voltage_point      = VOUT_UNDER_VOLTAGE;
  user_data.under_voltage_rec_point  = 0;

  user_data.over_voltage_point       = VOUT_OVER_VOLTAGE;
  user_data.over_voltage_rec_point   = 0;

  user_data.over_temp_point          = TEMP_MAX_VALUE;
  user_data.over_temp_rec_point      = TEMP_REC_VALUE;
  user_data.temp_recover_mode        = 1;

  user_data.vbus_target              = VOUT_VOLTAGE;
  user_data.coef_target              = VOUT_VOLTAGE;
  user_data.vout_trim_delta          = 0;
  user_data.vout_can_delta           = 0;

  user_data.shareloop_kp             = 120.0f;
  user_data.shareloop_ki             = 10.0f;
  user_data.shareloop_out_max_min    = SHARE_OUT_MAX;

  user_data_vout_protect_update();
}

void load_data_app(void)
{
  load_data_from_flash(&user_data);

  user_data_vout_protect_update();

  llc.protection_point.under_voltage_point      = user_data.under_voltage_point;
  llc.protection_point.under_voltage_rec_point  = user_data.under_voltage_rec_point;

  llc.protection_point.over_voltage_point       = user_data.over_voltage_point;
  llc.protection_point.over_voltage_rec_point   = user_data.over_voltage_rec_point;

  llc.protection_point.over_current_point       = user_data.over_current_point;
  llc.protection_point.over_current_rec_point   = user_data.ibus_rec_target;
  llc.protection_point.short_current_point      = user_data.short_current_point;

  llc.protection_point.over_temp_point          = user_data.over_temp_point;
  llc.protection_point.over_temp_rec_point      = user_data.over_temp_rec_point;

  llc.temp_recover_mode                         = user_data.temp_recover_mode;

  llc.coef_target                               = user_data.coef_target;
  llc.vbus_target                               = user_data.coef_target;
  llc.vout_trim_delta                           = user_data.vout_trim_delta;
  llc.can_com_voltag_delta                      = user_data.vout_can_delta;

  llc.ibus_target                               = user_data.ibus_target;
  llc.ibus_rec_target                           = user_data.ibus_rec_target;
  llc.ibus_ref                                  = user_data.ibus_ref;

  llc.shareloop_kp_init                         = user_data.shareloop_kp;
  llc.shareloop_ki_init                         = user_data.shareloop_ki;

  llc.shareloop.out_max                         = user_data.shareloop_out_max_min;
  llc.shareloop.out_min                         = -(user_data.shareloop_out_max_min);

  llc.protec_set_flag                           = 1;
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
	 
	DEM_CR |= (uint32_t)DEM_CR_TRCENA; //ê1?ü?ú×ù??
	DWT_CYCCNT = (uint32_t)0u; //?? CYCCNT ??êy?? 0
	DWT_CR |= (uint32_t)DWT_CR_CYCCNTENA; //ê1?ü CYCCNT ??
  adc_init_app();
  hrpwm_init_app();
	gpio_init_config();
	dac_init_app();
	cmp_init_app();
//		iir_init_app();
//		cordic_init_app();
  tmr_init_app();
	can_init_app();	
	can_send_data_init();
	
	loadshare_pwm_init();
#if(!UART_FUNC)
  User_VOFA_Init();
#else
  user_uart_init(UART0);
#endif

  initPRController(&Volt_Rctrl);
	reset_value();
	llc.vloop_kp_init = 200;
	llc.vloop_ki_init = 50;
	llc.iloop_kp_init = 300.0f;;
	llc.iloop_ki_init = 10.0f;;

	llc.shareloop_out_max_init = 4.0f;

//	llc.vloop.kp = llc.vloop_kp_init;//12000 ;
//	llc.vloop.ki = llc.vloop_ki_init;//1000 ;
	llc.last_error_abs = 12.0f;
	llc.addr_set_flag = 0;
	llc.R_Out_ratio = 0;
	llc.R_Out_ratio_max = 0;
	llc.start_contr = 1;
	llc.ibus_ref = IOUT_TARGET_CURRENT;
	llc.para_scale_factor_is_ok = 0;
	

	
  __LL_ADC_REG_Conv_Start(ADC0);
  __LL_ADC_REG_Conv_Start(ADC1);
	__LL_ADC_REG_Conv_Start(ADC2);
	LL_CMP_Start(CMP1);

  LL_TMR_Start_IT(TMR7);
  hrpwm_start_count();
  LL_TMR_Start_IT(TMR8);
	LL_TMR_Start_IT(TMR9);
	LL_TMR_Start_IT(TMR3);
		
}


