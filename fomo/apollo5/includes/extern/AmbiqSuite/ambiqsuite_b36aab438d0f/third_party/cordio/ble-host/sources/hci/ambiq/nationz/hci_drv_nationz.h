//*****************************************************************************
//
//! @file hci_drv_nationz.h
//!
//! @brief Support functions for the Nationz BTLE radio.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef HCI_DRV_NATIONZ_H
#define HCI_DRV_NATIONZ_H


//*****************************************************************************
//
// Follow BSP settings for the IOM.
//
//*****************************************************************************
#define HCI_DRV_NATIONZ_IOM                     0
#define HCI_DRV_NATIONZ_CS                      1

#define HCI_DRV_NATIONZ_INTERRUPT               AM_HAL_INTERRUPT_IOMASTER0
//*****************************************************************************
//
// NATIONZ vendor specific events
//
//*****************************************************************************


// Tx power level in dBm.
typedef enum
{
  TX_POWER_LEVEL_MINUS_10P0_dBm = 0x3,
  TX_POWER_LEVEL_0P0_dBm = 0x8,
  TX_POWER_LEVEL_PLUS_3P0_dBm = 0xF,
  TX_POWER_LEVEL_INVALID = 0x10,
}txPowerLevel_t;


bool_t HciVsNZ_SetRfPowerLevelEx(txPowerLevel_t txPowerlevel);
bool_t HciVsNZ_SetBDAddr(uint8_t addr[6]);

#endif // HCI_DRV_NATIONZ_H
