#include "main.h"
#include "adc_app.h"

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
  user_adc_init.over_samp_cfg.trig_en   = true;
  user_adc_init.over_samp_cfg.ratio     = ADC_OVER_SAMP_RATIO_2;
  user_adc_init.over_samp_cfg.shift     = ADC_OVER_SAMP_SHIFT_RIGHT_1;
  user_adc_init.over_samp_cfg.norm_mode = ADC_NORM_OVER_SAMP_CONTINUE;
  user_adc_init.over_samp_cfg.reg_en    = true;
  user_adc_init.over_samp_cfg.inj_en    = false;
  LL_ADC_Init(ADC0, &user_adc_init);
  //User ADC Regular Common Config
  user_adc_reg_com_cfg.dis_cont_en = false;
  user_adc_reg_com_cfg.conv_mode   = ADC_REG_CONV_SINGLE;
  user_adc_reg_com_cfg.seq_len     = ADC_REG_SEQ_LEN_2;
  user_adc_reg_com_cfg.trig_pol    = ADC_SEQ_TRIG_POL_HW_RISING;
  user_adc_reg_com_cfg.trig_evt    = ADC_SEQ_TRIG_EVT_HRPWM_ADCTRG0;
  LL_ADC_REG_ComCfg(ADC0, &user_adc_reg_com_cfg);



  /*******************************  S_TRIM  ADC0_IN2  PA1  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_1;
  user_adc_reg_ch_cfg.ch          = S_TRIM_SAMPLE_CH2;
  user_adc_reg_ch_cfg.done_int_en = false;
  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
  LL_ADC_REG_ChCfg(ADC0, &user_adc_reg_ch_cfg);
  //User ADC DMA Config
  user_adc_dma_cfg.ch          = S_TRIM_SAMPLE_CH2;
  user_adc_dma_cfg.circ_en     = true;
  user_adc_dma_cfg.fix_addr_en = false;
  user_adc_dma_cfg.half_int_en = true;
  user_adc_dma_cfg.cplt_int_en = true;
  user_adc_dma_cfg.addr        = (uint32_t)&s_trim_samp;
  user_adc_dma_cfg.len         = sizeof(s_trim_samp);
  LL_ADC_DMA_Cfg(ADC0, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = S_TRIM_SAMPLE_CH2;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_0;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = 0;
  user_adc_cal_cfg.gain     = 8192;
  LL_ADC_Cal_Cfg(ADC0, &user_adc_cal_cfg);
  /*******************************  S_TRIM  ADC0_IN2  PA1  *******************************/

 
  /*******************************  VOUT  ADC0_CH4  PA3  CMP1_INP1 *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_2;
  user_adc_reg_ch_cfg.ch          = VOUT_SAMP_CH4;
  user_adc_reg_ch_cfg.done_int_en = false;
  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
  LL_ADC_REG_ChCfg(ADC0, &user_adc_reg_ch_cfg);
  //User ADC DMA Config
  user_adc_dma_cfg.ch          = VOUT_SAMP_CH4;
  user_adc_dma_cfg.circ_en     = true;
  user_adc_dma_cfg.fix_addr_en = false;
  user_adc_dma_cfg.half_int_en = true;
  user_adc_dma_cfg.cplt_int_en = true;
  user_adc_dma_cfg.addr        = (uint32_t)&vfb_samp;
  user_adc_dma_cfg.len         = sizeof(vfb_samp);
  LL_ADC_DMA_Cfg(ADC0, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = VOUT_SAMP_CH4;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_2;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = 0;
  user_adc_cal_cfg.gain     = 8192;
  LL_ADC_Cal_Cfg(ADC0, &user_adc_cal_cfg);
  /*******************************  VOUT  ADC0_CH4  PA3  CMP1_INP1 *******************************/


  //User ADC Init
  user_adc_init.overrun_mode   = ADC_OVERRUN_DATA_OVERWRITTEN;
  user_adc_init.dual_mode      = ADC_DUAL_MODE_INDEPEND;
  //user_adc_init.dual_phase_dly = 0;
  //Oversample Config
  user_adc_init.over_samp_cfg.trig_en   = true;
  user_adc_init.over_samp_cfg.ratio     = ADC_OVER_SAMP_RATIO_2;
  user_adc_init.over_samp_cfg.shift     = ADC_OVER_SAMP_SHIFT_RIGHT_1;
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

  /*******************************  IOUT  ADC1_CH3  PA6  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_1;
  user_adc_reg_ch_cfg.ch          = IOUT_SAMPLE_CH3;
  user_adc_reg_ch_cfg.done_int_en = false;
  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
  LL_ADC_REG_ChCfg(ADC1, &user_adc_reg_ch_cfg);
  //User ADC DMA Config
  user_adc_dma_cfg.ch          = IOUT_SAMPLE_CH3;
  user_adc_dma_cfg.circ_en     = true;
  user_adc_dma_cfg.fix_addr_en = false;
  user_adc_dma_cfg.half_int_en = true;
  user_adc_dma_cfg.cplt_int_en = true;
  user_adc_dma_cfg.addr        = (uint32_t)&iout_samp;
  user_adc_dma_cfg.len         = sizeof(iout_samp);
  LL_ADC_DMA_Cfg(ADC1, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = IOUT_SAMPLE_CH3;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_0;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = IOUT_OFFSET_VAL_CHECK_VALUE;
  user_adc_cal_cfg.gain     = IOUT_GAIN_VAL_CHECK_VALUE;
  LL_ADC_Cal_Cfg(ADC1, &user_adc_cal_cfg);
  /*******************************  IOUT  ADC1_CH3  PA6  *******************************/
	
  /*******************************  LOADSHARE  ADC1_CH12  PB2  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_2;
  user_adc_reg_ch_cfg.ch          = LOADSHARE_SAMPLE_CH12;
  user_adc_reg_ch_cfg.done_int_en = false;
  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
  LL_ADC_REG_ChCfg(ADC1, &user_adc_reg_ch_cfg);
  //User ADC DMA Config
  user_adc_dma_cfg.ch          = LOADSHARE_SAMPLE_CH12;
  user_adc_dma_cfg.circ_en     = true;
  user_adc_dma_cfg.fix_addr_en = false;
  user_adc_dma_cfg.half_int_en = true;
  user_adc_dma_cfg.cplt_int_en = true;
  user_adc_dma_cfg.addr        = (uint32_t)&loadshare_samp;
  user_adc_dma_cfg.len         = sizeof(loadshare_samp);
  LL_ADC_DMA_Cfg(ADC1, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = LOADSHARE_SAMPLE_CH12;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_1;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = 0;
  user_adc_cal_cfg.gain     = 8192;
  LL_ADC_Cal_Cfg(ADC1, &user_adc_cal_cfg);
  /*******************************  LOADSHARE  ADC1_CH12  PB2  *******************************/
	



  //User ADC Init
  user_adc_init.overrun_mode   = ADC_OVERRUN_DATA_OVERWRITTEN;
  user_adc_init.dual_mode      = ADC_DUAL_MODE_INDEPEND;
  //user_adc_init.dual_phase_dly = 0;
  //Oversample Config
  user_adc_init.over_samp_cfg.trig_en   = true;
  user_adc_init.over_samp_cfg.ratio     = ADC_OVER_SAMP_RATIO_2;
  user_adc_init.over_samp_cfg.shift     = ADC_OVER_SAMP_SHIFT_RIGHT_1;
  user_adc_init.over_samp_cfg.norm_mode = ADC_NORM_OVER_SAMP_CONTINUE;
  user_adc_init.over_samp_cfg.reg_en    = true;
  user_adc_init.over_samp_cfg.inj_en    = false;
  LL_ADC_Init(ADC2, &user_adc_init);
  //User ADC Regular Common Config
  user_adc_reg_com_cfg.dis_cont_en = false;
  user_adc_reg_com_cfg.conv_mode   = ADC_REG_CONV_SINGLE;
  user_adc_reg_com_cfg.seq_len     = ADC_REG_SEQ_LEN_1;
  user_adc_reg_com_cfg.trig_pol    = ADC_SEQ_TRIG_POL_HW_RISING;
  user_adc_reg_com_cfg.trig_evt    = ADC_SEQ_TRIG_EVT_HRPWM_ADCTRG0;
  LL_ADC_REG_ComCfg(ADC2, &user_adc_reg_com_cfg);
	
  /*******************************  SR_TEMP  ADC2_IN1  PB1  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_1;
  user_adc_reg_ch_cfg.ch          = SR_TEMP_SAMPLE_CH1;
  user_adc_reg_ch_cfg.done_int_en = false;
  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
  LL_ADC_REG_ChCfg(ADC2, &user_adc_reg_ch_cfg);
  //User ADC DMA Config
  user_adc_dma_cfg.ch          = SR_TEMP_SAMPLE_CH1;
  user_adc_dma_cfg.circ_en     = true;
  user_adc_dma_cfg.fix_addr_en = false;
  user_adc_dma_cfg.half_int_en = true;
  user_adc_dma_cfg.cplt_int_en = true;
  user_adc_dma_cfg.addr        = (uint32_t)&sr_temp;
  user_adc_dma_cfg.len         = sizeof(sr_temp);
  LL_ADC_DMA_Cfg(ADC2, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = SR_TEMP_SAMPLE_CH1;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_0;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = 0;
  user_adc_cal_cfg.gain     = 8192;
  LL_ADC_Cal_Cfg(ADC2, &user_adc_cal_cfg);
  /*******************************  SR_TEMP  ADC2_IN1  PB1  *******************************/

 

  //User HRPWM Common ADC Trigger Config
  user_hrpwm_comm_adctrig_cfg.trig_evt    = HRPWM_COMM_ADC02_TRIG_EVT_MST_PWM_CMPB;
  user_hrpwm_comm_adctrig_cfg.trig_len    = HRPWM_COMM_ADC_TRIG_EVT_LEN_15CLK;
  user_hrpwm_comm_adctrig_cfg.upd_src     = HRPWM_COMM_ADC_TRIG_UPD_SRC_MST_PWM;
  user_hrpwm_comm_adctrig_cfg.post_scaler = 0;
  LL_HRPWM_Comm_ADCTrigCfg(HRPWM, HRPWM_ADC_TRIG_NUM_0, &user_hrpwm_comm_adctrig_cfg);


