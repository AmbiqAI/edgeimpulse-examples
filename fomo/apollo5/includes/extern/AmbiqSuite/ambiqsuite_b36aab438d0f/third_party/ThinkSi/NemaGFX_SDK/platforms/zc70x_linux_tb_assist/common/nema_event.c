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

#define _XOPEN_SOURCE

// #include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>

#include "nema_dc_hal.h"
#include "nema_dc.h"
#include "nema_event.h"
#include "nema_kb_map.inc"

#define NEMADCFILE  "/dev/nemadc"
#define NEMAFILE    "/dev/nema"

static int max_input_fd = -1;
struct input_event ie;

static int mouse_x = 0;
static int mouse_y = 0;

static int mouse_released_x = 1;
static int mouse_released_y = 1;

static int middle_clicked = 0;

static int maxx = 1000;
static int maxy = 1000;

#define CLAMPX( x )  ( (x) < 0 ? 0 : (x) >= maxx ? maxx : (x) );
#define CLAMPY( y )  ( (y) < 0 ? 0 : (y) >= maxy ? maxy : (y) );

#define MOUSE_LEFT   1272
#define MOUSE_RIGHT  1273
#define MOUSE_MIDDLE 1274
#define MOUSE_SCROLL 2008
#define MOUSE_MOVE_H 2000
#define MOUSE_MOVE_V 2001

#define MOUSE_LEFT_PRESSED    (10000+MOUSE_LEFT)
#define MOUSE_LEFT_RELEASED   (MOUSE_LEFT)
#define MOUSE_RIGHT_PRESSED   (10000+MOUSE_RIGHT)
#define MOUSE_RIGHT_RELEASED  (MOUSE_RIGHT)
#define MOUSE_MIDDLE_PRESSED  (10000+MOUSE_MIDDLE)
#define MOUSE_MIDDLE_RELEASED (MOUSE_MIDDLE)

#define MAX_EVENTS 10

#define FB_GPU  0
#define FB_FREE 1
#define FB_DC   2

int epfd;
int epfd_nema;

int _timer;

struct epoll_event ev;
struct epoll_event evlist[MAX_EVENTS];
struct epoll_event evlist_nema[MAX_EVENTS];

#define NUM_LAYERS 1U

static uintptr_t triple_fbs[NUM_LAYERS][3];
static unsigned nemadc_needs_swap  = 0;
static unsigned nemadc_needs_swap1 = 0;
static bool do_double_buffering = false;
static bool do_single_buffering = false;

uintptr_t
nema_init_triple_fb(int layer, uintptr_t fb0_phys, uintptr_t fb1_phys, uintptr_t fb2_phys)
{
    triple_fbs[layer][FB_GPU]  = fb0_phys;
    triple_fbs[layer][FB_DC]   = fb1_phys;
    triple_fbs[layer][FB_FREE] = fb2_phys;

    if (fb2_phys == 0U) {
        if (fb1_phys == 0U) {
            do_single_buffering = true;
        }
        else {
            do_double_buffering = true;
        }
    }

    return triple_fbs[layer][FB_GPU];
}

static void
nemadc_swap_fb(int layer) {
    //swap free and dc FB
    //should NOT be interrupted by a vsync...
    uintptr_t tmp = triple_fbs[layer][FB_FREE];
    triple_fbs[layer][FB_FREE] = triple_fbs[layer][FB_DC];
    triple_fbs[layer][FB_DC]  = tmp;
}

uintptr_t
nema_swap_fb(int layer)
{
    if (layer < 0) {
        layer = 0;
    }

    // double duffers
    if (do_double_buffering) {
        uintptr_t tmp = triple_fbs[layer][FB_DC];
        triple_fbs[layer][FB_DC] = triple_fbs[layer][FB_GPU];
        triple_fbs[layer][FB_GPU]  = tmp;

        nemadc_wait_vsync();
        nemadc_set_layer_addr(layer, triple_fbs[layer][FB_DC]);
    }
    // single buffer
    else if (do_single_buffering) {
        nemadc_set_layer_addr(layer, triple_fbs[layer][0]);

    }
    // tripple buffer
    else {
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
    }
    return triple_fbs[layer][FB_GPU];
}

