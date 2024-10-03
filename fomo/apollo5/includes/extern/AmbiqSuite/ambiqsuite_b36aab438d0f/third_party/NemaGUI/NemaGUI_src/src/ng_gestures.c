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

#include "ng_gestures.h"
#include "ng_tree.h"
#include "ng_display.h"
#include "ng_screen_trans.h"
#include "ng_timer.h"
#include "nema_sys_defs.h"
#include "ng_timer.h"

// this is the node that the original press was assigned to
static tree_node_t *pressed_node  = NULL;

static int     cur_node_x         = 0;
static int     cur_node_y         = 0;
static int     mouse_dxy          = 0;
// swiped screens per sec
// e.g. 1 means the total transition would take 1 second
// e.g. 0.1 means we the total transition is fast, it would take 0.1 second
static float   swipe_velocity     = 0.f;

static void
ng_reset_swipe_velocity(void) {
    swipe_velocity = 0.f;
}

// calculates swipe_velocity (screens/sec)
// takes current velocity and averages with previous velocities to avoid spikes
static void
ng_update_swipe_velocity(float velocity_screens_per_frame) {
    float tmp_swipe_velocity = velocity_screens_per_frame*ng_timer_get_frequency();
    swipe_velocity = swipe_velocity*0.8f + tmp_swipe_velocity*0.2f;
}

static void abort_popup(){
    ng_display_set_mode(DISPLAY_SCREEN);
}

//returns true if popup is active and point (x,y) is inside the popup
bool 
ng_gestures_is_inside_popup(int x, int y) {
    if ( ng_display_get_mode() != DISPLAY_POPUP ) {
        return false;
    }

    if (x >= popup_node->this_->x + popup_off_x &&
            y >= popup_node->this_->y + popup_off_y &&
            x <  popup_node->this_->x + popup_off_x + popup_node->this_->w &&
            y <  popup_node->this_->y + popup_off_y + popup_node->this_->h) {
            return true;
    } else {
        return false;
    }
}

static GESTURE_FUNC_ABORT_PRESS(abort_press_generic)
{
    ng_gitem_unset_flag(node->this_, GITEMF_PRESS);
    ng_gitem_unset_flag(node->this_, GITEMF_HIGHLIGHTED);

    if ((node->this_->gestures != NULL) && (node->this_->gestures->abort != NULL)) {
        node->this_->gestures->abort(node);
    }
    pressed_node = NULL;
}

static GESTURE_FUNC_PRESS(press_generic)
{
    (void)x;
    (void)y;
    ng_gitem_set_flag(node->this_, GITEMF_HIGHLIGHTED);

    if ( (((node->this_->gestures != NULL) && ((node)->this_->gestures->press != NULL))) ) {
        node->this_->gestures->press(node, cur_node_x, cur_node_y);
    }

    if ( node->this_->event != NULL ) {
         ng_event_handle(node->this_->event, EV_TRIGGER_PRESS);
    }
}

static GESTURE_FUNC_RELEASE(release_generic)
{
    ng_gitem_unset_flag(node->this_, GITEMF_HIGHLIGHTED);


    if ( (((node->this_->gestures != NULL) && ((node)->this_->gestures->release != NULL))) ) {
        node->this_->gestures->release(node);
    }

    if ( node->this_->event != NULL ) {
         ng_event_handle(node->this_->event, EV_TRIGGER_RELEASE);
    }
}

static GESTURE_FUNC_SWIPE(swipe_generic)
{
    if ( (((node->this_->gestures != NULL) && ((node)->this_->gestures->swipe != NULL))) ) {
        node->this_->gestures->swipe(node, x, y, dx, dy);
    }

    if ( node->this_->event != NULL ) {
         ng_event_handle(node->this_->event, EV_TRIGGER_DRAG);
    }
}

tree_node_t*
ng_gestures_press(nema_event_t *event, int event_press_x, int event_press_y) {
    (void)event;

    ng_reset_swipe_velocity();

    pressed_node    = NULL;

    // if doing screen transition and mouse is pressed, pause transition
    if ( DOING_SCREEN_TRANSITION ) {
        pressed_node = NG_CUR_SCREEN_GROUP_NODES[NG_CUR_SCREEN_NODE_INDEX];
        ng_screen_trans_pause(NG_SCREEN_TRANSITION_EVENT);

    } else {
        int x_off = 0;
        int y_off = 0;

        (ng_gestures_is_inside_popup(event_press_x, event_press_y) == true ) ? x_off+=popup_off_x : 0;
        (ng_gestures_is_inside_popup(event_press_x, event_press_y) == true ) ? y_off+=popup_off_y : 0;

        // find a node under cursor that supports PRESS or RELEASE
        pressed_node = ng_tree_get_node_under_cursor( NULL, GITEMF_PRESS | GITEMF_RELEASE, event_press_x, event_press_y, x_off, y_off, &cur_node_x  , &cur_node_y);
        //at worst case we have pressed a screen or a popup, thus pressed_node != NULL
        // Press Selected Node
        if ( pressed_node != NULL ) {
            ng_gitem_set_flag(pressed_node->this_, GITEMF_HIGHLIGHTED);

            // if selected_node supports PRESS (it may support only release), press it
            if (pressed_node->this_->flags & GITEMF_PRESS) {
                press_generic(pressed_node, cur_node_x, cur_node_y);
            }

            NG_DISPLAY_UPDATE = true;
        } 
    }

    return pressed_node;
}

