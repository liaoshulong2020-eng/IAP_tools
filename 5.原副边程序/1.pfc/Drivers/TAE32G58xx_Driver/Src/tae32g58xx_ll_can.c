/**
  ******************************************************************************
  * @file    tae32g58xx_ll_can.c
  * @author  MCD Application Team
  * @brief   CAN LL module driver
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 Tai-Action.
  * All rights reserved.</center></h2>
  *
  * This software is licensed by Tai-Action under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "tae32g58xx_ll.h"
#include <string.h>


#define DBG_TAG             "CAN LL"
#define DBG_LVL             DBG_ERROR
#include "dbg/tae_dbg.h"


/** @addtogroup TAE32G58xx_LL_Driver
  * @{
  */

/** @defgroup CAN_LL CAN LL
  * @brief    CAN LL Module Driver
  * @{
  */

#ifdef LL_CAN_MODULE_ENABLED

/* Private Constants ---------------------------------------------------------*/
/* Private Macros ------------------------------------------------------------*/
/* Private Types -------------------------------------------------------------*/
/** @defgroup CAN_LL_Private_Types CAN LL Private Types
  * @brief    CAN LL Private Types
  * @{
  */

/**
  * @brief CAN IRQ callback function type definition
  */
typedef void (*CAN_LLIRQCallback)(CAN_TypeDef *Instance);

/**
  * @brief CAN Transmission Tx definition
  */
typedef struct __CAN_TransTxTypeDef {
    CAN_StateETypeDef state;                    /*!< CAN Transmission State         */
    CAN_LLIRQCallback isr;                      /*!< Interrupt Service Routine      */
} CAN_TransTxTypeDef;

/**
  * @brief CAN Transmission Rx definition
  */
typedef struct __CAN_TransRxTypeDef {
    uint32_t *buf;                              /*!< CAN Rx Buffer Pointer          */
    CAN_RxBufFormatTypeDef *buf_fmt;            /*!< CAN Rx Buffer format Pointer   */
    CAN_StateETypeDef state;                    /*!< CAN Transmission State         */
    CAN_LLIRQCallback isr;                      /*!< Interrupt Service Routine      */
} CAN_TransRxTypeDef;

/**
  * @brief CAN handle Structure definition
  */
typedef struct __CAN_HandleTypeDef {
    volatile CAN_TransTxTypeDef tx_ctrl_ptb;    /*!< CAN PTB Transmission Tx Control    */
    volatile CAN_TransTxTypeDef tx_ctrl_stb;    /*!< CAN STB Transmission Tx Control    */
    volatile CAN_TransRxTypeDef rx_ctrl;        /*!< CAN Transmission Rx Control        */

    CAN_UserCallbackTypeDef user_callback;      /*!< User Callback                      */
} CAN_HandleTypeDef;

/**
  * @}
  */


/* Private Variables ---------------------------------------------------------*/
/** @defgroup CAN_LL_Private_Variables CAN LL Private Variables
  * @brief    CAN LL Private Variables
  * @{
  */

/**
  * @brief CAN Handle global variable
  */
static CAN_HandleTypeDef can_hdl_global[CAN_INSTANCE_NUMS];

/**
  * @}
  */


/* Private Function Prototypes -----------------------------------------------*/
/** @addtogroup CAN_LL_Private_Functions
  * @{
  */
static CAN_HandleTypeDef *CAN_Handle_Get(CAN_TypeDef *Instance);
static void CAN_TxISR_PTB(CAN_TypeDef *Instance);
static void CAN_TxISR_STB(CAN_TypeDef *Instance);
static void CAN_RxISR(CAN_TypeDef *Instance);

static LL_StatusETypeDef CAN_AcceptFilRst(CAN_TypeDef *Instance, CAN_AcceptFilSlotETypeDef slot);
static LL_StatusETypeDef CAN_AcceptFilCfg(CAN_TypeDef *Instance, CAN_AcceptFilCfgTypeDef *fil_cfg);
/**
  * @}
  */


/* Exported Variables --------------------------------------------------------*/
/* Exported Functions --------------------------------------------------------*/
/** @defgroup CAN_LL_Exported_Functions CAN LL Exported Functions
  * @brief    CAN LL Exported Functions
  * @{
  */

/** @defgroup CAN_LL_Exported_Functions_Group1 CAN Init and DeInit Functions
  * @brief    CAN Init and DeInit Functions
  * @{
  */

/**
  * @brief  CAN LL Init
  * @param  Instance Specifies CAN peripheral
  * @param  user_cfg user config pointer
  * @return Status of the Initialization
  */