void
nemadc_vsync_handler(int layer) {
    //quickly set HW layers
    for (unsigned i = 0; i < NUM_LAYERS; ++i) {
        if ( (nemadc_needs_swap1 & (1U<<i)) != 0U ) {
            nemadc_set_layer_addr(i, triple_fbs[i][FB_FREE]);
//            nemadc_swap_fb(i);
        }
    }

    nemadc_needs_swap = nemadc_needs_swap1;
    nemadc_needs_swap1 = 0;
}

/* init sigaction */
static void init_sigaction(void)
{
    struct sigaction act;

    act.sa_handler = nemadc_vsync_handler;
    act.sa_flags   = 0;
    sigemptyset(&act.sa_mask);

    if (sigaction(SIGUSR1, &act, NULL) < 0) {
        perror("sigaction");
        // exit(0);
    }
}

int hide_cursor    = 0;

int
nema_event_init(int flags, int mouse_init_x, int mouse_init_y, int mouse_max_x, int mouse_max_y) {
    int fd;
    epfd      = epoll_create(20);
    epfd_nema = epoll_create(1);

    ev.events = EPOLLIN;            /* Only interested in input events */

    hide_cursor    = (flags & NEMA_EVENT_HIDE_CURSOR) != 0;
    //-------------------------------------------------------------
    //                     01234567890123456
    char tmp_filename[] = "/dev/input/eventX";
    for (char c = '0'; c <= '9'; ++c) {
        tmp_filename[16] = c;
        printf("Opening %s\n", tmp_filename);
        if ( (fd = open(tmp_filename, O_RDONLY)) == -1 ) {
            continue;
        } else {
            printf("Found input device\n");
            max_input_fd = fd;
            ev.data.fd = fd;
            if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
                perror("epoll_ctl");
                continue;
            }
        }
    }
    //-------------------------------------------------------------
    // if((fd = open(NEMAFILE, O_RDONLY)) == -1) {
    //     perror("opening nema device");
    //     return -1;
    // }

    // ev.data.fd = fd;
    // printf("nema fd: %d\n", fd);

    // if (epoll_ctl(epfd_nema, EPOLL_CTL_ADD, fd, &ev) == -1) {
    //     perror("epoll_ctl nema");
    //     return -1;
    // }
    //-------------------------------------------------------------
    if((fd = open(NEMADCFILE, O_RDWR)) == -1) {
        perror("opening nemadc device");
        return -1;
    }

    printf("dc fd: %d\n", fd);
    //dummy write in order to register for receiving signal on IRQ
    if (write(fd, (void *)0, 1) < 0)
    {
        perror("write to nemadc");
        return -1;
    }
    //-------------------------------------------------------------

    init_sigaction();

    _timer = nema_timer_create();

    maxx    = mouse_max_x;
    maxy    = mouse_max_y;
    mouse_x = mouse_init_x;
    mouse_y = mouse_init_y;

    nemadc_cursor_enable( hide_cursor == 0 );
    nemadc_cursor_xy(mouse_x, mouse_y);

    middle_clicked = 0;

    return 0;
}

int nema_timer_create(void) {
    /* create new timer */
    int fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (fd <= 0) {
        printf("Failed to create timer\n");
    }

    return fd;
}

int nema_timer_unset(int fd) {
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev) == -1) {
        perror("epoll_ctl");
        return -1;
    }

    return fd;
}

int nema_timer_set_oneshot(int fd, uint32_t timeout_milisecs) {
    if (fd <= 0) {
        return fd;
    }

    struct itimerspec timeout;

    uint32_t sec  = timeout_milisecs/1000U;
    uint32_t nsec = (timeout_milisecs%1000U)*1000U*1000U;

    /* set timeout */
    timeout.it_value.tv_sec     = sec;
    timeout.it_value.tv_nsec    = nsec;
    timeout.it_interval.tv_sec  = 0; /* recurring */
    timeout.it_interval.tv_nsec = 0;

    int ret = timerfd_settime(fd, 0, &timeout, NULL);
    if (ret != 0) {
        printf("Failed to set timer duration\n");
        return -1;
    }

    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;            /* Only interested in input events */
    ev.data.fd = fd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("epoll_ctl");
        return -1;
    }

    return fd;
}

