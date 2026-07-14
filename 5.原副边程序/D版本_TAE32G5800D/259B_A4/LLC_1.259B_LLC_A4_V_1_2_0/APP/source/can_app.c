// �ϲ������߼���CAN���մ���

#include "main.h"
#include "can_app.h"
#include "variables_define_app.h"
#include "vofa_app.h"
#include "pri_sec_commun_app.h"

#define KP_KI_SCALE_FACTOR 100

DeviceInfo devices[6];
int deviceCount = 0;
#define USER_CAN_RX_FRM_NUMS (1)
uint8_t dev_index ;

void User_CAN_RxCpltCallback(void);
void can_send_16bit_simple(uint8_t cmd, uint16_t value);
RAMCODE
static void llc_uart_apply_mode(uint8_t mode)
{
  if(mode > LLC_UART_MODE_IAP)
    {
      return;
    }

  if(mode == LLC_UART_MODE_VOFA)
    {
      pfc_comm_deinit();
      User_VOFA_UART_DeInit();
      llc_uart_work_mode = LLC_UART_MODE_VOFA;
      User_VOFA_Init();
    }
  else
    {
      User_VOFA_UART_DeInit();
      llc_uart_work_mode = mode;
      if(mode == LLC_UART_MODE_PFC_COMM)
        {
          pfc_comm_init();
        }
      else
        {
          pfc_comm_deinit();
        }
    }

  can_send_16bit_simple(CMD_UART_MODE, (uint16_t)llc_uart_work_mode);
}

typedef struct __CAN_UserCtrlTypeDef
{
  CAN_TypeDef* Instance;
  volatile uint32_t tx_cnt;
  volatile uint32_t rx_cnt;
  CAN_TxBufFormatTypeDef txbuf_fmt;
  CAN_RxBufFormatTypeDef rxbuf_fmt[USER_CAN_RX_FRM_NUMS];
  uint8_t rx_buf[USER_CAN_RX_FRM_NUMS][64];
} CAN_UserCtrlTypeDef;

CAN_UserCtrlTypeDef user_can_ctrl;

// CAN������ʱ����
static inline void can_ctrl_delay(void)
{
  CAN1->CTRL |= 1<<4;
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
  CAN1->CTRL &= ~(1<<4);
}
// =============================================================================
// 16λ���ݴ�������
// =============================================================================

// 16λ������ȡ����������CRC��
static inline uint16_t extract_16bit_data(uint8_t start_index)
{
  uint8_t current_rx_index = user_can_ctrl.rx_cnt;
  uint8_t low_byte = user_can_ctrl.rx_buf[current_rx_index][start_index];
  uint8_t high_byte = user_can_ctrl.rx_buf[current_rx_index][start_index + 1];
  return (high_byte << 8) | low_byte;
}

// 16λ���ݴ�������������CRC��
RAMCODE
bool process_16bit_simple(uint8_t cmd, uint16_t* result_value)
{
  uint8_t current_rx_index = user_can_ctrl.rx_cnt;

  // ��֤���ݳ��ȣ�������Ҫ4�ֽڣ�����+����+2�ֽ����ݣ�
  if (user_can_ctrl.rxbuf_fmt[current_rx_index].data_len_code < 4)
    {
      return false;
    }

  // ��֤����
  uint8_t received_cmd = user_can_ctrl.rx_buf[current_rx_index][1];
  if(received_cmd != cmd)
    {
      return false;
    }

  // ��ȡ16λ����
  *result_value = extract_16bit_data(2);
  return true;
}

// 16λ���ݴ�����������CRC��
RAMCODE
bool process_16bit_with_crc(uint8_t cmd, uint16_t* result_value)
{
  uint8_t current_rx_index = user_can_ctrl.rx_cnt;

  // ��֤���ݳ��ȣ�������Ҫ5�ֽڣ�����+����+2�ֽ�����+CRC��
  if (user_can_ctrl.rxbuf_fmt[current_rx_index].data_len_code < 5)
    {
      return false;
    }

  // ��֤����
  uint8_t received_cmd = user_can_ctrl.rx_buf[current_rx_index][1];
  if(received_cmd != cmd)
    {
      return false;
    }

  // ��ȡ���ݺ�CRC
  uint8_t low_byte = user_can_ctrl.rx_buf[current_rx_index][2];
  uint8_t high_byte = user_can_ctrl.rx_buf[current_rx_index][3];
  uint8_t received_crc = user_can_ctrl.rx_buf[current_rx_index][4];

  // ��֤CRC
  uint8_t crc_data[3] = {received_cmd, low_byte, high_byte};
  uint8_t calculated_crc = crc8(crc_data, sizeof(crc_data));

  if(calculated_crc != received_crc)
    {
      return false; // CRCУ��ʧ��
    }

  // ��װ����
  *result_value = (high_byte << 8) | low_byte;
  return true;
}

