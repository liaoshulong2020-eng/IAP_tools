// ============================================================================
// MCU 端 CAN 通讯优化补丁
// 用于修复与上位机的通讯问题
// ============================================================================

#include "main.h"
#include "can_app.h"
#include "variables_define_app.h"

// ============================================================================
// 优化1：增加接收缓冲区数量
// ============================================================================
// 原代码：#define USER_CAN_RX_FRM_NUMS (1)
// 优化后：#define USER_CAN_RX_FRM_NUMS (4)
//
// 原因：只有1个接收缓冲区，高频率命令容易丢失
// 效果：支持同时接收4个CAN帧，减少丢包率

// ============================================================================
// 优化2：完善数据长度检查
// ============================================================================
// 在 process_16bit_simple() 和 process_16bit_with_crc() 中添加最大长度检查

RAMCODE
bool process_16bit_simple_optimized(uint8_t cmd, uint16_t* result_value)
{
  uint8_t current_rx_index = user_can_ctrl.rx_cnt;

  // 【优化】添加最大长度检查，防止缓冲区溢出
  if (user_can_ctrl.rxbuf_fmt[current_rx_index].data_len_code < 4 ||
      user_can_ctrl.rxbuf_fmt[current_rx_index].data_len_code > 8)
    {
      return false;
    }

  // 验证命令
  uint8_t received_cmd = user_can_ctrl.rx_buf[current_rx_index][1];
  if(received_cmd != cmd)
    {
      return false;
    }

  // 提取16位数据
  *result_value = extract_16bit_data(2);
  return true;
}

RAMCODE
bool process_16bit_with_crc_optimized(uint8_t cmd, uint16_t* result_value)
{
  uint8_t current_rx_index = user_can_ctrl.rx_cnt;

  // 【优化】添加最大长度检查
  if (user_can_ctrl.rxbuf_fmt[current_rx_index].data_len_code < 5 ||
      user_can_ctrl.rxbuf_fmt[current_rx_index].data_len_code > 8)
    {
      return false;
    }

  // 验证命令
  uint8_t received_cmd = user_can_ctrl.rx_buf[current_rx_index][1];
  if(received_cmd != cmd)
    {
      return false;
    }

  // 提取16位数据
  *result_value = extract_16bit_data(2);

  // 【优化】验证CRC
  uint8_t received_crc = user_can_ctrl.rx_buf[current_rx_index][4];
  uint8_t calculated_crc = crc8_calc(&user_can_ctrl.rx_buf[current_rx_index][0], 4);

  if(received_crc != calculated_crc)
    {
      return false;
    }

  return true;
}

// ============================================================================
// 优化3：改进32位数据处理
// ============================================================================

