#ifndef __VARIABLES_DEFINE_APP_H__
#define __VARIABLES_DEFINE_APP_H__
#include "main.h"
//<<< Use Configuration Wizard in Context Menu >>>


#define TEST_MODE							0
#define PWM_MODE							1
#define	OPEN_LOOP							0

#define IOUT_RATIO_82k				1


#define	START_DELAY_VALUE			128
#define IDEL_DELAY_VALUE			10000
#define SOFTSTART_DELAY_VALUE	400.0 //ms
#define	SOFTSTART_ONE_STEP_VALUE		(float)(VOUT_VOLTAGE_VALUE/(SOFTSTART_DELAY_VALUE/0.1))

//寄存器基地址
 #define DWT_CR *(uint32_t*)0xE0001000 //DWT 基地址
 #define DWT_CYCCNT *(uint32_t*)0xE0001004 //CYCNT 计数器地址
 #define DEM_CR *(uint32_t*)0xE000EDFC //调试控制器地址(上表的地址)
 #define DEM_CR_TRCENA (1<<24) //定义使能跟踪位
 #define DWT_CR_CYCCNTENA (1<<0) //定义使能 CYCCNT 位

/*================================================HRPWM================================================*/
typedef struct
{
    uint32_t period;
    uint32_t compa;
    uint32_t compb;
    uint32_t compc;
    uint32_t compd;
    uint32_t outb_cmp;

} LLC_HRPWM_TypeDef;


#define HRPWM_BASIC_FREQ            (float)180000            // KHZ
#define HRPWM_DIV_X                 (float)16
#define HRPWM_FINAL_FREQ            (float)HRPWM_BASIC_FREQ * HRPWM_DIV_X // = 2,880,000 KHZ
#define HRPWM_ONE_STEP_TIME     		((float)(1000000/(HRPWM_BASIC_FREQ*HRPWM_DIV_X)))  // = 0.347222 ns

#define LLC_SW_FREQ_MAX                    (350) // 350k
#define LLC_SW_PERIOD_MIN                (uint32_t)(HRPWM_FINAL_FREQ/LLC_SW_FREQ_MAX)// = 8228

#define LLC_SW_FREQ_PFM_MODE            (300) // 300k
#define LLC_SW_PERIOD_PFM_MODE        (uint32_t)(HRPWM_FINAL_FREQ/LLC_SW_FREQ_PFM_MODE)// = 9600

#define LLC_SW_FREQ_PWM_MODE            (300) // 300k
#define LLC_SW_PERIOD_PWM_MODE        (uint32_t)(HRPWM_FINAL_FREQ/LLC_SW_FREQ_PWM_MODE)// = 9600

#define LLC_SW_FREQ_BURST                (300) // 300k
#define LLC_SW_PERIOD_BURST            (uint32_t)(HRPWM_FINAL_FREQ/LLC_SW_FREQ_BURST)// = 9600

#define LLC_SW_FREQ_FR                    (270)    // 270k
#define LLC_SW_PERIOD_FR                (uint32_t)(HRPWM_FINAL_FREQ/LLC_SW_FREQ_FR)// = 10666

#define LLC_SW_FREQ_MIN                    (210) // 210k
#define LLC_SW_PERIOD_MAX                (uint32_t)(HRPWM_FINAL_FREQ/LLC_SW_FREQ_MIN)// = 13714

#define LLC_DEADTIME                        (float)(300)    // 350ns
#define LLC_DEADTIME_COUNT            ((uint32_t)(LLC_DEADTIME/HRPWM_ONE_STEP_TIME))// = 1008

#define LLC_DEADTIME_DUTY                (float)(250)    // 150ns
#define LLC_DEADTIME_DUTY_COUNT    ((uint32_t)(LLC_DEADTIME_DUTY/HRPWM_ONE_STEP_TIME))// = 432