//    //Preload Enable, software need to generate a update event to update the shadow register to the working register
//    //If Preload Disable, don't need this action.
//    __LL_HRPWM_Comm_MstPWMSwUpdReg_Set(HRPWM);



}

float alpha = 0.8;
uint16_t loadshare_filter_cnt = 0;
bool loadshare_filter_comp = 0;
uint16_t loadshare_filter = LOADSHARE_FILTER_CNT;
RAMCODE
void get_adc_value(void)
{
//	llc.temp_value = 	temp_samp[0];
//	llc.s_trim_value = 	s_trim_samp[0];
//	llc.vfb_value = 	vfb_samp[0];
//	llc.loadshare_value  = 	loadshare_samp[0];
//	llc.iout_value  = 	iout_samp[0];
//	llc.addr1_value  = 	can_addr1_samp[0];
	
	llc.sr_temp_value = sr_temp[0];


  llc.vout_value_store[0] = vfb_samp [0]*VOUT_SAMP_RATIO + llc.vout_value_store[0] - (llc.vout_value_store[0] / VOUT_FILTER_ORDER);
  llc.vout_filter_cnt = (llc.vout_filter_cnt + 1) % (4) ;

  llc.iout_value_store[llc.iout_filter_cnt] = ((iout_samp [0]*SAMP_RATIO)/IOUT_GAIN_VAL) + llc.iout_value_store[llc.iout_filter_cnt] - (llc.iout_value_store[llc.iout_filter_cnt]/ IOUT_FILTER_ORDER);
  llc.iout_filter_cnt = (llc.iout_filter_cnt + 1) % (4);

  llc.iout_value = ((iout_samp [0]*SAMP_RATIO-IOUT_OFFSET_VAL)/IOUT_GAIN_VAL) + llc.iout_value - (llc.iout_value / IOUT_FILTER_ORDER);

	llc.loadshare_value_store =	(((loadshare_samp [0]*SAMP_RATIO-IOUT_OFFSET_VAL)/IOUT_GAIN_VAL) + llc.loadshare_value_store - (llc.loadshare_value_store / LOADSHARE_FILTER_CNT));
	llc.shareloop.ref  = (llc.loadshare_value_store / LOADSHARE_FILTER_CNT)*1.249f-0.6184f;
	if(llc.shareloop.ref<0)llc.shareloop.ref=0;
	llc.loadcurrent_value_store = ((iout_samp [0]*SAMP_RATIO-IOUT_OFFSET_VAL)/IOUT_GAIN_VAL) + llc.loadcurrent_value_store - (llc.loadcurrent_value_store/ LOADSHARE_FILTER_CNT);
	llc.shareloop.rel = llc.loadcurrent_value_store/LOADSHARE_FILTER_CNT;
	
  if(llc.vout_filter_cnt == 0)
    {
      llc.temp_value_store =	(sr_temp[0] + llc.temp_value_store - (llc.temp_value_store / TEMP_FILTER_CNT));
      llc.temp_value = llc.temp_value_store / TEMP_FILTER_CNT ;
      llc.temp_c = (int8_t)(llc.temp_value*(float)(-0.02841) + (float)152.7);
    }

  if(llc.vout_filter_cnt == 1)
    {
      llc.s_trim_value_store =	(s_trim_samp[0]*SAMP_RATIO + llc.s_trim_value_store - (llc.s_trim_value_store/ TRIM_FILTER_CNT));
      llc.s_trim_value = llc.s_trim_value_store / TRIM_FILTER_CNT;
      llc.vout_trim = ((float)1.852*(llc.s_trim_value*llc.s_trim_value*llc.s_trim_value))-((float)7.562*(llc.s_trim_value*llc.s_trim_value))+((float)14.03*llc.s_trim_value);
    }

  if(llc.vout_filter_cnt == 2)
    {

//      llc.loadshare_value_store =	(((loadshare_samp[0]*SAMP_RATIO-IOUT_OFFSET_VAL)/IOUT_GAIN_VAL) + llc.loadshare_value_store - (llc.loadshare_value_store / LOADSHARE_FILTER_CNT));

//      llc.loadshare_move_value = llc.loadshare_value_store / LOADSHARE_FILTER_CNT;

//      llc.loadshare_value = llc.loadshare_move_value;
//      llc.shareloop.ref = llc.loadshare_value;


    }

  if(llc.vout_filter_cnt == 3)
    {
      if((!llc.addr_set_flag))
        {
          llc.addr_cnt++; 
          if(llc.addr_cnt > 5000)
            {
              can_addr_set();
            }
        }

//		llc.temp_filtered = llc.temp_value + llc.temp_filtered - (llc.temp_filtered >> 3);
//		txdata_buf.vin_squares_average = llc.vin_squares_average;
//		txdata_buf.Fault_Flag = llc.Fault_Flag;
//		txdata_buf.temp_pri_ad = llc.temp_filtered >> 3;
    }
}

