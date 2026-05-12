#ifndef PRI_SEC_COMMUN_H
#define PRI_SEC_COMMUN_H

#include "main.h"
#include <stdint.h>

void user_uart_init();
void User_UART_DeInit();
void User_Uart_TxCpltCallback(void);
void User_Uart_RxCpltCallback(void);

#define USER_UART_COM_BAUDRATE          (115200)
#define USER_UART										UART0


// ================= 类型定义 =================

/**
 * @brief Float/uint32_t 联合体,用于数据打包
 * @note 支持浮点数与字节数组之间的转换
 */
typedef union {
    float f;          // 浮点数访问
    uint32_t u32;     // 32位整数访问
    uint8_t b[4];     // 字节数组访问（用于通信）
} float_union_t;

/**
 * @brief 状态标志位联合体 (16位)
 * @note 使用位域节省空间，可通过 .all 整体访问或 .bits 按位访问
 */
typedef union {
    uint16_t all;     // 整体访问（用于清零或通信传输）
    struct {
        uint16_t input_ok           : 1;  // Bit 0:  输入电压正常
        uint16_t input_under_v      : 1;  // Bit 1:  输入欠压
        uint16_t input_over_v       : 1;  // Bit 2:  输入过压
        uint16_t output_under_v     : 1;  // Bit 3:  输出欠压
        uint16_t output_over_v      : 1;  // Bit 4:  输出过压
        uint16_t output_over_i      : 1;  // Bit 5:  输出过流
        uint16_t pfc_normal         : 1;  // Bit 6:  PFC正常运行
        uint16_t pre_finish         : 1;  // Bit 7:  预充电完成
        uint16_t start_delay        : 1;  // Bit 8:  启动延时中
        uint16_t current_calib      : 1;  // Bit 9:  电流校准完成
        uint16_t protect_set        : 1;  // Bit 10: 保护设置完成
        uint16_t input_mode_dc      : 1;  // Bit 11: DC输入模式
        uint16_t reserved           : 4;  // Bit 12-15: 保留位
    } bits;
} status_flags_union_t;

// ================= PFC 通信协议定义 =================

// PFC 发送帧协议
#define COMM_FRAME_HEADER_BYTE    0x55    // 帧头标识
#define COMM_FRAME_TAIL_BYTE      0xAA    // 帧尾标识
#define COMM_CMD_PFC_DETAIL_INFO  0x02    // 命令字：详细信息上报

// LLC 接收帧协议
#define LLC_FRAME_HEADER          0xAA    // 帧头标识
#define LLC_FRAME_TAIL            0x55    // 帧尾标识
#define LLC_FRAME_LENGTH          8       // 固定帧长度
#define CMD_LLC_VBUS_TARGET       0x11    // 命令字：目标总线电压

// ================= PFC 上报数据结构体 =================

/**
 * @brief PFC详细信息上报数据包
 * 
 * @note ?? 扩展友好设计：
 *       - 添加新字段后，无需手动修改任何宏定义
 *       - 所有长度自动通过 sizeof() 计算
 *       - 编译时断言自动验证结构体大小
 * 
 * @warning 必须使用 __attribute__((packed)) 以避免编译器自动对齐填充
 */
