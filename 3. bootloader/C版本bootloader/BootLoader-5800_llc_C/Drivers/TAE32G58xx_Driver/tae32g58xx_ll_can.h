/**
  ******************************************************************************
  * @file    tae32g58xx_ll_can.h
  * @author  MCD Application Team
  * @brief   Header file for CAN LL module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _TAE32G58XX_LL_CAN_H_
#define _TAE32G58XX_LL_CAN_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "tae32g58xx_ll_def.h"


/** @addtogroup TAE32G58xx_LL_Driver
  * @{
  */

/** @addtogroup CAN_LL
  * @{
  */


/* Exported Constants --------------------------------------------------------*/
/** @defgroup CAN_LL_Exported_Constants CAN LL Exported Constants
  * @brief    CAN LL Exported Constants
  * @{
  */

/**
  * @brief CAN 2.0 Max Data Length Definition
  */
#define CAN_20_DAT_LEN_MAX      (8)

/**
  * @}
  */


/* Exported Macros -----------------------------------------------------------*/
/** @defgroup CAN_LL_Exported_Macros CAN LL Exported Macros
  * @brief    CAN LL Exported Macros
  * @{
  */

/**
  * @brief  RX buffer almost full warning limit set
  * @param  __CAN__ Specifies CAN peripheral
  * @param  val set value
  * @return None
  */
#define __LL_CAN_RxBufAlmostFullLimit_Set(__CAN__, val) \
        MODIFY_REG((__CAN__)->CTRL, CAN0_CTRL_AFWL_Msk, (((val) & 0xfUL) << CAN0_CTRL_AFWL_Pos))

/**
  * @brief  Error warning limit set
  * @param  __CAN__ Specifies CAN peripheral
  * @param  val set value
  * @return None
  */
#define __LL_CAN_ErrWarnLimit_Set(__CAN__, val)         \
        MODIFY_REG((__CAN__)->CTRL, CAN0_CTRL_EWL_Msk, (((val) & 0xfUL) << CAN0_CTRL_EWL_Pos))

/**
  * @brief  RX buffer release
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_RxBufRelease(__CAN__)                  SET_BIT((__CAN__)->CTRL, CAN0_CTRL_RREL_Msk)

/**
  * @brief  CAN FD Enable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_FD_En(__CAN__)                         SET_BIT((__CAN__)->CTRL, CAN0_CTRL_FD_EN_Msk)

/**
  * @brief  CAN FD Disable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_FD_Dis(__CAN__)                        CLEAR_BIT((__CAN__)->CTRL, CAN0_CTRL_FD_EN_Msk)

/**
  * @brief  Judge is CAN FD Enable or not
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 CAN FD is Disable
  * @retval 1 CAN FD is Enable
  */
#define __LL_CAN_IsFDEn(__CAN__)                        READ_BIT_SHIFT((__CAN__)->CTRL, CAN0_CTRL_FD_EN_Msk, CAN0_CTRL_FD_EN_Pos)

/**
  * @brief  CAN FD ISO Enable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_FD_ISO_En(__CAN__)                     SET_BIT((__CAN__)->CTRL, CAN0_CTRL_FD_ISO_Msk)

/**
  * @brief  CAN FD ISO Disable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_FD_ISO_Dis(__CAN__)                    CLEAR_BIT((__CAN__)->CTRL, CAN0_CTRL_FD_ISO_Msk)

/**
  * @brief  TX secondary buffer next set
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TxSecNext_Set(__CAN__)                 SET_BIT((__CAN__)->CTRL, CAN0_CTRL_TSNEXT_Msk)

/**
  * @brief  TX buffer select PTB
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TxBufSel_PTB(__CAN__)                  CLEAR_BIT((__CAN__)->CTRL, CAN0_CTRL_TBSEL_Msk)

/**
  * @brief  TX buffer select STB
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TxBufSel_STB(__CAN__)                  SET_BIT((__CAN__)->CTRL, CAN0_CTRL_TBSEL_Msk)

/**
  * @brief  Listen only mode Enable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_ListenOnlyMode_En(__CAN__)             SET_BIT((__CAN__)->CTRL, CAN0_CTRL_LOM_Msk)

/**
  * @brief  Listen only mode Disable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_ListenOnlyMode_Dis(__CAN__)            CLEAR_BIT((__CAN__)->CTRL, CAN0_CTRL_LOM_Msk)

/**
  * @brief  TX primary enable set
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TxPriEn_Set(__CAN__)                   SET_BIT((__CAN__)->CTRL, CAN0_CTRL_TPE_Msk)

/**
  * @brief  TX primary enable get
  * @param  __CAN__ Specifies CAN peripheral
  * @return TX primary enable status
  */
#define __LL_CAN_TxPriEn_Get(__CAN__)                   READ_BIT_SHIFT((__CAN__)->CTRL, CAN0_CTRL_TPE_Msk, CAN0_CTRL_TPE_Pos)

/**
  * @brief  TX primary abort set
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TxPriAbort_Set(__CAN__)                SET_BIT((__CAN__)->CTRL, CAN0_CTRL_TPA_Msk)

/**
  * @brief  TX secondary one set
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TxSecOne_Set(__CAN__)                  SET_BIT((__CAN__)->CTRL, CAN0_CTRL_TSONE_Msk)

/**
  * @brief  TX secondary one get
  * @param  __CAN__ Specifies CAN peripheral
  * @return TX secondary one status
  */
#define __LL_CAN_TxSecOne_Get(__CAN__)                  READ_BIT_SHIFT((__CAN__)->CTRL, CAN0_CTRL_TSONE_Msk, CAN0_CTRL_TSONE_Pos)

/**
  * @brief  TX secondary all set
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TxSecAll_Set(__CAN__)                  SET_BIT((__CAN__)->CTRL, CAN0_CTRL_TSALL_Msk)

/**
  * @brief  TX secondary all get
  * @param  __CAN__ Specifies CAN peripheral
  * @return TX secondary send all status
  */
#define __LL_CAN_TxSecAll_Get(__CAN__)                  READ_BIT_SHIFT((__CAN__)->CTRL, CAN0_CTRL_TSALL_Msk, CAN0_CTRL_TSALL_Pos)

/**
  * @brief  TX secondary abort set
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TxSecAbort_Set(__CAN__)                SET_BIT((__CAN__)->CTRL, CAN0_CTRL_TSA_Msk)

/**
  * @brief  CAN reset set
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_Reset_Set(__CAN__)                     SET_BIT((__CAN__)->CTRL, CAN0_CTRL_RESET_Msk)

/**
  * @brief  CAN reset clear
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_Reset_Clr(__CAN__)                     CLEAR_BIT((__CAN__)->CTRL, CAN0_CTRL_RESET_Msk)

/**
  * @brief  CAN reset status get
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 CAN reset has set
  * @retval 1 CAN reset has clear
  */
#define __LL_CAN_ResetSta_Get(__CAN__)                  READ_BIT_SHIFT((__CAN__)->CTRL, CAN0_CTRL_RESET_Msk, CAN0_CTRL_RESET_Pos)

