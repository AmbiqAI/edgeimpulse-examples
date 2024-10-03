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

#include "ng_label.h"
#include "ng_rect.h"
#include "ng_draw_prim.h"

DRAW_FUNC(ng_label_draw){
	gitem_label_t *label = NG_LABEL(git);

    if (git->flags & GITEMF_FILL_COLOR){
        ng_fill_rect(label->x, label->y, label->w, label->h, label->color, 0);
    }

    nema_bind_font(label->text->fonts[label->text->index]);
	nema_set_blend_fill(NEMA_BL_SRC);

    int font_x = git->x + x_off;
    int font_y = git->y + y_off;

    nema_print(label->text->texts[label->text->index], font_x, font_y, git->w, git->h, label->text_color, label->text->alignment);
}

void ng_label_set_text_color(gitem_base_t *git, uint32_t rgba) {
    gitem_label_t *label = NG_LABEL(git);
	label->text_color = rgba;
}