int nema_timer_set_periodic(int fd, uint32_t timeout_milisecs) {
    if (fd <= 0) {
        return fd;
    }

    struct itimerspec timeout;

    uint32_t sec  = timeout_milisecs/1000U;
    uint32_t nsec = (timeout_milisecs%1000U)*1000U*1000U;

    /* set timeout */
    timeout.it_value.tv_sec     = sec;
    timeout.it_value.tv_nsec    = nsec;
    timeout.it_interval.tv_sec  = sec; /* recurring */
    timeout.it_interval.tv_nsec = nsec;

    int ret = timerfd_settime(fd, 0, &timeout, NULL);
    if (ret != 0) {
        printf("Failed to set timer duration\n");
        return -1;
    }

    ev.events = EPOLLIN | EPOLLET;            /* Only interested in input events */
    ev.data.fd = fd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("epoll_ctl");
        return -1;
    }

    return fd;
}


void nema_timer_stop(int fd) {
    if (fd < 0) {
        return;
    }

    struct itimerspec timeout;

    /* set timeout */
    timeout.it_value.tv_sec     = 0;
    timeout.it_value.tv_nsec    = 0;

    timerfd_settime(fd, 0, &timeout, NULL);

    ev.events = EPOLLIN | EPOLLET;            /* Only interested in input events */
    ev.data.fd = fd;

    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev) == -1) {
        perror("epoll_ctl");;
    }
}

void
nema_event_force_cursor_xy(int x, int y) {
    mouse_x = x;
    mouse_y = y;
    nemadc_cursor_xy(mouse_x, mouse_y);
}

