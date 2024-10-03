//*****************************************************************************
//
//! @file lvgl_watch_demo.h
//!
//! @brief Global includes for the lvgl_watch_demo.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef TINYUSB_CDC_MSC_FREERTOS_H
#define TINYUSB_CDC_MSC_FREERTOS_H

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
// LVGL header file.
//
//*****************************************************************************
#include "board.h"
#include "tusb.h"

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
#include "usb_task.h"
#include "rtos.h"

//*****************************************************************************
//
// ISR priority definition.
//
//*****************************************************************************
#define USB_ISR_PRIORITY                (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY)

#endif // TINYUSB_CDC_MSC_FREERTOS_H
