//*****************************************************************************
//
//! @file mspi_mspi_display.h
//!
//! @brief Global includes for the mspi_mspi_display example.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef MSPI_MSPI_DISPLAY_H
#define MSPI_MSPI_DISPLAY_H

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

//*****************************************************************************
//
// Example include files.
//
//*****************************************************************************
#include "am_devices_mspi_rm69330.h"
#include "am_devices_mspi_atxp128.h"

//*****************************************************************************
//
// Example Compile time configurations
//
//*****************************************************************************
//#define IMAGE_BMP
//#define ENABLE_LOGGING
// To test with different FB alignment
#define FB_ALIGNMENT_OFFSET    0 // 0 - 3
// To test image sanity with with FPGA, as the refresh rate is too high
#define TE_DELAY               1 // 1
//#define CQ_RAW

#define DISPLAY_MSPI_MODULE    AM_BSP_MSPI_DISPLAY_INST
#define FLASH_MSPI_MODULE      AM_BSP_MSPI_PSRAM_INST

// Size of temporary buffer being used
// Ideally this should be as large as possible for efficiency - within the limits of MSPI and IOM max transaction sizes
// The impact on memory could be either ways - as increasing this increases the temp buf size
// however, at the same time - it reduces the memory required for command queue, as there are less
// number of transactions needed to transfer full frame buffer
#define TEMP_BUFFER_SIZE          4096
// Total number of TEMP_BUFFER_SIZE fragments needs to transfer one full frame buffer
#define NUM_FRAGMENTS             ((FRAME_SIZE + TEMP_BUFFER_SIZE - 1) / TEMP_BUFFER_SIZE)


//#define FLASH_XIP_ENABLE
//*****************************************************************************
//
// Macros
//
//*****************************************************************************
#define DISPLAY_MSPI_IRQn        MSPI_IRQn(DISPLAY_MSPI_MODULE)
#define FLASH_MSPI_IRQn          MSPI_IRQn(FLASH_MSPI_MODULE)

// Display Characteristics
#define ROW_NUM                   AM_DEVICES_RM69330_NUM_ROWS
#define COLUMN_NUM                AM_DEVICES_RM69330_NUM_COLUMNS
#define PIXEL_SIZE                AM_DEVICES_RM69330_PIXEL_SIZE
#define ROW_SIZE                  (COLUMN_NUM * PIXEL_SIZE)
#define FRAME_SIZE                ROW_NUM * ROW_SIZE // Display device is limited to its resolution ratio// Temp Buffer in SRAM to read FLASH data to, and write DISPLAY data from


// FLASH space is divided into following sections for this experiment
// FB_Src - this is where we'll initialize Source Frame Buffer Images - Base1 & Base2
// FB - Active Frame buffers - FB1 & FB2
#define FLASH_BASE              MSPI_XIP_BASEADDRn(FLASH_MSPI_MODULE)
#define FLASH_IMAGE_BASE        FLASH_BASE
#define FLASH_IMAGE_OFFSET      (FLASH_IMAGE_BASE - FLASH_BASE)
#define FLASH_IMAGE1_OFFSET     (FLASH_IMAGE_OFFSET)
#define FLASH_IMAGE2_OFFSET     ( ( (FLASH_IMAGE1_OFFSET + FRAME_SIZE + FLASH_PAGE_SIZE - 1)/FLASH_PAGE_SIZE )* FLASH_PAGE_SIZE)
#define FLASH_PAGE_SIZE         (AM_DEVICES_MSPI_ATXP128_SECTOR_SIZE)

//#define FLASH_SRCFB_BASE      FLASH_BASE
//#define FLASH_SRCFB_OFFSET    (FLASH_SRCFB_BASE - FLASH_BASE)
//#define FLASH_SRCFB1_OFFSET   (FLASH_SRCFB_OFFSET)
//#define FLASH_SRCFB2_OFFSET   (((FLASH_SRCFB1_OFFSET + FRAME_SIZE + FLASH_PAGE_SIZE - 1)/FLASH_PAGE_SIZE)*FLASH_PAGE_SIZE)


//#define FLASH_SRCFB_SIZE      (1024*1024)
//#define FLASH_ACTFB_BASE      (FLASH_SRCFB_SIZE + FLASH_SRCFB_BASE)       //0x10 0000
//#define FLASH_ACTFB_OFFSET    (FLASH_ACTFB_BASE - FLASH_BASE)
//#define FLASH_ACTFB1_OFFSET   (FLASH_ACTFB_OFFSET  + FB_ALIGNMENT_OFFSET)
//#define FLASH_ACTFB2_OFFSET   (((FLASH_ACTFB1_OFFSET + FRAME_SIZE + FLASH_PAGE_SIZE - 1)/FLASH_PAGE_SIZE)*FLASH_PAGE_SIZE  + FB_ALIGNMENT_OFFSET)
//#define FLASH_ACTFB_SIZE      (1024*1024)


//*****************************************************************************
//
// Debug
//
//*****************************************************************************
#define DBG1_GPIO          33
#define DBG2_GPIO          32

#define DEBUG_GPIO_HIGH(gpio)   am_hal_gpio_state_write(gpio, AM_HAL_GPIO_OUTPUT_SET)
#define DEBUG_GPIO_LOW(gpio)    am_hal_gpio_state_write(gpio, AM_HAL_GPIO_OUTPUT_CLEAR)
#define DEBUG_GPIO_TOGGLE(gpio) am_hal_gpio_output_toggle(gpio)

//#define ENABLE_LOGGING

#ifdef ENABLE_LOGGING
#define DEBUG_PRINT am_util_stdio_printf
#else
#define DEBUG_PRINT(...)
#endif
#define DEBUG_PRINT_SUCCESS(...)


//*****************************************************************************
//
// Global externs
//
//*****************************************************************************
// Temp Buffer in SRAM to read FLASH data to, and write DISPLAY data from

extern void                  *g_MSPIDisplayHandle;
extern void                  *g_MSPIFlashHandle;
extern void                  *g_FlashHandle;

extern volatile bool         g_bTEInt;
extern volatile bool         g_bDisplayDone;
extern volatile bool         g_bNewDisplay;


//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
uint32_t display_init(void);
uint32_t init_mspi_mspi_xfer(void);
uint32_t start_mspi_mspi_xfer(uint32_t psramOffset, uint32_t ui32NumBytes);
uint32_t start_mspi_xfer(uint32_t address, uint32_t ui32NumBytes);
uint32_t psram_init(void);
uint32_t psram_data_init(void);



#endif // MSPI_MSPI_DISPLAY_H