/**
  * @brief  CAN loop back mode external enable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_LoopBackModeExt_En(__CAN__)            SET_BIT((__CAN__)->CTRL, CAN0_CTRL_LBME_Msk)

/**
  * @brief  CAN loop back mode external disable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_LoopBackModeExt_Dis(__CAN__)           CLEAR_BIT((__CAN__)->CTRL, CAN0_CTRL_LBME_Msk)

/**
  * @brief  CAN loop back mode internal enable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_LoopBackModeInternal_En(__CAN__)       SET_BIT((__CAN__)->CTRL, CAN0_CTRL_LBMI_Msk)

/**
  * @brief  CAN loop back mode internal disable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_LoopBackModeInternal_Dis(__CAN__)      CLEAR_BIT((__CAN__)->CTRL, CAN0_CTRL_LBMI_Msk)

/**
  * @brief  CAN TX primary single shot enable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TxPriSingleShot_En(__CAN__)            SET_BIT((__CAN__)->CTRL, CAN0_CTRL_TPSS_Msk)

/**
  * @brief  CAN TX primary single shot disable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TxPriSingleShot_Dis(__CAN__)           CLEAR_BIT((__CAN__)->CTRL, CAN0_CTRL_TPSS_Msk)

/**
  * @brief  CAN TX secondary single shot enable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TxSecSingleShot_En(__CAN__)            SET_BIT((__CAN__)->CTRL, CAN0_CTRL_TSSS_Msk)

/**
  * @brief  CAN TX secondary single shot disable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TxSecSingleShot_Dis(__CAN__)           CLEAR_BIT((__CAN__)->CTRL, CAN0_CTRL_TSSS_Msk)


/**
  * @brief  Judge is RX buffer overflow or not
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 Isn't RX buffer overflow
  * @retval 1 Is RX buffer overflow
  */
#define __LL_CAN_IsRxBufOver(__CAN__)                   READ_BIT_SHIFT((__CAN__)->STATUS, CAN0_STATUS_ROV_Msk, CAN0_STATUS_ROV_Pos)

/**
  * @brief  RX buffer status get
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 RX buffer empty
  * @retval 1 empty < RX buffer < almost full
  * @retval 2 RX buffer >= almost full
  * @retval 3 RX buffer full
  */
#define __LL_CAN_RxBufSta_Get(__CAN__)                  READ_BIT_SHIFT((__CAN__)->STATUS, CAN0_STATUS_RSTAT_Msk, CAN0_STATUS_RSTAT_Pos)

/**
  * @brief  TX secondary status get
  * @param  __CAN__ Specifies CAN peripheral
  * @return Number of secondary buffer filled messages
  */
#define __LL_CAN_TxSecSta_Get(__CAN__)                  READ_BIT_SHIFT((__CAN__)->STATUS, CAN0_STATUS_TSSTAT_Msk, CAN0_STATUS_TSSTAT_Pos)

/**
  * @brief  Judge is RX active or not
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 Isn't RX active
  * @retval 1 Is RX active
  */
#define __LL_CAN_IsRxActive(__CAN__)                    READ_BIT_SHIFT((__CAN__)->STATUS, CAN0_STATUS_RACTIVE_Msk, CAN0_STATUS_RACTIVE_Pos)

/**
  * @brief  Judge is TX active or not
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 Isn't TX active
  * @retval 1 Is TX active
  */
#define __LL_CAN_IsTxActive(__CAN__)                    READ_BIT_SHIFT((__CAN__)->STATUS, CAN0_STATUS_TACTIVE_Msk, CAN0_STATUS_TACTIVE_Pos)

/**
  * @brief  Judge is bus off or not
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 Is bus on
  * @retval 1 Is bus off
  */
#define __LL_CAN_IsBusOff(__CAN__)                      READ_BIT_SHIFT((__CAN__)->STATUS, CAN0_STATUS_BUSOFF_Msk, CAN0_STATUS_BUSOFF_Pos)


/**
  * @brief  Error passive interrupt enable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_ErrPassive_INT_En(__CAN__)             SET_BIT((__CAN__)->INTREN, CAN0_INTREN_EPIE_Msk)

/**
  * @brief  Error passive interrupt disable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_ErrPassive_INT_Dis(__CAN__)            CLEAR_BIT((__CAN__)->INTREN, CAN0_INTREN_EPIE_Msk)

/**
  * @brief  Arbitration lost interrupt enable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_ArbLost_INT_En(__CAN__)                SET_BIT((__CAN__)->INTREN, CAN0_INTREN_ALIE_Msk)

/**
  * @brief  Arbitration lost interrupt disable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_ArbLost_INT_Dis(__CAN__)               CLEAR_BIT((__CAN__)->INTREN, CAN0_INTREN_ALIE_Msk)

/**
  * @brief  Bus error interrupt enable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_BusErr_INT_En(__CAN__)                 SET_BIT((__CAN__)->INTREN, CAN0_INTREN_BEIE_Msk)

/**
  * @brief  Bus error interrupt disable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_BusErr_INT_Dis(__CAN__)                CLEAR_BIT((__CAN__)->INTREN, CAN0_INTREN_BEIE_Msk)

/**
  * @brief  RX interrupt enable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_Rx_INT_En(__CAN__)                     SET_BIT((__CAN__)->INTREN, CAN0_INTREN_RIE_Msk)

/**
  * @brief  RX interrupt disable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_Rx_INT_Dis(__CAN__)                    CLEAR_BIT((__CAN__)->INTREN, CAN0_INTREN_RIE_Msk)

/**
  * @brief  RX buffer overrun interrupt enable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_RxBufOver_INT_En(__CAN__)              SET_BIT((__CAN__)->INTREN, CAN0_INTREN_ROIE_Msk)

/**
  * @brief  RX buffer overrun interrupt disable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_RxBufOver_INT_Dis(__CAN__)             CLEAR_BIT((__CAN__)->INTREN, CAN0_INTREN_ROIE_Msk)

/**
  * @brief  RX buffer full interrupt enable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_RxBufFull_INT_En(__CAN__)              SET_BIT((__CAN__)->INTREN, CAN0_INTREN_RFIE_Msk)

/**
  * @brief  RX buffer full interrupt disable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_RxBufFull_INT_Dis(__CAN__)             CLEAR_BIT((__CAN__)->INTREN, CAN0_INTREN_RFIE_Msk)

/**
  * @brief  RX buffer almost full interrupt enable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_RxBufAlmostFull_INT_En(__CAN__)        SET_BIT((__CAN__)->INTREN, CAN0_INTREN_RAFIE_Msk)

/**
  * @brief  RX buffer almost full interrupt disable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_RxBufAlmostFull_INT_Dis(__CAN__)       CLEAR_BIT((__CAN__)->INTREN, CAN0_INTREN_RAFIE_Msk)

/**
  * @brief  TX primary interrupt enable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TxPri_INT_En(__CAN__)                  SET_BIT((__CAN__)->INTREN, CAN0_INTREN_TPIE_Msk)

/**
  * @brief  TX primary interrupt disable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TxPri_INT_Dis(__CAN__)                 CLEAR_BIT((__CAN__)->INTREN, CAN0_INTREN_TPIE_Msk)

/**
  * @brief  TX secondary interrupt enable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TxSec_INT_En(__CAN__)                  SET_BIT((__CAN__)->INTREN, CAN0_INTREN_TSIE_Msk)

/**
  * @brief  TX secondary interrupt disable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TxSec_INT_Dis(__CAN__)                 CLEAR_BIT((__CAN__)->INTREN, CAN0_INTREN_TSIE_Msk)

/**
  * @brief  Error interrupt enalbe
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_Err_INT_En(__CAN__)                    SET_BIT((__CAN__)->INTREN, CAN0_INTREN_EIE_Msk)

/**
  * @brief  Error interrupt disalbe
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_Err_INT_Dis(__CAN__)                   CLEAR_BIT((__CAN__)->INTREN, CAN0_INTREN_EIE_Msk)

/**
  * @brief  Judge is TX secondary buffer full or not
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 Isn't TX secondary buffer full
  * @retval 1 Is TX secondary buffer full
  */
