#ifndef _GPIO_APP_H
#define _GPIO_APP_H
#include "main.h"

void gpio_init_config(void);
void gpio_state_init(void);
void led_breath(void);
void on_off_ctrl_check_app();
void enable_current_sharing(void);
void disable_current_sharing(void);
void pfc_state_check_app(void);
bool read_sync_sign(void);
void enable_sync_start(void);
void disable_sync_start(void);
void read_ac_alarm_app(void);

#define ENACTRL_PORT_C    			GPIOC
#define ENACTRL_PIN_14      		GPIO_PIN_14

#define ON_OFF_CTRL_PORT    		GPIOC
#define ON_OFF_CTRL_PIN_15	    GPIO_PIN_15

#define CB_CTRL_PORT    				GPIOA
#define CB_CTRL_PIN_15	      	GPIO_PIN_15

#define POWER_DET_PORT    			GPIOB
#define POWER_DET_PIN_4	     		GPIO_PIN_4

#define SYNC_IN_PORT    				GPIOB
#define SYNC_IN_PIN_10	      	GPIO_PIN_10

#define SYNC_OUT_PORT    				GPIOB
#define SYNC_OUT_PIN_11	     		GPIO_PIN_11

#define	AC_ALARM_PIN_PORT				GPIOC
#define	AC_ALARM_PIN_13					GPIO_PIN_13

#define	ADDR1_PIN_PORT					GPIOC
#define	ADDR1_PIN								GPIO_PIN_0

#define	ADDR2_PIN_PORT					GPIOB
#define	ADDR2_PIN								GPIO_PIN_0

#define	ADDR3_PIN_PORT					GPIOA
#define	ADDR3_PIN								GPIO_PIN_7

#endif 
/******************************** END OF FILE ********************/




