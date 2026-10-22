#ifndef __VARIABLES_DEFINE_APP_H__
#define __VARIABLES_DEFINE_APP_H__
#include "main.h"
//<<< Use Configuration Wizard in Context Menu >>>


#define TEST_MODE							0
#define PWM_MODE							1
#define	OPEN_LOOP							0

#define IOUT_RATIO_82k				1
#define	IOUT_RATIO_47k				0
#define IOUT_RATIO_91k				0
#define IOUT_RATIO_1k8				0


#define	START_DELAY_VALUE			128
#define IDEL_DELAY_VALUE			10000
#define SOFTSTART_DELAY_VALUE	400.0 //ms
#define	SOFTSTART_ONE_STEP_VALUE		(float)(VOUT_VOLTAGE_VALUE/(SOFTSTART_DELAY_VALUE/0.1))

//�Ĵ�������ַ
 #define DWT_CR *(uint32_t*)0xE0001000 //DWT ����ַ
 #define DWT_CYCCNT *(uint32_t*)0xE0001004 //CYCNT ��������ַ
 #define DEM_CR *(uint32_t*)0xE000EDFC //���Կ�������ַ(�ϱ��ĵ�ַ)
 #define DEM_CR_TRCENA (1<<24) //����ʹ�ܸ���λ
 #define DWT_CR_CYCCNTENA (1<<0) //����ʹ�� CYCCNT λ

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
#define HRPWM_ONE_STEP_TIME     ((float)(1000000/(HRPWM_BASIC_FREQ*HRPWM_DIV_X)))  // = 0.347222 ns

#define    LLC_SW_FREQ_MAX                    (350) // 350k
#define LLC_SW_PERIOD_MIN                (uint32_t)(HRPWM_FINAL_FREQ/LLC_SW_FREQ_MAX)// = 8228

#define    LLC_SW_FREQ_PFM_MODE            (300) // 300k
#define LLC_SW_PERIOD_PFM_MODE        (uint32_t)(HRPWM_FINAL_FREQ/LLC_SW_FREQ_PFM_MODE)// = 9600

#define    LLC_SW_FREQ_PWM_MODE            (300) // 300k
#define LLC_SW_PERIOD_PWM_MODE        (uint32_t)(HRPWM_FINAL_FREQ/LLC_SW_FREQ_PWM_MODE)// = 9600

#define    LLC_SW_FREQ_BURST                (300) // 300k
#define LLC_SW_PERIOD_BURST            (uint32_t)(HRPWM_FINAL_FREQ/LLC_SW_FREQ_BURST)// = 9600

#define    LLC_SW_FREQ_FR                    (270)    // 270k
#define LLC_SW_PERIOD_FR                (uint32_t)(HRPWM_FINAL_FREQ/LLC_SW_FREQ_FR)// = 10666

#define    LLC_SW_FREQ_MIN                    (210) // 210k
#define LLC_SW_PERIOD_MAX                (uint32_t)(HRPWM_FINAL_FREQ/LLC_SW_FREQ_MIN)// = 13714

#define LLC_DEADTIME                        (float)(350)    // 350ns
#define LLC_DEADTIME_COUNT            ((uint32_t)(LLC_DEADTIME/HRPWM_ONE_STEP_TIME))// = 1008

#define LLC_DEADTIME_DUTY                (float)(150)    // 150ns
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

#define IOUT_SAMPLE_CH1           ADC_CH_1   	  		//����������� 							PA0 ADC0_IN1 CMP2_INP0

#define VOUT_SAMP_CH4							ADC_CH_4					//�����ѹ����							PA3 ADC0_IN4,CMP1_INP1

/*============================ ADC1 ============================*/
#define S_TRIM_SAMPLE_CH2         ADC_CH_2      		//LLC�����ѹԶ�˲����ź� 	PA1 ADC1_IN2
#define LOADSHARE_SAMPLE_CH12      ADC_CH_12     	  	//ϵͳ�������ߵ�ѹ����		PB2 ADC1_IN12

/*============================ ADC2 ============================*/
#define SR_TEMP_SAMPLE_CH1        ADC_CH_1     	 		//ADDR2����									PB1 ADC2_IN1


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
#define VBUS_SLOW_FILTER_CNT		32
/*--------------------ADC VREF Define--------------------*/
#define	SAMP_RATIO												(float)(2.9/8192)		//��������


