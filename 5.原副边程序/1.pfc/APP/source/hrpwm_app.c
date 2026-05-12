#include "main.h"

#define PFC_DRIVER_SLAVE_3	HRPWM_SLV_PWM_3
#define PFC_PRE_SLAVE_0			HRPWM_SLV_PWM_0

void hrpwm_data_init(void)
{
  /*mpwm init*/
  mpwm.period  = PFC_DRIVER_PERIOD_VALUE;                   //100K
  mpwm.compa   = 20;
  mpwm.compb   = 30;
  mpwm.compc   = 40;
  mpwm.compd   = 3199;
  /*pwm2 init*/
  pwm2.period  = PFC_DRIVER_PERIOD_VALUE;                    //100K
  pwm2.compa   = 100;
  pwm2.compb   = 50;
  /*pwm3 init*/
  pwm3.period  = PFC_DRIVER_PERIOD_VALUE;                    //100K
  pwm3.compa   = 100;
  pwm3.compb   = 100;
  pwm3.compc   = 100;
  pwm3.compd   = 50;
	
	  /*pwm0 init*/
  pwm0.period  = PFC_DRIVER_PERIOD_VALUE;                    //100K
  pwm0.compa   = 100;
  pwm0.compb   = 100;
  pwm0.compc   = 100;
  pwm0.compd   = 50;
	pfc.pre_driver_period = PFC_DRIVER_PERIOD_VALUE;
	pfc.pre_driver_cmpb = 100;
	
}