void
ng_gestures_release(nema_event_t *event) {
    // if doing swipe-transition, finish it
    if ( ng_event_is_paused(NG_SCREEN_TRANSITION_EVENT) ) {
        float duration = SCREEN_TRANSITION_DURATION_SECS/swipe_velocity;

        if ( duration < 0.f ) {
            duration = -duration;
        }

        if ( duration > SCREEN_TRANSITION_DURATION_SECS ) {
            duration = SCREEN_TRANSITION_DURATION_SECS;
        }

        bool abort = NG_TRANSITION(NG_SCREEN_TRANSITION_EVENT)->progress < 0.33f;
        ng_screen_trans_resume(NG_SCREEN_TRANSITION_EVENT, duration, abort);
    }
    
    // if a node is pressed, release it
    else if ( pressed_node != NULL ) {

        if ( (ng_display_get_mode() == DISPLAY_POPUP) && (ng_gestures_is_inside_popup(event->mouse_x, event->mouse_y) == false) ) {
                abort_popup();
        } else if (pressed_node->this_->flags & GITEMF_RELEASE) {
            release_generic(pressed_node);
        }
        
        NG_DISPLAY_UPDATE = true;
    }

    pressed_node  = NULL;
}

void
ng_gestures_swipe(nema_event_t *event, int mouse_dx, int mouse_dy){
    cur_node_x += mouse_dx;
    cur_node_y += mouse_dy;

    uint8_t layout = NG_LAYOUT_PER_GROUP[NG_CUR_SCREEN_GROUP_INDEX];
    mouse_dxy = layout != 0 ? mouse_dy : mouse_dx;

    // if doing a transition, update the transition
    if ( DOING_SCREEN_TRANSITION ) {
        if ( mouse_dxy != 0 ) {
            //velocity: screens/frame
            float velocity  = layout ? (float)mouse_dy/NG_RESY : (float)mouse_dx/NG_RESX;
            ng_screen_trans_swipe(velocity);
            //velocity: screens/second
            ng_update_swipe_velocity(velocity);
        }

    } else {
        int x_off       = 0;
        int y_off       = 0;
        //(cur_node_x, cur_node_y) refer to the pressed node. We might need these values, 
        int cur_node2_x = cur_node_x;
        int cur_node2_y = cur_node_y;

        if ( ng_gestures_is_inside_popup(cur_node_x, cur_node_y) == true ) {
            x_off += popup_off_x;
            y_off += popup_off_y;
        }

        //get the tree node under cursor
        tree_node_t *swipe_node = ng_tree_get_node_under_cursor( NULL, GITEMF_DRAG, event->mouse_x, event->mouse_y, x_off, y_off, &cur_node2_x, &cur_node2_y);

        //If the pressed_node does not support swipe, abort press
        if ( (pressed_node != NULL) && ( (pressed_node->this_->flags & GITEMF_DRAG) == 0) ) {
            abort_press_generic(pressed_node);
            NG_DISPLAY_UPDATE = true;
            pressed_node         = NULL;
            return;
        }

        if ( ng_display_get_mode() == DISPLAY_SCREEN ) {
            //We can swipe either the pressed_node
            if ( pressed_node != NULL ) {
                if ( pressed_node->this_->type == GITEM_MAIN_SCREEN ) {
                    float velocity  = layout ? (float)mouse_dy/NG_RESY : (float)mouse_dx/NG_RESX;
                    NG_TRANSITION(NG_SCREEN_TRANSITION_EVENT)->progress = 0.f;
                    ng_screen_trans_swipe(velocity);
                    ng_update_swipe_velocity(velocity);
                } else {
                    swipe_generic(pressed_node, cur_node_x, cur_node_y, mouse_dx, mouse_dy);
                }
            //or the swipe_node (node under cursor)
            } else if ( swipe_node != NULL ) {
                if ( swipe_node->this_->type == GITEM_MAIN_SCREEN ) {
                    float velocity  = layout ? (float)mouse_dy/NG_RESY : (float)mouse_dx/NG_RESX;
                    NG_TRANSITION(NG_SCREEN_TRANSITION_EVENT)->progress = 0.f;
                    ng_screen_trans_swipe(velocity);
                    ng_update_swipe_velocity(velocity);
                } else {
                    swipe_generic(swipe_node, cur_node2_x, cur_node2_y, mouse_dx, mouse_dy);
                    cur_node_x = cur_node2_x;
                    cur_node_y = cur_node2_y;
                }

                //keep the swipe_node if needed in "release"
                pressed_node = swipe_node;
            }

            NG_DISPLAY_UPDATE = true;
        }
    }
}