#define __LL_CAN_IsTxSecBufFull(__CAN__)                READ_BIT_SHIFT((__CAN__)->INTREN, CAN0_INTREN_TSFF_Msk, CAN0_INTREN_TSFF_Pos)

/**
  * @brief  All Interrupt Enable Status Get
  * @param  __CAN__ Specifies CAN peripheral
  * @return All Interrupt Enable Status
  */
#define __LL_CAN_AllIntEn_Get(__CAN__)                  READ_REG((__CAN__)->INTREN)


/**
  * @brief  Judge is error warning limit reached or not
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 Isn't error warning limit reached
  * @retval 1 Is error warning limit reached
  */
#define __LL_CAN_IsErrWarnLimitReached(__CAN__)         READ_BIT_SHIFT((__CAN__)->INTRST, CAN0_INTRST_EWARN_Msk, CAN0_INTRST_EWARN_Pos)

/**
  * @brief  Judge is error passive mode active or not
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 Error passive mode isn't active
  * @retval 1 Error passive mode is active
  */
#define __LL_CAN_IsErrPassiveModeActive(__CAN__)        READ_BIT_SHIFT((__CAN__)->INTRST, CAN0_INTRST_EPASS_Msk, CAN0_INTRST_EPASS_Pos)

/**
  * @brief  Judge is Error Passive Interrupt Pending or not
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 Isn't Error Passive Interrupt Pending
  * @retval 1 Is Error Passive Interrupt Pending
  */
#define __LL_CAN_IsErrPassiveIntPnd(__CAN__)            READ_BIT_SHIFT((__CAN__)->INTRST, CAN0_INTRST_EPIF_Msk, CAN0_INTRST_EPIF_Pos)

/**
  * @brief  Error Passive Interrupt Pending Clear
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_ErrPassiveIntPnd_Clr(__CAN__)          WRITE_REG((__CAN__)->INTRST, CAN0_INTRST_EPIF_Msk)

/**
  * @brief  Judge is Arbitration Lost Interrupt Pending or not
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 Isn't Arbitration Lost Interrupt Pending
  * @retval 1 Is Arbitration Lost Interrupt Pending
  */
#define __LL_CAN_IsArbLostIntPnd(__CAN__)               READ_BIT_SHIFT((__CAN__)->INTRST, CAN0_INTRST_ALIF_Msk, CAN0_INTRST_ALIF_Pos)

/**
  * @brief  Arbitration Lost Interrupt Pending Clear
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_ArbLostIntPnd_Clr(__CAN__)             WRITE_REG((__CAN__)->INTRST, CAN0_INTRST_ALIF_Msk)

/**
  * @brief  Judge is Bus Error Interrupt Pending or not
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 Isn't Bus Error Interrupt Pending
  * @retval 1 Is Bus Error Interrupt Pending
  */
#define __LL_CAN_IsBusErrIntPnd(__CAN__)                READ_BIT_SHIFT((__CAN__)->INTRST, CAN0_INTRST_BEIF_Msk, CAN0_INTRST_BEIF_Pos)

/**
  * @brief  Bus Error Interrupt Pending Clear
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_BusErrIntPnd_Clr(__CAN__)              WRITE_REG((__CAN__)->INTRST, CAN0_INTRST_BEIF_Msk)

/**
  * @brief  Judge is Receive Interrupt Pending or not
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 Isn't Receive Interrupt Pending
  * @retval 1 Is Receive Interrupt Pending
  */
#define __LL_CAN_IsRxIntPnd(__CAN__)                    READ_BIT_SHIFT((__CAN__)->INTRST, CAN0_INTRST_RIF_Msk, CAN0_INTRST_RIF_Pos)

/**
  * @brief  Receive Interrupt Pending Clear
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_RxIntPnd_Clr(__CAN__)                  WRITE_REG((__CAN__)->INTRST, CAN0_INTRST_RIF_Msk)

/**
  * @brief  Judge is Rx Buffer Overflow Interrupt Pending or not
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 Isn't Rx Buffer Overflow Interrupt Pending
  * @retval 1 Is Rx Buffer Overflow Interrupt Pending
  */
#define __LL_CAN_IsRxOverIntPnd(__CAN__)                READ_BIT_SHIFT((__CAN__)->INTRST, CAN0_INTRST_ROIF_Msk, CAN0_INTRST_ROIF_Pos)

/**
  * @brief  Rx Buffer Overflow Interrupt Pending Clear
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_RxOverIntPnd_Clr(__CAN__)              WRITE_REG((__CAN__)->INTRST, CAN0_INTRST_ROIF_Msk)

/**
  * @brief  Judge is Rx Buffer Full Interrupt Pending or not
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 Isn't Rx Buffer Full Interrupt Pending
  * @retval 1 Is Rx Buffer Full Interrupt Pending
  */
#define __LL_CAN_IsRxFullIntPnd(__CAN__)                READ_BIT_SHIFT((__CAN__)->INTRST, CAN0_INTRST_RFIF_Msk, CAN0_INTRST_RFIF_Pos)

/**
  * @brief  Rx Buffer Full Interrupt Pending Clear
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_RxFullIntPnd_Clr(__CAN__)              WRITE_REG((__CAN__)->INTRST, CAN0_INTRST_RFIF_Msk)

/**
  * @brief  Judge is Rx Buffer Almost Full Interrupt Pending or not
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 Isn't Rx Buffer Almost Full Interrupt Pending
  * @retval 1 Is Rx Buffer Almost Full Interrupt Pending
  */
#define __LL_CAN_IsRxAlmostFullIntPnd(__CAN__)          READ_BIT_SHIFT((__CAN__)->INTRST, CAN0_INTRST_RAFIF_Msk, CAN0_INTRST_RAFIF_Pos)

/**
  * @brief  Rx Buffer Almost Full Interrupt Pending Clear
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_RxAlmostFullIntPnd_Clr(__CAN__)        WRITE_REG((__CAN__)->INTRST, CAN0_INTRST_RAFIF_Msk)

/**
  * @brief  Judge is Transmission Primary Interrupt Pending or not
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 Isn't Transmission Primary Interrupt Pending
  * @retval 1 Is Transmission Primary Interrupt Pending
  */
#define __LL_CAN_IsTxPriIntPnd(__CAN__)                 READ_BIT_SHIFT((__CAN__)->INTRST, CAN0_INTRST_TPIF_Msk, CAN0_INTRST_TPIF_Pos)

/**
  * @brief  Transmission Primary Interrupt Pending Clear
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TxPriIntPnd_Clr(__CAN__)               WRITE_REG((__CAN__)->INTRST, CAN0_INTRST_TPIF_Msk)

/**
  * @brief  Judge is Transmission Secondary Interrupt Pending or not
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 Isn't Transmission Secondary Interrupt Pending
  * @retval 1 Is Transmission Secondary Interrupt Pending
  */
