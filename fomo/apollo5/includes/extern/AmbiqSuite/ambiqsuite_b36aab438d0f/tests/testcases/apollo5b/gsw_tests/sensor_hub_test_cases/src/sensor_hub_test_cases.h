
//*****************************************************************************
//
//! @file sensor_hub_test_cases.h
//!
//! @brief Sensor hub test cases definitions.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef SENSOR_HUB_TEST_CASES_H
#define SENSOR_HUB_TEST_CASES_H

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
// LVGL header file.
//
//*****************************************************************************
#include "lvgl.h"

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
#include "message_buffer.h"
#include "stream_buffer.h"

//*****************************************************************************
//
// Task include files.
//
//*****************************************************************************
#include "gui_task.h"
#include "display_task_copy.h"
#include "rtos.h"
#include "process_sensors_task.h"
#include "emmc_task.h"
#include "bts_task.h"

#include "nema_hal.h"
#include "nema_math.h"
#include "nema_core.h"
#include "nema_regs.h"
#include "nema_utils.h"
#include "nema_event.h"
#include "nema_graphics.h"
#include "nema_programHW.h"
#include "nema_error.h"

// Debug GPIOs
#define GPIO_GUI_TASK 95
// 96-101 assigned in display_task_copy.h
#define GPIO_DISPLAY_ON_OFF 102
#define GPIO_SLEEP_WAKE 103
#define GPIO_GUI_TIMER 104
#define GPIO_TICK_ERR 105
#define GPIO_STIMER_ISR 106
#define GPIO_TICK_HANDLER 107
#define GPIO_DELTA_TOO_SMALL 108
#define GPIO_EMMC_TASK 109


#define ENABLE_PRINTF

// Task enables
#define GUI_TASK_ENABLE        1
#define PROCESS_SENSORS_TASK_ENABLE 0
#define EMMC_TASK_ENABLE       1
#define BTS_TASK_ENABLE 1

// How much data to read from Slave before sending to eMMC 
#define EMMC_BLOCK_SIZE 512
#define MSG_BUF_BLOCKS  32
#define MSG_BUF_BYTES   EMMC_BLOCK_SIZE*MSG_BUF_BLOCKS
#define EMMC_XFER_LIM   100 // end the test after this many EMMC transfers

extern MessageBufferHandle_t g_xSensorDataMessageBuffer;

//*****************************************************************************
//
// PSRAM address.
//
//*****************************************************************************
#include "am_devices_mspi_psram_aps25616n.h"
#define MSPI_PSRAM_MODULE               0
#define MSPI_FLASH_MODULE               2 //AM_BSP_MSPI_FLASH_MODULE

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
#define ADC_ISR_PRIORITY                (1)
#define AUDADC_ISR_PRIORITY             (2)
#define PSRAM_ISR_PRIORITY              (NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY + 2)
#define FLASH_ISR_PRIORITY              (NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY + 1)
#define IOM_ISR_PRIORITY                (NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY)
#define USB_ISR_PRIORITY                (NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY)

extern void globalTearDown(void);
#endif
