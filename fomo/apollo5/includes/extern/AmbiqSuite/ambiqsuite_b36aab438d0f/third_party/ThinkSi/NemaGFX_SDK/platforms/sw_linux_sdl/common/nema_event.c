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

// #include <stdio.h>
#include <SDL2/SDL.h>

#include "nema_dc_hal.h"
#include "nema_dc.h"
#include "nema_event.h"

static int mouse_x = 0;
static int mouse_y = 0;
static int middle_clicked = 0;

// static int maxx = 1000;
// static int maxy = 1000;

// #define CLAMPX( x )  ( (x) < 0 ? 0 : (x) >= maxx ? maxx : (x) );
// #define CLAMPY( y )  ( (y) < 0 ? 0 : (y) >= maxy ? maxy : (y) );

#define MAX_EVENTS 10

#define NUM_LAYERS 1

static uintptr_t triple_fbs[NUM_LAYERS][3];
// static int nemadc_needs_swap  = 0;
// static int nemadc_needs_swap1 = 0;
// static int do_double_buffering = 0;

uintptr_t
nema_init_triple_fb(int layer, uintptr_t fb0_phys, uintptr_t fb1_phys, uintptr_t fb2_phys)
{
    triple_fbs[layer][0]  = fb0_phys;
    return triple_fbs[layer][0];
}

uintptr_t
nema_swap_fb(int layer)
{
    nemadc_set_layer_addr(layer, triple_fbs[layer][0]);
    return triple_fbs[layer][0];
}

int
nema_event_init(int flags, int mouse_init_x, int mouse_init_y, int mouse_max_x, int mouse_max_y) {
    // if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
    //     SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init() failed: %s\n", SDL_GetError());
    //     return -1;
    // }

    middle_clicked = 0;

    return 0;
}


/***************************************************************************
 *
 *                              TIMERS
 *
 ***************************************************************************/

typedef struct _nema_timer_t {
    uint8_t used;
    int sdl_timer_id;
} nema_timer_t;

static nema_timer_t timers[MAX_EVENTS] = {0};

int nema_timer_create(void) {
    int timer_id = -1;

    for (timer_id = 0; timer_id < MAX_EVENTS; ++timer_id) {
        if ( timers[timer_id].used == 0U ) {
            timers[timer_id].used = 1U;
            return timer_id + 1;
        }
    }

    return 0;
}

void nema_timer_destroy(int timer_id) {
    --timer_id;

    if ( timer_id >= 0 && timer_id < MAX_EVENTS ) {
        if ( timers[timer_id].sdl_timer_id > 0 ) {
            SDL_RemoveTimer( timers[timer_id].sdl_timer_id );
            timers[timer_id].sdl_timer_id = 0;
        }
        timers[timer_id].used = 0;
    }
}

/* with the same code as before: */
Uint32 oneshot_callback(Uint32 interval, void *param)
{
    SDL_Event event;
    SDL_UserEvent userevent;

    /* In this example, our callback pushes a function
    into the queue */

    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = param;
    // userevent.data1 = &my_function;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return(0);
}

/* with the same code as before: */
Uint32 periodic_callback(Uint32 interval, void *param)
{
    SDL_Event event;
    SDL_UserEvent userevent;

    /* In this example, our callback pushes a function
    into the queue, and causes our callback to be called again at the
    same interval: */

    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = param;
    // userevent.data1 = &my_function;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return(interval);
}

static int _nema_timer_set(int timer_id, uint32_t timeout_milisecs, int oneshot) {
    nema_timer_stop(timer_id);
    int _tid = timer_id-1;

    if (_tid < 0 || _tid >= MAX_EVENTS) {
        return -1;
    }

    if (timers[_tid].used == 0U) {
        return -1;
    }

    int sdl_timer_id = SDL_AddTimer(timeout_milisecs, oneshot ? oneshot_callback : periodic_callback, (void *)(uintptr_t)timer_id);

    if (sdl_timer_id <= 0) {
        return -1;
    }

    timers[_tid].sdl_timer_id = sdl_timer_id;

    return timer_id;
}

int nema_timer_set_oneshot(int timer_id, uint32_t timeout_milisecs) {
    int ret = _nema_timer_set(timer_id, timeout_milisecs, 1);
    return ret;
}

int nema_timer_set_periodic(int timer_id, uint32_t timeout_milisecs) {
    int ret = _nema_timer_set(timer_id, timeout_milisecs, 0);
    return ret;
}


