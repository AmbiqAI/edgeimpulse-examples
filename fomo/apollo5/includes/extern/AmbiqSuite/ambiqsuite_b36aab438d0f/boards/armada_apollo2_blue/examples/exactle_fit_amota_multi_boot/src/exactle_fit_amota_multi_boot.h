//*****************************************************************************
//
//! @file multi_boot.h
//!
//! @brief Bootloader program accepting multiple host protocols.
//!
//! This is a bootloader program that supports flash programming over UART,
//! SPI, and I2C. The correct protocol is selected automatically at boot time.
//!
//! SWO is configured in 1M baud, 8-n-1 mode.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef MULTI_BOOT_H
#define MULTI_BOOT_H

#include "am_bootloader.h"

//*****************************************************************************
//
// Message buffers.
//
// Note: The RX buffer needs to be 32-bit aligned to be compatible with the
// flash helper functions, but we also need an 8-bit pointer to it for copying
// data from the IOS interface, which is only 8 bits wide.
//
//*****************************************************************************
extern uint8_t g_pui8TxBuffer[8];
extern uint32_t g_pui32RxBuffer[AM_HAL_FLASH_PAGE_SIZE / 4];
extern uint8_t *g_pui8RxBuffer;
extern uint32_t g_ui32BytesInBuffer;

//*****************************************************************************
//
// Globals to keep track of the image write state.
//
//*****************************************************************************
extern uint32_t *g_pui32WriteAddress;
extern uint32_t g_ui32BytesReceived;

extern volatile bool g_bImageValid;

extern uint32_t g_ui32CRC;

//*****************************************************************************
//
// Image structure to hold data about the downloaded boot image.
//
//*****************************************************************************
extern am_bl_storage_image_t g_sImage;

#endif // MULTI_BOOT_H
