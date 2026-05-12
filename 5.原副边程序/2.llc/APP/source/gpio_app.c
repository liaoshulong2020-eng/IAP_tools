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

	User_GPIO_Init.Pin = ENACTRL_PIN_14;
  LL_GPIO_Init(ENACTRL_PORT_C, &User_GPIO_Init);
	
	User_GPIO_Init.Pin = CB_CTRL_PIN_15;
  LL_GPIO_Init(CB_CTRL_PORT, &User_GPIO_Init);
	
	User_GPIO_Init.Pull      = GPIO_PULLDOWN;
	User_GPIO_Init.Pin = SYNC_OUT_PIN_11;
  LL_GPIO_Init(SYNC_OUT_PORT, &User_GPIO_Init);
	
//	User_GPIO_Init.Pull      = GPIO_PULLDOWN;
	User_GPIO_Init.Pin = GPIO_PIN_10;
  LL_GPIO_Init(GPIOA, &User_GPIO_Init);					//UART_RX 测试使用
	
/*--------------------	OUTPUT	--------------------*/	

/*--------------------	INPUT	 --------------------*/	
	User_GPIO_Init.Alternate = GPIO_AF0_INPUT;
	
	User_GPIO_Init.Pull      = GPIO_PULLUP;

	User_GPIO_Init.Pin = ADDR1_PIN;					
  LL_GPIO_Init(ADDR1_PIN_PORT, &User_GPIO_Init);
	
	User_GPIO_Init.Pin = ADDR2_PIN;					
  LL_GPIO_Init(ADDR2_PIN_PORT, &User_GPIO_Init);
	
	User_GPIO_Init.Pin = ADDR3_PIN;					
  LL_GPIO_Init(ADDR3_PIN_PORT, &User_GPIO_Init);
	
	
		

	User_GPIO_Init.Pull      = GPIO_PULLDOWN;
	User_GPIO_Init.Pin 			 = POWER_DET_PIN_4;						//PFC光耦（高有效）
  LL_GPIO_Init(POWER_DET_PORT, &User_GPIO_Init);
	
	User_GPIO_Init.Pull			 = GPIO_NOPULL;
	User_GPIO_Init.Pin 			 = AC_ALARM_PIN_13;						//接收原边AC掉电告警（>165V为低，<140V为高）
  LL_GPIO_Init(AC_ALARM_PIN_PORT, &User_GPIO_Init);
	
	User_GPIO_Init.Pin 			 = ON_OFF_CTRL_PIN_15;				//外部开关使能（低有效）
  LL_GPIO_Init(ON_OFF_CTRL_PORT, &User_GPIO_Init);
	
	User_GPIO_Init.Pull      = GPIO_PULLDOWN;
	User_GPIO_Init.Pin 			 = SYNC_IN_PIN_10;						//同步信号输入（低有效）
  LL_GPIO_Init(SYNC_IN_PORT, &User_GPIO_Init);
	
/*--------------------	INPUT	 --------------------*/		
}

RAMCODE
void gpio_state_init(void)
{
  LL_GPIO_WritePin(ENACTRL_PORT_C, ENACTRL_PIN_14, GPIO_PIN_RESET);
//	LL_GPIO_WritePin(SYNC_OUT_PORT, ON_OFF_CTRL_PIN_15, GPIO_PIN_SET);
}

RAMCODE
void led_breath(void)
{
	llc.led_cnt++;
	if(llc.led_cnt > 5000)
	{
//		LL_GPIO_TogglePin(LED_PORT_C,LED_PIN_13);
		llc.led_cnt = 0;
	}
}

RAMCODE
void on_off_ctrl_check_app(void)
{

	if(!LL_GPIO_ReadPin(ON_OFF_CTRL_PORT,ON_OFF_CTRL_PIN_15))
	{
		llc.off_ctrl_cnt++;
		
		if(llc.off_ctrl_cnt > 10 )
		{
			llc.off_ctrl_cnt = 0;
			llc.on_off_ctrl_ok = 0;
		}
		
	}
	else
	{
		llc.on_off_ctrl_cnt++;
		
		if(llc.on_off_ctrl_cnt > 50)
		{
			llc.on_off_ctrl_ok = 1 ;
			llc.on_off_ctrl_cnt = 0 ;
		}

	}	
}

RAMCODE
void enable_current_sharing(void)
{
	LL_GPIO_WritePin(CB_CTRL_PORT, CB_CTRL_PIN_15, GPIO_PIN_RESET);	//使能均流
}

RAMCODE
void disable_current_sharing(void)
{
	LL_GPIO_WritePin(CB_CTRL_PORT, CB_CTRL_PIN_15, GPIO_PIN_SET);//不使能均流
}

RAMCODE
void enable_sync_start(void)
{
	LL_GPIO_WritePin(SYNC_OUT_PORT, SYNC_OUT_PIN_11, GPIO_PIN_SET);	//使能同步启机
}

RAMCODE
void disable_sync_start(void)
{
	LL_GPIO_WritePin(SYNC_OUT_PORT, SYNC_OUT_PIN_11, GPIO_PIN_RESET);	//关闭同步启机信号
}

RAMCODE
void ac_is_ok(void)
{
	LL_GPIO_WritePin(ENACTRL_PORT_C, ENACTRL_PIN_14, GPIO_PIN_RESET);	
}

RAMCODE
void ac_is_no_ok(void)
{
	LL_GPIO_WritePin(ENACTRL_PORT_C, ENACTRL_PIN_14, GPIO_PIN_SET);	
}

RAMCODE
bool read_sync_sign(void)
{
	return LL_GPIO_ReadPin(SYNC_IN_PORT, SYNC_IN_PIN_10);	//同步启机信号输入
}



RAMCODE
void pfc_state_check_app(void)
{
	if(!LL_GPIO_ReadPin(POWER_DET_PORT, POWER_DET_PIN_4))
	{
		llc.pfc_is_ok = 0 ;
	}
	else
	{
		llc.pfc_is_ok_cnt++;
		if(llc.pfc_is_ok_cnt > 5000)
		{
			llc.pfc_is_ok = 1 ;
			llc.pfc_is_ok_cnt = 0;
		}
	}
}

RAMCODE
void read_ac_alarm_app(void)
{
	if(!LL_GPIO_ReadPin(AC_ALARM_PIN_PORT, AC_ALARM_PIN_13)) 	//ac_alarm
	{
			ac_is_no_ok();
			llc.ac_is_ok = 0;
	}
	else
	{
		llc.ac_is_ok_cnt++;
		if(llc.ac_is_ok_cnt > 5000)
		{
			ac_is_ok();
			llc.ac_is_ok = 1;
			llc.ac_is_ok_cnt = 0;
		}

	}
}


