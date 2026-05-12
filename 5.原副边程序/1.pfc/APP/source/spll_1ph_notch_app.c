/**
  ******************************************************************************
  * @file    APP/spll_1ph_sogi_app.c
  * @author  004 
  * @version V1.0.0
  * @date    17-04-2020
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 Tai-Micro</center></h2>
  *
  *
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "spll_1ph_notch_app.h"
#include "variables_define_app.h"
RAMCODE
void SPLL_1PH_NOTCH_RUN(SPLL_1PH_NOTCH *spll_obj, int32_t acValue)
{			
	// 鉴相
	spll_iir_indata = acValue * spll_obj->cosine >> 10;		//the max of spll_indata is 2608
	
	//陷波滤波器
	LL_IIR_Calc_Multi(IIR0, &spll_iir_indata, 1, IIR_COEF_GRP_AUTO, (int16_t*)&spll_obj->y_notch1,300);
	
	spll_obj->spll_erro_sum	= spll_obj->spll_erro_sum + spll_obj->y_notch1;		//error累加
	spll_obj->dwn		= (spll_obj->y_notch1*400 + spll_obj->spll_erro_sum)>>1;
	spll_obj->wn = (spll_obj->dwn + 3277308)/1000;	//放大1000倍	//819327=2*3.14159*50*2608 与spll_indata的最大量化有关
	spll_obj->theta		= (spll_obj->theta*1000 + spll_obj->wn)/1000;
	if(spll_obj->theta < 0){
		spll_obj->theta = spll_obj->theta + 65504;
	} else if(spll_obj->theta > 65504){							//16386=2*3.1415926*2608 与spll_indata的最大量化有关
		spll_obj->theta = spll_obj->theta - 65504;
	}
	spll_obj->cosine_prev	= spll_obj->cosine;
	spll_obj->sine_prev		= spll_obj->sine;
	spll_obj->cosine		= cosdata[spll_obj->theta>>5];
	spll_obj->sine			= sindata[spll_obj->theta>>5];
}

//int32_t theta_index;
//extern int32_t theta_index;
//#define PI 3.14159
//// 鉴相系数
//#define SPLL_INDATA_MAX 2608
//#define SPLL_COSINE_SHIFT 12

//// 滤波器相关参数
//#define SPLL_NOTCH_FILTER_COEFF_GROUP IIR_COEF_GRP_AUTO
//#define SPLL_NOTCH_FILTER_LENGTH 300

//// PLL 参数
//#define SPLL_ERROR_SUM_GAIN 400
//#define SPLL_WN_OFFSET (2 * PI * SPLL_INDATA_MAX * 50 * 4)
//#define SPLL_WN_SCALE 10
//#define SPLL_THETA_MAX (2 * PI * SPLL_INDATA_MAX * 4)
//#define THETA_AMP	10000

//uint32_t spll_indata_max = SPLL_INDATA_MAX;
//uint32_t spll_cosine_shift = SPLL_COSINE_SHIFT;
//uint32_t spll_error_sum_gain = SPLL_ERROR_SUM_GAIN;
//uint32_t spll_wn_offset = SPLL_WN_OFFSET;
//uint32_t spll_wn_scale = SPLL_WN_SCALE;
//uint32_t spll_theta_max = SPLL_THETA_MAX;
//uint32_t theta_amp = THETA_AMP;
//RAMCODE
//void SPLL_1PH_NOTCH_RUN(SPLL_1PH_NOTCH *spll_obj, int32_t acValue)
//{
//		spll_theta_max = (2 * PI * spll_indata_max * 4);
//		spll_wn_offset = (2 * PI * spll_indata_max * 50 * 4);
//    // 鉴相
//    spll_iir_indata = (int32_t)(acValue * spll_obj->cosine) >> spll_cosine_shift;
//		//计算得到y_notch
//    LL_IIR_Calc_Multi(IIR0, &spll_iir_indata, 1, SPLL_NOTCH_FILTER_COEFF_GROUP, (int16_t *)&spll_obj->y_notch1, SPLL_NOTCH_FILTER_LENGTH);
//    spll_obj->spll_erro_sum += spll_obj->y_notch1; // error累加
//    spll_obj->dwn = (spll_obj->y_notch1 * spll_error_sum_gain + spll_obj->spll_erro_sum) >> 1;
//    spll_obj->wn = (spll_obj->dwn + spll_wn_offset) / spll_wn_scale;

//    // 更新 theta
//    spll_obj->theta = (spll_obj->theta * theta_amp + spll_obj->wn) /theta_amp;
//    if (spll_obj->theta < 0)
//    {
//        spll_obj->theta += spll_theta_max;
//    }
//    else if (spll_obj->theta >= spll_theta_max)
//    {
//        spll_obj->theta -= spll_theta_max;
//    }

//    // 更新 cosine 和 sine
//    spll_obj->cosine_prev = spll_obj->cosine;
//    spll_obj->sine_prev = spll_obj->sine;
//     theta_index = (int32_t)(spll_obj->theta / SPLL_THETA_MAX * (LEN_COSSIN_BUF - 1));
//    spll_obj->cosine = cosdata[spll_obj->theta >> 1];
//    spll_obj->sine = sindata[spll_obj->theta >> 1];
//}