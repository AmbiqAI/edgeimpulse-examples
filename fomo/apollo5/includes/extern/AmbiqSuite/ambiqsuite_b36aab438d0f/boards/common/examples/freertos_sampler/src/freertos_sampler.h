//*****************************************************************************
//
//! @file freertos_sampler.h
//!
//! @brief Example showing various freeRTOS features.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef FREERTOS_SAMPLER_H
#define FREERTOS_SAMPLER_H

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
#include "am_devices.h"
#include "am_util.h"

//*****************************************************************************
//
// FreeRTOS include files.
//
//*****************************************************************************
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

//*****************************************************************************
//
// Task include files.
//
//*****************************************************************************
#include "app_task.h"
#include "serial_task.h"
#include "itm_task.h"

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define RTOS_TICK_BUTTON0_RELEASED (0x00000001)
#define RTOS_TICK_1_SECOND         (0x00000002)

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern TaskHandle_t xTaskDelayTask;
extern TaskHandle_t xTaskNotifyTest;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
void TaskDelayTask(void *pvParameters);
void TaskNotifyTest(void *pvParameters);
void TaskNotifyTestSetUp(void);


#ifdef __cplusplus
}
#endif

#endif // FREERTOS_SAMPLER_H

