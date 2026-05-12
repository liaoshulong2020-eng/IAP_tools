#include "main.h"
#include "dac_app.h"
void dac_init_app()
{
		 DAC_InitTypeDef user_dac_init;

    //VREFBUF Enable
    __LL_SYSCTRL_SpRegWrite_Unlock(SYSCTRL);
    __LL_SYSCTRL_VREFBUF_En(SYSCTRL);
    __LL_SYSCTRL_SpRegWrite_Lock(SYSCTRL);

    //User DAC Init
    memset((void *)&user_dac_init, 0x00, sizeof(user_dac_init));
    user_dac_init.trig_en = false;
    user_dac_init.buf_out_en = false;
    user_dac_init.bypass_buf_out_en = false;
	
    LL_DAC_Init(DAC1, &user_dac_init);
		LL_DAC_Init(DAC0, &user_dac_init);
	
		LL_DAC_ValueSet(DAC1,VOUT_OVER_VOLTAGE_DAC_VALUE);		//OVP
	
		LL_DAC_ValueSet(DAC0,IOUT_OCP_CURRENT_DAC_DAC_VALUE);	//OCP IOUT_OCP_CURRENT_DAC_DAC_VALUE
	
	  LL_DAC_Start(DAC1);
		LL_DAC_Start(DAC0);
}

void cmp_init_app()
{
    CMP_InitTypeDef user_cmp_init;

    memset((void *)&user_cmp_init, 0x00, sizeof(user_cmp_init));

    //User CMP Init
    user_cmp_init.rising_int_en  = true;
    user_cmp_init.falling_int_en = true;
    user_cmp_init.output_invert_en = false;
    user_cmp_init.output_dbc = 0;

    user_cmp_init.hyst = CMP_HYST_0mv;
    user_cmp_init.blk_evt = CMP_BLK_EVT_NONE;
    user_cmp_init.input_src = CMP_INPUT_SRC_ANOLOG;
    user_cmp_init.neg_in_src = CMP_NEG_IN_SRC_DACz_OUT;  //DAC1_OUT
    user_cmp_init.posi_in_src = CMP_POSI_IN_SRC_CMP_INP1;//PA3 OVP
    LL_CMP_Init(OVP_CMP, &user_cmp_init);
	
	  user_cmp_init.output_dbc = 100;
		user_cmp_init.neg_in_src = CMP_NEG_IN_SRC_DACz_OUT;  //DAC0_OUT
		user_cmp_init.posi_in_src = CMP_POSI_IN_SRC_CMP_INP0;
		LL_CMP_Init(OCP_CMP, &user_cmp_init);
	    //CMP Start
   
		LL_CMP_Start(OCP_CMP);
}