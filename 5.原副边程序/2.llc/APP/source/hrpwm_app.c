#include "main.h"
#include "hrpwm_app.h"

#define LLC_DRIVER_SLAVE_2	HRPWM_SLV_PWM_2
#define SR_DRIVER_SLAVE_3		HRPWM_SLV_PWM_3

void hrpwm_data_init(void)		//给PWM一个初始值
{
  /*mpwm init*/
  mpwm.period  = LLC_SW_PERIOD_MIN;         //200k
  mpwm.compa   = LLC_SW_PERIOD_MIN >> 1;
  mpwm.compb   = LLC_SW_PERIOD_MIN >> 3;
  mpwm.compc   = LLC_SW_PERIOD_MIN >> 2;
  mpwm.compd   = LLC_SW_PERIOD_MIN >> 1;

  /*pwm0	均流母线电压调节信号*/
  pwm0.period  = LLC_SW_PERIOD_MIN;       //200k
  pwm0.compa   = LLC_SW_PERIOD_MIN >> 2;
  pwm0.compb   = LLC_SW_PERIOD_MIN >> 2;
  pwm0.compc   = LLC_SW_PERIOD_MIN >> 2;
  pwm0.compd   = LLC_SW_PERIOD_MIN >> 2;

  /*pwm2	LLC驱动信号*/
  pwm2.period  = LLC_SW_PERIOD_MIN;       //200k
  pwm2.compa   = 50;  //0.47 duty
  pwm2.compb   = (pwm2.period >> 1) - LLC_DEADTIME_COUNT;
  pwm2.compc   = (pwm2.period >> 1) + 50;
  pwm2.compd   =  pwm2.period - LLC_DEADTIME_COUNT;

  /*pwm3  LLC副边同步整流驱动信号 */		//
  pwm3.period  = LLC_SW_PERIOD_MIN;        	//200k
  pwm3.compa   = 600;  //
  pwm3.compb   = (pwm3.period >> 1) - LLC_DEADTIME_COUNT;
  pwm3.compc   = (pwm3.period >> 1) + 600;
  pwm3.compd   =  pwm3.period - LLC_DEADTIME_COUNT;


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
  LL_HRPWM_TmrBaseCfg(Instance, LLC_DRIVER_SLAVE_2, &user_hrpwm_tmr_base_cfg);
  LL_HRPWM_TmrBaseCfg(Instance, SR_DRIVER_SLAVE_3, &user_hrpwm_tmr_base_cfg);

  HRPWM_Mst_SyncCfgTypeDef HRPWM_Mst_SyncCfgStruct;
  HRPWM_Mst_SyncCfgStruct.in_src = HRPWM_MST_SYNC_IN_SRC_TMR9_TRGO;
  HRPWM_Mst_SyncCfgStruct.mode = HRPWM_MST_SYNC_MODE_SLV;
//HRPWM_Mst_SyncCfgStruct.out_pol =
//HRPWM_Mst_SyncCfgStruct.out_src =
#if 0
  LL_HRPWM_Mst_SyncCfg(HRPWM, &HRPWM_Mst_SyncCfgStruct);
#endif

  //User HRPWM Timer Compare Config
  user_hrpwm_tmr_cmp_cfg.pre_load_en    = true;
  user_hrpwm_tmr_cmp_cfg.cmp_a_val      = 1000;//mpwm.compa;
  user_hrpwm_tmr_cmp_cfg.cmp_b_val      = mpwm.compb;
  user_hrpwm_tmr_cmp_cfg.cmp_c_val      = mpwm.compc;
  user_hrpwm_tmr_cmp_cfg.cmp_d_val      = mpwm.compd;
  //Master PWM Timer Compare Config
  LL_HRPWM_TmrCmpCfg(Instance, HRPWM_MST_PWM, &user_hrpwm_tmr_cmp_cfg);

  //User HRPWM Timer Compare Config
  user_hrpwm_tmr_cmp_cfg.pre_load_en    = true;
  user_hrpwm_tmr_cmp_cfg.cmp_a_val      = pwm2.compa;
  user_hrpwm_tmr_cmp_cfg.cmp_b_val      = pwm2.compb;
  user_hrpwm_tmr_cmp_cfg.cmp_c_val      = pwm2.compc;
  user_hrpwm_tmr_cmp_cfg.cmp_d_val      = pwm2.compd;
  user_hrpwm_tmr_cmp_cfg.pre_load_en    = true;
  user_hrpwm_tmr_cmp_cfg.mst_pwm_upd_en = true;
  //Slave PWMx Timer Compare Config
  LL_HRPWM_TmrCmpCfg(Instance, LLC_DRIVER_SLAVE_2, &user_hrpwm_tmr_cmp_cfg);

  //User HRPWM Timer Compare Config
  user_hrpwm_tmr_cmp_cfg.cmp_a_val      = pwm3.compa;
  user_hrpwm_tmr_cmp_cfg.cmp_b_val      = pwm3.compb;
  user_hrpwm_tmr_cmp_cfg.cmp_c_val      = pwm3.compc;
  user_hrpwm_tmr_cmp_cfg.cmp_d_val      = pwm3.compd;
  //Slave PWMx Timer Compare Config
  LL_HRPWM_TmrCmpCfg(Instance, SR_DRIVER_SLAVE_3, &user_hrpwm_tmr_cmp_cfg);


  //User HRPWM Slave PWMx Output Config
  user_hrpwm_slv_output_cfg.Aout_set_evt_msk = HRPWM_SLV_OUT_CTRL_EVT_PWMx_CMPA;
  user_hrpwm_slv_output_cfg.Aout_clr_evt_msk = HRPWM_SLV_OUT_CTRL_EVT_PWMx_CMPB
																							|HRPWM_SLV_OUT_CTRL_EVT_PWMx_EVT0;
  user_hrpwm_slv_output_cfg.Aout_pol         = HRPWM_SLV_OUT_POL_ACT_HITH;
  user_hrpwm_slv_output_cfg.Aout_idle_lvl    = HRPWM_SLV_OUT_IDLE_LVL_INVLD;
  user_hrpwm_slv_output_cfg.Aout_flt_lvl     = HRPWM_SLV_OUT_FAULT_LVL_INVLD;
  user_hrpwm_slv_output_cfg.Bout_set_evt_msk = HRPWM_SLV_OUT_CTRL_EVT_PWMx_CMPC;
  user_hrpwm_slv_output_cfg.Bout_clr_evt_msk = HRPWM_SLV_OUT_CTRL_EVT_PWMx_CMPD | HRPWM_SLV_OUT_CTRL_EVT_PWMx_UPDATE|HRPWM_SLV_OUT_CTRL_EVT_PWMx_EVT0;; //|HRPWM_SLV_OUT_CTRL_EVT_PWMx_ROLL_OVER;//|HRPWM_SLV_OUT_CTRL_EVT_PWMx_UPDATE;
  user_hrpwm_slv_output_cfg.Bout_pol         = HRPWM_SLV_OUT_POL_ACT_HITH;
  user_hrpwm_slv_output_cfg.Bout_idle_lvl    = HRPWM_SLV_OUT_IDLE_LVL_INVLD;
  user_hrpwm_slv_output_cfg.Bout_flt_lvl     = HRPWM_SLV_OUT_FAULT_LVL_INVLD;
  user_hrpwm_slv_output_cfg.swap_en               = false;
  user_hrpwm_slv_output_cfg.dly_prot_en           = false;
  user_hrpwm_slv_output_cfg.bal_idle_auto_rcvr_en = false;
  user_hrpwm_slv_output_cfg.dly_prot_mode         = HRPWM_SLV_DLY_PROT_MECH_OUTA_EVTy_IDLE;
  LL_HRPWM_Slv_OutputCfg(Instance, SR_DRIVER_SLAVE_3, &user_hrpwm_slv_output_cfg);

//	user_hrpwm_slv_output_cfg.Aout_set_evt_msk = HRPWM_SLV_OUT_CTRL_EVT_MST_PWM_PERIOD;
//  user_hrpwm_slv_output_cfg.Aout_clr_evt_msk = HRPWM_SLV_OUT_CTRL_EVT_MST_PWM_CMPA;
  LL_HRPWM_Slv_OutputCfg(Instance, LLC_DRIVER_SLAVE_2, &user_hrpwm_slv_output_cfg);


  HRPWM_Slv_TmrEvtACfgTypeDef 	user_hrpwm_slv_eventcfg;
  HRPWM_Slv_TmrEvtFilCfgTypeDef user_hrpwm_slv_eventfilt;

  user_hrpwm_slv_eventcfg.enable = true;
  user_hrpwm_slv_eventcfg.rst_mode = HRPWM_SLV_EVTA_CNTR_RST_RSTRO_ALWAYS;
  user_hrpwm_slv_eventcfg.src = HRPWM_EXT_EVT_NUM_0;
  user_hrpwm_slv_eventcfg.thres = 0;

  user_hrpwm_slv_eventfilt.blk_latch_en = false;
  user_hrpwm_slv_eventfilt.fil = 0;

#if 0
  LL_HRPWM_Slv_TmrEvtACfg(HRPWM, LLC_DRIVER_SLAVE_2, &user_hrpwm_slv_eventcfg, &user_hrpwm_slv_eventfilt);
  LL_HRPWM_Slv_TmrEvtACfg(HRPWM, SR_DRIVER_SLAVE_3, &user_hrpwm_slv_eventcfg, &user_hrpwm_slv_eventfilt);
#endif


  HRPWM_Comm_ExtEvtCfgTypeDef user_hrpwm_slv_extevtcfg;

	user_hrpwm_slv_extevtcfg.src = HRPWM_COMM_EXT_EVT0_INPUT_SRC_CMP1_OUT;
	user_hrpwm_slv_extevtcfg.fil_len  = 0;
	user_hrpwm_slv_extevtcfg.act_edge = HRPWM_COMM_EXT_EVT_INPUT_EDGE_ACT_LVL;
	user_hrpwm_slv_extevtcfg.fast_mode_en = true;
	user_hrpwm_slv_extevtcfg.samp_clk_div = 0;
	user_hrpwm_slv_extevtcfg.pol = HRPWM_COMM_EXT_EVT_INPUT_POL_ACT_HIGH;
//
	LL_HRPWM_Comm_ExtEvtCfg(HRPWM,HRPWM_EXT_EVT_NUM_0,&user_hrpwm_slv_extevtcfg);


  //User HRPWM Common Fault Config
  user_hrpwm_comm_flt_cfg.sys_flt_int_en = true;
  user_hrpwm_comm_flt_cfg.samp_clk_div   = HRPWM_COMM_FLT_SAMP_CLK_DIV_1;
  user_hrpwm_comm_flt_cfg.input_en = true;
  user_hrpwm_comm_flt_cfg.int_en   = true;	//IRQ
  user_hrpwm_comm_flt_cfg.fil_len  = 0;
  user_hrpwm_comm_flt_cfg.thres    = 0;
  user_hrpwm_comm_flt_cfg.pol      = HRPWM_COMM_FLT_INPUT_POL_ACT_HIGH;
//  user_hrpwm_comm_flt_cfg.src      = HRPWM_COMM_FLT0_INPUT_SRC_CMP1_OUT;
  user_hrpwm_comm_flt_cfg.rst_mode = HRPWM_COMM_FLT_CNTR_RST_RSTRO_ALWAYS;
  user_hrpwm_comm_flt_cfg.blk_en   = false;
  user_hrpwm_comm_flt_cfg.blk_src  = HRPWM_COMM_FLT_BLK_SRC_FIXED_WIN;

//  LL_HRPWM_Comm_FltCfg(HRPWM, HRPWM_FLT_NUM_0, &user_hrpwm_comm_flt_cfg);

  user_hrpwm_comm_flt_cfg.fil_len  = 5;
  user_hrpwm_comm_flt_cfg.src      = HRPWM_COMM_FLT4_INPUT_SRC_CMP2_OUT;
  LL_HRPWM_Comm_FltCfg(HRPWM, HRPWM_FLT_NUM_4, &user_hrpwm_comm_flt_cfg);

//  __LL_HRPWM_Slv_Flt0_En(HRPWM, LLC_DRIVER_SLAVE_2);
//  __LL_HRPWM_Slv_Flt0_En(HRPWM, SR_DRIVER_SLAVE_3);

	__LL_HRPWM_Slv_Flt4_En(HRPWM, LLC_DRIVER_SLAVE_2);
	__LL_HRPWM_Slv_Flt4_En(HRPWM, SR_DRIVER_SLAVE_3);

  //Preload Enable, software need to generate a update event to update the shadow register to the working register
  //If Preload Disable or has other update source, don't need this action.
  LL_HRPWM_RegUpd_Frc(Instance, HRPWM_MST_PWM);
  LL_HRPWM_RegUpd_Frc(Instance, LLC_DRIVER_SLAVE_2);
  LL_HRPWM_RegUpd_Frc(Instance, SR_DRIVER_SLAVE_3);


  //User HRPWM Common ADC Trigger Config
  user_hrpwm_comm_adctrig_cfg.trig_evt    = HRPWM_COMM_ADC02_TRIG_EVT_MST_PWM_CMPA;
  user_hrpwm_comm_adctrig_cfg.trig_len    = HRPWM_COMM_ADC_TRIG_EVT_LEN_15CLK;
  user_hrpwm_comm_adctrig_cfg.upd_src     = HRPWM_COMM_ADC_TRIG_UPD_SRC_MST_PWM;
  user_hrpwm_comm_adctrig_cfg.post_scaler = 0;
  LL_HRPWM_Comm_ADCTrigCfg(Instance, HRPWM_ADC_TRIG_NUM_0, &user_hrpwm_comm_adctrig_cfg);


  //Preload Enable, software need to generate a update event to update the shadow register to the working register
  //If Preload Disable, don't need this action.
  __LL_HRPWM_Comm_MstPWMSwUpdReg_Set(Instance);
//  HRPWM->PWM[2].REG.PWMCR0 |= (1 << 26);
//  HRPWM->PWM[3].REG.PWMCR0 |= (1 << 26);
  HRPWM->Master.MCR0 |= (1 << 26);
  //HRPWM DLL Start
  LL_HRPWM_Comm_DLL_Start(Instance);

//    //HRPWM Master Timer Counter Start
//    LL_HRPWM_TmrCntr_Start(Instance, HRPWM_MST_PWM);

}


