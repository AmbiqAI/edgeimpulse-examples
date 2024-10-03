//*****************************************************************************
//
//! @file freertos_mspi_iom_display.h
//!
//! @brief Global includes for the freertos_mspi_iom_display example.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef FREERTOS_MSPI_MSPI_DISPLAY_H
#define FREERTOS_MSPI_MSPI_DISPLAY_H

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
// FreeRTOS include files.
//
//*****************************************************************************
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"
#include "timers.h"
//#include "rtos.h"

//*****************************************************************************
//
// Task include files.
//
//*****************************************************************************
//#include "compose_task.h"
#include "main_task.h"
#include "render_task.h"

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_devices_mspi_rm67162.h"
#include "am_devices_mspi_rm69330.h"
#include "am_devices_mspi_psram_aps6404l.h"
#include "am_util.h"

//*****************************************************************************
// Customize the following for the test
//*****************************************************************************
//#define ENABLE_LOGGING
// To test image sanity with with FPGA, as the refresh rate is too high
#define TE_DELAY               20 // 1

//#define DISPLAY_MSPI_MODULE    AM_BSP_MSPI_DISPLAY_INST
#define DISPLAY_MSPI_MODULE    0
#define PSRAM_MSPI_MODULE      AM_BSP_MSPI_PSRAM_INST

// Whether to set the display render window
// This example always uses full screen, and hence this can be disabled for optimization
//#define CONFIG_DISPLAY_WINDOW

//*****************************************************************************
//
// Macros
//
//*****************************************************************************
#if defined (AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
#define DISPLAY_MSPI_IRQn        MSPI_IRQn(DISPLAY_MSPI_MODULE)
#else
#warning "This example only runs with Apollo4 devices."
#endif

// Display Characteristics
#define ROW_NUM                   AM_DEVICES_RM69330_NUM_ROWS
#define COLUMN_NUM                AM_DEVICES_RM69330_NUM_COLUMNS
#define PIXEL_SIZE                AM_DEVICES_RM69330_PIXEL_SIZE
#define ROW_SIZE                  (COLUMN_NUM * PIXEL_SIZE)
#define FRAME_SIZE                ROW_NUM * ROW_SIZE // Display device is limited to its resolution ratio// Temp Buffer in SRAM to read PSRAM data to, and write DISPLAY data from

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

#define RENDER_EVENT_START_NEW_FRAME        0x1

//*****************************************************************************
//
// Global externs
//
//*****************************************************************************
extern void                  *g_MSPIDisplayHandle;

extern volatile bool         g_bTEInt;
extern volatile bool         g_bDisplayDone;
extern volatile bool         g_bNewDisplay;

extern uint32_t              fb1;
extern uint32_t              fb2;

extern EventGroupHandle_t xMainEventHandle;
extern EventGroupHandle_t xRenderEventHandle;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
uint32_t display_init(void);

#endif // FREERTOS_MSPI_MSPI_DISPLAY_H