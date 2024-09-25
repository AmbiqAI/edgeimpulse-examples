/*******************************************************************************
 * Copyright (c) 2022 Think Silicon S.A.
 *
   Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this header file and/or associated documentation files to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Materials, and to permit persons to whom the Materials are furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * NEMAGFX API. THE UNMODIFIED, NORMATIVE VERSIONS OF THINK-SILICON NEMAGFX
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT:
 *   https://think-silicon.com/products/software/nemagfx-api
 *
 *  The software is provided 'as is', without warranty of any kind, express or
 *  implied, including but not limited to the warranties of merchantability,
 *  fitness for a particular purpose and noninfringement. In no event shall
 *  Think Silicon S.A. be liable for any claim, damages or other liability, whether
 *  in an action of contract, tort or otherwise, arising from, out of or in
 *  connection with the software or the use or other dealings in the software.
 ******************************************************************************/

#ifndef BAREMETAL
#include "FreeRTOS.h"
#include "portable.h"
#include "timers.h"
#include "semphr.h"

#include "nema_dc_hal.h"
#include "nema_dc.h"
#include "nema_event.h"

#include "am_bsp.h"
#include "am_util_delay.h"
#include "am_util_stdio.h"
#include "string.h"
#include "am_util_debug.h"
#include "am_devices_tma525.h"

#ifdef USE_TOUCH
static int touch_init();
#endif

#define CLAMPX( x )  ( (x) < 0 ? 0 : (x) >= maxx ? maxx : (x) );
#define CLAMPY( y )  ( (y) < 0 ? 0 : (y) >= maxy ? maxy : (y) );

#define MAX_EVENTS 10

#define FB_GPU  0
#define FB_DC   1
#define FB_FREE 2

#define NUM_LAYERS 1U

static uintptr_t triple_fbs[NUM_LAYERS][3];

uintptr_t
nema_init_triple_fb(int layer, uintptr_t fb0_phys, uintptr_t fb1_phys, uintptr_t fb2_phys)
{
    //actually doing always 2 framebuffers
    //fb2_phys is ignored

    triple_fbs[layer][FB_GPU]  = fb0_phys;
    triple_fbs[layer][FB_DC]   = fb1_phys;
    // triple_fbs[layer][FB_FREE] = fb2_phys;

    return triple_fbs[layer][FB_GPU];
}

uintptr_t
nema_swap_fb(int layer)
{
    if (layer < 0) {
        layer = 0;
    }

    {
        uintptr_t tmp = triple_fbs[layer][FB_DC];
        triple_fbs[layer][FB_DC] = triple_fbs[layer][FB_GPU];
        triple_fbs[layer][FB_GPU]  = tmp;

        // nemadc_wait_vsync();
        nemadc_set_layer_addr(layer, triple_fbs[layer][FB_DC]);
    }
    return triple_fbs[layer][FB_GPU];
}

static int timer1_is_initialized = 0;

static SemaphoreHandle_t xSemaphore = NULL;

#ifdef USE_TOUCH

bool touch_release = false;
bool i2c_read_flag = false;
nema_event_t    g_s_last_event;
nema_event_t    g_s_buf_touch;

void disp_touch_cb(void *x);

void
disp_touch_cb(void *x)
{
    am_devices_tc_tma525_info_t sTouchInfo = {0};
//    if (am_devices_tma525_get_point((uint16_t*)(&g_s_buf_touch.mouse_x),
//                               (uint16_t*)(&g_s_buf_touch.mouse_y),
//                               &touch_release) == AM_DEVICES_TMA525_STATUS_SUCCESS)
    if (am_devices_tma525_get_point((am_devices_tc_tma525_info_t *)&sTouchInfo) == AM_DEVICES_TMA525_STATUS_SUCCESS)
    {
        g_s_buf_touch.mouse_x = sTouchInfo.x0;
        g_s_buf_touch.mouse_y = sTouchInfo.y0;
        touch_release = sTouchInfo.touch_released;
        
        if (touch_release == true)
        {
            i2c_read_flag = false;
        }
        else
        {
            i2c_read_flag = true;
        }
    }
}
static int
touch_init()
{
    am_devices_tma525_init(AM_BSP_TP_IOM_MODULE, disp_touch_cb, NULL);

    return 0;
}
#endif

