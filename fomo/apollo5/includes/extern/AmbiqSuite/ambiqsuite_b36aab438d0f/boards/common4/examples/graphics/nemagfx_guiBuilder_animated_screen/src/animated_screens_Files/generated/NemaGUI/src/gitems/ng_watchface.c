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

// static GESTURE_FUNC_ABORT_PRESS(abort) {

// }

// static GESTURE_FUNC_PRESS(press) {

// }

// static GESTURE_FUNC_RELEASE(release) {

// }

// static GESTURE_FUNC_SWIPE(swipe) {

// }

const gitem_gestures_t watchface_gestures = {NULL, NULL, NULL, NULL};

DRAW_FUNC(ng_watchface_draw) {
    gitem_watchface_t *watchface = NG_WATCHFACE(git);

    //Draw background
    if ( watchface->flags & GITEMF_FILL_IMAGE ) {
        ng_blit_rect_fit(watchface->image, watchface->x + x_off, watchface->y + y_off, watchface->w, watchface->h, 0, (watchface->color & 0xff000000U) >> 24);
    } else {
        if ( watchface->flags & GITEMF_FILL_COLOR ) {
            ng_fill_rect(watchface->x + x_off, watchface->y + y_off, watchface->w, watchface->h, watchface->color, 0);
        } else {
            ng_draw_primitive_rect(watchface->x + x_off, watchface->y + y_off, watchface->w, watchface->h, watchface->color, watchface->pen_width, 0);
        }
    }
}

void ng_watchface_set_image(gitem_base_t *git, void *asset_ptr) {
    gitem_watchface_t *watchface = NG_WATCHFACE(git);
	watchface->image = (img_obj_t*) asset_ptr;
}

void ng_watchface_update(gitem_base_t *git) {
    #ifdef WALL_TIME_CLOCKS
    gitem_watchface_t *watchface = NG_WATCHFACE(git);

    //Update second hand
    if ( watchface->sec != NULL ) {
        gitem_needle_t *sec_hand  = NG_NEEDLE(watchface->sec);
        float sec  = NG_WALL_TIME;
        sec_hand->angle = sec/60.f*360.f;
    }

    //Update minute hand
    if ( watchface->minute != NULL ) {
        gitem_needle_t *min_hand  = NG_NEEDLE(watchface->minute);
        float min  = NG_WALL_TIME/60.f;
        min_hand->angle = min/60.f*360.f;
    }

    //Update hour hand
    if ( watchface->hour != NULL ) {
        gitem_needle_t *hour_hand = NG_NEEDLE(watchface->hour);
        float hour = NG_WALL_TIME/3600.f;
        hour_hand->angle = hour/12.f*360.f;
    }
    #else
    //User code for updating the hand angles goes here
    #endif
}
