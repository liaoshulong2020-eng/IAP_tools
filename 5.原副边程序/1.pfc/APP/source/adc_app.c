#include "main.h"


void adc_init_app(void)
{
  ADC_InitTypeDef       user_adc_init;
  ADC_REG_ComCfgTypeDef user_adc_reg_com_cfg;
  ADC_REG_ChCfgTypeDef  user_adc_reg_ch_cfg;
  ADC_DMA_CfgTypeDef    user_adc_dma_cfg;
  ADC_Cal_CfgTypeDef    user_adc_cal_cfg;
  HRPWM_Comm_ADCTrigCfgTypeDef user_hrpwm_comm_adctrig_cfg;

  memset((void*)&user_adc_init,        0x00, sizeof(user_adc_init));
  memset((void*)&user_adc_reg_com_cfg, 0x00, sizeof(user_adc_reg_com_cfg));
  memset((void*)&user_adc_reg_ch_cfg,  0x00, sizeof(user_adc_reg_ch_cfg));
  memset((void*)&user_adc_dma_cfg,     0x00, sizeof(user_adc_dma_cfg));
  memset((void*)&user_adc_cal_cfg,     0x00, sizeof(user_adc_cal_cfg));
  //User ADC Init
  user_adc_init.overrun_mode   = ADC_OVERRUN_DATA_OVERWRITTEN;
  user_adc_init.dual_mode      = ADC_DUAL_MODE_INDEPEND;
  //user_adc_init.dual_phase_dly = 0;
  //Oversample Config
  user_adc_init.over_samp_cfg.trig_en   = false;
  user_adc_init.over_samp_cfg.ratio     = ADC_OVER_SAMP_RATIO_2;
  user_adc_init.over_samp_cfg.shift     = ADC_OVER_SAMP_SHIFT_RIGHT_1;
  user_adc_init.over_samp_cfg.norm_mode = ADC_NORM_OVER_SAMP_CONTINUE;
  user_adc_init.over_samp_cfg.reg_en    = true;
  user_adc_init.over_samp_cfg.inj_en    = false;
  LL_ADC_Init(ADC0, &user_adc_init);
  //User ADC Regular Common Config
  user_adc_reg_com_cfg.dis_cont_en = false;
  user_adc_reg_com_cfg.conv_mode   = ADC_REG_CONV_SINGLE;
  user_adc_reg_com_cfg.seq_len     = ADC_REG_SEQ_LEN_4;
  user_adc_reg_com_cfg.trig_pol    = ADC_SEQ_TRIG_POL_HW_RISING;
  user_adc_reg_com_cfg.trig_evt    = ADC_SEQ_TRIG_EVT_HRPWM_ADCTRG0;
  LL_ADC_REG_ComCfg(ADC0, &user_adc_reg_com_cfg);

	/*******************************  ACL  ADC0_CH2  PA1  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_1;
  user_adc_reg_ch_cfg.ch          = ACL_SAMPLE_ADC0_CH2;
  user_adc_reg_ch_cfg.done_int_en = false;
  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
  LL_ADC_REG_ChCfg(ADC0, &user_adc_reg_ch_cfg);
  //User ADC DMA Config
  user_adc_dma_cfg.ch          = ACL_SAMPLE_ADC0_CH2;
  user_adc_dma_cfg.circ_en     = true;
  user_adc_dma_cfg.fix_addr_en = false;
  user_adc_dma_cfg.half_int_en = true;
  user_adc_dma_cfg.cplt_int_en = true;
  user_adc_dma_cfg.addr        = (uint32_t)&acl_samp[0];
  user_adc_dma_cfg.len         = sizeof(acl_samp);
  LL_ADC_DMA_Cfg(ADC0, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = ACL_SAMPLE_ADC0_CH2;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_0;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = -90;
  user_adc_cal_cfg.gain     = (uint16_t)(8192*1.029);
  LL_ADC_Cal_Cfg(ADC0, &user_adc_cal_cfg);
  /*******************************  ACL  ADC0_CH2  PA1  *******************************/

  /*******************************  ACN  ADC0_CH3  PA2  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_2;
  user_adc_reg_ch_cfg.ch          = ACN_SAMPLE_ADC0_CH3;
  user_adc_reg_ch_cfg.done_int_en = false;
  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
  LL_ADC_REG_ChCfg(ADC0, &user_adc_reg_ch_cfg);
  //User ADC DMA Config
  user_adc_dma_cfg.ch          = ACN_SAMPLE_ADC0_CH3;
  user_adc_dma_cfg.circ_en     = true;
  user_adc_dma_cfg.fix_addr_en = false;
  user_adc_dma_cfg.half_int_en = true;
  user_adc_dma_cfg.cplt_int_en = true;
  user_adc_dma_cfg.addr        = (uint32_t)&acn_samp;
  user_adc_dma_cfg.len         = sizeof(acn_samp);
  LL_ADC_DMA_Cfg(ADC0, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = ACN_SAMPLE_ADC0_CH3;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_1;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = -118;
  user_adc_cal_cfg.gain     = (uint16_t)(8192*1.039);
  LL_ADC_Cal_Cfg(ADC0, &user_adc_cal_cfg);
  /*******************************  ACN  ADC0_CH3  PA2  *******************************/

  /*******************************  I_SURGE  ADC0_CH4  PA3  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_3;
  user_adc_reg_ch_cfg.ch          = I_SURGE_SAMPLE_ADC0_CH4;
  user_adc_reg_ch_cfg.done_int_en = false;
  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
  LL_ADC_REG_ChCfg(ADC0, &user_adc_reg_ch_cfg);
  //User ADC DMA Config
  user_adc_dma_cfg.ch          = I_SURGE_SAMPLE_ADC0_CH4;
  user_adc_dma_cfg.circ_en     = true;
  user_adc_dma_cfg.fix_addr_en = false;
  user_adc_dma_cfg.half_int_en = true;
  user_adc_dma_cfg.cplt_int_en = true;
  user_adc_dma_cfg.addr        = (uint32_t)&i_surge_samp;
  user_adc_dma_cfg.len         = sizeof(i_surge_samp);
  LL_ADC_DMA_Cfg(ADC0, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = I_SURGE_SAMPLE_ADC0_CH4;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_2;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = 0;
  user_adc_cal_cfg.gain     = 0x2000;
  LL_ADC_Cal_Cfg(ADC0, &user_adc_cal_cfg);
  /*******************************  I_SURGE  ADC0_CH4  PA3  *******************************/

  /*******************************  I_Cr  ADC0_CH14  PB11  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_4;
  user_adc_reg_ch_cfg.ch          = I_Cr_SAMPLE_ADC0_CH14;
  user_adc_reg_ch_cfg.done_int_en = false;
  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
  LL_ADC_REG_ChCfg(ADC0, &user_adc_reg_ch_cfg);
  //User ADC DMA Config
  user_adc_dma_cfg.ch          = I_Cr_SAMPLE_ADC0_CH14;
  user_adc_dma_cfg.circ_en     = true;
  user_adc_dma_cfg.fix_addr_en = false;
  user_adc_dma_cfg.half_int_en = true;
  user_adc_dma_cfg.cplt_int_en = true;
  user_adc_dma_cfg.addr        = (uint32_t)&i_surge_samp;
  user_adc_dma_cfg.len         = sizeof(i_surge_samp);
  LL_ADC_DMA_Cfg(ADC0, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = I_Cr_SAMPLE_ADC0_CH14;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_3;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = 0;
  user_adc_cal_cfg.gain     = 0x2000;
  LL_ADC_Cal_Cfg(ADC0, &user_adc_cal_cfg);
  /*******************************  I_Cr  ADC0_CH14  PB11  *******************************/

  //User ADC Init
  user_adc_init.overrun_mode   = ADC_OVERRUN_DATA_OVERWRITTEN;
  user_adc_init.dual_mode      = ADC_DUAL_MODE_INDEPEND;
  //user_adc_init.dual_phase_dly = 0;
  //Oversample Config
  user_adc_init.over_samp_cfg.trig_en   = false;
  user_adc_init.over_samp_cfg.ratio     = ADC_OVER_SAMP_RATIO_2;
  user_adc_init.over_samp_cfg.shift     = ADC_OVER_SAMP_SHIFT_RIGHT_1;
  user_adc_init.over_samp_cfg.norm_mode = ADC_NORM_OVER_SAMP_CONTINUE;
  user_adc_init.over_samp_cfg.reg_en    = true;
  user_adc_init.over_samp_cfg.inj_en    = false;
  LL_ADC_Init(ADC1, &user_adc_init);
  //User ADC Regular Common Config
  user_adc_reg_com_cfg.dis_cont_en = false;
  user_adc_reg_com_cfg.conv_mode   = ADC_REG_CONV_SINGLE;
  user_adc_reg_com_cfg.seq_len     = ADC_REG_SEQ_LEN_4;
  user_adc_reg_com_cfg.trig_pol    = ADC_SEQ_TRIG_POL_HW_RISING;
  user_adc_reg_com_cfg.trig_evt    = ADC_SEQ_TRIG_EVT_HRPWM_ADCTRG0;
  LL_ADC_REG_ComCfg(ADC1, &user_adc_reg_com_cfg);

  /*******************************  VBUS  ADC1_IN6  PA6  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_1;
  user_adc_reg_ch_cfg.ch          = PFC_VBUS_SAMPLE_ADC1_CH3;
  user_adc_reg_ch_cfg.done_int_en = false;
  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
  LL_ADC_REG_ChCfg(ADC1, &user_adc_reg_ch_cfg);
  //User ADC DMA Config
  user_adc_dma_cfg.ch          = PFC_VBUS_SAMPLE_ADC1_CH3;
  user_adc_dma_cfg.circ_en     = true;
  user_adc_dma_cfg.fix_addr_en = false;
  user_adc_dma_cfg.half_int_en = true;
  user_adc_dma_cfg.cplt_int_en = true;
  user_adc_dma_cfg.addr        = (uint32_t)&vbus_samp;
  user_adc_dma_cfg.len         = sizeof(vbus_samp);
  LL_ADC_DMA_Cfg(ADC1, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = PFC_VBUS_SAMPLE_ADC1_CH3;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_0;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = (int16_t)33;
  user_adc_cal_cfg.gain     = (uint16_t)(8192);
  LL_ADC_Cal_Cfg(ADC1, &user_adc_cal_cfg);
  /*******************************  VBUS  ADC1_IN6  PA6  *******************************/

  /*******************************  OCP  ADC1_IN13  PA5  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_2;
  user_adc_reg_ch_cfg.ch          = PFC_OCP_SAMPLE_ADC1_CH13;
  user_adc_reg_ch_cfg.done_int_en = false;
  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
  LL_ADC_REG_ChCfg(ADC1, &user_adc_reg_ch_cfg);
  //User ADC DMA Config
  user_adc_dma_cfg.ch          = PFC_OCP_SAMPLE_ADC1_CH13;
  user_adc_dma_cfg.circ_en     = true;
  user_adc_dma_cfg.fix_addr_en = false;
  user_adc_dma_cfg.half_int_en = true;
  user_adc_dma_cfg.cplt_int_en = true;
  user_adc_dma_cfg.addr        = (uint32_t)&i_ocp_samp;
  user_adc_dma_cfg.len         = sizeof(i_ocp_samp);
  LL_ADC_DMA_Cfg(ADC1, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = PFC_OCP_SAMPLE_ADC1_CH13;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_1;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = 0;
  user_adc_cal_cfg.gain     = 0x2000;
  LL_ADC_Cal_Cfg(ADC1, &user_adc_cal_cfg);
  /*******************************  OCP  ADC1_IN13  PA5  *******************************/

  /*******************************  R_NTC  ADC1_IN12  PB2  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_3;
  user_adc_reg_ch_cfg.ch          = R_NTC_SAMPLE_ADC1_CH12;
  user_adc_reg_ch_cfg.done_int_en = false;
  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
  LL_ADC_REG_ChCfg(ADC1, &user_adc_reg_ch_cfg);
  //User ADC DMA Config
  user_adc_dma_cfg.ch          = R_NTC_SAMPLE_ADC1_CH12;
  user_adc_dma_cfg.circ_en     = true;
  user_adc_dma_cfg.fix_addr_en = false;
  user_adc_dma_cfg.half_int_en = true;
  user_adc_dma_cfg.cplt_int_en = true;
  user_adc_dma_cfg.addr        = (uint32_t)&r_ntc_samp;
  user_adc_dma_cfg.len         = sizeof(r_ntc_samp);
  LL_ADC_DMA_Cfg(ADC1, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = R_NTC_SAMPLE_ADC1_CH12;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_2;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = 0;
  user_adc_cal_cfg.gain     = 0x2000;
  LL_ADC_Cal_Cfg(ADC1, &user_adc_cal_cfg);
  /*******************************  R_NTC  ADC1_IN12  PB2  *******************************/

  /*******************************  I_SAMP  ADC1_IN16  PA4  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_4;
  user_adc_reg_ch_cfg.ch          = PFC_I_SAMPLE_ADC1_CH16;
  user_adc_reg_ch_cfg.done_int_en = true;
  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
  LL_ADC_REG_ChCfg(ADC1, &user_adc_reg_ch_cfg);
  //User ADC DMA Config
  user_adc_dma_cfg.ch          = PFC_I_SAMPLE_ADC1_CH16;
  user_adc_dma_cfg.circ_en     = true;
  user_adc_dma_cfg.fix_addr_en = false;
  user_adc_dma_cfg.half_int_en = true;
  user_adc_dma_cfg.cplt_int_en = true;
  user_adc_dma_cfg.addr        = (uint32_t)&i_samp;
  user_adc_dma_cfg.len         = sizeof(i_samp);
  LL_ADC_DMA_Cfg(ADC1, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = PFC_I_SAMPLE_ADC1_CH16;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_3;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = 0;
  user_adc_cal_cfg.gain     = 8192;
  LL_ADC_Cal_Cfg(ADC1, &user_adc_cal_cfg);
  /*******************************  I_SAMP  ADC1_IN16  PA4  *******************************/


//  //User HRPWM Common ADC Trigger Config
//  user_hrpwm_comm_adctrig_cfg.trig_evt    = HRPWM_COMM_ADC02_TRIG_EVT_MST_PWM_CMPB;
//  user_hrpwm_comm_adctrig_cfg.trig_len    = HRPWM_COMM_ADC_TRIG_EVT_LEN_15CLK;
//  user_hrpwm_comm_adctrig_cfg.upd_src     = HRPWM_COMM_ADC_TRIG_UPD_SRC_MST_PWM;
//  user_hrpwm_comm_adctrig_cfg.post_scaler = 0;
//  LL_HRPWM_Comm_ADCTrigCfg(HRPWM, HRPWM_ADC_TRIG_NUM_0, &user_hrpwm_comm_adctrig_cfg);


//    //Preload Enable, software need to generate a update event to update the shadow register to the working register
//    //If Preload Disable, don't need this action.
//    __LL_HRPWM_Comm_MstPWMSwUpdReg_Set(HRPWM);



}