// 16λ���ݷ��ͺ���������CRC��
RAMCODE
void can_send_16bit_simple(uint8_t cmd, uint16_t value)
{
  CAN_TxBufFormatTypeDef tx_buf_fmt;

  uint8_t data_bytes[4] =
  {
    0x00,                          // �����ֽ�
    cmd,                           // ����
    (uint8_t)(value & 0xFF),       // ���ֽ�
    (uint8_t)((value >> 8) & 0xFF) // ���ֽ�
  };

  tx_buf_fmt.id_extension = 1;
  tx_buf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(llc.can_addr);
  tx_buf_fmt.data_len_code = sizeof(data_bytes);
  tx_buf_fmt.remote_tx_req = 0;

  LL_CAN_TransmitPTB_CPU(CAN1, &tx_buf_fmt, (uint32_t*)data_bytes);
}

// 16λ���ݷ��ͺ�������CRC��
RAMCODE
void can_send_16bit_with_crc(uint8_t cmd, uint16_t value)
{
  CAN_TxBufFormatTypeDef tx_buf_fmt;

  uint8_t low_byte = (uint8_t)(value & 0xFF);
  uint8_t high_byte = (uint8_t)((value >> 8) & 0xFF);

  // ����CRC
  uint8_t crc_data[3] = {cmd, low_byte, high_byte};
  uint8_t crc = crc8(crc_data, sizeof(crc_data));

  uint8_t data_bytes[5] =
  {
    0x00,      // �����ֽ�
    cmd,       // ����
    low_byte,  // ���ֽ�
    high_byte, // ���ֽ�
    crc        // CRC
  };

  tx_buf_fmt.id_extension = 1;
  tx_buf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(llc.can_addr);
  tx_buf_fmt.data_len_code = sizeof(data_bytes);
  tx_buf_fmt.remote_tx_req = 0;

  LL_CAN_TransmitPTB_CPU(CAN1, &tx_buf_fmt, (uint32_t*)data_bytes);
}

// =============================================================================
// 32λ���ݴ�������
// =============================================================================

// 32λ������ȡ����������CRC��
static inline uint32_t extract_32bit_data(uint8_t start_index)
{
  uint8_t current_rx_index = user_can_ctrl.rx_cnt;
  uint8_t byte0 = user_can_ctrl.rx_buf[current_rx_index][start_index];     // ����ֽ�
  uint8_t byte1 = user_can_ctrl.rx_buf[current_rx_index][start_index + 1];
  uint8_t byte2 = user_can_ctrl.rx_buf[current_rx_index][start_index + 2];
  uint8_t byte3 = user_can_ctrl.rx_buf[current_rx_index][start_index + 3]; // ����ֽ�

  return ((uint32_t)byte3 << 24) |
         ((uint32_t)byte2 << 16) |
         ((uint32_t)byte1 << 8) |
         byte0;
}

// 32λ���ݴ�������������CRC��
RAMCODE
bool process_32bit_simple(uint8_t cmd, uint32_t* result_value)
{
  uint8_t current_rx_index = user_can_ctrl.rx_cnt;

  // ��֤���ݳ��ȣ�������Ҫ6�ֽڣ�����+����+4�ֽ����ݣ�
  if (user_can_ctrl.rxbuf_fmt[current_rx_index].data_len_code < 6)
    {
      return false;
    }

  // ��֤����
  uint8_t received_cmd = user_can_ctrl.rx_buf[current_rx_index][1];
  if(received_cmd != cmd)
    {
      return false;
    }

  // ��ȡ32λ����
  *result_value = extract_32bit_data(2);
  return true;
}

