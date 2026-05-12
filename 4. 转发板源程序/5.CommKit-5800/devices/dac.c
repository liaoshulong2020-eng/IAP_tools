#include "dac.h"

/*
 * 初始化
 */
void dac_init(DAC_TypeDef *dac)
{
	DAC_InitTypeDef init={0};

	//VREFBUF Enable：采用2.9V
//	__LL_SYSCTRL_SpRegWrite_Unlock(SYSCTRL);
//	__LL_SYSCTRL_VREFBUF_En(SYSCTRL);
//	__LL_SYSCTRL_SpRegWrite_Lock(SYSCTRL);

	//User DAC Init
	init.trig_en=false;
	init.buf_out_en=true; //引脚输出使能配置
	init.bypass_buf_out_en=false;
	LL_DAC_Init(dac,&init);

	//DAC Start Output
	LL_DAC_Start(dac);
}

/*
 * 输出模拟电压
 */
void dac_output(DAC_TypeDef *dac,ushort mv)
{
	ushort value;
	value=(ulong)mv*QMAX(12)/DAC_VREF;
	LL_DAC_ValueSet(dac,value);
}
