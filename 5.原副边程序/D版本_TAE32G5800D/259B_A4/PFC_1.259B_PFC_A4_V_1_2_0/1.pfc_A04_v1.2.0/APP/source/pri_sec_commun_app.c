#include "pri_sec_commun_app.h"
#include <string.h>
#include "tae32g58xx_ll.h"
#include <string.h>
#include "main.h"
#define DBG_TAG     "User UART"
#define DBG_LVL     DBG_LOG
#include "dbg/tae_dbg.h"

#if(UART_FUNC)
// ================= 全局变量 =================

volatile PFC_REPORT_DATA_TypeDef pfc_report_data;
volatile float llc_send_vbus_target = 0.0f;
static uint8_t uart_rx_buf[LLC_FRAME_LENGTH];

#define PFC_IAP_BOOT_MAGIC_ADDR     (0x2001FFF0UL)
#define PFC_IAP_BOOT_MAGIC_VALUE    (0x50464349UL)

RAMCODE
static void pfc_request_iap_boot(void)
{
    *(volatile uint32_t *)PFC_IAP_BOOT_MAGIC_ADDR = PFC_IAP_BOOT_MAGIC_VALUE;
    __DSB();
    NVIC_SystemReset();
}


void user_uart_init()
{
    UART_InitTypeDef uart_init;

    memset((void *)&uart_init, 0, sizeof(uart_init));

    //User UART Init
    uart_init.baudrate = USER_UART_COM_BAUDRATE;
    uart_init.dat_len  = UART_DAT_LEN_8b;
    uart_init.parity   = UART_PARITY_NO;
    uart_init.stop_len = UART_STOP_LEN_1b;
	  uart_init.user_callback.TxCpltCallback = User_Uart_TxCpltCallback;
		uart_init.user_callback.RxCpltCallback = User_Uart_RxCpltCallback;
    LL_UART_Init(USER_UART, &uart_init);
	                          
		__LL_UART_RxFull_INT_En(USER_UART);
	  LL_NVIC_SetPriority(UART0_IRQn, 4, 0);
		llc_comm_init();
}


// ================= 私有函数:校验和计算 =================

RAMCODE
static uint8_t calculate_checksum(const uint8_t *data, uint16_t len)
{
    uint8_t checksum = 0;
    for (uint16_t i = 0; i < len; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

RAMCODE
static uint8_t llc_calculate_crc8(const uint8_t *data, uint16_t len)
{
    uint8_t crc = 0x00;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc = crc << 1;
            }
        }
    }
    return crc;
}

// ================= 发送逻辑 (自动适配版本) =================

/**
 * @brief 发送PFC详细信息帧
 * @note ?? 自动适配设计：
 *       - 帧长度自动从 sizeof() 计算
 *       - 校验和范围自动适配数据大小
 *       - 添加新字段后无需修改此函数
 */