// 32λ����CRC��������
RAMCODE
bool process_32bit_with_crc(uint8_t cmd, float* result_value, int scale_factor)
{
  uint8_t current_rx_index = user_can_ctrl.rx_cnt;

  // ��֤���ݳ��ȣ�������Ҫ7�ֽڣ�����+����+4�ֽ�����+CRC��
  if (user_can_ctrl.rxbuf_fmt[current_rx_index].data_len_code < 7)
    {
      return false;
    }

  // ��ȡ���յ�������
  uint8_t received_cmd = user_can_ctrl.rx_buf[current_rx_index][1];
  uint8_t data_byte0 = user_can_ctrl.rx_buf[current_rx_index][2];  // ���λ�ֽ�
  uint8_t data_byte1 = user_can_ctrl.rx_buf[current_rx_index][3];
  uint8_t data_byte2 = user_can_ctrl.rx_buf[current_rx_index][4];
  uint8_t data_byte3 = user_can_ctrl.rx_buf[current_rx_index][5];  // ���λ�ֽ�
  uint8_t received_crc = user_can_ctrl.rx_buf[current_rx_index][6];

  // ��֤�����Ƿ�ƥ��
  if(received_cmd != cmd)
    {
      return false;
    }

  // �ؽ�CRC�������ݣ�����+4�ֽ����ݣ�
  uint8_t crc_data[5] = {received_cmd, data_byte0, data_byte1, data_byte2, data_byte3};
  uint8_t calculated_crc = crc8(crc_data, sizeof(crc_data));

  // ��֤CRC
  if(calculated_crc != received_crc)
    {
      return false; // CRCУ��ʧ��
    }

  // ����32λ����
  uint32_t raw_value = ((uint32_t)data_byte3 << 24) |
                       ((uint32_t)data_byte2 << 16) |
                       ((uint32_t)data_byte1 << 8) |
                       data_byte0;

  // ת��Ϊ������
  float float_value = (float)raw_value / (float)scale_factor;

//  // ��Χ��飨KP/KIͨ����0-100000��Χ�ڣ�
//  if(float_value < 0.0f || float_value > 100000.0f)
//    {
//      return false; // ��ֵ������Χ
//    }

  // ������
  *result_value = float_value;
  return true;
}

// 32λ���ݴ�����������CRC������ԭʼ���ݣ�
RAMCODE
bool process_32bit_with_crc_raw(uint8_t cmd, uint32_t* result_value)
{
  uint8_t current_rx_index = user_can_ctrl.rx_cnt;

  // ��֤���ݳ��ȣ�������Ҫ7�ֽڣ�����+����+4�ֽ�����+CRC��
  if (user_can_ctrl.rxbuf_fmt[current_rx_index].data_len_code < 7)
    {
      return false;
    }

  // ��ȡ���յ�������
  uint8_t received_cmd = user_can_ctrl.rx_buf[current_rx_index][1];
  uint8_t data_byte0 = user_can_ctrl.rx_buf[current_rx_index][2];
  uint8_t data_byte1 = user_can_ctrl.rx_buf[current_rx_index][3];
  uint8_t data_byte2 = user_can_ctrl.rx_buf[current_rx_index][4];
  uint8_t data_byte3 = user_can_ctrl.rx_buf[current_rx_index][5];
  uint8_t received_crc = user_can_ctrl.rx_buf[current_rx_index][6];

  // ��֤����
  if(received_cmd != cmd)
    {
      return false;
    }

  // ��֤CRC
  uint8_t crc_data[5] = {received_cmd, data_byte0, data_byte1, data_byte2, data_byte3};
  uint8_t calculated_crc = crc8(crc_data, sizeof(crc_data));

  if(calculated_crc != received_crc)
    {
      return false;
    }

  // ����32λ����
  *result_value = ((uint32_t)data_byte3 << 24) |
                  ((uint32_t)data_byte2 << 16) |
                  ((uint32_t)data_byte1 << 8) |
                  data_byte0;

  return true;
}

// 32λ���ݷ��ͺ���������CRC��
RAMCODE
void can_send_32bit_simple(uint8_t cmd, uint32_t value)
{
  CAN_TxBufFormatTypeDef tx_buf_fmt;

  uint8_t data_bytes[6] =
  {
    0x00,                             // �����ֽ�
    cmd,                              // ����
    (uint8_t)(value & 0xFF),          // byte0 (����ֽ�)
    (uint8_t)((value >> 8) & 0xFF),   // byte1
    (uint8_t)((value >> 16) & 0xFF),  // byte2
    (uint8_t)((value >> 24) & 0xFF)   // byte3 (����ֽ�)
  };

  tx_buf_fmt.id_extension = 1;
  tx_buf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(llc.can_addr);
  tx_buf_fmt.data_len_code = sizeof(data_bytes);
  tx_buf_fmt.remote_tx_req = 0;

  LL_CAN_TransmitPTB_CPU(CAN1, &tx_buf_fmt, (uint32_t*)data_bytes);
}

