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

/** --------------------------------- Header File Inclusion */
#include "BT_common.h"


/* ---------------------------------- Global Definitions */
#if defined BT_SPI
	#define hci_transport_write_data    hci_spi_send_data
#endif /* BT_UART && !BT_BCSP */

/* ---------------------------------- Function Declarations */

/** HCI SPI Initialization & Shutdown */
void hci_spi_init (void);
void hci_spi_bt_init (bool bColdBoot);
void hci_spi_bt_shutdown (void);

/* HCI SPI Send Data */
API_RESULT hci_spi_send_data (UCHAR, UCHAR *, UINT16, UCHAR);

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


bool HciVscSetRfPowerLevelEx(txPowerLevel_t txPowerlevel);
#define HciVsA3_SetRfPowerLevelEx HciVscSetRfPowerLevelEx
void HciVscSetBDAddr(uint8_t *bd_addr);
#define HciVsA3_SetBDAddr HciVscSetBDAddr
void HciDrvBleSleepSet(bool enable);

#ifdef __cplusplus
}
#endif

#endif // HCI_DRV_APOLLO3_H
