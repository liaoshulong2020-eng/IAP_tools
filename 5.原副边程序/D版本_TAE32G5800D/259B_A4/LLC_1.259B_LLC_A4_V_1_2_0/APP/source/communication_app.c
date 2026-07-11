#include "main.h"
#include "communication_app.h"

RAMCODE
void send_massage_get(void)
{
	llc.send_massage.state.loop_mode 			= llc.output_mode;
	llc.send_massage.state.llc_state			= llc.state;
	llc.send_massage.state.switch_mode 		=	llc.switch_mode;
	llc.send_massage.state.llc_drv_state 	= llc.llc_drv_state;
	llc.send_massage.state.llc_sr_state 	= llc.llc_sr_state;
	
	llc.send_massage.state.over_voltage 	= llc.fault_state.bit.over_voltage;
	llc.send_massage.state.under_voltage 	= llc.fault_state.bit.under_voltage;
	llc.send_massage.state.over_current 	= llc.fault_state.bit.over_current;
	llc.send_massage.state.over_temp			= llc.fault_state.bit.over_temp;
	llc.send_massage.state.pfc_fault 			= llc.fault_state.bit.pfc_fault;
	
	
	llc.send_massage.power.output_target_voltage = llc.vbus_target;
	llc.send_massage.power.output_rel_voltage = llc.vbus_rel;
	llc.send_massage.power.output_rel_current	=	llc.iout_rel_slow;
	
	SET_FAULT_STATE(llc.start_contr, can_data.power_para.power_state.power_on_off);
  SET_FAULT_STATE(llc.fault_state.bit.over_voltage, can_data.power_para.power_state.over_voltage);
  SET_FAULT_STATE(llc.fault_state.bit.under_voltage, can_data.power_para.power_state.under_voltage);
  SET_FAULT_STATE(llc.fault_state.bit.over_current, can_data.power_para.power_state.over_current);
  SET_FAULT_STATE(llc.fault_state.bit.over_temp, can_data.power_para.power_state.over_temp);

  float voltage = llc.vbus_rel_slow  + llc.delta_voltage ;
    if (voltage < 0) voltage = 0;
    int voltage_millivolts = (int)(voltage * 1000);
    can_data.power_para.vol_high_bit = (voltage_millivolts >> 8) & 0xFF;
    can_data.power_para.vol_low_bit = voltage_millivolts & 0xFF;
				
	can_data.power_para.cur_high_bit =  (((int)(llc.iout_rel_slow*10) ) >> 8 ) & 0xFF;
	can_data.power_para.cur_low_bit = (((int)(llc.iout_rel_slow*10) )  ) & 0xFF;
	can_data.power_para.temp_bit = llc.temp_c;
	
}
RAMCODE
void send_massage_share_get(void) 
{
	can_share_power.share_power.cur_high_bit = (((int)(llc.loadshare_value*10) ) >> 8 ) & 0xFF;
	can_share_power.share_power.cur_low_bit  = (((int)(llc.loadshare_value*10) )  ) & 0xFF;

	can_share_power.share_power.load_current_high_bit = (((int)(llc.iout_rel_slow*10) ) >> 8 ) & 0xFF;
	can_share_power.share_power.load_current_low_bit  = (((int)(llc.iout_rel_slow*10) )  ) & 0xFF;
	
	can_share_power.share_power.cmd = 0x88;
}

