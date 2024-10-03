//*****************************************************************************
//
//! @file freertos_sample.h
//!
//! @brief freertos sample definitions.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef FREERTOS_SAMPLE_H
#define FREERTOS_SAMPLE_H

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

//*****************************************************************************
//
// Task include files.
//
//*****************************************************************************
#include "rtos.h"
#include "hello_task.h"
#include "timer_task.h"

//*****************************************************************************
//
// Preprocessor Definitions and Global Variables
//
//*****************************************************************************
// Toggle these GPIO to indicate task entry
#define GPIO_HELLO_TASK     74   // Turbo FMC GPIO0
#define GPIO_IDLE_TASK      75   // Turbo FMC GPIO1
#define GPIO_SLEEP_ENTRY    76  // Turbo FMC GPIO2
#define GPIO_SLEEP_EXIT     77   // Turbo FMC GPIO3
#define GPIO_TIMER_ISR      78   // Turbo FMC GPIO4

#define HELLO_TASK_COUNT    10 // call UnityEnd from ApplicationIdleHook
                               // after this many iterations
#define TIMER_TASK_COUNT    10

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
#define TIMER_ISR_PRIORITY              NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY

//*****************************************************************************
//
// Function templates
//
//*****************************************************************************
extern void globalTearDown(void);
#endif
