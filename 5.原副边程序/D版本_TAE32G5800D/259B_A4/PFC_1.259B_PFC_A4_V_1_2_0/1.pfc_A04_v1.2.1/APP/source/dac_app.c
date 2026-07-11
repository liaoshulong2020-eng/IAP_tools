#include "main.h"

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
    LL_DAC_Init(DAC0, &user_dac_init);
		LL_DAC_Init(DAC4, &user_dac_init);
	
		LL_DAC_ValueSet(DAC0, BUS_OVP_DAC_VALUE);
		LL_DAC_ValueSet(DAC3, PFC_I_OCP_DAC_VALUE);
	
	  LL_DAC_Start(DAC0);
		LL_DAC_Start(DAC3);

}

