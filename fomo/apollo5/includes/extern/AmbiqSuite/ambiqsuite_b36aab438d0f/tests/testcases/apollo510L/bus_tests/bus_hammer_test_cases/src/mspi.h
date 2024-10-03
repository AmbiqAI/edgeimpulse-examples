//*****************************************************************************
//
//! @file mspi.h
//!
//! @brief MSPI setup and functions for the bus hammer test
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef MSPI_H
#define MSPI_H

//*****************************************************************************
//
// Required built-ins.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

//*****************************************************************************
//
// Standard AmbiqSuite includes.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_hal_global.h"
#include "am_util_delay.h"
#include "am_util_stdio.h"
#include "unity.h"

//*****************************************************************************
//
// Devices.
//
//*****************************************************************************
#include "am_devices_mspi_psram_aps25616n.h"
#include "am_devices_mspi_is25wx064.h"
#include "am_devices_mspi_atxp032.h"

#define PSRAM_ISR_PRIORITY              (4)

// Offset the target structure in MSPI devices in XIP space so that we can
// simultaneously use the same MSPI device as an XIP and DMA target.
// Address 0 is used as the DMA target for MSPI
#define MSPI_TARGET_MEM_OFFSET 0x80000
#define MSPI0_START         (MSPI0_APERTURE_START_ADDR + MSPI_TARGET_MEM_OFFSET)
#define MSPI1_START         (MSPI1_APERTURE_START_ADDR + MSPI_TARGET_MEM_OFFSET)
#define MSPI2_START         (MSPI2_APERTURE_START_ADDR + MSPI_TARGET_MEM_OFFSET)
#define MSPI3_START         (MSPI3_APERTURE_START_ADDR + MSPI_TARGET_MEM_OFFSET)

typedef enum
{
    NONE,
    PSRAM_APS25616N,
    FLASH_IS25WX064,
    FLASH_ATXP032
} MSPIDeviceType_t;

extern const MSPIDeviceType_t g_sMSPIDeviceTypes[];
extern void *g_pMSPIDeviceHandle[];
extern volatile float g_fDMAStopTime;
extern volatile bool g_bDMADone;

uint32_t mspi_dma_read(uint8_t source_mspi, uint8_t *rx_buffer, uint32_t read_address, uint32_t num_bytes);
uint32_t mspi_dma_write(uint8_t source_mspi, uint8_t *tx_buffer, uint32_t write_address, uint32_t num_bytes);
void setup_mspi(uint8_t *flash_prep_buffer,
                uint32_t flash_prep_length,
                uint8_t *flash_prep_dma,
                uint32_t flash_prep_dma_length);
void teardown_mspi();

#endif //MSPI_H