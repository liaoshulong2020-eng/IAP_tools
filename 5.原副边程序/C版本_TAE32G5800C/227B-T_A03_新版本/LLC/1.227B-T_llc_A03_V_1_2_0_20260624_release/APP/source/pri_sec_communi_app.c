/**
  ******************************************************************************
  * @file    pri_sec_commun.c
  * @brief   LLC与PFC双向通信 - 主循环轮询模式
  * @note    不依赖DMA中断回调，在主循环中检测DMA状态并解析
  ******************************************************************************
  */

#include "main.h"
#include "pri_sec_communi_app.h"
#include <string.h>

// =============================================================================
// 外部变量声明
// =============================================================================
extern volatile uint8_t uart_rx_buf[63];  // 来自user_uart.c的DMA接收缓冲区

// =============================================================================
// LLC发送相关定义(8字节固定协议)
// =============================================================================

// 命令字定义
#define CMD_LLC_VBUS_TARGET  0x11    // LLC目标总线电压
#define CMD_LLC_VOUT         0x12    // LLC输出电压
#define CMD_LLC_IOUT         0x13    // LLC输出电流
#define CMD_LLC_POWER        0x14    // LLC输出功率
#define CMD_LLC_TEMP         0x15    // LLC温度
#define CMD_LLC_STATUS       0x16    // LLC状态
#define CMD_HEARTBEAT        0x01    // 心跳包

// 8字节通信帧结构
#pragma pack(1)
typedef struct {
    uint8_t header;        // 帧头 0xAA
    uint8_t command;       // 命令字
    union {
        float   float_val; // 浮点数值
        uint8_t bytes[4];  // 4字节数据
        uint32_t uint_val; // 32位整数
    } data;
    uint8_t checksum;      // 校验和
    uint8_t tail;          // 帧尾 0x55
} comm_frame_8byte_t;
#pragma pack()

// 数据存储数组（用户可直接赋值）
float llc_data_array[DATA_ARRAY_SIZE] = {0};

// LLC发送相关全局变量
static bool uart_tx_busy = false;

// =============================================================================
// PFC接收相关定义和变量
// =============================================================================

// PFC发送帧协议定义（接收端）
#define PFC_FRAME_HEADER          0x55    // PFC帧头
#define PFC_FRAME_TAIL            0xAA    // PFC帧尾
#define PFC_CMD_DETAIL_INFO       0x02    // PFC详细信息命令

// 存储解析后的PFC数据
PFC_RECEIVED_DATA_TypeDef pfc_received_data = {0};
volatile bool pfc_data_updated = false;
volatile uint32_t pfc_parse_success_count = 0;
volatile uint32_t pfc_parse_error_count = 0;

// DMA状态跟踪
static uint32_t last_dma_count = 0;
static uint32_t parse_interval_counter = 0;

// =============================================================================
// 基础功能函数
// =============================================================================

// CRC8校验计算函数（LLC发送用）
static uint8_t calculate_crc8(uint8_t *data, uint16_t len)
{
    uint8_t crc = 0x00;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;
            else
                crc = crc << 1;
        }
    }
    return crc;
}

