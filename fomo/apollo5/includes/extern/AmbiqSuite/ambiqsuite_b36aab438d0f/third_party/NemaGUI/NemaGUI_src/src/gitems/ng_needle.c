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

#include "ng_needle.h"
#include "ng_draw_prim.h"

void nema_set_raster_color(uint32_t rgba8888);

DRAW_FUNC(ng_needle_draw){
    gitem_needle_t *needle = NG_NEEDLE(git);

    if((needle->color & 0xff000000) == 0){
        return;
    }

    //  P0-------P1
    //  | .rot    |
    //  P3-------P2
    //move rotation center to (0, 0)
    float x0 = -needle->x_rot;
    float y0 = -needle->y_rot;
    float x1 = x0 + git->w;
    float y1 = y0;
    float x2 = x1;
    float y2 = y1 + git->h;
    float x3 = x0;
    float y3 = y2;

    //translate rotation center to final coords
    int cx = git->x + x_off;
    int cy = git->y + y_off;

    //calculate rotation matrix
    nema_matrix3x3_t m;
    nema_mat3x3_load_identity(m);
    nema_mat3x3_rotate(m, needle->angle);
    nema_mat3x3_translate(m, cx, cy);

    //rotate points
    nema_mat3x3_mul_vec(m, &x0, &y0);
    nema_mat3x3_mul_vec(m, &x1, &y1);
    nema_mat3x3_mul_vec(m, &x2, &y2);
    nema_mat3x3_mul_vec(m, &x3, &y3);

    if ( git->flags & GITEMF_FILL_IMAGE ) {
        if(needle->image == NULL){
            return;
        }

        ng_blit_quad_fit( needle->image,
                          x0, y0,
                          x1, y1,
                          x2, y2,
                          x3, y3, 0,
                          (needle->color & 0xff000000) >> 24);
    } else {
        ng_fill_quad(x0, y0,
                     x1, y1,
                     x2, y2,
                     x3, y3, 0,
                     needle->color);
    }
}

void ng_needle_set_image(gitem_base_t *git, void *asset_ptr) {
    gitem_needle_t *needle = NG_NEEDLE(git);
	needle->image = (img_obj_t*) asset_ptr;
}
