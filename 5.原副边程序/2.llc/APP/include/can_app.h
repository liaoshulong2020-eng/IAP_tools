#ifndef __CAN__APP_H_
#define __CAN__APP_H_

#include "main.h"
#define USER_CAN_BAUDRATE           (125000)
#define USER_CAN_STD_FRM_ID         (0x20)		//�ͻ���λ��ͨ��ID
#define USER_CAN_EXT_FRM_ID         (0xA0000)	//ͨ��ʱ�Լ���ID

#define IAP_CAN_ID					0xAA55 //Jim edit: IAP CAN ID

#define KP_KI_SCALE_FACTOR 100

#define SET_FAULT_STATE(flag, can_flag) (can_flag = (flag) ? 1 : 0)
/*================================================ CAN ================================================*/

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
	
    CMD_PARA_SCALE = 0x27,			// ����������32λ����CRC��
    CMD_KP = 0x28,              // ����KP
    CMD_KI = 0x29,              // ����KI
    
    CMD_LLC_VOLTAGE_PROTECT = 0x3F, // 查询 LLC输出过压/欠压点
    CMD_LLC_OCP_PROTECT     = 0x40, // 查询 LLC输出过流/恢复点
    CMD_LLC_OSP_PROTECT     = 0x41, // 查询 LLC短路软/硬件点

    CMD_TEST = 0x42,            // 测试命令
    CMD_TEST2 = 0x43,           // 测试命令2
    CMD_LLC_OUT_PARA        = 0x44, // LLC vbus_target

    CMD_IAP = 0x50,             // IAP命令

    /* 原边(PFC)保护点查询命令 - 上位机发送0x3E，副边将从UART收到的PFC数据转发 */
    CMD_PFC_PROTECT         = 0x3E, // 查询PFC原边保护点（返回码0xBE）
} CommandType;


/*--------------------	��Դ�ϱ�֡	--------------------*/
#define RETURN_BIT_POWER				0x81	//byte1
#define POWER_STATE_GOOD				0x00	//byte2 ��Դ�޹���
#define POWER_STATE_FAULT				0x01	//byte2 ��Դ����

#define RETURN_BIT_VOLTAGE_PROTECT  0x83
#define RETURN_BIT_OCP_PROTECT      0x84
#define RETURN_BIT_OSP_PROTECT      0x85
#define RETURN_BIT_VOLTAGE_PARA     0x86

/* 原边(PFC)保护点上报帧返回码 */
#define RETURN_BIT_PFC_PROTECT      0xBE


/*--------------------	��Դ�ϱ�֡	--------------------*/

/*--------------------	��ѯ��Դ	--------------------*/
#define POWER_CMD								0X01	//��ѯ��Դ״̬����
#define VISION_CMD							0X02	//��ѯ��Դ�汾����
/*--------------------	��ѯ��Դ	--------------------*/

/*--------------------	�汾�ϱ�֡(�ͻ���)	--------------------*/
//�汾��ѯ������
#define 	VERSION_CMD						0x82	//byte1

//�������
#define 	COMPNET_TYPE					0x10	//byte2

//���ڶ��� 20260507
#define 	YEAR_NUM							26		//byte3
#define 	MONTH_NUM							05	 		//byte4
#define 	DAY_NUM								07 		//byte5

//�汾�Ŷ��� v1.0.0
#define 	MAJOR_VERSION					1
#define 	MINOR_VERSION					0
#define 	PATCH_VERSION					0
#define   VERSION_CODE 					(100 * MAJOR_VERSION + 10 * MINOR_VERSION + PATCH_VERSION)
/*--------------------	�汾�ϱ�֡(�ͻ���)	--------------------*/

/*--------------------	�汾�ϱ�֡(�з���)	--------------------*/
#define 	HLD_YEAR_NUM							26		//byte3
#define 	HLD_MONTH_NUM							05	 	//byte4
#define 	HLD_DAY_NUM								07 		//byte5

