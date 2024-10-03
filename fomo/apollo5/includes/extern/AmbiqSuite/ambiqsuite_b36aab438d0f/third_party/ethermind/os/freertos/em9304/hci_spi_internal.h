//*****************************************************************************
//
//! @file hci_spi_internal.h
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
#ifndef _H_HCI_SPI_INTERNAL_
#define _H_HCI_SPI_INTERNAL_

/* ---------------------------------- Header File Inclusion */
#include "BT_common.h"
#include "BT_api.h"
#include "BT_serial.h"
#include "hci_spi_em9304.h"

/* ---------------------------------- HCI UART Debug */
#ifdef HCI_SPI_DEBUG

#define HCI_SPI_ERR(...)               BT_debug_error(BT_MODULE_ID_TRANSPORT, __VA_ARGS__)
#define HCI_SPI_TRC(...)               BT_debug_trace(BT_MODULE_ID_TRANSPORT, __VA_ARGS__)
#define HCI_SPI_INF(...)               BT_debug_info(BT_MODULE_ID_TRANSPORT, __VA_ARGS__)

#define HCI_SPI_debug_dump_bytes(data, datalen) BT_debug_dump_bytes(BT_MODULE_ID_TRANSPORT, (data), (datalen))

#else /* HCI_SPI_DEBUG */

#define HCI_SPI_ERR                    BT_debug_null
#define HCI_SPI_TRC                    BT_debug_null
#define HCI_SPI_INF                    BT_debug_null

#define HCI_SPI_debug_dump_bytes(data, datalen)

#endif /* HCI_SPI_DEBUG */


/* ---------------------------------- Global Definitions */
#define hci_spi_lock(mutex)            BT_thread_mutex_lock(&mutex)
#define hci_spi_unlock(mutex)          BT_thread_mutex_unlock(&mutex)

#define hci_spi_signal(cond)           BT_thread_cond_signal(&cond)
#define hci_spi_wait(cond, mutex)      BT_thread_cond_wait(&cond,&mutex)


/* ---------------------------------- Internal Functions */


#endif /* _H_HCI_SPI_INTERNAL_ */