typedef struct {
    // ===== 实时测量值 (16 bytes) =====
    float_union_t vbus_target;       ///< 目标总线电压 (V)
    float_union_t vbus_rel;          ///< 实际总线电压 (V)
    float_union_t iloop_rel;         ///< 实际环路电流 (A)
    float_union_t vin_rel;           ///< 实际输入电压 (V)
    
    // ===== 原始采样值 (2 bytes) =====
    int16_t r_ntc_raw;               ///< NTC热敏电阻ADC原始值
    
    // ===== 保护设置点 (32 bytes) =====
    float_union_t vin_on_voltage_set;      ///< 输入启动电压阈值 (V)
    float_union_t vin_under_voltage_set;   ///< 输入欠压保护阈值 (V)
    float_union_t vin_over_voltage_set;    ///< 输入过压保护阈值 (V)
    float_union_t vin_max_voltage_set;     ///< 输入最高允许电压 (V)
    float_union_t vout_over_voltage_sw;    ///< 输出过压保护点-软件 (V)
    float_union_t bus_ovp_point_hw;        ///< 总线过压保护点-硬件 (V)
    float_union_t ipfc_ocp_current_sw;     ///< PFC过流保护点-软件 (A)
    float_union_t pfc_i_ocp_dac_point_hw;  ///< PFC过流保护点-硬件DAC (A)
    
    // ===== 控制状态 (6 bytes) =====
    uint8_t state;                   ///< PFC状态机当前状态
    uint8_t switch_frequency;        ///< 开关频率 (kHz，实际频率/1000)
    float_union_t duty_cycle;        ///< PWM占空比 (0.0 ~ 1.0)
    
    // ===== 状态标志位 (2 bytes) =====
    status_flags_union_t status_flags;  ///< 综合状态标志位
    
    // ===== ?? 扩展区域：在此添加新字段即可，无需修改其他代码 =====
    // 例如：
    // float_union_t new_voltage;     ///< 新增电压采样
    // uint16_t new_counter;          ///< 新增计数器
    
} __attribute__((packed)) PFC_REPORT_DATA_TypeDef;

// ================= ?? 自动计算的宏定义（无需手动修改）=================

/**
 * @brief 数据体长度 - 自动从结构体大小计算
 * @note 添加新字段后会自动更新，无需手动修改
 */
#define COMM_FRAME_DATA_SIZE      sizeof(PFC_REPORT_DATA_TypeDef)

/**
 * @brief 帧开销（固定值）
 * 组成：Header(1) + Cmd(1) + Len(1) + Checksum(1) + Tail(1) = 5 bytes
 */
#define COMM_FRAME_OVERHEAD       5

/**
 * @brief 完整帧总长度 - 自动计算
 * @note 公式：总长度 = 数据体大小 + 帧开销
 */
#define COMM_FRAME_TOTAL_SIZE     (COMM_FRAME_DATA_SIZE + COMM_FRAME_OVERHEAD)

/**
 * @brief 校验和计算起始位置（相对于帧头）
 * @note 校验和范围：[Cmd, Len, Data] = 从偏移1开始
 */
#define COMM_CHECKSUM_START_OFFSET  1

/**
 * @brief 校验和计算长度 - 自动计算
 * @note 范围：Cmd(1) + Len(1) + Data(自动)
 */
#define COMM_CHECKSUM_LENGTH      (2 + COMM_FRAME_DATA_SIZE)

/**
 * @brief 校验和位置（相对于帧头）
 * @note 位置：Header(1) + Cmd(1) + Len(1) + Data(自动) = 3 + 数据大小
 */
#define COMM_CHECKSUM_OFFSET      (3 + COMM_FRAME_DATA_SIZE)

/**
 * @brief 帧尾位置（相对于帧头）
 */
#define COMM_TAIL_OFFSET          (COMM_CHECKSUM_OFFSET + 1)

// ===== 编译时安全检查 =====

/**
 * @brief 验证基础类型大小
 */
_Static_assert(sizeof(float_union_t) == 4,
               "float_union_t size must be 4 bytes");

_Static_assert(sizeof(status_flags_union_t) == 2,
               "status_flags_union_t size must be 2 bytes");

/**
 * @brief 验证数据包大小限制
 * @note 限制原因：长度字段是 uint8_t，最大 255 字节
 */
_Static_assert(COMM_FRAME_DATA_SIZE <= 255,
               "Data size must not exceed 255 bytes (uint8_t limit)");

/**
 * @brief 验证帧总长度合理性
 * @note 典型 UART 缓冲区大小：256-1024 字节
 */
_Static_assert(COMM_FRAME_TOTAL_SIZE <= 512,
               "Total frame size should not exceed 512 bytes for typical UART buffers");

// ================= 帧结构文档宏 =================