#define LLC_DRIVER_DEADTIME            (float)    (100)
#define LLC_DRIVER_DEADTIME_COUNT    ((uint32_t)(LLC_DRIVER_DEADTIME/HRPWM_ONE_STEP_TIME))// = 288

#define LLC_DUTY_MIN_RAMPUP                (0.2f) // 20%
#define LLC_DUTY_MIN_RAMPUP_COUNT    (LLC_DUTY_MIN_RAMPUP * LLC_SW_PERIOD_PWM_MODE) // 0.2 * 9600 = 1920

#define LLC_DUTY_MIN_COUNT_RAMPUP            ((LLC_DEADTIME_COUNT+LLC_DUTY_MIN_RAMPUP_COUNT)*2) // (1008+1920)*2 = 5856

#define LLC_DUTY_MIN_COUNT            ((LLC_DEADTIME_COUNT+1500)*2) // (1008+1500)*2 = 5016
#define LLC_DUTY_MAX_COUNT            ((LLC_SW_PERIOD_MIN/2)-2*LLC_DEADTIME_COUNT) // (4114-2016) = 2098

#define LLC_DUTY_BURST_COUNT            ((LLC_SW_PERIOD_MIN/2)-LLC_DEADTIME_COUNT) // (4114-1008) = 3106

#define LLC_BURST_DUTY_MAX_COUNT    ((LLC_DEADTIME_COUNT+100)*2) // (1008+100)*2 = 2216
#define LLC_BURST_DUTY_COUNT        ((LLC_SW_PERIOD_MIN/2 - 2*LLC_DEADTIME_COUNT))  // (4114-2016) = 2098


/*================================================HRPWM================================================*/





/*================================================ ADC ================================================*/
#define BUF_LEN 1
/*============================ ADC0 ============================*/
#define S_TRIM_SAMPLE_CH2         ADC_CH_2      		//LLC输出电压远端补偿信号 	PA1 ADC0_IN2
#define VOUT_SAMP_CH4							ADC_CH_4					//输出电压采样							PA3 ADC0_IN4,CMP1_INP1
/*============================ ADC1 ============================*/
#define	IOUT_SAMPLE_CH3						ADC_CH_3					//输出电流采样							PA6 ADC1_IN3
#define LOADSHARE_SAMPLE_CH12     ADC_CH_12     	  //系统均流总线电压采样			PB2 ADC1_IN12
/*============================ ADC2 ============================*/
#define SR_TEMP_SAMPLE_CH1        ADC_CH_1     	 		//SR_OTP采样									PB1 ADC2_IN1


extern int16_t temp_samp				[BUF_LEN];
extern int16_t s_trim_samp			[BUF_LEN];
extern int16_t vout_samp				[BUF_LEN];
extern int16_t vfb_samp    			[BUF_LEN];
extern int16_t loadshare_samp		[BUF_LEN];
extern int16_t iout_samp				[BUF_LEN];
extern int16_t can_addr1_samp		[BUF_LEN];
extern int16_t can_addr2_samp		[BUF_LEN];
extern int16_t can_addr3_samp		[BUF_LEN];
extern int16_t sr_temp			 		[BUF_LEN];

extern	uint16_t loadshare_filter_cnt ;
extern	bool loadshare_filter_comp ;
extern  uint16_t loadshare_filter;
#define VOUT_FILTER_ORDER				1//2^2
#define IOUT_FILTER_ORDER				1//2^2
#define TEMP_FILTER_CNT					16//2^44
#define TRIM_FILTER_CNT					128//2^6
#define LOADSHARE_FILTER_CNT		4//2^15
#define ADDR1_FILTER_CNT				4
#define IOUT_SLOW_FILTER_CNT		16
#define VBUS_SLOW_FILTER_CNT		8
/*--------------------ADC VREF Define--------------------*/
#define	SAMP_RATIO												(float)(2.9/8192)		//采样比例

