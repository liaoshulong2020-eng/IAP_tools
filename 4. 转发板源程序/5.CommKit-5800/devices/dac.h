#ifndef DAC_H
#define DAC_H

#include "main.h"

/*
 * 初始化
 */
void dac_init(DAC_TypeDef *dac);

/*
 * 输出模拟电压
 */
void dac_output(DAC_TypeDef *dac,ushort mv);

#endif /* DAC_H */
