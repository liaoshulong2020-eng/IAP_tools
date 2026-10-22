// 锟较诧拷锟斤拷锟斤拷锟竭硷拷锟斤拷CAN锟斤拷锟秸达拷锟斤拷

#include "main.h"
#include "iap_runtime.h"
#include "can_app.h"
#include "variables_define_app.h"
#include "uart_app.h"
#include "func_app.h"

#define KP_KI_SCALE_FACTOR 100

DeviceInfo devices[6];
int deviceCount = 0;
#define USER_CAN_RX_FRM_NUMS (1)
uint8_t dev_index ;

void User_CAN_RxCpltCallback(void);

/* Ask the PFC APP to enter its bootloader before LLC resets into gateway mode. */
static void enter_pfc_iap_then_reset(uint32_t device_id)
{
  (void)uart_enter_pfc_iap(device_id);
  delay_ms(10);
  NVIC_SystemReset();
}

static inline void split_int16(uint8_t *high, uint8_t *low, int value)
{
  uint16_t data = (uint16_t)value;
  *high = (uint8_t)((data >> 8) & 0xFF);
  *low = (uint8_t)(data & 0xFF);
}

typedef struct __CAN_UserCtrlTypeDef
{
  CAN_TypeDef* Instance;
  volatile uint32_t tx_cnt;
  volatile uint32_t rx_cnt;
  CAN_TxBufFormatTypeDef txbuf_fmt;
  CAN_RxBufFormatTypeDef rxbuf_fmt[USER_CAN_RX_FRM_NUMS];
  uint8_t rx_buf[USER_CAN_RX_FRM_NUMS][64] __attribute__((aligned(4)));
} CAN_UserCtrlTypeDef;

CAN_UserCtrlTypeDef user_can_ctrl;

// CAN锟斤拷锟斤拷锟斤拷时锟斤拷锟斤拷
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
// 16位锟斤拷锟捷达拷锟斤拷锟斤拷锟斤拷
// =============================================================================

// 16位锟斤拷锟斤拷锟斤拷取锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷CRC锟斤拷
static inline uint16_t extract_16bit_data(uint8_t start_index)
{
  uint8_t current_rx_index = user_can_ctrl.rx_cnt;
  uint8_t low_byte = user_can_ctrl.rx_buf[current_rx_index][start_index];
  uint8_t high_byte = user_can_ctrl.rx_buf[current_rx_index][start_index + 1];
  return (high_byte << 8) | low_byte;
}

// 16位锟斤拷锟捷达拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷CRC锟斤拷
RAMCODE
bool process_16bit_simple(uint8_t cmd, uint16_t* result_value)
{
  uint8_t current_rx_index = user_can_ctrl.rx_cnt;

  // 锟斤拷证锟斤拷锟捷筹拷锟饺ｏ拷锟斤拷锟斤拷锟斤拷要4锟街节ｏ拷锟斤拷锟斤拷+锟斤拷锟斤拷+2锟街斤拷锟斤拷锟捷ｏ拷
  if (user_can_ctrl.rxbuf_fmt[current_rx_index].data_len_code < 4)
    {
      return false;
    }

  // 锟斤拷证锟斤拷锟斤拷
  uint8_t received_cmd = user_can_ctrl.rx_buf[current_rx_index][1];
  if(received_cmd != cmd)
    {
      return false;
    }

  // 锟斤拷取16位锟斤拷锟斤拷
  *result_value = extract_16bit_data(2);
  return true;
}

// 16位锟斤拷锟捷达拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷CRC锟斤拷
RAMCODE
bool process_16bit_with_crc(uint8_t cmd, uint16_t* result_value)
{
  uint8_t current_rx_index = user_can_ctrl.rx_cnt;

  // 锟斤拷证锟斤拷锟捷筹拷锟饺ｏ拷锟斤拷锟斤拷锟斤拷要5锟街节ｏ拷锟斤拷锟斤拷+锟斤拷锟斤拷+2锟街斤拷锟斤拷锟斤拷+CRC锟斤拷
  if (user_can_ctrl.rxbuf_fmt[current_rx_index].data_len_code < 5)
    {
      return false;
    }

  // 锟斤拷证锟斤拷锟斤拷
  uint8_t received_cmd = user_can_ctrl.rx_buf[current_rx_index][1];
  if(received_cmd != cmd)
    {
      return false;
    }

  // 锟斤拷取锟斤拷锟捷猴拷CRC
  uint8_t low_byte = user_can_ctrl.rx_buf[current_rx_index][2];
  uint8_t high_byte = user_can_ctrl.rx_buf[current_rx_index][3];
  uint8_t received_crc = user_can_ctrl.rx_buf[current_rx_index][4];

  // 锟斤拷证CRC
  uint8_t crc_data[3] = {received_cmd, low_byte, high_byte};
  uint8_t calculated_crc = crc8(crc_data, sizeof(crc_data));

  if(calculated_crc != received_crc)
    {
      return false; // CRC校锟斤拷失锟斤拷
    }

  // 锟斤拷装锟斤拷锟斤拷
  *result_value = (high_byte << 8) | low_byte;
  return true;
}