#define	RATED_POUT_WATT				((float)(600)) //额定输出功率
#define	MAX_POUT_WATT					((float)(800)) //最大输出功率	
#define	SAFE_POUT_WATT				((float)(500)) //降额输出功率
	

// <o>输出目标
//  <i>范围 10V~14V
//  <10-14>
#define VOUT_VOLTAGE					(float)12.0f
#define VOUT_UP_VOLTAGE				(float)12.5f
#define VOUT_DOWM_VOLTAGE			(float)11.5f
// <o>输出欠压(软件保护)
//  <i>范围 9V~11V
//  <9-11>
#define VOUT_UNDER_VOLTAGE			9.0f//10

// <o>输出过压(软件保护)
//  <i>范围 13V~16V
//  <13-16>
#define VOUT_OVER_VOLTAGE				14.0f


#define VOUT_GAIN								((float)0.16393)
#define	VOUT_SAMP_RATIO					(SAMP_RATIO/VOUT_GAIN)

#define VOUT_VOLTAGE_SAMP				(VOUT_VOLTAGE*VOUT_GAIN)
#define VOUT_UNDER_VOLTAGE_SAMP	(VOUT_UNDER_VOLTAGE*VOUT_GAIN)
#define VOUT_OVER_VOLTAGE_SAMP	(VOUT_OVER_VOLTAGE*VOUT_GAIN)

#define VOUT_VOLTAGE_VALUE			(VOUT_VOLTAGE_SAMP*8192/2.9)
#define VOUT_UNDER_VALUE				(VOUT_UNDER_VOLTAGE_SAMP*8192/2.9)
#define VOUT_OVER_VALUE					(VOUT_OVER_VOLTAGE_SAMP*8192/2.9)

#define IOUT_TARGET_CURRENT			(float)(55.0f)
#define IOUT_REC_TARGET_CURRENT	(float)(52.0)

#define IOUT_MAX_CURRENT				(float)(50.0f)
#define IOUT_OCP_CURRENT				(float)(60.0f)
#define IOUT_REC_CURRENT				(float)(58.0f)
#define IOUT_SHORT_CURRENT			(float)(65.0f)


#define IOUT_SR_ON_CURRENT			(float)(10.0f)
#define IOUT_SR_OFF_CURRENT			(float)(5.0f)

#define IOUT_GAIN_VAL						((float)0.031)  
#define IOUT_OFFSET_VAL					((float)0.291)	

//――――――――――――――――――――――――――――V01――――――――――――――――――――――――――――//

#define	IOUT_OFFSET_VAL_CHECK		(float)(1.079)
#define IOUT_GAIN_VAL_CHECK			(float)(0.8734)

//――――――――――――――――――――――――――――V01――――――――――――――――――――――――――――//

// 采样系统转换系数
#define IOUT_SAMPLING_COEFF_A		(IOUT_GAIN_VAL * 8192 / 2.9)
#define IOUT_SAMPLING_COEFF_B		(IOUT_OFFSET_VAL * 8192 / 2.9)

#define	IOUT_GAIN_VAL_CHECK_VALUE		(int32_t)((IOUT_GAIN_VAL_CHECK*8192))
#define	IOUT_OFFSET_VAL_CHECK_VALUE	(int32_t)(IOUT_SAMPLING_COEFF_A * IOUT_OFFSET_VAL_CHECK + IOUT_SAMPLING_COEFF_B * (1.0f - IOUT_GAIN_VAL_CHECK))



#define IOUT_MAX_VALUE					((IOUT_MAX_CURRENT*IOUT_GAIN_VAL+IOUT_OFFSET_VAL)*8192/2.9)
#define IOUT_OCP_VALUE					((IOUT_OCP_CURRENT*IOUT_GAIN_VAL+IOUT_OFFSET_VAL)*8192/2.9)
#define IOUT_REC_VALUE					((IOUT_REC_CURRENT*IOUT_GAIN_VAL+IOUT_OFFSET_VAL)*8192/2.9)