LL_StatusETypeDef LL_CAN_Init(CAN_TypeDef *Instance, CAN_UserCfgTypeDef *user_cfg)
{
    CAN_HandleTypeDef *can_hdl;

    //Assert param
    assert_param(IS_CAN_ALL_INSTANCE(Instance));
    assert_param(user_cfg != NULL);
    assert_param(user_cfg->baudrate_ss);

    if (!IS_CAN_ALL_INSTANCE(Instance) || user_cfg == NULL || !user_cfg->baudrate_ss) {
        return LL_INVALID;
    }

    //CAN handle get
    can_hdl = CAN_Handle_Get(Instance);

    if (can_hdl == NULL) {
        LOG_E("Get CAN handle error!\n");
        return LL_ERROR;
    } else if (can_hdl->tx_ctrl_ptb.state != CAN_STATE_RESET || can_hdl->tx_ctrl_stb.state != CAN_STATE_RESET ||
               can_hdl->rx_ctrl.state != CAN_STATE_RESET) {
        LOG_E("This CAN[0x%08x] is being used!\n", (uint32_t)Instance);
        return LL_BUSY;
    }

    can_hdl->tx_ctrl_ptb.state = CAN_STATE_BUSY;
    can_hdl->tx_ctrl_stb.state = CAN_STATE_BUSY;
    can_hdl->rx_ctrl.state = CAN_STATE_BUSY;

    /* Init the low level hardware eg. Clock, NVIC */
    LL_CAN_MspInit(Instance);

    /* Set reset status to config some register which can config in reset status only */
    __LL_CAN_Reset_Set(Instance);

    //SS Baudrate and Bit timing config
    __LL_CAN_SS_Prescaler_Set(Instance,
                              (user_cfg->func_clk_freq / (user_cfg->bit_timing_seg1_ss + user_cfg->bit_timing_seg2_ss + 3) / user_cfg->baudrate_ss - 1));
    __LL_CAN_SS_BitTimingSeg1_Set(Instance, user_cfg->bit_timing_seg1_ss);
    __LL_CAN_SS_BitTimingSeg2_Set(Instance, user_cfg->bit_timing_seg2_ss);
    __LL_CAN_SS_SyncJumpWidth_Set(Instance, user_cfg->bit_timing_sjw_ss);

    //FS(FD) Baudrate and Bit timing config
    if (user_cfg->fd_en) {
        __LL_CAN_FD_En(Instance);
        LL_FUNC_ALTER(user_cfg->fd_iso_en, __LL_CAN_FD_ISO_En(Instance), __LL_CAN_FD_ISO_Dis(Instance));

        assert_param(user_cfg->baudrate_fs);

        if (!user_cfg->baudrate_fs) {
            return LL_INVALID;
        }

        __LL_CAN_FS_Prescaler_Set(Instance,
                                  (user_cfg->func_clk_freq / (user_cfg->bit_timing_seg1_fs + user_cfg->bit_timing_seg2_fs + 3) / user_cfg->baudrate_fs - 1));
        __LL_CAN_FS_BitTimingSeg1_Set(Instance, user_cfg->bit_timing_seg1_fs);
        __LL_CAN_FS_BitTimingSeg2_Set(Instance, user_cfg->bit_timing_seg2_fs);
        __LL_CAN_FS_SyncJumpWidth_Set(Instance, user_cfg->bit_timing_sjw_fs);
    } else {
        __LL_CAN_FD_Dis(Instance);
    }

    //Acceptance filter Reset
    for (uint8_t i = 0; i < CAN_ACCEPT_FILT_SLOT_NUMS; i++) {
        CAN_AcceptFilRst(Instance, (CAN_AcceptFilSlotETypeDef)i);
    }

    //Acceptance filter config
    for (uint8_t i = 0; i < user_cfg->accept_fil_cfg_num; i++) {
        CAN_AcceptFilCfg(Instance, (CAN_AcceptFilCfgTypeDef *)&user_cfg->accept_fil_cfg_ptr[i]);
    }

    /* Clear reset status to config other register */
    __LL_CAN_Reset_Clr(Instance);

    //RX buffer almost full and error warning limit set
    __LL_CAN_RxBufAlmostFullLimit_Set(Instance, user_cfg->rx_almost_full_limit);
    __LL_CAN_ErrWarnLimit_Set(Instance, user_cfg->err_limit);

    //All Interrupt Pending Clear
    __LL_CAN_AllIntPnd_Clr(Instance);

    can_hdl->tx_ctrl_ptb.state = CAN_STATE_READY;
    can_hdl->tx_ctrl_stb.state = CAN_STATE_READY;
    can_hdl->rx_ctrl.state = CAN_STATE_READY;
    can_hdl->user_callback = user_cfg->user_callback;

    return LL_OK;
}

/**
  * @brief  CAN LL DeInit
  * @param  Instance Specifies CAN peripheral
  * @return Status of the DeInitialization
  */
LL_StatusETypeDef LL_CAN_DeInit(CAN_TypeDef *Instance)
{
    CAN_HandleTypeDef *can_hdl;

    //Assert param
    assert_param(IS_CAN_ALL_INSTANCE(Instance));

    if (!IS_CAN_ALL_INSTANCE(Instance)) {
        return LL_INVALID;
    }

    //CAN handle get
    can_hdl = CAN_Handle_Get(Instance);

    if (can_hdl == NULL) {
        LOG_E("Get CAN handle error!\n");
        return LL_ERROR;
    } else if (can_hdl->tx_ctrl_ptb.state == CAN_STATE_BUSY_TX || can_hdl->tx_ctrl_stb.state == CAN_STATE_BUSY_TX ||
               can_hdl->rx_ctrl.state == CAN_STATE_BUSY_RX) {
        LOG_E("This CAN[0x%08x] is being used!\n", (uint32_t)Instance);
        return LL_BUSY;
    }

    can_hdl->tx_ctrl_ptb.state = CAN_STATE_BUSY;
    can_hdl->tx_ctrl_stb.state = CAN_STATE_BUSY;
    can_hdl->rx_ctrl.state = CAN_STATE_BUSY;

    /* DeInit the low level hardware eg. Clock, NVIC */
    LL_CAN_MspDeInit(Instance);

    memset((void *)can_hdl, 0, sizeof(CAN_HandleTypeDef));
    can_hdl->tx_ctrl_ptb.state = CAN_STATE_RESET;
    can_hdl->tx_ctrl_stb.state = CAN_STATE_RESET;
    can_hdl->rx_ctrl.state = CAN_STATE_RESET;

    return LL_OK;
}

/**
  * @brief  Initializes the CAN MSP.
  * @param  Instance Specifies CAN peripheral
  * @return None
  */
__WEAK void LL_CAN_MspInit(CAN_TypeDef *Instance)
{
    /* Prevent unused argument(s) compilation warning */
    LL_UNUSED(Instance);

    /* NOTE: This function should not be modified, when the callback is needed,
             the LL_CAN_MspInit could be implemented in the user file
     */
}

/**
  * @brief  DeInitializes the CAN MSP
  * @param  Instance Specifies CAN peripheral
  * @return None
  */
__WEAK void LL_CAN_MspDeInit(CAN_TypeDef *Instance)
{
    /* Prevent unused argument(s) compilation warning */
    LL_UNUSED(Instance);

    /* NOTE: This function should not be modified, when the callback is needed,
             the LL_CAN_MspDeInit could be implemented in the user file
     */
}

/**
  * @brief  Register an User CAN Callback
  * @note   User can register callback only in CAN Ready State
  * @param  Instance Specifies CAN peripheral
  * @param  CallbackID ID of the callback to be registered
  * @param  pCallback pointer to the Callback function
  * @return LL Status
  */
LL_StatusETypeDef LL_CAN_RegisterCallback(CAN_TypeDef *Instance, CAN_UserCallbackIdETypeDef CallbackID, CAN_UserCallback pCallback)
{
    CAN_HandleTypeDef *can_hdl;

    //Assert param
    assert_param(IS_CAN_ALL_INSTANCE(Instance));

    if (!IS_CAN_ALL_INSTANCE(Instance)) {
        return LL_INVALID;
    }

    //CAN handle get
    can_hdl = CAN_Handle_Get(Instance);

    if (can_hdl == NULL) {
        LOG_E("Get CAN handle error!\n");
        return LL_ERROR;
    }

    //Check callback pointer valid or not
    if (pCallback == NULL) {
        LOG_E("The callback pointer which to be registered is NULL!\n");
        return LL_INVALID;
    }

    //Register user callback
    switch (CallbackID) {
        case CAN_PTB_TX_CPLT_CB_ID:
            if (can_hdl->tx_ctrl_ptb.state != CAN_STATE_READY) {
                LOG_E("This CAN[0x%08x] PTB Tx isn't in Ready state, can't register Tx Complete callback!\n", (uint32_t)Instance);
                return LL_FAILED;
            }

            can_hdl->user_callback.TxCpltCallback_ptb = pCallback;
            break;

        case CAN_STB_TX_CPLT_CB_ID:
            if (can_hdl->tx_ctrl_stb.state != CAN_STATE_READY) {
                LOG_E("This CAN[0x%08x] STB Tx isn't in Ready state, can't register Tx Half Complete callback!\n", (uint32_t)Instance);
                return LL_FAILED;
            }

            can_hdl->user_callback.TxCpltCallback_stb = pCallback;
            break;

        case CAN_RX_CPLT_CB_ID:
            if (can_hdl->rx_ctrl.state != CAN_STATE_READY) {
                LOG_E("This CAN[0x%08x] Rx isn't in Ready state, can't register Rx Complete callback!\n", (uint32_t)Instance);
                return LL_FAILED;
            }

            can_hdl->user_callback.RxCpltCallback = pCallback;
            break;

        default:
            LOG_E("CAN user callback ID-[%d] is Invalid!\n", CallbackID);
            return LL_INVALID;
    }

    return LL_OK;
}

