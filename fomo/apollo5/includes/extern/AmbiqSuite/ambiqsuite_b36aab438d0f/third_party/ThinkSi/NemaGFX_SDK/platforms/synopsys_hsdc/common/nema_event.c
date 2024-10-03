/* TSI 2023.xmo */
/*******************************************************************************
 * Copyright (c) 2023 Think Silicon Single Member PC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this header file and/or associated documentation files to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Materials, and to permit persons to whom the Materials are furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * NEMAGFX API. THE UNMODIFIED, NORMATIVE VERSIONS OF THINK-SILICON NEMAGFX
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT:
 *   https://think-silicon.com/products/software/nemagfx-api
 *
 *  The software is provided 'as is', without warranty of any kind, express or
 *  implied, including but not limited to the warranties of merchantability,
 *  fitness for a particular purpose and noninfringement. In no event shall
 *  Think Silicon Single Member PC be liable for any claim, damages or other
 *  liability, whether in an action of contract, tort or otherwise, arising
 *  from, out of or in connection with the software or the use or other dealings
 *  in the software.
 ******************************************************************************/

#include <stdint.h>
// #include <stdio.h>
#include <string.h>
#include "board.h"

#include "nema_dc_hal.h"
#include "nema_dc.h"
#include "nema_event.h"
#include "nema_utils.h"

// #define USE_FT5X06_TOUCH
// #define NO_TIMER1

#ifdef USE_FT5X06_TOUCH
#include "ft5x06_touch.h"
#endif

static int mouse_x = 0;
static int mouse_y = 0;
static int middle_clicked = 0;

static int maxx = 1000;
static int maxy = 1000;

#define CLAMPX( x )  ( (x) < 0 ? 0 : (x) >= maxx ? maxx : (x) );
#define CLAMPY( y )  ( (y) < 0 ? 0 : (y) >= maxy ? maxy : (y) );

#define MAX_EVENTS 10

#define FB_GPU  0
#define FB_FREE 1
#define FB_DC   2

#define NUM_LAYERS 1U

static uintptr_t triple_fbs[NUM_LAYERS][3];
static unsigned nemadc_needs_swap  = 0;
static unsigned nemadc_needs_swap1 = 0;
static int do_double_buffering = 0;

uintptr_t
nema_init_triple_fb(int layer, uintptr_t fb0_phys, uintptr_t fb1_phys, uintptr_t fb2_phys)
{
    triple_fbs[layer][FB_GPU]  = fb0_phys;
    triple_fbs[layer][FB_DC]   = fb1_phys;
    triple_fbs[layer][FB_FREE] = fb2_phys;

    if (fb2_phys == 0U) {
        do_double_buffering = 1;
    }

    return triple_fbs[layer][FB_GPU];
}

void
nemadc_swap_fb(int layer)
{
    //swap free and dc FB
    //should NOT be interrupted by a vsync...
    uintptr_t tmp = triple_fbs[layer][FB_FREE];
    triple_fbs[layer][FB_FREE] = triple_fbs[layer][FB_DC];
    triple_fbs[layer][FB_DC]  = tmp;
}

uintptr_t
nema_swap_fb(int layer)
{
    if (!do_double_buffering) {
        if (nemadc_needs_swap != 0U) {
            for (unsigned i = 0; i < NUM_LAYERS; ++i) {
                if ( (nemadc_needs_swap & (1U<<i)) != 0U ) {
                    nemadc_swap_fb(i);
                }
            }

            nemadc_needs_swap = 0U;
        }

        //swap free and gpu FB
        //should NOT be interrupted by a vsync...
        uintptr_t tmp = triple_fbs[layer][FB_FREE];
        triple_fbs[layer][FB_FREE] = triple_fbs[layer][FB_GPU];
        triple_fbs[layer][FB_GPU]  = tmp;

        nemadc_needs_swap1 = 1U << (unsigned)layer;

        nemadc_request_vsync_non_blocking();
    } else {
        uintptr_t tmp = triple_fbs[layer][FB_DC];
        triple_fbs[layer][FB_DC] = triple_fbs[layer][FB_GPU];
        triple_fbs[layer][FB_GPU]  = tmp;

        //nemadc_wait_vsync();
        nemadc_set_layer_addr(layer, triple_fbs[layer][FB_DC]);
    }
    return triple_fbs[layer][FB_GPU];
}

void
nemadc_vsync_handler(int layer)
{
    //quickly set HW layers
    for (unsigned i = 0U; i < NUM_LAYERS; ++i) {
        if ( (nemadc_needs_swap1 & (1U<<i)) != 0U ) {
            nemadc_set_layer_addr(i, triple_fbs[i][FB_FREE]);
//            nemadc_swap_fb(i);
        }
    }

    nemadc_needs_swap = nemadc_needs_swap1;
    nemadc_needs_swap1 = 0;
}

int block_on_event = 0;
int hide_cursor    = 0;

int timer1_is_initialized = 0;

volatile int timer1_passed = 0;

void timer1_handler (void)
{
    //Not allowed, DBG_print() is also working with interrupt, not possible to have
    //uart higher priority compared to timer (no nesting)
    //DBG_print("@@ Timer1\n\r");
    timer1_passed++;
}

