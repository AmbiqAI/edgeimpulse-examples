//*****************************************************************************
//
//! @file freertos_stability_test.h
//!
//! @brief Combines AMDTPS and several IOM channels to simulate a customer application
//! with BLE data transfer, Heartrate Monitor (HRM) SPI read, GPS I2C read, external
//! Flash read, and LCD Display write.
//
//! Apollo2_Blue Connections
//! IOM     0    1    2    3    4    5
//! SCK   5,1  8,1  0,5 42,5 39,5 48,5
//! SCK            27,5
//! MISO  6,1  9,1  1,5 43,5 40,5 49,5
//! MISO           25,5
//! MOSI  7,1 10,1  2,5 38,5 44,5 47,5
//! MOSI           28,5
//! CS0  11,1  6,5  3,5 26,7 29,6 24,4
//! CS0  23,1 12,1 26,4      34,6
//! CS0  42,1 35,1 36,5
//!
//! Apollo2 I2C
//! IOM     0    1    2    3    4    5
//! SCL   5,0  8,0 27,4 42,4 39,4 48,4
//! SCL             0,7
//! SDA   6,0  9,0 25,4 43,4 40,4 49,4
//! SDA             1,7
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef FREERTOS_AMDTP_H
#define FREERTOS_AMDTP_H

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
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

//*****************************************************************************
//
// Task include files.
//
//*****************************************************************************
#include "radio_task.h"
#include "flash_task.h"
#include "hrm_task.h"
#include "gps_task.h"
#include "rtc_task.h"

//*****************************************************************************
//
// Stability Test Configuration
//
//*****************************************************************************
#define HRM_LED             0
#define HRM_IOM             0
#define HRM_CTIMER          0
#define HRM_CTIMER_INT      AM_HAL_CTIMER_INT_TIMERA0
#define HRM_TRANSFER_SIZE   200

#define GPS_LED             1
#define GPS_IOM             1
#define GPS_CTIMER          0
#define GPS_CTIMER_INT      AM_HAL_CTIMER_INT_TIMERB0
#define GPS_TRANSFER_SIZE   1024

#define FLASH_LED           2
#define FLASH_IOM           2
#define FLASH_CTIMER        1
#define FLASH_CTIMER_INT    AM_HAL_CTIMER_INT_TIMERA1
#define FLASH_TRANSFER_SIZE 4096

#define LCD_LED             3
#define LCD_IOM             4
#define LCD_CTIMER          1
#define LCD_CTIMER_INT      AM_HAL_CTIMER_INT_TIMERB1
#define LCD_TRANSFER_SIZE   512

// Configure GPIOs for communicating with a SPI fram
static const uint32_t apollo2_iomce0[AM_REG_IOMSTR_NUM_MODULES][2] =
{
#ifdef AM_PART_APOLLO
    {11, AM_HAL_PIN_11_M0nCE0},
    {12, AM_HAL_PIN_12_M1nCE0},
#endif
#ifdef AM_PART_APOLLO2
    {11, AM_HAL_PIN_11_M0nCE0},
    {35, AM_HAL_PIN_35_M1nCE0},
    { 3, AM_HAL_PIN_3_M2nCE0},
    {26, AM_HAL_PIN_26_M3nCE0},
    {29, AM_HAL_PIN_29_M4nCE0},
    {24, AM_HAL_PIN_24_M5nCE0},
#endif
};

//! IOM interrupts.
static const uint32_t iomaster_interrupt[] = {
    AM_HAL_INTERRUPT_IOMASTER0,
    AM_HAL_INTERRUPT_IOMASTER1,
#ifndef AM_PART_APOLLO
    AM_HAL_INTERRUPT_IOMASTER2,
    AM_HAL_INTERRUPT_IOMASTER3,
    AM_HAL_INTERRUPT_IOMASTER4,
    AM_HAL_INTERRUPT_IOMASTER5,
#endif
};
#endif // FREERTOS_AMDTP_H