#define TEMP_MAX_C							0.524
#define TEMP_REC_C							0.853

#define TEMP_MAX_VALUE					110
#define TEMP_REC_VALUE					60



/*================================================ ADC ================================================*/


/*================================================ DAC ================================================*/
// <o>输出过压(硬件保护)
//  <i>范围 13V~16V
//  <13-16>
#define VOUT_OVER_VOLTAGE_DAC			14.0
#define VOUT_OVER_VOLTAGE_DAC_VALUE				(VOUT_OVER_VOLTAGE_DAC*VOUT_GAIN*4096.0f/2.9f)

// <o>输出过流(硬件保护)
//  <i>范围 10A-90A
//  <10-90>
#define IOUT_OCP_CURRENT_DAC									(75.0)
#define IOUT_OCP_CURRENT_DAC_DAC_VALUE				((IOUT_OCP_CURRENT_DAC*IOUT_GAIN_VAL+IOUT_OFFSET_VAL)*4096/2.9)

#define IOUT_OCP_REC_CURRENT_DAC							(65.0)
#define IOUT_OCP_REC_CURRENT_DAC_DAC_VALUE		((IOUT_OCP_REC_CURRENT_DAC*IOUT_GAIN_VAL+IOUT_OFFSET_VAL)*4096/2.9)


/*================================================ DAC ================================================*/

/*================================================ CMP ================================================*/

#define		OVP_CMP		CMP1
#define		OCP_CMP		CMP2

/*================================================ CMP ================================================*/

///*================================================ CAN ================================================*/
//#define SET_FAULT_STATE(flag, can_flag) (can_flag = (flag) ? 1 : 0)
//typedef enum
//{
//    CMD_QUERY = 0x01,           // 查询命令
//    CMD_VERSION = 0x02,         // 版本查询命令
//	  CMD_STOP = 0x04,            // 关机命令
//    CMD_START = 0x05,           // 开机命令

//    CMD_STORE_FLASH = 0x11,     // 存储到FLASH
//    CMD_LOAD_FLASH = 0x12,      // 从FLASH加载
//    
//    CMD_TEMP_RECOVER_ON = 0x13, // 开启温度恢复模式
//    CMD_TEMP_RECOVER_OFF = 0x14, // 关闭温度恢复模式
//    
//    CMD_OVERTEMP_POINT = 0xF,   // 设置过温点
//    CMD_OVERTEMP_REC_POINT = 0x10, // 设置过温恢复点
//    
//    CMD_FACTOR_VOLTAGE = 0x22,  // 设置因子电压
//    CMD_THEOR_VOLTAGE = 0x23,   // 设置理论电压
//    CMD_VOLTAGE_CRC = 0x24,     // 电压CRC校验
//  
//		CMD_CURRENT_GAIN = 0x25,		// CAN通信电流校准增益
//		CMD_CURRENT_OFFSET =0x26,		// CAN通信电流校准偏置
//		CMD_CURRENT_CRC =0x27,			// CAN通信校准参数校验
//	
//    CMD_KP = 0x28,              // 设置KP
//    CMD_KI = 0x29,              // 设置KI
//    
//    CMD_TEST = 0x42,            // 测试命令
//    CMD_TEST2 = 0x43,           // 测试命令2

//    CMD_IAP = 0x50,             // IAP命令
//} CommandType;
///*--------------------	电源上报帧	--------------------*/
//#define RETURN_BIT_POWER						0x81	//byte1
//#define POWER_STATE_GOOD				0x00	//byte2 电源无故障
//#define POWER_STATE_FAULT				0x01	//byte2 电源故障


///*--------------------	电源上报帧	--------------------*/

///*--------------------	查询电源	--------------------*/
//#define POWER_CMD								0X01	//查询电源状态命令
//#define VISION_CMD							0X02	//查询电源版本命令
///*--------------------	查询电源	--------------------*/