void hrpwm_init_app(void)
{
  LL_HRPWM_Init(HRPWM);
  hrpwm_data_init();
  user_hrpwm_config(HRPWM);
}

void hrpwm_start_count(void)
{
//	LL_HRPWM_TmrCntr_Start(HRPWM, HRPWM_MST_PWM);
//	LL_HRPWM_TmrCntr_Start(HRPWM, HRPWM_SLV_PWM_2);
//  LL_HRPWM_TmrCntr_Start(HRPWM, HRPWM_SLV_PWM_3);
  HRPWM->Master.MCR1 |= ((HRPWM_MST_MCR1_MCEN_Msk) | (HRPWM_MST_MCR1_CEN2_Msk) | (HRPWM_MST_MCR1_CEN3_Msk));
}

void hrpwm_sync_outdis(void)
{


}

RAMCODE
void hrpwm_outdis_app(void)
{
  HRPWM->Common.ODISR = ~0;
  llc.llc_drv_state		= 0	;
  llc.llc_sr_state		= 0	;
}

RAMCODE
void hrpwm_outen_app(void)
{
  HRPWM->Common.OENR	|= (1 << 4) | (1 << 5);
  llc.llc_drv_state		= 1	;
}

RAMCODE
void hrpwm_sren_app(void)
{
  HRPWM->Common.OENR |= (1 << 6) |		//PWM3 A通道 使能输出
                        (1 << 7) ;		//PWM3 B通道 使能输出
  llc.llc_sr_state		= 1	;
}

