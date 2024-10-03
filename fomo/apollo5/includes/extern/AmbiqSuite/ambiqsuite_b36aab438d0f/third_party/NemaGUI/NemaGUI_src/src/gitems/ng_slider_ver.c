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

#include "ng_slider.h"
#include "ng_slider_ver.h"
#include "ng_globals.h"

void ng_slider_vertical_set_indicator_y(gitem_base_t *git, int y)
{
    gitem_slider_t *slider  = NG_SLIDER(git);
    gitem_base_t *indicator = slider->indicator;

    //indicator radius
    int indic_r = indicator->h>>1;

    //set indicator position
    y -= indic_r;
    indicator->y = y <= indic_r ? indic_r : y;
    indicator->y = y >= slider->h - indic_r ? slider->h - indic_r : y;

    //clip indicator position
    if ( indicator->y < 0 ) {
        indicator->y = 0;
    } else if ( indicator->y > slider->h-indicator->h ) {
        indicator->y = slider->h-indicator->h;
    }
}

static GESTURE_FUNC_PRESS(press) {
    (void)x;
    gitem_slider_t *slider     = NG_SLIDER(node->this_);
    gitem_base_t   *indicator  = slider->indicator;

    float indicator_y = (float) (y - indicator->h/2);
    float         val = 1.f - indicator_y/(slider->h - indicator->h);

    ng_slider_set_percent(NG_GITEM(slider), val);
}

static GESTURE_FUNC_SWIPE(swipe) {
    (void)dx;
    (void)dy;

    press(node, x, y);
}

gitem_gestures_t gestures_slider_ver = {press, NULL, swipe, NULL};
