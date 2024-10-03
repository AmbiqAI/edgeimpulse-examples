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
#include "ng_slider.h"
#include "ng_slider_hor.h"

void ng_slider_horizontal_set_indicator_x(gitem_base_t *git, int x) {

    gitem_slider_t *slider  = NG_SLIDER(git);
    gitem_base_t *indicator = slider->indicator;

    //indicator radius
    int indic_r = indicator->w>>1;

    //set indicator position
    x -= indic_r;
    indicator->x = x <= indic_r ? indic_r : x;
    indicator->x = x >= slider->w - indic_r ? slider->w - indic_r : x;

    //clip indicator position
    if ( indicator->x < 0 ) {
        indicator->x = 0;
    } else if ( indicator->x > slider->w-indicator->w ) {
        indicator->x = slider->w-indicator->w;
    }
}

static GESTURE_FUNC_PRESS(press) {
    (void)y;
    gitem_slider_t *slider  = NG_SLIDER(node->this_);
    gitem_base_t *indicator = slider->indicator;

    float indicator_x = (float) (x - indicator->w/2);
    float         val = indicator_x/(slider->w - indicator->w);

    ng_slider_set_percent(NG_GITEM(slider), val);
}

static GESTURE_FUNC_SWIPE(swipe) {
    (void)dx;
    (void)dy;
	press(node, x, y);
}

gitem_gestures_t gestures_slider_hor = {press, NULL, swipe, NULL};