int
nema_event_init(int flags, int mouse_init_x, int mouse_init_y, int mouse_max_x, int mouse_max_y)
{
    timer1_is_initialized = 0;

    xSemaphore = xSemaphoreCreateMutex();

    if ( xSemaphore == NULL ) {
        return -1;
    }

#ifdef USE_TOUCH
    touch_init();
#endif

    return 0;
}

static void semaphore_take(void) {
    (void)xSemaphoreTake(xSemaphore, portMAX_DELAY);
}

static void semaphore_give(void) {
    (void)xSemaphoreGive( xSemaphore );
}

#define TIMER_1 1
static TimerHandle_t timer1_handle;
static UBaseType_t timer1_reload = pdTRUE;

static volatile uint32_t timer1_passed = 0;

static TaskHandle_t xHandlingTask = (TaskHandle_t) 0;

static void
wake_task(void) {
    if ( xHandlingTask != (TaskHandle_t) 0 )
    {
        (void)xTaskNotify( xHandlingTask,
                            0,
                            eNoAction);
    }
}

static void
timer1_handler(TimerHandle_t pxTimer)
{
    //Not allowed, DBG_print() is also working with interrupt, not possible to have
    //uart higher priority compared to timer (no nesting)
    //DBG_print("@@ Timer1\n\r");

    semaphore_take();
    timer1_passed++;
    semaphore_give();

    wake_task();
}

int
nema_timer_create(void)
{
    //we have only one timer
    if ( timer1_is_initialized != 0) {
        return -1;
    }

    timer1_handle = xTimerCreate(
            "timer1", /* name */
            pdMS_TO_TICKS(100), /* period/time */
            timer1_reload, /* auto reload */
            (void*)1, /* timer ID */
            timer1_handler); /* callback */

    if (timer1_handle==NULL) {
        // am_util_stdio_printf("failed to create timer 1Sec\r\n");
        return -1;
    }

    timer1_is_initialized = 1;

/*     xTimerStart(timer1_handle, 0); */

    return TIMER_1;
}

void
nema_timer_destroy(int timer_id)
{
    //If timer isn't initialized, we've nothing to destroy
    if ( timer1_is_initialized == 0) {
        return;
    }

    //Only TIMER_1 is available
    if (timer_id != TIMER_1) {
        return;
    }

    //stop current timer for safety
    nema_timer_stop(timer_id);
    (void)xTimerDelete(timer1_handle, 0);

    timer1_is_initialized = 0;
}

static int
timer_set(int timer1_id, uint32_t timeout_milisecs, UBaseType_t reload)
{
    if (timer1_is_initialized == 0) {
        return -1;
    }

    //Only TIMER_1 is available
    if (timer1_id != TIMER_1) {
        return -1;
    }

    if ( timer1_reload == reload) {
        //just change the timer period
        if (xTimerChangePeriod(timer1_handle, pdMS_TO_TICKS(timeout_milisecs), 0) != pdPASS) {
            return -1;
        }

        // No need to call xTimerStart()
        // xTimerChangePeriod() will cause the timer to start.
        //
        // if (xTimerStart(timer1_handle, 0) != pdPASS) {
        //     return -1;
        // }
    } else {
        if ( xTimerIsTimerActive(timer1_handle) == pdTRUE ) {
            //Timer is running.
            //Can't change reload mode
            return -2;
        }

        //delete previous timer
        if ( xTimerDelete(timer1_handle, 0) == pdFALSE ) {
            return -3;
        }

        timer1_reload = reload;

        timer1_handle = xTimerCreate(
                "timer1", /* name */
                pdMS_TO_TICKS(timeout_milisecs), /* period/time */
                timer1_reload, /* auto reload */
                (void*)1, /* timer ID */
                timer1_handler); /* callback */

        if (xTimerStart(timer1_handle, 0) != pdPASS) {
            return -4;
        }
    }

    return timer1_id;
}

int
nema_timer_set_oneshot(int timer_id, uint32_t timeout_milisecs)
{
    return timer_set(timer_id, timeout_milisecs, pdFALSE);
}

