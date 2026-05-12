/**
  ******************************************************************************
  * @file    pri_sec_commun.c
  * @brief   LLC与PFC双向通讯 - 非循环轮询模式
  *          副边(LLC)通过UART接收原边(PFC)发来的保护点及状态数据
  *          副边(LLC)通过UART向原边(PFC)发送自身运行数据
  ******************************************************************************
  */

#include "main.h"
#include "pri_sec_commun.h"
#include <string.h>

// =============================================================================
// 外部变量引用
// =============================================================================
extern volatile uint8_t uart_rx_buf[63];  /* 来自uart_app.c的DMA接收缓冲区 */

// =============================================================================
// LLC通讯帧定义（8字节固定协议）
// =============================================================================

/* PFC接收帧协议定义（接收端） */
#define PFC_FRAME_HEADER          0x55    /* PFC帧头 */
#define PFC_FRAME_TAIL            0xAA    /* PFC帧尾 */
#define PFC_CMD_DETAIL_INFO       0x02    /* PFC详细信息命令 */

/* 8字节通讯帧结构 */
#pragma pack(1)
typedef struct {
    uint8_t header;        /* 帧头 0xAA */
    uint8_t command;       /* 命令字 */
    union {
        float   float_val; /* 浮点数值 */
        uint8_t bytes[4];  /* 4字节数据 */
        uint32_t uint_val; /* 32位整数 */
    } data;
    uint8_t checksum;      /* 校验和 */
    uint8_t tail;          /* 帧尾 0x55 */
} comm_frame_8byte_t;
#pragma pack()

// =============================================================================
// 全局变量定义
// =============================================================================

/* LLC数据数组（供外部直接赋值） */
float llc_data_array[DATA_ARRAY_SIZE] = {0};

/* 存储最新接收到的PFC数据 */
PFC_RECEIVED_DATA_TypeDef pfc_received_data = {0};
volatile bool pfc_data_updated = false;
volatile uint32_t pfc_parse_success_count = 0;
volatile uint32_t pfc_parse_error_count = 0;

/* DMA状态跟踪 */
static uint32_t last_dma_count = 0;
static uint32_t parse_interval_counter = 0;

/* UART发送忙标志 */
static bool uart_tx_busy = false;

// =============================================================================
// 内部辅助函数
// =============================================================================

/**
  * @brief  CRC8校验计算（LLC发送用）
  */
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

/**
  * @brief  XOR校验计算（PFC接收帧验证用）
  */