int
nema_event_wait(nema_event_t *event, int block_until_event) {
    int ready = 0;

    int useful_event = 0;


    do {
        event->kb_event    = KB_EVENT_NONE;
        event->mouse_event = MOUSE_EVENT_NONE;
        event->kb_key = 0;

        event->mouse_dx = 0;
        event->mouse_dy = 0;

        event->timer_id = 0;
        event->timer_expirations = 0;

        do {
            /* wait  for  an I/O event on an epoll file descriptor */
            ready = epoll_wait(epfd, evlist, MAX_EVENTS, block_until_event ? -1 : 0);

            if (!block_until_event && ready <= 0) {
                return 0;
            }
        } while (ready <= 0);

        event->kb_event = KB_EVENT_NONE;
        event->mouse_event = MOUSE_EVENT_NONE;
        event->kb_key = 0;

        event->mouse_dx = 0;
        event->mouse_dy = 0;

        event->timer_id = 0;
        event->timer_expirations = 0;

        int cur_ev = -1;
        int timer_event = 0;

        nema_timer_set_oneshot(_timer, 16);

        do {
            --ready;
            ++cur_ev;

            if (evlist[cur_ev].data.fd <= max_input_fd) {
                do { } while(read(evlist[cur_ev].data.fd, &ie, sizeof(struct input_event)) < 0 );
                // printf("type: %d code: %d value: %d\n", ie.type, ie.code, ie.value);

                if (ie.type == EV_KEY && ie.code < 128) {
                    //keyboard input
                    event->kb_key   = nema_kb_map(ie.code, ie.value);
                    event->kb_event = ie.value == 2 ? KB_EVENT_HOLD    :
                                      ie.value == 1 ? KB_EVENT_PRESS   :
                                      ie.value == 0 ? KB_EVENT_RELEASE :
                                                      KB_EVENT_NONE;

                    if (event->kb_key != 0) {
                        useful_event += 1;
                    }
                }
                else if (ie.type == 1 || ie.type == 2 || ie.type == 3 || ie.type == 0) {
//                    printf("\tm: %d\n", evlist[cur_ev].data.fd);
                    int tmp_useful_event = 1;
                    int urgent_event = 0;

                    int prev_mouse_x = mouse_x;
                    int prev_mouse_y = mouse_y;

                    unsigned h = ie.type * 1000 + ie.code;
                    switch (h) {
                        // EV_ABS | X
                        case 3000:
                            mouse_x = (float)ie.value*1024.f/4096.f;
                            if (mouse_x < 0.f || mouse_x >= 800.f) {
                                break;
                            }

                            event->mouse_dx += mouse_x - prev_mouse_x;

                            if ( mouse_released_x != 0 ) {
                                event->mouse_dx = 0;
                                mouse_released_x = 0;
                            }

                            nemadc_cursor_enable( 0 );

                            event->mouse_x = mouse_x;

                            nemadc_cursor_xy(mouse_x, mouse_y);
                            break;
                        // EV_ABS | Y
                        case 3001:
                            mouse_y = ie.value*600.f/4096.f;

                            event->mouse_dy += mouse_y - prev_mouse_y;

                            nemadc_cursor_enable( 0 );
                            if ( mouse_released_y != 0 ) {
                                event->mouse_dy = 0;
                                mouse_released_y = 0;
                            }

                            event->mouse_y = mouse_y;

                            nemadc_cursor_xy(mouse_x, mouse_y);
                            break;
                        case MOUSE_MOVE_H:
                            mouse_x = CLAMPX( mouse_x + ie.value );
                            nemadc_cursor_enable( hide_cursor == 0 );
                            if (hide_cursor != 0) {
                                event->mouse_dx += ie.value;
                            }
                            else {
                                event->mouse_dx += mouse_x - prev_mouse_x;
                            }
                            event->mouse_x = mouse_x;

                            nemadc_cursor_xy(mouse_x, mouse_y);
                            break;
                        case MOUSE_MOVE_V:
                            mouse_y = CLAMPY( mouse_y + ie.value );
                            nemadc_cursor_enable( hide_cursor == 0 );
                            if (hide_cursor != 0) {
                                event->mouse_dy += ie.value;
                            }
                            else {
                                event->mouse_dy += mouse_y - prev_mouse_y;
                            }
                            event->mouse_y = mouse_y;

                            nemadc_cursor_xy(mouse_x, mouse_y);
                            break;
                        case MOUSE_LEFT:
                            urgent_event = 1;
                        case 1330:
                            if (ie.value == 1) {
                                event->mouse_event  =  MOUSE_EVENT_LEFT_CLICK;
                                event->mouse_state |=  MOUSE_STATE_LEFT_CLICKED;
                            } else {
                                event->mouse_event  =  MOUSE_EVENT_LEFT_RELEASE;
                                event->mouse_state &= ~MOUSE_STATE_LEFT_CLICKED;
                            }
                            mouse_released_x = 1;
                            mouse_released_y = 1;
                            break;
                        case MOUSE_RIGHT:
                            if (ie.value == 1) {
                                event->mouse_event  =  MOUSE_EVENT_RIGHT_CLICK;
                                event->mouse_state |=  MOUSE_STATE_RIGHT_CLICKED;
                            } else {
                                event->mouse_event  =  MOUSE_EVENT_RIGHT_RELEASE;
                                event->mouse_state &= ~MOUSE_STATE_RIGHT_CLICKED;

                            }
                            urgent_event = 1;
                            break;
                        case MOUSE_MIDDLE:
                            if (ie.value == 1) {
                                event->mouse_event  =  MOUSE_EVENT_MIDDLE_CLICK;
                                event->mouse_state |=  MOUSE_STATE_MIDDLE_CLICKED;
                            } else {
                                event->mouse_event  =  MOUSE_EVENT_MIDDLE_RELEASE;
                                event->mouse_state &= ~MOUSE_STATE_MIDDLE_CLICKED;

                            }
                            urgent_event = 1;
                            break;
                        case MOUSE_SCROLL:
                            if (ie.value == 1) {
                                event->mouse_event  =  MOUSE_EVENT_SCROLL_UP;
                            }
                            else {
                                event->mouse_event  =  MOUSE_EVENT_SCROLL_DOWN;
                            }
                            urgent_event = 1;
                            break;
                        default:
                            tmp_useful_event = 0;
                            break;
                    }
                    useful_event += tmp_useful_event;

                    if ( !timer_event && !urgent_event &&
                        ready <= 0) {
                        /* wait  for  an I/O event on an epoll file descriptor */
                        ready = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
                        cur_ev = -1;
                    }
                }
                else {

                }
            }
            else {
//                printf("\tt: %d\n", evlist[cur_ev].data.fd);
                if (evlist[cur_ev].data.fd != _timer) {
                    //timers
                    uint64_t value;
                    do { } while( read(evlist[cur_ev].data.fd, &value, 8) < 0 );
                    event->timer_id = evlist[cur_ev].data.fd;
                    event->timer_expirations += value;
                    ++useful_event;

                    //TODO: verify whether the followng statement should be used
                    if ( !timer_event && ready <= 0) {
                        /* wait  for  an I/O event on an epoll file descriptor */
                        ready = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
                        cur_ev = -1;
                   }
                } else {
                    timer_event = 1;
                }
            }

        } while (ready > 0);
        nema_timer_unset(_timer);
    } while (!useful_event);

    return 1;
}
