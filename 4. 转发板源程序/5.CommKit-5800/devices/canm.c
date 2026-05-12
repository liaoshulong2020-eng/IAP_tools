/*
 * canm.c
 *
 *  Created on: 2024年7月24日
 *      Author: Liang Jinfeng
 */

#include "canm.h"

/*******************************************************************************
 * 静态变量
 ******************************************************************************/

//接收缓冲区
static uchar rxbuff[64];

//接收到数据回调函数指针
static void (*on_recv)(ulong id,const void *data,uchar size)=NULL;

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void canm_init(ulong baudrate)
{
	CAN_UserCfgTypeDef can_user_cfg;
	CAN_AcceptFilCfgTypeDef can_acpt_fil_cfg[1];

	memset((void *)&can_user_cfg,     0x00, sizeof(can_user_cfg));
	memset((void *)&can_acpt_fil_cfg, 0x00, sizeof(can_acpt_fil_cfg));

	//CAN acceptance filter config
	can_acpt_fil_cfg[0].slot = CAN_ACCEPT_FILT_SLOT_0;
	can_acpt_fil_cfg[0].code_val = 0;
	can_acpt_fil_cfg[0].mask_val = 0xffffffff;
	can_acpt_fil_cfg[0].rx_frm = CAN_ACCEPT_FILT_FRM_STD_EXT;

	//CAN LL Init
	can_user_cfg.fd_en = false;
	can_user_cfg.fd_iso_en = false;
	can_user_cfg.func_clk_freq = 120000000UL;
	can_user_cfg.baudrate_ss = baudrate;
	can_user_cfg.bit_timing_seg1_ss = 6;
	can_user_cfg.bit_timing_seg2_ss = 1;
	can_user_cfg.bit_timing_sjw_ss  = 1;
	can_user_cfg.rx_almost_full_limit = CAN_RX_ALMOST_FULL_LIMIT_1;
	can_user_cfg.err_limit = CAN_ERR_WARN_LIMIT_104;
	can_user_cfg.accept_fil_cfg_ptr = can_acpt_fil_cfg;
	can_user_cfg.accept_fil_cfg_num = ARRAY_SIZE(can_acpt_fil_cfg);
	LL_CAN_Init(CAN1, &can_user_cfg);
}

/*
 * 设置接收回调函数
 */
void canm_set_recv_callback(void (*callback)(ulong id,const void *data,uchar size))
{
	on_recv=callback;
}

/*
 * 检查发送器是否在忙
 */
bool canm_tx_is_busy()
{
	if(__LL_CAN_TxPriEn_Get(CAN1))return true;
	return false;
}

/*
 * 发送数据
 */
void canm_send_data(ulong id,const void *data,uchar size)
{
	CAN_TxBufFormatTypeDef tx_buf_fmt;
	memset((void *)&tx_buf_fmt, 0x0, sizeof(tx_buf_fmt));
	tx_buf_fmt.id_extension = 1;
	tx_buf_fmt.id = __LL_CAN_FrameIDFormat_29Bits(id);
	tx_buf_fmt.data_len_code = size;
	LL_CAN_TransmitPTB_CPU(CAN1, &tx_buf_fmt,(uint32_t*)data);
}

/*
 * 接收轮询数据
 */
void canm_rx_poll()
{
	ulong id;
	uchar size;
	CAN_RxBufFormatTypeDef rx_buf_fmt={0};
//	memset((void *)&rx_buf_fmt, 0x0, sizeof rx_buf_fmt);
	if(LL_CAN_Receive_CPU(CAN1,&rx_buf_fmt,(uint32_t*)rxbuff)!=LL_OK)return;
	id=rx_buf_fmt.id;
	size=rx_buf_fmt.data_len_code;
	if(!on_recv)return;
	on_recv(id,rxbuff,size);
}