RAMCODE
void 	data_slow_filter(void)
{
  llc.iout_rel_slow_store =	(llc.iout_rel + llc.iout_rel_slow_store - (llc.iout_rel_slow_store/ IOUT_SLOW_FILTER_CNT));
  llc.iout_rel_slow = llc.iout_rel_slow_store / IOUT_SLOW_FILTER_CNT ;
//  llc.shareloop.rel = llc.iout_rel_slow  ;

  llc.vbus_rel_slow_store =	(llc.vbus_rel + llc.vbus_rel_slow_store - (llc.vbus_rel_slow_store/ VBUS_SLOW_FILTER_CNT));
  llc.vbus_rel_slow = llc.vbus_rel_slow_store / VBUS_SLOW_FILTER_CNT;
}

//void llc_low_temp_adj()
//{
//	llc.test_load = 12.5f;

//	float p1,p2,p3,p4,p5,p6;
//	float x = llc.test_load;
//	float y;
//	p1 = 0.0001f;
//	p2 = -0.0036f;
//	p3 = 0.0346f;
//	p4 = -0.1309f;
//	p5 = -0.17f;
//	
//	y=p1*x*x*x*x+p2*x*x*x+p3*x*x+p4*x+p5;

//	llc.test_vout = y;
//}



/* ============================================================= */
/* 低温补偿限幅配置 - 按需修改此处                                */
#define LLC_LOW_TEMP_LIMIT_H    (0.0f)       /* 补偿上限 (V)，负值，靠近 0  */
#define LLC_LOW_TEMP_LIMIT_L    (-1.2000f)   /* 补偿下限 (V)，负值，远离 0  */
/* ============================================================= */