/**
  * @brief  UnRegister an User CAN Callback
  * @note   User can unregister callback only in CAN Ready State
  * @param  Instance Specifies CAN peripheral
  * @param  CallbackID ID of the callback to be unregistered
  * @return LL Status
  */
LL_StatusETypeDef LL_CAN_UnRegisterCallback(CAN_TypeDef *Instance, CAN_UserCallbackIdETypeDef CallbackID)
{
    CAN_HandleTypeDef *can_hdl;

    //Assert param
    assert_param(IS_CAN_ALL_INSTANCE(Instance));

    if (!IS_CAN_ALL_INSTANCE(Instance)) {
        return LL_INVALID;
    }

    //CAN handle get
    can_hdl = CAN_Handle_Get(Instance);

    if (can_hdl == NULL) {
        LOG_E("Get CAN handle error!\n");
        return LL_ERROR;
    }

    //UnRegister user callback
    switch (CallbackID) {
        case CAN_PTB_TX_CPLT_CB_ID:
            if (can_hdl->tx_ctrl_ptb.state != CAN_STATE_READY) {
                LOG_E("This CAN[0x%08x] PTB Tx isn't in Ready state, can't unregister Tx Complete callback!\n", (uint32_t)Instance);
                return LL_FAILED;
            }

            can_hdl->user_callback.TxCpltCallback_ptb = NULL;
            break;

        case CAN_STB_TX_CPLT_CB_ID:
            if (can_hdl->tx_ctrl_stb.state != CAN_STATE_READY) {
                LOG_E("This CAN[0x%08x] STB Tx isn't in Ready state, can't unregister Tx Half Complete callback!\n", (uint32_t)Instance);
                return LL_FAILED;
            }

            can_hdl->user_callback.TxCpltCallback_stb = NULL;
            break;

        case CAN_RX_CPLT_CB_ID:
            if (can_hdl->rx_ctrl.state != CAN_STATE_READY) {
                LOG_E("This CAN[0x%08x] Rx isn't in Ready state, can't unregister Rx Complete callback!\n", (uint32_t)Instance);
                return LL_FAILED;
            }

            can_hdl->user_callback.RxCpltCallback = NULL;
            break;

        default:
            LOG_E("CAN user callback ID-[%d] is Invalid!\n", CallbackID);
            return LL_INVALID;
    }

    return LL_OK;
}

/**
  * @}
  */


/** @defgroup CAN_LL_Exported_Functions_Group2 CAN LL Exported Functions Misc
  * @brief    CAN LL Exported Functions Misc
  * @{
  */

/**
  * @brief  CAN LL reset enter
  * @param  Instance Specifies CAN peripheral
  * @return LL Status
  */
LL_StatusETypeDef LL_CAN_ResetEnter(CAN_TypeDef *Instance)
{
    //Assert param
    assert_param(IS_CAN_ALL_INSTANCE(Instance));

    if (!IS_CAN_ALL_INSTANCE(Instance)) {
        return LL_INVALID;
    }

    //CAN reset set
    __LL_CAN_Reset_Set(Instance);

    return LL_OK;
}

/**
  * @brief  CAN LL reset exit
  * @param  Instance Specifies CAN peripheral
  * @return LL Status
  */
LL_StatusETypeDef LL_CAN_ResetExit(CAN_TypeDef *Instance)
{
    CAN_HandleTypeDef *can_hdl;

    //Assert param
    assert_param(IS_CAN_ALL_INSTANCE(Instance));

    if (!IS_CAN_ALL_INSTANCE(Instance)) {
        return LL_INVALID;
    }

    //CAN handle get
    can_hdl = CAN_Handle_Get(Instance);

    if (can_hdl == NULL) {
        LOG_E("Get CAN handle error!\n");
        return LL_ERROR;
    }

    //CAN reset clear
    __LL_CAN_Reset_Clr(Instance);

    memset((void *)can_hdl, 0, sizeof(CAN_HandleTypeDef));
    can_hdl->tx_ctrl_ptb.state = CAN_STATE_READY;
    can_hdl->tx_ctrl_stb.state = CAN_STATE_READY;
    can_hdl->rx_ctrl.state = CAN_STATE_READY;

    return LL_OK;
}

/**
  * @brief  CAN LL acceptance filter config
  * @param  Instance Specifies CAN peripheral
  * @param  fil_cfg filter config pointer
  * @return LL Status
  */
LL_StatusETypeDef LL_CAN_AcceptFilCfg(CAN_TypeDef *Instance, CAN_AcceptFilCfgTypeDef *fil_cfg)
{
    uint8_t rst_sta;
    LL_StatusETypeDef ret;
    CAN_HandleTypeDef *can_hdl;

    //Assert param
    assert_param(IS_CAN_ALL_INSTANCE(Instance));
    assert_param(fil_cfg != NULL);

    if (!IS_CAN_ALL_INSTANCE(Instance) || fil_cfg == NULL) {
        return LL_INVALID;
    }

    //CAN handle get
    can_hdl = CAN_Handle_Get(Instance);

    if (can_hdl == NULL) {
        LOG_E("Get CAN handle error!\n");
        return LL_ERROR;
    }

    //CAN reset status save
    rst_sta = __LL_CAN_ResetSta_Get(Instance);

    //CAN reset set
    __LL_CAN_Reset_Set(Instance);

    //CAN accept filter config
    ret = CAN_AcceptFilCfg(Instance, fil_cfg);

    //CAN reset status restroe
    LL_FUNC_ALTER(rst_sta, __LL_CAN_Reset_Set(Instance), __LL_CAN_Reset_Clr(Instance));

    memset((void *)can_hdl, 0, sizeof(CAN_HandleTypeDef));
    can_hdl->tx_ctrl_ptb.state = CAN_STATE_READY;
    can_hdl->tx_ctrl_stb.state = CAN_STATE_READY;
    can_hdl->rx_ctrl.state = CAN_STATE_READY;

    return ret;
}

