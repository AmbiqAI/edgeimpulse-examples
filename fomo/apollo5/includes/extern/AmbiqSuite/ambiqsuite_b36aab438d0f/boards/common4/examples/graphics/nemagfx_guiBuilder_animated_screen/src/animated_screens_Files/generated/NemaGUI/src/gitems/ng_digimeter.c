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

#include "ng_digimeter.h"
#include "ng_event.h"
#include "ng_utils.h"
#include "nema_font.h"

DRAW_FUNC(ng_digimeter_draw)
{
	gitem_digimeter_t *digimeter = NG_DIGIMETER(git);

    if (git->flags & GITEMF_FILL_COLOR){
    	ng_fill_rect(digimeter->x + x_off, digimeter->y + y_off, digimeter->w, digimeter->h, digimeter->color, 0);
    }

    nema_bind_font(digimeter->font);
    nema_set_blend_fill(NEMA_BL_SRC);
    const int max_chars = 30;
    char text[30];
    float2str(text, max_chars, digimeter->value, digimeter->dec_precision);

    if ( (digimeter->int_precision > 0) && (digimeter->value < digimeter->max_value) ) {
        float val = digimeter->value;
        int trail_zeros = 0;
        for ( int i = 0; i < digimeter->int_precision; ++i ) {
            val = val*10;
            if ( val < digimeter->max_value ) {
                trail_zeros++;
            }
        }

        append_trailing_zeros(text, max_chars, trail_zeros);
    }

    if ( digimeter->suffix != NULL ) {
        concatenate_strings(text, digimeter->suffix);
    }

    nema_print(text, digimeter->x + x_off, digimeter->y + y_off, digimeter->w, digimeter->h, digimeter->text_color, digimeter->alignment);
}

static void ng_digimeter_change_value(gitem_digimeter_t *digimeter, float val) {
    float val_init = digimeter->value;
    val = CLAMP(val, digimeter->min_value, digimeter->max_value);
    digimeter->value = val;

    if ( ( val != val_init ) && ( digimeter->event != NULL ) ) {
        ng_event_handle(digimeter->event, EV_TRIGGER_VALUE_CHANGED);
    }
}

void ng_digimeter_count_up(gitem_base_t *git) {
    gitem_digimeter_t *digimeter = NG_DIGIMETER(git);
    ng_digimeter_change_value(digimeter, digimeter->value + digimeter->step);
}

void ng_digimeter_count_down(gitem_base_t *git) {
    gitem_digimeter_t *digimeter = NG_DIGIMETER(git);
    ng_digimeter_change_value(digimeter, digimeter->value - digimeter->step);
}

void ng_digimeter_set_value(gitem_base_t *git, float value) {
    gitem_digimeter_t *digimeter = NG_DIGIMETER(git);
    ng_digimeter_change_value(digimeter, value);
}

void ng_digimeter_set_percent(gitem_base_t *git, float percent) {
    gitem_digimeter_t *digimeter = NG_DIGIMETER(git);
    ng_digimeter_change_value(digimeter, percent*(digimeter->max_value - digimeter->min_value) + digimeter->min_value);
}

void ng_digimeter_set_text_color(gitem_base_t *git, uint32_t rgba) {
    gitem_digimeter_t *digimeter = NG_DIGIMETER(git);
	digimeter->text_color = rgba;
}
