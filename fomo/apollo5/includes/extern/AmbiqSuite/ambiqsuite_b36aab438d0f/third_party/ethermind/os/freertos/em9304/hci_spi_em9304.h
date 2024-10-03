//*****************************************************************************
//
//! @file hci_spi_em9304.h
//!
//! @brief This File contains the routines to Transmit and Receive Data from
//! to the EM9304 via the SPI Transport Layer for the BlueLitE/Ethermind BLE
//! host stack.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef _H_HCI_SPI_EM9304_
#define _H_HCI_SPI_EM9304_

/** --------------------------------- Header File Inclusion */
#include "BT_common.h"


/* ---------------------------------- Global Definitions */
#if defined BT_SPI
	#define hci_transport_write_data    hci_spi_send_data
#endif /* BT_UART && !BT_BCSP */

/* ---------------------------------- Function Declarations */
/** HCI SPI Initialization & Shutdown */
void hci_spi_init (void);
void hci_spi_bt_init (void);
void hci_spi_bt_shutdown (void);

/* HCI SPI Send Data */
API_RESULT hci_spi_send_data (UCHAR, UCHAR *, UINT16, UCHAR);
void HciDrvAssignBDAddress(uint8_t * customer_unique_bd_address);
void HciVsEM_SetBDAddress(void);

#endif /* _H_HCI_SPI_EM9304_ */