int
nema_timer_set_periodic(int timer_id, uint32_t timeout_milisecs)
{
    return timer_set(timer_id, timeout_milisecs, pdTRUE);
}

void
nema_timer_start(int timer_id)
{
    if (timer1_is_initialized == 0) {
        return;
    }

    if (timer_id != TIMER_1) {
        return;
    }

    xTimerStart(timer1_handle, 10);
}

void
nema_timer_stop(int timer_id)
{
    if (timer1_is_initialized == 0) {
        return;
    }

    //Only TIMER_1 is available
    if (timer_id != TIMER_1) {
        return;
    }

    if ( xTimerIsTimerActive(timer1_handle) == pdTRUE) {
        (void)xTimerStop(timer1_handle, 0);
    }
}

static void
wait_for_timer(void)
{
    BaseType_t xResult;

    xHandlingTask = xTaskGetCurrentTaskHandle();

    /* If a task is in the Blocked state to wait for a notification when the
       notification arrives then the task immediately exits the Blocked state
       and the notification does not remain pending. If a task was not waiting
       for a notification when a notification arrives then the notification
       will remain pending until the receiving task reads its notification
       value. */

    /* Wait to be notified of an interrupt. */
    xResult = xTaskNotifyWait( 0,    /* Don't clear bits on entry. */
                       0,                  /* Don't clear bits on exit. */
                       NULL,               /* No nitification value */
                       portMAX_DELAY );    /* Block indefinitely. */

    (void)xResult;
}

int
nema_event_wait(nema_event_t *event, int block_until_event)
{
    int i32WaitEvent = 0;
#ifdef USE_TOUCH
    event->mouse_event         = MOUSE_EVENT_NONE;
    //g_s_last_event.mouse_event = MOUSE_EVENT_NONE;
    if(i2c_read_flag)
	{
        //
	    // touch display panel.
        //
        i2c_read_flag = false;
        event->mouse_x     = g_s_buf_touch.mouse_x;
        event->mouse_y     = g_s_buf_touch.mouse_y;

        event->mouse_state = MOUSE_STATE_LEFT_CLICKED;
        // if previous state is not CLICKED, CLICK now
        if (g_s_last_event.mouse_state != event->mouse_state)
        {
            event->mouse_event         = MOUSE_EVENT_LEFT_CLICK;

            // it's a click, not a drag/swipe
            event->mouse_dx = 0;
            event->mouse_dy = 0;
        }
        else
        {
            event->mouse_dx = event->mouse_x - g_s_last_event.mouse_x;
            event->mouse_dy = event->mouse_y - g_s_last_event.mouse_y;
        }
        g_s_last_event.mouse_state = event->mouse_state;
        g_s_last_event.mouse_x     = event->mouse_x;
        g_s_last_event.mouse_y     = event->mouse_y;
        i32WaitEvent = 1;
	}
    else if (touch_release)
    {
        //
        // touch release
        //
        if ( g_s_last_event.mouse_state == MOUSE_STATE_LEFT_CLICKED)
        {
            event->mouse_event         = MOUSE_EVENT_LEFT_RELEASE;
            g_s_last_event.mouse_event = event->mouse_event;
            i32WaitEvent = 1;
        }
        event->mouse_state = MOUSE_STATE_NONE;
        g_s_last_event.mouse_state = event->mouse_state;
        touch_release = false;
    }

    if ( timer1_passed > 0 )
    {
        event->timer_id          = TIMER_1;
        event->timer_expirations = timer1_passed;
        timer1_passed = 0;
        i32WaitEvent = 1;
    }

    return i32WaitEvent;
#else

    do {
        if ( timer1_passed != 0U )
        {
            semaphore_take();
            uint32_t tmp_timer_passed = timer1_passed;
            timer1_passed = 0;
            semaphore_give();

            event->timer_id          = TIMER_1;
            event->timer_expirations = tmp_timer_passed;
            ++i32WaitEvent;
        }
        else
        {
            if ( block_until_event != 0 )
            {
                wait_for_timer();
            }
        }
    } while ( (block_until_event != 0) && (i32WaitEvent <= 0) );

    if (i32WaitEvent > 0)
    {
        return 1;
    }
#endif
    return 0;
}

#endif // BAREMETAL