///*--------------------	版本上报帧	--------------------*/
////版本查询命令字
//#define 	VERSION_CMD						0x82	//byte1

////组件类型
//#define 	COMPNET_TYPE					0x10	//byte2

////日期定义 20240220
//#define 	YEAR_NUM							24		//byte3
//#define 	MONTH_NUM							2	 		//byte4
//#define 	DAY_NUM								20 		//byte5

////版本号定义 v1.0.0
//#define 	MAJOR_VERSION					1
//#define 	MINOR_VERSION					0
//#define 	PATCH_VERSION					0
//#define   VERSION_CODE 					(100 * MAJOR_VERSION + 10 * MINOR_VERSION + PATCH_VERSION)	////byte6_low //byte7_high
///*--------------------	版本上报帧	--------------------*/
//#define 	HLD_YEAR_NUM							25		//byte3
//#define 	HLD_MONTH_NUM							07	 	//byte4
//#define 	HLD_DAY_NUM								07 		//byte5
//#define 	HLD_MAJOR_VERSION					1
//#define 	HLD_MINOR_VERSION					1
//#define 	HLD_PATCH_VERSION					22
//#define   HLD_VERSION_CODE 					(100 * HLD_MAJOR_VERSION + 10 * HLD_MINOR_VERSION + HLD_PATCH_VERSION)	////byte6_low //byte7_high


//typedef struct {
//    uint8_t Byte0; 						// 帧计数
//    uint8_t bit_inquire; 			// BIT控制命令
//    uint8_t Byte2; 						// 保留
//    uint8_t Byte3; 						// 保留
//    uint8_t Byte4; 						// 保留
//    uint8_t Byte5; 						// 保留
//    uint8_t Byte6; 						// 保留
//    uint8_t Byte7; 						// 保留
//} inquire_frame_TypeDef;	
//	
//typedef struct {	
//    union {
//        uint8_t power_state;  // 电源状态字节
//        struct {
//            uint8_t power_on_off : 1;     // Bit 0: 电源开关机状态 (1: 开机, 0: 关机)
//            uint8_t over_voltage : 1;     // Bit 1: 过压故障状态 (1: 过压, 0: 正常)
//            uint8_t under_voltage : 1;    // Bit 2: 欠压故障状态 (1: 欠压, 0: 正常)
//            uint8_t over_current : 1;     // Bit 3: 过流故障状态 (1: 过流, 0: 正常)
//            uint8_t over_temp : 1;        // Bit 4: 过温故障状态 (1: 过温, 0: 正常)
//            uint8_t comm_status : 1;      // Bit 5: 通讯状态 (1: 正常, 0: 故障)
//            uint8_t reserved : 1;         // Bit 6: 保留位
//            uint8_t reserved2 : 1;        // Bit 7: 保留位
//        };
//    };
//} power_state_TypeDef;
//typedef struct {	
//    uint8_t frame_count; 			// 帧计数
//    uint8_t return_bit; 	 		// BIT返回字
//    power_state_TypeDef power_state;           // 电源状态
//    uint8_t vol_low_bit; 			// 电压低8位(mv)
//    uint8_t vol_high_bit;			// 电压高8位(mv)
//    uint8_t cur_low_bit; 			// 电流低8位(0.1A)
//    uint8_t cur_high_bit; 		// 电流高8位(0.1A)
//    int8_t temp_bit; 					// 温度
//} power_upper_TypeDef;

//typedef struct {	
//		uint8_t frame_count; 			// 帧计数
//    uint8_t cmd; 	 		// BIT返回字
//    uint8_t power_state; 			// 电源状态
//    uint8_t load_current_low_bit; 			// 均流母线低8位(0.1A)
//    uint8_t load_current_high_bit;			// 均流母线高8位(0.1A)
//    uint8_t cur_low_bit; 			// 电流低8位(0.1A)
//    uint8_t cur_high_bit; 		// 电流高8位(0.1A)
//		uint8_t	reserve;
//	
//} share_power_TypeDef;

