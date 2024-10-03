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

#include "nema_utils.h"
#include "nema_math.h"

#include "ng_screen_trans.h"
#include "ng_event.h"
#include "ng_globals.h"
#include "ng_display.h"
#include "ng_timer.h"
#include "ng_gestures.h"
#include "ng_tuples.h"

bool goto_right_screen = false;
static bool abort_transition  = false;

void ng_screen_trans_initialize(ng_event_base_t *event, tree_node_t *to_screen, tree_node_t *from_screen, nema_transition_t effect, int go_right, float initial_progress) {

    if( ng_display_get_mode() == DISPLAY_SCREEN_TRANSITION ){
        return;
    }

    event->pause_toggle(event, true);
    ng_node_effect_direction_t *data = (ng_node_effect_direction_t*) event->action_data;
    data->node = to_screen;

    NG_TRANSITION(event)->progress = initial_progress >= 0.f ? initial_progress : -initial_progress;

    goto_right_screen = go_right;

    if (NG_BACKBUFFER_COUNT >= 2) {
        if (( ng_back_buffer_is_locked(0) == false ) && ( ng_back_buffer_is_locked(1) == false )) {
            tree_node_t *left, *right;
            if ( goto_right_screen ) {
                left  = to_screen;
                right = from_screen;
            } else {
                left  = from_screen;
                right = to_screen;
            }

            ng_display_screen_node_to_fb(&NG_BACKBUFFER[0], left , 0, 0);
            ng_display_screen_node_to_fb(&NG_BACKBUFFER[1], right, 0, 0);

            ng_display_bind_transition_buffers();
        }
    }

    abort_transition = false;
    global_screen_trans_effect = effect;
    
    ng_display_set_event(event);
    ng_display_set_mode(DISPLAY_SCREEN_TRANSITION);
}

// Resume as a TIMER triggered transition
void
ng_screen_trans_resume(ng_event_base_t *event, float duration, int abort) {
    abort_transition = (bool) abort;
    NG_TRANSITION(event)->duration = duration;

    ng_transition_revert_force(event, abort);
    event->pause_toggle(event, false);
    NG_DISPLAY_UPDATE = true;
}

void ng_screen_trans_pause(ng_event_base_t *event) {
    event->pause_toggle(event, true);
}

// SCREEN_TRANSITION_PAUSED (which means transitions follows SWIPE)
// Update SCREEN_TRANSITION_PROGRESS
// Stop TRANSITION if ( progress < 0 || progress > 1 )
// progress_diff range: [-1, 1]
void ng_screen_trans_swipe(float progress_diff) {
    if ( SCREEN_TRANSITION_STOPPED && (ng_display_get_mode() == DISPLAY_SCREEN_TRANSITION) ) {
        return;
    }

    if (progress_diff == 0.f)  {
        return;
    }

    if ( SCREEN_TRANSITION_PAUSED ){
        if(goto_right_screen){
            NG_TRANSITION(NG_SCREEN_TRANSITION_EVENT)->progress += progress_diff;
        }else{
            NG_TRANSITION(NG_SCREEN_TRANSITION_EVENT)->progress -= progress_diff;
        }
    }

    if ( (NG_TRANSITION(NG_SCREEN_TRANSITION_EVENT)->progress > 1.f ||
          NG_TRANSITION(NG_SCREEN_TRANSITION_EVENT)->progress < 0.f ) ) {

        if ( NG_TRANSITION(NG_SCREEN_TRANSITION_EVENT)->progress < 0.33f ) {
            abort_transition = true;
        }
        NG_SCREEN_TRANSITION_EVENT->stop(NG_SCREEN_TRANSITION_EVENT, true);
    }

    bool block_screen_transition = false;
    //if we use backbuffers and they are locked by another animation, block this screen transition
    if (NG_BACKBUFFER_COUNT >= 2) {
        if ( (ng_back_buffer_is_locked(0) == true) || (ng_back_buffer_is_locked(1) == true) ) {
            block_screen_transition = true;
        }
    }

    //Start new screen tranistion
    if ( (ng_display_get_mode() != DISPLAY_SCREEN_TRANSITION) && (block_screen_transition == false)) {
        goto_right_screen = progress_diff > 0.f;
        if ((  goto_right_screen && (NG_CUR_SCREEN_NODE_INDEX > 0)) ||
            ( !goto_right_screen && (NG_CUR_SCREEN_NODE_INDEX < NG_SCREENS_PER_GROUP[NG_CUR_SCREEN_GROUP_INDEX]-1))
            ) {

            int from = NG_CUR_SCREEN_NODE_INDEX;
            int to   = goto_right_screen ? NG_CUR_SCREEN_NODE_INDEX - 1 : NG_CUR_SCREEN_NODE_INDEX + 1;
            ng_screen_trans_initialize( NG_SCREEN_TRANSITION_EVENT, NG_CUR_SCREEN_GROUP_NODES[to], NG_CUR_SCREEN_GROUP_NODES[from], NG_EFFECT_PER_GROUP[NG_CUR_SCREEN_GROUP_INDEX], goto_right_screen, progress_diff);
        }
    }

    NG_DISPLAY_UPDATE = true;
}

void ng_screen_trans_swipable(ng_event_base_t *event, void *data) {
    if ( ( NG_SCREEN_TRANSITION_EVENT->status & EV_STATUS_LAST_RUN ) &&  ( (NG_TRANSITION(NG_SCREEN_TRANSITION_EVENT)->progress >= 1.f) || (NG_TRANSITION(NG_SCREEN_TRANSITION_EVENT)->progress <= 0.f)) ) {
        if ( abort_transition == false ) {
            ng_node_effect_direction_t *data = event->action_data;
            ng_tree_set_current_screen(data->node);
        }

        ng_display_set_mode(DISPLAY_SCREEN);
    }
}

void ng_callback_show_screen(ng_event_base_t *event, void *data) {
    ng_transition_t   *transition = NG_TRANSITION(event);

    ng_node_effect_direction_t *trans_data = (ng_node_effect_direction_t*) data;

    if ( ng_event_check_status_flag(event, EV_STATUS_FIRST_RUN) == true ) {
        int direction = 0;
        switch (trans_data->direction){
        case NG_DIRECTION_BOTTOM:
        case NG_DIRECTION_RIGHT:
            direction = 1;
            break;
        
        case NG_DIRECTION_TOP:
        case NG_DIRECTION_LEFT:
        default:
            direction = 0;
            break;
        }

        bool block_transition = false;
        #if BACK_BUFFERS >= 2
            if ( (back_buffer_is_locked(0) == true) || (back_buffer_is_locked(1) == true) ) {
                block_transition = true;
            }
        #endif

        if ( block_transition == true ) {
            event->stop(event, false);
        } else {
            ng_screen_trans_initialize(event, trans_data->node, ng_tree_get_current_screen(), trans_data->effect, direction, 0.f);
            ng_screen_trans_resume(event, transition->duration, 0);
        }

    } else if ( ng_event_check_status_flag(event, EV_STATUS_LAST_RUN) == true ) {
        ng_tree_set_current_screen( trans_data->node);
        ng_display_set_mode(DISPLAY_SCREEN);
       
    } else {
        // Do nothing, screen transition is controlled by event progress
    }
}

void ng_callback_set_screen(ng_event_base_t *event, void *data) {
    ng_tree_node_ptr_t *retrieved_data = (ng_tree_node_ptr_t*)(data);
    tree_node_t *target_screen = retrieved_data->node;
    ng_tree_set_current_screen( target_screen );
    ng_display_set_mode(DISPLAY_SCREEN);
}