#define __LL_CAN_IsTxSecIntPnd(__CAN__)                 READ_BIT_SHIFT((__CAN__)->INTRST, CAN0_INTRST_TSIF_Msk, CAN0_INTRST_TSIF_Pos)

/**
  * @brief  Transmission Secondary Interrupt Pending Clear
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TxSecIntPnd_Clr(__CAN__)               WRITE_REG((__CAN__)->INTRST, CAN0_INTRST_TSIF_Msk)

/**
  * @brief  Judge is Error Interrupt Pending or not
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 Isn't Error Interrupt Pending
  * @retval 1 Is Error Interrupt Pending
  */
#define __LL_CAN_IsErrIntPnd(__CAN__)                   READ_BIT_SHIFT((__CAN__)->INTRST, CAN0_INTRST_EIF_Msk, CAN0_INTRST_EIF_Pos)

/**
  * @brief  Error Interrupt Pending Clear
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_ErrIntPnd_Clr(__CAN__)                 WRITE_REG((__CAN__)->INTRST, CAN0_INTRST_EIF_Msk)

/**
  * @brief  Judge is Abort Interrupt Pending or not
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 Isn't Abort Interrupt Pending
  * @retval 1 Is Abort Interrupt Pending
  */
#define __LL_CAN_IsAbortIntPnd(__CAN__)                 READ_BIT_SHIFT((__CAN__)->INTRST, CAN0_INTRST_AIF_Msk, CAN0_INTRST_AIF_Pos)

/**
  * @brief  Abort Interrupt Pending Clear
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_AbortIntPnd_Clr(__CAN__)               WRITE_REG((__CAN__)->INTRST, CAN0_INTRST_AIF_Msk)

/**
  * @brief  Interrupt status get
  * @param  __CAN__ Specifies CAN peripheral
  * @return Interrupt status
  */
#define __LL_CAN_AllIntPnd_Get(__CAN__)                 READ_REG((__CAN__)->INTRST)

/**
  * @brief  All Interrupt Pending Clear
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_AllIntPnd_Clr(__CAN__)                 WRITE_REG((__CAN__)->INTRST, 0xffffffffUL)


/**
  * @brief  Fast speed bit timing segment1 set
  * @param  __CAN__ Specifies CAN peripheral
  * @param  val set value
  * @return None
  */
#define __LL_CAN_FS_BitTimingSeg1_Set(__CAN__, val)     \
        MODIFY_REG((__CAN__)->BITTIME, CAN0_BITTIME_F_SEG1_Msk, (((val) & 0xfUL) << CAN0_BITTIME_F_SEG1_Pos))

/**
  * @brief  Slow speed synchronization jump width set
  * @param  __CAN__ Specifies CAN peripheral
  * @param  val set value
  * @return None
  */
#define __LL_CAN_SS_SyncJumpWidth_Set(__CAN__, val)     \
        MODIFY_REG((__CAN__)->BITTIME, CAN0_BITTIME_S_SJW_Msk, (((val) & 0xfUL) << CAN0_BITTIME_S_SJW_Pos))

/**
  * @brief  Fast speed bit timing segment2 set
  * @param  __CAN__ Specifies CAN peripheral
  * @param  val set value
  * @return None
  */
#define __LL_CAN_FS_BitTimingSeg2_Set(__CAN__, val)     \
        MODIFY_REG((__CAN__)->BITTIME, CAN0_BITTIME_F_SEG2_Msk, (((val) & 0x7UL) << CAN0_BITTIME_F_SEG2_Pos))

/**
  * @brief  Slow speed bit timing segment2 set
  * @param  __CAN__ Specifies CAN peripheral
  * @param  val set value
  * @return None
  */
#define __LL_CAN_SS_BitTimingSeg2_Set(__CAN__, val)     \
        MODIFY_REG((__CAN__)->BITTIME, CAN0_BITTIME_S_SEG2_Msk, (((val) & 0x1fUL) << CAN0_BITTIME_S_SEG2_Pos))

/**
  * @brief  Fast speed synchronization jump width set
  * @param  __CAN__ Specifies CAN peripheral
  * @param  val set value
  * @return None
  */
#define __LL_CAN_FS_SyncJumpWidth_Set(__CAN__, val)     \
        MODIFY_REG((__CAN__)->BITTIME, CAN0_BITTIME_F_SJW_Msk, (((val) & 0x3UL) << CAN0_BITTIME_F_SJW_Pos))

/**
  * @brief  Slow speed bit timing segment1 set
  * @param  __CAN__ Specifies CAN peripheral
  * @param  val set value
  * @return None
  */
#define __LL_CAN_SS_BitTimingSeg1_Set(__CAN__, val)     \
        MODIFY_REG((__CAN__)->BITTIME, CAN0_BITTIME_S_SEG1_Msk, (((val) & 0x3fUL) << CAN0_BITTIME_S_SEG1_Pos))


/**
  * @brief  Transmitter Delay Compensation Enable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TransDlyComp_En(__CAN__)               SET_BIT((__CAN__)->PRESC, CAN0_PRESC_TDCEN_Msk)

/**
  * @brief  Transmitter Delay Compensation Disable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_TransDlyComp_Dis(__CAN__)              CLEAR_BIT((__CAN__)->PRESC, CAN0_PRESC_TDCEN_Msk)

/**
  * @brief  Secondary Sample Point Offset Set
  * @param  __CAN__ Specifies CAN peripheral
  * @param  val set value
  * @return None
  */
#define __LL_CAN_SecSamplePointOffset_Set(__CAN__, val) \
        MODIFY_REG((__CAN__)->PRESC, CAN0_PRESC_SSPOFF_Msk, (((val) & 0x1fUL) << CAN0_PRESC_SSPOFF_Pos))

/**
  * @brief  Fast speed prescaler set
  * @param  __CAN__ Specifies CAN peripheral
  * @param  val set value
  * @return None
  */
#define __LL_CAN_FS_Prescaler_Set(__CAN__, val)         \
        MODIFY_REG((__CAN__)->PRESC, CAN0_PRESC_F_PRESC_Msk, (((val) & 0xffUL) << CAN0_PRESC_F_PRESC_Pos))

/**
  * @brief  Slow speed prescaler set
  * @param  __CAN__ Specifies CAN peripheral
  * @param  val set value
  * @return None
  */
#define __LL_CAN_SS_Prescaler_Set(__CAN__, val)         \
        MODIFY_REG((__CAN__)->PRESC, CAN0_PRESC_S_PRESC_Msk, (((val) & 0xffUL) << CAN0_PRESC_S_PRESC_Pos))


/**
  * @brief  RX error count get
  * @param  __CAN__ Specifies CAN peripheral
  * @return RX error count
  */
#define __LL_CAN_RxErrCnt_Get(__CAN__)                  READ_BIT_SHIFT((__CAN__)->ERRST, CAN0_ERRST_RECNT_Msk, CAN0_ERRST_RECNT_Pos)

/**
  * @brief  TX error count get
  * @param  __CAN__ Specifies CAN peripheral
  * @return TX error count
  */
#define __LL_CAN_TxErrCnt_Get(__CAN__)                  READ_BIT_SHIFT((__CAN__)->ERRST, CAN0_ERRST_TECNT_Msk, CAN0_ERRST_TECNT_Pos)