//���Ŀ��
#define VOUT_VOLTAGE					(float)48.0//47.6//48.1
	
// ���Ƿѹ(��������)
#define VOUT_UNDER_VOLTAGE			43.0//10

//�����ѹ(��������)
#define VOUT_OVER_VOLTAGE				57.6

#define VOUT_GAIN								((float)0.0480769)//0.0905
#define	VOUT_SAMP_RATIO					(SAMP_RATIO/VOUT_GAIN)

#define VOUT_VOLTAGE_SAMP				(VOUT_VOLTAGE*VOUT_GAIN)
#define VOUT_UNDER_VOLTAGE_SAMP	(VOUT_UNDER_VOLTAGE*VOUT_GAIN)
#define VOUT_OVER_VOLTAGE_SAMP	(VOUT_OVER_VOLTAGE*VOUT_GAIN)

#define VOUT_VOLTAGE_VALUE			(VOUT_VOLTAGE_SAMP*8192/2.9)
#define VOUT_UNDER_VALUE				(VOUT_UNDER_VOLTAGE_SAMP*8192/2.9)
#define VOUT_OVER_VALUE					(VOUT_OVER_VOLTAGE_SAMP*8192/2.9)

//#define IOUT_TARGET_CURRENT			(17.0)

//#define IOUT_MAX_CURRENT				(12.5)
//#define IOUT_OCP_CURRENT				(15.0)
//#define IOUT_REC_CURRENT				(14.0)
//#define IOUT_SHORT_CURRENT			(18.0)

#define IOUT_TARGET_CURRENT			(14.0f+3.0f)//104//12//86

#define IOUT_MAX_CURRENT				(12.0f+3.0f)//84//84
#define IOUT_OCP_CURRENT				(15.0f+3.0f)//104//15//90
#define IOUT_REC_CURRENT				(13.0f+3.0f)//13//88
#define IOUT_SHORT_CURRENT			(16.0f+3.0f)//105//18//92


#define IOUT_SR_ON_CURRENT			2.0
#define IOUT_SR_OFF_CURRENT			0.5

#define IOUT_GAIN_VAL						((float)0.1)  
#define IOUT_OFFSET_VAL					((float)0.438)	
	
//��������������������������������������������������������V03��������������������������������������������������������//
// 0��
//#define	IOUT_OFFSET_VAL_CHECK		(float)(0.11)
//#define IOUT_GAIN_VAL_CHECK			(float)(0.6613)

// 1��
//#define	IOUT_OFFSET_VAL_CHECK		(float)(0.0459)
//#define IOUT_GAIN_VAL_CHECK			(float)(0.6946)

// 2��
//#define	IOUT_OFFSET_VAL_CHECK		(float)(-0.19)
//#define IOUT_GAIN_VAL_CHECK			(float)(0.8545)
	
// 3��
//#define	IOUT_OFFSET_VAL_CHECK		(float)(-0.0468)
//#define IOUT_GAIN_VAL_CHECK			(float)(0.6873)
	
// 4��
//#define	IOUT_OFFSET_VAL_CHECK		(float)(-0.0346)
//#define IOUT_GAIN_VAL_CHECK			(float)(0.8341)

// 5��
//#define	IOUT_OFFSET_VAL_CHECK		(float)(-0.0336)
//#define IOUT_GAIN_VAL_CHECK			(float)(0.67)
//��������������������������������������������������������V03��������������������������������������������������������//


//��������������������������������������������������������V04��������������������������������������������������������//

#define	IOUT_OFFSET_VAL_CHECK		(float)(-0.4517)
#define IOUT_GAIN_VAL_CHECK			(float)(0.6978)


//��������������������������������������������������������V04��������������������������������������������������������//

// ����ϵͳת��ϵ��
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
// �����ѹ(Ӳ������)
#define VOUT_OVER_VOLTAGE_DAC			53.0
#define VOUT_OVER_VOLTAGE_DAC_VALUE				(VOUT_OVER_VOLTAGE_DAC*VOUT_GAIN*4096.0f/2.9f)