/**
  * @brief  CAN Data Length Get
  * @param  Instance Specifies CAN peripheral
  * @param  dat_len_code Data length code
  * @return Data length in bytes unit
  */
uint8_t LL_CAN_DatLen_Get(CAN_TypeDef *Instance, uint8_t dat_len_code)
{
    //Check params to be valid
    if (!IS_CAN_ALL_INSTANCE(Instance) || dat_len_code >= 16) {
        return 0;
    }

    uint8_t dat_len = ((uint8_t []) {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64
    })[dat_len_code];

    return __LL_CAN_IsFDEn(Instance) ? dat_len : LL_MIN(dat_len, CAN_20_DAT_LEN_MAX);
}

/**
  * @brief  CAN Data Length Code Get
  * @param  Instance Specifies CAN peripheral
  * @param  dat_len Data length that can only config in [0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64]
  * @return Data length code @ref CAN_DatLenCodeETypeDef
  */
CAN_DatLenCodeETypeDef LL_CAN_DatLenCode_Get(CAN_TypeDef *Instance, uint8_t dat_len)
{
    //Check params to be valid
    if (!IS_CAN_ALL_INSTANCE(Instance) || (__LL_CAN_IsFDEn(Instance) && dat_len > 64) ||
        (!__LL_CAN_IsFDEn(Instance) && dat_len > CAN_20_DAT_LEN_MAX)) {
        return CAN_DAT_LEN_CODE_BYTE_0;
    }

    const uint8_t dat_len_array[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64
    };

    for (uint8_t i = 0; i < sizeof dat_len_array; i++) {
        if (dat_len_array[i] == dat_len) {
            return (CAN_DatLenCodeETypeDef)i;
        }
    }

    return CAN_DAT_LEN_CODE_BYTE_0;
}

/**
  * @}
  */


/** @defgroup CAN_LL_Exported_Functions_Group3 CAN Operation Functions
  * @brief    CAN Operation Functions
  * @{
  */

/**
  * @brief  PTB transmit one frame in CPU blocking mode
  * @param  Instance Specifies CAN peripheral
  * @param  buf_fmt Tx buffer format pointer
  * @param  buf     Tx buffer pointer
  * @return LL Status
  */
LL_StatusETypeDef LL_CAN_TransmitPTB_CPU(CAN_TypeDef *Instance, CAN_TxBufFormatTypeDef *buf_fmt, uint32_t *buf)
{
    uint8_t dat_len;
    CAN_HandleTypeDef *can_hdl;

    //Assert param
    assert_param(IS_CAN_ALL_INSTANCE(Instance));
    assert_param(buf_fmt != NULL);

    if (!IS_CAN_ALL_INSTANCE(Instance) || buf_fmt == NULL) {
        return LL_INVALID;
    }

    //CAN handle get
    can_hdl = CAN_Handle_Get(Instance);

    if (can_hdl == NULL) {
        LOG_E("Get CAN handle error!\n");
        return LL_ERROR;
    } else if (can_hdl->tx_ctrl_ptb.state != CAN_STATE_READY) {
        LOG_E("This CAN[0x%08x] PTB Tx isn't in READY state, can't start Tx!\n", (uint32_t)Instance);
        return LL_FAILED;
    }

    //Check last frame transmit complete or not
    if (__LL_CAN_TxPriEn_Get(Instance)) {
        return LL_ERROR;
    }

    can_hdl->tx_ctrl_ptb.state = CAN_STATE_BUSY_TX;
    can_hdl->tx_ctrl_ptb.isr = NULL;

    //TX buffer select PTB
    __LL_CAN_TxBufSel_PTB(Instance);

    //Write buffer format data
    __LL_CAN_TxBufReg_ID_Write(Instance, *((uint32_t *)buf_fmt));
    __LL_CAN_TxBufReg_Ctrl_Write(Instance, *(((uint32_t *)buf_fmt) + 1));

    //Write data to TX buffer
    if (buf != NULL) {
        dat_len = LL_CAN_DatLen_Get(Instance, buf_fmt->data_len_code);

        for (uint32_t i = 0; i < (dat_len + 3) / 4; i++) {
            __LL_CAN_TxBufReg_Data_Write(Instance, i, *buf++);
        }
    }

    //TX primary enable
    __LL_CAN_TxPriEn_Set(Instance);

    can_hdl->tx_ctrl_ptb.state = CAN_STATE_READY;

    return LL_OK;
}

/**
  * @brief  STB transmit one frame in CPU blocking mode
  * @param  Instance Specifies CAN peripheral
  * @param  buf_fmt Tx buffer format pointer
  * @param  buf     Tx buffer pointer
  * @return LL Status
  */
LL_StatusETypeDef LL_CAN_TransmitSTB_CPU(CAN_TypeDef *Instance, CAN_TxBufFormatTypeDef *buf_fmt, uint32_t *buf)
{
    uint8_t dat_len;
    CAN_HandleTypeDef *can_hdl;

    //Assert param
    assert_param(IS_CAN_ALL_INSTANCE(Instance));
    assert_param(buf_fmt != NULL);

    if (!IS_CAN_ALL_INSTANCE(Instance) || buf_fmt == NULL) {
        return LL_INVALID;
    }

    //CAN handle get
    can_hdl = CAN_Handle_Get(Instance);

    if (can_hdl == NULL) {
        LOG_E("Get CAN handle error!\n");
        return LL_ERROR;
    } else if (can_hdl->tx_ctrl_stb.state != CAN_STATE_READY) {
        LOG_E("This CAN[0x%08x] STB Tx isn't in READY state, can't start Tx!\n", (uint32_t)Instance);
        return LL_FAILED;
    }

    //Check last frame transmit complete or not
    if (__LL_CAN_TxSecOne_Get(Instance)) {
        return LL_ERROR;
    }

    can_hdl->tx_ctrl_stb.state = CAN_STATE_BUSY_TX;
    can_hdl->tx_ctrl_stb.isr = NULL;

    //TX buffer select STB
    __LL_CAN_TxBufSel_STB(Instance);

    //Write buffer format data
    __LL_CAN_TxBufReg_ID_Write(Instance, *((uint32_t *)buf_fmt));
    __LL_CAN_TxBufReg_Ctrl_Write(Instance, *(((uint32_t *)buf_fmt) + 1));

    //Write data to TX buffer
    if (buf != NULL) {
        dat_len = LL_CAN_DatLen_Get(Instance, buf_fmt->data_len_code);

        for (uint32_t i = 0; i < (dat_len + 3) / 4; i++) {
            __LL_CAN_TxBufReg_Data_Write(Instance, i, *buf++);
        }
    }

    //TX buffer Secondary next
    __LL_CAN_TxSecNext_Set(Instance);

    //TX secondary send one start
    __LL_CAN_TxSecOne_Set(Instance);

    can_hdl->tx_ctrl_stb.state = CAN_STATE_READY;

    return LL_OK;
}

