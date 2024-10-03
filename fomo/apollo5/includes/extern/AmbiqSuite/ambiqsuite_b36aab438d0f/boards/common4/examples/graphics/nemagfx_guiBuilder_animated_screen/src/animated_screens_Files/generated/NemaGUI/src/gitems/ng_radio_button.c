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

#include "ng_radio_button.h"
#include "ng_draw_prim.h"
#include "ng_event.h"

static GESTURE_FUNC_PRESS(press) {
    (void)x;
    (void)y;
	ng_gitem_set_flag(node->this_, GITEMF_HIGHLIGHTED);
}

static GESTURE_FUNC_RELEASE(release) {
    ng_gitem_unset_flag(node->this_, GITEMF_HIGHLIGHTED);

    tree_node_t *table = node;
    do {
        table = table->parent;
    } while(table->this_->type != GITEM_TABLE);

    ng_radio_button_toggle(table->first_child);
    ng_gitem_set_flag(node->this_, GITEMF_CHECKED);
}

gitem_gestures_t gestures_radio_button = {press, release, NULL, NULL};

DRAW_FUNC(ng_radio_button_draw){
    gitem_radio_button_t *radio_button = NG_RADIO_BUTTON(git);

    uint32_t color = radio_button->color;
    if (radio_button->flags & GITEMF_HIGHLIGHTED){
        color = radio_button->secondary_color;
    }

    if ( (color & 0xff000000) == 0){
        return;
    }


    if ( radio_button->flags & GITEMF_FILL_IMAGE ) {
        if ( radio_button->flags & GITEMF_CHECKED ) {
            ng_blit_rect_fit(radio_button->foreground_image, radio_button->x + x_off, radio_button->y + y_off, radio_button->w, radio_button->h, 0, (color >> 24) & 0xffU);
        } else {
            ng_blit_rect_fit(radio_button->background_image, radio_button->x + x_off, radio_button->y + y_off, radio_button->w, radio_button->h, 0, (color >> 24) & 0xffU);
        }

    } else {
        if((color & 0xff000000) == 0xff000000 ){
            nema_set_blend_fill(NEMA_BL_SRC);
        } else {
            nema_set_blend_fill(NEMA_BL_SIMPLE);
        }

        const int r_out  = radio_button->w>>1;
        const int x0     = radio_button->x + x_off + r_out;
        const int y0     = radio_button->y + y_off + r_out;

        nema_draw_circle(x0, y0, r_out, color);

        if ( radio_button->flags & GITEMF_CHECKED ) {
            nema_fill_circle(x0, y0, radio_button->radius, color);
        }
    }
}

void ng_radio_button_toggle(tree_node_t *node){
	while (node && node->this_) {
        if (node->this_->type == GITEM_RADIO_BUTTON) {
            ng_gitem_unset_flag(node->this_, GITEMF_CHECKED);
        }

        if (node->first_child) {
            ng_radio_button_toggle(node->first_child);
        }

        //go to next item at the same level;
        node = node->next;
    }
}

void ng_radio_button_set_secondary_color(gitem_base_t *git, uint32_t rgba) {
    gitem_radio_button_t *radio_button = NG_RADIO_BUTTON(git);
	radio_button->secondary_color = rgba;
}