#define 	HLD_MAJOR_VERSION					1
#define 	HLD_MINOR_VERSION					0
#define 	HLD_PATCH_VERSION					0
#define   HLD_VERSION_CODE 					(100 * HLD_MAJOR_VERSION + 10 * HLD_MINOR_VERSION + HLD_PATCH_VERSION)	////byte6_low //byte7_high
/*--------------------	�汾�ϱ�֡(�з���)	--------------------*/
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

        // ʹ��λ����ÿһλ��Դ״̬
        struct {
            uint8_t power_on_off : 1;     // Bit 0: ��Դ���ػ�״̬ (1: ����, 0: �ػ�)
            uint8_t over_voltage : 1;     // Bit 1: ��ѹ����״̬ (1: ��ѹ, 0: ����)
            uint8_t under_voltage : 1;    // Bit 2: Ƿѹ����״̬ (1: Ƿѹ, 0: ����)
            uint8_t over_current : 1;     // Bit 3: ��������״̬ (1: ����, 0: ����)
            uint8_t over_temp : 1;        // Bit 4: ���¹���״̬ (1: ����, 0: ����)
            uint8_t commun_status : 1;      // Bit 5: ͨѶ״̬ (1: ����, 0: ����)
            uint8_t reserved : 1;         // Bit 6: ����λ
            uint8_t reserved2 : 1;        // Bit 7: ����λ
        };
    };
} power_state_TypeDef;

typedef struct {	
    uint8_t frame_count;           // ֡����
    uint8_t return_bit;            // BIT������
    power_state_TypeDef power_state;           // ��Դ״̬
    uint8_t vol_low_bit;           // ��ѹ��8λ(mv)
    uint8_t vol_high_bit;          // ��ѹ��8λ(mv)
    uint8_t cur_low_bit;           // ������8λ(0.1A)
    uint8_t cur_high_bit;          // ������8λ(0.1A)
    int8_t temp_bit;               // �¶�
} power_upper_TypeDef;

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
		uint32_t 							can_addr; 
} custom_can_data_TypeDef;

typedef struct {
		power_upper_TypeDef		power_para;
		version_TypeDef				version_info;
		uint32_t 							can_addr;
    float 								para_scale_factor;        // �������ű���
    bool									para_scale_factor_is_ok	;	//�������ű����趨ok
} hld_can_data_TypeDef;


/* ---- LLC ����/����֡���� 8 �ֽڣ� ---- */
typedef struct {
    uint8_t frame_count;
    uint8_t return_bit;
    uint8_t llc_out_ovp_soft_low_bit;   /* �����ѹ����������8λ (mV) */
    uint8_t llc_out_ovp_soft_high_bit;  /* �����ѹ����������8λ (mV) */
    uint8_t llc_out_ovp_dac_low_bit;    /* �����ѹӲ��������8λ (mV) */
    uint8_t llc_out_ovp_dac_high_bit;   /* �����ѹӲ��������8λ (mV) */
    uint8_t llc_out_uvp_low_bit;        /* ���Ƿѹ������8λ (mV) */
    uint8_t llc_out_uvp_high_bit;       /* ���Ƿѹ������8λ (mV) */
} llc_out_voltage_protection_TypeDef;

typedef struct {
    uint8_t frame_count;
    uint8_t return_bit;
    uint8_t llc_iout_target_low_bit;    /* �������8λ (0.1A) */
    uint8_t llc_iout_target_high_bit;   /* �������8λ (0.1A) */
    uint8_t llc_ocp_soft_low_bit;       /* ��������������8λ (0.1A) */
    uint8_t llc_ocp_soft_high_bit;      /* ��������������8λ (0.1A) */
    uint8_t llc_ocp_rec_soft_low_bit;   /* �����ָ�����������8λ (0.1A) */
    uint8_t llc_ocp_rec_soft_high_bit;  /* �����ָ�����������8λ (0.1A) */
} llc_out_over_current_protection_TypeDef;

typedef struct {
    uint8_t frame_count;
    uint8_t return_bit;
    uint8_t llc_short_soft_low_bit;     /* ��·����������8λ (0.1A) */
    uint8_t llc_short_soft_high_bit;    /* ��·����������8λ (0.1A) */
    uint8_t llc_short_hard_low_bit;     /* ��·Ӳ��������8λ (0.1A) */
    uint8_t llc_short_hard_high_bit;    /* ��·Ӳ��������8λ (0.1A) */
    uint8_t reserved0;
    uint8_t reserved1;
} llc_out_short_current_protection_TypeDef;