/**
  * @brief  Receive one frame in CPU blocking mode
  * @param  Instance Specifies CAN peripheral
  * @param  buf_fmt Rx buffer format pointer
  * @param  buf     Rx buffer pointer
  * @return LL Status
  */
LL_StatusETypeDef LL_CAN_Receive_CPU(CAN_TypeDef *Instance, CAN_RxBufFormatTypeDef *buf_fmt, uint32_t *buf)
{
    CAN_HandleTypeDef *can_hdl;

    //Assert param
    assert_param(IS_CAN_ALL_INSTANCE(Instance));

    if (!IS_CAN_ALL_INSTANCE(Instance)) {
        return LL_INVALID;
    }

    //CAN handle get
    can_hdl = CAN_Handle_Get(Instance);

    if (can_hdl == NULL) {
        LOG_E("Get CAN handle error!\n");
        return LL_ERROR;
    } else if (can_hdl->rx_ctrl.state != CAN_STATE_READY) {
        LOG_E("This CAN[0x%08x] Rx isn't in READY state, can't start Rx!\n", (uint32_t)Instance);
        return LL_FAILED;
    }

    //Check Rx frame complete or not
    if (!__LL_CAN_IsRxIntPnd(Instance)) {
        return LL_ERROR;
    }

    can_hdl->rx_ctrl.state = CAN_STATE_BUSY_RX;
    can_hdl->rx_ctrl.isr = NULL;
    can_hdl->rx_ctrl.buf = buf;
    can_hdl->rx_ctrl.buf_fmt = buf_fmt;

    //Clear Receive Interrupt Pending
    __LL_CAN_RxIntPnd_Clr(Instance);

    //Read data from RxFIFO
    if (buf_fmt != NULL) {
        *((uint32_t *)buf_fmt + 0) = __LL_CAN_RxBufReg_ID_Read(Instance);
        *((uint32_t *)buf_fmt + 1) = __LL_CAN_RxBufReg_Ctrl_Read(Instance);

        if (buf != NULL) {
            memcpy((void *)buf, (void *)Instance->RBUFDT, LL_CAN_DatLen_Get(Instance, buf_fmt->data_len_code));
        }
    }

    //RxFIFO Release(Clear)
    __LL_CAN_RxBufRelease(Instance);

    can_hdl->rx_ctrl.state = CAN_STATE_READY;

    return LL_OK;
}

/**
  * @brief  PTB transmit one frame in non-blocking mode with Interrupt
  * @param  Instance Specifies CAN peripheral
  * @param  buf_fmt Tx buffer format pointer
  * @param  buf     Tx buffer pointer
  * @return LL Status
  */
LL_StatusETypeDef LL_CAN_TransmitPTB_IT(CAN_TypeDef *Instance, CAN_TxBufFormatTypeDef *buf_fmt, uint32_t *buf)
{
    uint8_t dat_len;
    CAN_HandleTypeDef *can_hdl;

    //Assert param
    assert_param(IS_CAN_ALL_INSTANCE(Instance));
    assert_param(buf_fmt != NULL);

    if (!IS_CAN_ALL_INSTANCE(Instance) || buf_fmt == NULL) {
        return LL_INVALID;
    }

    //CAN handle get
    can_hdl = CAN_Handle_Get(Instance);

    if (can_hdl == NULL) {
        LOG_E("Get CAN handle error!\n");
        return LL_ERROR;
    } else if (can_hdl->tx_ctrl_ptb.state != CAN_STATE_READY) {
        LOG_E("This CAN[0x%08x] PTB Tx isn't in READY state, can't start Tx!\n", (uint32_t)Instance);
        return LL_FAILED;
    }

    //Check last frame transmit complete or not
    if (__LL_CAN_TxPriEn_Get(Instance)) {
        return LL_ERROR;
    }

    can_hdl->tx_ctrl_ptb.state = CAN_STATE_BUSY_TX;
    can_hdl->tx_ctrl_ptb.isr = CAN_TxISR_PTB;

    //Clear Transmission Primary Interrupt Pending
    __LL_CAN_TxPriIntPnd_Clr(Instance);

    //TX buffer select PTB
    __LL_CAN_TxBufSel_PTB(Instance);

    //Write buffer format data
    __LL_CAN_TxBufReg_ID_Write(Instance, *((uint32_t *)buf_fmt));
    __LL_CAN_TxBufReg_Ctrl_Write(Instance, *(((uint32_t *)buf_fmt) + 1));

    //Write data to TX buffer
    if (buf != NULL) {
        dat_len = LL_CAN_DatLen_Get(Instance, buf_fmt->data_len_code);

        for (uint32_t i = 0; i < (dat_len + 3) / 4; i++) {
            __LL_CAN_TxBufReg_Data_Write(Instance, i, *buf++);
        }
    }

    //TX primary enable
    __LL_CAN_TxPriEn_Set(Instance);

    //Transmission Primary Interrupt Enable
    __LL_CAN_TxPri_INT_En(Instance);

    return LL_OK;
}

/**
  * @brief  STB transmit one frame in non-blocking mode with Interrupt
  * @param  Instance Specifies CAN peripheral
  * @param  buf_fmt Tx buffer format pointer
  * @param  buf     Tx buffer pointer
  * @return LL Status
  */
LL_StatusETypeDef LL_CAN_TransmitSTB_IT(CAN_TypeDef *Instance, CAN_TxBufFormatTypeDef *buf_fmt, uint32_t *buf)
{
    uint8_t dat_len;
    CAN_HandleTypeDef *can_hdl;

    //Assert param
    assert_param(IS_CAN_ALL_INSTANCE(Instance));
    assert_param(buf_fmt != NULL);

    if (!IS_CAN_ALL_INSTANCE(Instance) || buf_fmt == NULL) {
        return LL_INVALID;
    }

    //CAN handle get
    can_hdl = CAN_Handle_Get(Instance);

    if (can_hdl == NULL) {
        LOG_E("Get CAN handle error!\n");
        return LL_ERROR;
    } else if (can_hdl->tx_ctrl_stb.state != CAN_STATE_READY) {
        LOG_E("This CAN[0x%08x] STB Tx isn't in READY state, can't start Tx!\n", (uint32_t)Instance);
        return LL_FAILED;
    }

    //Check last frame transmit complete or not
    if (__LL_CAN_TxSecOne_Get(Instance)) {
        return LL_ERROR;
    }

    can_hdl->tx_ctrl_stb.state = CAN_STATE_BUSY_TX;
    can_hdl->tx_ctrl_stb.isr = CAN_TxISR_STB;

    //Clear Secondary Primary Interrupt Pending
    __LL_CAN_TxSecIntPnd_Clr(Instance);

    //TX buffer select STB
    __LL_CAN_TxBufSel_STB(Instance);

    //Write buffer format data
    __LL_CAN_TxBufReg_ID_Write(Instance, *((uint32_t *)buf_fmt));
    __LL_CAN_TxBufReg_Ctrl_Write(Instance, *(((uint32_t *)buf_fmt) + 1));

    //Write data to TX buffer
    if (buf != NULL) {
        dat_len = LL_CAN_DatLen_Get(Instance, buf_fmt->data_len_code);

        for (uint32_t i = 0; i < (dat_len + 3) / 4; i++) {
            __LL_CAN_TxBufReg_Data_Write(Instance, i, *buf++);
        }
    }

    //TX buffer Secondary next
    __LL_CAN_TxSecNext_Set(Instance);

    //TX secondary send one start
    __LL_CAN_TxSecOne_Set(Instance);

    //Transmission Secondary Interrupt Enable
    __LL_CAN_TxSec_INT_En(Instance);

    return LL_OK;
}

