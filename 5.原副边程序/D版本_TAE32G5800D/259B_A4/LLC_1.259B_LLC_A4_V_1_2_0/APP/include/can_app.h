#ifndef __CAN__APP_H_
#define __CAN__APP_H_

#include "main.h"
#define USER_CAN_BAUDRATE           (125000)
#define USER_CAN_STD_FRM_ID         (0x20)		//�ͻ���λ��ͨ��ID
#define USER_CAN_EXT_FRM_ID         (0xA0000)	//ͨ��ʱ�Լ���ID

#define IAP_CAN_ID					0xAA55 //Jim edit: IAP CAN ID


typedef struct {
    uint32_t id;
    // ����������Ҫ���豸��Ϣ�ֶ�
    float current_bus;
    float current_sigel;
} DeviceInfo;
	
void can_init_app(void);
void can_send_data(void* data_buf, size_t data_size);
void can_send_data_init(void);
void can_addr_set();
bool process_32bit_with_crc(uint8_t cmd, float* result_value, int scale_factor);
#define SEND_MASSAGE_ID							(0xB0000)	//?????????��ID
#define SEND_MASSAGE_EXT_FRM_ID     (0xC0000)	//?????����?????ID

extern uint32_t buf[64];  
extern void can_addr_set();
#endif
