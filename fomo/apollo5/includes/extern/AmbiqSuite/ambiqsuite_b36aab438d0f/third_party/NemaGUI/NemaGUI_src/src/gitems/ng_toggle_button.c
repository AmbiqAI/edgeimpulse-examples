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

#include "ng_globals.h"

#include "nema_utils.h"


// static GESTURE_FUNC_ABORT_PRESS(abort_press) {

// }

// static GESTURE_FUNC_PRESS(press) {

// }

static GESTURE_FUNC_RELEASE(release) {
    gitem_toggle_button_t *button = NG_TOGGLE_BUTTON(node->this_);
    uint16_t           init_state = button->cur_state;

    //Go to next state
    button->cur_state++;

    //Start over from state 0
    if ( button->cur_state >= button->max_state ) {
        button->cur_state = 0;
    }

    if ( (button->event) && (button->cur_state != init_state) ) {
        ng_event_handle(button->event, EV_TRIGGER_STATE_CHANGED);
    }
}

gitem_gestures_t gestures_toggle_button = {NULL, release, NULL, NULL};

DRAW_FUNC(ng_toggle_button_draw) {
    gitem_toggle_button_t *button = NG_TOGGLE_BUTTON(git);

    uint8_t opacity = (button->color & 0xff000000) >> 24;

    int x_draw = button->x + x_off;
    int y_draw = button->y + y_off;
    int w_draw = button->w;
    int h_draw = button->h;

    if ( (button->flags & GITEMF_PRESS_SCALE) && (button->flags & GITEMF_HIGHLIGHTED) ) {
        int dw = (int) (0.1f*w_draw);
        int dh = (int) (0.1f*h_draw);

        x_draw = x_draw - dw/2;
        y_draw = y_draw - dh/2;
        w_draw = w_draw + dw;
        h_draw = h_draw + dh;
    }

    ng_blit_rect_fit(button->images[button->cur_state], x_draw, y_draw, w_draw, h_draw, 0, opacity);
}