// �������(Ӳ������)
#define IOUT_OCP_CURRENT_DAC									(23.0)
#define IOUT_OCP_CURRENT_DAC_DAC_VALUE				((IOUT_OCP_CURRENT_DAC*IOUT_GAIN_VAL+IOUT_OFFSET_VAL)*4096/2.9)

#define IOUT_OCP_REC_CURRENT_DAC							(16.0)
#define IOUT_OCP_REC_CURRENT_DAC_DAC_VALUE		((IOUT_OCP_REC_CURRENT_DAC*IOUT_GAIN_VAL+IOUT_OFFSET_VAL)*4096/2.9)


/*================================================ DAC ================================================*/

/*================================================ CMP ================================================*/

#define		OVP_CMP		CMP1
#define		OCP_CMP		CMP2

/*================================================ CMP ================================================*/

/*================================================ CAN ================================================*/
#define SET_FAULT_STATE(flag, can_flag) (can_flag = (flag) ? 1 : 0)
typedef enum
{
    CMD_QUERY = 0x01,           // ��ѯ����
    CMD_VERSION = 0x02,         // �汾��ѯ����
	  CMD_STOP = 0x04,            // �ػ�����
    CMD_START = 0x05,           // ��������

    CMD_STORE_FLASH = 0x11,     // �洢��FLASH
    CMD_LOAD_FLASH = 0x12,      // ��FLASH����
    
    CMD_TEMP_RECOVER_ON = 0x13, // �����¶Ȼָ�ģʽ
    CMD_TEMP_RECOVER_OFF = 0x14, // �ر��¶Ȼָ�ģʽ
    
    CMD_OVERTEMP_POINT = 0xF,   // ���ù��µ�
    CMD_OVERTEMP_REC_POINT = 0x10, // ���ù��»ָ���
    
    CMD_FACTOR_VOLTAGE = 0x22,  // �������ӵ�ѹ
    CMD_THEOR_VOLTAGE = 0x23,   // �������۵�ѹ
    CMD_VOLTAGE_CRC = 0x24,     // ��ѹCRCУ��
  
		CMD_CURRENT_GAIN = 0x25,		// CANͨ�ŵ���У׼����
		CMD_CURRENT_OFFSET =0x26,		// CANͨ�ŵ���У׼ƫ��
		CMD_CURRENT_CRC =0x27,			// CANͨ��У׼����У��
	
    CMD_KP = 0x28,              // ����KP
    CMD_KI = 0x29,              // ����KI

    CMD_PFC_INPUT_OVP = 0x30,
    CMD_PFC_INPUT_UVP = 0x31,
    CMD_PFC_OUTPUT_OVP = 0x32,
    CMD_PFC_OUTPUT_UVP = 0x33,
    CMD_PFC_INPUT_OCP = 0x34,
    CMD_PFC_DATA = 0x35,
    CMD_PFC_DATA_LIVE1 = 0x36,
    CMD_PFC_DATA_LIVE2 = 0x37,

    CMD_LLC_TEMP_PROTECT = 0x3E,
    CMD_LLC_VOLTAGE_PROTECT = 0x3F,
    CMD_LLC_OCP_PROTECT = 0x40,
    CMD_LLC_OSP_PROTECT = 0x41,
    
    CMD_TEST = 0x42,            // ��������
    CMD_TEST2 = 0x43,           // ��������2
    CMD_LLC_OUT_PARA = 0x44,

    CMD_IAP = 0x50,             // IAP����
} CommandType;
/*--------------------	��Դ�ϱ�֡	--------------------*/
#define RETURN_BIT_POWER						0x81	//byte1
#define POWER_STATE_GOOD				0x00	//byte2 ��Դ�޹���
#define POWER_STATE_FAULT				0x01	//byte2 ��Դ����


/*--------------------	��Դ�ϱ�֡	--------------------*/

/*--------------------	��ѯ��Դ	--------------------*/
#define POWER_CMD								0X01	//��ѯ��Դ״̬����
#define VISION_CMD							0X02	//��ѯ��Դ�汾����
/*--------------------	��ѯ��Դ	--------------------*/

/*--------------------	�汾�ϱ�֡	--------------------*/
//�汾��ѯ������
#define 	VERSION_CMD						0x82	//byte1

//�������
#define 	COMPNET_TYPE					0x10	//byte2

