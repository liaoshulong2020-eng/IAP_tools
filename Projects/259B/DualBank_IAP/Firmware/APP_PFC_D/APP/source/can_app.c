#define USER_CAN_BAUDRATE           (500000)

/**
  * @brief User CAN standard frame ID
  */
#define USER_CAN_STD_FRM_ID         (0x12)

/**
  * @brief User CAN extended frame ID
  */
#define USER_CAN_EXT_FRM_ID         (0x345678)


void User_CAN_Init(CAN_TypeDef *Instance)
{
    CAN_UserCfgTypeDef can_user_cfg;
    CAN_AcceptFilCfgTypeDef can_acpt_fil_cfg[1];

    memset((void *)&can_user_cfg,     0x00, sizeof(can_user_cfg));
    memset((void *)&can_acpt_fil_cfg, 0x00, sizeof(can_acpt_fil_cfg));

    //CAN acceptance filter config
    can_acpt_fil_cfg[0].slot = CAN_ACCEPT_FILT_SLOT_0;
    can_acpt_fil_cfg[0].code_val = USER_CAN_STD_FRM_ID;
    can_acpt_fil_cfg[0].mask_val = 0;
    can_acpt_fil_cfg[0].rx_frm = CAN_ACCEPT_FILT_FRM_STD_EXT;

    //CAN LL Init
    can_user_cfg.fd_en = false;
    can_user_cfg.fd_iso_en = false;
    can_user_cfg.func_clk_freq = 120000000UL;
    can_user_cfg.baudrate_ss = USER_CAN_BAUDRATE;
    can_user_cfg.bit_timing_seg1_ss = 6;
    can_user_cfg.bit_timing_seg2_ss = 1;
    can_user_cfg.bit_timing_sjw_ss  = 1;
    can_user_cfg.rx_almost_full_limit = CAN_RX_ALMOST_FULL_LIMIT_1;
    can_user_cfg.err_limit = CAN_ERR_WARN_LIMIT_104;
    can_user_cfg.accept_fil_cfg_ptr = can_acpt_fil_cfg;
    can_user_cfg.accept_fil_cfg_num = ARRAY_SIZE(can_acpt_fil_cfg);
    LL_CAN_Init(Instance, &can_user_cfg);
}