RAMCODE
bool process_32bit_simple_optimized(uint8_t cmd, uint32_t* result_value)
{
  uint8_t current_rx_index = user_can_ctrl.rx_cnt;

  // 【优化】添加长度检查
  if (user_can_ctrl.rxbuf_fmt[current_rx_index].data_len_code < 6 ||
      user_can_ctrl.rxbuf_fmt[current_rx_index].data_len_code > 8)
    {
      return false;
    }

  // 验证命令
  uint8_t received_cmd = user_can_ctrl.rx_buf[current_rx_index][1];
  if(received_cmd != cmd)
    {
      return false;
    }

  // 提取32位数据（小端格式）
  uint8_t byte0 = user_can_ctrl.rx_buf[current_rx_index][2];
  uint8_t byte1 = user_can_ctrl.rx_buf[current_rx_index][3];
  uint8_t byte2 = user_can_ctrl.rx_buf[current_rx_index][4];
  uint8_t byte3 = user_can_ctrl.rx_buf[current_rx_index][5];

  *result_value = (byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0;
  return true;
}

RAMCODE
bool process_32bit_with_crc_optimized(uint8_t cmd, uint32_t* result_value)
{
  uint8_t current_rx_index = user_can_ctrl.rx_cnt;

  // 【优化】添加长度检查
  if (user_can_ctrl.rxbuf_fmt[current_rx_index].data_len_code < 7 ||
      user_can_ctrl.rxbuf_fmt[current_rx_index].data_len_code > 8)
    {
      return false;
    }

  // 验证命令
  uint8_t received_cmd = user_can_ctrl.rx_buf[current_rx_index][1];
  if(received_cmd != cmd)
    {
      return false;
    }

  // 提取32位数据
  uint8_t byte0 = user_can_ctrl.rx_buf[current_rx_index][2];
  uint8_t byte1 = user_can_ctrl.rx_buf[current_rx_index][3];
  uint8_t byte2 = user_can_ctrl.rx_buf[current_rx_index][4];
  uint8_t byte3 = user_can_ctrl.rx_buf[current_rx_index][5];

  *result_value = (byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0;

  // 【优化】验证CRC
  uint8_t received_crc = user_can_ctrl.rx_buf[current_rx_index][6];
  uint8_t calculated_crc = crc8_calc(&user_can_ctrl.rx_buf[current_rx_index][0], 6);

  if(received_crc != calculated_crc)
    {
      return false;
    }

  return true;
}

// ============================================================================
// 优化4：改进电压计算精度
// ============================================================================
// 原问题：电压计算涉及多个delta值，可能导致精度丢失
// 优化方案：使用更精确的计算方式

uint16_t calculate_output_voltage_optimized(void)
{
  // 【优化】使用更精确的计算方式
  // 原方式：voltage = (adc_value - delta1) * delta2 / delta3
  // 新方式：先进行整数运算，最后再除以缩放因子

  uint32_t adc_raw = llc.adc_data.vout_adc;
  uint32_t voltage_mv;

  // 使用64位中间变量避免溢出
  uint64_t temp = (uint64_t)adc_raw * llc.hld_can_data.voltage_scale_factor;
  voltage_mv = (uint16_t)(temp / 1000);

  return voltage_mv;
}

uint16_t calculate_output_current_optimized(void)
{
  // 【优化】使用更精确的计算方式
  uint32_t adc_raw = llc.adc_data.iout_adc;
  uint32_t current_01a;

  // 使用64位中间变量避免溢出
  uint64_t temp = (uint64_t)adc_raw * llc.hld_can_data.current_scale_factor;
  current_01a = (uint16_t)(temp / 1000);

  return current_01a;
}

// ============================================================================
// 优化5：改进参数缩放因子初始化
// ============================================================================
// 原问题：参数缩放因子初始值可能未正确设置
// 优化方案：在初始化时明确设置缩放因子

void can_send_data_init_optimized(void)
{
  // 【优化】明确初始化参数缩放因子
  llc.hld_can_data.para_scale_factor = KP_KI_SCALE_FACTOR;
  llc.hld_can_data.para_scale_factor_is_ok = 1;

  // 【优化】初始化电压和电流缩放因子
  llc.hld_can_data.voltage_scale_factor = VOLTAGE_SCALE_FACTOR;
  llc.hld_can_data.current_scale_factor = CURRENT_SCALE_FACTOR;

  // 【优化】初始化通讯诊断计数器
  llc.hld_can_data.cmd_rx_count = 0;
  llc.hld_can_data.crc_error_count = 0;
  llc.hld_can_data.data_len_error_count = 0;
}

// ============================================================================
// 优化6：增加通讯诊断功能
// ============================================================================

typedef struct
{
  uint32_t cmd_rx_count;           // 命令接收计数
  uint32_t crc_error_count;        // CRC错误计数
  uint32_t data_len_error_count;   // 数据长度错误计数
  uint32_t unknown_cmd_count;      // 未知命令计数
  uint32_t last_cmd_time;          // 最后一条命令接收时间
} CAN_DiagnosticTypeDef;

CAN_DiagnosticTypeDef can_diagnostic = {0};

// 记录命令接收
void can_diagnostic_record_cmd(uint8_t cmd)
{
  can_diagnostic.cmd_rx_count++;
  can_diagnostic.last_cmd_time = HAL_GetTick();
}

// 记录CRC错误
void can_diagnostic_record_crc_error(void)
{
  can_diagnostic.crc_error_count++;
}

// 记录数据长度错误
void can_diagnostic_record_len_error(void)
{
  can_diagnostic.data_len_error_count++;
}

// 记录未知命令
void can_diagnostic_record_unknown_cmd(void)
{
  can_diagnostic.unknown_cmd_count++;
}

// 获取诊断信息
void can_diagnostic_get_info(CAN_DiagnosticTypeDef* diag)
{
  if(diag != NULL)
    {
      *diag = can_diagnostic;
    }
}

// ============================================================================
// 优化7：改进CAN帧发送
// ============================================================================
// 原问题：发送函数没有返回值，无法判断发送是否成功
// 优化方案：添加发送状态检查

bool can_send_data_optimized(void* data_buf, size_t data_size)
{
  if(data_buf == NULL || data_size == 0 || data_size > 8)
    {
      return false;
    }

  // 【优化】检查发送缓冲区是否可用
  if(user_can_ctrl.tx_cnt >= 10)  // 防止发送队列过长
    {
      return false;
    }

  // 构建CAN帧
  CAN_TxBufFormatTypeDef tx_frame;
  tx_frame.can_id = llc.hld_can_data.device_id;
  tx_frame.can_dlc = data_size;

  memcpy(tx_frame.data, data_buf, data_size);

  // 发送CAN帧
  LL_CAN_Transmit(CAN1, &tx_frame);

  user_can_ctrl.tx_cnt++;
  return true;
}

bool can_send_data_with_crc_optimized(void* data_buf, size_t data_size)
{
  if(data_buf == NULL || data_size == 0 || data_size > 7)
    {
      return false;
    }

  // 【优化】检查发送缓冲区是否可用
  if(user_can_ctrl.tx_cnt >= 10)
    {
      return false;
    }

  // 计算CRC
  uint8_t crc = crc8_calc((uint8_t*)data_buf, data_size);

  // 构建CAN帧
  CAN_TxBufFormatTypeDef tx_frame;
  tx_frame.can_id = llc.hld_can_data.device_id;
  tx_frame.can_dlc = data_size + 1;

  memcpy(tx_frame.data, data_buf, data_size);
  tx_frame.data[data_size] = crc;

  // 发送CAN帧
  LL_CAN_Transmit(CAN1, &tx_frame);

  user_can_ctrl.tx_cnt++;
  return true;
}

// ============================================================================
// 优化8：改进命令处理流程
// ============================================================================
// 原问题：命令处理中没有统一的错误处理机制
// 优化方案：添加统一的命令处理框架

typedef bool (*cmd_handler_t)(void);

typedef struct
{
  uint8_t cmd;
  cmd_handler_t handler;
} cmd_handler_entry_t;

// 命令处理表
static const cmd_handler_entry_t cmd_handlers[] = {
  {0x01, handle_cmd_query},
  {0x02, handle_cmd_version},
  {0x04, handle_cmd_stop},
  {0x05, handle_cmd_start},
  {0x0F, handle_cmd_overtemp_point},
  {0x10, handle_cmd_overtemp_rec_point},
  {0x22, handle_cmd_factor_voltage},
  {0x23, handle_cmd_theor_voltage},
  {0x24, handle_cmd_voltage_crc},
  {0x27, handle_cmd_para_scale},
  {0x28, handle_cmd_kp},
  {0x29, handle_cmd_ki},
  {0x3F, handle_cmd_llc_voltage_protect},
  {0x40, handle_cmd_llc_ocp_protect},
  {0x41, handle_cmd_llc_osp_protect},
  {0x44, handle_cmd_llc_out_para},
};

#define CMD_HANDLER_COUNT (sizeof(cmd_handlers) / sizeof(cmd_handlers[0]))

// 统一的命令处理函数
bool process_command_optimized(uint8_t cmd)
{
  // 【优化】使用命令处理表，提高代码可维护性
  for(int i = 0; i < CMD_HANDLER_COUNT; i++)
    {
      if(cmd_handlers[i].cmd == cmd)
        {
          can_diagnostic_record_cmd(cmd);
          return cmd_handlers[i].handler();
        }
    }

  // 未知命令
  can_diagnostic_record_unknown_cmd();
  return false;
}

// ============================================================================
// 优化9：改进接收中断处理
// ============================================================================
// 原问题：接收中断处理中没有错误检查
// 优化方案：添加完整的错误检查和诊断

void User_CAN_RxCpltCallback_Optimized(void)
{
  uint8_t current_rx_index = user_can_ctrl.rx_cnt;

  // 【优化】检查接收缓冲区是否有效
  if(current_rx_index >= USER_CAN_RX_FRM_NUMS)
    {
      return;
    }

  // 【优化】检查数据长度
  uint8_t data_len = user_can_ctrl.rxbuf_fmt[current_rx_index].data_len_code;
  if(data_len < 2 || data_len > 8)
    {
      can_diagnostic_record_len_error();
      return;
    }

  // 【优化】获取命令码
  uint8_t cmd = user_can_ctrl.rx_buf[current_rx_index][1];

  // 【优化】处理命令
  if(!process_command_optimized(cmd))
    {
      // 命令处理失败，可能是CRC错误或未知命令
      if(cmd >= 0x0F && cmd <= 0x29)
        {
          can_diagnostic_record_crc_error();
        }
    }

  // 【优化】更新接收缓冲区索引
  user_can_ctrl.rx_cnt = (current_rx_index + 1) % USER_CAN_RX_FRM_NUMS;
}

// ============================================================================
// 优化10：改进电源状态上报
// ============================================================================
// 原问题：电源状态上报中的电压和电流计算可能不准确
// 优化方案：使用优化后的计算函数

void send_power_status_optimized(void)
{
  power_upper_TypeDef power_status;

  power_status.frame_count = llc.hld_can_data.frame_count++;
  power_status.return_bit = RETURN_BIT_POWER;

  // 【优化】使用优化后的状态字节构建
  power_status.power_status_byte = 0;
  if(llc.power_ctrl.on_off_ok_flag) power_status.power_status_byte |= 0x01;
  if(llc.protection.over_voltage_flag) power_status.power_status_byte |= 0x02;
  if(llc.protection.under_voltage_flag) power_status.power_status_byte |= 0x04;
  if(llc.protection.over_current_flag) power_status.power_status_byte |= 0x08;
  if(llc.protection.over_temp_flag) power_status.power_status_byte |= 0x10;
  power_status.power_status_byte |= 0x20;  // 通讯状态正常

  // 【优化】使用优化后的电压和电流计算
  uint16_t voltage_mv = calculate_output_voltage_optimized();
  uint16_t current_01a = calculate_output_current_optimized();

  power_status.voltage_low = voltage_mv & 0xFF;
  power_status.voltage_high = (voltage_mv >> 8) & 0xFF;
  power_status.current_low = current_01a & 0xFF;
  power_status.current_high = (current_01a >> 8) & 0xFF;
  power_status.temperature = llc.adc_data.temperature;

  // 发送
  can_send_data_optimized(&power_status, sizeof(power_status));
}

// ============================================================================
// 使用说明
// ============================================================================
/*
 * 1. 将 USER_CAN_RX_FRM_NUMS 从 1 改为 4
 *    位置：can_app.c 第10行
 *
 * 2. 替换以下函数为优化版本：
 *    - process_16bit_simple() -> process_16bit_simple_optimized()
 *    - process_16bit_with_crc() -> process_16bit_with_crc_optimized()
 *    - process_32bit_simple() -> process_32bit_simple_optimized()
 *    - process_32bit_with_crc() -> process_32bit_with_crc_optimized()
 *    - can_send_data() -> can_send_data_optimized()
 *    - can_send_data_with_crc() -> can_send_data_with_crc_optimized()
 *    - User_CAN_RxCpltCallback() -> User_CAN_RxCpltCallback_Optimized()
 *    - send_power_status() -> send_power_status_optimized()
 *
 * 3. 在初始化函数中调用：
 *    can_send_data_init_optimized();
 *
 * 4. 可选：添加诊断功能
 *    - 定期调用 can_diagnostic_get_info() 获取诊断信息
 *    - 通过上位机查询诊断数据
 *
 * 5. 编译和测试
 *    - 重新编译MCU代码
 *    - 测试与上位机的通讯
 *    - 验证数据准确性
 */

// ============================================================================
// 新增：0x3E 命令 — 查询温度保护点
// ============================================================================
// 上位机发送：[cnt, 0x3E, 0, 0, 0, 0, 0, 0]
// MCU 响应：  [cnt, 0xBE, ot_low, ot_high, rec_low, rec_high, 0, 0]
//   ot  = (uint16_t)(llc.protection_point.over_temp_point)     单位：°C（整数）
//   rec = (uint16_t)(llc.protection_point.over_temp_rec_point) 单位：°C（整数）
// ============================================================================

RAMCODE
bool handle_cmd_llc_temp_protect(void)
{
    uint8_t current_rx_index = user_can_ctrl.rx_cnt;
    uint8_t cnt = user_can_ctrl.rx_buf[current_rx_index][0];

    uint16_t ot  = (uint16_t)(llc.protection_point.over_temp_point);
    uint16_t rec = (uint16_t)(llc.protection_point.over_temp_rec_point);

    uint8_t resp[8];
    resp[0] = cnt;
    resp[1] = 0xBE;                     /* 响应码 = 0x3E | 0x80 */
    resp[2] = (uint8_t)(ot  & 0xFF);
    resp[3] = (uint8_t)((ot  >> 8) & 0xFF);
    resp[4] = (uint8_t)(rec & 0xFF);
    resp[5] = (uint8_t)((rec >> 8) & 0xFF);
    resp[6] = 0x00;
    resp[7] = 0x00;

    can_send_data(resp, 8);
    return true;
}

// 将此条目加入 cmd_handlers 表：
//   {0x3E, handle_cmd_llc_temp_protect},
// 位置：在 {0x3F, handle_cmd_llc_voltage_protect} 之前
