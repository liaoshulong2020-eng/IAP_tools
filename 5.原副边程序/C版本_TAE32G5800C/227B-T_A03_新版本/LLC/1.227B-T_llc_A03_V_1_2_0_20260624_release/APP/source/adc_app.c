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
  user_adc_reg_com_cfg.seq_len     = ADC_REG_SEQ_LEN_3;
  user_adc_reg_com_cfg.trig_pol    = ADC_SEQ_TRIG_POL_HW_RISING;
  user_adc_reg_com_cfg.trig_evt    = ADC_SEQ_TRIG_EVT_HRPWM_ADCTRG0;
  LL_ADC_REG_ComCfg(ADC0, &user_adc_reg_com_cfg);
  /*******************************  TEMP  ADC0_CH1  PA0  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_1;
  user_adc_reg_ch_cfg.ch          = TEMP_SAMPLE_CH1;
  user_adc_reg_ch_cfg.done_int_en = false;
  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
  LL_ADC_REG_ChCfg(ADC0, &user_adc_reg_ch_cfg);
  //User ADC DMA Config
  user_adc_dma_cfg.ch          = TEMP_SAMPLE_CH1;
  user_adc_dma_cfg.circ_en     = true;
  user_adc_dma_cfg.fix_addr_en = false;
  user_adc_dma_cfg.half_int_en = true;
  user_adc_dma_cfg.cplt_int_en = true;
  user_adc_dma_cfg.addr        = (uint32_t)&temp_samp;
  user_adc_dma_cfg.len         = sizeof(temp_samp);
  LL_ADC_DMA_Cfg(ADC0, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = TEMP_SAMPLE_CH1;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_0;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = 0;
  user_adc_cal_cfg.gain     = 8192;
  LL_ADC_Cal_Cfg(ADC0, &user_adc_cal_cfg);
  /*******************************  TEMP  ADC0_CH1  PA0  *******************************/

  /*******************************  S_TRIM  ADC0_CH2  PA1  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_2;
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
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_1;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = 0;
  user_adc_cal_cfg.gain     = 8192;
  LL_ADC_Cal_Cfg(ADC0, &user_adc_cal_cfg);
  /*******************************  S_TRIM  ADC0_CH2  PA1  *******************************/

  /*******************************  VOUT  ADC0_CH4  PA3  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_3;
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
  user_adc_dma_cfg.addr        = (uint32_t)&vout_samp;
  user_adc_dma_cfg.len         = sizeof(vout_samp);
  LL_ADC_DMA_Cfg(ADC0, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = VOUT_SAMP_CH4;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_2;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = 0;
  user_adc_cal_cfg.gain     = 0x2000;
  LL_ADC_Cal_Cfg(ADC0, &user_adc_cal_cfg);
  /*******************************  I_SURGE  ADC0_CH4  PA3  *******************************/


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

  /*******************************  VFB  ADC1_IN4  PA7  CMP1_INP0	 *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_1;
  user_adc_reg_ch_cfg.ch          = VFB_SAMPLE_CH4;
  user_adc_reg_ch_cfg.done_int_en = false;
  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
  LL_ADC_REG_ChCfg(ADC1, &user_adc_reg_ch_cfg);
  //User ADC DMA Config
  user_adc_dma_cfg.ch          = VFB_SAMPLE_CH4;
  user_adc_dma_cfg.circ_en     = true;
  user_adc_dma_cfg.fix_addr_en = false;
  user_adc_dma_cfg.half_int_en = true;
  user_adc_dma_cfg.cplt_int_en = true;
  user_adc_dma_cfg.addr        = (uint32_t)&vfb_samp;
  user_adc_dma_cfg.len         = sizeof(vfb_samp);
  LL_ADC_DMA_Cfg(ADC1, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = VFB_SAMPLE_CH4;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_0;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = 0;
  user_adc_cal_cfg.gain     = 8192;
  LL_ADC_Cal_Cfg(ADC1, &user_adc_cal_cfg);
  /*******************************  VFB  ADC1_IN4  PA7  CMP1_INP0	 *******************************/

  /*******************************  LOADSHARE  ADC1_IN12  PB2  *******************************/
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
  user_adc_cal_cfg.gain     = 0x2000;
  LL_ADC_Cal_Cfg(ADC1, &user_adc_cal_cfg);
  /*******************************  LOADSHARE  ADC1_IN12  PB2  *******************************/