//���ڶ��� 20240220
#define 	YEAR_NUM							26		//byte3
#define 	MONTH_NUM							07	 		//byte4
#define 	DAY_NUM								11 		//byte5

//�汾�Ŷ��� v1.0.0
#define 	MAJOR_VERSION					1
#define 	MINOR_VERSION					0
#define 	PATCH_VERSION					0
#define   VERSION_CODE 					(100 * MAJOR_VERSION + 10 * MINOR_VERSION + PATCH_VERSION)	////byte6_low //byte7_high
/*--------------------	�汾�ϱ�֡	--------------------*/
#define 	HLD_YEAR_NUM							26		//byte3
#define 	HLD_MONTH_NUM							07	 		//byte4
#define 	HLD_DAY_NUM								11 		//byte5
#define 	HLD_MAJOR_VERSION					1
#define 	HLD_MINOR_VERSION					1
#define 	HLD_PATCH_VERSION					22
#define   HLD_VERSION_CODE 					(100 * HLD_MAJOR_VERSION + 10 * HLD_MINOR_VERSION + HLD_PATCH_VERSION)	////byte6_low //byte7_high

#define RETURN_BIT_VOLTAGE_PROTECT 0x83
#define RETURN_BIT_OCP_PROTECT 0x84
#define RETURN_BIT_OSP_PROTECT 0x85
#define RETURN_BIT_VOLTAGE_PARA 0x86
#define RETURN_BIT_TEMP_PROTECT 0xBE

#define RETURN_BIT_PFC_INPUT_OVP  0x87
#define RETURN_BIT_PFC_INPUT_UVP  0x88
#define RETURN_BIT_PFC_OUTPUT_OVP 0x89
#define RETURN_BIT_PFC_OUTPUT_UVP 0x8A
#define RETURN_BIT_PFC_INPUT_OCP  0x8B
#define RETURN_BIT_PFC_DATA       0x8C
#define RETURN_BIT_PFC_LIVE1      0x8D
#define RETURN_BIT_PFC_LIVE2      0x8E


typedef struct {
    uint8_t Byte0; 						// ֡����
    uint8_t bit_inquire; 			// BIT��������
    uint8_t Byte2; 						// ����
    uint8_t Byte3; 						// ����
    uint8_t Byte4; 						// ����
    uint8_t Byte5; 						// ����
    uint8_t Byte6; 						// ����
    uint8_t Byte7; 						// ����
} inquire_frame_TypeDef;	
	
typedef struct {	
    union {
        uint8_t power_state;  // ��Դ״̬�ֽ�
        struct {
            uint8_t power_on_off : 1;     // Bit 0: ��Դ���ػ�״̬ (1: ����, 0: �ػ�)
            uint8_t over_voltage : 1;     // Bit 1: ��ѹ����״̬ (1: ��ѹ, 0: ����)
            uint8_t under_voltage : 1;    // Bit 2: Ƿѹ����״̬ (1: Ƿѹ, 0: ����)
            uint8_t over_current : 1;     // Bit 3: ��������״̬ (1: ����, 0: ����)
            uint8_t over_temp : 1;        // Bit 4: ���¹���״̬ (1: ����, 0: ����)
            uint8_t comm_status : 1;      // Bit 5: ͨѶ״̬ (1: ����, 0: ����)
            uint8_t reserved : 1;         // Bit 6: ����λ
            uint8_t reserved2 : 1;        // Bit 7: ����λ
        };
    };
} power_state_TypeDef;
typedef struct {	
    uint8_t frame_count; 			// ֡����
    uint8_t return_bit; 	 		// BIT������
    power_state_TypeDef power_state;           // ��Դ״̬
    uint8_t vol_low_bit; 			// ��ѹ��8λ(mv)
    uint8_t vol_high_bit;			// ��ѹ��8λ(mv)
    uint8_t cur_low_bit; 			// ������8λ(0.1A)
    uint8_t cur_high_bit; 		// ������8λ(0.1A)
    int8_t temp_bit; 					// �¶�
} power_upper_TypeDef;

typedef struct {
    uint8_t frame_count;
    uint8_t return_bit;
    uint8_t pfc_in_ovp_vol_low_bit;
    uint8_t pfc_in_ovp_vol_high_bit;
    uint8_t pfc_in_ovp_rec_vol_low_bit;
    uint8_t pfc_in_ovp_rec_vol_high_bit;
    uint8_t reserved0;
    uint8_t reserved1;
} pfc_input_ovp_TypeDef;