////typedef struct {
////    uint8_t frame_count; 			// 帧计数
////    uint8_t version_cmd; 			// 版本查询命令字
////    uint8_t Byte2; 						// 保留
////    uint8_t Byte3; 						// 保留
////    uint8_t Byte4; 						// 保留
////    uint8_t Byte5; 						// 保留
////    uint8_t Byte6; 						// 保留
////    uint8_t Byte7; 						// 保留
////} version_TypeDef;

//typedef struct {
//    uint8_t frame_count; 			// 帧计数
//    uint8_t version_cmd; 			// 版本查询命令字
//    uint8_t compnet_type; 		// 组件类型
//    uint8_t year; 						// 年
//    uint8_t month; 						// 月
//    uint8_t day; 							// 日
//    uint8_t version_low_bit;	// 保留
//    uint8_t version_high_bit; // 保留
//} version_TypeDef;

//typedef struct {
//		power_upper_TypeDef		power_para;
//		version_TypeDef				version_info;
//} can_data_TypeDef;

//typedef struct {
//		share_power_TypeDef		share_power;
//} can_share_power_TypeDef;


///*================================================ CAN ================================================*/

/*================================================ LLC ================================================*/
/*--------------------	LOOP_PARA	--------------------*/
#define VLOOP_KP							 6000
#define VLOOP_KI							 200

#define VOLT_LOOP_MIN          0
#define VOLT_LOOP_MAX          ((int32_t)(LLC_SW_PERIOD_MAX))

#define ILOOP_KP							 1000
#define ILOOP_KI							 10
#define CURR_LOOP_MIN          0
#define CURR_LOOP_MAX          ((int32_t)(LLC_SW_PERIOD_MAX))


#define	SHARE_OUT_MIN						(float)(-0.5)
#define	SHARE_OUT_MAX						(float)(0.5)

/*--------------------	LOOP_PARA	--------------------*/



typedef enum
{
    state_idel = 0,		//待机状态（初始）
    state_rampup	,		//启动状态
    state_on			,		//运行状态
    state_fault		,		//故障状态

} LLC_STATE_TypeDef;

struct FAULT_BIT_TypeDef
{
    bool over_voltage  ;		//过压
    bool under_voltage	;		//欠压
    bool over_current	;		//过流
    bool over_temp			;		//过温
		bool pfc_fault			;		//pfc故障
} ;


//struct FAULT_BIT
//{
//	union
//	{
//		uint32_t fault_all;
//		struct FAULT_BIT_TypeDef fault_bit;
//	}fault_union;
//};

 struct	send_state_TypeDef
{
		unsigned int loop_mode			: 	1		;	// 电压环 / 电流环
		unsigned int switch_mode		: 	2		;	// 打嗝 / 调频 / 调宽
    unsigned int llc_state			: 	2		;	// idel / rampup / on /fault
		unsigned int llc_drv_state	:		1		;	// driver 开/关
		unsigned int llc_sr_state		:		1		;	// sr driver 开/关
		unsigned int over_voltage		: 	1		;	
    unsigned int under_voltage	:		1		;
    unsigned int over_current		:		1		;
    unsigned int over_temp			:		1		;
    unsigned int pfc_fault 			: 	1		;
    unsigned int reserved				: 	4		;
};

 struct	send_power_TypeDef
{
		uint8_t output_target_voltage;	
		uint8_t output_rel_voltage;
		uint8_t output_rel_current;
};

struct send_massage_TypeDef
{
	struct send_state_TypeDef state;
	struct send_power_TypeDef power;
};


 struct	status_fault_bits
{
    unsigned int over_voltage		: 	1		;
    unsigned int under_voltage	:		1		;
    unsigned int over_current		:		1		;
    unsigned int over_temp			:		1		;
    unsigned int pfc_fault 			: 	1		;
    unsigned int reserved				: 	11	;
};
union FAULT_DEFINE {
	uint16_t                 all;
	struct status_fault_bits  bit;
};

