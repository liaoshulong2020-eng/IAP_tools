#ifndef __VARIABLES_DEFINE_APP_H__
#define __VARIABLES_DEFINE_APP_H__

#include "main.h"
//<<< Use Configuration Wizard in Context Menu >>>

#define TEST_MODE		0
#define FEED_FORWARD_ENABLE		1

//寄存器基地址
#define DWT_CR *(uint32_t*)0xE0001000 //DWT 基地址
#define DWT_CYCCNT *(uint32_t*)0xE0001004 //CYCNT 计数器地址
#define DEM_CR *(uint32_t*)0xE000EDFC //调试控制器地址(上表的地址)
#define DEM_CR_TRCENA (1<<24) //定义使能跟踪位
#define DWT_CR_CYCCNTENA (1<<0) //定义使能 CYCCNT 位

/*============================ Defines (Constant) ============================*/
#define BUF_LEN 1
#define LEN_COSSIN_BUF			2048
#define START_COUNT_VAL			1000

////ADC filter
#define CUR_FLT_LEN	10
#define VOL_FLT_LEN	10
#define VBUS_FILTER_ORDER 	8	//2^4
#define	IIN_FILTER_ORDER 		4//2^2

//voltage loop defines
#define PI_I_HIGH_LIMIT		((1 << 30)-1)
#define PI_I_LOW_LIMIT		(-(1 << 12))
#define PI_OUTPUT_HIGH_LIMIT (32767) //max Q15 number
#define PI_OUTPUT_LOW_LIMIT (0) //min Q15 number
#define VBUS_FILTER_CNT (5)
#define IBUS_FILTER_CNT (0)
#define FEED_WARD_CNT   (15)

#define TXBUFFER_LEN 8
#define RXBUFFER_LEN 8


// <o>PFC_kp
//  <i>范围 0~500KHz
//  <0-5000000>
#define PFC_VLOOP_KP 	1.0f

// <o>PFC_ki
//  <i>范围 0~500KHz
//  <0-5000000>
#define PFC_VLOOP_KI 	0.0f

#define PFC_ILOOP_KP  10000.0f
#define PFC_ILOOP_KI	150.0f


/*============================HRPWM============================*/


typedef struct
{
  uint32_t period;
  uint32_t compa;
  uint32_t compb;
  uint32_t compc;
  uint32_t compd;
  uint32_t outb_cmp;

} PFC_HRPWM_TypeDef;

//About PWM
#define PFC_HRPWM_DIV						 				16
#define MCU_PWM_CLK 	 									180000
// <o>pfc驱动信号（KHz）
//  <i>范围 0~500KHz
//  <0-500>
#define PFC_DRIVER_CLK									100

// <o>可控硅驱动信号（KHz）
//  <i>范围 0~500KHz
//  <0-500>
#define PFC_SCR_DRV1_CLK								100

#define PFC_DRIVER_PERIOD_VALUE				  ((uint32_t)	MCU_PWM_CLK*PFC_HRPWM_DIV/PFC_DRIVER_CLK)

#define	PFC_SCR_DRV1_PERIOD_VALUE				((uint32_t)	MCU_PWM_CLK*PFC_HRPWM_DIV/PFC_SCR_DRV1_CLK)


// <o>死区大小（ns）
//  <i>范围 0~500ns

//  <0-500>
#define SCR_DRV1_DEADTIME								800
#define SCR_DRV1_DEADTIME_VAL 					((uint32_t)(SCR_DRV1_DEADTIME/(5.55/PFC_HRPWM_DIV)))


#define PFC_MAX_DUTY 										(uint32_t)(PFC_DRIVER_PERIOD_VALUE*0.95f)
#define PFC_MIN_DUTY 					  				100




/*============================HRPWM============================*/



/*=============================ADC=============================*/

/*--------------------ADC0------------------*/
#define ACL_SAMPLE_ADC0_CH2   						ADC_CH_2			//ADC0_IN2 ==> PA1	交流L线电压采样信号
#define ACN_SAMPLE_ADC0_CH3   						ADC_CH_3			//ADC0_IN3 ==> PA2	交流N线电压采样信号
#define I_SURGE_SAMPLE_ADC0_CH4   				ADC_CH_4			//ADC0_IN4 ==> PA3	电流采样信号
#define I_Cr_SAMPLE_ADC0_CH14   					ADC_CH_14		//ADC0_IN14 ==> PB11	I_Cr_SAMP采样信号

