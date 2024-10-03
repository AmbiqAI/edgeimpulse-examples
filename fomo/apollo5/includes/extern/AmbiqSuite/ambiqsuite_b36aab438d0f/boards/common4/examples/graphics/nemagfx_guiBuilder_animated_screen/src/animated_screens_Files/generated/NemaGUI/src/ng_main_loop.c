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

#include "ng_main_loop.h"
#include "ng_display.h"
#include "ng_event.h"
#include "ng_globals.h"
#include "ng_timer.h"

void ng_main_loop(const int run_once) {
    static nema_event_t event = {0};
    static tree_node_t *pressed_node = NULL;

    // x,y coordinates of the last press event
    int mouse_press_x      = -1;
    int mouse_press_y      = -1;
    NG_DISPLAY_UPDATE      = false;
    const int ng_timer_id  = ng_timer_get();

    //Trigger on application start-up events
    for (int i = 0; i < NG_EVENT_LIST_SIZE; ++i) {
        ng_event_base_t *event = NG_EVENT_LIST[i];

        if ( event->trigger == EV_TRIGGER_APP_START ) {
            event->start(event);
        }
    }

    do {
        bool press_event       = false;
        bool release_event     = false;
        bool swipe_event       = false;
        bool press_r_event     = false;
        bool timer_event       = false;

        int mouse_dx           =  0;
        int mouse_dy           =  0;

        //wait for press/release/swipe/timer event
        bool block_until_event = (run_once == 0) && (!NG_DISPLAY_UPDATE);
        if ( nema_event_wait(&event, block_until_event) > 0) {
            do {

                // get mouse/touch event
                switch (event.mouse_event) {
                    case MOUSE_EVENT_LEFT_CLICK:
                        press_event   = true;
                        mouse_press_x = event.mouse_x;
                        mouse_press_y = event.mouse_y;
                        break;
                    case MOUSE_EVENT_LEFT_RELEASE:
                        release_event = true;
                        break;
                    case MOUSE_EVENT_RIGHT_CLICK:
                        press_r_event = true;
                        break;
                    default:
                        break;
                }

                // check if the user is swipping
                // if yes, set swipe_event = true
                if ( (event.mouse_state == MOUSE_STATE_LEFT_CLICKED) ) {
                    //accumulate drag events
                    mouse_dx += event.mouse_dx;
                    mouse_dy += event.mouse_dy;

                    int swipe_margin = 5;

                    if(pressed_node != NULL){
                        // if the pressed node doesn't support SWIPE , don't swipe immediately
                        // give it some margin
                        swipe_margin = ((pressed_node->this_->flags & GITEMF_DRAG) != 0U) ? 0 : swipe_margin;
                    }

                    if ( (event.mouse_dx != 0 || event.mouse_dy != 0 ) &&
                         (nema_abs(event.mouse_x - mouse_press_x) > swipe_margin  ||
                          nema_abs(event.mouse_y - mouse_press_y) > swipe_margin ) ) {
                            swipe_event = true;
                    }
                }

                if ( event.timer_id == ng_timer_id ) {
                    timer_event = true;
                }

            } while ( nema_event_wait(&event, 0) );
        } else {
            //no events, nothing to do in this iteration
            if ( NG_DISPLAY_UPDATE == false ) {
                continue;
            }
        }

        //Process event
        if ( press_event ){
            pressed_node = ng_gestures_press(&event, mouse_press_x, mouse_press_y);
        }

        if( swipe_event ) {
            ng_gestures_swipe(&event, mouse_dx, mouse_dy);
        }

        if ( release_event ) {
            ng_gestures_release(&event);
        }

        if( timer_event ) {
            ng_timer_handler();
        }

        if ( press_r_event ) {
            //On right click, change the transition effect
            NG_EFFECT_PER_GROUP[NG_CUR_SCREEN_GROUP_INDEX] = (NG_EFFECT_PER_GROUP[NG_CUR_SCREEN_GROUP_INDEX]+1)%NEMA_TRANS_MAX;
            NG_DISPLAY_UPDATE = true;
        }

        if(NG_DISPLAY_UPDATE == true){
            NG_DISPLAY_UPDATE = false;
            ng_display();
        }

    } while(run_once == 0);
}
