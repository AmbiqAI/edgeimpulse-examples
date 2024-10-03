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

#include "ng_gitem.h"
#include "ng_swipe_window.h"
#include "ng_globals.h"
#include "nema_math.h"
#include "ng_callbacks.h"

static GESTURE_FUNC_PRESS(press) {
    (void)x;
    (void)y;
	ng_gitem_unset_flag(node->this_, GITEMF_STOP_RECUR);

    gitem_swipe_window_t *window = NG_SWIPE_WINDOW(node->this_);
    if ( (window->animation != NULL) && (window->animation->src_gitem == node->this_) && (window->animation->status == EV_STATUS_RUNNING) ) {
       window->animation->pause_toggle(NG_EVENT(window->animation), true);
    }
}

static GESTURE_FUNC_RELEASE(release) {
    ng_gitem_unset_flag(node->this_, GITEMF_STOP_RECUR);

    gitem_swipe_window_t *window = NG_SWIPE_WINDOW(node->this_);
    gitem_base_t *subwindow = node->first_child->this_;
    float max_duration = 0.3f;
    int new_page = window->cur_page_index;

    if ( window->layout == NG_LAYOUT_HOR ) {
        int step_x    = window->w + window->spacing;
        int diff      = -subwindow->x % step_x;
        int target_x  = 0;

        bool align_page = (subwindow->x != 0) && ( (-subwindow->x % (window->w)) != 0 );

        if ( align_page == true ) {
            if ( -subwindow->x % step_x > step_x / 2 ) {
                target_x = subwindow->x - step_x + diff;
            } else {
                target_x = subwindow->x + diff;
            }

            int dx = subwindow->x - target_x;
            float duration = max_duration*(float)nema_abs(dx)/(float)window->w;

            window->animation_data.git = subwindow;
            window->animation_data.a   = subwindow->x;
            window->animation_data.b   = target_x;
            if ( (window->animation != NULL) && (window->animation->status == EV_STATUS_PAUSED) ) {
                window->animation->pause_toggle(NG_EVENT(window->animation), false);
            } else {
                window->animation = NULL;
                for ( int i = 1; i < NG_TEMP_ANIMATIONS_COUNT; ++i ) {
                    if ( (NG_EVENT_LIST[i]->status == EV_STATUS_STOPPED) ) {
                        window->animation = NG_TRANSITION(NG_EVENT_LIST[i]);
                        window->animation->src_gitem              = node->this_;
                        window->animation->callback               = ng_animate_int_int;
                        window->animation->action->act_gitptr_int = ng_gitem_set_x;
                        window->animation->duration               = duration;
                        window->animation->progress               = duration/max_duration;
                        window->animation->action_data            = &(window->animation_data);
                        window->animation->pause_toggle(NG_EVENT(window->animation), false);
                        break;
                    }
                }

                //If the animation is not available, swap page instantly
                if ( window->animation == NULL ) {
                    subwindow->x = target_x;
                }
            }

            new_page =  -target_x/window->w;
        } else {
            if ( window->animation != NULL ) {
                window->animation = NULL;
            }
            new_page =  -subwindow->x/window->w;
        }
    } else {
        int step_y    = window->h + window->spacing;
        int diff      = -subwindow->y % step_y;
        int target_y  = 0;

        bool align_page = (subwindow->y != 0) && ( (-subwindow->y % (window->h)) != 0 );

        if ( align_page == true ) {
            if ( -subwindow->y % step_y > step_y / 2 ) {
                target_y = subwindow->y - step_y + diff;
            } else {
                target_y = subwindow->y + diff;
            }

            int dy = subwindow->y - target_y;
            float duration = max_duration*(float)nema_abs(dy)/(float)window->h;

            window->animation_data.git = subwindow;
            window->animation_data.a   = subwindow->y;
            window->animation_data.b   = target_y;

            if ( (window->animation != NULL) && (window->animation->status == EV_STATUS_PAUSED) ) {
                window->animation->pause_toggle(NG_EVENT(window->animation), false);
            } else {
                window->animation = NULL;
                for ( int i = 1; i < NG_TEMP_ANIMATIONS_COUNT; ++i ) {
                    if ( (NG_EVENT_LIST[i]->status == EV_STATUS_STOPPED) ) {
                        window->animation = NG_TRANSITION(NG_EVENT_LIST[i]);
                        window->animation->src_gitem              = node->this_;
                        window->animation->callback               = ng_animate_int_int;
                        window->animation->action->act_gitptr_int = ng_gitem_set_y;
                        window->animation->duration               = duration;
                        window->animation->progress               = duration/max_duration;
                        window->animation->action_data            = &(window->animation_data);
                        window->animation->pause_toggle(NG_EVENT(window->animation), false);
                        break;
                    }
                }

                //If the animation is not available, swap page instantly
                if ( window->animation == NULL ) {
                    subwindow->y = target_y;
                }
            }
            new_page =  -target_y/window->h;
        } else {
            if ( window->animation != NULL ) {
                window->animation = NULL;
            }
            new_page =  -subwindow->y/window->h;
        }
    }

    //Highlight the indicator
    if ( (window->indicators_parent) && (new_page != window->cur_page_index) ) {
        tree_node_t *indicator_node = NULL;

        for ( int i = 0; i < window->page_count; ++i ) {
            if ( i == 0 ) {
                indicator_node = node->first_child->next->first_child;
            } else {
                indicator_node = indicator_node->next;
            }

            if ( i == window->cur_page_index ) {
                ng_gitem_unset_flag(indicator_node->this_, GITEMF_HIGHLIGHTED);
            } else if ( i == new_page ) {
                ng_gitem_set_flag(indicator_node->this_, GITEMF_HIGHLIGHTED);
            }
        }

        window->cur_page_index = new_page;
    }
}

static GESTURE_FUNC_SWIPE(swipe) {
    (void)x;
    (void)y;

    ng_gitem_set_flag(node->this_, GITEMF_STOP_RECUR);
    gitem_swipe_window_t  *window = NG_SWIPE_WINDOW(node->this_);
    gitem_base_t       *subwindow = node->first_child->this_;

    if ( (window->layout == NG_LAYOUT_VER) && (subwindow->h > node->this_->h) ) {
        int cont_y_min = subwindow->y;
        int cont_y_max = subwindow->h + cont_y_min;
        int cont_y_min_new = cont_y_min + dy;
        int cont_y_max_new = cont_y_max + dy;

        if (cont_y_max_new < node->this_->h && cont_y_min_new > 0) {
            //don't do anything
            cont_y_min_new = cont_y_min;
        } else if (cont_y_min_new > 0) {
            cont_y_min_new = 0;
        } else if (cont_y_max_new < node->this_->h) {
            cont_y_min_new = node->this_->h - subwindow->h;
        }

        subwindow->y = cont_y_min_new;
    } else if ( (window->layout == NG_LAYOUT_HOR) && (subwindow->w > node->this_->w) ) {
        int cont_x_min = subwindow->x;
        int cont_x_max = subwindow->w + cont_x_min;
        int cont_x_min_new = cont_x_min + dx;
        int cont_x_max_new = cont_x_max + dx;

        if (cont_x_max_new < node->this_->w && cont_x_min_new > 0) {
            //don't do anything
            cont_x_min_new = cont_x_min;
        } else if (cont_x_min_new > 0) {
            cont_x_min_new = 0;
        } else if (cont_x_max_new < node->this_->w) {
            cont_x_min_new = node->this_->w - subwindow->w;
        }

        subwindow->x = cont_x_min_new;
    }
}

gitem_gestures_t gestures_swipe_window = {press, release, swipe, NULL};