void nema_timer_stop(int timer_id) {
    int _tid = timer_id-1;

    if (_tid < 0 || _tid >= MAX_EVENTS) {
        return;
    }

    if (timers[_tid].used == 0U) {
        return;
    }

    if ( timers[_tid].sdl_timer_id > 0) {
        SDL_RemoveTimer( timers[_tid].sdl_timer_id );
        timers[_tid].sdl_timer_id = 0;
    }
}


/***************************************************************************
 *
 *
 *
 ***************************************************************************/

void
nema_event_force_cursor_xy(int x, int y) {
    mouse_x = x;
    mouse_y = y;
    nemadc_cursor_xy(mouse_x, mouse_y);
}

int
nema_event_wait(nema_event_t *event, int block_until_event) {

    event->kb_event = KB_EVENT_NONE;
    event->mouse_event = MOUSE_EVENT_NONE;
    event->kb_key = 0;

    event->mouse_dx = 0;
    event->mouse_dy = 0;

    event->timer_id = 0;
    event->timer_expirations = 0;

    SDL_Event sdl_event;

    do {
        sdl_event.type = 0;
        if (block_until_event != 0) {
            //return on fail
            if ( SDL_WaitEvent(&sdl_event) == 0 ) {
                return 0;
            }
        } else {
            //return on fail
            if ( SDL_PollEvent(&sdl_event) == 0 ) {
                return 0;
            } else {

            }
        }

        int useful_event = 1;
        switch (sdl_event.type)
        {
            case SDL_QUIT:
                exit(0);
                break;
            case SDL_USEREVENT: {
                /* and now we can call the function we wanted to call in the timer but couldn't because of the multithreading problems */

                event->timer_id = (int)(uintptr_t)sdl_event.user.data1;
                ++event->timer_expirations;
                break;
            }
            case SDL_MOUSEWHEEL: {
                if(sdl_event.wheel.y > 0) { // scroll up
                    event->mouse_event = MOUSE_EVENT_SCROLL_UP;
                }
                else if(sdl_event.wheel.y < 0) { // scroll down
                    event->mouse_event = MOUSE_EVENT_SCROLL_DOWN;
                } else {

                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
                switch (sdl_event.button.button) {
                    default:
                    case SDL_BUTTON_LEFT:
                        event->mouse_event = MOUSE_EVENT_LEFT_CLICK;
                        event->mouse_state |= MOUSE_STATE_LEFT_CLICKED;
                        break;
                    case SDL_BUTTON_RIGHT:
                        event->mouse_event = MOUSE_EVENT_RIGHT_CLICK;
                        event->mouse_state |= MOUSE_STATE_RIGHT_CLICKED;
                        break;
                    case SDL_BUTTON_MIDDLE:
                        event->mouse_event = MOUSE_EVENT_MIDDLE_CLICK;
                        event->mouse_state |= MOUSE_STATE_MIDDLE_CLICKED;
                        break;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                switch (sdl_event.button.button) {
                    default:
                    case SDL_BUTTON_LEFT:
                        event->mouse_event = MOUSE_EVENT_LEFT_RELEASE;
                        event->mouse_state &= ~MOUSE_STATE_LEFT_CLICKED;
                        break;
                    case SDL_BUTTON_RIGHT:
                        event->mouse_event = MOUSE_EVENT_RIGHT_RELEASE;
                        event->mouse_state &= ~MOUSE_STATE_RIGHT_CLICKED;
                        break;
                    case SDL_BUTTON_MIDDLE:
                        event->mouse_event = MOUSE_EVENT_MIDDLE_RELEASE;
                        event->mouse_state &= ~MOUSE_STATE_MIDDLE_CLICKED;
                        break;
                }
                break;

            case SDL_KEYDOWN:
                event->kb_event = KB_EVENT_PRESS;
                event->kb_key   = sdl_event.key.keysym.sym;
                break;

            case SDL_KEYUP:
                event->kb_event = KB_EVENT_RELEASE;
                event->kb_key   = sdl_event.key.keysym.sym;
                break;


            case SDL_MOUSEMOTION:
                event->mouse_dx = sdl_event.motion.xrel; //sdl_event.motion.x - event->mouse_x;
                event->mouse_dy = sdl_event.motion.yrel; //sdl_event.motion.y - event->mouse_y;
                event->mouse_x  = sdl_event.motion.x;
                event->mouse_y  = sdl_event.motion.y;
                break;
            default:
                useful_event = 0;
                break;
        }

        if ( (block_until_event == 0) || (useful_event == 1) ) {
            return 1;
        }
    } while ( 0);

    return 1;
}
