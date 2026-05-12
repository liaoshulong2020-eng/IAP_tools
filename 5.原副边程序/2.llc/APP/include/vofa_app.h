/**
  ******************************************************************************
  * @file    tmf5xxx_ll_adc_app.h
  * @author  Degital Power Application Team
  * @brief   Header file of ADC_APP config.h.
  *
  * <h2><center>&copy; Copyright (c) 2020 Tai-Action.
  * All rights reserved.</center></h2>
  *
  * This software is licensed by Tai-Action under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _USER_VOFA_H_
#define _USER_VOFA_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "tae32g58xx_it.h"
#include "main.h"

#define USER_UART_BAUDRATE         3000000   //波特率
#define USER_UART_TIMEOUT          10       //发送最大等待时间

//#define VOFA_UART                  UART2   //VOFA串口使用UART2	CAN 复用

#define VOFA_UART                  UART0    //VOFA串口使用UART0

#define cesu                0   //启用测速
#define cesu_gpio_port      GPIOB
#define cesu_gpio_pin       GPIO_PIN_15

/*要接收的数据：即需要调整的参数,如PID的系数，可以通过VOFA发送想要更改的数据给下位机，
通过下位机程序将接收值赋值给相应变量即可。*/
#define rec_data_len       5           //要接收的数据的字节数（每个数据要求一样的长度，且为字符串形式）
#define rec_data_num       4           //要接收的数据的个数
#define rec_byte_len       rec_data_len*rec_data_num          //要接收的字节数

/*发送的数据：即想要在VOFA上观察的数据，可以是ADC采集的电压或电流的值*/
#define tx_data_num        10           //发送数据的个数 
#define tx_data_len        (tx_data_num+1)*4

typedef union
{
   float   f[tx_data_num+1];
	 uint8_t c[tx_data_len];
}tx_fcunion;


//extern float tx_vofa_data[tx_data_num];    //发送的数据
extern float rx_vofa_data[rec_data_num];   //串口接收到的字符串数据经过处理后得到的浮点数据
extern tx_fcunion tx_vofa_data;
extern uint8_t vofa_flog;
void rec_buf(char* rxbuf,float* fc_buf);
void user_vofa_tx(void);
void User_VOFA_Init(void);
void User_VOFA_UART_DeInit(void);
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private types -------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MAIN_H_ */

/************************* (C) COPYRIGHT Tai-Action *****END OF FILE***********/

