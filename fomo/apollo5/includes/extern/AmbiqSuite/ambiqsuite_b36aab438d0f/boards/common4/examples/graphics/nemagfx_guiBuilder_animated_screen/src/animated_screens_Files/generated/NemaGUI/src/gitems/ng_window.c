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
#include "ng_event.h"
#include "ng_window.h"
#include "ng_globals.h"


static GESTURE_FUNC_PRESS(press) {
    (void)x;
    (void)y;
	ng_gitem_unset_flag(node->this_, GITEMF_STOP_RECUR);
}

static GESTURE_FUNC_RELEASE(release) {
    ng_gitem_unset_flag(node->this_, GITEMF_STOP_RECUR);
}

static GESTURE_FUNC_SWIPE(swipe) {
    (void)x;
    (void)y;

    ng_gitem_set_flag(node->this_, GITEMF_STOP_RECUR);
    gitem_base_t *subwindow = node->first_child->this_;

    if (subwindow->h > node->this_->h) {
        int cont_y_min = subwindow->y;
        int cont_y_max = subwindow->h + cont_y_min;
        int cont_y_min_new = cont_y_min + dy;
        int cont_y_max_new = cont_y_max + dy;

        if (cont_y_max_new < node->this_->h && cont_y_min_new > 0)
            //don't do anything
            cont_y_min_new = cont_y_min;
        else if (cont_y_min_new > 0)
            cont_y_min_new = 0;
        else if (cont_y_max_new < node->this_->h)
            cont_y_min_new = node->this_->h - subwindow->h;

        subwindow->y = cont_y_min_new;
    }

    if (subwindow->w > node->this_->w) {
        int cont_x_min = subwindow->x;
        int cont_x_max = subwindow->w + cont_x_min;
        int cont_x_min_new = cont_x_min + dx;
        int cont_x_max_new = cont_x_max + dx;

        if (cont_x_max_new < node->this_->w && cont_x_min_new > 0)
            //don't do anything
            cont_x_min_new = cont_x_min;
        else if (cont_x_min_new > 0)
            cont_x_min_new = 0;
        else if (cont_x_max_new < node->this_->w)
            cont_x_min_new = node->this_->w - subwindow->w;

        subwindow->x = cont_x_min_new;
    }
}

gitem_gestures_t gestures_window = {press, release, swipe, NULL};

void ng_window_set_source(tree_node_t *window, tree_node_t *source){
    window->first_child->first_child = source;
    window->first_child->this_->w    = source->this_->w;
    window->first_child->this_->h    = source->this_->h;
}