static uint8_t calculate_xor_checksum(const uint8_t *data, uint16_t len)
{
    uint8_t checksum = 0;
    for (uint16_t i = 0; i < len; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

/**
  * @brief  构建8字节通讯帧（LLC发送给PFC）
  */
static void build_8byte_frame(comm_frame_8byte_t *frame, uint8_t cmd, float value)
{
    frame->header = COMM_FRAME_HEADER;
    frame->command = cmd;
    frame->data.float_val = value;
    /* 校验范围：command + data（共5字节） */
    frame->checksum = calculate_crc8(&frame->command, 5);
    frame->tail = COMM_FRAME_TAIL;
}

// =============================================================================
// PFC数据解析函数
// =============================================================================

/**
  * @brief  从uart_rx_buf解析PFC数据包
  * @return true: 解析成功, false: 解析失败
  *
  * @note   帧格式：
  *         [0]    = 0x55（帧头）
  *         [1]    = 0x02（命令字）
  *         [2]    = data_len（数据长度，= sizeof(PFC_RECEIVED_DATA_TypeDef)）
  *         [3..60]= PFC_RECEIVED_DATA_TypeDef结构体（58字节）
  *         [61]   = XOR校验（对[1..60]共60字节）
  *         [62]   = 0xAA（帧尾）
  */
bool parse_pfc_data_from_buffer(void)
{
    /* 拷贝到本地缓冲区，避免volatile访问竞争 */
    uint8_t local_buf[63];
    memcpy(local_buf, (const void*)uart_rx_buf, 63);

    /* 1. 验证帧头 */
    if (local_buf[0] != PFC_FRAME_HEADER) {
        pfc_parse_error_count++;
        return false;
    }

    /* 2. 验证命令字 */
    if (local_buf[1] != PFC_CMD_DETAIL_INFO) {
        pfc_parse_error_count++;
        return false;
    }

    /* 3. 验证数据长度 */
    uint8_t data_len = local_buf[2];
    if (data_len != sizeof(PFC_RECEIVED_DATA_TypeDef)) {
        pfc_parse_error_count++;
        return false;
    }

    /* 4. 验证帧尾 */
    if (local_buf[62] != PFC_FRAME_TAIL) {
        pfc_parse_error_count++;
        return false;
    }

    /* 5. 校验验证（范围：[1]到[60]共60字节） */
    uint8_t received_checksum = local_buf[61];
    uint8_t calculated_checksum = calculate_xor_checksum(&local_buf[1], 60);

    if (received_checksum != calculated_checksum) {
        pfc_parse_error_count++;
        return false;
    }

    /* 6. 提取数据结构（位置：[3]到[60]共58字节） */
    memcpy(&pfc_received_data, &local_buf[3], sizeof(PFC_RECEIVED_DATA_TypeDef));

    /* 7. 更新状态 */
    pfc_data_updated = true;
    pfc_parse_success_count++;

    return true;
}

/**
  * @brief  检查DMA传输状态并解析（方法2：检测传输计数变化）
  */
bool check_and_parse_pfc_data_method2(void)
{
    uint32_t current_count = DMA->CH[0].REG.CTR;

    if (current_count == 63 && last_dma_count != 63) {
        last_dma_count = current_count;
        return parse_pfc_data_from_buffer();
    }

    last_dma_count = current_count;
    return false;
}

/**
  * @brief  定时解析PFC数据（方法3：固定周期）
  * @param  interval: 调用间隔（每N次调用解析一次）
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
  * @brief  直接解析（方法4：最简单，直接在循环中调用）
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
    build_8byte_frame(&frame, CMD_LLC_VBUS_TARGET, llc_data_array[DATA_VBUS_TARGET]);
    uart_send_u8data((uint8_t*)&frame);
    return LL_OK;
}

LL_StatusETypeDef uart_send_llc_vout(void)
{
    comm_frame_8byte_t frame;
    build_8byte_frame(&frame, CMD_LLC_VOUT, llc_data_array[DATA_VOUT]);
    uart_send_u8data((uint8_t*)&frame);
    return LL_OK;
}

LL_StatusETypeDef uart_send_llc_iout(void)
{
    comm_frame_8byte_t frame;
    build_8byte_frame(&frame, CMD_LLC_IOUT, llc_data_array[DATA_IOUT]);
    uart_send_u8data((uint8_t*)&frame);
    return LL_OK;
}

LL_StatusETypeDef uart_send_llc_power(void)
{
    comm_frame_8byte_t frame;
    build_8byte_frame(&frame, CMD_LLC_POWER, llc_data_array[DATA_POWER]);
    uart_send_u8data((uint8_t*)&frame);
    return LL_OK;
}

LL_StatusETypeDef uart_send_llc_temp(void)
{
    comm_frame_8byte_t frame;
    build_8byte_frame(&frame, CMD_LLC_TEMP, llc_data_array[DATA_TEMP]);
    uart_send_u8data((uint8_t*)&frame);
    return LL_OK;
}

LL_StatusETypeDef uart_send_data_by_index(uint8_t cmd, uint8_t data_index)
{
    if (data_index >= DATA_ARRAY_SIZE) {
        return LL_INVALID;
    }
    comm_frame_8byte_t frame;
    build_8byte_frame(&frame, cmd, llc_data_array[data_index]);
    uart_send_u8data((uint8_t*)&frame);
    return LL_OK;
}

LL_StatusETypeDef uart_send_heartbeat(void)
{
    comm_frame_8byte_t frame;
    build_8byte_frame(&frame, CMD_HEARTBEAT, 0.0f);
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

/**
  * @brief  LLC周期性发送任务（轮流发送各类数据）
  *         在主循环或定时器中调用
  */
void uart_periodic_send_task(void)
{
    static uint8_t send_sequence = 0;

    switch (send_sequence) {
        case 0: uart_send_llc_vbus_target(); send_sequence++; break;
        case 1: uart_send_llc_vout();        send_sequence++; break;
        case 2: uart_send_llc_iout();        send_sequence++; break;
        case 3: uart_send_llc_power();       send_sequence++; break;
        case 4: uart_send_llc_temp();        send_sequence++; break;
        case 5: uart_send_heartbeat();       send_sequence = 0; break;
        default: send_sequence = 0; break;
    }
}

// =============================================================================
// LLC数据管理
// =============================================================================

void update_llc_data(uint8_t index, float value)
{
    if (index < DATA_ARRAY_SIZE) {
        llc_data_array[index] = value;
    }
}

void update_llc_data_batch(float vbus_target, float vout, float iout, float power, float temp)
{
    llc_data_array[DATA_VBUS_TARGET] = vbus_target;
    llc_data_array[DATA_VOUT]        = vout;
    llc_data_array[DATA_IOUT]        = iout;
    llc_data_array[DATA_POWER]       = power;
    llc_data_array[DATA_TEMP]        = temp;
}

float get_llc_data(uint8_t index)
{
    if (index < DATA_ARRAY_SIZE) {
        return llc_data_array[index];
    }
    return 0.0f;
}

float* get_llc_data_array(void)
{
    return llc_data_array;
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

// =============================================================================
// PFC数据访问接口
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

float get_pfc_vbus_target(void)      { return pfc_received_data.vbus_target.f; }
float get_pfc_vbus_rel(void)         { return pfc_received_data.vbus_rel.f; }
float get_pfc_iloop_rel(void)        { return pfc_received_data.iloop_rel.f; }
float get_pfc_vin_rel(void)          { return pfc_received_data.vin_rel.f; }
int16_t get_pfc_ntc_raw(void)        { return pfc_received_data.r_ntc_raw; }
float get_pfc_vin_on_voltage(void)   { return pfc_received_data.vin_on_voltage_set.f; }
float get_pfc_vin_under_voltage(void){ return pfc_received_data.vin_under_voltage_set.f; }
float get_pfc_vin_over_voltage(void) { return pfc_received_data.vin_over_voltage_set.f; }
float get_pfc_vin_max_voltage(void)  { return pfc_received_data.vin_max_voltage_set.f; }
float get_pfc_vout_over_voltage(void){ return pfc_received_data.vout_over_voltage_sw.f; }
float get_pfc_bus_ovp_point(void)    { return pfc_received_data.bus_ovp_point_hw.f; }
float get_pfc_ipfc_ocp_current(void) { return pfc_received_data.ipfc_ocp_current_sw.f; }
float get_pfc_i_ocp_dac_point(void)  { return pfc_received_data.pfc_i_ocp_dac_point_hw.f; }
uint8_t get_pfc_state(void)          { return pfc_received_data.state; }
float get_pfc_duty_cycle(void)       { return pfc_received_data.duty_cycle.f; }
uint8_t get_pfc_switch_frequency(void){ return pfc_received_data.switch_frequency; }

bool is_pfc_input_ok(void)           { return pfc_received_data.status_flags.bits.input_ok; }
bool is_pfc_input_under_v(void)      { return pfc_received_data.status_flags.bits.input_under_v; }
bool is_pfc_input_over_v(void)       { return pfc_received_data.status_flags.bits.input_over_v; }
bool is_pfc_output_under_v(void)     { return pfc_received_data.status_flags.bits.output_under_v; }
bool is_pfc_output_over_v(void)      { return pfc_received_data.status_flags.bits.output_over_v; }
bool is_pfc_output_over_i(void)      { return pfc_received_data.status_flags.bits.output_over_i; }
bool is_pfc_running(void)            { return pfc_received_data.status_flags.bits.pfc_normal; }
bool is_pfc_pre_finish(void)         { return pfc_received_data.status_flags.bits.pre_finish; }
bool is_pfc_start_delay(void)        { return pfc_received_data.status_flags.bits.start_delay; }
bool is_pfc_current_calib_ok(void)   { return pfc_received_data.status_flags.bits.current_calib; }
bool is_pfc_protect_set_ok(void)     { return pfc_received_data.status_flags.bits.protect_set; }
bool is_pfc_input_dc_mode(void)      { return pfc_received_data.status_flags.bits.input_mode_dc; }

uint16_t get_pfc_status_flags(void)
{
    return pfc_received_data.status_flags.all;
}

const PFC_RECEIVED_DATA_TypeDef* get_pfc_data_struct(void)
{
    return &pfc_received_data;
}

// =============================================================================
// UART回调函数（中断/DMA完成时调用）
// =============================================================================

void User_Uart_RxCpltCallback(void)
{
    /* DMA接收完成中断回调，解析PFC数据 */
    parse_pfc_data_from_buffer();
}

void User_Uart_TxCpltCallback(void)
{
    uart_tx_busy = false;
}