// 32λ���ݷ��ͺ�������CRC��
RAMCODE
void can_send_32bit_with_crc(uint8_t cmd, uint32_t value)
{
  CAN_TxBufFormatTypeDef tx_buf_fmt;

  uint8_t byte0 = (uint8_t)(value & 0xFF);
  uint8_t byte1 = (uint8_t)((value >> 8) & 0xFF);
  uint8_t byte2 = (uint8_t)((value >> 16) & 0xFF);
  uint8_t byte3 = (uint8_t)((value >> 24) & 0xFF);

  // ����CRC
  uint8_t crc_data[5] = {cmd, byte0, byte1, byte2, byte3};
  uint8_t crc = crc8(crc_data, sizeof(crc_data));

  uint8_t data_bytes[7] =
  {
    0x00,   // �����ֽ�
    cmd,    // ����
    byte0,  // ����ֽ�
    byte1,
    byte2,
    byte3,  // ����ֽ�
    crc     // CRC
  };

  tx_buf_fmt.id_extension = 1;
  tx_buf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(llc.can_addr);
  tx_buf_fmt.data_len_code = sizeof(data_bytes);
  tx_buf_fmt.remote_tx_req = 0;

  LL_CAN_TransmitPTB_CPU(CAN1, &tx_buf_fmt, (uint32_t*)data_bytes);
}

// 32λ���������ͺ�������CRC�����ţ�
RAMCODE
void can_send_32bit_float_with_crc(uint8_t cmd, float value, int scale_factor)
{
  uint32_t scaled_value = (uint32_t)(value * scale_factor);
  can_send_32bit_with_crc(cmd, scaled_value);
}

