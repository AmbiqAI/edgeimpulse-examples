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

#include "ng_event_transition.h"
#include "ng_timer.h"
#include "ng_globals.h"

inline static void set_start_time_from_progress(ng_transition_t *transition) {
    float progress = transition->progress;

    if ( (transition->status & EV_STATUS_REVERSED) != 0 ) {
        progress = 1.f-progress;
    }

    transition->start_time = NG_WALL_TIME - transition->duration*progress;
}

inline static void update_progress(ng_event_base_t *event) {
    ng_transition_t *transition = NG_TRANSITION(event);
    float progress = (NG_WALL_TIME - transition->start_time)/transition->duration;
    if ( progress > 1.f ) {
        progress = 1.f;
    }

    if ( (event->status & EV_STATUS_REVERSED) != 0 ) {
        progress = 1.f-progress;
    }

    transition->progress = progress;
}

inline static float get_terminal_progress( ng_event_base_t *event ) {
    return ng_event_check_status_flag(event, EV_STATUS_REVERSED) ? 0.f : 1.f;
}

inline static float get_initial_progress( ng_transition_t *transition ) {
    return ng_event_check_status_flag(NG_EVENT(transition), EV_STATUS_REVERSED) ? 1.f : 0.f;
}

inline static void reset(ng_transition_t *transition) {
    ng_event_unset_status_flag(NG_EVENT(transition), EV_STATUS_REVERSED);
    transition->progress = get_initial_progress(transition);
    set_start_time_from_progress(transition);
    ng_event_run_callback(NG_EVENT(transition), 0);
    ng_event_set_status(NG_EVENT(transition) ,EV_STATUS_STOPPED);
}

inline static void finish(ng_event_base_t *event) {
    ng_event_unset_status_flag(event, EV_STATUS_REVERSED);
    ng_event_unset_status_flag(event, EV_STATUS_FIRST_RUN);
    ng_transition_t *transition = NG_TRANSITION(event);
    transition->progress   = get_terminal_progress(event);
    set_start_time_from_progress(transition);
    ng_event_run_callback(event, EV_STATUS_LAST_RUN);
    ng_event_set_status(event, EV_STATUS_STOPPED);
}

void ng_transition_start(ng_event_base_t *event) {
    ng_transition_t *transition = NG_TRANSITION(event);
    NG_WALL_TIME = nema_get_time();
    transition->progress = 0.f;
    transition->start_time = NG_WALL_TIME;
    ng_event_set_status(event, EV_STATUS_RUNNING);
    ng_event_run_callback(event, EV_STATUS_FIRST_RUN);
    ng_timer_start();
}

void ng_transition_stop(ng_event_base_t *event, bool force_finish) {
    if ( force_finish == true ) {
        finish(event);
    } else {
        reset(NG_TRANSITION(event));
    }
}

void ng_transition_revert(ng_event_base_t *event) {
    ng_transition_t *transition = NG_TRANSITION(event);
    ng_event_flip_status_flag(event, EV_STATUS_REVERSED);
    set_start_time_from_progress(transition);
}

void ng_transition_pause_toggle(ng_event_base_t *event, bool pause){
    ng_transition_t *transition = NG_TRANSITION(event);
    if ( pause == true ) {
        ng_event_set_status(event, EV_STATUS_PAUSED);

    } else if ( event->status != EV_STATUS_RUNNING ) {
        // resume not running events
        NG_WALL_TIME = nema_get_time();
        set_start_time_from_progress(transition);
        ng_event_set_status(event, EV_STATUS_RUNNING);
        ng_timer_start();
    }
}

void ng_transition_revert_force(ng_event_base_t *event, int set) {
    if ( set != 0 ) {
        ng_event_set_status_flag(event, EV_STATUS_REVERSED);
    } else {
        ng_event_unset_status_flag(event, EV_STATUS_REVERSED);
    }

    set_start_time_from_progress(NG_TRANSITION(event));
}

void ng_transition_handler(ng_event_base_t *event, uint32_t trigger) {
    ng_transition_t *transition = NG_TRANSITION(event);

    if ( trigger == EV_TRIGGER_TIMER ) {
        update_progress(event);

        if( transition->progress == get_terminal_progress(event) ) {
            ng_transition_stop(event, true);

            // if we need to reverse on next user trigger, reverse
            if ( ng_event_check_retrigger_flag(event,  EV_RETRIGGER_REVERSE )  ) {
                ng_transition_revert(event);
            }
        } else {
            ng_event_run_callback(event, 0);
        }

    } else  {
        if ( ng_event_is_stopped(event) ) {
            //user trigger
            if ( ng_event_check_status_flag(event, EV_STATUS_REVERSED)) {
                ng_event_flip_status_flag(event, EV_STATUS_REVERSED);
            }
            ng_transition_start(event);
        } else {
            //retrigger
            if ( ng_event_check_retrigger_flag(event, EV_RETRIGGER_RESTART) ) {
                ng_transition_stop(event, false);
                ng_transition_start(event);
            } else if ( ng_event_check_retrigger_flag(event, EV_RETRIGGER_RESET) ) {
                ng_transition_stop(event, false);
            } else if ( ng_event_check_retrigger_flag(event, EV_RETRIGGER_FINISH) ) {
                ng_transition_stop(event, true);
            }

            if ( ng_event_is_running(event) ) {
                if ( ng_event_check_retrigger_flag(event,  EV_RETRIGGER_REVERSE ) ) {
                    ng_transition_revert(event);
                }

                if ( ng_event_check_retrigger_flag(event, EV_RETRIGGER_PAUSE) ) {
                    ng_transition_pause_toggle(event, true);
                }
            } else if ( ng_event_is_paused(event) ) {
                if ( ng_event_check_retrigger_flag(event, EV_RETRIGGER_RESUME) ) {
                    ng_transition_pause_toggle(event, false);
                }
            }
        }
    }
}