#include "main.h"
#include "can_app.h"
#include "variables_define_app.h"

DeviceInfo devices[6];  // 设备信息数组
int deviceCount = 0;    // 当前设备数量
#define USER_CAN_RX_FRM_NUMS        (1)

typedef struct __CAN_UserCtrlTypeDef
{
  CAN_TypeDef* Instance;                                   /*!< CAN Instance                       */
  volatile uint32_t tx_cnt;                                /*!< tx count                           */
  volatile uint32_t rx_cnt;                                /*!< rx count                           */
  CAN_TxBufFormatTypeDef txbuf_fmt;                        /*!< standard frame tx buffer format    */
  CAN_RxBufFormatTypeDef rxbuf_fmt[USER_CAN_RX_FRM_NUMS]; /*!< rx buffer format                   */
  uint8_t rx_buf[USER_CAN_RX_FRM_NUMS][64];               /*!< rx buffer                          */
} CAN_UserCtrlTypeDef;

CAN_UserCtrlTypeDef user_can_ctrl;

// ====================== CAN 控制延迟（保留原固定配置） ======================
static inline void can_ctrl_delay(void)
{
  CAN1->CTRL |= 1<<4;
  __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
  __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
  __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
  __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
  CAN1->CTRL &= ~(1<<4);
}

// ====================== 16位数据处理函数 ======================
static inline uint16_t extract_16bit_data(uint8_t start_index)
{
  uint8_t idx = user_can_ctrl.rx_cnt;
  return ((uint16_t)user_can_ctrl.rx_buf[idx][start_index + 1] << 8) |
          user_can_ctrl.rx_buf[idx][start_index];
}

RAMCODE bool process_16bit_simple(uint8_t cmd, uint16_t* result_value)
{
  uint8_t idx = user_can_ctrl.rx_cnt;
  if (user_can_ctrl.rxbuf_fmt[idx].data_len_code < 4) return false;
  if (user_can_ctrl.rx_buf[idx][1] != cmd) return false;

  *result_value = extract_16bit_data(2);
  return true;
}

RAMCODE bool process_16bit_with_crc(uint8_t cmd, uint16_t* result_value)
{
  uint8_t idx = user_can_ctrl.rx_cnt;
  if (user_can_ctrl.rxbuf_fmt[idx].data_len_code < 5) return false;
  if (user_can_ctrl.rx_buf[idx][1] != cmd) return false;

  uint8_t low          = user_can_ctrl.rx_buf[idx][2];
  uint8_t high         = user_can_ctrl.rx_buf[idx][3];
  uint8_t received_crc = user_can_ctrl.rx_buf[idx][4];

  uint8_t crc_data[3] = {cmd, low, high};
  if (crc8(crc_data, 3) != received_crc) return false;

  *result_value = ((uint16_t)high << 8) | low;
  return true;
}

// ====================== 32位数据处理函数 ======================
static inline uint32_t extract_32bit_data(uint8_t start_index)
{
  uint8_t idx = user_can_ctrl.rx_cnt;
  return ((uint32_t)user_can_ctrl.rx_buf[idx][start_index + 3] << 24) |
         ((uint32_t)user_can_ctrl.rx_buf[idx][start_index + 2] << 16) |
         ((uint32_t)user_can_ctrl.rx_buf[idx][start_index + 1] << 8)  |
          user_can_ctrl.rx_buf[idx][start_index];
}

RAMCODE bool process_32bit_with_crc(uint8_t cmd, float* result_value, int scale_factor)
{
  uint8_t idx = user_can_ctrl.rx_cnt;
  if (user_can_ctrl.rxbuf_fmt[idx].data_len_code < 7) return false;
  if (user_can_ctrl.rx_buf[idx][1] != cmd) return false;

  uint8_t d[4] = {user_can_ctrl.rx_buf[idx][2], user_can_ctrl.rx_buf[idx][3],
                  user_can_ctrl.rx_buf[idx][4], user_can_ctrl.rx_buf[idx][5]};
  uint8_t received_crc = user_can_ctrl.rx_buf[idx][6];

  uint8_t crc_data[5] = {cmd, d[0], d[1], d[2], d[3]};
  if (crc8(crc_data, 5) != received_crc) return false;

  uint32_t raw = ((uint32_t)d[3] << 24) | ((uint32_t)d[2] << 16) |
                 ((uint32_t)d[1] << 8)  | d[0];
  *result_value = (float)raw / (float)scale_factor;
  return true;
}

