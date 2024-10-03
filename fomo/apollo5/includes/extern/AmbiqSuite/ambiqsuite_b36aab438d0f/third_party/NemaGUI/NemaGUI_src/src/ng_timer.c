// -----------------------------------------------------------------------------
// Copyright (c) 2021 Think Silicon S.A.
// Think Silicon S.A. Confidential Proprietary
// -----------------------------------------------------------------------------
//     All Rights reserved - Unpublished -rights reserved under
//         the Copyright laws of the European Union
//
//  This file includes the Confidential information of Think Silicon S.A.
//  The receiver of this Confidential Information shall not disclose
//  it to any third party and shall protect its confidentiality by
//  using the same degree of care, but not less than a reasonable
//  degree of care, as the receiver uses to protect receiver's own
//  Confidential Information. The entire notice must be reproduced on all
//  authorised copies and copies may only be made to the extent permitted
//  by a licensing agreement from Think Silicon S.A..
//
//  The software is provided 'as is', without warranty of any kind, express or
//  implied, including but not limited to the warranties of merchantability,
//  fitness for a particular purpose and noninfringement. In no event shall
//  Think Silicon S.A. be liable for any claim, damages or other liability, whether
//  in an action of contract, tort or otherwise, arising from, out of or in
//  connection with the software or the use or other dealings in the software.
//
//
//                    Think Silicon S.A.
//                    http://www.think-silicon.com
//                    Patras Science Park
//                    Rion Achaias 26504
//                    Greece
// -----------------------------------------------------------------------------

#include "ng_timer.h"
#include "ng_globals.h"
#include "nema_event.h"
#include "nema_utils.h"


static int master_timer   = -1;
static int timer_period   = 16; // default timer peridod (16ms => 60 fps)
static bool is_running    = false;
static float frequency_hz = 1.f/16.f;

int ng_timer_create() {
    if (master_timer < 0 ) {
        master_timer = nema_timer_create();
    }

    frequency_hz = 1000.f/(float)timer_period;

    return master_timer;
}

void ng_timer_set_period(int ms) {
    timer_period = ms;
    frequency_hz = 1000.f/(float)ms;
}

void ng_timer_start() {
    if ( is_running == false ) {
        nema_timer_set_periodic(master_timer, timer_period);
        is_running = true;
    }
}

void ng_timer_stop() {
    nema_timer_stop(master_timer);
    is_running = false;
}

int ng_timer_get(){
    return master_timer;
}

bool ng_timer_is_running() {
    return is_running;
}

void ng_timer_handler(void) {
    bool activate_timer = false;

    for ( int i = 0; i < NG_EVENT_LIST_SIZE; ++i ) {
        ng_event_base_t *event = NG_EVENT_LIST[i];

        if ( ng_event_is_running(event) ){
            //Set timer as trigger (to be handled at execution)
            uint32_t trigger = event->trigger;
            event->trigger   = EV_TRIGGER_TIMER;
            ng_event_handle(event, EV_TRIGGER_TIMER);
            //Reset trigger to its default value
            event->trigger = trigger;
            activate_timer = true;
        }
    }

    if( activate_timer == false ) {
        ng_timer_stop();
    }
}

int ng_timer_get_period() {
    return timer_period;
}

float ng_timer_get_frequency() {
    return frequency_hz;
}