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

	/*******************************  I_SAMPLE  ADC0_CH1  PA0  CMP2_INP0	*******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_1;
  user_adc_reg_ch_cfg.ch          = PFC_I_SAMPLE_ADC0_CH1;
  user_adc_reg_ch_cfg.done_int_en = false;
  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
  LL_ADC_REG_ChCfg(ADC0, &user_adc_reg_ch_cfg);
  //User ADC DMA Config
  user_adc_dma_cfg.ch          = PFC_I_SAMPLE_ADC0_CH1;
  user_adc_dma_cfg.circ_en     = true;
  user_adc_dma_cfg.fix_addr_en = false;
  user_adc_dma_cfg.half_int_en = true;
  user_adc_dma_cfg.cplt_int_en = true;
  user_adc_dma_cfg.addr        = (uint32_t)&i_ocp_samp[0];
  user_adc_dma_cfg.len         = sizeof(i_ocp_samp);
  LL_ADC_DMA_Cfg(ADC0, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = PFC_I_SAMPLE_ADC0_CH1;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_0;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = 0;
  user_adc_cal_cfg.gain     = 8192;
  LL_ADC_Cal_Cfg(ADC0, &user_adc_cal_cfg);
	/*******************************  I_SAMPLE  ADC0_CH1  PA0  CMP2_INP0	*******************************/


	/*******************************  ACL  ADC0_CH2  PA1  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_2;
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
  user_adc_cal_cfg.offset   = -41;
  user_adc_cal_cfg.gain     = 8462;
  LL_ADC_Cal_Cfg(ADC0, &user_adc_cal_cfg);
  /*******************************  ACL  ADC0_CH2  PA1  *******************************/

  /*******************************  ACN  ADC0_CH3  PA2  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_3;
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
  user_adc_cal_cfg.offset   = -34;
  user_adc_cal_cfg.gain     = 8454;
  LL_ADC_Cal_Cfg(ADC0, &user_adc_cal_cfg);
  /*******************************  ACN  ADC0_CH3  PA2  *******************************/

  /*******************************  PFC_VBUS  ADC0_CH15  PB0 CMP3_INP0	*******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_4;
  user_adc_reg_ch_cfg.ch          = PFC_VBUS_SAMPLE_ADC0_CH15;
  user_adc_reg_ch_cfg.done_int_en = false;
  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
  LL_ADC_REG_ChCfg(ADC0, &user_adc_reg_ch_cfg);
  //User ADC DMA Config
  user_adc_dma_cfg.ch          = PFC_VBUS_SAMPLE_ADC0_CH15;
  user_adc_dma_cfg.circ_en     = true;
  user_adc_dma_cfg.fix_addr_en = false;
  user_adc_dma_cfg.half_int_en = true;
  user_adc_dma_cfg.cplt_int_en = true;
  user_adc_dma_cfg.addr        = (uint32_t)&vbus_samp;
  user_adc_dma_cfg.len         = sizeof(vbus_samp);
  LL_ADC_DMA_Cfg(ADC0, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = PFC_VBUS_SAMPLE_ADC0_CH15;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_2;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = 7;
  user_adc_cal_cfg.gain     = 8281;
  LL_ADC_Cal_Cfg(ADC0, &user_adc_cal_cfg);
  /*******************************  PFC_VBUS  ADC0_CH15  PB0 CMP3_INP0	*******************************/







  //User ADC Init
  user_adc_init.overrun_mode   = ADC_OVERRUN_DATA_OVERWRITTEN;
  user_adc_init.dual_mode      = ADC_DUAL_MODE_INDEPEND;
  //user_adc_init.dual_phase_dly = 0;
  //Oversample Config
  user_adc_init.over_samp_cfg.trig_en   = false;
  user_adc_init.over_samp_cfg.ratio     = ADC_OVER_SAMP_RATIO_4;
  user_adc_init.over_samp_cfg.shift     = ADC_OVER_SAMP_SHIFT_RIGHT_2;
  user_adc_init.over_samp_cfg.norm_mode = ADC_NORM_OVER_SAMP_CONTINUE;
  user_adc_init.over_samp_cfg.reg_en    = true;
  user_adc_init.over_samp_cfg.inj_en    = false;
  LL_ADC_Init(ADC1, &user_adc_init);
  //User ADC Regular Common Config
  user_adc_reg_com_cfg.dis_cont_en = false;
  user_adc_reg_com_cfg.conv_mode   = ADC_REG_CONV_SINGLE;
  user_adc_reg_com_cfg.seq_len     = ADC_REG_SEQ_LEN_2;
  user_adc_reg_com_cfg.trig_pol    = ADC_SEQ_TRIG_POL_HW_RISING;
  user_adc_reg_com_cfg.trig_evt    = ADC_SEQ_TRIG_EVT_HRPWM_ADCTRG0;
  LL_ADC_REG_ComCfg(ADC1, &user_adc_reg_com_cfg);

  /*******************************  TEMP  ADC1_IN13  PA5  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_1;
  user_adc_reg_ch_cfg.ch          = TEMP_SAMPLE_ADC1_CH13;
  user_adc_reg_ch_cfg.done_int_en = false;
  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
  LL_ADC_REG_ChCfg(ADC1, &user_adc_reg_ch_cfg);
  //User ADC DMA Config
  user_adc_dma_cfg.ch          = TEMP_SAMPLE_ADC1_CH13;
  user_adc_dma_cfg.circ_en     = true;
  user_adc_dma_cfg.fix_addr_en = false;
  user_adc_dma_cfg.half_int_en = true;
  user_adc_dma_cfg.cplt_int_en = true;
  user_adc_dma_cfg.addr        = (uint32_t)&temp_samp;
  user_adc_dma_cfg.len         = sizeof(temp_samp);
  LL_ADC_DMA_Cfg(ADC1, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = TEMP_SAMPLE_ADC1_CH13;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_0;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = 0;
  user_adc_cal_cfg.gain     = 8192;
  LL_ADC_Cal_Cfg(ADC1, &user_adc_cal_cfg);
  /*******************************  TEMP  ADC1_IN13  PA5  *******************************/

  /*******************************  R_NTC  ADC1_IN12  PB2  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_2;
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
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_1;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = 0;
  user_adc_cal_cfg.gain     = 0x2000;
  LL_ADC_Cal_Cfg(ADC1, &user_adc_cal_cfg);
  /*******************************  R_NTC  ADC1_IN12  PB2  *******************************/

}