// 异或校验和计算函数（PFC接收用）
static uint8_t calculate_xor_checksum(const uint8_t *data, uint16_t len)
{
    uint8_t checksum = 0;
    for (uint16_t i = 0; i < len; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

// 构建8字节通信帧（LLC发送用）
static void build_8byte_frame(comm_frame_8byte_t *frame, uint8_t cmd, float value)
{
    frame->header = COMM_FRAME_HEADER;
    frame->command = cmd;
    frame->data.float_val = value;
    
    // 计算校验和（从命令字到数据结束，共5字节）
    frame->checksum = calculate_crc8(&frame->command, 5);
    frame->tail = COMM_FRAME_TAIL;
}

// =============================================================================
// PFC数据解析函数（主循环轮询模式）
// =============================================================================

/**
 * @brief 解析uart_rx_buf中的PFC数据包
 * @return true: 解析成功, false: 解析失败
 * 
 * @note 在主循环中调用此函数，无需依赖DMA中断回调
 */
bool parse_pfc_data_from_buffer(void)
{
    // 创建局部缓冲区（避免volatile访问开销）
    uint8_t local_buf[63];
    memcpy(local_buf, (const void*)uart_rx_buf, 63);
    
    // 1. 验证帧头
    if (local_buf[0] != PFC_FRAME_HEADER) {
        pfc_parse_error_count++;
        return false;
    }
    
    // 2. 验证命令字
    if (local_buf[1] != PFC_CMD_DETAIL_INFO) {
        pfc_parse_error_count++;
        return false;
    }
    
    // 3. 验证数据长度
    uint8_t data_len = local_buf[2];
    if (data_len != sizeof(PFC_RECEIVED_DATA_TypeDef)) {
        pfc_parse_error_count++;
        return false;
    }
    
    // 4. 验证帧尾
    if (local_buf[62] != PFC_FRAME_TAIL) {
        pfc_parse_error_count++;
        return false;
    }
    
    // 5. 校验和验证（范围：[1] to [60]，共60字节）
    uint8_t received_checksum = local_buf[61];
    uint8_t calculated_checksum = calculate_xor_checksum(&local_buf[1], 60);
    
    if (received_checksum != calculated_checksum) {
        pfc_parse_error_count++;
        return false;
    }
    
    // 6. 提取数据体（位置：[3] to [60]，共58字节）
    memcpy(&pfc_received_data, &local_buf[3], sizeof(PFC_RECEIVED_DATA_TypeDef));
    
    // 7. 更新状态
    pfc_data_updated = true;
    pfc_parse_success_count++;
    
    return true;
}

/**
 * @brief 检查DMA接收状态并解析（方法1：检查DMA传输完成标志）
 * @return true: 有新数据并解析成功, false: 无新数据或解析失败
 * 
 * @note 在主循环中周期调用此函数
 */
//bool check_and_parse_pfc_data_method1(void)
//{
//    // 检查DMA传输完成标志
//    if (DMA->CH[0].REG.CSR & 0x02) {  // DMA_CHANNEL_0 传输完成标志
//        // 清除传输完成标志
//        DMA->CH[0].REG.CER = 1;
//        
//        // 解析数据
//        return parse_pfc_data_from_buffer();
//    }
//    
//    return false;
//}

/**
 * @brief 检查DMA接收状态并解析（方法2：检查传输计数变化）
 * @return true: 有新数据并解析成功, false: 无新数据或解析失败
 * 
 * @note 在主循环中周期调用此函数
 */
bool check_and_parse_pfc_data_method2(void)
{
    // 读取当前DMA传输计数
    uint32_t current_count = DMA->CH[0].REG.CTR;
    
    // 如果传输计数回到初始值，说明接收完成
    if (current_count == 63 && last_dma_count != 63) {
        last_dma_count = current_count;
        
        // 解析数据
        return parse_pfc_data_from_buffer();
    }
    
    last_dma_count = current_count;
    return false;
}

/**
 * @brief 定时解析PFC数据（方法3：固定周期解析）
 * @return true: 解析成功, false: 解析失败
 * 
 * @note 每隔N次调用解析一次（适合高速主循环）
 *       例如：主循环1ms一次，每10次解析一次，即10ms解析一次
 */
bool periodic_parse_pfc_data(uint32_t interval)
{
    parse_interval_counter++;
    
    if (parse_interval_counter >= interval) {
        parse_interval_counter = 0;
        return parse_pfc_data_from_buffer();
    }
    
    return false;
}

/**
 * @brief 简单直接解析（方法4：直接调用，最简单）
 * @return true: 解析成功, false: 解析失败
 * 
 * @note 在主循环中直接周期调用，依赖校验和来判断数据有效性
 *       这是最简单但可能最有效的方法
 */
bool simple_parse_pfc_data(void)
{
    return parse_pfc_data_from_buffer();
}

// =============================================================================
// LLC发送函数
// =============================================================================

LL_StatusETypeDef uart_send_llc_vbus_target(void)
{
    comm_frame_8byte_t frame;
    float vbus_target = llc_data_array[DATA_VBUS_TARGET];
    
    build_8byte_frame(&frame, CMD_LLC_VBUS_TARGET, vbus_target);
    uart_send_u8data((uint8_t*)&frame);
    
    return LL_OK;
}

LL_StatusETypeDef uart_send_llc_vout(void)
{
    comm_frame_8byte_t frame;
    float vout = llc_data_array[DATA_VOUT];
    
    build_8byte_frame(&frame, CMD_LLC_VOUT, vout);
    uart_send_u8data((uint8_t*)&frame);
    
    return LL_OK;
}

LL_StatusETypeDef uart_send_llc_iout(void)
{
    comm_frame_8byte_t frame;
    float iout = llc_data_array[DATA_IOUT];
    
    build_8byte_frame(&frame, CMD_LLC_IOUT, iout);
    uart_send_u8data((uint8_t*)&frame);
    
    return LL_OK;
}

LL_StatusETypeDef uart_send_llc_power(void)
{
    comm_frame_8byte_t frame;
    float power = llc_data_array[DATA_POWER];
    
    build_8byte_frame(&frame, CMD_LLC_POWER, power);
    uart_send_u8data((uint8_t*)&frame);
    
    return LL_OK;
}

LL_StatusETypeDef uart_send_llc_temp(void)
{
    comm_frame_8byte_t frame;
    float temp = llc_data_array[DATA_TEMP];
    
    build_8byte_frame(&frame, CMD_LLC_TEMP, temp);
    uart_send_u8data((uint8_t*)&frame);
    
    return LL_OK;
}

LL_StatusETypeDef uart_send_data_by_index(uint8_t cmd, uint8_t data_index)
{
    if (data_index >= DATA_ARRAY_SIZE) {
        return LL_INVALID;
    }
    
    comm_frame_8byte_t frame;
    float value = llc_data_array[data_index];
    
    build_8byte_frame(&frame, cmd, value);
    uart_send_u8data((uint8_t*)&frame);
    
    return LL_OK;
}

LL_StatusETypeDef uart_send_heartbeat(void)
{
    comm_frame_8byte_t frame;
    float heartbeat_data = 0.0f;
    
    build_8byte_frame(&frame, CMD_HEARTBEAT, heartbeat_data);
    uart_send_u8data((uint8_t*)&frame);
    
    return LL_OK;
}

LL_StatusETypeDef uart_send_float_data(uint8_t cmd, float value)
{
    comm_frame_8byte_t frame;
    build_8byte_frame(&frame, cmd, value);
    
    uart_send_u8data((uint8_t*)&frame);
    
    return LL_OK;
}

// =============================================================================
// PFC数据获取接口
// =============================================================================

bool is_pfc_data_updated(void)
{
    return pfc_data_updated;
}

void clear_pfc_data_update_flag(void)
{
    pfc_data_updated = false;
}

uint32_t get_pfc_parse_success_count(void)
{
    return pfc_parse_success_count;
}

uint32_t get_pfc_parse_error_count(void)
{
    return pfc_parse_error_count;
}

float get_pfc_vbus_target(void)
{
    return pfc_received_data.vbus_target.f;
}

float get_pfc_vbus_rel(void)
{
    return pfc_received_data.vbus_rel.f;
}

float get_pfc_iloop_rel(void)
{
    return pfc_received_data.iloop_rel.f;
}

float get_pfc_vin_rel(void)
{
    return pfc_received_data.vin_rel.f;
}

int16_t get_pfc_ntc_raw(void)
{
    return pfc_received_data.r_ntc_raw;
}

float get_pfc_vin_on_voltage(void)
{
    return pfc_received_data.vin_on_voltage_set.f;
}

float get_pfc_vin_under_voltage(void)
{
    return pfc_received_data.vin_under_voltage_set.f;
}

float get_pfc_vin_over_voltage(void)
{
    return pfc_received_data.vin_over_voltage_set.f;
}

float get_pfc_vin_max_voltage(void)
{
    return pfc_received_data.vin_max_voltage_set.f;
}

float get_pfc_vout_over_voltage(void)
{
    return pfc_received_data.vout_over_voltage_sw.f;
}

float get_pfc_bus_ovp_point(void)
{
    return pfc_received_data.bus_ovp_point_hw.f;
}

float get_pfc_ipfc_ocp_current(void)
{
    return pfc_received_data.ipfc_ocp_current_sw.f;
}

float get_pfc_i_ocp_dac_point(void)
{
    return pfc_received_data.pfc_i_ocp_dac_point_hw.f;
}

uint8_t get_pfc_state(void)
{
    return pfc_received_data.state;
}

float get_pfc_duty_cycle(void)
{
    return pfc_received_data.duty_cycle.f;
}

uint8_t get_pfc_switch_frequency(void)
{
    return pfc_received_data.switch_frequency;
}

bool is_pfc_input_ok(void)
{
    return pfc_received_data.status_flags.bits.input_ok;
}

bool is_pfc_input_under_v(void)
{
    return pfc_received_data.status_flags.bits.input_under_v;
}

bool is_pfc_input_over_v(void)
{
    return pfc_received_data.status_flags.bits.input_over_v;
}

bool is_pfc_output_under_v(void)
{
    return pfc_received_data.status_flags.bits.output_under_v;
}

bool is_pfc_output_over_v(void)
{
    return pfc_received_data.status_flags.bits.output_over_v;
}

bool is_pfc_output_over_i(void)
{
    return pfc_received_data.status_flags.bits.output_over_i;
}

bool is_pfc_running(void)
{
    return pfc_received_data.status_flags.bits.pfc_normal;
}

bool is_pfc_pre_finish(void)
{
    return pfc_received_data.status_flags.bits.pre_finish;
}

bool is_pfc_start_delay(void)
{
    return pfc_received_data.status_flags.bits.start_delay;
}

bool is_pfc_current_calib_ok(void)
{
    return pfc_received_data.status_flags.bits.current_calib;
}

bool is_pfc_protect_set_ok(void)
{
    return pfc_received_data.status_flags.bits.protect_set;
}

bool is_pfc_input_dc_mode(void)
{
    return pfc_received_data.status_flags.bits.input_mode_dc;
}

uint16_t get_pfc_status_flags(void)
{
    return pfc_received_data.status_flags.all;
}

const PFC_RECEIVED_DATA_TypeDef* get_pfc_data_struct(void)
{
    return &pfc_received_data;
}

// =============================================================================
// LLC发送周期任务和辅助函数
// =============================================================================

void uart_periodic_send_task(void)
{
    static uint8_t send_sequence = 0;
    
    switch (send_sequence) {
        case 0:
            uart_send_llc_vbus_target();
            send_sequence++;
            break;
        case 1:
            uart_send_llc_vout();
            send_sequence++;
            break;
        case 2:
            uart_send_llc_iout();
            send_sequence++;
            break;
        case 3:
            uart_send_llc_power();
            send_sequence++;
            break;
        case 4:
            uart_send_llc_temp();
            send_sequence++;
            break;
        case 5:
            uart_send_heartbeat();
            send_sequence = 0;
            break;
        default:
            send_sequence = 0;
            break;
    }
}

void update_llc_data(uint8_t index, float value)
{
    if (index < DATA_ARRAY_SIZE) {
        llc_data_array[index] = value;
    }
}

void update_llc_data_batch(float vbus_target, float vout, float iout, float power, float temp)
{
    llc_data_array[DATA_VBUS_TARGET] = vbus_target;
    llc_data_array[DATA_VOUT] = vout;
    llc_data_array[DATA_IOUT] = iout;
    llc_data_array[DATA_POWER] = power;
    llc_data_array[DATA_TEMP] = temp;
}

float get_llc_data(uint8_t index)
{
    if (index < DATA_ARRAY_SIZE) {
        return llc_data_array[index];
    }
    return 0.0f;
}

bool is_uart_busy(void)
{
    return uart_tx_busy;
}

void uart_force_reset(void)
{
    uart_tx_busy = false;
}

void uart_send_info(void)
{
    uart_send_llc_vbus_target();
}

float* get_llc_data_array(void)
{
    return llc_data_array;
}

// =============================================================================
// UART回调函数（保留但可能不会被调用）
// =============================================================================

void User_Uart_RxCpltCallback(void)
{
    // 如果DMA中断回调能工作，也可以在这里解析
    parse_pfc_data_from_buffer();
}

void User_Uart_TxCpltCallback(void)
{
    uart_tx_busy = false;
}