RAMCODE
void hrpwm_srdis_app(void)
{
  HRPWM->Common.ODISR |= (1 << 6) |	//PWM3 A通道 禁止输出
                         (1 << 7) ;	//PWM3 B通道 禁止输出
  llc.llc_sr_state		= 0	;
}

RAMCODE
void hrpwm_update_app()
{
//		llc.duty = (llc.period >>1) - 400;
	
#if 1	
		pwm2.period = llc.period;
    pwm2.compa  = 50;
    pwm2.compb  = llc.duty;
    pwm2.compc  = (llc.period >>1) + 50;
    pwm2.compd  = (llc.period >>1) + llc.duty;
	  
	  
		if(llc.period > LLC_SW_PERIOD_FR)		//183k	16000
		{
					pwm3.period = llc.period;
					pwm3.compa  = LLC_DRIVER_DEADTIME_COUNT;
					pwm3.compb  = (LLC_SW_PERIOD_FR /2) - LLC_DEADTIME_COUNT-50;
					if(pwm3.compb > llc.duty) pwm3.compb = llc.duty;
			
			    pwm3.compc  = (llc.period /2) + LLC_DRIVER_DEADTIME_COUNT;
									
//			  if(llc.period > SR_CMPSA_FREQ_VALUE ) //150k
//				{
//				
//					pwm3.compb  = (LLC_SW_PERIOD_FR >>1) - LLC_DEADTIME_COUNT + SR_CMPSA_VALUE;
//					pwm3.compd  = LLC_SW_PERIOD_FR - LLC_DEADTIME_COUNT + SR_CMPSA_VALUE;
//				}
//				else
//				{
					pwm3.compd =  pwm3.compc  + ( pwm3.compb - pwm3.compa );
			
					if(pwm3.compd  > (llc.period - LLC_DEADTIME_COUNT-50))
					{
					  pwm3.compd  = (llc.period - LLC_DEADTIME_COUNT-50);
					
					}
//				}
		}
		else
		{
					pwm3.period = llc.period;
					pwm3.compa  = LLC_DRIVER_DEADTIME_COUNT;
					pwm3.compb  = (llc.period / 2) - LLC_DEADTIME_COUNT-50;
					if(pwm3.compb > pwm2.compb) pwm3.compb = pwm2.compb;
					if(pwm3.compb < pwm3.compa + 100) pwm3.compb = pwm3.compa + 100;  
			
			
					pwm3.compc  = (llc.period / 2) + LLC_DRIVER_DEADTIME_COUNT;
					pwm3.compd  = llc.period - LLC_DEADTIME_COUNT-50;
					if(pwm3.compd > pwm2.compd) pwm3.compd = pwm2.compd;
					if(pwm3.compd < pwm3.compc + 100) pwm3.compd = pwm3.compc + 100;
		}	
		#endif
		
//		pwm2.period = llc.period;
//    pwm2.compa  = 50;
//    pwm2.compb  = llc.duty;
//    pwm2.compc  = (llc.period >>1) + 50;
//    pwm2.compd  = (llc.period >>1) + llc.duty;
//
//
//		if(llc.period > LLC_SW_PERIOD_FR)		//188k	16000
//		{
//					pwm3.period = llc.period;
//					pwm3.compa  = 600;
//					pwm3.compb  = (LLC_SW_PERIOD_FR /2) - LLC_DEADTIME_COUNT;
//			#if 1
//			    pwm3.compc  = (llc.period /2)+600;
//			#endif

//					pwm3.compd =  pwm3.compc  + ( pwm3.compb - pwm3.compa );
//
////					if(pwm3.compd  > (llc.period - LLC_DEADTIME_COUNT))
////					{
////					  pwm3.compd  = (llc.period - LLC_DEADTIME_COUNT);
////
////					}
//
//		}
//		else
//		{
//					pwm3.period = llc.period;
//					pwm3.compa  = 600;
//					pwm3.compb  = (llc.period / 2) - LLC_DEADTIME_COUNT;
//					pwm3.compc  = (llc.period / 2) + 600;
//					pwm3.compd  = llc.period - LLC_DEADTIME_COUNT;
//		}


  HRPWM->Common.CR0 |=	(1 << 0) |	//Mpwm
                        (1 << 2) | 	//pwm3
                        (1 << 3); 	//pwm4
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();

  HRPWM->Master.MPER  = pwm2.period;
//	HRPWM->Master.MCMPAR = pwm2.compa;
  HRPWM->Master.MCMPBR = pwm2.compb;
  HRPWM->Master.MCMPCR = pwm2.compc;
  HRPWM->Master.MCMPDR = pwm2.compd;

  HRPWM->PWM[2].REG.PERR  = pwm2.period;
  HRPWM->PWM[2].REG.CMPAR = pwm2.compa;
  HRPWM->PWM[2].REG.CMPBR = pwm2.compb;
  HRPWM->PWM[2].REG.CMPCR = pwm2.compc;
  HRPWM->PWM[2].REG.CMPDR = pwm2.compd;

  HRPWM->PWM[3].REG.PERR  = pwm3.period;
  HRPWM->PWM[3].REG.CMPAR = pwm3.compa;
  HRPWM->PWM[3].REG.CMPBR = pwm3.compb;
  HRPWM->PWM[3].REG.CMPCR = pwm3.compc;
  HRPWM->PWM[3].REG.CMPDR = pwm3.compd;


  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();


  HRPWM->Common.CR0 &= 	~((1 << 0) |
                          (1 << 2) |
                          (1 << 3));
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();


// if(llc.switch_mode == burst_mode || llc.mode_hd)


  if((llc.sr_pwm) && (llc.switch_mode == pfm_mode || llc.switch_mode == max_pfm_mode || llc.switch_mode == pwm_mode))
    {

      hrpwm_sren_app();
    }
  else
    {
      hrpwm_srdis_app();
    }





//	    if(llc.u16BrustCnt > 0)
//		{
//			 llc.u16BrustCnt --;
//			 hrpwm_outen_app();

//			 if((llc.SrPwm_status == 1)&&(llc.switch_mode == pfm_mode))		//输出电流大于10A 且为 变频模式
//			 {
//			   hrpwm_sren_app();																					//打开SR

//			 }
//			 else
//			 {
//				hrpwm_srdis_app();																					//关闭SR

//			 }
//		}
//    else
//		{
//			 llc.u16BrustCnt = 0;
//			 llc.duty = 0;
//			 hrpwm_outdis_app();
//			 hrpwm_srdis_app();
//			llc.test_tx |=(1<<4);
//		}

}
RAMCODE
void LL_HRPWM_OVP_IRQHandler(HRPWM_TypeDef* Instance)
{
  hrpwm_srdis_app();
  hrpwm_outdis_app();
  llc.state = state_fault;
	llc.fault_state.bit.over_voltage = 1;
  llc.led_cnt++;
  if(llc.led_cnt > 1000)
    {
      LL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
      llc.led_cnt = 0;
    }
}