/*--------------------ADC1------------------*/

#define PFC_VBUS_SAMPLE_ADC1_CH3   				ADC_CH_3			//ADC1_IN6 ==> PA6  PFC母线电压采样
#define PFC_OCP_SAMPLE_ADC1_CH13   				ADC_CH_13		//ADC1_IN13 ==> PA5	PFC电流采样
#define R_NTC_SAMPLE_ADC1_CH12   					ADC_CH_12		//ADC1_IN12 ==> PB2  NTC采样
#define PFC_I_SAMPLE_ADC1_CH16   					ADC_CH_16		//ADC1_IN16 ==> PA4	PFC电流采样


/*--------------------ADC VREF Define--------------------*/
#define	SAMP_RATIO												(float)(2.9/8192)		//采样比例

#define VAC_DIFF  												(float)(10.0/1420)		//	R32/(R32+R218+R219+R222) //电网电压
#define VAC_SAMP_RATIO										(SAMP_RATIO/VAC_DIFF)



#define	VIN_VOLTAGE				  							220.0f

// <o>pfc输入欠压电压
//  <i>范围 0~300V
//  <0-300>
#define	VAC_IN_UNDER_VOLTAGE								145.0f

// <o>pfc最小输入电压(欠压恢复)
//  <i>范围 0~300V
//  <0-300>
#define	VAC_IN_ON_VOLTAGE		  							155.0f

// <o>pfc输入过压电压
//  <i>范围 0~300V
//  <0-300>
#define	VAC_IN_OVER_VOLTAGE 	  						278.0f //120.0//

// <o>pfc输入最大电压(过压恢复)
//  <i>范围 0~300V
//  <0-300>
#define	VAC_IN_MAX_VOLTAGE 	 								272.0f

#define	VDC_IN_UNDER_VOLTAGE								(170.0f/1.414f) //70.0//
#define	VDC_IN_ON_VOLTAGE		  							(176.0f/1.414f) //76.0//
#define	VDC_IN_OVER_VOLTAGE 	  						(280.0f) //120.0//
#define	VDC_IN_MAX_VOLTAGE 	 								(275.0f)

#define	SQUARE_ROOT_2											1.4142f

#define VAC_FULL_RANGE 										426.0f
#define	VAC_FR_SQ													(VAC_FULL_RANGE * VAC_FULL_RANGE)
#define VIN_LP_ADJ_SQ1										(VIN_LP_ADJ_VOLTAGE1 * VIN_LP_ADJ_VOLTAGE1)
#define VAC_MIN_ON_SQ               			(VIN_ON_VOLTAGE * VIN_ON_VOLTAGE)
#define VAC_MIN_OFF_SQ              			(VIN_UNDER_VOLTAGE * VIN_UNDER_VOLTAGE)
#define	VAC_MAX_ON_SQ 										(VIN_MAX_VOLTAGE * VIN_MAX_VOLTAGE)
#define	VAC_MAX_OFF_SQ 										(VIN_OVER_VOLTAGE * VIN_OVER_VOLTAGE)

#define VAC_MIN_OFF_SQ_AVG          			((VAC_MIN_OFF_SQ/VAC_FR_SQ) * 32767)
#define VAC_MIN_ON_SQ_AVG           			((VAC_MIN_ON_SQ/VAC_FR_SQ) * 32767)
#define	VAC_MAX_ON_SQ_AVG									((VAC_MAX_ON_SQ / VAC_FR_SQ) * 32767)
#define	VAC_MAX_OFF_SQ_AVG								((VAC_MAX_OFF_SQ / VAC_FR_SQ) * 32767)

#define VIN_SAMP_POSITIVE_VOLTAGE					(VAC_DIFF*VIN_VOLTAGE*SQUARE_ROOT_2)				//	正半周输入电压标准值 220V->311V->2.5263 V		ADC采样电压
#define VIN_SAMP_NEGATIVE_VOLTAGE					(VAC_DIFF*VIN_VOLTAGE*SQUARE_ROOT_2)				//	负半周输入电压标准值 220V->311V->0.4734 V		ADC采样电压