//  /*******************************  ADDR1  ADC1_IN13  PA5  *******************************/
//  //User ADC Regular Channel Config
//  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_3;
//  user_adc_reg_ch_cfg.ch          = ADDR1_SAMPLE_CH13;
//  user_adc_reg_ch_cfg.done_int_en = false;
//  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
//  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
//  LL_ADC_REG_ChCfg(ADC1, &user_adc_reg_ch_cfg);
//  //User ADC DMA Config
//  user_adc_dma_cfg.ch          = ADDR1_SAMPLE_CH13;
//  user_adc_dma_cfg.circ_en     = true;
//  user_adc_dma_cfg.fix_addr_en = false;
//  user_adc_dma_cfg.half_int_en = true;
//  user_adc_dma_cfg.cplt_int_en = true;
//  user_adc_dma_cfg.addr        = (uint32_t)&can_addr1_samp;
//  user_adc_dma_cfg.len         = sizeof(can_addr1_samp);
//  LL_ADC_DMA_Cfg(ADC1, &user_adc_dma_cfg);
//  //User ADC Calibration Config
//  user_adc_cal_cfg.ch       = ADDR1_SAMPLE_CH13;
//  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_1;
//  user_adc_cal_cfg.sat_dis  = false;
//  user_adc_cal_cfg.offset   = 0;
//  user_adc_cal_cfg.gain     = 0x2000;
//  LL_ADC_Cal_Cfg(ADC1, &user_adc_cal_cfg);
//  /*******************************  ADDR1  ADC1_IN13  PA5  *******************************/

//  /*******************************  ADDR3  ADC1_IN3  PA6  *******************************/
//  //User ADC Regular Channel Config
//  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_4;
//  user_adc_reg_ch_cfg.ch          = ADDR3_SAMPLE_CH3;
//  user_adc_reg_ch_cfg.done_int_en = false;
//  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
//  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
//  LL_ADC_REG_ChCfg(ADC1, &user_adc_reg_ch_cfg);
//  //User ADC DMA Config
//  user_adc_dma_cfg.ch          = ADDR3_SAMPLE_CH3;
//  user_adc_dma_cfg.circ_en     = true;
//  user_adc_dma_cfg.fix_addr_en = false;
//  user_adc_dma_cfg.half_int_en = true;
//  user_adc_dma_cfg.cplt_int_en = true;
//  user_adc_dma_cfg.addr        = (uint32_t)&can_addr3_samp;
//  user_adc_dma_cfg.len         = sizeof(can_addr3_samp);
//  LL_ADC_DMA_Cfg(ADC1, &user_adc_dma_cfg);
//  //User ADC Calibration Config
//  user_adc_cal_cfg.ch       = ADDR3_SAMPLE_CH3;
//  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_1;
//  user_adc_cal_cfg.sat_dis  = false;
//  user_adc_cal_cfg.offset   = 0;
//  user_adc_cal_cfg.gain     = 0x2000;
//  LL_ADC_Cal_Cfg(ADC1, &user_adc_cal_cfg);
//  /*******************************  ADDR3  ADC1_IN3  PA6  *******************************/



  //User ADC Init
  user_adc_init.overrun_mode   = ADC_OVERRUN_DATA_OVERWRITTEN;
  user_adc_init.dual_mode      = ADC_DUAL_MODE_INDEPEND;
  //user_adc_init.dual_phase_dly = 0;
  //Oversample Config
  user_adc_init.over_samp_cfg.trig_en   = true;
  user_adc_init.over_samp_cfg.ratio     = ADC_OVER_SAMP_RATIO_8;
  user_adc_init.over_samp_cfg.shift     = ADC_OVER_SAMP_SHIFT_RIGHT_3;
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
  /*******************************  IOUT  ADC2_IN12  PB0  *******************************/
  //User ADC Regular Channel Config
  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_1;
  user_adc_reg_ch_cfg.ch          = IOUT_SAMPLE_CH12;
  user_adc_reg_ch_cfg.done_int_en = false;
  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
  LL_ADC_REG_ChCfg(ADC2, &user_adc_reg_ch_cfg);
  //User ADC DMA Config
  user_adc_dma_cfg.ch          = IOUT_SAMPLE_CH12;
  user_adc_dma_cfg.circ_en     = true;
  user_adc_dma_cfg.fix_addr_en = false;
  user_adc_dma_cfg.half_int_en = true;
  user_adc_dma_cfg.cplt_int_en = true;
  user_adc_dma_cfg.addr        = (uint32_t)&iout_samp;
  user_adc_dma_cfg.len         = sizeof(iout_samp);
  LL_ADC_DMA_Cfg(ADC2, &user_adc_dma_cfg);
  //User ADC Calibration Config
  user_adc_cal_cfg.ch       = IOUT_SAMPLE_CH12;
  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_0;
  user_adc_cal_cfg.sat_dis  = false;
  user_adc_cal_cfg.offset   = IOUT_OFFSET_VAL_CHECK_VALUE;
  user_adc_cal_cfg.gain     = IOUT_GAIN_VAL_CHECK_VALUE;
  LL_ADC_Cal_Cfg(ADC2, &user_adc_cal_cfg);
  /*******************************  IOUT  ADC2_IN12  PB0  *******************************/