RAMCODE bool process_32bit_with_crc_raw(uint8_t cmd, uint32_t* result_value)
{
  uint8_t idx = user_can_ctrl.rx_cnt;
  if (user_can_ctrl.rxbuf_fmt[idx].data_len_code < 7) return false;
  if (user_can_ctrl.rx_buf[idx][1] != cmd) return false;

  uint8_t d[4] = {user_can_ctrl.rx_buf[idx][2], user_can_ctrl.rx_buf[idx][3],
                  user_can_ctrl.rx_buf[idx][4], user_can_ctrl.rx_buf[idx][5]};
  uint8_t received_crc = user_can_ctrl.rx_buf[idx][6];

  uint8_t crc_data[5] = {cmd, d[0], d[1], d[2], d[3]};
  if (crc8(crc_data, 5) != received_crc) return false;

  *result_value = ((uint32_t)d[3] << 24) | ((uint32_t)d[2] << 16) |
                  ((uint32_t)d[1] << 8)  | d[0];
  return true;
}

// ====================== 发送函数 ======================
RAMCODE void can_send_data(void* data_buf, size_t data_size)
{
  CAN_TxBufFormatTypeDef tx_buf_fmt;

  tx_buf_fmt.id_extension = 1;
  tx_buf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(llc.can_addr);
  tx_buf_fmt.data_len_code = data_size;
  tx_buf_fmt.remote_tx_req = 0;

  LL_StatusETypeDef status = LL_CAN_TransmitPTB_CPU(CAN1, &tx_buf_fmt, (uint32_t*)data_buf);
  if (status != LL_OK)
  {
    // Error handling
  }
}

RAMCODE void can_send_data_with_crc(void* data_buf, size_t data_size)
{
  CAN_TxBufFormatTypeDef tx_buf_fmt;

  uint8_t data_with_crc[8];
  if(data_buf == NULL || data_size == 0 || data_size > 7)return;
  memcpy(data_with_crc, data_buf, data_size);

  uint8_t crc = crc8((uint8_t*)data_buf, data_size);
  data_with_crc[data_size] = crc;

  tx_buf_fmt.id_extension = 1;
  tx_buf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(llc.can_addr);
  tx_buf_fmt.data_len_code = data_size + 1;
  tx_buf_fmt.remote_tx_req = 0;

  LL_StatusETypeDef status = LL_CAN_TransmitPTB_CPU(CAN1, &tx_buf_fmt, (uint32_t*)data_with_crc);
  if (status != LL_OK)
  {
    printf("Error sending data with CRC\n");
  }
}

void can_send_float_data(uint32_t cmd, float data, int scale_factor)
{
  CAN_TxBufFormatTypeDef tx_buf_fmt;
  int scaled_value = (int)(data * scale_factor);

  uint8_t high_bit = (scaled_value >> 8) & 0xFF;
  uint8_t low_bit  = scaled_value & 0xFF;
  uint8_t data_bytes[4] = {0, cmd, low_bit, high_bit};

  tx_buf_fmt.id_extension = 1;
  tx_buf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(llc.can_addr);
  tx_buf_fmt.data_len_code = sizeof(data_bytes);
  tx_buf_fmt.remote_tx_req = 0;

  LL_StatusETypeDef status = LL_CAN_TransmitPTB_CPU(CAN1, &tx_buf_fmt, (uint32_t*)data_bytes);
  if (status != LL_OK)
  {
    printf("Error sending data\n");
  }
}