RAMCODE
LL_StatusETypeDef uart_send_pfc_detail_info(void)
{
    // 使用 volatile 缓冲区（关键：防止编译器优化）
    // 缓冲区大小自动从宏计算
    static volatile uint8_t frame_buffer[COMM_FRAME_TOTAL_SIZE];
    PFC_REPORT_DATA_TypeDef local_data;
    
    // ========================================
    // 步骤1: 填充局部结构体
    // ========================================
    
    // 实时值
    local_data.vbus_target.f = pfc.vbus_target;
    local_data.vbus_rel.f = pfc.vbus_rel;
    local_data.iloop_rel.f = pfc.iloop.rel;
    local_data.vin_rel.f = pfc.vin_rel;
    local_data.r_ntc_raw = r_ntc_samp[0];
    
    // 保护设置点
    local_data.vin_on_voltage_set.f = pfc.vin_on_voltage;
    local_data.vin_under_voltage_set.f = pfc.vin_under_voltage;
    local_data.vin_over_voltage_set.f = pfc.vin_over_voltage;
    local_data.vin_max_voltage_set.f = pfc.vin_max_voltage;
    local_data.vout_over_voltage_sw.f = VOUT_OVER_VOLTAGE;
    local_data.bus_ovp_point_hw.f = BUS_OVP_POINT;
    local_data.ipfc_ocp_current_sw.f = IPFC_OCP_CURRENT;
    local_data.pfc_i_ocp_dac_point_hw.f = PFC_I_OCP_DAC_POINT;
    
    // 控制状态
    local_data.state = (uint8_t)pfc.state;
    local_data.switch_frequency = (uint8_t)(PFC_DRIVER_CLK / 1000);
    local_data.duty_cycle.f = pfc.duty;
    
    // 状态标志位
    local_data.status_flags.all = 0;
    local_data.status_flags.bits.input_ok       = pfc.is_ac_ok;
    local_data.status_flags.bits.input_under_v  = pfc.under_input_flag;
    local_data.status_flags.bits.input_over_v   = pfc.over_input_flag;
    local_data.status_flags.bits.output_under_v = (pfc.vbus_rel < VOUT_UNDER_VOLTAGE);
    local_data.status_flags.bits.output_over_v  = pfc.V_over_output_flag;
    local_data.status_flags.bits.output_over_i  = pfc.I_over_output_flag;
    local_data.status_flags.bits.pfc_normal     = (pfc.state == State_On);
    local_data.status_flags.bits.pre_finish     = pfc.pre_finish_flg;
    local_data.status_flags.bits.start_delay    = pfc.start_cnt_flg;
    local_data.status_flags.bits.current_calib  = pfc.check_current_is_ok;
    local_data.status_flags.bits.protect_set    = pfc.set_protect_is_ok;
    local_data.status_flags.bits.input_mode_dc  = (pfc.input_mode == DC_MODE);
    
    // ?? 新字段填充区域
    // 如添加新字段，在此处填充
    // 例如：
    // local_data.temperature.f = get_temperature();
    // local_data.error_code = pfc.error_code;
    
    // 保存到全局变量
    memcpy((void*)&pfc_report_data, &local_data, sizeof(PFC_REPORT_DATA_TypeDef));
    
    // ========================================
    // 步骤2: 构造帧头和元数据
    // ========================================
    
    frame_buffer[0] = COMM_FRAME_HEADER_BYTE;
    frame_buffer[1] = COMM_CMD_PFC_DETAIL_INFO;
    
    // ?? 关键：长度字段自动填充（从 sizeof 计算）
    frame_buffer[2] = (uint8_t)COMM_FRAME_DATA_SIZE;
    
    // ========================================
    // 步骤3: 复制数据体
    // ========================================
    
    // ?? 自动适配：循环次数由 COMM_FRAME_DATA_SIZE 决定
    const uint8_t *src = (const uint8_t*)&local_data;
    for (uint16_t i = 0; i < COMM_FRAME_DATA_SIZE; i++) {
        frame_buffer[3 + i] = src[i];
    }
    
    // ========================================
    // 步骤4: 计算并写入校验和
    // ========================================
    
    // ?? 自动适配：校验和范围由宏自动计算
    // 范围：frame_buffer[1] 到 frame_buffer[2 + COMM_FRAME_DATA_SIZE]
    uint8_t checksum = 0;
    for (uint16_t i = 0; i < COMM_CHECKSUM_LENGTH; i++) {
        checksum ^= frame_buffer[COMM_CHECKSUM_START_OFFSET + i];
    }
    
    // 先准备数据，减少 volatile 访问
    uint8_t tail_checksum = checksum;
    uint8_t tail_byte = COMM_FRAME_TAIL_BYTE;
    
    // ?? 自动适配：位置由宏计算
    frame_buffer[COMM_CHECKSUM_OFFSET] = tail_checksum;
    frame_buffer[COMM_TAIL_OFFSET] = tail_byte;
    
    // ========================================
    // 步骤5: DMA 传输
    // ========================================
    
    // ?? 自动适配：传输长度由宏计算
    return LL_UART_Transmit_DMA(USER_UART, (uint8_t*)frame_buffer, COMM_FRAME_TOTAL_SIZE);
}

// ================= 接收与解析逻辑 =================

RAMCODE
static void parse_llc_frame(const uint8_t *frame)
{
    // 帧格式验证
    if (frame[0] != LLC_FRAME_HEADER || frame[LLC_FRAME_LENGTH - 1] != LLC_FRAME_TAIL) {
        return;
    }
    
    // CRC校验
    uint8_t calc_crc = llc_calculate_crc8(&frame[1], 5);
    if (calc_crc != frame[6]) {
        return;
    }
    
    // 解析命令
    uint8_t cmd = frame[1];
    
    if (cmd == CMD_LLC_ENTER_IAP) {
        pfc_request_iap_boot();
    } else if (cmd == CMD_LLC_VBUS_TARGET) {
        float_union_t converter;
        converter.b[0] = frame[2];
        converter.b[1] = frame[3];
        converter.b[2] = frame[4];
        converter.b[3] = frame[5];
        
        llc_send_vbus_target = converter.f;
    }
    
    // ?? 新命令处理区域
    // 如果添加了新的 LLC 命令，在此处添加解析代码
    // else if (cmd == CMD_LLC_NEW_COMMAND) {
    //     // 处理新命令
    // }
}

// ================= 核心业务逻辑 =================

RAMCODE
void set_vbus_voltage(void)
{
    // 范围检查
    if ((llc_send_vbus_target > VOUT_RECOVER_VOLTAGE) && 
        (llc_send_vbus_target < VOUT_OVER_VOLTAGE)) {
        pfc.vbus_target = llc_send_vbus_target;
    } else {
        pfc.vbus_target = VOUT_VOLTAGE;
    }
    
    // 运行状态下立即应用
    if (pfc.state == State_On) {
        pfc.vbus_ref = pfc.vbus_target;
    }
}

// ================= 中断回调 =================

RAMCODE
void llc_comm_init(void)
{
    LL_UART_Receive_IT(USER_UART, uart_rx_buf, LLC_FRAME_LENGTH);
}

RAMCODE
void User_Uart_RxCpltCallback(void)
{
    parse_llc_frame(uart_rx_buf);
    set_vbus_voltage();
    LL_UART_Receive_IT(USER_UART, uart_rx_buf, LLC_FRAME_LENGTH);
}

RAMCODE
void User_Uart_TxCpltCallback(void)
{
    DMA->CH[0].REG.CER = 1;
}


#endif