/**
  * @brief  Error code get
  * @param  __CAN__ Specifies CAN peripheral
  * @retval 0 no error
  * @retval 1 bit error
  * @retval 2 form error
  * @retval 3 stuff error
  * @retval 4 acknowledgement error
  * @retval 5 CRC error
  * @retval 6 other error
  * @retval 7 not used
  */
#define __LL_CAN_ErrCode_Get(__CAN__)                   READ_BIT_SHIFT((__CAN__)->ERRST, CAN0_ERRST_KOER_Msk, CAN0_ERRST_KOER_Pos)

/**
  * @brief  Arbitration lost capture get
  * @param  __CAN__ Specifies CAN peripheral
  * @return bit position in the frame where the arbitration has been lost
  */
#define __LL_CAN_ArbLostCapture_Get(__CAN__)            READ_BIT_SHIFT((__CAN__)->ERRST, CAN0_ERRST_ALC_Msk, CAN0_ERRST_ALC_Pos)


/**
  * @brief  Acceptance filter enable
  * @param  __CAN__ Specifies CAN peripheral
  * @param  fil_num Acceptance filter slot number
  * @note   fil_num value range [0, 15]
  * @return None
  */
#define __LL_CAN_AcceptFil_En(__CAN__, fil_num)         SET_BIT((__CAN__)->ACFEN, BIT(fil_num))

/**
  * @brief  Acceptance filter disable
  * @param  __CAN__ Specifies CAN peripheral
  * @param  fil_num Acceptance filter slot number
  * @note   fil_num value range [0, 15]
  * @return None
  */
#define __LL_CAN_AcceptFil_Dis(__CAN__, fil_num)        CLEAR_BIT((__CAN__)->ACFEN, BIT(fil_num))


/**
  * @brief  Acceptance filter content select mask
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_AcceptFilContentSel_Mask(__CAN__)      SET_BIT((__CAN__)->ACFCTRL, CAN0_ACFCTRL_SELMASK_Msk)

/**
  * @brief  Acceptance filter content select code
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_AcceptFilContentSel_Code(__CAN__)      CLEAR_BIT((__CAN__)->ACFCTRL, CAN0_ACFCTRL_SELMASK_Msk)

/**
  * @brief  Acceptance filter address set
  * @param  __CAN__ Specifies CAN peripheral
  * @param  val set value
  * @return None
  */
#define __LL_CAN_AcceptFilAddr_Set(__CAN__, val)        \
        MODIFY_REG((__CAN__)->ACFCTRL, CAN0_ACFCTRL_ACFADR_Msk, (((val) & 0xfUL) << CAN0_ACFCTRL_ACFADR_Pos))


/**
  * @brief  Acceptance mask IDE bit check enable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_AcceptFilMaskIDE_En(__CAN__)           SET_BIT((__CAN__)->ACF, CAN0_ACF_AIDEE_Msk)

/**
  * @brief  Acceptance mask IDE bit check disable
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_AcceptFilMaskIDE_Dis(__CAN__)          CLEAR_BIT((__CAN__)->ACF, CAN0_ACF_AIDEE_Msk)

/**
  * @brief  Acceptance filter accepts only extended frames
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_AcceptFilMaskIDESel_Ext(__CAN__)       SET_BIT((__CAN__)->ACF, CAN0_ACF_AIDE_Msk)

/**
  * @brief  Acceptance filter accepts only standard frames
  * @param  __CAN__ Specifies CAN peripheral
  * @return None
  */
#define __LL_CAN_AcceptFilMaskIDESel_Std(__CAN__)       CLEAR_BIT((__CAN__)->ACF, CAN0_ACF_AIDE_Msk)

/**
  * @brief  Acceptance filter Rx frame type set
  * @param  __CAN__ Specifies CAN peripheral
  * @param  frm_type Rx frame type
  * @return None
  */
#define __LL_CAN_AcceptFilRxFrm_Set(__CAN__, frm_type)  \
        MODIFY_REG((__CAN__)->ACF, CAN0_ACF_AIDEE_Msk | CAN0_ACF_AIDE_Msk, ((frm_type) & 0x3) << CAN0_ACF_AIDE_Pos)

/**
  * @brief  Acceptance filter code or mask set
  * @param  __CAN__ Specifies CAN peripheral
  * @param  val set value
  * @return None
  */
#define __LL_CAN_AcceptFilCodeOrMaskVal_Set(__CAN__, val)   \
        MODIFY_REG((__CAN__)->ACF, CAN0_ACF_ACF_X_Msk, (((val) & 0x1fffffffUL) << CAN0_ACF_ACF_X_Pos))


/**
  * @brief  Rx Buffer ID Register Read
  * @param  __CAN__ Specifies CAN peripheral
  * @return Read Value
  */
#define __LL_CAN_RxBufReg_ID_Read(__CAN__)              READ_REG((__CAN__)->RBUFID)


/**
  * @brief  Rx Buffer Control Register Read
  * @param  __CAN__ Specifies CAN peripheral
  * @return Read Value
  */
#define __LL_CAN_RxBufReg_Ctrl_Read(__CAN__)            READ_REG((__CAN__)->RBUFCR)


/**
  * @brief  Rx Buffer Data Register Read
  * @param  __CAN__ Specifies CAN peripheral
  * @param  num Data Register Number range [0, 15]
  * @return Read Value
  */
#define __LL_CAN_RxBufReg_Data_Read(__CAN__, num)       READ_REG((__CAN__)->RBUFDT[((uint32_t)(num) % 16)])


/**
  * @brief  Tx Buffer ID Register Write
  * @param  __CAN__ Specifies CAN peripheral
  * @param  val Write Value
  * @return None
  */
#define __LL_CAN_TxBufReg_ID_Write(__CAN__, val)        WRITE_REG((__CAN__)->TBUFID, val)


/**
  * @brief  Tx Buffer Control Register Write
  * @param  __CAN__ Specifies CAN peripheral
  * @param  val Write Value
  * @return None
  */
#define __LL_CAN_TxBufReg_Ctrl_Write(__CAN__, val)      WRITE_REG((__CAN__)->TBUFCR, val)


/**
  * @brief  Tx Buffer Data Register Write
  * @param  __CAN__ Specifies CAN peripheral
  * @param  num Data Register Number range [0, 15]
  * @param  val Write Value
  * @return None
  */
#define __LL_CAN_TxBufReg_Data_Write(__CAN__, num, val) WRITE_REG((__CAN__)->TBUFDT[((uint32_t)(num) % 16)], val)



/**
  * @brief CAN frame ID format to 11 bits
  */
#define __LL_CAN_FrameIDFormat_11Bits(n)                ((n) & 0x7FFUL)

/**
  * @brief CAN frame ID format to 29 bits
  */
#define __LL_CAN_FrameIDFormat_29Bits(n)                ((n) & 0x1FFFFFFFUL)


/**
  * @brief CAN TxFIFO Size Get
  * @param  __CAN__ Specifies CAN peripheral
  * @return CAN TxFIFO Size in Byte Unit
  */
#define __LL_CAN_TxFIFOSize_Get(__CAN__)                (sizeof((__CAN__)->TBUFDT))

/**
  * @brief CAN RxFIFO Size Get
  * @param  __CAN__ Specifies CAN peripheral
  * @return CAN RxFIFO Size in Byte Unit
  */