//  /*******************************  ADDR2  ADC2_IN1  PB1  *******************************/
//  //User ADC Regular Channel Config
//  user_adc_reg_ch_cfg.seq_num     = ADC_REG_SEQ_NUM_2;
//  user_adc_reg_ch_cfg.ch          = ADDR2_SAMPLE_CH1;
//  user_adc_reg_ch_cfg.done_int_en = false;
//  user_adc_reg_ch_cfg.input_mode  = ADC_INPUT_MODE_SINGLE_END;
//  user_adc_reg_ch_cfg.samp_time   = ADC_SAMP_TIME_6_CYCLES;
//  LL_ADC_REG_ChCfg(ADC2, &user_adc_reg_ch_cfg);
//  //User ADC DMA Config
//  user_adc_dma_cfg.ch          = ADDR2_SAMPLE_CH1;
//  user_adc_dma_cfg.circ_en     = true;
//  user_adc_dma_cfg.fix_addr_en = false;
//  user_adc_dma_cfg.half_int_en = true;
//  user_adc_dma_cfg.cplt_int_en = true;
//  user_adc_dma_cfg.addr        = (uint32_t)&can_addr2_samp;
//  user_adc_dma_cfg.len         = sizeof(can_addr2_samp);
//  LL_ADC_DMA_Cfg(ADC2, &user_adc_dma_cfg);
//  //User ADC Calibration Config
//  user_adc_cal_cfg.ch       = ADDR2_SAMPLE_CH1;
//  user_adc_cal_cfg.coef_grp = ADC_CAL_COEF_GRP_1;
//  user_adc_cal_cfg.sat_dis  = false;
//  user_adc_cal_cfg.offset   = 0;
//  user_adc_cal_cfg.gain     = 0x2000;
//  LL_ADC_Cal_Cfg(ADC2, &user_adc_cal_cfg);
//  /*******************************  ADDR2  ADC2_IN1  PB1  *******************************/


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