void user_hrpwm_config(HRPWM_TypeDef* Instance)
{
  HRPWM_TmrBaseCfgTypeDef				user_hrpwm_tmr_base_cfg;
  HRPWM_TmrCmpCfgTypeDef				user_hrpwm_tmr_cmp_cfg;
  HRPWM_Slv_OutputCfgTypeDef		user_hrpwm_slv_output_cfg;
  HRPWM_Slv_DeadTimeCfgTypeDef 	user_hrpwm_slv_deadtime_cfg;
  HRPWM_Comm_FltCfgTypeDef			user_hrpwm_comm_flt_cfg;
  HRPWM_Comm_ADCTrigCfgTypeDef user_hrpwm_comm_adctrig_cfg;
  memset((void*)&user_hrpwm_tmr_base_cfg,      0x00, sizeof(user_hrpwm_tmr_base_cfg));
  memset((void*)&user_hrpwm_tmr_cmp_cfg,       0x00, sizeof(user_hrpwm_tmr_cmp_cfg));
  memset((void*)&user_hrpwm_slv_output_cfg,    0x00, sizeof(user_hrpwm_slv_output_cfg));
  memset((void*)&user_hrpwm_slv_deadtime_cfg,  0x00, sizeof(user_hrpwm_slv_deadtime_cfg));
  memset((void*)&user_hrpwm_comm_flt_cfg,  			0x00, sizeof(user_hrpwm_comm_flt_cfg));
  memset((void*)&user_hrpwm_comm_adctrig_cfg, 0x00, sizeof(user_hrpwm_comm_adctrig_cfg));
  //User HRPWM Timer Base Config
  user_hrpwm_tmr_base_cfg.rep_prd    = 0;
  user_hrpwm_tmr_base_cfg.cntr_prd   = mpwm.period;
  user_hrpwm_tmr_base_cfg.int_en_msk = HRPWM_SLV_INT_NONE;
  user_hrpwm_tmr_base_cfg.clk_prescl = HRPWM_CLK_PRESCL_MUL_16;
  user_hrpwm_tmr_base_cfg.work_mode  = HRPWM_WORK_MODE_CONTINUE;
  user_hrpwm_tmr_base_cfg.sync_rst_en      = false;
  user_hrpwm_tmr_base_cfg.sync_start_en    = false;
  user_hrpwm_tmr_base_cfg.single_retrig_en = false;
  user_hrpwm_tmr_base_cfg.cntr_rst_evt     = HRPWM_SLV0_CNTR_RST_EVT_MST_PWM_PRD;
  user_hrpwm_tmr_base_cfg.resync_mode      = HRPWM_SLV_RESYNC_NEXT_RST_ROLLOVER;
  user_hrpwm_tmr_base_cfg.half_mode_en        = false;
  user_hrpwm_tmr_base_cfg.intlvd_mode         = HRPWM_INTLVD_MODE_CLOSE;
  user_hrpwm_tmr_base_cfg.push_pull_en        = false;
  user_hrpwm_tmr_base_cfg.trig_half_en        = false;
  user_hrpwm_tmr_base_cfg.cmpA_greatr_than_en = false;
  user_hrpwm_tmr_base_cfg.cmpC_greatr_than_en = false;
  user_hrpwm_tmr_base_cfg.upd_gate            = HRPWM_SLV_UPD_GATE_BST_DMA_INDEPEND;
  user_hrpwm_tmr_base_cfg.cmpB_auto_dly_mode  = HRPWM_SLV_CMPB_AUTO_DLY_ALWAYS;
  user_hrpwm_tmr_base_cfg.cmpD_auto_dly_mode  = HRPWM_SLV_CMPD_AUTO_DLY_ALWAYS;

  //Master PWM Timer Base Config
  LL_HRPWM_TmrBaseCfg(Instance, HRPWM_MST_PWM, &user_hrpwm_tmr_base_cfg);
  //Slave PWMx Timer Base Config
  LL_HRPWM_TmrBaseCfg(Instance, PFC_DRIVER_SLAVE_3, &user_hrpwm_tmr_base_cfg);
  LL_HRPWM_TmrBaseCfg(Instance, PFC_PRE_SLAVE_0, &user_hrpwm_tmr_base_cfg);
  //User HRPWM Timer Compare Config
  user_hrpwm_tmr_cmp_cfg.pre_load_en    = true;
  user_hrpwm_tmr_cmp_cfg.cmp_a_val      = mpwm.compa;
  user_hrpwm_tmr_cmp_cfg.cmp_b_val      = mpwm.compb;
  user_hrpwm_tmr_cmp_cfg.cmp_c_val      = mpwm.compc;
  user_hrpwm_tmr_cmp_cfg.cmp_d_val      = mpwm.compd;
  //Master PWM Timer Compare Config
  LL_HRPWM_TmrCmpCfg(Instance, HRPWM_MST_PWM, &user_hrpwm_tmr_cmp_cfg);

  //User HRPWM Timer Compare Config
  user_hrpwm_tmr_cmp_cfg.pre_load_en    = true;
  user_hrpwm_tmr_cmp_cfg.cmp_a_val      = pwm3.compa;
  user_hrpwm_tmr_cmp_cfg.cmp_b_val      = pwm3.compb;
  user_hrpwm_tmr_cmp_cfg.cmp_c_val      = pwm3.compc;
  user_hrpwm_tmr_cmp_cfg.cmp_d_val      = pwm3.compd;
  user_hrpwm_tmr_cmp_cfg.pre_load_en    = true;
  user_hrpwm_tmr_cmp_cfg.mst_pwm_upd_en = true;
  //Slave PWMx Timer Compare Config
  LL_HRPWM_TmrCmpCfg(Instance, PFC_DRIVER_SLAVE_3, &user_hrpwm_tmr_cmp_cfg);

  //User HRPWM Timer Compare Config
  user_hrpwm_tmr_cmp_cfg.pre_load_en    = true;
  user_hrpwm_tmr_cmp_cfg.cmp_a_val      = pwm0.compa;
  user_hrpwm_tmr_cmp_cfg.cmp_b_val      = pwm0.compb;
  user_hrpwm_tmr_cmp_cfg.cmp_c_val      = pwm0.compc;
  user_hrpwm_tmr_cmp_cfg.cmp_d_val      = pwm0.compd;
  user_hrpwm_tmr_cmp_cfg.pre_load_en    = true;
  user_hrpwm_tmr_cmp_cfg.mst_pwm_upd_en = true;
  //Slave PWMx Timer Compare Config
  LL_HRPWM_TmrCmpCfg(Instance, PFC_PRE_SLAVE_0, &user_hrpwm_tmr_cmp_cfg);


  //User HRPWM Slave PWMx Output Config
  user_hrpwm_slv_output_cfg.Aout_set_evt_msk = HRPWM_SLV_OUT_CTRL_EVT_PWMx_ROLL_OVER;
  user_hrpwm_slv_output_cfg.Aout_clr_evt_msk = HRPWM_SLV_OUT_CTRL_EVT_PWMx_CMPB
																							|HRPWM_SLV_OUT_CTRL_EVT_PWMx_EVT0
																							|HRPWM_SLV_OUT_CTRL_EVT_PWMx_EVT1;
																							
  user_hrpwm_slv_output_cfg.Aout_pol         = HRPWM_SLV_OUT_POL_ACT_HITH;
  user_hrpwm_slv_output_cfg.Aout_idle_lvl    = HRPWM_SLV_OUT_IDLE_LVL_INVLD;
  user_hrpwm_slv_output_cfg.Aout_flt_lvl     = HRPWM_SLV_OUT_FAULT_LVL_INVLD;
  user_hrpwm_slv_output_cfg.Bout_set_evt_msk = HRPWM_SLV_OUT_CTRL_EVT_PWMx_CMPC;
  user_hrpwm_slv_output_cfg.Bout_clr_evt_msk = HRPWM_SLV_OUT_CTRL_EVT_PWMx_CMPD;
  user_hrpwm_slv_output_cfg.Bout_pol         = HRPWM_SLV_OUT_POL_ACT_HITH;
  user_hrpwm_slv_output_cfg.Bout_idle_lvl    = HRPWM_SLV_OUT_IDLE_LVL_INVLD;
  user_hrpwm_slv_output_cfg.Bout_flt_lvl     = HRPWM_SLV_OUT_FAULT_LVL_INVLD;
  user_hrpwm_slv_output_cfg.swap_en               = false;
  user_hrpwm_slv_output_cfg.dly_prot_en           = false;
  user_hrpwm_slv_output_cfg.bal_idle_auto_rcvr_en = false;
  user_hrpwm_slv_output_cfg.dly_prot_mode         = HRPWM_SLV_DLY_PROT_MECH_OUTA_EVTy_IDLE;
  LL_HRPWM_Slv_OutputCfg(Instance, PFC_DRIVER_SLAVE_3, &user_hrpwm_slv_output_cfg);

  user_hrpwm_slv_output_cfg.Aout_set_evt_msk = HRPWM_SLV_OUT_CTRL_EVT_PWMx_ROLL_OVER;
  user_hrpwm_slv_output_cfg.Aout_clr_evt_msk = HRPWM_SLV_OUT_CTRL_EVT_PWMx_CMPB;
  LL_HRPWM_Slv_OutputCfg(Instance, PFC_PRE_SLAVE_0, &user_hrpwm_slv_output_cfg);
	
	HRPWM_Slv_TmrEvtACfgTypeDef 	user_hrpwm_slv_eventcfg;
	HRPWM_Slv_TmrEvtFilCfgTypeDef user_hrpwm_slv_eventfilt;
	
	user_hrpwm_slv_eventcfg.enable = true;
	user_hrpwm_slv_eventcfg.rst_mode = HRPWM_SLV_EVTA_CNTR_RST_RSTRO_ALWAYS;
	user_hrpwm_slv_eventcfg.src = HRPWM_EXT_EVT_NUM_0;
	user_hrpwm_slv_eventcfg.thres = 0;
	
	user_hrpwm_slv_eventfilt.blk_latch_en = false;
	user_hrpwm_slv_eventfilt.fil = 0;
	
	LL_HRPWM_Slv_TmrEvtACfg(HRPWM,PFC_DRIVER_SLAVE_3,&user_hrpwm_slv_eventcfg,&user_hrpwm_slv_eventfilt);


	HRPWM_Comm_ExtEvtCfgTypeDef user_hrpwm_slv_extevtcfg;
	
	user_hrpwm_slv_extevtcfg.src = HRPWM_COMM_EXT_EVT0_INPUT_SRC_CMP1_OUT;
	user_hrpwm_slv_extevtcfg.fil_len  = 8;
	user_hrpwm_slv_extevtcfg.act_edge = HRPWM_COMM_EXT_EVT_INPUT_EDGE_ACT_LVL;
	user_hrpwm_slv_extevtcfg.fast_mode_en = true;
	user_hrpwm_slv_extevtcfg.samp_clk_div = 0; 
	user_hrpwm_slv_extevtcfg.pol = HRPWM_COMM_EXT_EVT_INPUT_POL_ACT_HIGH;
	
	LL_HRPWM_Comm_ExtEvtCfg(HRPWM,HRPWM_EXT_EVT_NUM_0,&user_hrpwm_slv_extevtcfg);// EVENT0 CMP1
	
	user_hrpwm_slv_extevtcfg.src = HRPWM_COMM_EXT_EVT1_INPUT_SRC_CMP3_OUT;
	LL_HRPWM_Comm_ExtEvtCfg(HRPWM,HRPWM_EXT_EVT_NUM_1,&user_hrpwm_slv_extevtcfg);//	EVENT1 CMP3
	


//  //User HRPWM Common Fault Config
//  user_hrpwm_comm_flt_cfg.sys_flt_int_en = true;
//  user_hrpwm_comm_flt_cfg.samp_clk_div   = HRPWM_COMM_FLT_SAMP_CLK_DIV_1;
//  user_hrpwm_comm_flt_cfg.input_en = true;
//  user_hrpwm_comm_flt_cfg.int_en   = false;
//  user_hrpwm_comm_flt_cfg.fil_len  = 4;
//  user_hrpwm_comm_flt_cfg.thres    = 0;
//  user_hrpwm_comm_flt_cfg.pol      = HRPWM_COMM_FLT_INPUT_POL_ACT_HIGH;
//  user_hrpwm_comm_flt_cfg.src      = HRPWM_COMM_FLT0_INPUT_SRC_CMP1_OUT;
//  user_hrpwm_comm_flt_cfg.rst_mode = HRPWM_COMM_FLT_CNTR_RST_RSTRO_ALWAYS;
//  user_hrpwm_comm_flt_cfg.blk_en   = false;
//  user_hrpwm_comm_flt_cfg.blk_src  = HRPWM_COMM_FLT_BLK_SRC_FIXED_WIN;
//  LL_HRPWM_Comm_FltCfg(HRPWM, HRPWM_FLT_NUM_0, &user_hrpwm_comm_flt_cfg);

//	__LL_HRPWM_Slv_Flt0_En(HRPWM, PFC_DRIVER_SLAVE_3);

  //Preload Enable, software need to generate a update event to update the shadow register to the working register
  //If Preload Disable or has other update source, don't need this action.
  LL_HRPWM_RegUpd_Frc(Instance, HRPWM_MST_PWM);
  LL_HRPWM_RegUpd_Frc(Instance, PFC_DRIVER_SLAVE_3);


  //User HRPWM Common ADC Trigger Config
  user_hrpwm_comm_adctrig_cfg.trig_evt    = HRPWM_COMM_ADC02_TRIG_EVT_PWM3_CMPC;
  user_hrpwm_comm_adctrig_cfg.trig_len    = HRPWM_COMM_ADC_TRIG_EVT_LEN_15CLK;
  user_hrpwm_comm_adctrig_cfg.upd_src     = HRPWM_COMM_ADC_TRIG_UPD_SRC_PWM3;
  user_hrpwm_comm_adctrig_cfg.post_scaler = 0;
  LL_HRPWM_Comm_ADCTrigCfg(Instance, HRPWM_ADC_TRIG_NUM_0, &user_hrpwm_comm_adctrig_cfg);


  //Preload Enable, software need to generate a update event to update the shadow register to the working register
  //If Preload Disable, don't need this action.
  __LL_HRPWM_Comm_MstPWMSwUpdReg_Set(Instance);

  HRPWM->PWM[3].REG.PWMCR0 |= (1 << 26);
  HRPWM->PWM[0].REG.PWMCR0 |= (1 << 26);	
  //HRPWM DLL Start
  LL_HRPWM_Comm_DLL_Start(Instance);

//    //HRPWM Master Timer Counter Start
//    LL_HRPWM_TmrCntr_Start(Instance, HRPWM_MST_PWM);

}


