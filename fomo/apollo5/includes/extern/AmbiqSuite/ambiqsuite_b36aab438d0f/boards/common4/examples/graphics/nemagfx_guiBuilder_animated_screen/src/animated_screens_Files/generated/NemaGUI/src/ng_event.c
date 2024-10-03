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

#include "nema_event.h"

#include "ng_event.h"
#include "ng_event_transition.h"
#include "ng_event_periodic.h"
#include "ng_event_periodic_transition.h"
#include "ng_globals.h"
#include "ng_display.h"
#include "ng_timer.h"
#include "ng_screen_trans.h"

//asign events to "source" gitems
void ng_event_init() {
    int master_timer = ng_timer_create();

    for (int i = 0; i < NG_EVENT_LIST_SIZE; ++i) {
        if(NG_EVENT_LIST[i]->src_gitem != NULL){
            ng_event_base_t **event = &NG_EVENT_LIST[i]->src_gitem->event;

            while (*event != NULL) {
                event = &(*event)->next;
            }

            *event = NG_EVENT_LIST[i];
        }
    }
}

void ng_event_set_status(ng_event_base_t *event, uint32_t status) {
    event->status &= (~EV_STATUS_MASK);
    event->status |= status;
}

void ng_event_set_status_flag(ng_event_base_t *event, int flag) {
    event->status |= flag;
}

void ng_event_unset_status_flag(ng_event_base_t *event, int flag) {
    event->status &= (~flag);
}

bool ng_event_check_status_flag(ng_event_base_t *event, int flag) {
    return (event->status & flag) == 0 ? false : true;
}

void ng_event_flip_status_flag(ng_event_base_t *event, int flag) {
    // flag is set, unset it
    if ( ng_event_check_status_flag(event, flag) ) {
        ng_event_unset_status_flag(event, flag);
    }
    // flag is not set, set it
    else {
        ng_event_set_status_flag(event, flag);
    }
}

bool ng_event_check_retrigger_flag(ng_event_base_t *event, int flag) {
    if ( (event->retrigger & flag) == 0 ) {
        return false;
    } else {
        return true;
    }
}

void ng_event_run_callback(ng_event_base_t *event, int status_flags) {
        if (status_flags != 0) {
            ng_event_set_status_flag(event, status_flags);
        }

        event->callback(event, event->action_data);

        if (status_flags != 0) {
            ng_event_unset_status_flag(event, status_flags);
        }
}

static inline bool needs_handling(ng_event_base_t *event, uint32_t trigger) {
    bool result = false;

    //if trigger matches
    //and the event is not running (it is paused or stopped) or can be re-triggered
    if ( ( event->trigger == trigger ) && ( (!ng_event_is_running(event)) || ( !ng_event_check_retrigger_flag(event, EV_RETRIGGER_IGNORE) ) ) ) {
        result = true;
    } else if (( trigger == EV_TRIGGER_TIMER ) && ( ng_event_is_running(event) )) {
        //if the trigger is timer and the event is running
        result = true;
    }

    return result;
}

void ng_event_handle(ng_event_base_t *event, uint32_t trigger_event) {
    NG_WALL_TIME = nema_get_time();

    while ( (event != NULL) && (event->handler != NULL) ) {
        if ( needs_handling(event, trigger_event) == true ) {
            event->handler(event, trigger_event);

            //Update the display if the event affects the current screen or we do not know the affected screen 
            if ( (NG_DISPLAY_UPDATE == false) && ((event->affected_screen_id == NG_CUR_SCREEN_GROUP_NODES[NG_CUR_SCREEN_NODE_INDEX]->this_->id) || (event->affected_screen_id < 0)) ) {
                NG_DISPLAY_UPDATE = true;
            //or if the current screen contains a window    
            } else if ( (NG_DISPLAY_UPDATE == false) && (NG_CUR_SCREEN_GROUP_NODES[NG_CUR_SCREEN_NODE_INDEX]->this_->flags & GITEMF_CONTAINS_WINDOW) ) {
                NG_DISPLAY_UPDATE = true;
            //or if a pop is being currently displayed    
            } else if ( (NG_DISPLAY_UPDATE == false) && (ng_display_get_mode() == DISPLAY_POPUP) ) {
                NG_DISPLAY_UPDATE = true;
            }
        }

        if ( (ng_event_is_running(event)) && (ng_timer_is_running() == false) ) {
            ng_timer_start();
        }

        event = event->next;

        if ( trigger_event == EV_TRIGGER_TIMER ) {
            return;
        }
    }
}