/**
  * @brief  Receive one frame in non-blocking mode with Interrupt
  * @param  Instance Specifies CAN peripheral
  * @param  buf_fmt Rx buffer format pointer
  * @param  buf     Rx buffer pointer
  * @return LL Status
  */
LL_StatusETypeDef LL_CAN_Receive_IT(CAN_TypeDef *Instance, CAN_RxBufFormatTypeDef *buf_fmt, uint32_t *buf)
{
    CAN_HandleTypeDef *can_hdl;

    //Assert param
    assert_param(IS_CAN_ALL_INSTANCE(Instance));

    if (!IS_CAN_ALL_INSTANCE(Instance)) {
        return LL_INVALID;
    }

    //CAN handle get
    can_hdl = CAN_Handle_Get(Instance);

    if (can_hdl == NULL) {
        LOG_E("Get CAN handle error!\n");
        return LL_ERROR;
    } else if (can_hdl->rx_ctrl.state != CAN_STATE_READY) {
        LOG_E("This CAN[0x%08x] Rx isn't in READY state, can't start Rx!\n", (uint32_t)Instance);
        return LL_FAILED;
    }

    can_hdl->rx_ctrl.state = CAN_STATE_BUSY_RX;
    can_hdl->rx_ctrl.isr = CAN_RxISR;
    can_hdl->rx_ctrl.buf = buf;
    can_hdl->rx_ctrl.buf_fmt = buf_fmt;

    //Receive Interrupt Enable
    __LL_CAN_Rx_INT_En(Instance);

    return LL_OK;
}

/**
  * @}
  */


/** @defgroup CAN_LL_Exported_Functions_Interrupt CAN Interrupt Handler and Callback
  * @brief    CAN Interrupt Handler and Callback
  * @{
  */

/**
  * @brief  CAN IRQ Handler
  * @param  Instance Specifies CAN peripheral
  * @note   All interrupt pending will be reset immediately after a read access
  * @return None
  */
void LL_CAN_IRQHandler(CAN_TypeDef *Instance)
{
    CAN_HandleTypeDef *can_hdl;
    uint32_t int_en, int_pending;

    //Assert param
    assert_param(IS_CAN_ALL_INSTANCE(Instance));

    if (!IS_CAN_ALL_INSTANCE(Instance)) {
        return;
    }

    //CAN handle get
    can_hdl = CAN_Handle_Get(Instance);

    if (can_hdl == NULL) {
        LOG_E("Get CAN handle error!\n");
        return;
    }

    //All Interrupt Enalbe and Pending Get
    int_en = __LL_CAN_AllIntEn_Get(Instance);
    int_pending = __LL_CAN_AllIntPnd_Get(Instance);


    //Rx Interrupt Handler
    if ((int_en & CAN0_INTREN_RIE_Msk) && (int_pending & CAN0_INTRST_RIF_Msk)) {
        //Clear Interrupt Pending
        __LL_CAN_RxIntPnd_Clr(Instance);

        if (can_hdl->rx_ctrl.isr) {
            can_hdl->rx_ctrl.isr(Instance);
        }

        //Callback
        LL_CAN_RxCallback(Instance);
    }

    //Rx Overrun Interrupt Handler
    if ((int_en & CAN0_INTREN_ROIE_Msk) && (int_pending & CAN0_INTRST_ROIF_Msk)) {
        //Clear Interrupt Pending
        __LL_CAN_RxOverIntPnd_Clr(Instance);

        //Callback
        LL_CAN_RxOverCallback(Instance);
    }

    //Rx Full Interrupt Handler
    if ((int_en & CAN0_INTREN_RFIE_Msk) && (int_pending & CAN0_INTRST_RFIF_Msk)) {
        //Clear Interrupt Pending
        __LL_CAN_RxFullIntPnd_Clr(Instance);

        //Callback
        LL_CAN_RxFullCallback(Instance);
    }

    //Rx Almost Full Interrupt Handler
    if ((int_en & CAN0_INTREN_RAFIE_Msk) && (int_pending & CAN0_INTRST_RAFIF_Msk)) {
        //Clear Interrupt Pending
        __LL_CAN_RxAlmostFullIntPnd_Clr(Instance);

        //Callback
        LL_CAN_RxAlmostFullCallback(Instance);
    }


    //Transmission Primary Interrupt Handler
    if ((int_en & CAN0_INTREN_TPIE_Msk) && (int_pending & CAN0_INTRST_TPIF_Msk)) {
        //Clear Interrupt Pending
        __LL_CAN_TxPriIntPnd_Clr(Instance);

        if (can_hdl->tx_ctrl_ptb.isr) {
            can_hdl->tx_ctrl_ptb.isr(Instance);
        }

        //Callback
        LL_CAN_TxPriCallback(Instance);
    }

    //Transmission Secondary Interrupt Handler
    if ((int_en & CAN0_INTREN_TSIE_Msk) && (int_pending & CAN0_INTRST_TSIF_Msk)) {
        //Clear Interrupt Pending
        __LL_CAN_TxSecIntPnd_Clr(Instance);

        if (can_hdl->tx_ctrl_stb.isr) {
            can_hdl->tx_ctrl_stb.isr(Instance);
        }

        //Callback
        LL_CAN_TxSecCallback(Instance);
    }


    //Error Interrupt Handler
    if ((int_en & CAN0_INTREN_EIE_Msk) && (int_pending & CAN0_INTRST_EIF_Msk)) {
        //Clear Interrupt Pending
        __LL_CAN_ErrIntPnd_Clr(Instance);

        //Callback
        LL_CAN_ErrCallback(Instance);
    }

    //Abort Interrupt Handler
    if (int_pending & CAN0_INTRST_AIF_Msk) {
        //Clear Interrupt Pending
        __LL_CAN_AbortIntPnd_Clr(Instance);

        //Callback
        LL_CAN_AbortCallback(Instance);
    }


    //Error Passive Interrupt Handler
    if ((int_en & CAN0_INTREN_EPIE_Msk) && (int_pending & CAN0_INTRST_EPIF_Msk)) {
        //Clear Interrupt Pending
        __LL_CAN_ErrPassiveIntPnd_Clr(Instance);

        //Callback
        LL_CAN_ErrPassiveCallback(Instance);
    }

    //Arbitration Lost Interrupt Handler
    if ((int_en & CAN0_INTREN_ALIE_Msk) && (int_pending & CAN0_INTRST_ALIF_Msk)) {
        //Clear Interrupt Pending
        __LL_CAN_ArbLostIntPnd_Clr(Instance);

        //Callback
        LL_CAN_ArbLostCallback(Instance);
    }

    //Bus Error Interrupt Handler
    if ((int_en & CAN0_INTREN_BEIE_Msk) && (int_pending & CAN0_INTRST_BEIF_Msk)) {
        //Clear Interrupt Pending
        __LL_CAN_BusErrIntPnd_Clr(Instance);

        //Callback
        LL_CAN_BusErrCallback(Instance);
    }
}