/**
 * @brief  低温 PFC 电压补偿增量计算
 *         仅在低温区间介入，常温返回 0.0f
 * @param  T : 散热片温度 (C)
 * @param  I : 电感电流 (A)
 * @return delta : 补偿增量 (V)，调用方: vout_ref = 48.0f - delta
 */
RAMCODE
void llc_low_temp_adj() {
    float T, I;
    I = llc.iout_rel;
    T = llc.temp_c;

    /* 0. 物理边界配置 */
    static const float TEMP_ENTER = -5.0f;    /* 介入点：T > -5°C 时不补偿          */
    static const float TEMP_FULL  = -15.0f;   /* 全量点：T < -15°C 时 100% 补偿生效 */
    static const float V_NOM      = 48.000f;  /* 48V 基准                           */
    static const float T_MIN      = -45.0f;   /* 钳位下限（钳位到拟合域外推保护）    */
    static const float T_MAX      = -5.0f;    /* 钳位上限                           */
    static const float I_MIN      = 0.0f;     /* 最小负载 0A                        */
    static const float I_MAX      = 12.5f;    /* 最大负载 12.5A                     */

    /* 1. 输入合法性校验：NaN 自判
     * T != T 在 NaN 时恒为真，兼容无 isnan() 的标准 MCU             */
    if ((T != T) || (I != I)) {
        llc.low_temp_delta = 0.0f;
        return;
    }

    /* 2. 常温不介入，直接返回 0 补偿 */
    if (T >= TEMP_ENTER) {
        llc.low_temp_delta = 0.0f;
        return;
    }

    /* 3. 输入钳位 (Saturation)
     * 超出物理边界的输入强制钳位，防止多项式在拟合域外发散          */
    float Tc = fmaxf(T_MIN, fminf(T, T_MAX));
    float Ic = fmaxf(I_MIN, fminf(I, I_MAX));

    /* 4. 归一化参数
     * ?? 必须与 MATLAB 拟合时完全一致，不可自行推算
     * 原始拟合范围: T ∈ [-29, -9]°C, I ∈ [0, 12.5]A
     * mean/std 由 MATLAB fit() 基于实际数据点计算得出               */
    static const float mean_T = -19.33333333f; /* MATLAB 导出原值，勿改 */
    static const float std_T  =   8.46561673f; /* MATLAB 导出原值，勿改 */
    static const float mean_I =   6.25000000f; /* MATLAB 导出原值，勿改 */
    static const float std_I  =   4.57453159f; /* MATLAB 导出原值，勿改 */

    /* 5. 标准化 (Z-Score Normalization)
     * 将物理单位转化为无量纲标准化数值，防止多项式大数溢出           */
    float zT = (Tc - mean_T) / std_T;         /* 温度标准化偏移量 */
    float zI = (Ic - mean_I) / std_I;         /* 电流标准化偏移量 */

    /* 6. 预计算幂项 (Power Terms) */
    float zT2 = zT * zT;                      /* zT 的平方   */
    float zI2 = zI * zI;                      /* zI 的平方   */
    float zI3 = zI2 * zI;                     /* zI 的立方   */
    float zI4 = zI3 * zI;                     /* zI 的四次方 */

    /* 7. 多项式系数 (Surface Fitting Coefficients)
     * ?? 与归一化参数强绑定，勿单独修改任意一项
     * 公式形式: V = p00 + p10*zT + p01*zI + p20*zT^2 + p11*zT*zI + ... */
    static const float p00 =  47.56590078563411f; /* 常数项 (截距)  */
    static const float p10 =   0.08559842019572f; /* zT 一次项      */
    static const float p01 =  -0.01432010137346f; /* zI 一次项      */
    static const float p20 =  -0.00275084175083f; /* zT 二次项      */
    static const float p11 =   0.02199420165542f; /* zT*zI 交互项   */
    static const float p02 =  -0.05484247234247f; /* zI 二次项      */
    static const float p03 =  -0.03833893143726f; /* zI 三次项      */
    static const float p13 =  -0.00646968836198f; /* zT*zI^3 交互项 */
    static const float p04 =   0.04719387755102f; /* zI 四次项      */

    /* 8. 多项式求值 (Equation Execution) */
    float V_fit = p00
                + p10 * zT
                + p01 * zI
                + p20 * zT2
                + p11 * zT * zI
                + p02 * zI2
                + p03 * zI3
                + p13 * zT * zI3
                + p04 * zI4;

    /* 9. 多项式结果合法性校验
     * 防止外推发散产生异常值写入输出，NaN 自判兼容标准 MCU           */
    if (V_fit != V_fit) {
        llc.low_temp_delta = 0.0f;
        return;
    }

     /* 10. 偏移量限幅
     * delta = V_fit - 48V，正常范围全为负值（低温下输出偏低）
     * 调用方用 48.0f - delta 抬高参考电压以补偿外部压降
     * 限幅值在顶部宏定义处修改，LIMIT_H 靠近 0，LIMIT_L 远离 0      */
    float delta = V_fit - V_NOM;
    if (delta > LLC_LOW_TEMP_LIMIT_H) delta = LLC_LOW_TEMP_LIMIT_H;
    if (delta < LLC_LOW_TEMP_LIMIT_L) delta = LLC_LOW_TEMP_LIMIT_L;

    /* 11. 软过渡淡入 (Soft Transition Blend)
     * T < -15°C : blend = 1.0，全量补偿
     * -15 ~ -5°C: blend 线性从 1.0 → 0.0，平滑介入防止环路震荡
     * T > -5°C  : 已在步骤 2 拦截，不会到达此处                     */
    if (T > TEMP_FULL) {
        float blend = (TEMP_ENTER - T) / (TEMP_ENTER - TEMP_FULL);
        if (blend < 0.0f) blend = 0.0f;   /* 浮点边界保护 */
        if (blend > 1.0f) blend = 1.0f;
        delta *= blend;
    }

    llc.low_temp_delta = delta;  /* 调用方: vout_ref = 48.0f - delta */
}
/* ============================================================= */