RAMCODE void can_send_float_data_crc(float data, int scale_factor)
{
  CAN_TxBufFormatTypeDef tx_buf_fmt;
  int scaled_value = (int)(data * scale_factor);

  uint8_t high_bit = (scaled_value >> 8) & 0xFF;
  uint8_t low_bit  = scaled_value & 0xFF;
  uint8_t data_bytes[2] = {low_bit, high_bit};

  uint8_t crc = crc8(data_bytes, sizeof(data_bytes));
  uint8_t data_with_crc[3] = {data_bytes[0], data_bytes[1], crc};

  tx_buf_fmt.id_extension = 1;
  tx_buf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(llc.can_addr);
  tx_buf_fmt.data_len_code = sizeof(data_with_crc);
  tx_buf_fmt.remote_tx_req = 0;

  LL_StatusETypeDef status = LL_CAN_TransmitPTB_CPU(CAN1, &tx_buf_fmt, (uint32_t*)data_with_crc);
  if (status != LL_OK)
  {
    printf("Error sending data\n");
  }
}

// ====================== 公共命令处理 ======================
RAMCODE void process_common_commands(uint8_t can_cmd, uint32_t received_id)
{
  switch(can_cmd)
  {
    case CMD_QUERY:
    {
			can_data.power_para.frame_count = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0];
      can_ctrl_delay();
      can_send_data((void*)&can_data.power_para, sizeof(can_data.power_para));
      break;
    }

    case CMD_VERSION:
    {
			can_data.version_info.frame_count = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0];
			hld_can_data.version_info.frame_count = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0];
      can_ctrl_delay();
      if(received_id == 0xB0000)
        can_send_data((void*)&hld_can_data.version_info, sizeof(hld_can_data.version_info));
      else
        can_send_data((void*)&can_data.version_info, sizeof(can_data.version_info));
      break;
    }

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

    case CMD_OVERTEMP_POINT:
    {
      uint16_t val;
      if(process_16bit_simple(CMD_OVERTEMP_POINT, &val))
      {
        user_data.over_temp_point = (float)val;
        llc.protection_point.over_temp_point = user_data.over_temp_point;
      }
      break;
    }

    case CMD_OVERTEMP_REC_POINT:
    {
      uint16_t val;
      if(process_16bit_with_crc(CMD_OVERTEMP_REC_POINT, &val))
      {
        user_data.over_temp_rec_point = (float)val;
        llc.protection_point.over_temp_rec_point = user_data.over_temp_rec_point;
      }
      break;
    }

    case CMD_FACTOR_VOLTAGE:
    {
      uint16_t val;
      if(process_16bit_simple(CMD_FACTOR_VOLTAGE, &val))
        llc.factor_voltage = val;
      break;
    }

    case CMD_THEOR_VOLTAGE:
    {
      uint16_t val;
      if(process_16bit_simple(CMD_THEOR_VOLTAGE, &val))
        llc.theor_voltage = val;
      break;
    }

		case CMD_VOLTAGE_CRC:
		{
			llc.voltage_crc = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][2];

			uint16_t factor_voltage_value     = (uint16_t)llc.factor_voltage;
			uint8_t  factor_voltage_low_byte  = (uint8_t)(factor_voltage_value & 0xFF);
			uint8_t  factor_voltage_high_byte = (uint8_t)((factor_voltage_value >> 8) & 0xFF);

			uint16_t theor_voltage_value      = (uint16_t)llc.theor_voltage;
			uint8_t  theor_voltage_low_byte   = (uint8_t)(theor_voltage_value & 0xFF);
			uint8_t  theor_voltage_high_byte  = (uint8_t)((theor_voltage_value >> 8) & 0xFF);

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
				float factor_voltage_float = llc.factor_voltage / 1000.0f;
				float theor_voltage_float  = llc.theor_voltage  / 1000.0f;

				if(factor_voltage_float > 0.1f)
				{
					user_data.coef_target =
							(llc.vbus_target / factor_voltage_float) * theor_voltage_float;

					user_data.coef_target =
							limit_float(user_data.coef_target, 22.0f, 30.0f);

					user_data.vout_trim_delta =
							user_data.coef_target - VOUT_VOLTAGE;

					user_data.vout_can_delta =
							llc.vbus_target - factor_voltage_float;

					user_data_vout_protect_update();

					llc.coef_target =
							user_data.coef_target;

					llc.vbus_target =
							user_data.coef_target;

					llc.vout_trim_delta =
							user_data.vout_trim_delta;

					llc.can_com_voltag_delta =
							user_data.vout_can_delta;

					llc.ibus_target =
							user_data.ibus_target;

					llc.ibus_rec_target =
							user_data.ibus_rec_target;

					llc.ibus_ref =
							user_data.ibus_ref;

					llc.vout_is_change = 1;
				}
				else
				{
					llc.factor_voltage = 0;
					llc.theor_voltage  = 0;
					llc.vout_is_change = 0;
				}
			}
			else
			{
				llc.factor_voltage = 0;
				llc.theor_voltage  = 0;
				llc.vout_is_change = 0;
			}

			break;
		}
				case CMD_PARA_SCALE:
				{
						uint32_t scale_value;
						// 应该使用 process_32bit_with_crc_raw 获取原始uint32值
						if(process_32bit_with_crc_raw(CMD_PARA_SCALE, &scale_value)) 
						{
								// 验证缩放因子范围（1-10000是合理范围）
								if(scale_value > 0 && scale_value <= 10000)
								{
										hld_can_data.para_scale_factor = scale_value;  // 直接存储uint32
										hld_can_data.para_scale_factor_is_ok = 1;
								}
								else
								{
										// 超出范围，使用默认值
										hld_can_data.para_scale_factor = KP_KI_SCALE_FACTOR;
										hld_can_data.para_scale_factor_is_ok = 0;
								}
						}
				}
				break;
				
    case CMD_KP:
    {
            float float_value;
						uint32_t current_scale = hld_can_data.para_scale_factor_is_ok ? 
																		 hld_can_data.para_scale_factor : 
																		 KP_KI_SCALE_FACTOR;
						if(process_32bit_with_crc(CMD_KP, &float_value, current_scale))
							{
								llc.shareloop_kp_init  = float_value;
							}
      break;
    }

    case CMD_KI:
    {
            float float_value;
						uint32_t current_scale = hld_can_data.para_scale_factor_is_ok ? 
																		 hld_can_data.para_scale_factor : 
																		 KP_KI_SCALE_FACTOR;
						if(process_32bit_with_crc(CMD_KI, &float_value, current_scale))
							{
								llc.shareloop_ki_init  = float_value;
							}
      break;
    }

    case CMD_TEST:
    {
            float float_value;
						uint32_t current_scale = hld_can_data.para_scale_factor_is_ok ? 
																		 hld_can_data.para_scale_factor : 
																		 KP_KI_SCALE_FACTOR;
						if(process_32bit_with_crc(CMD_TEST, &float_value, current_scale))
							{
								llc.shareloop_out_max_init  = float_value;
//								llc.shareloop.out_max		= llc.shareloop_out_max_init;
//								llc.shareloop.out_min 	= -llc.shareloop.out_max;
							}
      break;
    }

    case CMD_TEST2:
    {
            float float_value;
						uint32_t current_scale = hld_can_data.para_scale_factor_is_ok ? 
																		 hld_can_data.para_scale_factor : 
																		 KP_KI_SCALE_FACTOR;
						if(process_32bit_with_crc(CMD_TEST2, &float_value, current_scale))
							{
								llc.shareloop_ki_init  = float_value;
							}
      break; 
		}

    default:
      break;
  }
}