RAMCODE
static void can_send_8bytes(uint8_t data_bytes[8])
{
  CAN_TxBufFormatTypeDef tx_buf_fmt;

  tx_buf_fmt.id_extension = 1;
  tx_buf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(llc.can_addr);
  tx_buf_fmt.data_len_code = 8;
  tx_buf_fmt.remote_tx_req = 0;

  LL_CAN_TransmitPTB_CPU(CAN1, &tx_buf_fmt, (uint32_t*)data_bytes);
}
RAMCODE
void process_common_commands(uint8_t can_cmd, uint32_t received_id)
{
    switch(can_cmd) {
        case CMD_QUERY:
        {     
            can_ctrl_delay();
            can_send_data((void*)&can_data.power_para, sizeof(can_data.power_para));
        }
        break;

        case CMD_VERSION:
        {
            can_ctrl_delay();
            if(received_id == 0xB0000) {
                can_send_data((void*)&hld_can_data.version_info, sizeof(hld_can_data.version_info));
            } else  {
                can_send_data((void*)&can_data.version_info, sizeof(can_data.version_info));
            }
        }
        break;

        case CMD_START:
            llc.start_contr = 1;
            break;

        case CMD_STOP:
            llc.start_contr = 0;
            break;


        case CMD_UART_MODE:
        {
            uint8_t mode = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][2];
            llc_uart_apply_mode(mode);
        }
        break;
        case CMD_PFC_INPUT_OVP:
        case CMD_PFC_INPUT_UVP:
        case CMD_PFC_OUTPUT_OVP:
        case CMD_PFC_OUTPUT_UVP:
        case CMD_PFC_INPUT_OCP:
        case CMD_PFC_DATA:
        case CMD_PFC_DATA_LIVE1:
        case CMD_PFC_DATA_LIVE2:
        {
            uint8_t frame[8];
            if(pfc_comm_fill_can_response(can_cmd, frame))
              {
                can_ctrl_delay();
                can_send_8bytes(frame);
              }
        }
        break;
        case CMD_STORE_FLASH:
            save_data_flash();
            save_data_to_flash(&user_data);
            break;

        case CMD_LOAD_FLASH:
            load_data_app();
            break;
            
        case CMD_TEMP_RECOVER_ON:
            user_data.temp_recover_mode = 1;
            llc.temp_recover_mode = user_data.temp_recover_mode;
            break;
            
        case CMD_TEMP_RECOVER_OFF:
            user_data.temp_recover_mode = 0;
            llc.temp_recover_mode = user_data.temp_recover_mode;
            break;

        case CMD_OVERTEMP_POINT:  // ���µ㣨16λ������CRC��
        {
            uint16_t rx_data_16;
            if(process_16bit_simple(CMD_OVERTEMP_POINT, &rx_data_16)) {
                user_data.over_temp_point = (float)(rx_data_16);
                llc.protection_point.over_temp_point = user_data.over_temp_point;
            }
        }
        break;

        case CMD_OVERTEMP_REC_POINT:  // ���»ָ��㣨16λ����CRC��
        {
            uint16_t rx_data_16;
            if(process_16bit_with_crc(CMD_OVERTEMP_REC_POINT, &rx_data_16)) {
                user_data.over_temp_rec_point = (float)(rx_data_16);
                llc.protection_point.over_temp_rec_point = user_data.over_temp_rec_point;
            }
        }
        break;

        case CMD_FACTOR_VOLTAGE:  // У׼��ѹ��16λ������CRC��
        {
            uint16_t rx_data_16;
            if(process_16bit_simple(CMD_FACTOR_VOLTAGE, &rx_data_16)) {
                llc.factor_voltage = rx_data_16;
            }
        }
        break;

        case CMD_THEOR_VOLTAGE:  // ���۵�ѹ��16λ������CRC��
        {
            uint16_t rx_data_16;
            if(process_16bit_simple(CMD_THEOR_VOLTAGE, &rx_data_16)) {
                llc.theor_voltage = rx_data_16;
            }
        }
        break;

				case CMD_VOLTAGE_CRC:
				{
						llc.voltage_crc = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][2];

						uint16_t factor_voltage_value = (uint16_t)llc.factor_voltage;
						uint8_t factor_voltage_low_byte  = (uint8_t)(factor_voltage_value & 0xFF);
						uint8_t factor_voltage_high_byte = (uint8_t)((factor_voltage_value >> 8) & 0xFF);

						uint16_t theor_voltage_value = (uint16_t)llc.theor_voltage;
						uint8_t theor_voltage_low_byte  = (uint8_t)(theor_voltage_value & 0xFF);
						uint8_t theor_voltage_high_byte = (uint8_t)((theor_voltage_value >> 8) & 0xFF);

						uint8_t crc_data[9] =
						{
								CMD_VOLTAGE_CRC,
								factor_voltage_low_byte,
								factor_voltage_high_byte,
								0x00,
								0x00,
								theor_voltage_low_byte,
								theor_voltage_high_byte,
								0x00,
								0x00
						};

						llc.crc_data = crc8(crc_data, sizeof(crc_data));

						if(llc.crc_data == llc.voltage_crc)
						{
								float factor_voltage_float = llc.factor_voltage / 1000.0f; // ��ǰ����/��λ����ʾֵ
								float theor_voltage_float  = llc.theor_voltage  / 1000.0f; // ���ñ���ʵֵ

								if((factor_voltage_float > 1.0f) && (theor_voltage_float > 1.0f))
								{
										/*
										 * ����Ŀ��У׼��
										 * ���統ǰĿ��48.19V��ʵ��ӦΪ48.00V
										 * ��Ŀ�� = ԭĿ�� / ��ǰ��ʾֵ * ��ʵֵ
										 */
										user_data.coef_target = 
												(llc.vbus_target / factor_voltage_float) * theor_voltage_float;

										/*
										 * ��ʾ������
										 * ��λ����ʾֵ = llc.vbus_rel - llc.can_com_voltag_delta
										 * ���� 48.19 - 0.19 = 48.00
										 */
										user_data.vout_can_delta = 
												factor_voltage_float - theor_voltage_float;

										/*
										 * Ŀ���ѹ���48V��ƫ����
										 */
										user_data.vout_trim_delta = 
												user_data.coef_target - 48.0f;

										llc.coef_target = user_data.coef_target;
										llc.vbus_target = llc.coef_target;
										llc.vout_trim_delta = user_data.vout_trim_delta;
										llc.can_com_voltag_delta = user_data.vout_can_delta;
								}
								else
								{
										llc.factor_voltage = 0;
										llc.theor_voltage = 0;
								}
						}
						else
						{
								llc.factor_voltage = 0;
								llc.theor_voltage = 0;
						}
				}
				break;

