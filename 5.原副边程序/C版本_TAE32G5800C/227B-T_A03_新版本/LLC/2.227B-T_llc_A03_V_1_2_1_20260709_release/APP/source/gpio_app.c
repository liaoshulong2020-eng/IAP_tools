#include "main.h"
#include "gpio_app.h"




void gpio_init_config(void)
{
/*--------------------	OUTPUT	--------------------*/
  GPIO_InitTypeDef User_GPIO_Init;
  User_GPIO_Init.Alternate = GPIO_AF1_OUTPUT;
  User_GPIO_Init.Pull      = GPIO_NOPULL;
  User_GPIO_Init.IntMode   = GPIO_INT_MODE_CLOSE;
  User_GPIO_Init.OType     = GPIO_OTYPE_PP;
  User_GPIO_Init.Speed     = GPIO_SPEED_FREQ_HIGH;

  User_GPIO_Init.Pin = LED_PIN_13;
  LL_GPIO_Init(LED_PORT_C, &User_GPIO_Init);		//LED

	User_GPIO_Init.Pin = CB_CTRL_PIN_11;					//均流使能
  LL_GPIO_Init(CB_CTRL_PORT, &User_GPIO_Init);
	
	User_GPIO_Init.Pin = TEST_PIN_11;					//test_pin PB11 TEST
  LL_GPIO_Init(TEST_PIN_PORT, &User_GPIO_Init);
	
	
/*--------------------	OUTPUT	--------------------*/	

/*--------------------	INPUT	 --------------------*/	
	User_GPIO_Init.Alternate = GPIO_AF0_INPUT;
	
	User_GPIO_Init.Pin = ENACTRL_PIN_14;
  LL_GPIO_Init(ENACTRL_PORT_C, &User_GPIO_Init);	  //ENA_CTRL
	  
	User_GPIO_Init.Pull      = GPIO_PULLUP;
	User_GPIO_Init.Pin = ON_OFF_CTRL_PIN_15;					//外部开关使能
  LL_GPIO_Init(ON_OFF_CTRL_PORT, &User_GPIO_Init);
	
	User_GPIO_Init.Pin = ADDR1_PIN;					
  LL_GPIO_Init(ADDR1_PIN_PORT, &User_GPIO_Init);
	
	User_GPIO_Init.Pin = ADDR2_PIN;					
  LL_GPIO_Init(ADDR2_PIN_PORT, &User_GPIO_Init);
	
	User_GPIO_Init.Pin = ADDR3_PIN;					
  LL_GPIO_Init(ADDR3_PIN_PORT, &User_GPIO_Init);
	
	User_GPIO_Init.Pull      = GPIO_PULLDOWN;
	User_GPIO_Init.Pin = POWER_DET_PIN_9;						//PFC光耦
  LL_GPIO_Init(POWER_DET_PORT, &User_GPIO_Init);
	

/*--------------------	INPUT	 --------------------*/		
}

RAMCODE
void gpio_state_init(void)
{
  LL_GPIO_WritePin(LED_PORT_C, LED_PIN_13, GPIO_PIN_SET);
  LL_GPIO_WritePin(CB_CTRL_PORT, CB_CTRL_PIN_11, GPIO_PIN_SET);
	LL_GPIO_WritePin(TEST_PIN_PORT, TEST_PIN_11, GPIO_PIN_RESET);
}

RAMCODE
void led_breath(void)
{
	llc.led_cnt++;
	if(llc.led_cnt > 5000)
	{
		LL_GPIO_TogglePin(LED_PORT_C,LED_PIN_13);
		llc.led_cnt = 0;
	}
}

RAMCODE
void on_off_ctrl_check_app(void)
{

	if(LL_GPIO_ReadPin(ON_OFF_CTRL_PORT,ON_OFF_CTRL_PIN_15))
	{
		llc.off_ctrl_cnt++;
		
		if(llc.off_ctrl_cnt > 10 )
		{
			llc.off_ctrl_cnt = 0;
			llc.on_off_ctrl_ok = 0;
			if(((llc.state == state_on) || (llc.state == state_rampup)) && !llc.fault_state.all && !llc.HwOcp)
			{
				llc.on_off_ctrl_fault = 1;
			}
		}
		
	}
	else
	{
		llc.on_off_ctrl_cnt++;
		
		if(llc.on_off_ctrl_cnt > 50)
		{
			llc.on_off_ctrl_ok = 1 ;
			llc.on_off_ctrl_cnt = 0 ;
			llc.on_off_ctrl_fault = 0;
		}

	}	
}

RAMCODE
void enable_current_sharing(void)
{
	LL_GPIO_WritePin(CB_CTRL_PORT, CB_CTRL_PIN_11, GPIO_PIN_RESET);	//使能均流
}

RAMCODE
void disable_current_sharing(void)
{
	LL_GPIO_WritePin(CB_CTRL_PORT, CB_CTRL_PIN_11, GPIO_PIN_SET);//不使能均流
}

RAMCODE
void pfc_state_check_app(void)
{
	if(!LL_GPIO_ReadPin(POWER_DET_PORT, POWER_DET_PIN_9))
	{
		llc.pfc_is_ok = 0 ;

	}
	else
	{
		llc.pfc_is_ok_cnt++;
		if(llc.pfc_is_ok_cnt > 5)
		{
			llc.pfc_is_ok = 1 ;
			llc.pfc_is_ok_cnt = 0;
		}
	
	}
		
}