RAMCODE
void LL_HRPWM_OCP_IRQHandler(HRPWM_TypeDef* Instance)
{
    
  //  if(llc.loop_out < LLC_SW_PERIOD_MIN)
    {
      hrpwm_srdis_app();
      hrpwm_outdis_app();
			llc.state = state_fault;
			llc.fault_state.bit.over_current = 1;
//			LL_CMP_Stop(OCP_CMP);
      llc.HwOcp  = 1;
      llc.short_current_flag ++;
      llc.current_loop_latch = 0;
      //Clear Interrupt Pending
      __LL_HRPWM_Comm_Flt4IntPnd_Clr(Instance);
    }



}


RAMCODE
void HRPWM_COMM_IRQHandler(void)
{
  uint32_t int_en, int_pending;
  int_en = __LL_HRPWM_Comm_AllIntEn_Get(HRPWM);

  int_pending = __LL_HRPWM_Comm_AllIntPnd_Get(HRPWM);

		//Fault 4 Interrupt Handler
    if ((int_en & HRPWM_COM_IER_FLT4IE_Msk) && (int_pending & HRPWM_COM_ISR_FLT4_Msk)) 
			{
				{
					hrpwm_srdis_app();
					hrpwm_outdis_app();
					llc.state = state_fault;
					
					llc.fault_state.bit.over_current = 1;
					llc.HwOcp  = 1;
					llc.short_current_flag ++;
					llc.current_loop_latch = 0;
//					LL_CMP_Stop(OCP_CMP);
					__LL_HRPWM_Comm_Flt4IntPnd_Clr(HRPWM);
				}
			}
}