#define	VIN_SAMP_UNDER_VOLTAGE					  ((VAC_DIFF*VIN_UNDER_VOLTAGE)*8192/2.9)
#define	VIN_SAMP_ON_VOLTAGE					  		((VAC_DIFF*VIN_ON_VOLTAGE)*8192/2.9)
#define	VIN_SAMP_MAX_VOLTAGE					  	((VAC_DIFF*VIN_MAX_VOLTAGE)*8192/2.9)
#define	VIN_SAMP_OVER_VOLTAGE					 		((VAC_DIFF*VIN_OVER_VOLTAGE)*8192/2.9)



// <o>最大功率（W） 
//<i>范围 0~3500W
//  <0-3500>
#define POWER_MAX							1000


#define VPFC_AMP							((float)0.006412515)//((float)(9.1/(470*3+9.1)))																// 	(R33/(R220+R221+R226+R33))

#define PFC_BUS_SAMP_RATIO		(float)(SAMP_RATIO/VPFC_AMP)

// <o>输出目标电压（V）
//<i>范围 0~500V
//  <0-500>
#define	VOUT_VOLTAGE  				415	//414 //200//

// <o>pfc输出欠压电压
//  <i>范围 0~500V
//  <0-500>
#define	VOUT_UNDER_VOLTAGE		360 //160//

// <o>pfc过压电压
//  <i>范围 0~500V
//  <0-500>
#define VOUT_OVER_VOLTAGE     445 //220//

// <o>pfc过压恢复电压
//  <i>范围 0~500V
//  <0-500>
#define	VOUT_RECOVER_VOLTAGE 	355

#define VLOOP_ERROR_TH1_VOLTAGE		30




#define VPFC_OUT_VOLTAGE					 	VPFC_AMP*VOUT_VOLTAGE
#define VPFC_SAMP_UNDER_VOLTAGE		 	VPFC_AMP*VOUT_UNDER_VOLTAGE
#define VPFC_SAMP_OVER_VOLTAGE			VPFC_AMP*VOUT_OVER_VOLTAGE
#define VPFC_SAMP_RECOVER_VOLTAGE		VPFC_AMP*VOUT_RECOVER_VOLTAGE
#define	VLOOP_SAMP_ERROR_TH1_VOLTAGE	VPFC_AMP*VLOOP_ERROR_TH1_VOLTAGE


#define VPFC_OUT_VOLTAGE_VALUE			(VPFC_OUT_VOLTAGE*8192/2.9)				//	PFC输出电压采样值			400V->5789 
#define VPFC_SAMP_UNDER_VALUE				(VPFC_SAMP_UNDER_VOLTAGE*8192/2.9)
#define VPFC_SAMP_OVER_VALUE				(VPFC_SAMP_OVER_VOLTAGE*8192/2.9)
#define VPFC_SAMP_RECOVER_VALUE			(VPFC_SAMP_RECOVER_VOLTAGE*8192/2.9)

#define VLOOP_ERROR_TH1_VALUE				(VLOOP_SAMP_ERROR_TH1_VOLTAGE*8192/2.9)

// <o>最大电流（A）
//<i>范围 0~20A
//  <0-20>
#define IPFC_MAX_CURRENT     			((float)2.8)
#define IPFC_OCP_CURRENT     			((float)13)
#define IPFC_OCP_REC_CURRENT     	((float)11)
#define IPFC_OFFSET_VAL           ((float)0.245)
#define IPFC_SCALE_VAL 						((float)0.1785)

#define CURRENT_OFFSET_AD_VALUE		(IPFC_OFFSET_VAL*8192/2.9)

#define VMAX_FOR_IMAX							(POWER_MAX/IPFC_MAX_CURRENT)	//PFC最大电流时对应的电压
#define	LIMIT_K										 VMAX_FOR_IMAX								//用于限制最大功率

#define NEGATIVE 0
#define POSITIVE 1