void hrpwm_init_app()
{
  LL_HRPWM_Init(HRPWM);
  hrpwm_data_init();
  user_hrpwm_config(HRPWM);
}

void hrpwm_start_count()
{
  LL_HRPWM_TmrCntr_Start(HRPWM, HRPWM_MST_PWM);
  LL_HRPWM_TmrCntr_Start(HRPWM, HRPWM_SLV_PWM_3);
  LL_HRPWM_TmrCntr_Start(HRPWM, HRPWM_SLV_PWM_0);	
}

void hrpwm_pre_outdis()
{
	pfc.pre_driver_cmpb = 100;
  HRPWM->Common.ODISR |= (1 << 0) | (1 << 1);
}

void hrpwm_outdis_app()
{
  HRPWM->Common.ODISR |= ((1 << 6) | (1 << 7));
}

void hrpwm_pre_outen()
{
	HRPWM->Common.OENR |= (1 << 0) | (1 << 1);
}

void hrpwm_outen_app()
{
  HRPWM->Common.OENR |= (1 << 6) | (1 << 7);
}
RAMCODE
void hrpwm_update_app()
{
  HRPWM->Common.CR0 |=	(1 << 0) |
												(1 << 1) |
                        (1 << 4); //pwm3
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();

//  pfc.duty = (pfc.vloop.loop_out * PFC_DRIVER_PERIOD_VALUE / 1200  );		//PID计算结果调整输出占空比
  pfc.duty = (uint32_t)(pfc.current_output * PFC_DRIVER_PERIOD_VALUE);		//PID计算结果调整输出占空比

  if(pfc.duty > PFC_MAX_DUTY)
    {
      pfc.duty = PFC_MAX_DUTY;
			pfc_current_integral_add_en = false;
    }
  else if(pfc.duty < PFC_MIN_DUTY)
    {
      pfc.duty = PFC_MIN_DUTY;
    }
	else
		{
			pfc_current_integral_add_en = true;
		}
  HRPWM->PWM[0].REG.CMPBR = pfc.pre_driver_cmpb ;
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  HRPWM->PWM[3].REG.CMPBR = pfc.duty ;
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
		
  HRPWM->PWM[3].REG.CMPCR = pfc.duty >> 1 ;
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();


  HRPWM->Common.CR0 &= 	~((1 << 0) |
													(1 << 1) |
                          (1 << 4));
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
}

