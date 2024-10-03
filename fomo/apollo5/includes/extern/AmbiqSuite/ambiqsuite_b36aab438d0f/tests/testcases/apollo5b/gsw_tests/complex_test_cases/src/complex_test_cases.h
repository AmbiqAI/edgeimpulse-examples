
//*****************************************************************************
//
//! @file complex_test_cases.h
//!
//! @brief Customer application-like test case simultaneously exercising eMMC,
//!   XIP MSPI PSRAM, SDIO, graphics, DSPI, crypto, and CPU.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef COMPLEX_TEST_CASES_H
#define COMPLEX_TEST_CASES_H

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
#include "unity.h"

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

#include "nema_hal.h"
#include "nema_math.h"
#include "nema_core.h"
#include "nema_regs.h"
#include "nema_utils.h"
#include "nema_event.h"
#include "nema_graphics.h"
#include "nema_programHW.h"
#include "nema_error.h"
#include "nema_vg.h"
#include "nema_vg_tsvg.h"
#include "nema_vg_font.h"

//*****************************************************************************
//
// Task include files.
//
//*****************************************************************************
#include "rtos.h"
#include "playback_task.h"
#include "crypto_task.h"
#include "file_writer_task.h"
#include "wifi_source_task.h"
#include "gui_task.h"
#include "display_task.h"

//*****************************************************************************
//
// Other include files.
//
//*****************************************************************************


//*****************************************************************************
//
// Preprocessor Definitions and Global Variables
//
//*****************************************************************************
// Toggle these GPIO to indicate task entry
#define GPIO_HELLO_TASK     5    // Turbo FMC GPIO0
#define GPIO_IDLE_TASK      6    // Turbo FMC GPIO1
#define GPIO_SLEEP_ENTRY    14   // Turbo FMC GPIO2
#define GPIO_SLEEP_EXIT     7    // Turbo FMC GPIO3
#define GPIO_TIMER_ISR      4    // Turbo FMC GPIO4

#define HELLO_TASK_COUNT    10   // call UnityEnd from ApplicationIdleHook
                                 // after this many iterations
#define PLAYBACK_NUM_FILES  100  // Exit playback_task after playing this many files
#define FS_SCRUB_NUM_FILES  200  // Scrub the filesystem after writing this many files

#define DECODED_BUFFER_SIZE     1024 // size of decoded buffer / filewriter buffer
#define PLAYBACK_FILE_BUFFER_SIZE   2200    // size of the MP3 file read buffer for the playback task

#define PLAYBACK_FILE_MATCH_PATTERN    "????????.mp3" // Valid MP3 file pattern for playback selection
#define FILE_DIR_TEMP       "/tmp"      // Directory for files being downloaded
#define FILE_DIR_MUSIC      "/music"    // Directory for already downloaded files

// Hardware configuration
#define SDIO_WIFI_INTERFACE     1       // If 1, RS9116 is on SDIO1 and eMMC is on SDIO0
                                        // if 0, RS9116 is on SDIO0 and eMMC is on SDIO1
// Static parameters for this test's SDIO WiFi target

//*****************************************************************************
//
// PSRAM address.
//
//*****************************************************************************
#define MSPI_PSRAM_MODULE               0
#define MSPI_FLASH_MODULE               2 // AM_BSP_MSPI_FLASH_MODULE

#if (MSPI_PSRAM_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS           (MSPI0_APERTURE_START_ADDR)
#elif (MSPI_PSRAM_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS           (MSPI1_APERTURE_START_ADDR)
#elif (MSPI_PSRAM_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS           (MSPI2_APERTURE_START_ADDR)
#endif // #if (MSPI_PSRAM_MODULE == 0)

#if (MSPI_FLASH_MODULE == 0)
#define FLASH_XIP_BASE_ADDRESS          (MSPI0_APERTURE_START_ADDR)
#elif (MSPI_FLASH_MODULE == 1)
#define FLASH_XIP_BASE_ADDRESS          (MSPI1_APERTURE_START_ADDR)
#elif (MSPI_FLASH_MODULE == 2)
#define FLASH_XIP_BASE_ADDRESS          (MSPI2_APERTURE_START_ADDR)
#endif // #if (MSPI_FLASH_MODULE == 0)

#define MSPI_PSRAM_SIZE                 (0x800000)

// MSPI PSRAM XIPMM map

#define DECODED_BUFFER_LEN              (DECODED_BUFFER_SIZE)
#define DECODED_BUFFER_ADDRESS          (MSPI_XIP_BASE_ADDRESS)

#define PLAYBACK_FILE_BUFFER_LEN        (PLAYBACK_FILE_BUFFER_SIZE)
#define PLAYBACK_FILE_BUFFER_ADDRESS    (DECODED_BUFFER_ADDRESS + DECODED_BUFFER_LEN)

#define WIFI_BUFFER_LEN                 (0x10000)
#define WIFI_BUFFER_ADDRESS             (PLAYBACK_FILE_BUFFER_ADDRESS + PLAYBACK_FILE_BUFFER_LEN)

#define GFX_MEM_LEN                     (0x800000)
#define GFX_MEM_ADDRESS                 (WIFI_BUFFER_ADDRESS + WIFI_BUFFER_LEN)

//*****************************************************************************
//
// Task priority definition (https://www.freertos.org/RTOS-task-priority.html)
// - Priorities can be from 0 to configMAX_PRIORITIES, where 0 is lowest priority
// - configMAX_PRIORITIES is defined in FreeRTOSConfig.h
// - Idle task has priority 0
//
//*****************************************************************************
#define TASK_PRIORITY_LOW               (4)
#define TASK_PRIORITY_MIDDLE            (TASK_PRIORITY_LOW + 1)
#define TASK_PRIORITY_HIGH              (TASK_PRIORITY_MIDDLE + 1)
#define TASK_PRIORITY_HIGHEST           (TASK_PRIORITY_MIDDLE + 2)

//*****************************************************************************
//
// ISR priority definition (https://www.freertos.org/RTOS-Cortex-M3-M4.html)
// - FreeRTOS tasks ending in "FromISR" cannot be called from an interrupt whose
//   logical priority is higher than configMAX_SYSCALL_INTERRUPT_PRIORITY
// - configMAX_SYSCALL_INTERRUPT_PRIORITY is defined in FreeRTOSConfig.h
// - For Cortex-M interrupts, the highest priority is 0, which is default for
//   all interrupts.
// - Interrupts which call FreeRTOS tasks ending in "FromISR" must be numerically
//   greater than or equal to NCIV_configMAX_SYSCALL_INTERRUPT_PRIORITY
//
//*****************************************************************************
#define TIMER_ISR_PRIORITY              configMAX_SYSCALL_INTERRUPT_PRIORITY
#define PSRAM_ISR_PRIORITY              (configMAX_SYSCALL_INTERRUPT_PRIORITY + 2)

//*****************************************************************************
//
// Function templates
//
//*****************************************************************************
extern void globalTearDown(void);
#endif // COMPLEX_TEST_CASES_H
