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
#include "am_devices_mspi_rm67162.h"
#include "am_devices_mspi_psram_aps6404l.h"

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
#define CQ_RAW

#define DISPLAY_MSPI_MODULE    AM_BSP_MSPI_DISPLAY_INST
#define PSRAM_MSPI_MODULE      AM_BSP_MSPI_PSRAM_INST

// Size of temporary buffer being used
// Ideally this should be as large as possible for efficiency - within the limits of MSPI and IOM max transaction sizes
// The impact on memory could be either ways - as increasing this increases the temp buf size
// however, at the same time - it reduces the memory required for command queue, as there are less
// number of transactions needed to transfer full frame buffer
#define TEMP_BUFFER_SIZE          4096
// Total number of TEMP_BUFFER_SIZE fragments needs to transfer one full frame buffer
#define NUM_FRAGMENTS             ((FRAME_SIZE + TEMP_BUFFER_SIZE - 1) / TEMP_BUFFER_SIZE)


//*****************************************************************************
//
// Macros
//
//*****************************************************************************
#define DISPLAY_MSPI_IRQn        MSPI_IRQn(DISPLAY_MSPI_MODULE)
#define PSRAM_MSPI_IRQn          MSPI_IRQn(PSRAM_MSPI_MODULE)

// Display Characteristics
#define ROW_NUM                   AM_DEVICES_RM67162_NUM_ROWS
#define COLUMN_NUM                AM_DEVICES_RM67162_NUM_COLUMNS
#define FRAME_SIZE                ROW_NUM * COLUMN_NUM // Display device is limited to its resolution ratio// Temp Buffer in SRAM to read PSRAM data to, and write DISPLAY data from


// PSRAM space is divided into following sections for this experiment
// FB_Src - this is where we'll initialize Source Frame Buffer Images - Base1 & Base2
// FB - Active Frame buffers - FB1 & FB2
#define PSRAM_BASE            MSPI_XIP_BASEADDRn(PSRAM_MSPI_MODULE)
#define PSRAM_SRCFB_BASE      PSRAM_BASE
#define PSRAM_SRCFB_OFFSET    (PSRAM_SRCFB_BASE - PSRAM_BASE)
#define PSRAM_SRCFB1_OFFSET   (PSRAM_SRCFB_OFFSET)
#define PSRAM_SRCFB2_OFFSET   (((PSRAM_SRCFB1_OFFSET + FRAME_SIZE + PSRAM_PAGE_SIZE - 1)/PSRAM_PAGE_SIZE)*PSRAM_PAGE_SIZE)
#define PSRAM_SRCFB_SIZE      (1024*1024)
#define PSRAM_ACTFB_BASE      (PSRAM_SRCFB_SIZE + PSRAM_SRCFB_BASE)
#define PSRAM_ACTFB_OFFSET    (PSRAM_ACTFB_BASE - PSRAM_BASE)
#define PSRAM_ACTFB1_OFFSET   (PSRAM_ACTFB_OFFSET  + FB_ALIGNMENT_OFFSET)
#define PSRAM_ACTFB2_OFFSET   (((PSRAM_ACTFB1_OFFSET + FRAME_SIZE + PSRAM_PAGE_SIZE - 1)/PSRAM_PAGE_SIZE)*PSRAM_PAGE_SIZE  + FB_ALIGNMENT_OFFSET)
#define PSRAM_ACTFB_SIZE      (1024*1024)
#define PSRAM_PAGE_SIZE       1024

//*****************************************************************************
//
// Debug
//
//*****************************************************************************
#define DBG1_GPIO          33
#define DBG2_GPIO          32

#define DEBUG_GPIO_HIGH(gpio)   am_hal_gpio_state_write(gpio, AM_HAL_GPIO_OUTPUT_SET)
#define DEBUG_GPIO_LOW(gpio)    am_hal_gpio_state_write(gpio, AM_HAL_GPIO_OUTPUT_CLEAR)

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
// Temp Buffer in SRAM to read PSRAM data to, and write DISPLAY data from
extern uint32_t              g_TempBuf[2][TEMP_BUFFER_SIZE / 4];
extern void                  *g_MSPIDisplayHandle;
extern void                  *g_MSPIPsramHandle;
extern void                  *g_PsramHandle;

extern volatile bool         g_bTEInt;
extern volatile bool         g_bDisplayDone;
extern volatile bool         g_bNewDisplay;
extern volatile uint32_t     g_actFb;

extern volatile bool         g_bNewFB;



//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
uint32_t display_init(void);
uint32_t init_mspi_mspi_xfer(void);
uint32_t start_mspi_mspi_xfer(uint32_t psramOffset, uint32_t ui32NumBytes);

uint32_t psram_init(void);
uint32_t psram_data_init(void);
uint32_t compose(void);



#endif // MSPI_MSPI_DISPLAY_H