typedef struct
{
    uint32_t over_voltage_cnt  ;		//过压计数
    uint32_t under_voltage_cnt	;		//欠压计数
    uint32_t over_current_cnt	;			//过流计数
    uint32_t over_temp_cnt			;		//过温计数
    uint32_t over_temp_rec_cnt			;		//过温计数	
} FAULT_CNT_TypeDef;

typedef struct
{
    float over_voltage_point  		;		//过压保护点
		float over_voltage_rec_point	;		//过压恢复点
    float under_voltage_point			;		//欠压保护点
    float under_voltage_rec_point	;		//欠压恢复点	
    float over_current_point			;		//过流保护点
    float over_current_rec_point	;		//过流恢复点	
	  float short_current_point			;		//短路保护点	
    float over_temp_point					;		//过温保护点
    float over_temp_rec_point			;		//过温恢复点	
} PROTECTION_POINT_TypeDef;

typedef struct
{
  float data_error;
  float rel;
  float ref;
  float kp;
  float ki;
  uint8_t data_move;
  float integral;
  float	integral_max;
  float	integral_min;
  float	out_max;
  float loop_out;
  float	out_min;
  float integral_storage;
  float integral_en_k;
  bool integral_inside_flag;
  bool out_inside_flag;
  bool twopart_no_same_flag;
  float p_out;
	uint32_t gain;
	float error_abs;
	
} LLC_PID_TypeDef ;

typedef enum
{
	voltage_mode = 0,
	current_mode = 1,
}OUTPUT_MODE_TypeDef;

typedef enum
{
	burst_mode = 0,
	burst_pfm_mode = 1,
	duty_pfm_mode = 2,
	pwm_mode = 3,
	pfm_mode = 4,
	max_pfm_mode = 5,
}SWITCH_MODE;

typedef enum
{
	trim_normal,
	trim_down = 1,
	trim_up = 2,
}TRIM_DIRECT_TypeDef;

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
//get_value and filter_value
float vout_value1;
float vout_value ;
float vout_value_store[4] ;
uint8_t  vout_filter_cnt	;
float iout_value ;
float iout_value_store[4] ;
float	can_iout_store_disp;
float	can_iout;
float can_iout_disp;	
uint8_t  iout_filter_cnt	;	
float temp_value;
float temp_value_store;
int8_t temp_c;
float low_temp_delta;
float sr_temp_value;
	
float s_trim_value;	
float s_trim_value_store;
float vout_trim_filter;
float vout_trim_filter_1;
float loadshare_value;
float loadshare_value_store;
float loadcurrent_value_store;
uint8_t  loadshare_filter_cnt;
float loadshare_avg_value;
float loadshare_move_value;
uint16_t addr1_value;
uint32_t addr1_value_store;

OUTPUT_MODE_TypeDef output_mode;
uint32_t current_mode_cnt;
uint32_t voltage_mode_cnt;

uint32_t short_current_cnt;
uint32_t short_current_flag;
bool current_loop_latch;
uint32_t ocp_time;
uint32_t ocp_burst_cnt;
uint16_t HwOcp;

bool llc_drv_state;
bool llc_sr_state;

//电压参数
float vbus_target;
float vbus_ref;
float vbus_rel;
float target_delta;
float target_test;
float target_check_delta;
float error_abs;
float vbus_rel_slow_store;
float vbus_rel_slow;
float factor_voltage;
float theor_voltage;
uint8_t voltage_crc;
uint8_t crc_data;
float coef_target;
float factor_voltage_float;
float theor_voltage_float;
float can_com_voltag_delta;
//电流参数
float ibus_target;
float ibus_ref;
float	ibus_rec_target;
float iout_rel;
float iout_value_1;
float iout_rel_slow_store;
float iout_rel_slow;
float can_com_current_gain;
float can_com_current_offset;
//均流环参数
float  delta_voltage;
uint16_t share_duty;
//state
LLC_STATE_TypeDef state;
PROTECTION_POINT_TypeDef protection_point;
FAULT_CNT_TypeDef fault_cnt;
union FAULT_DEFINE fault_state;
bool temp_recover_mode;