int
nema_event_init(int flags, int mouse_init_x, int mouse_init_y, int mouse_max_x, int mouse_max_y)
{
    block_on_event = (flags & NEMA_EVENT_BLOCK) != 0;
    hide_cursor    = (flags & NEMA_EVENT_HIDE_CURSOR) != 0;

    timer1_is_initialized = 0;

    int err = subs_timer_open(1);
    if (err != 0) {
        DBG_print("** error on Timer1 init **\r\n");
    }

    //set timer callback
    // subs_timer_ioctl_t   timer_params;
    // timer_params.cmd  = SUBS_TIMER_SET_CB;
    // timer_params.arg1 = 0;
    // timer_params.arg2 = (uint32_t)timer1_handler;
    // subs_timer_ioctl(1, &timer_params);

    maxx    = mouse_max_x;
    maxy    = mouse_max_y;
    mouse_x = mouse_init_x;
    mouse_y = mouse_init_y;

    middle_clicked = 0;

    return 0;
}

int nema_timer_create(void)
{
    //we have only one timer
    if ( timer1_is_initialized != 0 ) {
        return -1;
    }

    timer1_is_initialized = 1;

    return 1;
}

int nema_timer_set_oneshot(int fd, uint32_t timeout_milisecs)
{
    //Only TIMER_1 is available
    if (fd != TIMER_1) {
        return -1;
    }

    // subs_timer_ioctl_t   timer_params;

    /**
    * Init timer0 for 1 second
    */
    float secs = (float)timeout_milisecs/1000.f;
    timer_config.timer_number  = 0;
    timer_config.timer_tick    = board_getCoreFreq()*secs;   //After x clk's a callback
    timer_config.timer_callback= timer0_callback;            //callback function
    board_timerStart(&timer_config);

    return fd;
}

int nema_timer_set_periodic(int fd, uint32_t timeout_milisecs)
{
    //Only TIMER_1 is available
    if (fd != TIMER_1) {
        return -1;
    }

    subs_timer_ioctl_t   timer_params;

    timer_params.cmd  = SUBS_TIMER_SET_INTERVAL;
    timer_params.arg1 = 1; //periodic
    timer_params.arg2 = 0.001f*CLK_CPU*timeout_milisecs;

    subs_timer_ioctl(1, &timer_params);

    timer_params.cmd  = SUBS_TIMER_SET_ENABLE;
    timer_params.arg1 = 1; //enable
    timer_params.arg2 = 0;
    subs_timer_ioctl(1, &timer_params);
    int_enable(INTNO_TIMER1);
    return fd;
}


void nema_timer_stop(int fd)
{
    //Only TIMER_1 is available
    if (fd != TIMER_1) {
        return;
    }

    // subs_timer_ioctl_t   timer_params;

    // subs_timer_ioctl(1, &timer_params);

    // timer_params.cmd  = SUBS_TIMER_SET_ENABLE;
    // timer_params.arg1 = 0;
    // timer_params.arg2 = 0;
    // subs_timer_ioctl(1, &timer_params);
    timer_stop(fd);

//    int_disable(INTNO_TIMER1);
}

void
nema_event_wait(nema_event_t *event)
{
    int ready = 0;

    //Init event fields (to be updated later)
    event->mouse_event       = MOUSE_EVENT_NONE;
    event->mouse_state       = MOUSE_STATE_NONE;
    event->mouse_dx          = 0;
    event->mouse_dy          = 0;
    event->mouse_x           = 0;
    event->mouse_y           = 0;
    event->kb_event          = KB_EVENT_NONE;
    event->kb_key            = 0;
    event->timer_id          = 0;
    event->timer_expirations = 0;

    do {

    #ifdef USE_FT5X06_TOUCH
        static touch_event_t touch_event;

        //Read data from touchscreen
        if ( get_touch(&touch_event) > 0 ) {
            event->mouse_x    = touch_event.x;
            event->mouse_y    = touch_event.y;

            if ( touch_event.event == TOUCH_EVENT_DRAG ) {
                event->mouse_event = MOUSE_EVENT_NONE;
                event->mouse_state = MOUSE_STATE_LEFT_CLICKED;
                event->mouse_dx    = -touch_event.dx;
                event->mouse_dy    = -touch_event.dy;
            }else if(touch_event.event == TOUCH_EVENT_PRESS){
                event->mouse_event = MOUSE_EVENT_LEFT_CLICK;
            }else if(touch_event.event == TOUCH_EVENT_RELEASE){
                event->mouse_event = MOUSE_EVENT_LEFT_RELEASE;
            } else {

            }

            if ( touch_event.gesture == TOUCH_GESTURE_ZOOMIN ){
                event->mouse_event = MOUSE_EVENT_SCROLL_UP;
            }else if ( touch_event.gesture == TOUCH_GESTURE_ZOOMOUT ){
                event->mouse_event = MOUSE_EVENT_SCROLL_DOWN;
            } else {

            }

            ready++;
        }
    #endif

        if (timer1_passed != 0){
            event->timer_id          = TIMER_1;
            event->timer_expirations = timer1_passed;
            timer1_passed            = 0;
            ++ready;
        }

        // If no event, enter sleep mode
        if(block_on_event && ready <= 0) {
            _arc_sleep(0);
        }

    } while (block_on_event && ready <= 0);

    return;
}