#define PFC_I_OCP_VALUE     						((IPFC_OCP_CURRENT*IPFC_SCALE_VAL + IPFC_OFFSET_VAL)*8192/2.9)//2048
#define PFC_I_OCP_REC_VALUE     				((IPFC_OCP_REC_CURRENT*IPFC_SCALE_VAL + IPFC_OFFSET_VAL)*8192/2.9)
/*=============================ADC=============================*/
/*=============================DAC=============================*/

// <o>PFC母线电压过压保护值
//<i>范围 0~4096	对应0~3.3V
//  <0-4096>
#define BUS_OVP_POINT     						 	440																	//420(372)  245 (217)
#define	BUS_OVP_DAC_VALUE								((BUS_OVP_POINT*VPFC_AMP)*4096/2.9) //4050//(410) //

// <o>PFC过流保护值
//<i>范围 0~4096	对应0~3.3V
//  <0-4096>
#define PFC_I_OCP_DAC_POINT							((float)(IPFC_OCP_CURRENT+1))
#define	PFC_I_OCP_DAC_VALUE							((PFC_I_OCP_DAC_POINT*IPFC_SCALE_VAL + IPFC_OFFSET_VAL)*4096/2.9)

/*=============================DAC=============================*/

/*=============================GPIO=============================*/

/*=============================GPIO=============================*/

/*=============================VLOOP=============================*/
#define VLOOP_OUTPUT_MAX					(int32_t)(1600)  // 1000/176
#define VLOOP_OUTPUT_MIN					(int32_t)(0)
#define VLOOP_INTEGRA_MAX					(1<<16)
#define VLOOP_INTEGRA_MIN					(-(1<<16))
/*=============================VLOOP=============================*/


/*=============================ILOOP=============================*/
#define ILOOP_INPUT_MAX						(int32_t)(15)  //(8192)
#define ILOOP_INPUT_MIN						(int32_t)(0)

#define	ILOOP_OUTPUT_MAX_LIMIT		(0.2f)
#define ILOOP_OUTPUT_MIN_LIMIT		(-0.82f)
#define ILOOP_INTEGRA_MAX_FORWD		(0.2f)
#define ILOOP_INTEGRA_MIN_FORWD		(-0.82f)
#define ILOOP_INTEGRA_MAX					(1<<8)
#define ILOOP_INTEGRA_MIN					(-(1<<8))
#define	ILOOP_OUTPUT_MAX					(1.0f)
#define	ILOOP_OUTPUT_MIN					(0.0f)
/*=============================ILOOP=============================*/


/*=============================STRUCT=============================*/
typedef enum
{
  NO_ZERO_POINT = 0x00U,      /*!< Fault channel 0 identifier */
  POSI_TO_NEGA  = 0x01U,      /*!< Fault channel 1 identifier */
  NEGA_TO_POSI  = 0x02U,      /*!< Fault channel 2 identifier */

} ZERO_POINT_CHECK;

typedef enum
{
  UNKNOW_MODE = 0,
  DC_MODE = 0x01U,      /*!< DC */
  AC_MODE  = 0x02U,      /*!< AC */

} INPUT_MODE;

typedef enum
{
  State_Idel = 0,		//待机状态（初始）
  State_Rampup	,		//启动状态
  State_On			,		//运行状态
  State_Fault		,		//故障状态

} STATE;

typedef enum
{
  I_STATE_1 = 0,
  I_STATE_2,
  I_STATE_3,
  I_STATE_4,
  I_STATE_5,
} INTERRUPT_STATE;

typedef enum
{
  UART_FUNC = 0,
  VOFA_FUNC = 1,
} UART_FUNC_TYPE;
typedef struct
{
  uint16_t vin_squares_average;
  int16_t		temp_pri_ad	;
  bool Fault_Flag;

} PFC_TXDATA_TypeDef ;

typedef struct
{
  int32_t vout;
  int32_t iout;
  int32_t pout;
  bool Fault_Flag;

} PFC_RXDATA_TypeDef ;


typedef struct
{
  float data_error;
  float data_error_n1;
  float rel;
  float ref;
  float kp;
  float ki;
  float k1;
  float k2;
  float error;
  float integral;
  float	integral_max;
  float	integral_min;
  float	out_max;
  float loop_out;
  float	out_min;
  int32_t integral_storage;
  uint8_t integral_en_k;
  bool integral_inside_flag;
  bool out_inside_flag;
  bool twopart_no_same_flag;
  float p_out;
  float out_max_limit;
  float out_min_limit;
} PFC_PID_TypeDef ;