#define __LL_CAN_RxFIFOSize_Get(__CAN__)                (sizeof((__CAN__)->RBUFDT))

/**
  * @}
  */


/* Exported Types ------------------------------------------------------------*/
/** @defgroup CAN_LL_Exported_Types CAN LL Exported Types
  * @brief    CAN LL Exported Types
  * @{
  */

/**
  * @brief CAN Uer callback function type definition
  */
typedef void (*CAN_UserCallback)(void);

/**
  * @brief CAN Instance Definition
  */
typedef enum {
    CAN_INSTANCE_0 = 0,         /*!< CAN Instance 0         */
    CAN_INSTANCE_1,             /*!< CAN Instance 1         */
    CAN_INSTANCE_NUMS,          /*!< CAN Instance Numbers   */
} CAN_InstanceETypeDef;

/**
  * @brief CAN State definition
  */
typedef enum {
    CAN_STATE_RESET,            /*!< Peripheral not Initialized                         */
    CAN_STATE_READY,            /*!< Peripheral Initialized and ready for use           */
    CAN_STATE_BUSY,             /*!< an internal process is ongoing                     */
    CAN_STATE_BUSY_TX,          /*!< Data Transmission process is ongoing               */
    CAN_STATE_BUSY_RX,          /*!< Data Reception process is ongoing                  */
    CAN_STATE_ERROR,            /*!< CAN error state                                    */
} CAN_StateETypeDef;


/**
  *@brief CAN RX buffer status enum type define
  */
typedef enum {
    CAN_RX_BUF_STA_EMPTY = 0,       /*!< CAN RX buffer empty        */
    CAN_RX_BUF_STA_FEW,             /*!< CAN RX buffer few          */
    CAN_RX_BUF_STA_ALMOST_FULL,     /*!< CAN RX buffer almost full  */
    CAN_RX_BUF_STA_FULL,            /*!< CAN RX buffer full         */
} CAN_RxBufStaETypeDef;

/**
  * @brief CAN acceptance filter slot definition.
  */
typedef enum {
    CAN_ACCEPT_FILT_SLOT_0 = 0,     /*!< CAN acceptance filter slot 0       */
    CAN_ACCEPT_FILT_SLOT_1,         /*!< CAN acceptance filter slot 1       */
    CAN_ACCEPT_FILT_SLOT_2,         /*!< CAN acceptance filter slot 2       */
    CAN_ACCEPT_FILT_SLOT_3,         /*!< CAN acceptance filter slot 3       */
    CAN_ACCEPT_FILT_SLOT_4,         /*!< CAN acceptance filter slot 4       */
    CAN_ACCEPT_FILT_SLOT_5,         /*!< CAN acceptance filter slot 5       */
    CAN_ACCEPT_FILT_SLOT_6,         /*!< CAN acceptance filter slot 6       */
    CAN_ACCEPT_FILT_SLOT_7,         /*!< CAN acceptance filter slot 7       */
    CAN_ACCEPT_FILT_SLOT_8,         /*!< CAN acceptance filter slot 8       */
    CAN_ACCEPT_FILT_SLOT_9,         /*!< CAN acceptance filter slot 9       */
    CAN_ACCEPT_FILT_SLOT_10,        /*!< CAN acceptance filter slot 10      */
    CAN_ACCEPT_FILT_SLOT_11,        /*!< CAN acceptance filter slot 11      */
    CAN_ACCEPT_FILT_SLOT_12,        /*!< CAN acceptance filter slot 12      */
    CAN_ACCEPT_FILT_SLOT_13,        /*!< CAN acceptance filter slot 13      */
    CAN_ACCEPT_FILT_SLOT_14,        /*!< CAN acceptance filter slot 14      */
    CAN_ACCEPT_FILT_SLOT_15,        /*!< CAN acceptance filter slot 15      */
    CAN_ACCEPT_FILT_SLOT_NUMS,      /*!< CAN acceptance filter slot Numbers */
} CAN_AcceptFilSlotETypeDef;

/**
  * @brief CAN acceptance filter Rx frame type definition.
  */
typedef enum {
    CAN_ACCEPT_FILT_FRM_STD_EXT = 0,/*!< CAN acceptance filter Rx frame type Standard and Extend    */
    CAN_ACCEPT_FILT_FRM_STD     = 2,/*!< CAN acceptance filter Rx frame type Standard               */
    CAN_ACCEPT_FILT_FRM_EXT,        /*!< CAN acceptance filter Rx frame type Extend                 */
} CAN_AcceptFilRxFrmETypeDef;

/**
  * @brief CAN RX buffer almost full warnning limit definition
  */
typedef enum {
    CAN_RX_ALMOST_FULL_LIMIT_0 = 0, /*!< CAN RX buffer almost full warnning limit: 0 Byte   */
    CAN_RX_ALMOST_FULL_LIMIT_1,     /*!< CAN RX buffer almost full warnning limit: 1 Byte   */
    CAN_RX_ALMOST_FULL_LIMIT_2,     /*!< CAN RX buffer almost full warnning limit: 2 Byte   */
    CAN_RX_ALMOST_FULL_LIMIT_3,     /*!< CAN RX buffer almost full warnning limit: 3 Byte   */
    CAN_RX_ALMOST_FULL_LIMIT_4,     /*!< CAN RX buffer almost full warnning limit: 4 Byte   */
    CAN_RX_ALMOST_FULL_LIMIT_5,     /*!< CAN RX buffer almost full warnning limit: 5 Byte   */
    CAN_RX_ALMOST_FULL_LIMIT_6,     /*!< CAN RX buffer almost full warnning limit: 6 Byte   */
    CAN_RX_ALMOST_FULL_LIMIT_7,     /*!< CAN RX buffer almost full warnning limit: 7 Byte   */
    CAN_RX_ALMOST_FULL_LIMIT_8,     /*!< CAN RX buffer almost full warnning limit: 8 Byte   */
    CAN_RX_ALMOST_FULL_LIMIT_9,     /*!< CAN RX buffer almost full warnning limit: 9 Byte   */
    CAN_RX_ALMOST_FULL_LIMIT_10,    /*!< CAN RX buffer almost full warnning limit: 10 Byte  */
    CAN_RX_ALMOST_FULL_LIMIT_11,    /*!< CAN RX buffer almost full warnning limit: 11 Byte  */
    CAN_RX_ALMOST_FULL_LIMIT_12,    /*!< CAN RX buffer almost full warnning limit: 12 Byte  */
    CAN_RX_ALMOST_FULL_LIMIT_13,    /*!< CAN RX buffer almost full warnning limit: 13 Byte  */
    CAN_RX_ALMOST_FULL_LIMIT_14,    /*!< CAN RX buffer almost full warnning limit: 14 Byte  */
    CAN_RX_ALMOST_FULL_LIMIT_15,    /*!< CAN RX buffer almost full warnning limit: 15 Byte  */
} CAN_RxAlmostFullLimitETypeDef;

/**
  * @brief CAN programmable error warning limit definition
  */