// 16位锟斤拷锟捷凤拷锟酵猴拷锟斤拷锟斤拷锟斤拷锟斤拷CRC锟斤拷
RAMCODE
void can_send_16bit_simple(uint8_t cmd, uint16_t value)
{
  CAN_TxBufFormatTypeDef tx_buf_fmt;

  uint8_t data_bytes[4] =
  {
    0x00,                          // 锟斤拷锟斤拷锟街斤拷
    cmd,                           // 锟斤拷锟斤拷
    (uint8_t)(value & 0xFF),       // 锟斤拷锟街斤拷
    (uint8_t)((value >> 8) & 0xFF) // 锟斤拷锟街斤拷
  };

  tx_buf_fmt.id_extension = 1;
  tx_buf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(llc.can_addr);
  tx_buf_fmt.data_len_code = sizeof(data_bytes);
  tx_buf_fmt.remote_tx_req = 0;

  LL_CAN_TransmitPTB_CPU(CAN1, &tx_buf_fmt, (uint32_t*)data_bytes);
}

// 16位锟斤拷锟捷凤拷锟酵猴拷锟斤拷锟斤拷锟斤拷CRC锟斤拷
RAMCODE
void can_send_16bit_with_crc(uint8_t cmd, uint16_t value)
{
  CAN_TxBufFormatTypeDef tx_buf_fmt;

  uint8_t low_byte = (uint8_t)(value & 0xFF);
  uint8_t high_byte = (uint8_t)((value >> 8) & 0xFF);

  // 锟斤拷锟斤拷CRC
  uint8_t crc_data[3] = {cmd, low_byte, high_byte};
  uint8_t crc = crc8(crc_data, sizeof(crc_data));

  uint8_t data_bytes[5] =
  {
    0x00,      // 锟斤拷锟斤拷锟街斤拷
    cmd,       // 锟斤拷锟斤拷
    low_byte,  // 锟斤拷锟街斤拷
    high_byte, // 锟斤拷锟街斤拷
    crc        // CRC
  };

  tx_buf_fmt.id_extension = 1;
  tx_buf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(llc.can_addr);
  tx_buf_fmt.data_len_code = sizeof(data_bytes);
  tx_buf_fmt.remote_tx_req = 0;

  LL_CAN_TransmitPTB_CPU(CAN1, &tx_buf_fmt, (uint32_t*)data_bytes);
}

// =============================================================================
// 32位锟斤拷锟捷达拷锟斤拷锟斤拷锟斤拷
// =============================================================================

// 32位锟斤拷锟斤拷锟斤拷取锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷CRC锟斤拷
static inline uint32_t extract_32bit_data(uint8_t start_index)
{
  uint8_t current_rx_index = user_can_ctrl.rx_cnt;
  uint8_t byte0 = user_can_ctrl.rx_buf[current_rx_index][start_index];     // 锟斤拷锟斤拷纸锟?
  uint8_t byte1 = user_can_ctrl.rx_buf[current_rx_index][start_index + 1];
  uint8_t byte2 = user_can_ctrl.rx_buf[current_rx_index][start_index + 2];
  uint8_t byte3 = user_can_ctrl.rx_buf[current_rx_index][start_index + 3]; // 锟斤拷锟斤拷纸锟?

  return ((uint32_t)byte3 << 24) |
         ((uint32_t)byte2 << 16) |
         ((uint32_t)byte1 << 8) |
         byte0;
}

// 32位锟斤拷锟捷达拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷CRC锟斤拷
RAMCODE
bool process_32bit_simple(uint8_t cmd, uint32_t* result_value)
{
  uint8_t current_rx_index = user_can_ctrl.rx_cnt;

  // 锟斤拷证锟斤拷锟捷筹拷锟饺ｏ拷锟斤拷锟斤拷锟斤拷要6锟街节ｏ拷锟斤拷锟斤拷+锟斤拷锟斤拷+4锟街斤拷锟斤拷锟捷ｏ拷
  if (user_can_ctrl.rxbuf_fmt[current_rx_index].data_len_code < 6)
    {
      return false;
    }

  // 锟斤拷证锟斤拷锟斤拷
  uint8_t received_cmd = user_can_ctrl.rx_buf[current_rx_index][1];
  if(received_cmd != cmd)
    {
      return false;
    }

  // 锟斤拷取32位锟斤拷锟斤拷
  *result_value = extract_32bit_data(2);
  return true;
}

