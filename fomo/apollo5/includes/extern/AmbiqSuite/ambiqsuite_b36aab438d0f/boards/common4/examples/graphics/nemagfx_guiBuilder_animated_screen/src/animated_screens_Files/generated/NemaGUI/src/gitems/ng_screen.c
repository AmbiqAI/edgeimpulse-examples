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

#include "ng_screen.h"
#include "ng_draw_prim.h"
#include "ng_globals.h"

DRAW_FUNC(ng_screen_draw){
	gitem_screen_t *screen = NG_SCREEN(git);
    if(git->flags & GITEMF_FILL_IMAGE){
        ng_blit_rect_fit(screen->image, screen->x + x_off, screen->y + y_off, screen->w, screen->h, 0, 0xffU);
    }else{
        ng_fill_rect(screen->x + x_off, screen->y + y_off, screen->w, screen->h, screen->color, 0);
    }
}

void ng_screen_set_image(gitem_base_t *git, void *asset_ptr) {
    gitem_screen_t *screen = NG_SCREEN(git);
	screen->image = (img_obj_t*) asset_ptr;
}