// ====================== 查找设备 ======================
RAMCODE int findDevice(uint32_t id)
{
  for(int i = 0; i < deviceCount; i++)
  {
    if(devices[i].id == id)
      return i;
  }
  return -1;
}

// ====================== 初始化（保留原有配置） ======================
void can_init_app(void)
{
  CAN_UserCfgTypeDef can_user_cfg;
  CAN_AcceptFilCfgTypeDef can_acpt_fil_cfg[4];

  memset((void*)&can_user_cfg,     0x00, sizeof(can_user_cfg));
  memset((void*)&can_acpt_fil_cfg, 0x00, sizeof(can_acpt_fil_cfg));

  // CAN acceptance filter config（保留原配置）
  can_acpt_fil_cfg[0].slot      = CAN_ACCEPT_FILT_SLOT_0;
  can_acpt_fil_cfg[0].code_val  = USER_CAN_STD_FRM_ID;
  can_acpt_fil_cfg[0].mask_val  = 0;
  can_acpt_fil_cfg[0].rx_frm    = CAN_ACCEPT_FILT_FRM_STD_EXT;

  can_acpt_fil_cfg[1].slot      = CAN_ACCEPT_FILT_SLOT_1;
  can_acpt_fil_cfg[1].code_val  = 0xA0000;
  can_acpt_fil_cfg[1].mask_val  = 0xFF;
  can_acpt_fil_cfg[1].rx_frm    = CAN_ACCEPT_FILT_FRM_STD_EXT;

  can_acpt_fil_cfg[2].slot      = CAN_ACCEPT_FILT_SLOT_2;
  can_acpt_fil_cfg[2].code_val  = 0xB0000;
  can_acpt_fil_cfg[2].mask_val  = 0xFF;
  can_acpt_fil_cfg[2].rx_frm    = CAN_ACCEPT_FILT_FRM_STD_EXT;

  can_acpt_fil_cfg[3].slot      = CAN_ACCEPT_FILT_SLOT_3;
  can_acpt_fil_cfg[3].code_val  = IAP_CAN_ID;
  can_acpt_fil_cfg[3].mask_val  = 0;
  can_acpt_fil_cfg[3].rx_frm    = CAN_ACCEPT_FILT_FRM_STD_EXT;

  // CAN LL Init（保留原配置）
  can_user_cfg.fd_en                = false;
  can_user_cfg.fd_iso_en            = false;
  can_user_cfg.func_clk_freq        = 120000000UL;
  can_user_cfg.baudrate_ss          = USER_CAN_BAUDRATE;
  can_user_cfg.bit_timing_seg1_ss   = 6;
  can_user_cfg.bit_timing_seg2_ss   = 1;
  can_user_cfg.bit_timing_sjw_ss    = 1;
  can_user_cfg.rx_almost_full_limit = CAN_RX_ALMOST_FULL_LIMIT_1;
  can_user_cfg.err_limit            = CAN_ERR_WARN_LIMIT_104;
  can_user_cfg.accept_fil_cfg_ptr   = can_acpt_fil_cfg;
  can_user_cfg.accept_fil_cfg_num   = ARRAY_SIZE(can_acpt_fil_cfg);

  LL_CAN_Init(CAN1, &can_user_cfg);

  // User CAN control init
  memset((void*)&user_can_ctrl, 0x00, sizeof(user_can_ctrl));
  user_can_ctrl.Instance            = CAN1;
  user_can_ctrl.txbuf_fmt.id_extension = 1;
  user_can_ctrl.txbuf_fmt.id        = __LL_CAN_FrameIDFormat_29Bits(USER_CAN_STD_FRM_ID);

  for(int i = 0; i < USER_CAN_RX_FRM_NUMS; i++)
  {
    user_can_ctrl.rxbuf_fmt[i].id_extension  = 1;
    user_can_ctrl.rxbuf_fmt[i].data_len_code = 8;
  }

  LL_CAN_Receive_IT(user_can_ctrl.Instance, &user_can_ctrl.rxbuf_fmt[0], user_can_ctrl.rx_buf[0]);
  for(int i = 0; i < 8; i++)
  {
    llc.can_buf[i] = i;
  }
  __LL_CAN_Rx_INT_En(CAN1);
}

