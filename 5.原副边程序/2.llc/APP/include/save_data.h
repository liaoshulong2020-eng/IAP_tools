/*
 * save.h
 *
 *  Created on: 2024年8月6日
 *      Author: Liang Jinfeng
 */

#ifndef SAVE_H_
#define SAVE_H_

#include "main.h"
void user_flash_check();
//定义Flash存储开始地址：存放在124~127KB这块扇区，共4KB
#define SAVE_OFFSET		(124*1024) //必须为4KB的整数倍
#define SAVE_ADDR		(0x08000000+SAVE_OFFSET) //0x0801F000

/*
 * 定义数据存储类型
 */
typedef struct
{
	uint8_t save_flag[8];			//Flash存储标志字

//	uint32_t vin_ac_uvp; 			//AC输入电压欠压保护点，单位：mV
//	uint32_t vin_ac_uv_rec; 	//AC输入电压欠压恢复点，单位：mV
//	uint32_t vin_ac_ovp; 			//AC输入电压过压保护点，单位：mV
//	uint32_t vin_ac_ov_rec; 	//AC输入电压过压恢复点，单位：mV

//	uint32_t vin_dc_uvp; 			//DC输入电压欠压保护点，单位：mV
//	uint32_t vin_dc_uv_rec; 	//DC输入电压欠压恢复点，单位：mV
//	uint32_t vin_dc_ovp; 			//DC输入电压过压保护点，单位：mV
//	uint32_t vin_dc_ov_rec; 	//DC输入电压过压恢复点，单位：mV

//	uint32_t vout_target; 		//目标输出电压，单位：mV

//	uint32_t vout_uvp; 				//输出电压欠压保护点，单位：mV
//	uint32_t vout_uv_rec; 		//输出电压欠压恢复点，单位：mV
//	uint32_t vout_ovp; 				//输出电压过压保护点，单位：mV
//	uint32_t vout_ov_rec; 		//输出电压过压恢复点，单位：mV

//	uint32_t iout_max;				//输出电流额定最大值，单位：mA
//	uint32_t iout_ocp; 				//输出电流过流保护点，单位：mA
//	uint32_t iout_oc_rec; 		//输出电流过流恢复点，单位：mA

//	int16_t tem_otp;					//过温保护点，单位：摄氏度
//	int16_t tem_ot_rec;				//过温恢复点，单位：摄氏度
//	uint8_t temp_recover_mode;

    float over_voltage_point  		;		//过压保护点
		float over_voltage_rec_point	;		//过压恢复点
    float under_voltage_point			;		//欠压保护点
    float under_voltage_rec_point	;		//欠压恢复点	
    float over_current_point			;		//过流保护点
    float over_current_rec_point	;		//过流恢复点	
		float short_current_point			;		//短路保护点
    float over_temp_point					;		//过温保护点
    float over_temp_rec_point			;		//过温恢复点	
		bool  temp_recover_mode				;		//过温恢复模式		
		
		float vbus_target; 								//目标输出电压
		float coef_target;								//校准电压
		
		float ibus_target							; 	//目标输出电流（恒流点）
		float ibus_rec_target					;		//过流恢复电流
		float ibus_ref								;		//电流环目标值
		
		float iout_gain_1;
		float iout_offset_1;
		
		float iout_gain_2;
		float iout_offset_2;		
		
		float vout_trim_delta;				;		//外部调压偏移量
		float	vout_can_delta					;		//CAN通信上报偏移量
		float iout_can_gain						;		//CAN通信上报电流增益
		float iout_can_offset					;		//CAN通信上报电流偏置
		


}save_data_t;

extern save_data_t flash_data,user_data;
/*
 * 保存数据
 */
bool save_data_to_flash(save_data_t *data);

/*
 * 加载数据
 */
bool load_data_from_flash(save_data_t *data);

#endif /* SAVE_H_ */
