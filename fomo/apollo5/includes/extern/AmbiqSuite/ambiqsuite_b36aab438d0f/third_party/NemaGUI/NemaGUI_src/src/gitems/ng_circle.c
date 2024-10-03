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

#include "ng_circle.h"
#include "ng_draw_prim.h"

DRAW_FUNC(ng_circle_draw){
	gitem_circle_t *circle = SAFE_CAST(git, gitem_base_t *, gitem_circle_t *);

    const int r  = git->w>>1;
    const int x0 = git->x + x_off + r;
    const int y0 = git->y + y_off + r;

    if (circle->flags & GITEMF_FILL_COLOR){
        ng_fill_primitive_circle(x0, y0, r, circle->color, 0);
    }else{
        ng_draw_primitive_circle(x0, y0, r, circle->color, 0);
    }
}