typedef enum {
    CAN_ERR_WARN_LIMIT_8 = 0,       /*!< CAN programmable error warning limit: 8 bytes      */
    CAN_ERR_WARN_LIMIT_16,          /*!< CAN programmable error warning limit: 16 bytes     */
    CAN_ERR_WARN_LIMIT_24,          /*!< CAN programmable error warning limit: 24 bytes     */
    CAN_ERR_WARN_LIMIT_32,          /*!< CAN programmable error warning limit: 32 bytes     */
    CAN_ERR_WARN_LIMIT_40,          /*!< CAN programmable error warning limit: 40 bytes     */
    CAN_ERR_WARN_LIMIT_48,          /*!< CAN programmable error warning limit: 48 bytes     */
    CAN_ERR_WARN_LIMIT_56,          /*!< CAN programmable error warning limit: 56 bytes     */
    CAN_ERR_WARN_LIMIT_64,          /*!< CAN programmable error warning limit: 64 bytes     */
    CAN_ERR_WARN_LIMIT_72,          /*!< CAN programmable error warning limit: 72 bytes     */
    CAN_ERR_WARN_LIMIT_80,          /*!< CAN programmable error warning limit: 80 bytes     */
    CAN_ERR_WARN_LIMIT_88,          /*!< CAN programmable error warning limit: 88 bytes     */
    CAN_ERR_WARN_LIMIT_96,          /*!< CAN programmable error warning limit: 96 bytes     */
    CAN_ERR_WARN_LIMIT_104,         /*!< CAN programmable error warning limit: 104 bytes    */
    CAN_ERR_WARN_LIMIT_112,         /*!< CAN programmable error warning limit: 112 bytes    */
    CAN_ERR_WARN_LIMIT_120,         /*!< CAN programmable error warning limit: 120 bytes    */
    CAN_ERR_WARN_LIMIT_128,         /*!< CAN programmable error warning limit: 128 bytes    */
} CAN_ErrWarnLimitETypeDef;

/**
  * @brief CAN Data Length Code definition
  * @note  data_len_code > 8 is only valid with FD Enable
  */
typedef enum {
    CAN_DAT_LEN_CODE_BYTE_0 = 0,    /*!< CAN Data Length 0 Byte     */
    CAN_DAT_LEN_CODE_BYTE_1,        /*!< CAN Data Length 1 Byte     */
    CAN_DAT_LEN_CODE_BYTES_2,       /*!< CAN Data Length 2 Bytes    */
    CAN_DAT_LEN_CODE_BYTES_3,       /*!< CAN Data Length 3 Bytes    */
    CAN_DAT_LEN_CODE_BYTES_4,       /*!< CAN Data Length 4 Bytes    */
    CAN_DAT_LEN_CODE_BYTES_5,       /*!< CAN Data Length 5 Bytes    */
    CAN_DAT_LEN_CODE_BYTES_6,       /*!< CAN Data Length 6 Bytes    */
    CAN_DAT_LEN_CODE_BYTES_7,       /*!< CAN Data Length 7 Bytes    */
    CAN_DAT_LEN_CODE_BYTES_8,       /*!< CAN Data Length 8 Bytes    */
    CAN_DAT_LEN_CODE_BYTES_12,      /*!< CAN Data Length 12 Bytes   */
    CAN_DAT_LEN_CODE_BYTES_16,      /*!< CAN Data Length 16 Bytes   */
    CAN_DAT_LEN_CODE_BYTES_20,      /*!< CAN Data Length 20 Bytes   */
    CAN_DAT_LEN_CODE_BYTES_24,      /*!< CAN Data Length 24 Bytes   */
    CAN_DAT_LEN_CODE_BYTES_32,      /*!< CAN Data Length 32 Bytes   */
    CAN_DAT_LEN_CODE_BYTES_48,      /*!< CAN Data Length 48 Bytes   */
    CAN_DAT_LEN_CODE_BYTES_64,      /*!< CAN Data Length 64 Bytes   */
} CAN_DatLenCodeETypeDef;


/**
  * @brief CAN RX buffer format type definition
  */
typedef struct __CAN_RxBufFormatTypeDef {
    /*! Standard/Extended iDentifier value
     */
    uint32_t id                  : 29,

             /*! Reserved bit.
              */
             reserved1           : 2,

             /*! Error State Indicator. This is a read-only status bit for RBUF and is not available
              *  in TBUF. The protocol machine automatically embeds the correct value of ESI into
              *  transmitted frames. ESI is only included in CAN FD frames and does not exist in CAN
              *  2.0 frames.
              */
             err_state_indicator : 1;

    /*! The Data Length Code (DLC) in RBUF and TBUF defines the length of the payload(the
     *  number of payload bytes in a frame).
     */
    uint32_t data_len_code       : 4,

             /*! Bit Rate Switch
              *  0: nominal / slow bit rate for the complete frame.
              *  1: switch to data / fast bit rate for the data payload and the CRC
              *  Only CAN FD frames can switch the bitrate. Therefore BRS is forced to 0 if EDL=0
              */
             bit_rate_switch     : 1,

             /*! Extended Data Length
              *  0: CAN 2.0 frame (up to 8 bytes payload)
              *  1: CAN FD frame (up to 64 bytes payload)
              */
             extended_data_len   : 1,

             /*! Remote Transmission Request
              *  0: data frame
              *  1: remote frame
              *  Only CAN 2.0 frames can be remote frames. There is no remote frame for CAN FD.
              *  Therefore RTR is forced to 0 if EDL=1 in the TBUF.
              */
             remote_tx_req       : 1,

             /*! IDentifier Extension
              *  0: Standard Format: ID(10:0)
              *  1: Extended Format: ID(28:0)
              */
             id_extension        : 1,

             /*! Reserved bit.
              */
             reserved2           : 24;
} CAN_RxBufFormatTypeDef;

/**
  * @brief CAN TX buffer format type definition
  */
typedef struct __CAN_TxBufFormatTypeDef {
    /*! Standard/Extended iDentifier value
     */
    uint32_t id                  : 29,

             /*! Reserved bit.
              */
             reserved1           : 3;

    /*! The Data Length Code (DLC) in RBUF and TBUF defines the length of the payload(the
     *  number of payload bytes in a frame).
     */
    uint32_t data_len_code       : 4,

             /*! Bit Rate Switch
              *  0: nominal / slow bit rate for the complete frame.
              *  1: switch to data / fast bit rate for the data payload and the CRC
              *  Only CAN FD frames can switch the bitrate. Therefore BRS is forced to 0 if EDL=0
              */
             bit_rate_switch     : 1,

             /*! Extended Data Length
              *  0: CAN 2.0 frame (up to 8 bytes payload)
              *  1: CAN FD frame (up to 64 bytes payload)
              */
             extended_data_len   : 1,

             /*! Remote Transmission Request
              *  0: data frame
              *  1: remote frame
              *  Only CAN 2.0 frames can be remote frames. There is no remote frame for CAN FD.
              *  Therefore RTR is forced to 0 if EDL=1 in the TBUF.
              */
             remote_tx_req       : 1,

             /*! IDentifier Extension
              *  0: Standard Format: ID(10:0)
              *  1: Extended Format: ID(28:0)
              */
             id_extension        : 1,

             /*! Reserved bit.
              */
             reserved2           : 24;
} CAN_TxBufFormatTypeDef;


/**
 * @brief CAN IRQ Callback ID definition
 */
typedef enum {
    CAN_PTB_TX_CPLT_CB_ID,                  /*!< CAN PTB Tx Completed callback ID   */
    CAN_STB_TX_CPLT_CB_ID,                  /*!< CAN STB Tx Completed callback ID   */
    CAN_RX_CPLT_CB_ID,                      /*!< CAN Rx Completed callback ID       */
    //CAN_ERROR_CB_ID,                      /*!< CAN Error callback ID              */
} CAN_UserCallbackIdETypeDef;