/**
 * @brief 帧结构说明（用于调试输出）
 * 
 * 帧格式：
 * +--------+-----+-----+----------+----------+------+
 * | Header | Cmd | Len |   Data   | Checksum | Tail |
 * |  0x55  | 0x02| AUTO|   AUTO   |   AUTO   | 0xAA |
 * +--------+-----+-----+----------+----------+------+
 *    1B     1B    1B    sizeof()      1B       1B
 * 
 * AUTO = 自动从 sizeof(PFC_REPORT_DATA_TypeDef) 计算
 */
#define COMM_FRAME_FORMAT_DOC \
    "Frame: [0x55][0x02][LEN][DATA...][CHK][0xAA], " \
    "LEN=" XSTR(COMM_FRAME_DATA_SIZE) ", " \
    "TOTAL=" XSTR(COMM_FRAME_TOTAL_SIZE)

// 辅助宏：将数字转为字符串
#define XSTR(x) STR(x)
#define STR(x) #x

// ================= 函数接口声明 =================

/**
 * @brief 初始化LLC通信接口
 * @note 启动UART接收中断，准备接收LLC发送的命令帧
 */
void llc_comm_init(void);

/**
 * @brief 发送PFC详细信息到LLC
 * @return LL_OK: 发送成功启动, LL_ERROR: 发送失败
 * @note 该函数会自动：
 *       - 填充全局结构体 pfc_report_data
 *       - 计算数据长度（自动从 sizeof 获取）
 *       - 计算校验和
 *       - 通过 DMA 发送完整帧
 * @note 针对 Ofast 优化进行了特殊处理，使用 volatile 防止优化
 */
LL_StatusETypeDef uart_send_pfc_detail_info(void);

/**
 * @brief 根据LLC指令设置总线电压目标值
 * @note 会对接收到的目标电压进行范围检查
 * @note 只有在 PFC 运行状态下才会立即应用新的目标值
 */
void set_vbus_voltage(void);

/**
 * @brief UART接收完成中断回调函数
 * @note 由UART驱动层调用，用于处理接收到的LLC命令帧
 */
void User_Uart_RxCpltCallback(void);

/**
 * @brief UART发送完成中断回调函数
 * @note 由UART驱动层调用，可用于发送完成后的处理（当前为空实现）
 */
void User_Uart_TxCpltCallback(void);

// ================= 全局变量声明 =================

/**
 * @brief PFC上报数据全局变量
 * @note 使用 volatile 修饰，防止编译器优化
 * @note 该变量会在 uart_send_pfc_detail_info() 中被填充
 */
extern volatile PFC_REPORT_DATA_TypeDef pfc_report_data;

/**
 * @brief LLC发送的目标总线电压
 * @note 使用 volatile 修饰，防止编译器优化
 * @note 该变量由 UART 接收中断更新，主循环中读取使用
 */
extern volatile float llc_send_vbus_target;

// ================= ?? 扩展指南 =================

/**
 * @example 如何添加新字段到通信协议
 * 
 * 步骤1：在 PFC_REPORT_DATA_TypeDef 结构体末尾添加新字段
 * --------------------------------------------------------------
 * typedef struct {
 *     // ... 现有字段 ...
 *     
 *     // ?? 新字段
 *     float_union_t temperature;    // 新增温度
 *     uint16_t error_code;          // 新增错误码
 *     
 * } __attribute__((packed)) PFC_REPORT_DATA_TypeDef;
 * 
 * 步骤2：在 uart_send_pfc_detail_info() 中填充新字段
 * --------------------------------------------------------------
 * local_data.temperature.f = get_temperature();
 * local_data.error_code = pfc.error_code;
 * 
 * 步骤3：编译 - 无需修改任何其他代码！
 * --------------------------------------------------------------
 * - COMM_FRAME_DATA_SIZE 自动更新
 * - COMM_FRAME_TOTAL_SIZE 自动更新
 * - 校验和计算自动适配
 * - 帧长度字段自动填充正确值
 * 
 * 注意事项：
 * - 建议新字段添加在结构体末尾（保持向下兼容）
 * - 注意数据对齐（packed 模式下无自动填充）
 * - 总大小不要超过 255 字节（uint8_t 长度字段限制）
 */

#endif /* PRI_SEC_COMMUN_H */