// 32位锟斤拷锟斤拷CRC锟斤拷锟斤拷锟斤拷锟斤拷
RAMCODE
bool process_32bit_with_crc(uint8_t cmd, float* result_value, int scale_factor)
{
  uint8_t current_rx_index = user_can_ctrl.rx_cnt;

  // 锟斤拷证锟斤拷锟捷筹拷锟饺ｏ拷锟斤拷锟斤拷锟斤拷要7锟街节ｏ拷锟斤拷锟斤拷+锟斤拷锟斤拷+4锟街斤拷锟斤拷锟斤拷+CRC锟斤拷
  if (user_can_ctrl.rxbuf_fmt[current_rx_index].data_len_code < 7)
    {
      return false;
    }

  // 锟斤拷取锟斤拷锟秸碉拷锟斤拷锟斤拷锟斤拷
  uint8_t received_cmd = user_can_ctrl.rx_buf[current_rx_index][1];
  uint8_t data_byte0 = user_can_ctrl.rx_buf[current_rx_index][2];  // 锟斤拷锟轿伙拷纸锟?
  uint8_t data_byte1 = user_can_ctrl.rx_buf[current_rx_index][3];
  uint8_t data_byte2 = user_can_ctrl.rx_buf[current_rx_index][4];
  uint8_t data_byte3 = user_can_ctrl.rx_buf[current_rx_index][5];  // 锟斤拷锟轿伙拷纸锟?
  uint8_t received_crc = user_can_ctrl.rx_buf[current_rx_index][6];

  // 锟斤拷证锟斤拷锟斤拷锟角凤拷匹锟斤拷
  if(received_cmd != cmd)
    {
      return false;
    }

  // 锟截斤拷CRC锟斤拷锟斤拷锟斤拷锟捷ｏ拷锟斤拷锟斤拷+4锟街斤拷锟斤拷锟捷ｏ拷
  uint8_t crc_data[5] = {received_cmd, data_byte0, data_byte1, data_byte2, data_byte3};
  uint8_t calculated_crc = crc8(crc_data, sizeof(crc_data));

  // 锟斤拷证CRC
  if(calculated_crc != received_crc)
    {
      return false; // CRC校锟斤拷失锟斤拷
    }

  // 锟斤拷锟斤拷32位锟斤拷锟斤拷
  uint32_t raw_value = ((uint32_t)data_byte3 << 24) |
                       ((uint32_t)data_byte2 << 16) |
                       ((uint32_t)data_byte1 << 8) |
                       data_byte0;

  // 转锟斤拷为锟斤拷锟斤拷锟斤拷
  float float_value = (float)raw_value / (float)scale_factor;

//  // 锟斤拷围锟斤拷椋↘P/KI通锟斤拷锟斤拷0-100000锟斤拷围锟节ｏ拷
//  if(float_value < 0.0f || float_value > 100000.0f)
//    {
//      return false; // 锟斤拷值锟斤拷锟斤拷锟斤拷围
//    }

  // 锟斤拷锟斤拷锟斤拷
  *result_value = float_value;
  return true;
}

// 32位锟斤拷锟捷达拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷CRC锟斤拷锟斤拷锟斤拷原始锟斤拷锟捷ｏ拷
RAMCODE
bool process_32bit_with_crc_raw(uint8_t cmd, uint32_t* result_value)
{
  uint8_t current_rx_index = user_can_ctrl.rx_cnt;

  // 锟斤拷证锟斤拷锟捷筹拷锟饺ｏ拷锟斤拷锟斤拷锟斤拷要7锟街节ｏ拷锟斤拷锟斤拷+锟斤拷锟斤拷+4锟街斤拷锟斤拷锟斤拷+CRC锟斤拷
  if (user_can_ctrl.rxbuf_fmt[current_rx_index].data_len_code < 7)
    {
      return false;
    }

  // 锟斤拷取锟斤拷锟秸碉拷锟斤拷锟斤拷锟斤拷
  uint8_t received_cmd = user_can_ctrl.rx_buf[current_rx_index][1];
  uint8_t data_byte0 = user_can_ctrl.rx_buf[current_rx_index][2];
  uint8_t data_byte1 = user_can_ctrl.rx_buf[current_rx_index][3];
  uint8_t data_byte2 = user_can_ctrl.rx_buf[current_rx_index][4];
  uint8_t data_byte3 = user_can_ctrl.rx_buf[current_rx_index][5];
  uint8_t received_crc = user_can_ctrl.rx_buf[current_rx_index][6];

  // 锟斤拷证锟斤拷锟斤拷
  if(received_cmd != cmd)
    {
      return false;
    }

  // 锟斤拷证CRC
  uint8_t crc_data[5] = {received_cmd, data_byte0, data_byte1, data_byte2, data_byte3};
  uint8_t calculated_crc = crc8(crc_data, sizeof(crc_data));

  if(calculated_crc != received_crc)
    {
      return false;
    }

  // 锟斤拷锟斤拷32位锟斤拷锟斤拷
  *result_value = ((uint32_t)data_byte3 << 24) |
                  ((uint32_t)data_byte2 << 16) |
                  ((uint32_t)data_byte1 << 8) |
                  data_byte0;

  return true;
}

// 32位锟斤拷锟捷凤拷锟酵猴拷锟斤拷锟斤拷锟斤拷锟斤拷CRC锟斤拷
RAMCODE
void can_send_32bit_simple(uint8_t cmd, uint32_t value)
{
  CAN_TxBufFormatTypeDef tx_buf_fmt;

  uint8_t data_bytes[6] =
  {
    0x00,                             // 锟斤拷锟斤拷锟街斤拷
    cmd,                              // 锟斤拷锟斤拷
    (uint8_t)(value & 0xFF),          // byte0 (锟斤拷锟斤拷纸锟?)
    (uint8_t)((value >> 8) & 0xFF),   // byte1
    (uint8_t)((value >> 16) & 0xFF),  // byte2
    (uint8_t)((value >> 24) & 0xFF)   // byte3 (锟斤拷锟斤拷纸锟?)
  };

  tx_buf_fmt.id_extension = 1;
  tx_buf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(llc.can_addr);
  tx_buf_fmt.data_len_code = sizeof(data_bytes);
  tx_buf_fmt.remote_tx_req = 0;

  LL_CAN_TransmitPTB_CPU(CAN1, &tx_buf_fmt, (uint32_t*)data_bytes);
}

