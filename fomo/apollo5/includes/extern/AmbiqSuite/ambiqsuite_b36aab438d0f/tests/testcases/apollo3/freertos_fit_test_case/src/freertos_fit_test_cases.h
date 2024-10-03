//*****************************************************************************
//
//! @file freertos_fit_test_cases.h
//!
//! @brief Global includes for the freertos_fit_test_cases.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef FREERTOS_FIT_TEST_CASE_H
#define FREERTOS_FIT_TEST_CASE_H

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
//#include "rtos.h"

//*****************************************************************************
//
// Task include files.
//
//*****************************************************************************
#include "radio_task.h"
#include "spi_fram_task.h"
#include "i2c_fram_task.h"
#include "burst_mode_task.h"
#include "ios_task.h"

#if FIREBALL_CARD || FIREBALL2_CARD
//
// The Fireball device card multiplexes various devices including each of an SPI
// FRAM. The Fireball device driver controls access to these devices.
// If the Fireball card is not used, FRAM devices can be connected directly
// to appropriate GPIO pins.
//
#include "am_devices_fireball.h"
#endif

typedef enum
{
    BLE_POWER_OFF = 0,
    BLE_POWER_ON,
    BLE_POWER_RESUME
}FREERTOS_FIT_BLE_STATUS;

//extern void *pWidget;
//extern char errStr[128];
extern FREERTOS_FIT_BLE_STATUS eBLE_Power_Status;


#endif // FREERTOS_FIT_TEST_CASE_H

