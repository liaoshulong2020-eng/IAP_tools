#include "main.h"
#include "protection_app.h"
int No_fault(void);
void fault_check_app(void);
extern void fault_check_app(void);


//void under_input_voltage_check_app(void)		//输入欠压
RAMCODE
void input_voltage_check_app(void)
{
  //	static uint16_t AC_OK_count  = 0;
//   static uint16_t VIN_OV_count = 0;
  /*==========AC VOLTAGE STATE CHECK============*/
  if(pfc.under_input_flag)
    {
      if((pfc.vin_rel ) > pfc.vin_on_voltage)//VIN_ON_VOLTAGE)
        {
          pfc.under_input_cnt ++ ;
        }
      else
        {
          pfc.under_input_cnt = 0;
        }
      if(pfc.under_input_cnt > 50)         //about 500ms
        {
          pfc.under_input_cnt = 0;
          pfc.is_ac_ok = 1;
          pfc.under_input_flag = 0;
					hrpwm_pre_outen();
					pfc.fault_num &= ~(1<<0);
        }
    }
  else
    {
      if((pfc.vin_rel ) < pfc.vin_under_voltage)//VIN_UNDER_VOLTAGE)
        {
          //	AC_OK_count++;
          pfc.under_input_cnt ++ ;
        }
      else
        {
          //	AC_OK_count = 0;
          pfc.under_input_cnt = 0;
        }
      if(pfc.under_input_cnt > 50)
        {
          //	AC_OK_count = 0;
          hrpwm_outdis_app();
          relay_ctrl_disable_low();
          pfc.state = State_Fault;
					pfc_nok_signal();
					
          pfc.under_input_cnt = 0;
          pfc.is_ac_ok = 0;
          pfc.under_input_flag = 1;
					pfc.pre_finish_flg = 0;
					pfc.close_pwm_flag |=(1<<2);
					pfc.fault_num |= (1<<0);
          pfc.Fault_Flag = 1;
        }
    }
  if(pfc.over_input_flag)
    {
      if((pfc.vin_rel ) < pfc.vin_max_voltage)//VIN_MAX_VOLTAGE)
        {
          //  VIN_OV_count++;
          pfc.over_input_rec_cnt ++ ;
        }
      else
        {
          // VIN_OV_count = 0;
          pfc.over_input_rec_cnt = 0;
        }
      if(pfc.over_input_rec_cnt > 50)       //about 1000ms
        {
          //  VIN_OV_count = 0;
          pfc.over_input_rec_cnt = 0;
          pfc.over_input_flag = 0;
					pfc.fault_num &= ~(1<<1);
        }
    }
  else
    {
      if((pfc.vin_rel ) > pfc.vin_over_voltage)//VIN_OVER_VOLTAGE)
        {
          //   VIN_OV_count++;
          pfc.over_input_cnt ++;
        }
      else
        {
          //  VIN_OV_count = 0;
          pfc.over_input_cnt = 0;
        }
      if(pfc.over_input_cnt > 50)       //about 1000ms
        {
          //   VIN_OV_count = 0;
          hrpwm_outdis_app();
					pfc.close_pwm_flag |=(1<<3);
//          LL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
//          if(pfc.state == State_Idel)
//            {
//              pfc.state = State_Idel;
//            }
//          else
//            {
//              pfc.state = State_Fault;
//            }
          pfc.over_input_cnt = 0;
          pfc.over_input_flag = 1;
					pfc.fault_num |= (1<<1);
          pfc.Fault_Flag = 1;
        }
    }
}

RAMCODE
void over_output_voltage_check_app(void)
{
//   static uint16_t BUS_OV_count = 0;
  static uint16_t BUS_OK_count = 0;
//		if(pfc.V_over_output_flag)
//		{
//			if((pfc.vdc_filtered >> VBUS_FILTER_CNT) < VPFC_SAMP_MIN_VALUE){
//				pfc.V_over_output_cnt ++ ;
//			}
//			else{
//				pfc.V_over_output_cnt = 0;

//			}
//			if(pfc.V_over_output_cnt>100){		//10ms
//				pfc.V_over_output_cnt = 0;
//					pfc.V_over_output_flag = 0;
//			}
//		}
//		else
//		{
  if((pfc.vbus_rel) > VOUT_OVER_VOLTAGE)
    {
      //	BUS_OV_count++;
      pfc.V_over_output_cnt ++;
//            pfc.fault_num |= BIT(0);
    }
  else
    {
      //BUS_OV_count = 0;
      pfc.V_over_output_cnt = 0;
    }
  if(pfc.V_over_output_cnt > 150)
    {
      //	BUS_OV_count = 0;
//				hrpwm_outdis_app();
//				LL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);  //PFC故障命令
//				if(pfc.state == State_Idel)
//				{
//					pfc.state = State_Idel;
//				}
//				else
//				{
//					pfc.state = State_Fault;
//				}
      pfc.V_over_output_cnt = 0;
      pfc.V_over_output_flag = 1;
			pfc.fault_num |= (1<<2);
//				pfc.Fault_Flag = 1;
    }
//		}

//		//BUS_OK
//		if(pfc.is_vbus_ok){
//				if(((pfc.vdc_filtered >> VBUS_FILTER_CNT) < VPFC_SAMP_UNDER_VALUE) || (pfc.V_over_output_flag)
//					|| pfc.over_input_flag || pfc.under_input_flag || (pfc.state != State_On)){
//						BUS_OK_count = 0;
//						pfc.is_vbus_ok = 0;
//						LL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
////            LL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_RESET);
//				}
//		}
//		else{
//				if(((pfc.vdc_filtered >> VBUS_FILTER_CNT) > VPFC_SAMP_RECOVER_VALUE) && (pfc.state == State_On)\
//			&& (pfc.V_over_output_flag == 0) && (pfc.over_input_flag == 0) && (0 == pfc.under_input_flag)){
//						if(BUS_OK_count ++ > 3000){
//								pfc.is_vbus_ok = 1;
//								pfc.Fault_Flag = 0;
//								BUS_OK_count = 0;
//							 LL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);
////                LL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_SET);
//						}
//				}
//				else{
//						BUS_OK_count = 0;
//				}
//		}

}