typedef struct
{
  volatile	uint32_t	tik;
  volatile	uint32_t	tok;
  volatile	float			cycle_100k;
  volatile	float			cycle_10k;
  volatile	float			cycle3;
  volatile	uint8_t		test_1;

} TIME_ACCESS_TypeDef;

typedef struct
{
  float 	acl_value				;
  float		acn_value				;
  float		i_surge_value		;
  float		vbus_value			;
  float 	i_ocp_value			;
  float		i_samp_check		;
  float		r_ntc_value			;
  float		i_value					;
  float   vbus_samp;
  float   vin_voltage;

  float ac_voltage;					//用于正负半周环路运算
  float vin_squared;		      //输入电压平方
  float ac_voltage_avg;			//用于Cal the vin ac avg
  float ac_voltage_rms;		  //输入电压平方

  float rated_power;					//定功率


  bool positive_period;						//正周期
  uint32_t positive_cnt;		//正周期计数
  float positive_squares_sum;		//正半周期平方和
  float positive_average_sum;
  uint32_t		ac_drop_count;
  bool ac_drop;

  bool negative_period;						//负周期
  uint32_t negative_cnt;		//负周期计数
  float negative_squares_sum;		//负半周期平方和
  float	negative_average_sum;

  float  vin_squares_average;//平方和的平均值
  float  vin_squares_average_delta;
  float	vin_squared_slow_average;
  float vin_squared_slow_average_filterd;
  float	vin_squared_for_ac_drop;
  float	vin_squares_average_2;//平方和的平均值的平方
  bool last_cycle;					//上一周期
  ZERO_POINT_CHECK zero_point;		//过零点

  INPUT_MODE	input_mode;				//输入模式
  uint32_t ac_check_cnt;				//输入模式判断计数
  uint32_t check_input_mode_cnt;	//输入AC/DC计数
  bool input_check_is_ok;				//输入模式判断完成标志位
  bool set_protect_is_ok; 			//保护点设置完成标志位


  float  vbus_value_store[4];
  float vbus_value_store_sum;
  float vbus_filter_value;
  float vbus_filter_value_store[4];
  float vbus_filter_value_store_sum;
  uint8_t vbus_vol_cnt;

  float vbus_move_filter_sum;
  float vbus_move_filter_store[500];
  float vbus_move_filter_value;
  uint32_t vbus_move_filter_cnt;
  float vbus_value_error_th;

  float vbus_value_old;
  float vbus_value_new;
  float vbus_value_error;

  bool switch_vbus_flag;
  uint32_t switch_vbus_cnt;

  float  ibus_value_store[4];
  float ibus_value_store_sum;
  uint8_t  ibus_curr_cnt;
  bool check_current_is_ok;
  uint32_t check_current_data_sum;
  uint16_t check_current_cnt;
  uint16_t check_current_data;

  float 	vbus_target;						//目标电压
  float 	vbus_rel;								//当前瞬时电压
  float 	vbus_ref;								//自加电压值
  float 	vout_rel_value;
  float	vdc_LPF;
  float  vdc_filtered;
  float 	vbus_notch_filer_data;
  float  vdc_bus_square;
  float	vbus_cal_avg;
  float	vbus_target_offset;

  float 	ac_current_ref;					//电流参考(来自电压环输出)
//		uint32_t 	ac_current_ref1;					//电流参考(来自电压环输出)
//		uint32_t 	ac_current_ref2;					//电流参考(来自电压环输出)
//		uint32_t 	ac_current_refdelta;
//		uint32_t 	ac_current_refdelta1;
//		uint32_t 	ac_current_deltadelta;



  float 	ibus_target;						//目标电流
  float 	ibus_target_average;		//平均电流
  float 	ibus_samp_rel;					//当前瞬时电流
  float	ibus_target_offset;
  float  vff_multiplier;
  float  ibus_LPF;
  float  ibus_filtered;
  float	ibus_cal_avg;
  float  temp_filtered;

  float	vac_voltage;
  float 	vin_rel;
  float 	vin_sum;
  float 	vin_filtered;
  float  vin_filtered_last;
  float  vin_filtered1;
  float  vin_filtered_max;

  float	move_vin_sum;
  float	move_calcu_vin[200];
  float	move_vin_average;

  float	k_forward;

  uint8_t fault_flag;
  bool pwm_sw;
  uint32_t duty;
  int swap_cnt;
  uint32_t swap_death_cnt;
  int rep_cnt;
  PFC_PID_TypeDef iloop;
  PFC_PID_TypeDef vloop;

  int num_ten, num_bit, num_cnt;
  int asci_num;
  volatile STATE state;
  volatile INTERRUPT_STATE interrupt_state;


  float vin_on_voltage;
  float vin_under_voltage;
  float vin_max_voltage;
  float vin_over_voltage;

  bool is_no_fault;
  bool idel_is_ok ;
  bool rampup_is_ok ;
  bool on_is_ok ;
  bool is_ac_ok;
  bool is_vbus_ok;
  bool start_cnt_flg;
  bool pre_finish_flg;
//		bool is_ibus_ok;
//		bool is_ac_ov;
  bool under_input_flag;
  bool over_input_flag;
  bool V_over_output_flag;
  bool I_over_output_flag;
  bool Fault_Flag;
  int32_t no_fault_cnt;
  int32_t fault_cnt;
  int32_t under_input_cnt;
  int32_t over_input_cnt;
  int32_t V_over_output_cnt;
  int32_t I_over_output_cnt;
  int32_t V_under_output_cnt;
  uint32_t over_input_rec_cnt;
  uint32_t dcdc_on_cnt;
  uint32_t ramp_cnt;
  uint32_t vin_step_cnt;
  uint32_t led_cnt;

  float debug1;
  uint32_t fault_num;
  uint32_t close_pwm_flag;

  float current_output;

  uint32_t test_samp_cnt;
  uint32_t test_tmr_cnt;

  TIME_ACCESS_TypeDef	time;

  uint32_t pre_driver_period;
  uint32_t pre_driver_cmpb;
	UART_FUNC_TYPE uart_func;
	bool uart_func_is_check;

} pfcTypeDef;
// <o.0..2>´®¿Ú¹¦ÄÜÑ¡Ôñ
//  <0=>VOF+
//  <1=>Ô­¸±±ßÍ¨Ñ¶
#define UART_FUNC 1