float alpha = 0.8;

RAMCODE
void get_adc_value(void)
{
//  pfc.acl_value			=	 acl_samp [0]		 	;
//  pfc.acn_value			=	 acn_samp [0] 		;
  pfc.i_surge_value	=	 i_surge_samp [0] ;
//  pfc.vbus_value		=	 vbus_samp [0] 		;
  pfc.i_ocp_value		=	 i_ocp_samp [0]		;
	if(r_ntc_samp [0] < 0)
	{
		pfc.r_ntc_value		=	 0 	;
	}
	else
	{
  pfc.r_ntc_value		=	 r_ntc_samp [0] 	;
	}
//  pfc.i_value				=	 i_samp [0] 			;
	pfc.vbus_samp = vbus_samp [0];
		if(i_samp [0] > CURRENT_OFFSET_AD_VALUE)
	{
		pfc.i_samp_check = i_samp [0] - pfc.check_current_data;
	}
	else
	{
		pfc.i_samp_check = CURRENT_OFFSET_AD_VALUE;
	}
	if(pfc.i_samp_check < 0)
	{
		 pfc.i_samp_check = 0;
	}

//  pfc.acl_value = alpha * acl_samp [0] + (1 - alpha) * pfc.acl_value;
//  pfc.acn_value = alpha * acn_samp [0] + (1 - alpha) * pfc.acn_value;

	pfc.acl_value =  acl_samp [0]*VAC_SAMP_RATIO;
  pfc.acn_value =  acn_samp [0]*VAC_SAMP_RATIO ;

		
		pfc.vbus_value_store_sum -= pfc.vbus_value_store[pfc.vbus_vol_cnt];
    pfc.vbus_value_store[pfc.vbus_vol_cnt] = ((float)pfc.vbus_samp * PFC_BUS_SAMP_RATIO) + pfc.vbus_value_store[pfc.vbus_vol_cnt] - (pfc.vbus_value_store[pfc.vbus_vol_cnt] / VBUS_FILTER_ORDER);
		pfc.vbus_value_store_sum += pfc.vbus_value_store[pfc.vbus_vol_cnt];


//    pfc.vbus_vol_cnt = (pfc.vbus_vol_cnt + 1) % 4;
    pfc.vbus_value = pfc.vbus_value_store_sum / 32; // 输出电压采样平均值
	
		pfc.vbus_value_old = pfc.vbus_move_filter_store[pfc.vbus_move_filter_cnt];
		pfc.vbus_move_filter_sum -= pfc.vbus_value_old;
		pfc.vbus_value_new = pfc.vbus_value;
		pfc.vbus_move_filter_store[pfc.vbus_move_filter_cnt] = pfc.vbus_value_new;
		pfc.vbus_move_filter_sum += pfc.vbus_move_filter_store[pfc.vbus_move_filter_cnt];
		pfc.vbus_value_error = pfc.vbus_value_new - pfc.vbus_value_old;
		pfc.vbus_move_filter_cnt = (pfc.vbus_move_filter_cnt+1)%500;
		pfc.vbus_move_filter_value = pfc.vbus_move_filter_sum / 500;

		
//		pfc.vbus_filter_value_store[pfc.vbus_vol_cnt] =	(float)pfc.vbus_notch_filer_data*PFC_BUS_SAMP_RATIO + pfc.vbus_filter_value_store[pfc.vbus_vol_cnt] - (pfc.vbus_filter_value_store[pfc.vbus_vol_cnt] / VBUS_FILTER_ORDER );
//		pfc.vbus_filter_value_store_sum = pfc.vbus_filter_value_store[0] + pfc.vbus_filter_value_store[1] + pfc.vbus_filter_value_store[2] + pfc.vbus_filter_value_store[3];
//		pfc.vbus_filter_value = pfc.vbus_filter_value_store_sum / 64;
//		

		
		if(abs_f(pfc.vbus_value_error) < pfc.vbus_value_error_th)
		{
			if(pfc.switch_vbus_flag == 1)
			{
				pfc.switch_vbus_cnt++;
				pfc.vbus_rel = pfc.vbus_value;
			}
			if(pfc.switch_vbus_cnt > 200)
			{
				
				pfc.vbus_rel = pfc.vbus_move_filter_value;
				pfc.switch_vbus_flag = 0;
			}
		}
		else
		{
			pfc.vbus_rel = pfc.vbus_value;
			pfc.switch_vbus_flag = 1;
			pfc.switch_vbus_cnt = 0;
		}
		pfc.vbus_vol_cnt = (pfc.vbus_vol_cnt + 1) % (4);
	
		pfc.ibus_value_store[0] = pfc.i_samp_check  + pfc.ibus_value_store[0]  - (pfc.ibus_value_store[0]  / IIN_FILTER_ORDER);
//  pfc.ibus_curr_cnt = (pfc.ibus_curr_cnt + 1) % (4);
//  pfc.ibus_value_store_sum = pfc.ibus_value_store[0] +	pfc.ibus_value_store[1] + pfc.ibus_value_store[2] + pfc.ibus_value_store[3];
  pfc.i_value = pfc.ibus_value_store[0] / IIN_FILTER_ORDER;

  pfc.iloop.rel = ((float)((pfc.i_samp_check*SAMP_RATIO)-(float)IPFC_OFFSET_VAL)/IPFC_SCALE_VAL);

if(pfc.iloop.rel < 0)
{
pfc.iloop.rel = 0;
}
//		pfc.temp_filtered = pfc.temp_value + pfc.temp_filtered - (pfc.temp_filtered >> 3);
//		txdata_buf.vin_squares_average = pfc.vin_squares_average;
//		txdata_buf.Fault_Flag = pfc.Fault_Flag;
//		txdata_buf.temp_pri_ad = pfc.temp_filtered >> 3;
}

void check_current_zero_offset(void)
{
	if((pfc.pre_finish_flg)&&(!pfc.check_current_is_ok))
	{
		if(pfc.check_current_cnt > 1024)
		{
			pfc.check_current_is_ok = 1;
			pfc.check_current_data = pfc.check_current_data_sum / 1024;
		}
		else
		{
			pfc.check_current_cnt++;
			pfc.check_current_data_sum += (pfc.i_value - (CURRENT_OFFSET_AD_VALUE));
		}
	}
}