RAMCODE
void over_output_current_check_app(void)		//输出过流
{
//	static uint16_t IUS_OC_count = 0;
// static uint16_t IUS_OK_count = 0;
//  if(pfc.I_over_output_flag)
//    {
//      if((pfc.i_value) < PFC_I_OCP_REC_VALUE)
//        {
//          //	IUS_OK_count++;
//          pfc.I_over_output_cnt ++;
//        }
//      else
//        {
//          //IUS_OK_count = 0;
//          pfc.I_over_output_cnt = 0;

//        }
//      if(pfc.I_over_output_cnt > 15000) 		//10ms
//        {
//          //	IUS_OK_count = 0;
//          pfc.I_over_output_cnt = 0;
//          pfc.I_over_output_flag = 0;
//        }
//    }
//  else
//    {
  if((pfc.iloop.rel) > IPFC_OCP_CURRENT)
    {
      //	IUS_OC_count++;
      pfc.I_over_output_cnt ++;
//            pfc.fault_num |= BIT(0);
    }
  else
    {
      //	IUS_OC_count = 0;
      pfc.I_over_output_cnt = 0;
    }
  if(pfc.I_over_output_cnt > 100)
    {
//          hrpwm_outdis_app();
//          LL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
      if(pfc.state == State_Idel)
        {
          pfc.state = State_Idel;
        }
      else
        {
          pfc.state = State_Fault;
        }
      //IUS_OC_count = 0;
      pfc.I_over_output_cnt = 0;
      pfc.I_over_output_flag = 1;
			pfc.fault_num |= (1<<3);
      pfc.Fault_Flag = 1;
    }
//    }

  /*  if(pfc.out_current_value > IOUT_OC_POINT)
    {
        pfc.I_over_output_flag = 1 ;				//过流
        																		//红灯亮
    }
    else
    {
        pfc.I_over_output_flag = 0;				//非过流
    }*/
}

RAMCODE
void fault_check_app(void)
{
#if TEST_MODE
	over_output_current_check_app();				//输出过流检查
  over_output_voltage_check_app();				//输出过压检查
#else
  // under_input_voltage_check_app(); 				//输入欠压检查
  input_voltage_check_app();
//		output_voltage_check_app();
  over_output_current_check_app();				//输出过流检查
//		over_input_voltage_check_app();					//输入过压检查
  over_output_voltage_check_app();				//输出过压检查
#endif
  if(No_fault())
    {
      pfc.is_no_fault = 1;
    }
  else
    {
      pfc.is_no_fault = 0;
    }
}

RAMCODE
int No_fault(void)
{
  if(!(pfc.under_input_flag | pfc.over_input_flag | pfc.V_over_output_flag | pfc.I_over_output_flag))
    {
//			pfc.is_no_fault = 1;
      return 1;
    }
  else
//		pfc.is_no_fault = 0;
    return 0;

}
RAMCODE
int Idel_No_fault(void)
{
  if(!(pfc.under_input_flag | pfc.over_input_flag | pfc.V_over_output_flag))
    {
      pfc.idel_is_ok = 1;
      return 1;
    }
  else
    pfc.idel_is_ok = 0;
  return 0;

}
RAMCODE
int Rampup_No_fault(void)
{
  if(!(pfc.under_input_flag | pfc.over_input_flag | pfc.V_over_output_flag))
    {
      pfc.rampup_is_ok = 1;
      return 1;
    }
  else
    pfc.rampup_is_ok = 0;
  return 0;
}

RAMCODE
int On_No_fault(void)
{
#if TEST_MODE
#else
  if(!(pfc.under_input_flag | pfc.over_input_flag | pfc.V_over_output_flag))
    {
      pfc.on_is_ok = 1;
      return 1;
    }
  else
    pfc.on_is_ok = 0;
  return 0;
#endif
}