// ====================== 接收回调 ======================
RAMCODE void LL_CAN_RxCallback(CAN_TypeDef* Instance)
{
  if(user_can_ctrl.rx_cnt >= USER_CAN_RX_FRM_NUMS) return;

  uint32_t received_id = user_can_ctrl.rxbuf_fmt[user_can_ctrl.rx_cnt].id;
  uint8_t  can_cmd     = user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][1];

  switch(received_id)
  {
    case USER_CAN_STD_FRM_ID:
      process_common_commands(can_cmd, received_id);
      break;

    case 0xB0000:
      process_common_commands(can_cmd, received_id);
      break;

    case IAP_CAN_ID:
      if((user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0] == 2 ||  // LLC: 2, PFC: 1
          user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][0] == 1) &&
         user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt][1] == 0x41)
      {
        NVIC_SystemReset();
      }
      break;

    default:
      if(received_id == llc.can_addr)
        process_common_commands(can_cmd, received_id);
      break;
  }

  user_can_ctrl.rx_cnt++;
  if(user_can_ctrl.rx_cnt >= USER_CAN_RX_FRM_NUMS)
    user_can_ctrl.rx_cnt = 0;

  LL_CAN_Receive_IT(user_can_ctrl.Instance,
                    &user_can_ctrl.rxbuf_fmt[user_can_ctrl.rx_cnt],
                    user_can_ctrl.rx_buf[user_can_ctrl.rx_cnt]);
}