// 32位锟斤拷锟捷凤拷锟酵猴拷锟斤拷锟斤拷锟斤拷CRC锟斤拷
RAMCODE
void can_send_32bit_with_crc(uint8_t cmd, uint32_t value)
{
  CAN_TxBufFormatTypeDef tx_buf_fmt;

  uint8_t byte0 = (uint8_t)(value & 0xFF);
  uint8_t byte1 = (uint8_t)((value >> 8) & 0xFF);
  uint8_t byte2 = (uint8_t)((value >> 16) & 0xFF);
  uint8_t byte3 = (uint8_t)((value >> 24) & 0xFF);

  // 锟斤拷锟斤拷CRC
  uint8_t crc_data[5] = {cmd, byte0, byte1, byte2, byte3};
  uint8_t crc = crc8(crc_data, sizeof(crc_data));

  uint8_t data_bytes[7] =
  {
    0x00,   // 锟斤拷锟斤拷锟街斤拷
    cmd,    // 锟斤拷锟斤拷
    byte0,  // 锟斤拷锟斤拷纸锟?
    byte1,
    byte2,
    byte3,  // 锟斤拷锟斤拷纸锟?
    crc     // CRC
  };

  tx_buf_fmt.id_extension = 1;
  tx_buf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(llc.can_addr);
  tx_buf_fmt.data_len_code = sizeof(data_bytes);
  tx_buf_fmt.remote_tx_req = 0;

  LL_CAN_TransmitPTB_CPU(CAN1, &tx_buf_fmt, (uint32_t*)data_bytes);
}

