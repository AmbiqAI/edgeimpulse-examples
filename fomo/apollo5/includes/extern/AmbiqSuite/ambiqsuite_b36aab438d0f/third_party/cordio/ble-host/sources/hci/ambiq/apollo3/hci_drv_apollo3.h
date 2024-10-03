//*****************************************************************************
//
//! @file hci_drv_apollo3.h
//!
//! @brief Support functions for the Nationz BTLE radio in Apollo3.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef HCI_DRV_APOLLO3_H
#define HCI_DRV_APOLLO3_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// NATIONZ vendor specific events
//
//*****************************************************************************


// Tx power level in dBm.
typedef enum
{
  TX_POWER_LEVEL_MINUS_10P0_dBm = 0x4,
  TX_POWER_LEVEL_MINUS_5P0_dBm = 0x5,
  TX_POWER_LEVEL_0P0_dBm = 0x8,
  TX_POWER_LEVEL_PLUS_3P0_dBm = 0xF,
  TX_POWER_LEVEL_INVALID = 0x10,
}txPowerLevel_t;


bool_t HciVscSetRfPowerLevelEx(txPowerLevel_t txPowerlevel);
#define HciVsA3_SetRfPowerLevelEx HciVscSetRfPowerLevelEx
void   HciVscConstantTransmission(uint8_t txchannel);
#define HciVsA3_ConstantTransmission HciVscConstantTransmission
void   HciVscCarrierWaveMode(uint8_t txchannel);
#define HciVsA3_CarrierWaveMode HciVscCarrierWaveMode
bool_t HciVscSetCustom_BDAddr(uint8_t *bd_addr);
extern void HciVscUpdateBDAddress(void);

//*****************************************************************************
//
// Hci driver functions unique to Apollo3
//
//*****************************************************************************
extern void HciDrvHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg);
extern void HciDrvHandlerInit(wsfHandlerId_t handlerId);
extern void HciDrvIntService(void);

#ifdef __cplusplus
}
#endif

#endif // HCI_DRV_APOLLO3_H
