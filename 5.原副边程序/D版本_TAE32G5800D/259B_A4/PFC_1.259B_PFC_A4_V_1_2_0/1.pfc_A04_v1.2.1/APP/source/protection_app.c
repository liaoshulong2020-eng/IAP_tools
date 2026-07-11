#include "main.h"
#include "protection_app.h"
int No_fault(void);
void fault_check_app(void);
extern void fault_check_app(void);

/* --- 硬件与环境定义 --- */
#define ISR_FREQ_HZ         10000.0f    // 中断频率 10kHz
#define AC_FREQ_HZ          50.0f       // 电网频率 50Hz
#define VIN_REF_RMS         155.0f      // 基准电压（取额定最低，保证不误触）
#define VIN_UV_THRES_V      20.0f       // 欠压判定阈值 50V

/* --- 裕量定义 --- */
#define PROTECTION_MARGIN   2.0f        // 裕量倍数 (建议 1.8 - 2.5)

// 常数 K = PI * sqrt(2) * AC_FREQ = 222.144
#define CONST_K             222.144f 

// 155V 下低于 50V 的基础点数 (理论值约 72.6)
#define THEORY_BASE_CNT     ((VIN_UV_THRES_V * ISR_FREQ_HZ) / (VIN_REF_RMS * CONST_K))

// 最终带裕量的计数值限制 (155V/50k/2.0倍时，结果约为 145)
#define FINAL_UV_CNT_LIMIT  ((uint16_t)(THEORY_BASE_CNT * PROTECTION_MARGIN))

/* --- 变量与中断逻辑 --- */
RAMCODE
void pfc_fast_input_under_protect(void) 
{
    // 1. 纯算术取绝对值
    float vIn_abs = (pfc.vin_voltage < 0.0f) ? -pfc.vin_voltage : pfc.vin_voltage;

    // 2. 快速欠压检测
    if (vIn_abs < VIN_UV_THRES_V) 
    {
        // 如果连续低压，累加计数
        if (++pfc.fast_input_under_cnt >= FINAL_UV_CNT_LIMIT) 
        {
						pfc_nok_signal();
					  hrpwm_outdis_app();
						relay_ctrl_disable_low();
						pfc.pre_finish_flg = 0;
						pfc.input_check_is_ok = 0;
						pfc.is_ac_ok = 0;
						pfc.state = State_Fault;
						Reset_value();
        }
    } 
    else 
    {
        // 只要电压回到阈值以上，立即清零计数（避开过零点）
        pfc.fast_input_under_cnt = 0;
    }
}


//void under_input_voltage_check_app(void)		//输入欠压
RAMCODE
void input_voltage_check_app(void)
{
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
					pfc_ac_is_ok();
          pfc.under_input_flag = 0;
					hrpwm_pre_outen();
					pfc.fault_num &= ~(1<<0);
        }
    }
  else
    {
      if((pfc.vin_rel ) < pfc.vin_under_voltage)//VIN_UNDER_VOLTAGE)
        {
					if(!pfc.is_llc_rampup)
					{
					  pfc.under_input_cnt ++ ;
					}
					else
					{
						pfc.under_input_cnt = 0;
					}

        }
      else
        {
          pfc.under_input_cnt = 0;
        }
      if(pfc.under_input_cnt > 50)
        {
          hrpwm_outdis_app();
          relay_ctrl_disable_low();
          pfc.state = State_Fault;
					pfc_nok_signal();
					pfc_ac_is_nok();
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
          pfc.over_input_rec_cnt ++ ;
        }
      else
        {
          pfc.over_input_rec_cnt = 0;
        }
      if(pfc.over_input_rec_cnt > 50)       //about 1000ms
        {
          pfc.over_input_rec_cnt = 0;
          pfc.over_input_flag = 0;
					pfc.fault_num &= ~(1<<1);
        }
    }
  else
    {
      if((pfc.vin_rel ) > pfc.vin_over_voltage)//VIN_OVER_VOLTAGE)
        {
          pfc.over_input_cnt ++;
        }
      else
        {
          pfc.over_input_cnt = 0;
        }
      if(pfc.over_input_cnt > 50)       //about 1000ms
        {
          hrpwm_outdis_app();
					pfc.close_pwm_flag |=(1<<3);
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
  static uint16_t BUS_OK_count = 0;
  if((pfc.vbus_rel) > VOUT_OVER_VOLTAGE)
    {
      pfc.V_over_output_cnt ++;
    }
  else
    {
      //BUS_OV_count = 0;
      pfc.V_over_output_cnt = 0;
    }
  if(pfc.V_over_output_cnt > 150)
    {
      pfc.V_over_output_cnt = 0;
      pfc.V_over_output_flag = 1;
			pfc.fault_num |= (1<<2);
    }




}


RAMCODE
void over_output_current_check_app(void)		//输出过流
{
  if((pfc.iloop.rel) > IPFC_OCP_CURRENT)
    {
      pfc.I_over_output_cnt ++;
    }
  else
    {
      pfc.I_over_output_cnt = 0;
    }
  if(pfc.I_over_output_cnt > 100)
    {
      if(pfc.state == State_Idel)
        {
          pfc.state = State_Idel;
        }
      else
        {
          pfc.state = State_Fault;
        }
      pfc.I_over_output_cnt = 0;
      pfc.I_over_output_flag = 1;
			pfc.fault_num |= (1<<3);
      pfc.Fault_Flag = 1;
    }
}

RAMCODE
void fault_check_app(void)
{
#if TEST_MODE
	over_output_current_check_app();				//输出过流检查
  over_output_voltage_check_app();				//输出过压检查
	 pfc.set_protect_is_ok = 1;
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


