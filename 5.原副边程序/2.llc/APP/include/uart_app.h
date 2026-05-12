/**
  ******************************************************************************
  * @file    uart_app.h
  * @brief   UART底层驱动头文件（副边UART通讯）
  ******************************************************************************
  */
#ifndef _USER_UART_H_
#define _USER_UART_H_
#include "main.h"

void user_uart_init(UART_TypeDef *Instance);
void User_UART_DeInit(void);
void User_Uart_TxCpltCallback(void);
void User_Uart_RxCpltCallback(void);
void uart_receive_data(void);
void uart_send_u8data(uint8_t *buf);

#define USER_UART_COM_BAUDRATE      (115200)
#define USER_UART                   UART0

#endif /* _USER_UART_H_ */