//				case CMD_PARA_SCALE:
//				{
//					  float float_value;
//            if(process_32bit_with_crc(CMD_KP, &float_value, 1)) 
//						{
//                llc.para_scale_factor = float_value;
//								llc.para_scale_factor_is_ok = 1;
//            }
//				
//				}
//				break;

        case CMD_KP:  // KP������32λ����CRC�����㣩
        {
            float float_value;
//					if(llc.para_scale_factor_is_ok)
					{
						if(process_32bit_with_crc(CMD_KP, &float_value, KP_KI_SCALE_FACTOR))
							{
								user_data.shareloop_kp_init  = float_value;
                llc.shareloop_kp_init =	user_data.shareloop_kp_init;
							}
					}
        }
        break;

        case CMD_KI:  // KI������32λ����CRC�����㣩
        {
            float float_value;
//					if(llc.para_scale_factor_is_ok)
					{
            if(process_32bit_with_crc(CMD_KI, &float_value, KP_KI_SCALE_FACTOR)) 
							{
								user_data.shareloop_ki_init  = float_value;
                llc.shareloop_ki_init =	user_data.shareloop_ki_init;
							}
					}
        }
        break;

        case CMD_TEST:  // ���Բ�����32λ����CRC��
        {
            float rx_data_32;
//					if(llc.para_scale_factor_is_ok)
          {  
						if(process_32bit_with_crc(CMD_TEST, &rx_data_32, KP_KI_SCALE_FACTOR)) 
						{
							user_data.shareloop_out_max_init = (float)rx_data_32;
							llc.shareloop.out_max = user_data.shareloop_out_max_init;
							llc.shareloop.out_min = -user_data.shareloop_out_max_init;
            }
					}
        }
        break;

        case CMD_TEST2:  // ���Բ���2��32λ����CRC��
        {
            float rx_data_32;
//					if(llc.para_scale_factor_is_ok)
						{
            if(process_32bit_with_crc(CMD_TEST2, &rx_data_32, KP_KI_SCALE_FACTOR))
						{
//							llc.shareloop.out_min = (float)rx_data_32;
            }
						}
        }
        break;

        default:
            // δ֪�������������־��¼
            break;
    }
}

void can_init_app(void)
{
  CAN_UserCfgTypeDef can_user_cfg;
  CAN_AcceptFilCfgTypeDef can_acpt_fil_cfg[4];

  memset((void*)&can_user_cfg, 0x00, sizeof(can_user_cfg));
  memset((void*)&can_acpt_fil_cfg, 0x00, sizeof(can_acpt_fil_cfg));

  //CAN acceptance filter config
  can_acpt_fil_cfg[0].slot = CAN_ACCEPT_FILT_SLOT_0;
  can_acpt_fil_cfg[0].code_val = USER_CAN_STD_FRM_ID;
  can_acpt_fil_cfg[0].mask_val = 0;
  can_acpt_fil_cfg[0].rx_frm = CAN_ACCEPT_FILT_FRM_STD_EXT;

  can_acpt_fil_cfg[1].slot = CAN_ACCEPT_FILT_SLOT_1;
  can_acpt_fil_cfg[1].code_val = 0xA0000;
  can_acpt_fil_cfg[1].mask_val = 0xFF;
  can_acpt_fil_cfg[1].rx_frm = CAN_ACCEPT_FILT_FRM_STD_EXT;

  can_acpt_fil_cfg[2].slot = CAN_ACCEPT_FILT_SLOT_2;
  can_acpt_fil_cfg[2].code_val = 0xB0000;
  can_acpt_fil_cfg[2].mask_val = 0xFF;
  can_acpt_fil_cfg[2].rx_frm = CAN_ACCEPT_FILT_FRM_STD_EXT;

  can_acpt_fil_cfg[3].slot = CAN_ACCEPT_FILT_SLOT_3;
  can_acpt_fil_cfg[3].code_val = IAP_CAN_ID;
  can_acpt_fil_cfg[3].mask_val = 0;
  can_acpt_fil_cfg[3].rx_frm = CAN_ACCEPT_FILT_FRM_STD_EXT;

  //CAN LL Init
  can_user_cfg.fd_en = false;
  can_user_cfg.fd_iso_en = false;
  can_user_cfg.func_clk_freq = 120000000UL;
  can_user_cfg.baudrate_ss = USER_CAN_BAUDRATE;
  can_user_cfg.bit_timing_seg1_ss = 6;
  can_user_cfg.bit_timing_seg2_ss = 1;
  can_user_cfg.bit_timing_sjw_ss = 1;
  can_user_cfg.rx_almost_full_limit = CAN_RX_ALMOST_FULL_LIMIT_1;
  can_user_cfg.err_limit = CAN_ERR_WARN_LIMIT_104;
  can_user_cfg.accept_fil_cfg_ptr = can_acpt_fil_cfg;
  can_user_cfg.accept_fil_cfg_num = ARRAY_SIZE(can_acpt_fil_cfg);

  LL_CAN_Init(CAN1, &can_user_cfg);

  //User CAN control init
  memset((void*)&user_can_ctrl, 0x00, sizeof(user_can_ctrl));
  user_can_ctrl.Instance = CAN1;
  user_can_ctrl.txbuf_fmt.id_extension = 1;
  user_can_ctrl.txbuf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(USER_CAN_STD_FRM_ID);

  // ���ý��ջ�������ʽ
  for(int i = 0; i < USER_CAN_RX_FRM_NUMS; i++)
    {
      user_can_ctrl.rxbuf_fmt[i].id_extension = 1;
      user_can_ctrl.rxbuf_fmt[i].data_len_code = 8;
    }

  // ����CAN�����ж�
  LL_CAN_Receive_IT(user_can_ctrl.Instance, &user_can_ctrl.rxbuf_fmt[0], user_can_ctrl.rx_buf[0]);
  for(int i = 0; i < 8; i++)
    {
      llc.can_buf[i] = i;
    }
  __LL_CAN_Rx_INT_En(CAN1);
}