typedef struct {
    uint8_t frame_count;
    uint8_t return_bit;
    uint8_t pfc_in_uvp_vol_low_bit;
    uint8_t pfc_in_uvp_vol_high_bit;
    uint8_t pfc_in_uvp_rec_vol_low_bit;
    uint8_t pfc_in_uvp_rec_vol_high_bit;
    uint8_t reserved0;
    uint8_t reserved1;
} pfc_input_uvp_TypeDef;

typedef struct {
    uint8_t frame_count;
    uint8_t return_bit;
    uint8_t pfc_out_ovp_vol_low_bit;
    uint8_t pfc_out_ovp_vol_high_bit;
    uint8_t pfc_out_ovp_rec_vol_low_bit;
    uint8_t pfc_out_ovp_rec_vol_high_bit;
    uint8_t reserved0;
    uint8_t reserved1;
} pfc_output_ovp_TypeDef;

typedef struct {
    uint8_t frame_count;
    uint8_t return_bit;
    uint8_t pfc_out_uvp_vol_low_bit;
    uint8_t pfc_out_uvp_vol_high_bit;
    uint8_t pfc_out_uvp_rec_vol_low_bit;
    uint8_t pfc_out_uvp_rec_vol_high_bit;
    uint8_t reserved0;
    uint8_t reserved1;
} pfc_output_uvp_TypeDef;

typedef struct {
    uint8_t frame_count;
    uint8_t return_bit;
    uint8_t pfc_in_ocp_soft_low_bit;
    uint8_t pfc_in_ocp_soft_high_bit;
    uint8_t pfc_in_ocp_dac_low_bit;
    uint8_t pfc_in_ocp_dac_high_bit;
    uint8_t reserved0;
    uint8_t reserved1;
} pfc_input_ocp_TypeDef;

typedef struct {
    uint8_t frame_count;
    uint8_t return_bit;
    uint8_t pfc_vbus_target_low_bit;
    uint8_t pfc_vbus_target_high_bit;
    uint8_t pfc_vbus_ref_low_bit;
    uint8_t pfc_vbus_ref_high_bit;
    uint8_t pfc_vbus_rel_low_bit;
    uint8_t pfc_vbus_rel_high_bit;
} pfc_vbus_voltage_TypeDef;

typedef struct {
    uint8_t frame_count;
    uint8_t return_bit;
    uint8_t pfc_vin_rel_low_bit;
    uint8_t pfc_vin_rel_high_bit;
    uint8_t pfc_iloop_rel_low_bit;
    uint8_t pfc_iloop_rel_high_bit;
    uint8_t pfc_ntc_low_bit;
    uint8_t pfc_ntc_high_bit;
} pfc_live1_TypeDef;

typedef struct {
    uint8_t frame_count;
    uint8_t return_bit;
    uint8_t pfc_state;
    uint8_t pfc_freq_khz;
    uint8_t pfc_duty_low_bit;
    uint8_t pfc_duty_high_bit;
    uint8_t pfc_status_low_bit;
    uint8_t pfc_status_high_bit;
} pfc_live2_TypeDef;

typedef struct {
    uint8_t frame_count;
    uint8_t return_bit;
    uint8_t llc_out_ovp_soft_low_bit;
    uint8_t llc_out_ovp_soft_high_bit;
    uint8_t llc_out_ovp_dac_low_bit;
    uint8_t llc_out_ovp_dac_high_bit;
    uint8_t llc_out_uvp_low_bit;
    uint8_t llc_out_uvp_high_bit;
} llc_out_voltage_protection_TypeDef;

typedef struct {
    uint8_t frame_count;
    uint8_t return_bit;
    uint8_t llc_iout_target_low_bit;
    uint8_t llc_iout_target_high_bit;
    uint8_t llc_ocp_soft_low_bit;
    uint8_t llc_ocp_soft_high_bit;
    uint8_t llc_ocp_rec_soft_low_bit;
    uint8_t llc_ocp_rec_soft_high_bit;
} llc_out_over_current_protection_TypeDef;

