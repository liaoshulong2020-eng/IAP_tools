/**
  ******************************************************************************
  * @file    pri_sec_commun.h
  * @brief   LLC与PFC双向通讯头文件 - 非循环轮询模式
  *          副边通过UART接收原边(PFC)发来的保护点数据
  ******************************************************************************
  */

#ifndef PRI_SEC_COMMUN_H
#define PRI_SEC_COMMUN_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "main.h"

// =============================================================================
// LLC通讯协议定义
// =============================================================================
#define COMM_FRAME_HEADER    0xAA
#define COMM_FRAME_TAIL      0x55
#define COMM_FRAME_LENGTH    8

// LLC发送命令字定义
#define CMD_LLC_VBUS_TARGET  0x11
#define CMD_LLC_VOUT         0x12
#define CMD_LLC_IOUT         0x13
#define CMD_LLC_POWER        0x14
#define CMD_LLC_TEMP         0x15
#define CMD_LLC_STATUS       0x16
#define CMD_HEARTBEAT        0x01

// LLC数据数组索引定义
#define DATA_VBUS_TARGET    0
#define DATA_VOUT           1
#define DATA_IOUT           2
#define DATA_POWER          3
#define DATA_TEMP           4
#define DATA_VBUS_ACTUAL    5
#define DATA_EFFICIENCY     6
#define DATA_FREQUENCY      7
#define DATA_ARRAY_SIZE     8

// =============================================================================
// PFC数据结构定义
// =============================================================================

typedef union {
    float f;
    uint32_t u32;
    uint8_t b[4];
} float_union_t;

typedef union {
    uint16_t all;
    struct {
        uint16_t input_ok           : 1;
        uint16_t input_under_v      : 1;
        uint16_t input_over_v       : 1;
        uint16_t output_under_v     : 1;
        uint16_t output_over_v      : 1;
        uint16_t output_over_i      : 1;
        uint16_t pfc_normal         : 1;
        uint16_t pre_finish         : 1;
        uint16_t start_delay        : 1;
        uint16_t current_calib      : 1;
        uint16_t protect_set        : 1;
        uint16_t input_mode_dc      : 1;
        uint16_t reserved           : 4;
    } bits;
} status_flags_union_t;

typedef struct {
    float_union_t vbus_target;              // PFC目标母线电压
    float_union_t vbus_rel;                 // PFC实际母线电压
    float_union_t iloop_rel;                // PFC电流环实际值
    float_union_t vin_rel;                  // PFC输入电压实际值
    int16_t r_ntc_raw;                      // NTC原始值
    float_union_t vin_on_voltage_set;       // 输入启动电压设定
    float_union_t vin_under_voltage_set;    // 输入欠压设定
    float_union_t vin_over_voltage_set;     // 输入过压设定
    float_union_t vin_max_voltage_set;      // 输入最大电压设定
    float_union_t vout_over_voltage_sw;     // 输出过压软件保护点
    float_union_t bus_ovp_point_hw;         // 母线过压硬件保护点
    float_union_t ipfc_ocp_current_sw;      // PFC过流软件保护点
    float_union_t pfc_i_ocp_dac_point_hw;  // PFC过流DAC硬件保护点
    uint8_t state;                          // PFC状态
    uint8_t switch_frequency;              // 开关频率
    float_union_t duty_cycle;              // 占空比
    status_flags_union_t status_flags;     // 状态标志位（16位）
} __attribute__((packed)) PFC_RECEIVED_DATA_TypeDef;

// =============================================================================
// 全局变量声明
// =============================================================================

extern float llc_data_array[DATA_ARRAY_SIZE];
extern PFC_RECEIVED_DATA_TypeDef pfc_received_data;
extern volatile bool pfc_data_updated;

// =============================================================================
// PFC数据解析函数
// =============================================================================

bool parse_pfc_data_from_buffer(void);
bool check_and_parse_pfc_data_method2(void);
bool periodic_parse_pfc_data(uint32_t interval);
bool simple_parse_pfc_data(void);

// =============================================================================
// LLC发送函数接口
// =============================================================================

LL_StatusETypeDef uart_send_llc_vbus_target(void);
LL_StatusETypeDef uart_send_llc_vout(void);
LL_StatusETypeDef uart_send_llc_iout(void);
LL_StatusETypeDef uart_send_llc_power(void);
LL_StatusETypeDef uart_send_llc_temp(void);
LL_StatusETypeDef uart_send_data_by_index(uint8_t cmd, uint8_t data_index);
LL_StatusETypeDef uart_send_heartbeat(void);
LL_StatusETypeDef uart_send_float_data(uint8_t cmd, float value);
void uart_periodic_send_task(void);
void uart_send_info(void);

// =============================================================================
// LLC数据管理
// =============================================================================

void update_llc_data(uint8_t index, float value);
void update_llc_data_batch(float vbus_target, float vout, float iout, float power, float temp);
float get_llc_data(uint8_t index);
float* get_llc_data_array(void);
bool is_uart_busy(void);
void uart_force_reset(void);

// =============================================================================
// PFC数据访问接口
// =============================================================================

bool is_pfc_data_updated(void);
void clear_pfc_data_update_flag(void);
uint32_t get_pfc_parse_success_count(void);
uint32_t get_pfc_parse_error_count(void);

// 实时测量值
float get_pfc_vbus_target(void);
float get_pfc_vbus_rel(void);
float get_pfc_iloop_rel(void);
float get_pfc_vin_rel(void);
int16_t get_pfc_ntc_raw(void);

// 保护点设定
float get_pfc_vin_on_voltage(void);
float get_pfc_vin_under_voltage(void);
float get_pfc_vin_over_voltage(void);
float get_pfc_vin_max_voltage(void);
float get_pfc_vout_over_voltage(void);
float get_pfc_bus_ovp_point(void);
float get_pfc_ipfc_ocp_current(void);
float get_pfc_i_ocp_dac_point(void);

// 运行状态
uint8_t get_pfc_state(void);
float get_pfc_duty_cycle(void);
uint8_t get_pfc_switch_frequency(void);

// 状态标志位
bool is_pfc_input_ok(void);
bool is_pfc_input_under_v(void);
bool is_pfc_input_over_v(void);
bool is_pfc_output_under_v(void);
bool is_pfc_output_over_v(void);
bool is_pfc_output_over_i(void);
bool is_pfc_running(void);
bool is_pfc_pre_finish(void);
bool is_pfc_start_delay(void);
bool is_pfc_current_calib_ok(void);
bool is_pfc_protect_set_ok(void);
bool is_pfc_input_dc_mode(void);
uint16_t get_pfc_status_flags(void);
const PFC_RECEIVED_DATA_TypeDef* get_pfc_data_struct(void);

// =============================================================================
// UART回调函数
// =============================================================================

void User_Uart_RxCpltCallback(void);
void User_Uart_TxCpltCallback(void);

// =============================================================================
// 便捷宏定义
// =============================================================================

#define PFC_DATA_READY()        is_pfc_data_updated()
#define CLEAR_PFC_FLAG()        clear_pfc_data_update_flag()
#define PFC_VBUS()              get_pfc_vbus_rel()
#define PFC_VIN()               get_pfc_vin_rel()
#define PFC_ILOOP()             get_pfc_iloop_rel()
#define PFC_IS_RUNNING()        is_pfc_running()

#endif /* PRI_SEC_COMMUN_H */