RAMCODE
void can_send_data(void* data_buf, size_t data_size)
{
  CAN_TxBufFormatTypeDef tx_buf_fmt;

  tx_buf_fmt.id_extension = 1;
  tx_buf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(llc.can_addr);
  tx_buf_fmt.data_len_code = data_size;
  tx_buf_fmt.remote_tx_req = 0;

  LL_StatusETypeDef status = LL_CAN_TransmitPTB_CPU(CAN1, &tx_buf_fmt, (uint32_t*)data_buf);

  if(status != LL_OK)
    {
      // Error handling
    }
}

RAMCODE
void can_send_data_with_crc(void* data_buf, size_t data_size)
{
  CAN_TxBufFormatTypeDef tx_buf_fmt;

  uint8_t data_with_crc[data_size + 1];
  memcpy(data_with_crc, data_buf, data_size);

  uint8_t crc = crc8((uint8_t*)data_buf, data_size);
  data_with_crc[data_size] = crc;

  tx_buf_fmt.id_extension = 1;
  tx_buf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(llc.can_addr);
  tx_buf_fmt.data_len_code = sizeof(data_with_crc);
  tx_buf_fmt.remote_tx_req = 0;

  LL_StatusETypeDef status = LL_CAN_TransmitPTB_CPU(CAN1, &tx_buf_fmt, (uint32_t*)data_with_crc);

  if(status != LL_OK)
    {
      printf("Error sending data with CRC\n");
    }
}

void can_send_float_data(uint32_t cmd, float data, int scale_factor)
{
  CAN_TxBufFormatTypeDef tx_buf_fmt;
  int scaled_value = (int)(data * scale_factor);

  uint8_t high_bit = (scaled_value >> 8) & 0xFF;
  uint8_t low_bit = scaled_value & 0xFF;

  uint8_t data_bytes[4] = {0, cmd, low_bit, high_bit};

  tx_buf_fmt.id_extension = 1;
  tx_buf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(llc.can_addr);
  tx_buf_fmt.data_len_code = sizeof(data_bytes);
  tx_buf_fmt.remote_tx_req = 0;

  LL_StatusETypeDef status = LL_CAN_TransmitPTB_CPU(CAN1, &tx_buf_fmt, (uint32_t*)data_bytes);

  if(status != LL_OK)
    {
      printf("Error sending data\n");
    }
}

RAMCODE
void can_send_float_data_crc(float data, int scale_factor)
{
  CAN_TxBufFormatTypeDef tx_buf_fmt;
  int scaled_value = (int)(data * scale_factor);

  uint8_t high_bit = (scaled_value >> 8) & 0xFF;
  uint8_t low_bit = scaled_value & 0xFF;

  uint8_t data_bytes[2] = {low_bit, high_bit};

  uint8_t crc = crc8(data_bytes, sizeof(data_bytes));
  uint8_t data_with_crc[3] = {data_bytes[0], data_bytes[1], crc};

  tx_buf_fmt.id_extension = 1;
  tx_buf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(llc.can_addr);
  tx_buf_fmt.data_len_code = sizeof(data_with_crc);
  tx_buf_fmt.remote_tx_req = 0;

  LL_StatusETypeDef status = LL_CAN_TransmitPTB_CPU(CAN1, &tx_buf_fmt, (uint32_t*)data_with_crc);

  if(status != LL_OK)
    {
      printf("Error sending data\n");
    }
}

