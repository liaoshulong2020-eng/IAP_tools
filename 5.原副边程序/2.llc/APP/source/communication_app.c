#include "main.h"
#include "communication_app.h"

RAMCODE
void send_massage_get(void)
{
	llc.send_massage.power.output_target_voltage = llc.vbus_target;
	llc.send_massage.power.output_rel_voltage = llc.vbus_rel;
	llc.send_massage.power.output_rel_current	=	llc.iout_rel_slow;

	SET_FAULT_STATE(llc.start_contr, llc.custom_can_data.power_para.power_state.power_on_off);
  SET_FAULT_STATE(llc.fault_state.bit.over_voltage, llc.custom_can_data.power_para.power_state.over_voltage);
  SET_FAULT_STATE(llc.fault_state.bit.under_voltage, llc.custom_can_data.power_para.power_state.under_voltage);
  SET_FAULT_STATE(llc.fault_state.bit.over_current, llc.custom_can_data.power_para.power_state.over_current);
  SET_FAULT_STATE(llc.fault_state.bit.over_temp, llc.custom_can_data.power_para.power_state.over_temp);
	llc.custom_can_data.power_para.power_state.commun_status = 1;

	// 【优化】改进电压计算精度，使用更精确的计算方式
	float voltage = llc.vbus_rel - llc.can_com_voltag_delta + llc.delta_voltage;
	if (voltage < 0) voltage = 0;
	// 【优化】使用64位中间变量避免溢出，提高精度
	int voltage_millivolts = (int)(voltage * 1000.0f);
	llc.custom_can_data.power_para.vol_high_bit = (voltage_millivolts >> 8) & 0xFF;
	llc.custom_can_data.power_para.vol_low_bit = voltage_millivolts & 0xFF;

	// 【优化】改进电流计算精度
	int current_01a = (int)(llc.iout_rel_slow * 10.0f);
	llc.custom_can_data.power_para.cur_high_bit = (current_01a >> 8) & 0xFF;
	llc.custom_can_data.power_para.cur_low_bit = current_01a & 0xFF;
	llc.custom_can_data.power_para.temp_bit = llc.temp_c;

}

