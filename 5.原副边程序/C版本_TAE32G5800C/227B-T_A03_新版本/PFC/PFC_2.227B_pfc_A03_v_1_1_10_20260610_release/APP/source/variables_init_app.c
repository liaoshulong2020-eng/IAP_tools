#include "main.h"




RAMCODE
void Reset_value(void)
{
		hrpwm_outdis_app();	
		pfc.close_pwm_flag |=(1<<0);

    pfc.vbus_target = VOUT_VOLTAGE ;
		pfc.state = State_Idel;


	
//    pfc.iloop.kp = PFC_I_KP;
//    pfc.iloop.ki = PFC_I_KI;
//    pfc.iloop.ki = 200;//200;
//    pfc.iloop.kd = 5000;//500;
//    pfc.iloop.kda = 10;//100;
//    pfc.vbus_ref = 0;

//    pfc.vloop.outputmax = PI_OUTPUT_LOW_LIMIT;
//    pfc.vloop.outputmin = PI_OUTPUT_LOW_LIMIT;
//    pfc.vloop.p_part    = 0;
//    pfc.vloop.i_part    = 0;
//    pfc.vloop.output = 0 ;

//    pfc.iloop.p_part = 0;
//    pfc.iloop.i_part = 0;
//    pfc.iloop.d_part = 0;
//    pfc.iloop.output = 0;
//    pfc.duty = PFC_MIN_DUTY;
//    hrpwm_updata_app();

//    pfc.fault_cnt = 0;
    pfc.vbus_rel = 0;					//µ±Ç°Öµ¹é0
    pfc.under_input_flag = 0;
    pfc.over_input_flag  = 0;
    pfc.V_over_output_flag = 0;
    pfc.I_over_output_flag = 0;
    pfc.start_cnt_flg = 0;
		pfc.check_current_is_ok = 0;
    pfc.state = State_Idel;
//    pfc.interrupt_state = I_STATE_1;
//    fre_cnt = 0;
//    irq_cnt = 0;
    start_cnt = 0;
//    pfc.iloop.error_n = 0;
//    pfc.iloop.error_n_1 = 0;
//    pfc.ibus_target_offset = 380;//0x3bb;
//    pfc.vbus_target_offset = 0;
//    pfc.dcdc_on_cnt = 0;
//    pfc.iloop.error_n_2 = 0;
//    pfc.ramp_cnt = 0;
//    LL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);

//		pfc.pre_finish_flg = 0;
//		pfc.pre_driver_cmpb = 100;

}

