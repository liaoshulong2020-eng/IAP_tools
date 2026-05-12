#include "main.h"
#include "iir_app.h"

void iir_init_app()
{
    LL_IIR_Init(IIR0);

    IIR_UserCfgTypeDef iir_user_cfg;

    memset((void*)&iir_user_cfg, 0x00, sizeof(iir_user_cfg));

    //IIR Config
    iir_user_cfg.order = IIR_ORDER_2;
    iir_user_cfg.out_scale = IIR_SCALE_2_POWER_12;
    iir_user_cfg.fb_scale  = IIR_SCALE_2_POWER_13;
    iir_user_cfg.coef_grp_nums = IIR_COEF_GRP_NUM_1;
	
    iir_user_cfg.coef[0].Ax[0] = (int32_t) -7912;
    iir_user_cfg.coef[0].Ax[1] = (int32_t)  3817;
    iir_user_cfg.coef[0].Ax[2] = (int32_t)  0;
    iir_user_cfg.coef[0].Ax[3] = (int32_t)  0;
    
		iir_user_cfg.coef[0].Bx[0] = (int32_t)   7913<<12;
    iir_user_cfg.coef[0].Bx[1] = (int32_t) -(15825<<12);
    iir_user_cfg.coef[0].Bx[2] = (int32_t)   7913<<12;
    iir_user_cfg.coef[0].Bx[3] = (int32_t)   0;
    iir_user_cfg.coef[0].Bx[4] = (int32_t)   0;
		
    iir_user_cfg.bdry_Lx[0] = 0;
    iir_user_cfg.bdry_Lx[1] = 0;
    iir_user_cfg.bdry_Lx[2] = 0;
    iir_user_cfg.bdry_Lx[3] = 0;

    iir_user_cfg.lmt_cfg.enable = true;
    iir_user_cfg.lmt_cfg.min = -33554432;//Q25
    iir_user_cfg.lmt_cfg.max = 33554432;
		
//    iir_user_cfg.lmt_cfg.enable = true;
//    iir_user_cfg.lmt_cfg.min = ((int64_t)1<<24)-1;//Q25
//    iir_user_cfg.lmt_cfg.max = -((int64_t)1<<24);
    LL_IIR_Config(IIR0, &iir_user_cfg);



    LL_IIR_Init(IIR1);
    memset((void*)&iir_user_cfg, 0x00, sizeof(iir_user_cfg));

    //IIR Config
    iir_user_cfg.order = IIR_ORDER_2;
    iir_user_cfg.out_scale = IIR_SCALE_2_POWER_16;
    iir_user_cfg.fb_scale  = IIR_SCALE_2_POWER_14;
    iir_user_cfg.coef_grp_nums = IIR_COEF_GRP_NUM_1;
		
    iir_user_cfg.coef[0].Ax[0] = (int32_t) -32443;
    iir_user_cfg.coef[0].Ax[1] = (int32_t)  16124;
    iir_user_cfg.coef[0].Ax[2] = (int32_t)  0;
    iir_user_cfg.coef[0].Ax[3] = (int32_t)  0;
		
    iir_user_cfg.coef[0].Bx[0] = (int32_t)   16254<<16;
    iir_user_cfg.coef[0].Bx[1] = (int32_t) -(32443<<16);
    iir_user_cfg.coef[0].Bx[2] = (int32_t)   16254<<16;
    iir_user_cfg.coef[0].Bx[3] = (int32_t)   0;
    iir_user_cfg.coef[0].Bx[4] = (int32_t)   0;
	


    iir_user_cfg.bdry_Lx[0] = 0;
    iir_user_cfg.bdry_Lx[1] = 0;
    iir_user_cfg.bdry_Lx[2] = 0;
    iir_user_cfg.bdry_Lx[3] = 0;

    iir_user_cfg.lmt_cfg.enable = true;
    iir_user_cfg.lmt_cfg.min = -33554432;//Q25
    iir_user_cfg.lmt_cfg.max = 33554432;
		
//    iir_user_cfg.lmt_cfg.enable = true;
//    iir_user_cfg.lmt_cfg.min = ((int64_t)1<<24)-1;//Q25
//    iir_user_cfg.lmt_cfg.max = -((int64_t)1<<24);
    LL_IIR_Config(IIR1, &iir_user_cfg);

}