/**
  * @brief CAN IRQ Callback structure definition
  */
typedef struct __CAN_UserCallbackTypeDef {
    CAN_UserCallback TxCpltCallback_ptb;    /*!< CAN PTB Tx Completed callback      */
    CAN_UserCallback TxCpltCallback_stb;    /*!< CAN STB Tx Completed callback      */
    CAN_UserCallback RxCpltCallback;        /*!< CAN Rx Completed callback          */
    //CAN_UserCallback ErrorCallback;       /*!< CAN Error callback                 */
} CAN_UserCallbackTypeDef;

/**
  * @brief CAN acceptance filter config type definition
  */
typedef struct __CAN_AcceptFilCfgTypeDef {
    CAN_AcceptFilSlotETypeDef slot;                     /*!< acceptance filter slot number      */
    uint32_t code_val;                                  /*!< acceptance filter code value       */
    uint32_t mask_val;                                  /*!< acceptance filter mask value       */
    CAN_AcceptFilRxFrmETypeDef rx_frm;                  /*!< acceptance filter Rx frame type    */
} CAN_AcceptFilCfgTypeDef;

/**
  * @brief CAN user config type definition
  */
typedef struct __CAN_UserCfgTypeDef {
    bool fd_en;                                         /*!< CAN FD Enable                              */
    bool fd_iso_en;                                     /*!< CAN FD ISO Enable                          */
    uint32_t func_clk_freq;                             /*!< CAN function clock freq                    */

    uint32_t baudrate_ss;                               /*!< SS baudrate                                */
    uint8_t bit_timing_seg1_ss;                         /*!< SS bit timing segment1                     */
    uint8_t bit_timing_seg2_ss;                         /*!< SS bit timing degment2                     */
    uint8_t bit_timing_sjw_ss;                          /*!< SS bit timing synchronization jump width   */

    uint32_t baudrate_fs;                               /*!< FS baudrate                                */
    uint8_t bit_timing_seg1_fs;                         /*!< FS bit timing segment1                     */
    uint8_t bit_timing_seg2_fs;                         /*!< FS bit timing degment2                     */
    uint8_t bit_timing_sjw_fs;                          /*!< FS bit timing synchronization jump width   */

    CAN_RxAlmostFullLimitETypeDef rx_almost_full_limit; /*!< rx buffer almost full warning limit        */
    CAN_ErrWarnLimitETypeDef err_limit;                 /*!< error warning limit                        */
    CAN_AcceptFilCfgTypeDef *accept_fil_cfg_ptr;        /*!< acceptance filter config pointer           */
    uint8_t accept_fil_cfg_num;                         /*!< acceptance filter config number            */

    CAN_UserCallbackTypeDef  user_callback;             /*!< User Callback                              */
} CAN_UserCfgTypeDef;

/**
  * @}
  */


/* Exported Variables --------------------------------------------------------*/
/* Exported Functions --------------------------------------------------------*/
/** @addtogroup CAN_LL_Exported_Functions
  * @{
  */

/** @addtogroup CAN_LL_Exported_Functions_Group1
  * @{
  */
LL_StatusETypeDef LL_CAN_Init(CAN_TypeDef *Instance, CAN_UserCfgTypeDef *user_cfg);
LL_StatusETypeDef LL_CAN_DeInit(CAN_TypeDef *Instance);
void LL_CAN_MspInit(CAN_TypeDef *Instance);
void LL_CAN_MspDeInit(CAN_TypeDef *Instance);
LL_StatusETypeDef LL_CAN_RegisterCallback(CAN_TypeDef *Instance, CAN_UserCallbackIdETypeDef CallbackID, CAN_UserCallback pCallback);
LL_StatusETypeDef LL_CAN_UnRegisterCallback(CAN_TypeDef *Instance, CAN_UserCallbackIdETypeDef CallbackID);
/**
  * @}
  */


/** @addtogroup CAN_LL_Exported_Functions_Group2
  * @{
  */
LL_StatusETypeDef LL_CAN_ResetEnter(CAN_TypeDef *Instance);
LL_StatusETypeDef LL_CAN_ResetExit(CAN_TypeDef *Instance);
LL_StatusETypeDef LL_CAN_AcceptFilCfg(CAN_TypeDef *Instance, CAN_AcceptFilCfgTypeDef *fil_cfg);
uint8_t LL_CAN_DatLen_Get(CAN_TypeDef *Instance, uint8_t dat_len_code);
CAN_DatLenCodeETypeDef LL_CAN_DatLenCode_Get(CAN_TypeDef *Instance, uint8_t dat_len);
/**
  * @}
  */


/** @addtogroup CAN_LL_Exported_Functions_Group3
  * @{
  */
LL_StatusETypeDef LL_CAN_TransmitPTB_CPU(CAN_TypeDef *Instance, CAN_TxBufFormatTypeDef *buf_fmt, uint32_t *buf);
LL_StatusETypeDef LL_CAN_TransmitSTB_CPU(CAN_TypeDef *Instance, CAN_TxBufFormatTypeDef *buf_fmt, uint32_t *buf);
LL_StatusETypeDef LL_CAN_Receive_CPU(CAN_TypeDef *Instance, CAN_RxBufFormatTypeDef *buf_fmt, uint32_t *buf);
LL_StatusETypeDef LL_CAN_TransmitPTB_IT(CAN_TypeDef *Instance, CAN_TxBufFormatTypeDef *buf_fmt, uint32_t *buf);
LL_StatusETypeDef LL_CAN_TransmitSTB_IT(CAN_TypeDef *Instance, CAN_TxBufFormatTypeDef *buf_fmt, uint32_t *buf);
LL_StatusETypeDef LL_CAN_Receive_IT(CAN_TypeDef *Instance, CAN_RxBufFormatTypeDef *buf_fmt, uint32_t *buf);
/**
  * @}
  */


/** @addtogroup CAN_LL_Exported_Functions_Interrupt
  * @{
  */
void LL_CAN_IRQHandler(CAN_TypeDef *Instance);

void LL_CAN_RxCallback(CAN_TypeDef *Instance);
void LL_CAN_RxOverCallback(CAN_TypeDef *Instance);
void LL_CAN_RxFullCallback(CAN_TypeDef *Instance);
void LL_CAN_RxAlmostFullCallback(CAN_TypeDef *Instance);
void LL_CAN_TxPriCallback(CAN_TypeDef *Instance);
void LL_CAN_TxSecCallback(CAN_TypeDef *Instance);
void LL_CAN_ErrCallback(CAN_TypeDef *Instance);
void LL_CAN_AbortCallback(CAN_TypeDef *Instance);

void LL_CAN_ErrPassiveCallback(CAN_TypeDef *Instance);
void LL_CAN_ArbLostCallback(CAN_TypeDef *Instance);
void LL_CAN_BusErrCallback(CAN_TypeDef *Instance);
/**
  * @}
  */

/**
  * @}
  */


/**
  * @}
  */

/**
  * @}
  */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _TAE32G58XX_LL_CAN_H_ */


/************************* (C) COPYRIGHT Tai-Action *****END OF FILE***********/