//struct send_state_TypeDef send_state;
//struct send_massage_TypeDef send_massage;
struct	send_massage_TypeDef	send_massage;
uint32_t state_on_cnt;
uint32_t state_fault_cnt;		//故障状态恢复计数
uint32_t fault_under_cnt;		//打嗝时间控制

//pid_para
LLC_PID_TypeDef vloop;
LLC_PID_TypeDef iloop;	
LLC_PID_TypeDef shareloop;	
float vloop_kp_init;
float vloop_ki_init;
float loop_out;
float loop_out_test;
uint32_t vloop_switch_cnt;
//pr_para
float pr_output;
uint32_t PR_output_cnt;

//pwm_para
uint32_t period;
uint32_t duty;
uint16_t burst_cnt;	

//SR
bool				sr_pwm;
uint8_t			fast_off_sr_cnt;
uint32_t 		sr_open_cnt;
uint32_t		sr_close_cnt;

//mode flag
SWITCH_MODE	switch_mode;

//led
uint32_t 		led_cnt;	//呼吸灯计数

//辅源ok延迟
uint8_t			start_delay_cnt;


//on_off_ctrl
uint8_t			on_off_ctrl_cnt;	//外部按键开关消抖计数
bool				on_off_ctrl_ok;	//外部按键开关
uint8_t			off_ctrl_cnt;
bool 				start_contr;

uint32_t		idel_delay_cnt;		//idle状态下延迟计数
bool				idel_delay_ok;		//idle状态延迟完成
uint32_t 		ramup_delay_cnt;

uint32_t		softstart_delay_cnt;		//idle状态下延迟计数
bool				softstart_delay_ok;		//idle状态延迟完成

//pfc_is_ok_cnt
uint32_t 		pfc_is_ok_cnt;		//pfc光耦信号
bool				pfc_is_ok;

uint32_t		ac_is_ok_cnt;			//AC_alarm
bool				ac_is_ok;

//protect/fault flag
volatile	uint32_t test_tx;		//vofa_tx 发送的故障标志位
volatile	uint32_t can_receive[2];
	uint32_t receive_cmd;
uint8_t can_buf[8];  

TIME_ACCESS_TypeDef	time;
volatile float pr_control_output;
	float vout_trim;
	float vout_trim_delta;
	
uint32_t	test_cnt;
uint16_t burst_cnt_temp;
uint8_t burst_period;
uint32_t fault1_cnt;
uint32_t freq_contr;
uint8_t div_freq_1k;
uint32_t	div_freq_1s;
float last_error_abs;
Can_LevelUnion can_addr_check;

uint32_t addr_cnt;
bool		addr_set_flag;

custom_can_data_TypeDef custom_can_data;
hld_can_data_TypeDef 		hld_can_data;
user_can_TypeDef        user_can;

float R_Out_ratio;
float R_Out_ratio_max;
uint16_t pr_open_cnt;
uint16_t pr_close_cnt;
uint8_t 	pid_state_flag;

uint32_t over_voltage_check_cnt;
bool 		 check_ovp_flag;
bool 	current_mode_flag;

float shareloop_kp_init;
float shareloop_ki_init;
float shareloop_out_max_init;

float test_temp;
float test_load;
float test_vout;

bool first_load;

TRIM_DIRECT_TypeDef trim_direct;

} llcTypeDef;

/*================================================ LLC ================================================*/



extern LLC_HRPWM_TypeDef mpwm, pwm0, pwm1,  pwm2, pwm3, pwm4, pwm5;
extern llcTypeDef llc;










#define UART_FUNC 1



//<<< end of configuration section >>>
#endif
