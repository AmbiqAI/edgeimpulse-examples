//*****************************************************************************
//
//! @file hci_drv_em9305.h
//!
//! @brief Support functions for the EM9305 radio.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef HCI_DRV_EM9305_H
#define HCI_DRV_EM9305_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "am_devices_em9305.h"

#define LE_FEATS_LEN                   0x08

#define LL_FEATURES_BYTE0  ( HCI_LE_SUP_FEAT_ENCRYPTION  \
                                 | HCI_LE_SUP_FEAT_CONN_PARAM_REQ_PROC \
                                 | HCI_LE_SUP_FEAT_EXT_REJECT_IND \
                                 | HCI_LE_SUP_FEAT_SLV_INIT_FEAT_EXCH \
                                 | HCI_LE_SUP_FEAT_LE_PING \
                                 | HCI_LE_SUP_FEAT_DATA_LEN_EXT \
                                 | HCI_LE_SUP_FEAT_PRIVACY \
                                 | HCI_LE_SUP_FEAT_EXT_SCAN_FILT_POLICY )

#define LL_FEATURES_BYTE1  ( HCI_LE_SUP_FEAT_LE_2M_PHY \
                             | HCI_LE_SUP_FEAT_LE_EXT_ADV \
                             | HCI_LE_SUP_FEAT_LE_PER_ADV \
                             | HCI_LE_SUP_FEAT_CH_SEL_2 \
                             | HCI_LE_SUP_FEAT_LE_POWER_CLASS_1)

#define LL_FEATURES_BYTE2  ( HCI_LE_SUP_FEAT_MIN_NUN_USED_CHAN)

#define LL_FEATURES_BYTE3  (HCI_LE_SUP_FEAT_PAST_SENDER \
                             | HCI_LE_SUP_FEAT_PAST_RECIPIENT \
                             | HCI_LE_SUP_FEAT_REMOTE_PUB_KEY_VALIDATION )

#define LL_FEATURES_BYTE4  (HCI_LE_SUP_FEAT_POWER_CONTROL_REQUEST \
                             | HCI_LE_SUP_FEAT_POWER_CHANGE_IND \
                             | HCI_LE_SUP_FEAT_REMOTE_PUB_KEY_VALIDATION \
                             | HCI_LE_SUP_FEAT_PATH_LOSS_MONITOR \
                             | HCI_LE_SUP_FEAT_PRR_ADV_ADI\
                             | HCI_LE_SUP_FEAT_SUBRATING \
                             | HCI_LE_SUP_FEAT_CHAN_CLASSIFICATION)

#define LL_FEATURES_BYTE5 (HCI_LE_SUP_FEAT_ADV_CODING_SELECTION \
                             | HCI_LE_SUP_FEAT_PER_ADV_WITH_RESP_SCANNER)

#define TX_POWER_LEVEL_DEFAULT TX_POWER_LEVEL_0P0_dBm

// Tx power level in dBm
typedef enum
{
  TX_POWER_LEVEL_MINUS_20P0_dBm = 0xEC,
  TX_POWER_LEVEL_MINUS_15P0_dBm = 0xF1,
  TX_POWER_LEVEL_MINUS_10P0_dBm = 0xF6,
  TX_POWER_LEVEL_MINUS_5P0_dBm  = 0xFB,
  TX_POWER_LEVEL_0P0_dBm        = 0x0,
  TX_POWER_LEVEL_PLUS_3P0_dBm   = 0x3,
  TX_POWER_LEVEL_PLUS_4P0_dBm   = 0x4,
  TX_POWER_LEVEL_PLUS_6P0_dBm   = 0x6,
  TX_POWER_LEVEL_PLUS_10P0_dBm  = 0xA,

  TX_POWER_LEVEL_INVALID        = 0x15  // Range: -127 to +20
}txPowerLevel_t;

#define MIN_SWITCHING_PATTERN_LEN  (0x02)
#define TEST_LEN_DEFAULT        (0x25)

// For FCC continous wave testing
#define PAYL_CONTINUOUS_WAVE        0x10
// For FCC continuous modulation testing
#define PAYL_CONTINUOUS_MODULATE    0x11

#define CHL_2402_INDEX    0        // low frequency
#define CHL_2440_INDEX    19       // medium frequency
#define CHL_2480_INDEX    39       // high frequency

//*****************************************************************************
//
// Hci driver functions unique to EM9305
//
//*****************************************************************************
bool_t HciVscSetRfPowerLevelEx(txPowerLevel_t txPowerlevel);
void   HciVscConstantTransmission(uint8_t txchannel);
void   HciVscCarrierWaveMode(uint8_t txchannel);
bool_t HciVscSetCustom_BDAddr(uint8_t *bd_addr);
extern void HciVscUpdateBDAddress(void);
extern void HciVscUpdateLinklayerFeature(void);
extern void HciVscDisablePowerCtrl(void);
extern void HciDrvHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg);
extern void HciDrvHandlerInit(wsfHandlerId_t handlerId);
extern void HciDrvIntService(void);

#ifdef __cplusplus
}
#endif

#endif // HCI_DRV_EM9305_H