uint16_t loadshare_filter_cnt = 0;
bool loadshare_filter_comp = 0;
uint16_t loadshare_filter = LOADSHARE_FILTER_CNT;
static uint32_t share_loop_ref_flt_cnt = 0;
RAMCODE
void get_adc_value(void)
{
//	llc.temp_value = 	temp_samp[0];
//	llc.s_trim_value = 	s_trim_samp[0];
  llc.vout_value1 = 	vout_samp[0];
//	llc.vfb_value = 	vfb_samp[0];
//	llc.loadshare_value  = 	loadshare_samp[0];
//	llc.iout_value  = 	iout_samp[0];
//	llc.addr1_value  = 	can_addr1_samp[0];



  llc.vout_value_store[0] = vfb_samp [0]*VOUT_SAMP_RATIO + llc.vout_value_store[0] - (llc.vout_value_store[0] / VOUT_FILTER_ORDER);
  llc.vout_filter_cnt = (llc.vout_filter_cnt + 1) % (4);

  llc.iout_value_store[0] = ((iout_samp [0]*SAMP_RATIO-IOUT_OFFSET_VAL)/IOUT_GAIN_VAL) + llc.iout_value_store[0] - (llc.iout_value_store[0]/ IOUT_FILTER_ORDER);
  llc.iout_filter_cnt = (llc.iout_filter_cnt + 1) % (4);

  llc.iout_value = ((iout_samp [0]*SAMP_RATIO-IOUT_OFFSET_VAL)/IOUT_GAIN_VAL) + llc.iout_value - (llc.iout_value / IOUT_FILTER_ORDER);

      llc.loadshare_value_store =	(((loadshare_samp [0]*SAMP_RATIO-IOUT_OFFSET_VAL)/IOUT_GAIN_VAL) + llc.loadshare_value_store - (llc.loadshare_value_store / LOADSHARE_FILTER_CNT));
      llc.shareloop.ref  = (llc.loadshare_value_store / LOADSHARE_FILTER_CNT);
      
			llc.loadcurrent_value_store = ((iout_samp [0]*SAMP_RATIO-IOUT_OFFSET_VAL)/IOUT_GAIN_VAL) + llc.loadcurrent_value_store - (llc.loadcurrent_value_store/ LOADSHARE_FILTER_CNT);
			llc.shareloop.rel = llc.loadcurrent_value_store/LOADSHARE_FILTER_CNT;
	
  if(llc.vout_filter_cnt == 0)
    {
      llc.temp_value_store =	(temp_samp[0] + llc.temp_value_store - (llc.temp_value_store / TEMP_FILTER_CNT));
      llc.temp_value = llc.temp_value_store / TEMP_FILTER_CNT ;
      llc.temp_c = (int8_t)(llc.temp_value*(float)(-0.02841) + (float)152.7);
    }

  if(llc.vout_filter_cnt == 1)
    {
      llc.s_trim_value_store =	(s_trim_samp[0]*SAMP_RATIO + llc.s_trim_value_store - (llc.s_trim_value_store/ TRIM_FILTER_CNT));
      llc.s_trim_value = llc.s_trim_value_store / TRIM_FILTER_CNT;
//			if(llc.s_trim_value >1.87f && llc.s_trim_value < 1.895f )
//			{
//					llc.vout_trim = 28.0f;
//			}
//			else
			{
			   llc.vout_trim = llc.s_trim_value*14.8493f-0.004f;

			}
				 llc.vout_hw_trim_delta = 0;//llc.vout_trim - 28.0f;
    }

  if(llc.vout_filter_cnt == 2)
    {
//			float alpha;
//			float x,y;
//      llc.loadshare_value_store =	((loadshare_samp[0]) + llc.loadshare_value_store - (llc.loadshare_value_store / LOADSHARE_FILTER_CNT));

//      llc.loadshare_move_value = llc.loadshare_value_store / LOADSHARE_FILTER_CNT;

//      llc.loadshare_value = llc.loadshare_move_value;
//      llc.shareloop.ref = (llc.loadshare_value*SAMP_RATIO-IOUT_OFFSET_VAL)/IOUT_GAIN_VAL;
//			if (share_loop_ref_flt_cnt == 0)
//					share_loop_ref_flt_cnt = 1;
//			
//			if (share_loop_ref_flt_cnt < 10000) {
//						alpha = 1.0f / (float)share_loop_ref_flt_cnt++;
//				} else {
//						alpha = 0.0001f;
//					share_loop_ref_flt_cnt = 10000;
//				}
//		llc.shareloop_ref_flt = (1.0f - alpha) * llc.shareloop_ref_flt + alpha * llc.shareloop.ref;
			

    }

  if(llc.vout_filter_cnt == 3)
    {
//      llc.addr1_value_store =	(can_addr1_samp[0] + llc.addr1_value_store - (llc.addr1_value_store >> ADDR1_FILTER_CNT));
//      llc.addr1_value = llc.addr1_value_store >> ADDR1_FILTER_CNT;
//			
//			llc.addr2_value_store =	(can_addr2_samp[0] + llc.addr2_value_store - (llc.addr2_value_store >> ADDR1_FILTER_CNT));
//      llc.addr2_value = llc.addr2_value_store >> ADDR1_FILTER_CNT;
//			
//			llc.addr3_value_store =	(can_addr3_samp[0] + llc.addr3_value_store - (llc.addr3_value_store >> ADDR1_FILTER_CNT));
//      llc.addr3_value = llc.addr3_value_store >> ADDR1_FILTER_CNT;



//		llc.temp_filtered = llc.temp_value + llc.temp_filtered - (llc.temp_filtered >> 3);
//		txdata_buf.vin_squares_average = llc.vin_squares_average;
//		txdata_buf.Fault_Flag = llc.Fault_Flag;
//		txdata_buf.temp_pri_ad = llc.temp_filtered >> 3;
    }
}

static uint32_t share_loop_rel_flt_cnt = 0;
RAMCODE
void 	data_slow_filter(void)
{
	float rel_alpha;
	
  llc.iout_rel_slow_store =	(llc.iout_rel + llc.iout_rel_slow_store - (llc.iout_rel_slow_store/ IOUT_SLOW_FILTER_CNT));
  llc.iout_rel_slow = llc.iout_rel_slow_store / IOUT_SLOW_FILTER_CNT ;
	
	
	if (share_loop_rel_flt_cnt == 0)
					share_loop_rel_flt_cnt = 1;
			
		if (share_loop_rel_flt_cnt < 1000) 
				{
					rel_alpha = 1.0f / (float)share_loop_rel_flt_cnt++;
				} 
				else
				{
					rel_alpha = 0.001f;
					share_loop_rel_flt_cnt = 1000;
				}
//		llc.shareloop.rel = (1.0f - rel_alpha) * llc.shareloop.rel + rel_alpha * llc.iout_rel_slow;
			
	
  llc.vbus_rel_slow_store =	(llc.vbus_rel + llc.vbus_rel_slow_store - (llc.vbus_rel_slow_store/ VBUS_SLOW_FILTER_CNT));
  llc.vbus_rel_slow = llc.vbus_rel_slow_store / VBUS_SLOW_FILTER_CNT;
}

