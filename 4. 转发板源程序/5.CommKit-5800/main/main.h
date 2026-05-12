/*
 * main.h
 *
 *  Created on: 2024年3月20日
 *      Author: Liang Jinfeng
 */

#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * includes
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "tae32g58xx_ll.h"
#include "dbg/user_debug.h"

/*******************************************************************************
 * 自定义类型
 ******************************************************************************/

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned long  ulong;

typedef unsigned char  U8;
typedef signed   char  S8;
typedef unsigned short U16;
typedef signed   short S16;
typedef unsigned long  U32;
typedef signed   long  S32;
typedef float          FP32;
typedef double         FP64;

/*
 * bool类型在stdbool.h文件中定义
 */
//typedef unsigned char  bool;
//#define true 1
//#define false 0

//位操作
#define set_bit(data,bit) ((data)|=1<<(bit))
#define clear_bit(data,bit) ((data)&=~(1<<(bit)))

/*******************************************************************************
 * 通用配置
 ******************************************************************************/

//Qbit最大值
#define QMAX(bit)			(1<<bit)

/*******************************************************************************
 * MCU配置
 ******************************************************************************/

//CPU主频
#define Fcpu				180000000UL //180MHz

//ADC参考电压：mV
#define ADC_VREF			2900 //2900mV
//ADC参考电压：V
#define ADC_VREF_V			(ADC_VREF/1000) //2.9V

//DAC参考电压：mV
#define DAC_VREF			2900 //2900mV

//毫秒延时
#define lldelayms			LL_Delay

//重启CPU
#define sys_reset			NVIC_SystemReset

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* MAIN_H */
