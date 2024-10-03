//*****************************************************************************
//
//! @file audio_test_config.h
//!
//! @brief Global includes for the audio_data_flow_test.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AUDIO_TEST_CONFIG_H
#define AUDIO_TEST_CONFIG_H

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
#ifdef SYSTEM_VIEW
#include "SEGGER_SYSVIEW_FreeRTOS.h"
#endif
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

#ifdef SYSTEM_VIEW
#include "SEGGER_SYSVIEW.h"
#endif

//*****************************************************************************
//
// Mqueue include files.
//
//*****************************************************************************

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <sched.h>
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>


//*****************************************************************************
//
// Task include files.
//
//*****************************************************************************



#include "emmc_task.h"
#include "rtos.h"
#include "uart_task.h"
#include "mspi_task.h"

#if defined(BAREMETAL) && defined(SYSTEM_VIEW)
#error SYSTEM_VIEW cannot be used in baremetal!
#endif

//*****************************************************************************
//
// Task Config.
//
//*****************************************************************************
#define MSPI_TASK_ENABLE       1
#define EMMC_TASK_ENABLE       1
#define UART_TASK_ENABLE       1

// If TASK_LOOP_COUNT is commented the test will run forever.
#define TASK_LOOP_COUNT        50

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

#define MSPI_PSRAM_SIZE                 (0x00800000)

//The 0-0x100000 area is used to hold graphics texture
#define PSRAM_TEXTURE_AREA_OFFSET       (0x0)
//The 0x100000-0x200000 area is used to hold the code.
#define PSRAM_CODE_AREA_OFFSET          (0x100000)
//The 0x200000 started area is used to hold the heap.
#define PSRAM_HEAP_AREA_OFFSET          (0x200000)

//*****************************************************************************
//
// Task priority definition.
//
//*****************************************************************************
#define TASK_PRIORITY_LOW               (4)
#define TASK_PRIORITY_MIDDLE            (TASK_PRIORITY_LOW + 1)
#define TASK_PRIORITY_HIGH              (TASK_PRIORITY_MIDDLE + 1)
#define TASK_PRIORITY_HIGHEST           (TASK_PRIORITY_MIDDLE + 2)

//*****************************************************************************
//
// ISR priority definition.
//
//*****************************************************************************
#define PSRAM_ISR_PRIORITY              (NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY + 2)
#define FLASH_ISR_PRIORITY              (NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY + 1)
#define IOM_ISR_PRIORITY                (NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY)
#define UART_ISR_PRIORITY               (NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY)

// To test PRAM and EMMC+UART 
//#define TEST_PSRAM
//#define TEST_EMMC_UART

#define TEST_NUM                5
#define TEST_CONFIG_NUM         24

extern uint8_t emmcDataSent;
extern uint8_t psramDataSent;


extern void globalTearDown(void);

#endif // AUDIO_TEST_CONFIG_H
