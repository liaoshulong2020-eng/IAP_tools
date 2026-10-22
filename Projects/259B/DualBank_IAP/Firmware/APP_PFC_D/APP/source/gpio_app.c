
#include "main.h"



/* Private Constants ---------------------------------------------------------*/
/** @defgroup User_GPIO_Flow_LED_Private_Constants User GPIO_Flow_LED Private Constants
  * @brief    User GPIO_Flow_LED Private Constants
  * @{
  */

#define LED_PORT_A    					GPIOA
#define LED_PIN_10      				GPIO_PIN_10	//PC13 实际硬件未设计

#define AC_ALARM_PORT_C    			GPIOC
#define AC_ALARM_PIN_14      		GPIO_PIN_14

#define POWER_DET_PORT    			GPIOC
#define POWER_DET_PIN_15	      GPIO_PIN_15

#define	RELAY_PORT_A						GPIOA
#define	RELAY_PIN_8							GPIO_PIN_8

/**
  * @brief  User GPIO Example
  * @param  None
  * @return None
  */
void gpio_init_app()
{
	gpio_init_config();
  gpio_output_init();
  gpio_input_init();
  gpio_state_init();
}

/**
  * @}
  */


/* Private Functions ---------------------------------------------------------*/
/** @addtogroup User_GPIO_Flow_LED_Private_Functions
  * @{
  */

/**
  * @brief  User GPIO Init
  * @param  None
  * @return None
  */
void gpio_init_config()
{
  GPIO_InitTypeDef GPIO_Init_Structure;
  GPIO_Init_Structure.Alternate = GPIO_AF1_OUTPUT;
  GPIO_Init_Structure.Pull      = GPIO_NOPULL;
  GPIO_Init_Structure.IntMode   = GPIO_INT_MODE_CLOSE;
  GPIO_Init_Structure.OType     = GPIO_OTYPE_PP;
  GPIO_Init_Structure.Speed     = GPIO_SPEED_FREQ_HIGH;
  //User GPIO LED Init
  GPIO_Init_Structure.Pin = LED_PIN_10;
  LL_GPIO_Init(LED_PORT_A, &GPIO_Init_Structure);
  //User GPIO LED4 Init
  GPIO_Init_Structure.Pin = POWER_DET_PIN_15;
  LL_GPIO_Init(POWER_DET_PORT, &GPIO_Init_Structure);
	

//	GPIO_Init_Structure.Pin = GPIO_PIN_10;
//  LL_GPIO_Init(GPIOA, &GPIO_Init_Structure);		//test pin

	GPIO_Init_Structure.Pull      = GPIO_NOPULL;
	GPIO_Init_Structure.Pin = AC_ALARM_PIN_14;	
	LL_GPIO_Init(AC_ALARM_PORT_C, &GPIO_Init_Structure);
	
//	GPIO_Init_Structure.Pin = RELAY_PIN_8;
//  LL_GPIO_Init(RELAY_PORT_A, &GPIO_Init_Structure);
}
void gpio_output_init()
{


}

void gpio_input_init()
{
}

void gpio_state_init()
{
  LL_GPIO_WritePin(LED_PORT_A, LED_PIN_10, GPIO_PIN_SET);
  LL_GPIO_WritePin(POWER_DET_PORT, POWER_DET_PIN_15, GPIO_PIN_RESET);
	LL_GPIO_WritePin(RELAY_PORT_A, RELAY_PIN_8, GPIO_PIN_RESET);
	LL_GPIO_WritePin(AC_ALARM_PORT_C, AC_ALARM_PIN_14, GPIO_PIN_SET);	
}

/**
  * @}
  */
//void breathing_light()
//{
//  pfc.led_cnt++;
//  if(pfc.led_cnt > 60000)
//    {
//      pfc.led_cnt = 0;
//    }
//}

void pfc_ok_signal()
{
	LL_GPIO_WritePin(POWER_DET_PORT, POWER_DET_PIN_15, GPIO_PIN_SET);
}

void pfc_nok_signal()
{
	LL_GPIO_WritePin(POWER_DET_PORT, POWER_DET_PIN_15, GPIO_PIN_RESET);
}

void pfc_ac_is_ok()	 //AC_ALARM
{
	LL_GPIO_WritePin(AC_ALARM_PORT_C, AC_ALARM_PIN_14, GPIO_PIN_RESET);
}

void pfc_ac_is_nok()	//AC_ALARM
{
	LL_GPIO_WritePin(AC_ALARM_PORT_C, AC_ALARM_PIN_14, GPIO_PIN_SET);
}
/**
  * @}
  */
RAMCODE
void relay_ctrl_enable_high(void)
{
	pfc.pre_driver_cmpb = pfc.pre_driver_period+100;
	//  LL_GPIO_WritePin(RELAY_PORT_A, RELAY_PIN_8, GPIO_PIN_SET);
}

RAMCODE
void relay_ctrl_disable_low(void)
{
	
		hrpwm_pre_outdis();
//  LL_GPIO_WritePin(RELAY_PORT_A, RELAY_PIN_8, GPIO_PIN_RESET);
}

RAMCODE
void relay_ctrl_disable_idel(void)
{
		pfc.pre_driver_cmpb = 100;
//  LL_GPIO_WritePin(RELAY_PORT_A, RELAY_PIN_8, GPIO_PIN_RESET);
}
/**
  * @}
  */
RAMCODE
void led_breath(void)
{
	pfc.led_cnt++;
	if(pfc.led_cnt > 5000)
	{
		pfc.led_cnt = 0;
	}
}

/************************* (C) COPYRIGHT Tai-Action *****END OF FILE***********/