typedef struct {
    uint8_t frame_count;
    uint8_t return_bit;
    uint8_t llc_short_soft_low_bit;
    uint8_t llc_short_soft_high_bit;
    uint8_t llc_short_hard_low_bit;
    uint8_t llc_short_hard_high_bit;
    uint8_t reserved0;
    uint8_t reserved1;
} llc_out_short_current_protection_TypeDef;

typedef struct {
    uint8_t frame_count;
    uint8_t return_bit;
    uint8_t llc_over_temp_low_bit;
    uint8_t llc_over_temp_high_bit;
    uint8_t llc_over_temp_rec_low_bit;
    uint8_t llc_over_temp_rec_high_bit;
    uint8_t reserved0;
    uint8_t reserved1;
} llc_temp_protection_TypeDef;

typedef struct {
    uint8_t frame_count;
    uint8_t return_bit;
    uint8_t llc_vbus_target_low_bit;
    uint8_t llc_vbus_target_high_bit;
    uint8_t llc_coef_target_low_bit;
    uint8_t llc_coef_target_high_bit;
    uint8_t llc_vbus_ref_low_bit;
    uint8_t llc_vbus_ref_high_bit;
} llc_out_voltage_para_TypeDef;

typedef struct {	
		uint8_t frame_count; 			// ֡����
    uint8_t cmd; 	 		// BIT������
    uint8_t power_state; 			// ��Դ״̬
    uint8_t load_current_low_bit; 			// ����ĸ�ߵ�8λ(0.1A)
    uint8_t load_current_high_bit;			// ����ĸ�߸�8λ(0.1A)
    uint8_t cur_low_bit; 			// ������8λ(0.1A)
    uint8_t cur_high_bit; 		// ������8λ(0.1A)
		uint8_t	reserve;
	
} share_power_TypeDef;

//typedef struct {
//    uint8_t frame_count; 			// ֡����
//    uint8_t version_cmd; 			// �汾��ѯ������
//    uint8_t Byte2; 						// ����
//    uint8_t Byte3; 						// ����
//    uint8_t Byte4; 						// ����
//    uint8_t Byte5; 						// ����
//    uint8_t Byte6; 						// ����
//    uint8_t Byte7; 						// ����
//} version_TypeDef;

typedef struct {
    uint8_t frame_count; 			// ֡����
    uint8_t version_cmd; 			// �汾��ѯ������
    uint8_t compnet_type; 		// �������
    uint8_t year; 						// ��
    uint8_t month; 						// ��
    uint8_t day; 							// ��
    uint8_t version_low_bit;	// ����
    uint8_t version_high_bit; // ����
} version_TypeDef;

typedef struct {
		power_upper_TypeDef		power_para;
		version_TypeDef				version_info;
} can_data_TypeDef;

typedef struct {
		share_power_TypeDef		share_power;
} can_share_power_TypeDef;

typedef struct { pfc_vbus_voltage_TypeDef vbus; } pfc_data_TypeDef;

typedef struct {
    pfc_input_ovp_TypeDef in_ovp;
    pfc_input_uvp_TypeDef in_uvp;
    pfc_output_ovp_TypeDef out_ovp;
    pfc_output_uvp_TypeDef out_uvp;
    pfc_input_ocp_TypeDef in_ocp;
} pfc_protect_TypeDef;

typedef struct {
    pfc_data_TypeDef data;
    pfc_protect_TypeDef protect;
    pfc_live1_TypeDef live1;
    pfc_live2_TypeDef live2;
} can_pfc_TypeDef;

typedef struct {
    can_pfc_TypeDef pfc;
    struct {
        llc_out_voltage_protection_TypeDef llc_voltage_protection_point;
        llc_out_over_current_protection_TypeDef llc_over_current_point;
        llc_out_short_current_protection_TypeDef llc_short_current_point;
        llc_temp_protection_TypeDef llc_temp_point;
        llc_out_voltage_para_TypeDef llc_voltage_output_para;
    } llc;
} user_can_TypeDef;


/*================================================ CAN ================================================*/

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
    state_idel = 0,		//����״̬����ʼ��
    state_rampup	,		//����״̬
    state_on			,		//����״̬
    state_fault		,		//����״̬

} LLC_STATE_TypeDef;