float alpha = 0.8;

RAMCODE
void get_adc_value(void)
{
    // -----------------------------
    // 原始ADC采样
    // -----------------------------
    pfc.i_surge_value     = i_surge_samp[0];
    pfc.i_ocp_value       = i_ocp_samp[0];
    pfc.r_ntc_value       = r_ntc_samp[0];
    pfc.vbus_samp         = vbus_samp[0];

    // -----------------------------
    // 电流去零偏
    // -----------------------------
    {
        int32_t di = (int32_t)i_ocp_samp[0] - (int32_t)pfc.check_current_data;
        if (di < 0) di = 0;
        pfc.i_samp_check = (float)di;
    }

    // -----------------------------
    // 输入电压采样（缩放）
    // -----------------------------
    pfc.acl_value = acl_samp[0] * VAC_SAMP_RATIO;
    pfc.acn_value = acn_samp[0] * VAC_SAMP_RATIO;

    // -----------------------------
    // VBUS 一阶IIR滤波
    // -----------------------------
    pfc.vbus_value_store_sum -= pfc.vbus_value_store[0];

    pfc.vbus_value_store[0] =
        ((float)pfc.vbus_samp * PFC_BUS_SAMP_RATIO)
        + pfc.vbus_value_store[0]
        - (pfc.vbus_value_store[0] / VBUS_FILTER_ORDER);

    pfc.vbus_value_store_sum += pfc.vbus_value_store[0];

    pfc.vbus_value = pfc.vbus_value_store_sum / VBUS_FILTER_ORDER  - 3.0f;

    // -----------------------------
    // VBUS 移动平均滤波
    // -----------------------------
    pfc.vbus_value_old = pfc.vbus_move_filter_store[pfc.vbus_move_filter_cnt];
    pfc.vbus_move_filter_sum -= pfc.vbus_value_old;

    pfc.vbus_value_new = pfc.vbus_value;
    pfc.vbus_move_filter_store[pfc.vbus_move_filter_cnt] = pfc.vbus_value_new;
    pfc.vbus_move_filter_sum += pfc.vbus_move_filter_store[pfc.vbus_move_filter_cnt];

    pfc.vbus_move_filter_cnt = (pfc.vbus_move_filter_cnt + 1) % 1000;
    pfc.vbus_move_filter_value = pfc.vbus_move_filter_sum / 1000;

    // -----------------------------
    // VBUS 滤波输出
    // -----------------------------
//    pfc.vbus_rel = pfc.vbus_value;

    // -----------------------------
    // IBUS 一阶IIR
    // -----------------------------
    pfc.ibus_value_store[0] =
        pfc.i_samp_check * SAMP_RATIO / IPFC_SCALE_VAL
        + pfc.ibus_value_store[0]
        - (pfc.ibus_value_store[0] / IIN_FILTER_ORDER);

    pfc.i_value = pfc.ibus_value_store[0] / IIN_FILTER_ORDER;

    // -----------------------------
    // 电流环采样（物理量标定）
    // -----------------------------
    pfc.iloop.rel = (float)(pfc.i_samp_check * SAMP_RATIO) / IPFC_SCALE_VAL;
    if (pfc.iloop.rel < 0.0f)
    {
        pfc.iloop.rel = 0.0f;
    }
}

// ===============================
// 电流零偏校准：采 1024 点平均得到零电流ADC基线
// 必须保证校准期间“真实电流≈0”
// ===============================
void check_current_zero_offset(void)
{
    if (pfc.pre_finish_flg && !pfc.check_current_is_ok)
    {
        if (pfc.check_current_cnt == 0)
        {
            pfc.check_current_data_sum = 0;
        }

        pfc.check_current_data_sum += (uint32_t)i_ocp_samp[0];
        pfc.check_current_cnt++;

        if (pfc.check_current_cnt >= 1024)
        {
            pfc.check_current_data = (uint16_t)(pfc.check_current_data_sum >> 10); // /1024
            pfc.check_current_is_ok = 1;

        }
    }
}