void can_send_data_init(void)
{
  can_data.power_para.return_bit = RETURN_BIT_POWER;

  can_data.version_info.version_cmd = VERSION_CMD;
  can_data.version_info.compnet_type = COMPNET_TYPE;
  can_data.version_info.year = YEAR_NUM;
  can_data.version_info.month = MONTH_NUM;
  can_data.version_info.day = DAY_NUM;
  can_data.version_info.version_low_bit = VERSION_CODE & 0xFF;
  can_data.version_info.version_high_bit = (VERSION_CODE >> 8) & 0xFF;

  hld_can_data.power_para.return_bit = RETURN_BIT_POWER;
  hld_can_data.version_info.version_cmd = VERSION_CMD;
  hld_can_data.version_info.compnet_type = COMPNET_TYPE;
  hld_can_data.version_info.year = HLD_YEAR_NUM;
  hld_can_data.version_info.month = HLD_MONTH_NUM;
  hld_can_data.version_info.day = HLD_DAY_NUM;
  hld_can_data.version_info.version_low_bit = HLD_VERSION_CODE & 0xFF;
  hld_can_data.version_info.version_high_bit = (HLD_VERSION_CODE >> 8) & 0xFF;
}

RAMCODE
int findDevice(uint32_t id)
{
  for(int i = 0; i < deviceCount; i++)
    {
      if(devices[i].id == id)
        {
          return i;
        }
    }
  return -1;
}

RAMCODE
void LL_CAN_RxCallback(CAN_TypeDef* Instance)
{


    if(user_can_ctrl.rx_cnt < USER_CAN_RX_FRM_NUMS) {
        uint32_t received_id = user_can_ctrl.rxbuf_fmt[user_can_ctrl.rx_cnt].id;
        uint8_t can_cmd = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][1];

        switch(received_id) {
            case USER_CAN_STD_FRM_ID:
                process_common_commands(can_cmd, received_id);
                break;

            case 0xB0000:
                // HLD�豸����
                process_common_commands(can_cmd, received_id);
                break;

            case IAP_CAN_ID:
                // IAP target: LLC handles itself, PFC is forwarded through UART0.
                if(user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][1] == 0x41) {
                    if(user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0] == 2) {
                        NVIC_SystemReset();
                    } else if(user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0] == 1) {
                        pfc_iap_forward_can_frame(user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt],
                                                  user_can_ctrl.rxbuf_fmt[user_can_ctrl.rx_cnt].data_len_code);
                    }
                }
                break;

            default:
                // ����Ƿ�ΪLLC�豸��Χ��ID��0xA0000-0xA0007��
                if(received_id == llc.can_addr) {
                    process_common_commands(can_cmd, received_id);
                }
                break;
        }

        // ���½��ռ�����
        user_can_ctrl.rx_cnt++;
        if(user_can_ctrl.rx_cnt >= USER_CAN_RX_FRM_NUMS) {
            user_can_ctrl.rx_cnt = 0;
        }

        // ��������CAN�����ж�
        LL_CAN_Receive_IT(user_can_ctrl.Instance, 
                         &user_can_ctrl.rxbuf_fmt[user_can_ctrl.rx_cnt], 
                         user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt]);
    }
}

void can_addr_set()
{
  llc.can_addr_check.can1_level = (LL_GPIO_ReadPin(ADDR1_PIN_PORT, ADDR1_PIN)) ? 1 : 0;
  llc.can_addr_check.can2_level = (LL_GPIO_ReadPin(ADDR2_PIN_PORT, ADDR2_PIN)) ? 1 : 0;
  llc.can_addr_check.can3_level = (LL_GPIO_ReadPin(ADDR3_PIN_PORT, ADDR3_PIN)) ? 1 : 0;

  switch (llc.can_addr_check.can_addr_level_bits)
    {
    case 0:   // 000
      llc.can_addr = 0xA0000;
      break;
    case 1:   // 001
      llc.can_addr = 0xA0001;
      break;
    case 2:   // 010
      llc.can_addr = 0xA0002;
      break;
    case 3:   // 011
      llc.can_addr = 0xA0003;
      break;
    case 4:   // 100
      llc.can_addr = 0xA0004;
      break;
    case 5:   // 101
      llc.can_addr = 0xA0005;
      break;
    case 6:   // 110
      llc.can_addr = 0xA0006;
      break;
    case 7:   // 111
      llc.can_addr = 0xA0007;
      break;
    default:
      llc.can_addr = 0xA0000; // Ĭ��ֵ
      break;
    }
	 dev_index = llc.can_addr & 0x7;
  llc.addr_set_flag = 1;
	LL_NVIC_EnableIRQ(CAN1_IRQn);
}