struct FAULT_BIT_TypeDef
{
    bool over_voltage  ;		//��ѹ
    bool under_voltage	;		//Ƿѹ
    bool over_current	;		//����
    bool over_temp			;		//����
		bool pfc_fault			;		//pfc����
} ;


//struct FAULT_BIT
//{
//	union
//	{
//		uint32_t fault_all;
//		struct FAULT_BIT_TypeDef fault_bit;
//	}fault_union;
//};
typedef union {
    struct {
        uint8_t can1_level : 1;  // ��ʾ��һ����ƽ�ĸߵ�
        uint8_t can2_level : 1;  // ��ʾ�ڶ�����ƽ�ĸߵ�
        uint8_t can3_level : 1;  // ��ʾ��������ƽ�ĸߵ�
        uint8_t reserved : 5; // ����λ��ȷ���ܹ�ռ�� 8 λ
    };
    uint8_t can_addr_level_bits;  // ʹ��һ���ֽ����洢���е�ƽ�����
} Can_LevelUnion;
 struct	send_state_TypeDef
{
		unsigned int loop_mode			: 	1		;	// ��ѹ�� / ������
		unsigned int switch_mode		: 	2		;	// ���� / ��Ƶ / ����
    unsigned int llc_state			: 	2		;	// idel / rampup / on /fault
		unsigned int llc_drv_state	:		1		;	// driver ��/��
		unsigned int llc_sr_state		:		1		;	// sr driver ��/��
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
    uint32_t over_voltage_cnt  ;		//��ѹ����
    uint32_t under_voltage_cnt	;		//Ƿѹ����
    uint32_t over_current_cnt	;			//��������
    uint32_t over_temp_cnt			;		//���¼���
    uint32_t over_temp_rec_cnt			;		//���¼���	
} FAULT_CNT_TypeDef;

typedef struct
{
    float over_voltage_point  		;		//��ѹ������
		float over_voltage_rec_point	;		//��ѹ�ָ���
    float under_voltage_point			;		//Ƿѹ������
    float under_voltage_rec_point	;		//Ƿѹ�ָ���	
    float over_current_point			;		//����������
    float over_current_rec_point	;		//�����ָ���	
    float over_temp_point					;		//���±�����
    float over_temp_rec_point			;		//���»ָ���	
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

//��ѹ����
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
//��������
float ibus_target;
float ibus_ref;
float iout_rel;
float iout_value_1;
float iout_rel_slow_store;
float iout_rel_slow;
float can_com_current_gain;
float can_com_current_offset;
//����������
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
uint32_t state_fault_cnt;		//����״̬�ָ�����
uint32_t fault_under_cnt;		//����ʱ�����

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
uint32_t 		led_cnt;	//�����Ƽ���

//��Դok�ӳ�
uint8_t			start_delay_cnt;


//on_off_ctrl
uint8_t			on_off_ctrl_cnt;	//�ⲿ����������������
bool				on_off_ctrl_ok;	//�ⲿ��������
uint8_t			off_ctrl_cnt;
bool on_off_ctrl_fault;
bool 				start_contr;

uint32_t		idel_delay_cnt;		//idle״̬���ӳټ���
bool				idel_delay_ok;		//idle״̬�ӳ����
uint32_t 		ramup_delay_cnt;

uint32_t		softstart_delay_cnt;		//idle״̬���ӳټ���
bool				softstart_delay_ok;		//idle״̬�ӳ����

//pfc_is_ok_cnt
uint32_t 		pfc_is_ok_cnt;		//pfc�����ź�
bool				pfc_is_ok;
user_can_TypeDef user_can;

uint32_t		ac_is_ok_cnt;			//AC_alarm
bool				ac_is_ok;

//protect/fault flag
volatile	uint32_t test_tx;		//vofa_tx ���͵Ĺ��ϱ�־λ
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
uint32_t can_addr;
uint32_t addr_cnt;
bool		addr_set_flag;

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
} llcTypeDef;

/*================================================ LLC ================================================*/



extern LLC_HRPWM_TypeDef mpwm, pwm0, pwm1,  pwm2, pwm3, pwm4, pwm5;
extern llcTypeDef llc;
extern can_data_TypeDef can_data,hld_can_data;
extern can_share_power_TypeDef can_share_power;










#define UART_FUNC 1



//<<< end of configuration section >>>
#endif