/**
  * @brief  CAN RX Interrupt Callback
  * @param  Instance Specifies CAN peripheral
  * @return None
  */
__WEAK void LL_CAN_RxCallback(CAN_TypeDef *Instance)
{
    /* Prevent unused argument(s) compilation warning */
    LL_UNUSED(Instance);

    /* NOTE: This function should not be modified, when the callback is needed,
             the LL_CAN_RxCallback could be implemented in the user file
     */
}

/**
  * @brief  CAN RX Over Interrupt Callback
  * @param  Instance Specifies CAN peripheral
  * @return None
  */
__WEAK void LL_CAN_RxOverCallback(CAN_TypeDef *Instance)
{
    /* Prevent unused argument(s) compilation warning */
    LL_UNUSED(Instance);

    /* NOTE: This function should not be modified, when the callback is needed,
             the LL_CAN_RxOverCallback could be implemented in the user file
     */
}

/**
  * @brief  CAN RX Full Interrupt Callback
  * @param  Instance Specifies CAN peripheral
  * @return None
  */
__WEAK void LL_CAN_RxFullCallback(CAN_TypeDef *Instance)
{
    /* Prevent unused argument(s) compilation warning */
    LL_UNUSED(Instance);

    /* NOTE: This function should not be modified, when the callback is needed,
             the LL_CAN_RxFullCallback could be implemented in the user file
     */
}

/**
  * @brief  CAN RX Almost Full Interrupt Callback
  * @param  Instance Specifies CAN peripheral
  * @return None
  */
__WEAK void LL_CAN_RxAlmostFullCallback(CAN_TypeDef *Instance)
{
    /* Prevent unused argument(s) compilation warning */
    LL_UNUSED(Instance);

    /* NOTE: This function should not be modified, when the callback is needed,
             the LL_CAN_RxAlmostFullCallback could be implemented in the user file
     */
}

/**
  * @brief  CAN TX Primary Interrupt Callback
  * @param  Instance Specifies CAN peripheral
  * @return None
  */
__WEAK void LL_CAN_TxPriCallback(CAN_TypeDef *Instance)
{
    /* Prevent unused argument(s) compilation warning */
    LL_UNUSED(Instance);

    /* NOTE: This function should not be modified, when the callback is needed,
             the LL_CAN_TxPriCallback could be implemented in the user file
     */
}

/**
  * @brief  CAN TX Secondary Interrupt Callback
  * @param  Instance Specifies CAN peripheral
  * @return None
  */
__WEAK void LL_CAN_TxSecCallback(CAN_TypeDef *Instance)
{
    /* Prevent unused argument(s) compilation warning */
    LL_UNUSED(Instance);

    /* NOTE: This function should not be modified, when the callback is needed,
             the LL_CAN_TxSecCallback could be implemented in the user file
     */
}

/**
  * @brief  CAN Error Interrupt Callback
  * @param  Instance Specifies CAN peripheral
  * @return None
  */
__WEAK void LL_CAN_ErrCallback(CAN_TypeDef *Instance)
{
    /* Prevent unused argument(s) compilation warning */
    LL_UNUSED(Instance);

    /* NOTE: This function should not be modified, when the callback is needed,
             the LL_CAN_ErrCallback could be implemented in the user file
     */
}

/**
  * @brief  CAN Abort Interrupt Callback
  * @param  Instance Specifies CAN peripheral
  * @return None
  */
__WEAK void LL_CAN_AbortCallback(CAN_TypeDef *Instance)
{
    /* Prevent unused argument(s) compilation warning */
    LL_UNUSED(Instance);

    /* NOTE: This function should not be modified, when the callback is needed,
             the LL_CAN_AbortCallback could be implemented in the user file
     */
}

/**
  * @brief  CAN Error Passive Interrupt Callback
  * @param  Instance Specifies CAN peripheral
  * @return None
  */
__WEAK void LL_CAN_ErrPassiveCallback(CAN_TypeDef *Instance)
{
    /* Prevent unused argument(s) compilation warning */
    LL_UNUSED(Instance);

    /* NOTE: This function should not be modified, when the callback is needed,
             the LL_CAN_ErrPassiveCallback could be implemented in the user file
     */
}

/**
  * @brief  CAN Arbitration Lost Interrupt Callback
  * @param  Instance Specifies CAN peripheral
  * @return None
  */
__WEAK void LL_CAN_ArbLostCallback(CAN_TypeDef *Instance)
{
    /* Prevent unused argument(s) compilation warning */
    LL_UNUSED(Instance);

    /* NOTE: This function should not be modified, when the callback is needed,
             the LL_CAN_ArbLostCallback could be implemented in the user file
     */
}

/**
  * @brief  CAN Bus Error Interrupt Callback
  * @param  Instance Specifies CAN peripheral
  * @return None
  */
__WEAK void LL_CAN_BusErrCallback(CAN_TypeDef *Instance)
{
    /* Prevent unused argument(s) compilation warning */
    LL_UNUSED(Instance);

    /* NOTE: This function should not be modified, when the callback is needed,
             the LL_CAN_BusErrCallback could be implemented in the user file
     */
}

/**
  * @}
  */

/**
  * @}
  */


/* Private Functions ---------------------------------------------------------*/
/** @defgroup CAN_LL_Private_Functions CAN LL Private Functions
  * @brief    CAN LL Private Functions
  * @{
  */

/**
  * @brief  CAN Handle Get
  * @param  Instance Specifies CAN peripheral
  * @return CAN_HandleTypeDef pointer
  */
static CAN_HandleTypeDef *CAN_Handle_Get(CAN_TypeDef *Instance)
{
    //Assert param
    assert_param(IS_CAN_ALL_INSTANCE(Instance));

    if (Instance == CAN0) {
        return &can_hdl_global[CAN_INSTANCE_0];
    } else if (Instance == CAN1) {
        return &can_hdl_global[CAN_INSTANCE_1];
    }

    return NULL;
}

