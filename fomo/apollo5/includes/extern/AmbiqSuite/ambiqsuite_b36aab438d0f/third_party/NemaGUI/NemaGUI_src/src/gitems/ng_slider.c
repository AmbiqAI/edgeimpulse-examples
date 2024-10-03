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
#include "ng_container.h"
#include "ng_event.h"
#include "ng_progress_bar.h"

static void update_indicator_position(gitem_slider_t *slider) {
    if ( slider->type == GITEM_HORIZONTAL_SLIDER ) {
        int x = slider->value*slider->w;
        ng_slider_horizontal_set_indicator_x(NG_GITEM(slider), x);
    } else {
        int y = slider->h - slider->value*slider->h;
        ng_slider_vertical_set_indicator_y(NG_GITEM(slider), y);
    }
}

void ng_slider_set_percent(gitem_base_t *git, float percent) {
    gitem_slider_t *slider = NG_SLIDER(git);
    float val_init = slider->value;
    slider->value  = CLAMP(percent, 0.f, 1.f);

    if ( val_init != percent ) {
        update_indicator_position(slider);
        ng_progress_bar_set_percent(slider->progress, percent);

        if (slider->event) {
            ng_event_handle(slider->event, EV_TRIGGER_VALUE_CHANGED);
        }
    }
}