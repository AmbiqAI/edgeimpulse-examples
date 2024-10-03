//*****************************************************************************
//
//! @file usb_task.c
//!
//! @brief Task to handle audio related operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdbool.h>

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

#include "acore_thread.h"

#include "am_bsp.h"
#include "am_util_debug.h"
#include "am_util_stdio.h"
#include "am_util_delay.h"

#include "webusb_controller.h"
#include "link_controller.h"

#include "usb_task.h"

#define ENABLE_SYSPLL_FOR_HS ((BOARD_DEVICE_RHPORT_SPEED == OPT_MODE_HIGH_SPEED) && defined(AM_PART_APOLLO5B) && !defined(APOLLO5_FPGA))

/*
 * LOCAL FUNCTION DECLARATIONS
 ******************************************************************************
 */

static void usb_task(void *pvParameters);

//*****************************************************************************
//
// Local variable.
//
//*****************************************************************************
static metal_thread_t usb_task_handle;

void usb_task_setup(void)
{
    #if ENABLE_SYSPLL_FOR_HS
    am_hal_clkmgr_clock_config(AM_HAL_CLKMGR_CLK_ID_SYSPLL, AM_HAL_USB_PHY_CLK_FREQ_HZ, NULL);
    #endif // ENABLE_SYSPLL_FOR_HS

    webusb_init();

    link_controller_init();

    int ret = metal_thread_init(&usb_task_handle, usb_task, "usb task",
                                4 * 1024, 0, 3);
    if (ret)
    {
        am_util_debug_printf("metal_thread_init usb task failure %d\n", ret);

        return;
    }
}

void usb_task_teardown(void)
{
    webusb_deinit();

    metal_thread_suspend(&usb_task_handle);

    am_util_debug_printf("usb task stop\r\n");
}
//*****************************************************************************
//
// Task to handle usb event
//
//*****************************************************************************
static void usb_task(void *pvParameters)
{
    (void)pvParameters;

    am_util_debug_printf("usb task start\r\n");

    while (1)
    {
        // tinyusb device task
        webusb_task();

        vTaskDelay(pdMS_TO_TICKS(4));
    }
}
