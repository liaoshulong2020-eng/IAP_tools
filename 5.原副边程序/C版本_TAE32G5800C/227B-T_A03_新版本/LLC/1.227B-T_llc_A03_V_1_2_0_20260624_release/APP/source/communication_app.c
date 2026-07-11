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
	can_data.power_para.power_state.comm_status = 1;

	
	llc.send_massage.power.output_target_voltage = llc.vbus_target;
	llc.send_massage.power.output_rel_voltage = llc.vbus_rel;
	llc.send_massage.power.output_rel_current	=	llc.iout_rel_slow;
	
  // ?¨¹D?¦Ì??¡ä?a1??¡é1y?1?¡é?¡¤?1?¡é1y¨¢¡Â?¡é1y???¡é¡Á¡ä¨¬?
	SET_FAULT_STATE(llc.start_contr, can_data.power_para.power_state.power_on_off);
  SET_FAULT_STATE(llc.fault_state.bit.over_voltage, can_data.power_para.power_state.over_voltage);
  SET_FAULT_STATE(llc.fault_state.bit.under_voltage, can_data.power_para.power_state.under_voltage);
  SET_FAULT_STATE(llc.fault_state.bit.over_current, can_data.power_para.power_state.over_current);
  SET_FAULT_STATE(llc.fault_state.bit.over_temp, can_data.power_para.power_state.over_temp);

  float voltage = llc.vbus_rel - llc.can_com_voltag_delta - llc.delta_voltage;
    
    // ¡¤??¡ì?T??
    if (voltage < 0) voltage = 0;
    
    int voltage_millivolts = (int)(voltage * 1000);
    
    can_data.power_para.vol_high_bit = (voltage_millivolts >> 8) & 0xFF;
    can_data.power_para.vol_low_bit = voltage_millivolts & 0xFF;
		

//	can_data.power_para.vol_high_bit =  (((int)((llc.vbus_rel-llc.can_com_voltag_delta)*1000) ) >> 8 ) & 0xFF;
//	can_data.power_para.vol_low_bit = (((int)((llc.vbus_rel-llc.can_com_voltag_delta)*1000) )  ) & 0xFF;
		float current1 = (llc.iout_rel_slow*1+0);
		llc.can_iout_disp = (current1);
	can_data.power_para.cur_high_bit =  (((int)(llc.can_iout_disp*10) ) >> 8 ) & 0xFF;
	can_data.power_para.cur_low_bit = (((int)(llc.can_iout_disp*10) )  ) & 0xFF;
	can_data.power_para.temp_bit = llc.temp_c;
}



