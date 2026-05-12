#include "pri_sec_commun.h"
#include <string.h>
#include "tae32g58xx_ll.h"
#include <string.h>
#include "main.h"
#define DBG_TAG     "User UART"
#define DBG_LVL     DBG_LOG
#include "dbg/tae_dbg.h"

#if(UART_FUNC)
// ================= ȫ�ֱ��� =================

volatile PFC_REPORT_DATA_TypeDef pfc_report_data;
volatile float llc_send_vbus_target = 0.0f;
static uint8_t uart_rx_buf[LLC_FRAME_LENGTH];


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


// ================= ˽�к���:У��ͼ��� =================

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

// ================= �����߼� (�Զ�����汾) =================

/**
 * @brief ����PFC��ϸ��Ϣ֡
 * @note ?? �Զ�������ƣ�
 *       - ֡�����Զ��� sizeof() ����
 *       - У��ͷ�Χ�Զ��������ݴ�С
 *       - �������ֶκ������޸Ĵ˺���
 */
RAMCODE
LL_StatusETypeDef uart_send_pfc_detail_info(void)
{
    // ʹ�� volatile ���������ؼ�����ֹ�������Ż���
    // ��������С�Զ��Ӻ����
    static volatile uint8_t frame_buffer[COMM_FRAME_TOTAL_SIZE];
    PFC_REPORT_DATA_TypeDef local_data;
    
    // ========================================
    // ����1: ���ֲ��ṹ��
    // ========================================
    
    // ʵʱֵ
    local_data.vbus_target.f = pfc.vbus_target;
    local_data.vbus_rel.f = pfc.vbus_rel;
    local_data.iloop_rel.f = pfc.iloop.rel;
    local_data.vin_rel.f = pfc.vin_rel;
    local_data.r_ntc_raw = r_ntc_samp[0];
    
    // �������õ�
    local_data.vin_on_voltage_set.f = pfc.vin_on_voltage;
    local_data.vin_under_voltage_set.f = pfc.vin_under_voltage;
    local_data.vin_over_voltage_set.f = pfc.vin_over_voltage;
    local_data.vin_max_voltage_set.f = pfc.vin_max_voltage;
    local_data.vout_over_voltage_sw.f = VOUT_OVER_VOLTAGE;
    local_data.bus_ovp_point_hw.f = BUS_OVP_POINT;
    local_data.ipfc_ocp_current_sw.f = IPFC_OCP_CURRENT;
    local_data.pfc_i_ocp_dac_point_hw.f = PFC_I_OCP_DAC_POINT;
    
    // ����״̬
    local_data.state = (uint8_t)pfc.state;
    local_data.switch_frequency = (uint8_t)(PFC_DRIVER_CLK / 1000);
    local_data.duty_cycle.f = pfc.duty;
    
    // ״̬��־λ
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
    
    // ?? ���ֶ��������
    // ���������ֶΣ��ڴ˴����
    // ���磺
    // local_data.temperature.f = get_temperature();
    // local_data.error_code = pfc.error_code;
    
    // ���浽ȫ�ֱ���
    memcpy((void*)&pfc_report_data, &local_data, sizeof(PFC_REPORT_DATA_TypeDef));
    
    // ========================================
    // ����2: ����֡ͷ��Ԫ����
    // ========================================
    
    frame_buffer[0] = COMM_FRAME_HEADER_BYTE;
    frame_buffer[1] = COMM_CMD_PFC_DETAIL_INFO;
    
    // ?? �ؼ��������ֶ��Զ���䣨�� sizeof ���㣩
    frame_buffer[2] = (uint8_t)COMM_FRAME_DATA_SIZE;
    
    // ========================================
    // ����3: ����������
    // ========================================
    
    // ?? �Զ����䣺ѭ�������� COMM_FRAME_DATA_SIZE ����
    const uint8_t *src = (const uint8_t*)&local_data;
    for (uint16_t i = 0; i < COMM_FRAME_DATA_SIZE; i++) {
        frame_buffer[3 + i] = src[i];
    }
    
    // ========================================
    // ����4: ���㲢д��У���
    // ========================================
    
    // ?? �Զ����䣺У��ͷ�Χ�ɺ��Զ�����
    // ��Χ��frame_buffer[1] �� frame_buffer[2 + COMM_FRAME_DATA_SIZE]
    uint8_t checksum = 0;
    for (uint16_t i = 0; i < COMM_CHECKSUM_LENGTH; i++) {
        checksum ^= frame_buffer[COMM_CHECKSUM_START_OFFSET + i];
    }
    
    // ��׼�����ݣ����� volatile ����
    uint8_t tail_checksum = checksum;
    uint8_t tail_byte = COMM_FRAME_TAIL_BYTE;
    
    // ?? �Զ����䣺λ���ɺ����
    frame_buffer[COMM_CHECKSUM_OFFSET] = tail_checksum;
    frame_buffer[COMM_TAIL_OFFSET] = tail_byte;
    
    // ========================================
    // ����5: DMA ����
    // ========================================
    
    // ?? �Զ����䣺���䳤���ɺ����
    return LL_UART_Transmit_DMA(USER_UART, (uint8_t*)frame_buffer, COMM_FRAME_TOTAL_SIZE);
}

// ================= ����������߼� =================

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
    
    // 命令处理
    uint8_t cmd = frame[1];
    
    // IAP进入命令：cmd=0xFF表示要求PFC进入bootloader模式
    if (cmd == 0xFF) {
        // 收到IAP进入命令，复位进入bootloader
        NVIC_SystemReset();
        return;
    }
    
    if (cmd == CMD_LLC_VBUS_TARGET) {
        float_union_t converter;
        converter.b[0] = frame[2];
        converter.b[1] = frame[3];
        converter.b[2] = frame[4];
        converter.b[3] = frame[5];
        
        llc_send_vbus_target = converter.f;
    }
    
    // ?? �����������
    // ����������µ� LLC ����ڴ˴����ӽ�������
    // else if (cmd == CMD_LLC_NEW_COMMAND) {
    //     // ����������
    // }
}

// ================= ����ҵ���߼� =================

RAMCODE
void set_vbus_voltage(void)
{
    // ��Χ���
    if ((llc_send_vbus_target > VOUT_RECOVER_VOLTAGE) && 
        (llc_send_vbus_target < VOUT_OVER_VOLTAGE)) {
        pfc.vbus_target = llc_send_vbus_target;
    } else {
        pfc.vbus_target = VOUT_VOLTAGE;
    }
    
    // ����״̬������Ӧ��
    if (pfc.state == State_On) {
        pfc.vbus_ref = pfc.vbus_target;
    }
}

// ================= �жϻص� =================

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

