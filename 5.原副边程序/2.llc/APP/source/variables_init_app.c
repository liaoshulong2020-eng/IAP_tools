#include "main.h"

void reset_value(void);



RAMCODE
void reset_value(void)
{

	hrpwm_outdis_app();


	llc.state = state_idel;
	
	llc.output_mode = voltage_mode;	//电压模式
//	llc.vbus_target = VOUT_VOLTAGE;
	llc.target_test = llc.vbus_target;
	llc.state_on_cnt = 0;	//稳压时间控制
	llc.vbus_ref = 0;
	
	
	llc.iloop.kp = ILOOP_KP;
	llc.iloop.ki = ILOOP_KI;
	
	llc.sr_pwm = 0;
	
	llc.idel_delay_cnt = 0;
	llc.idel_delay_ok = 0;
	

//	llc.start_contr = 1;
	
	if(!llc.fault_state.bit.over_temp)
	{
		llc.fault_state.bit.over_temp = 0;
	}
	llc.temp_recover_mode = 1;	// 过温恢复
	llc.PR_output_cnt = 0;
	llc.fault_state.bit.under_voltage = 0;
	
//	llc.protection_point.under_voltage_point = VOUT_UNDER_VOLTAGE;
//	llc.protection_point.over_temp_point = TEMP_MAX_VALUE;
//	llc.protection_point.over_temp_rec_point = TEMP_REC_VALUE;
		llc.loop_out_test = 0 ;
	disable_current_sharing();
	ShareCurrDutySet();
//	ShareCurrDutySet_Max();
}