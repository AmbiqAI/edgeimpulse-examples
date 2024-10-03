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

#include "ng_button.h"
#include "ng_draw_prim.h"

DRAW_FUNC(ng_button_draw){
	gitem_button_t *button = NG_BUTTON(git);

    if ( button->flags & GITEMF_FILL_IMAGE ) {
    	img_obj_t *image = (button->flags & GITEMF_HIGHLIGHTED) ? button->secondary_image : button->primary_image;
        ng_blit_rect_fit(image, button->x + x_off, button->y + y_off, button->w, button->h, 0, (button->color >> 24) & 0xffU);
    } else {
    	uint32_t color = (button->flags & GITEMF_HIGHLIGHTED) ? button->secondary_color : button->color;
        	if ( button->flags & GITEMF_FILL_COLOR ) {
        		ng_fill_rect(button->x + x_off, button->y + y_off, button->w, button->h, color, 0);
        	} else {
        		ng_draw_primitive_rect(button->x + x_off, button->y + y_off, button->w, button->h, color, button->pen_width, 0);
        	}
    }
}

void ng_button_set_primary_image(gitem_base_t *git, void *asset_ptr) {
	gitem_button_t *button = NG_BUTTON(git);
	button->primary_image = (img_obj_t*) asset_ptr;
}

void ng_button_set_secondary_image(gitem_base_t *git, void *asset_ptr) {
	gitem_button_t *button = NG_BUTTON(git);
	button->secondary_image = (img_obj_t*) asset_ptr;
}

void ng_button_set_secondary_color(gitem_base_t *git, uint32_t rgba) {
	gitem_button_t *button = NG_BUTTON(git);
	button->secondary_color = rgba;
}