// 32位锟斤拷锟斤拷锟斤拷锟斤拷锟酵猴拷锟斤拷锟斤拷锟斤拷CRC锟斤拷锟斤拷锟脚ｏ拷
RAMCODE
void can_send_32bit_float_with_crc(uint8_t cmd, float value, int scale_factor)
{
  uint32_t scaled_value = (uint32_t)(value * scale_factor);
  can_send_32bit_with_crc(cmd, scaled_value);
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

        case CMD_OVERTEMP_POINT:  // 锟斤拷锟铰点（16位锟斤拷锟斤拷锟斤拷CRC锟斤拷
        {
            uint16_t rx_data_16;
            if(process_16bit_simple(CMD_OVERTEMP_POINT, &rx_data_16)) {
                user_data.over_temp_point = (float)(rx_data_16);
                llc.protection_point.over_temp_point = user_data.over_temp_point;
            }
        }
        break;

        case CMD_OVERTEMP_REC_POINT:  // 锟斤拷锟铰恢革拷锟姐（16位锟斤拷锟斤拷CRC锟斤拷
        {
            uint16_t rx_data_16;
            if(process_16bit_with_crc(CMD_OVERTEMP_REC_POINT, &rx_data_16)) {
                user_data.over_temp_rec_point = (float)(rx_data_16);
                llc.protection_point.over_temp_rec_point = user_data.over_temp_rec_point;
            }
        }
        break;

        case CMD_FACTOR_VOLTAGE:  // 校准锟斤拷压锟斤拷16位锟斤拷锟斤拷锟斤拷CRC锟斤拷
        {
            uint16_t rx_data_16;
            if(process_16bit_simple(CMD_FACTOR_VOLTAGE, &rx_data_16)) {
                llc.factor_voltage = rx_data_16;
            }
        }
        break;

        case CMD_THEOR_VOLTAGE:  // 锟斤拷锟桔碉拷压锟斤拷16位锟斤拷锟斤拷锟斤拷CRC锟斤拷
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
								float factor_voltage_float = llc.factor_voltage / 1000.0f; // 锟斤拷前锟斤拷锟斤拷/锟斤拷位锟斤拷锟斤拷示值
								float theor_voltage_float  = llc.theor_voltage  / 1000.0f; // 锟斤拷锟矫憋拷锟斤拷实值

								if((factor_voltage_float > 1.0f) && (theor_voltage_float > 1.0f))
								{
										/*
										 * 锟斤拷锟斤拷目锟斤拷校准锟斤拷
										 * 锟斤拷锟界当前目锟斤拷48.19V锟斤拷实锟斤拷应为48.00V
										 * 锟斤拷目锟斤拷 = 原目锟斤拷 / 锟斤拷前锟斤拷示值 * 锟斤拷实值
										 */
										user_data.coef_target = 
												(llc.vbus_target / factor_voltage_float) * theor_voltage_float;

										/*
										 * 锟斤拷示锟斤拷锟斤拷锟斤拷
										 * 锟斤拷位锟斤拷锟斤拷示值 = llc.vbus_rel - llc.can_com_voltag_delta
										 * 锟斤拷锟斤拷 48.19 - 0.19 = 48.00
										 */
										user_data.vout_can_delta = 
												factor_voltage_float - theor_voltage_float;

										/*
										 * 目锟斤拷锟窖癸拷锟斤拷48V锟斤拷偏锟斤拷锟斤拷
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

        case CMD_KP:  // KP锟斤拷锟斤拷锟斤拷32位锟斤拷锟斤拷CRC锟斤拷锟斤拷锟姐）
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

        case CMD_KI:  // KI锟斤拷锟斤拷锟斤拷32位锟斤拷锟斤拷CRC锟斤拷锟斤拷锟姐）
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

        case CMD_TEST:  // 锟斤拷锟皆诧拷锟斤拷锟斤拷32位锟斤拷锟斤拷CRC锟斤拷
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

        case CMD_TEST2:  // 锟斤拷锟皆诧拷锟斤拷2锟斤拷32位锟斤拷锟斤拷CRC锟斤拷
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

        case CMD_LLC_TEMP_PROTECT:
        {
            can_ctrl_delay();
            llc.user_can.llc.llc_temp_point.frame_count = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0];
            int over_temp = (int)llc.protection_point.over_temp_point;
            int over_temp_rec = (int)llc.protection_point.over_temp_rec_point;
            split_int16(&llc.user_can.llc.llc_temp_point.llc_over_temp_high_bit,
                        &llc.user_can.llc.llc_temp_point.llc_over_temp_low_bit,
                        over_temp);
            split_int16(&llc.user_can.llc.llc_temp_point.llc_over_temp_rec_high_bit,
                        &llc.user_can.llc.llc_temp_point.llc_over_temp_rec_low_bit,
                        over_temp_rec);
            can_send_data((void*)&llc.user_can.llc.llc_temp_point, sizeof(llc.user_can.llc.llc_temp_point));
        }
        break;

        case CMD_LLC_VOLTAGE_PROTECT:
        {
            can_ctrl_delay();
            llc.user_can.llc.llc_voltage_protection_point.frame_count = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0];

            int uvp_voltage = (int)(llc.protection_point.under_voltage_point * 1000.0f);
            int ovp_voltage = (int)(llc.protection_point.over_voltage_point * 1000.0f);
            int dac_ovp_voltage = (int)(VOUT_OVER_VOLTAGE_DAC * 1000.0f);

            split_int16(&llc.user_can.llc.llc_voltage_protection_point.llc_out_ovp_soft_high_bit,
                        &llc.user_can.llc.llc_voltage_protection_point.llc_out_ovp_soft_low_bit,
                        ovp_voltage);
            split_int16(&llc.user_can.llc.llc_voltage_protection_point.llc_out_ovp_dac_high_bit,
                        &llc.user_can.llc.llc_voltage_protection_point.llc_out_ovp_dac_low_bit,
                        dac_ovp_voltage);
            split_int16(&llc.user_can.llc.llc_voltage_protection_point.llc_out_uvp_high_bit,
                        &llc.user_can.llc.llc_voltage_protection_point.llc_out_uvp_low_bit,
                        uvp_voltage);

            can_send_data((void*)&llc.user_can.llc.llc_voltage_protection_point, sizeof(llc.user_can.llc.llc_voltage_protection_point));
        }
        break;

        case CMD_LLC_OCP_PROTECT:
        {
            can_ctrl_delay();
            llc.user_can.llc.llc_over_current_point.frame_count = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0];

            int ocp_soft_point = (int)(llc.protection_point.over_current_point * 10.0f);
            int ocp_rec_soft_point = (int)(IOUT_REC_CURRENT * 10.0f);
            int iout_target_point = (int)(IOUT_TARGET_CURRENT * 10.0f);

            split_int16(&llc.user_can.llc.llc_over_current_point.llc_iout_target_high_bit,
                        &llc.user_can.llc.llc_over_current_point.llc_iout_target_low_bit,
                        iout_target_point);
            split_int16(&llc.user_can.llc.llc_over_current_point.llc_ocp_soft_high_bit,
                        &llc.user_can.llc.llc_over_current_point.llc_ocp_soft_low_bit,
                        ocp_soft_point);
            split_int16(&llc.user_can.llc.llc_over_current_point.llc_ocp_rec_soft_high_bit,
                        &llc.user_can.llc.llc_over_current_point.llc_ocp_rec_soft_low_bit,
                        ocp_rec_soft_point);

            can_send_data((void*)&llc.user_can.llc.llc_over_current_point, sizeof(llc.user_can.llc.llc_over_current_point));
        }
        break;

        case CMD_LLC_OSP_PROTECT:
        {
            can_ctrl_delay();
            llc.user_can.llc.llc_short_current_point.frame_count = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0];

            int osp_soft_point = (int)(IOUT_SHORT_CURRENT * 10.0f);
            int osp_hard_point = (int)(IOUT_OCP_CURRENT_DAC * 10.0f);

            split_int16(&llc.user_can.llc.llc_short_current_point.llc_short_soft_high_bit,
                        &llc.user_can.llc.llc_short_current_point.llc_short_soft_low_bit,
                        osp_soft_point);
            split_int16(&llc.user_can.llc.llc_short_current_point.llc_short_hard_high_bit,
                        &llc.user_can.llc.llc_short_current_point.llc_short_hard_low_bit,
                        osp_hard_point);

            can_send_data((void*)&llc.user_can.llc.llc_short_current_point, sizeof(llc.user_can.llc.llc_short_current_point));
        }
        break;

        case CMD_LLC_OUT_PARA:
        {
            can_ctrl_delay();
            llc.user_can.llc.llc_voltage_output_para.frame_count = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0];

            int llc_vbus_target = (int)(llc.vbus_target * 1000.0f);
            int llc_vbus_ref = (int)(llc.vbus_ref * 1000.0f);
            int llc_coef_target = (int)(llc.coef_target * 1000.0f);

            split_int16(&llc.user_can.llc.llc_voltage_output_para.llc_vbus_target_high_bit,
                        &llc.user_can.llc.llc_voltage_output_para.llc_vbus_target_low_bit,
                        llc_vbus_target);
            split_int16(&llc.user_can.llc.llc_voltage_output_para.llc_coef_target_high_bit,
                        &llc.user_can.llc.llc_voltage_output_para.llc_coef_target_low_bit,
                        llc_coef_target);
            split_int16(&llc.user_can.llc.llc_voltage_output_para.llc_vbus_ref_high_bit,
                        &llc.user_can.llc.llc_voltage_output_para.llc_vbus_ref_low_bit,
                        llc_vbus_ref);

            can_send_data((void*)&llc.user_can.llc.llc_voltage_output_para, sizeof(llc.user_can.llc.llc_voltage_output_para));
        }
        break;

        case CMD_PFC_INPUT_OVP:
        {
            can_ctrl_delay();
            llc.user_can.pfc.protect.in_ovp.frame_count = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0];
            can_send_data((void*)&llc.user_can.pfc.protect.in_ovp, sizeof(llc.user_can.pfc.protect.in_ovp));
        }
        break;

        case CMD_PFC_INPUT_UVP:
        {
            can_ctrl_delay();
            llc.user_can.pfc.protect.in_uvp.frame_count = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0];
            can_send_data((void*)&llc.user_can.pfc.protect.in_uvp, sizeof(llc.user_can.pfc.protect.in_uvp));
        }
        break;

        case CMD_PFC_OUTPUT_OVP:
        {
            can_ctrl_delay();
            llc.user_can.pfc.protect.out_ovp.frame_count = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0];
            can_send_data((void*)&llc.user_can.pfc.protect.out_ovp, sizeof(llc.user_can.pfc.protect.out_ovp));
        }
        break;

        case CMD_PFC_OUTPUT_UVP:
        {
            can_ctrl_delay();
            llc.user_can.pfc.protect.out_uvp.frame_count = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0];
            can_send_data((void*)&llc.user_can.pfc.protect.out_uvp, sizeof(llc.user_can.pfc.protect.out_uvp));
        }
        break;

        case CMD_PFC_INPUT_OCP:
        {
            can_ctrl_delay();
            llc.user_can.pfc.protect.in_ocp.frame_count = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0];
            can_send_data((void*)&llc.user_can.pfc.protect.in_ocp, sizeof(llc.user_can.pfc.protect.in_ocp));
        }
        break;

        case CMD_PFC_DATA:
        {
            can_ctrl_delay();
            llc.user_can.pfc.data.vbus.frame_count = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0];
            can_send_data((void*)&llc.user_can.pfc.data.vbus, sizeof(llc.user_can.pfc.data.vbus));
        }
        break;

        case CMD_PFC_DATA_LIVE1:
        {
            can_ctrl_delay();
            llc.user_can.pfc.live1.frame_count = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0];
            can_send_data((void*)&llc.user_can.pfc.live1, sizeof(llc.user_can.pfc.live1));
        }
        break;

        case CMD_PFC_DATA_LIVE2:
        {
            can_ctrl_delay();
            llc.user_can.pfc.live2.frame_count = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0];
            can_send_data((void*)&llc.user_can.pfc.live2, sizeof(llc.user_can.pfc.live2));
        }
        break;
        default:
            // 未知锟斤拷锟筋，锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷志锟斤拷录
            break;
    }
}

void can_init_app(void)
{
  CAN_UserCfgTypeDef can_user_cfg;
  CAN_AcceptFilCfgTypeDef can_acpt_fil_cfg[5];

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

  can_acpt_fil_cfg[4] = can_acpt_fil_cfg[3];
  can_acpt_fil_cfg[4].slot = CAN_ACCEPT_FILT_SLOT_4;
  can_acpt_fil_cfg[4].code_val = iap_runtime_can_id();

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

  // 锟斤拷锟矫斤拷锟秸伙拷锟斤拷锟斤拷锟斤拷式
  for(int i = 0; i < USER_CAN_RX_FRM_NUMS; i++)
    {
      user_can_ctrl.rxbuf_fmt[i].id_extension = 1;
      user_can_ctrl.rxbuf_fmt[i].data_len_code = 8;
    }

  // 锟斤拷锟斤拷CAN锟斤拷锟斤拷锟叫讹拷
  LL_CAN_Receive_IT(user_can_ctrl.Instance, &user_can_ctrl.rxbuf_fmt[0],
                    (uint32_t *)user_can_ctrl.rx_buf[0]);
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

  uint8_t data_with_crc[8];
  if(data_buf == NULL || data_size == 0 || data_size > 7)return;
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

  llc.user_can.llc.llc_voltage_protection_point.return_bit = RETURN_BIT_VOLTAGE_PROTECT;
  llc.user_can.llc.llc_over_current_point.return_bit = RETURN_BIT_OCP_PROTECT;
  llc.user_can.llc.llc_short_current_point.return_bit = RETURN_BIT_OSP_PROTECT;
  llc.user_can.llc.llc_voltage_output_para.return_bit = RETURN_BIT_VOLTAGE_PARA;
  llc.user_can.llc.llc_temp_point.return_bit = RETURN_BIT_TEMP_PROTECT;

  llc.user_can.pfc.protect.in_ovp.return_bit = RETURN_BIT_PFC_INPUT_OVP;
  llc.user_can.pfc.protect.in_uvp.return_bit = RETURN_BIT_PFC_INPUT_UVP;
  llc.user_can.pfc.protect.out_ovp.return_bit = RETURN_BIT_PFC_OUTPUT_OVP;
  llc.user_can.pfc.protect.out_uvp.return_bit = RETURN_BIT_PFC_OUTPUT_UVP;
  llc.user_can.pfc.protect.in_ocp.return_bit = RETURN_BIT_PFC_INPUT_OCP;
  llc.user_can.pfc.data.vbus.return_bit = RETURN_BIT_PFC_DATA;
  llc.user_can.pfc.live1.return_bit = RETURN_BIT_PFC_LIVE1;
  llc.user_can.pfc.live2.return_bit = RETURN_BIT_PFC_LIVE2;
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

        if(iap_runtime_accept_id(received_id) &&
           (user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0] == 2 ||
            user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0] == 1) && can_cmd == 0x41)
        {
            if(user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0] == 1) enter_pfc_iap_then_reset(received_id);
            else NVIC_SystemReset();
        }

        switch(received_id) {
            case USER_CAN_STD_FRM_ID:
                process_common_commands(can_cmd, received_id);
                break;

            case 0xB0000:
                // HLD锟借备锟斤拷锟斤拷
                process_common_commands(can_cmd, received_id);
                break;

            case IAP_CAN_ID:
                // IAP锟斤拷锟斤拷
                if((user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0] == 2 ||  // LLC: 2, PFC: 1
                    user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0] == 1) &&
                   user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][1] == 0x41) {
                    if(user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0] == 1) enter_pfc_iap_then_reset(received_id);
                    else NVIC_SystemReset();
                }
                break;

            default:
                // 锟斤拷锟斤拷欠锟轿狶LC锟借备锟斤拷围锟斤拷ID锟斤拷0xA0000-0xA0007锟斤拷
                if(received_id == llc.can_addr) {
                    process_common_commands(can_cmd, received_id);
                }
                break;
        }

        // 锟斤拷锟铰斤拷锟秸硷拷锟斤拷锟斤拷
        user_can_ctrl.rx_cnt++;
        if(user_can_ctrl.rx_cnt >= USER_CAN_RX_FRM_NUMS) {
            user_can_ctrl.rx_cnt = 0;
        }

        // 锟斤拷锟斤拷锟斤拷锟斤拷CAN锟斤拷锟斤拷锟叫讹拷
        LL_CAN_Receive_IT(user_can_ctrl.Instance, 
                         &user_can_ctrl.rxbuf_fmt[user_can_ctrl.rx_cnt], 
                         (uint32_t *)user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt]);
    }
}

void pfc_uart_to_llc_massage(void)
{
    int vin_over_v = (int)(pfc_received_data.vin_over_voltage_set.f * 10.0f);
    int vin_max_v = (int)(pfc_received_data.vin_max_voltage_set.f * 10.0f);
    int vin_under_v = (int)(pfc_received_data.vin_under_voltage_set.f * 10.0f);
    int vin_on_v = (int)(pfc_received_data.vin_on_voltage_set.f * 10.0f);
    int vout_over_v = (int)(pfc_received_data.vout_over_voltage_sw.f * 10.0f);
    int bus_ovp_v = (int)(pfc_received_data.bus_ovp_point_hw.f * 10.0f);
    int out_uvp_v = (int)(pfc_vout_uvp_point * 10.0f);
    int out_uvp_rec_v = (int)(pfc_vout_uvp_recovery * 10.0f);
    int ocp_soft = (int)(pfc_received_data.ipfc_ocp_current_sw.f * 10.0f);
    int ocp_dac = (int)(pfc_received_data.pfc_i_ocp_dac_point_hw.f * 10.0f);
    int vbus_target = (int)(pfc_received_data.vbus_target.f * 10.0f);
    int vbus_rel = (int)(pfc_received_data.vbus_rel.f * 10.0f);
    int vbus_ref = (int)(pfc_received_data.vbus_ref.f * 10.0f);
    int vin_rel = (int)(pfc_received_data.vin_rel.f * 10.0f);
    int iloop_rel = (int)(pfc_received_data.iloop_rel.f * 10.0f);
    int16_t ntc = pfc_received_data.r_ntc_raw;
    int duty = (int)(pfc_received_data.duty_cycle.f * 10.0f);
    uint16_t status = pfc_received_data.status_flags.all;

    split_int16(&llc.user_can.pfc.protect.in_ovp.pfc_in_ovp_vol_high_bit, &llc.user_can.pfc.protect.in_ovp.pfc_in_ovp_vol_low_bit, vin_over_v);
    split_int16(&llc.user_can.pfc.protect.in_ovp.pfc_in_ovp_rec_vol_high_bit, &llc.user_can.pfc.protect.in_ovp.pfc_in_ovp_rec_vol_low_bit, vin_max_v);
    split_int16(&llc.user_can.pfc.protect.in_uvp.pfc_in_uvp_vol_high_bit, &llc.user_can.pfc.protect.in_uvp.pfc_in_uvp_vol_low_bit, vin_under_v);
    split_int16(&llc.user_can.pfc.protect.in_uvp.pfc_in_uvp_rec_vol_high_bit, &llc.user_can.pfc.protect.in_uvp.pfc_in_uvp_rec_vol_low_bit, vin_on_v);
    split_int16(&llc.user_can.pfc.protect.out_ovp.pfc_out_ovp_vol_high_bit, &llc.user_can.pfc.protect.out_ovp.pfc_out_ovp_vol_low_bit, vout_over_v);
    split_int16(&llc.user_can.pfc.protect.out_ovp.pfc_out_ovp_rec_vol_high_bit, &llc.user_can.pfc.protect.out_ovp.pfc_out_ovp_rec_vol_low_bit, bus_ovp_v);
    split_int16(&llc.user_can.pfc.protect.out_uvp.pfc_out_uvp_vol_high_bit, &llc.user_can.pfc.protect.out_uvp.pfc_out_uvp_vol_low_bit, out_uvp_v);
    split_int16(&llc.user_can.pfc.protect.out_uvp.pfc_out_uvp_rec_vol_high_bit, &llc.user_can.pfc.protect.out_uvp.pfc_out_uvp_rec_vol_low_bit, out_uvp_rec_v);
    split_int16(&llc.user_can.pfc.protect.in_ocp.pfc_in_ocp_soft_high_bit, &llc.user_can.pfc.protect.in_ocp.pfc_in_ocp_soft_low_bit, ocp_soft);
    split_int16(&llc.user_can.pfc.protect.in_ocp.pfc_in_ocp_dac_high_bit, &llc.user_can.pfc.protect.in_ocp.pfc_in_ocp_dac_low_bit, ocp_dac);
    split_int16(&llc.user_can.pfc.data.vbus.pfc_vbus_target_high_bit, &llc.user_can.pfc.data.vbus.pfc_vbus_target_low_bit, vbus_target);
    split_int16(&llc.user_can.pfc.data.vbus.pfc_vbus_ref_high_bit, &llc.user_can.pfc.data.vbus.pfc_vbus_ref_low_bit, vbus_ref);
    split_int16(&llc.user_can.pfc.data.vbus.pfc_vbus_rel_high_bit, &llc.user_can.pfc.data.vbus.pfc_vbus_rel_low_bit, vbus_rel);
    split_int16(&llc.user_can.pfc.live1.pfc_vin_rel_high_bit, &llc.user_can.pfc.live1.pfc_vin_rel_low_bit, vin_rel);
    split_int16(&llc.user_can.pfc.live1.pfc_iloop_rel_high_bit, &llc.user_can.pfc.live1.pfc_iloop_rel_low_bit, iloop_rel);
    split_int16(&llc.user_can.pfc.live1.pfc_ntc_high_bit, &llc.user_can.pfc.live1.pfc_ntc_low_bit, ntc);

    llc.user_can.pfc.live2.pfc_state = pfc_received_data.state;
    llc.user_can.pfc.live2.pfc_freq_khz = pfc_received_data.switch_frequency;
    split_int16(&llc.user_can.pfc.live2.pfc_duty_high_bit, &llc.user_can.pfc.live2.pfc_duty_low_bit, duty);
    split_int16(&llc.user_can.pfc.live2.pfc_status_high_bit, &llc.user_can.pfc.live2.pfc_status_low_bit, status);

    /* 保留字向新上位机报告链路状态；旧上位机会忽略这两个字节。 */
    llc.user_can.pfc.protect.in_ovp.reserved0 = pfc_uart_data_valid;
    llc.user_can.pfc.protect.in_ovp.reserved1 = pfc_uart_report_sequence;
    llc.user_can.pfc.protect.in_uvp.reserved0 = pfc_uart_data_valid;
    llc.user_can.pfc.protect.in_uvp.reserved1 = pfc_uart_report_sequence;
    llc.user_can.pfc.protect.out_ovp.reserved0 = pfc_uart_data_valid;
    llc.user_can.pfc.protect.out_ovp.reserved1 = pfc_uart_report_sequence;
    llc.user_can.pfc.protect.out_uvp.reserved0 =
        (uint8_t)(pfc_uart_data_valid && pfc_uart_protocol_version >= 2U);
    llc.user_can.pfc.protect.out_uvp.reserved1 = pfc_uart_report_sequence;
    llc.user_can.pfc.protect.in_ocp.reserved0 = pfc_uart_data_valid;
    llc.user_can.pfc.protect.in_ocp.reserved1 = pfc_uart_report_sequence;
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
      llc.can_addr = 0xA0000; // 默锟斤拷值
      break;
    }
	 dev_index = llc.can_addr & 0x7;
  llc.addr_set_flag = 1;
	LL_NVIC_EnableIRQ(CAN1_IRQn);
}