// ====================== 地址设置（保留原有） ======================
void can_addr_set(void)
{
  llc.can_addr_check.can1_level = (LL_GPIO_ReadPin(ADDR1_PIN_PORT, ADDR1_PIN)) ? 1 : 0;
  llc.can_addr_check.can2_level = (LL_GPIO_ReadPin(ADDR2_PIN_PORT, ADDR2_PIN)) ? 1 : 0;
  llc.can_addr_check.can3_level = (LL_GPIO_ReadPin(ADDR3_PIN_PORT, ADDR3_PIN)) ? 1 : 0;

  switch (llc.can_addr_check.can_addr_level_bits)
  {
    case 0:   llc.can_addr = 0xA0000; break;
    case 1:   llc.can_addr = 0xA0001; break;
    case 2:   llc.can_addr = 0xA0002; break;
    case 3:   llc.can_addr = 0xA0003; break;
    case 4:   llc.can_addr = 0xA0004; break;
    case 5:   llc.can_addr = 0xA0005; break;
    case 6:   llc.can_addr = 0xA0006; break;
    case 7:   llc.can_addr = 0xA0007; break;
    default:  llc.can_addr = 0xA0000; break;
  }

  llc.addr_set_flag = 1;
		//NVIC UART0 Interrupt Enable
		LL_NVIC_EnableIRQ(CAN1_IRQn);
		LL_NVIC_SetPriority(CAN1_IRQn, 4, 0);
}

// ====================== 数据初始化（保留原有变量名） ======================
void can_send_data_init(void)
{
  can_data.power_para.return_bit = RETURN_BIT_POWER;

  can_data.version_info.version_cmd      = VERSION_CMD;
  can_data.version_info.compnet_type     = COMPNET_TYPE;
  can_data.version_info.year             = YEAR_NUM;
  can_data.version_info.month            = MONTH_NUM;
  can_data.version_info.day              = DAY_NUM;
  can_data.version_info.version_low_bit  = VERSION_CODE & 0xFF;
  can_data.version_info.version_high_bit = (VERSION_CODE >> 8) & 0xFF;

  hld_can_data.power_para.return_bit = RETURN_BIT_POWER;

  hld_can_data.version_info.version_cmd      = VERSION_CMD;
  hld_can_data.version_info.compnet_type     = COMPNET_TYPE;
  hld_can_data.version_info.year             = HLD_YEAR_NUM;
  hld_can_data.version_info.month            = HLD_MONTH_NUM;
  hld_can_data.version_info.day             = HLD_DAY_NUM;
  hld_can_data.version_info.version_low_bit  = HLD_VERSION_CODE & 0xFF;
  hld_can_data.version_info.version_high_bit = (HLD_VERSION_CODE >> 8) & 0xFF;
}