typedef struct {
    uint8_t frame_count;
    uint8_t return_bit;
    uint8_t llc_vbus_target_low_bit;    /* �����ѹĿ�� ��8λ (mV) */
    uint8_t llc_vbus_target_high_bit;   /* �����ѹĿ�� ��8λ (mV) */
    uint8_t llc_coef_target_low_bit;    /* У׼��ѹ ��8λ (mV) */
    uint8_t llc_coef_target_high_bit;   /* У׼��ѹ ��8λ (mV) */
    uint8_t llc_vbus_ref_low_bit;       /* ��ǰ��ѹ�ο� ��8λ (mV) */
    uint8_t llc_vbus_ref_high_bit;      /* ��ǰ��ѹ�ο� ��8λ (mV) */
} llc_out_voltage_para_TypeDef;

typedef struct {
    llc_out_voltage_protection_TypeDef       llc_voltage_protection_point;
    llc_out_over_current_protection_TypeDef  llc_over_current_point;
    llc_out_short_current_protection_TypeDef llc_short_current_point;
    llc_out_voltage_para_TypeDef             llc_voltage_output_para;
} can_llc_TypeDef;

/* ---- PFC原边保护点上报帧（8字节，返回码0xBE） ---- */
/* 上位机发送CMD_PFC_PROTECT(0x3E)查询，副边将UART收到的PFC保护点数据分多帧上报 */
/* 帧1：PFC输入电压保护点 */
typedef struct {
    uint8_t frame_count;
    uint8_t return_bit;                     /* 0xBE */
    uint8_t pfc_vin_uvp_low;               /* PFC输入欠压点 低8位 (mV) */
    uint8_t pfc_vin_uvp_high;              /* PFC输入欠压点 高8位 (mV) */
    uint8_t pfc_vin_ovp_low;               /* PFC输入过压点 低8位 (mV) */
    uint8_t pfc_vin_ovp_high;              /* PFC输入过压点 高8位 (mV) */
    uint8_t pfc_vin_on_low;                /* PFC输入启动电压 低8位 (mV) */
    uint8_t pfc_vin_on_high;               /* PFC输入启动电压 高8位 (mV) */
} pfc_vin_protect_TypeDef;

/* 帧2：PFC输出电压及过流保护点 */
typedef struct {
    uint8_t frame_count;
    uint8_t return_bit;                     /* 0xBF */
    uint8_t pfc_vout_ovp_sw_low;           /* PFC输出过压软件点 低8位 (mV) */
    uint8_t pfc_vout_ovp_sw_high;          /* PFC输出过压软件点 高8位 (mV) */
    uint8_t pfc_bus_ovp_hw_low;            /* PFC母线过压硬件点 低8位 (mV) */
    uint8_t pfc_bus_ovp_hw_high;           /* PFC母线过压硬件点 高8位 (mV) */
    uint8_t pfc_ocp_sw_low;                /* PFC过流软件点 低8位 (0.1A) */
    uint8_t pfc_ocp_sw_high;               /* PFC过流软件点 高8位 (0.1A) */
} pfc_vout_protect_TypeDef;

typedef struct {
    pfc_vin_protect_TypeDef  pfc_vin_protect;
    pfc_vout_protect_TypeDef pfc_vout_protect;
} can_pfc_TypeDef;

typedef struct {
    can_llc_TypeDef llc;
    can_pfc_TypeDef pfc;
} user_can_TypeDef;

typedef union {
    struct {
        uint8_t can1_level : 1;  // ��ʾ��һ����ƽ�ĸߵ�
        uint8_t can2_level : 1;  // ��ʾ�ڶ�����ƽ�ĸߵ�
        uint8_t can3_level : 1;  // ��ʾ��������ƽ�ĸߵ�
        uint8_t reserved : 5; // ����λ��ȷ���ܹ�ռ�� 8 λ
    };
    uint8_t can_addr_level_bits;  // ʹ��һ���ֽ����洢���е�ƽ�����
} Can_LevelUnion;

void can_init_app(void);
void can_send_data(void* data_buf, size_t data_size);
void can_send_data_init(void);
void can_addr_set();
bool process_32bit_with_crc(uint8_t cmd, float* result_value, int scale_factor);
void can_send_pfc_protect_data(void);  /* 将PFC保护点数据通过CAN上报给上位机 */
#define SEND_MASSAGE_ID							(0xB0000)
#define SEND_MASSAGE_EXT_FRM_ID     (0xC0000)

/* PFC输出电压保护点帧返回码（第二帧） */
#define RETURN_BIT_PFC_VOUT_PROTECT 0xBF

extern uint32_t buf[64];  
extern void can_addr_set();
#endif

/*================================================ CAN ================================================*/
