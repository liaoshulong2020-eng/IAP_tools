#ifndef _USER_UART_H_
#define _USER_UART_H_
#include "main.h"

void user_uart_init();
void User_UART_DeInit();
void User_Uart_TxCpltCallback(void);
void User_Uart_RxCpltCallback(void);

#define USER_UART_COM_BAUDRATE          (115200)
#define USER_UART										UART0

#endif