/**
  * @brief  CAN PTB transmit data handle in Interrupt mode
  * @param  Instance Specifies CAN peripheral
  * @return None
  */
static void CAN_TxISR_PTB(CAN_TypeDef *Instance)
{
    CAN_HandleTypeDef *can_hdl;

    //CAN handle get
    can_hdl = CAN_Handle_Get(Instance);

    if (can_hdl == NULL) {
        LOG_E("Get CAN handle error!\n");
        return;
    }

    //Transmission Primary Interrupt Disable
    __LL_CAN_TxPri_INT_Dis(Instance);

    can_hdl->tx_ctrl_ptb.isr = NULL;
    can_hdl->tx_ctrl_ptb.state = CAN_STATE_READY;

    if (can_hdl->user_callback.TxCpltCallback_ptb) {
        can_hdl->user_callback.TxCpltCallback_ptb();
    }
}

/**
  * @brief  CAN STB transmit data handle in Interrupt mode
  * @param  Instance Specifies CAN peripheral
  * @return None
  */
static void CAN_TxISR_STB(CAN_TypeDef *Instance)
{
    CAN_HandleTypeDef *can_hdl;

    //CAN handle get
    can_hdl = CAN_Handle_Get(Instance);

    if (can_hdl == NULL) {
        LOG_E("Get CAN handle error!\n");
        return;
    }

    //Transmission Secondary Interrupt Disable
    __LL_CAN_TxSec_INT_Dis(Instance);

    can_hdl->tx_ctrl_stb.isr = NULL;
    can_hdl->tx_ctrl_stb.state = CAN_STATE_READY;

    if (can_hdl->user_callback.TxCpltCallback_stb) {
        can_hdl->user_callback.TxCpltCallback_stb();
    }
}

/**
  * @brief  CAN receive data handle in Interrupt mode
  * @param  Instance Specifies CAN peripheral
  * @return None
  */
static void CAN_RxISR(CAN_TypeDef *Instance)
{
    CAN_HandleTypeDef *can_hdl;

    //CAN handle get
    can_hdl = CAN_Handle_Get(Instance);

    if (can_hdl == NULL) {
        LOG_E("Get CAN handle error!\n");
        return;
    }

    //Receive Interrupt Disable
    __LL_CAN_Rx_INT_Dis(Instance);

    //Read data from RxFIFO
    if (can_hdl->rx_ctrl.buf_fmt != NULL) {
        *((uint32_t *)can_hdl->rx_ctrl.buf_fmt + 0) = __LL_CAN_RxBufReg_ID_Read(Instance);
        *((uint32_t *)can_hdl->rx_ctrl.buf_fmt + 1) = __LL_CAN_RxBufReg_Ctrl_Read(Instance);

        if (can_hdl->rx_ctrl.buf != NULL) {
            memcpy((void *)can_hdl->rx_ctrl.buf, (void *)Instance->RBUFDT,
                   LL_CAN_DatLen_Get(Instance, can_hdl->rx_ctrl.buf_fmt->data_len_code));
        }
    }

    //RxFIFO Release(Clear)
    __LL_CAN_RxBufRelease(Instance);

    can_hdl->rx_ctrl.isr = NULL;
    can_hdl->rx_ctrl.state = CAN_STATE_READY;

    if (can_hdl->user_callback.RxCpltCallback) {
        can_hdl->user_callback.RxCpltCallback();
    }
}

/**
  * @brief  CAN acceptance filter reset
  * @param  Instance Specifies CAN peripheral
  * @param  slot Acceptance filter slot
  * @return Reset result
  */
static LL_StatusETypeDef CAN_AcceptFilRst(CAN_TypeDef *Instance, CAN_AcceptFilSlotETypeDef slot)
{
    //Assert param
    assert_param(IS_CAN_ALL_INSTANCE(Instance));
    assert_param(slot < CAN_ACCEPT_FILT_SLOT_NUMS);

    if (!IS_CAN_ALL_INSTANCE(Instance) || slot >= CAN_ACCEPT_FILT_SLOT_NUMS) {
        return LL_INVALID;
    }

    //Check in reset status or not
    if (!__LL_CAN_ResetSta_Get(Instance)) {
        LOG_E("Acceptance filter register can config in reset status only!\n");
        return LL_ERROR;
    }

    //CAN acceptance filter Code and Mask config
    __LL_CAN_AcceptFilAddr_Set(Instance, slot);
    __LL_CAN_AcceptFilContentSel_Mask(Instance);
    __LL_CAN_AcceptFilCodeOrMaskVal_Set(Instance, 0);
    __LL_CAN_AcceptFilRxFrm_Set(Instance, CAN_ACCEPT_FILT_FRM_STD_EXT);
    __LL_CAN_AcceptFilContentSel_Code(Instance);
    __LL_CAN_AcceptFilCodeOrMaskVal_Set(Instance, 0);

    //CAN acceptance filter disable
    __LL_CAN_AcceptFil_Dis(Instance, ((uint8_t)slot));

    return LL_OK;
}

/**
  * @brief  CAN acceptance filter config
  * @param  Instance Specifies CAN peripheral
  * @param  fil_cfg filter config pointer
  * @return Config result
  */
static LL_StatusETypeDef CAN_AcceptFilCfg(CAN_TypeDef *Instance, CAN_AcceptFilCfgTypeDef *fil_cfg)
{
    //Assert param
    assert_param(IS_CAN_ALL_INSTANCE(Instance));
    assert_param(fil_cfg != NULL);

    if (!IS_CAN_ALL_INSTANCE(Instance) || fil_cfg == NULL) {
        return LL_INVALID;
    }

    //Check in reset status or not
    if (!__LL_CAN_ResetSta_Get(Instance)) {
        LOG_E("Acceptance filter register can config in reset status only!\n");
        return LL_ERROR;
    }

    //CAN acceptance filter Code and Mask config
    __LL_CAN_AcceptFilAddr_Set(Instance, fil_cfg->slot);
    __LL_CAN_AcceptFilContentSel_Mask(Instance);
    __LL_CAN_AcceptFilCodeOrMaskVal_Set(Instance, fil_cfg->mask_val);
    __LL_CAN_AcceptFilRxFrm_Set(Instance, fil_cfg->rx_frm);
    __LL_CAN_AcceptFilContentSel_Code(Instance);
    __LL_CAN_AcceptFilCodeOrMaskVal_Set(Instance, fil_cfg->code_val);

    //CAN acceptance filter enable
    __LL_CAN_AcceptFil_En(Instance, ((uint8_t)fil_cfg->slot));

    return LL_OK;
}

/**
  * @}
  */


#endif  /* LL_CAN_MODULE_ENABLED */


/**
  * @}
  */

/**
  * @}
  */


/************************* (C) COPYRIGHT Tai-Action *****END OF FILE***********/