extern 	PFC_HRPWM_TypeDef mpwm, pwm0, pwm1,  pwm2, pwm3, pwm4, pwm5;
extern 	int16_t acl_samp     						[BUF_LEN];
extern 	int16_t acn_samp 								[BUF_LEN];
extern 	int16_t i_surge_samp						[BUF_LEN];
extern 	int16_t i_cr_samp								[BUF_LEN];
extern 	int16_t vbus_samp								[BUF_LEN];
extern 	int16_t i_ocp_samp							[BUF_LEN];
extern 	int16_t r_ntc_samp							[BUF_LEN];
extern 	int16_t i_samp									[BUF_LEN];
extern	int16_t vbus_notch_out;
extern	int16_t vbus_notch_in;
extern  uint32_t fre_cnt;
extern  uint32_t irq_cnt;
extern  uint32_t start_cnt;
extern  uint32_t led_cnt;
extern 	int16_t spll_iir_indata;
extern 	volatile PFC_TXDATA_TypeDef txdata_buf;
extern 	volatile PFC_RXDATA_TypeDef rxdata_buf;
extern 	volatile pfcTypeDef pfc;
extern 	volatile uint8_t TXBuffer[TXBUFFER_LEN], RXBuffer[RXBUFFER_LEN];
extern  int16_t cosdata[LEN_COSSIN_BUF], sindata[LEN_COSSIN_BUF];

extern 	uint32_t pfc_vloop_kp1;
extern 	uint32_t pfc_vloop_ki1;
extern	bool pfc_current_integral_keep_en;
extern	bool pfc_current_integral_add_en;
extern float frequency;
extern float magnitude;
extern int goertzel_calcul(void) ;
extern void collectData(uint32_t newSample);

#endif

//<<< end of configuration section >>>
