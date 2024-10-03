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

#include "ng_checkbox.h"
#include "ng_draw_prim.h"
#include "ng_event.h"
#include "ng_utils.h"

static GESTURE_FUNC_PRESS(press) {
    (void)x;
    (void)y;
    ng_gitem_set_flag(node->this_, GITEMF_HIGHLIGHTED);
}

static GESTURE_FUNC_RELEASE(release) {
    ng_gitem_unset_flag(node->this_, GITEMF_HIGHLIGHTED);

    if(node->this_->flags & GITEMF_CHECKED){
    	ng_gitem_unset_flag(node->this_, GITEMF_CHECKED);
    }else{
    	ng_gitem_set_flag(node->this_, GITEMF_CHECKED);
    }
}

gitem_gestures_t gestures_checkbox = {press, release, NULL, NULL};

DRAW_FUNC(ng_checkbox_draw)
{
    gitem_checkbox_t *checkbox = SAFE_CAST(git, gitem_base_t *, gitem_checkbox_t *);
    const uint32_t color = (checkbox->flags & GITEMF_HIGHLIGHTED) ? checkbox->secondary_color : checkbox->color;

    //Draw background
    if ( git->flags & GITEMF_FILL_COLOR ) {
        ng_draw_primitive_rect(checkbox->x + x_off, checkbox->y + y_off, checkbox->w, checkbox->h, color, checkbox->pen_width, 0);
    } else {
        ng_blit_rect_fit(checkbox->background_image, checkbox->x + x_off, checkbox->y + y_off, checkbox->w, checkbox->h, 0, (checkbox->color >> 24) & 0xffU);
    }
    //Draw foreground
    if ( checkbox->flags & GITEMF_CHECKED ) {
        if ( git->flags & GITEMF_FILL_IMAGE ) {
            ng_blit_rect_fit(checkbox->foreground_image, checkbox->x + x_off, checkbox->y + y_off, checkbox->w, checkbox->h, 0, (checkbox->color >> 24) & 0xffU);
        } else {
            int offset = checkbox->w > checkbox->h ? checkbox->w/6 : checkbox->h/6;
            offset = round_up(offset, 2); //round up to nearest even numbder
            ng_fill_rect(checkbox->x + x_off + offset, checkbox->y + y_off + offset, checkbox->w - 2*offset, checkbox->h - 2*offset, color, 0);
        }
    }
}

void ng_checkbox_set_image(gitem_base_t *git, void *asset_ptr) {
    gitem_checkbox_t *checkbox = NG_CHECKBOX(git);
	checkbox->foreground_image = (img_obj_t*) asset_ptr;
}

void ng_checkbox_set_secondary_color(gitem_base_t *git, uint32_t rgba) {
    gitem_checkbox_t *checkbox = NG_CHECKBOX(git);
	checkbox->secondary_color = rgba;
}
