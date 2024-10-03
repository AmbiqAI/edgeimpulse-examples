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

#include "ng_container.h"
#include "ng_draw_prim.h"

DRAW_FUNC(ng_container_draw){
	gitem_container_t *container = SAFE_CAST(git, gitem_base_t *, gitem_container_t *);

    if ( container->flags & GITEMF_FILL_IMAGE ) {
        ng_blit_rect_fit(container->image, container->x + x_off, container->y + y_off, container->w, container->h, 0, (container->color & 0xff000000U) >> 24);
    } else {
        if ( container->flags & GITEMF_FILL_COLOR ) {
            ng_fill_rect(container->x + x_off, container->y + y_off, container->w, container->h, container->color, 0);
        } else {
            ng_draw_primitive_rect(container->x + x_off, container->y + y_off, container->w, container->h, container->color, container->pen_width, 0);
        }
    }
}

void ng_container_set_image(gitem_base_t *git, void *asset_ptr) {
    gitem_container_t *container = NG_CONTAINER(git);
	container->image = (img_obj_t*) asset_ptr;
}
