//*****************************************************************************
//
//! @file Event_task.c
//!
//! @brief Task to handle timer and GPIO interrupt event
//! The task can schedule display task and render task through timer or external
//! interrupt event.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "nemagfx_watchface.h"
#include "rtos.h"
#include "event_task.h"
#include "render_task.h"
#include "display_task.h"

//*****************************************************************************
//
// Event task handle.
//
//*****************************************************************************
TaskHandle_t EventTaskHandle;

static TimerHandle_t TimerHandle = NULL;

//*****************************************************************************
//
// Defined one IOM instance for display touch event
//
//*****************************************************************************
nema_event_t g_event;
bool touch_press   = false;
bool touch_release = false;

static am_devices_tc_tma525_info_t g_sTouchInfo = {0};

//*****************************************************************************
//
// Defined global static functions
//
//*****************************************************************************
static void touch_handler(void *x);

//*****************************************************************************
//
//! @brief FreeRTOS Timer event hander
//!
//! This function handles timeout event
//!
//! @return None.
//
//*****************************************************************************
static void
timer_handler(void)
{
    xSemaphoreGiveFromISR(g_semEventStart, NULL);
}

//*****************************************************************************
//
//! @brief Touch event hander
//!
//! This function handles external event to read touch pointer coordinate.
//!
//! @return None.
//
//*****************************************************************************
static void
touch_handler(void *x)
{
    if (am_devices_tma525_nonblocking_get_point((am_devices_tc_tma525_info_t *)&g_sTouchInfo) == AM_DEVICES_TMA525_STATUS_SUCCESS)
    {
        g_event.mouse_x = g_sTouchInfo.x0;
        g_event.mouse_y = g_sTouchInfo.y0;
        touch_release = g_sTouchInfo.touch_released;
        
        if (touch_release == true)
        {
            touch_press = false;
            xTimerStartFromISR(TimerHandle, 0);
            //xSemaphoreGiveFromISR(g_semEventEnd, NULL);
        }
        else
        {
            touch_press = true;
            xTimerStopFromISR(TimerHandle, 0);
            //am_util_stdio_printf("(x,y) = (%d, %d)\n", g_event.mouse_x, g_event.mouse_y);
        }
        xSemaphoreGiveFromISR(g_semEventStart, NULL);
    }
}

//*****************************************************************************
//
//! @brief Initialize touch feature
//!
//! This function initializes IOM and external interrupt to implement the touch
//! function of the panel.
//!
//! @return None.
//
//*****************************************************************************
static void
touch_init(void)
{
    uint32_t IntNum = AM_BSP_GPIO_TOUCH_INT;
    //
    // Initialize touchpad
    //
    am_devices_tma525_init(AM_BSP_TP_IOM_MODULE, touch_handler, NULL);

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();
}

//*****************************************************************************
//
// Event Task.
//
//*****************************************************************************
void
EventTask(void *pvParameters)
{
    //
    // Initialize the display touch
    //
    touch_init();

    //
    // Create FreeRTOS timer with one second as it periodic.
    //
    TimerHandle = xTimerCreate((const char *)"timer",
                              (TickType_t )configTICK_RATE_HZ,
                              (UBaseType_t )pdTRUE,
                              (void *)1,
                              (TimerCallbackFunction_t)timer_handler);
    if (TimerHandle == NULL)
    {
        //
        // Timer create error.
        //
        while(1);
    }
    else
    {
        //
        // Start Timer.
        //
        xTimerStart(TimerHandle, 0);
    }

    while (1)
    {
        //
        // Wait for one of the events from timeout or external interrupt
        //
        xSemaphoreTake(g_semEventStart, portMAX_DELAY);

    }
}
