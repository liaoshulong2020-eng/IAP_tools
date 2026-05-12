#include "main.h"


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
    user_cmp_init.posi_in_src = CMP_POSI_IN_SRC_CMP_INP0;//PA7 OVP
    LL_CMP_Init(CMP1, &user_cmp_init);
	
	
		user_cmp_init.neg_in_src = CMP_NEG_IN_SRC_DACy_OUT;  //DAC0_OUT
	  user_cmp_init.posi_in_src = CMP_POSI_IN_SRC_CMP_INP0;//PB0 OCP
    LL_CMP_Init(CMP3, &user_cmp_init);

	    //CMP Start
    LL_CMP_Start(CMP1);
		LL_CMP_Start(CMP3);
}