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

#include "ng_progress_bar.h"
#include "ng_draw_prim.h"
#include "ng_event.h"
#include "nema_raster.h"
#include "nema_interpolators.h"

DRAW_FUNC(ng_horizontal_progress_bar_draw){

    gitem_progress_bar_t *prog_bar = NG_PROGRESS_BAR(git);

    int foreground_width = (int) (prog_bar->w*prog_bar->value);
    uint8_t opacity      = (prog_bar->color & 0xff000000) >> 24;

    if ( prog_bar->flags & GITEMF_FILL_COLOR  ){
        uint32_t color =  git->color;

        if ( opacity == 0xff ) {
            nema_set_blend_fill(NEMA_BL_SRC);
        } else {
            nema_set_blend_fill(NEMA_BL_SIMPLE);
        }

        //draw background
        nema_fill_rect(prog_bar->x + x_off, prog_bar->y + y_off, prog_bar->w, prog_bar->h, color);
        //draw foreground
        nema_fill_rect(prog_bar->x + x_off, prog_bar->y + y_off, foreground_width, prog_bar->h, prog_bar->foreground_color);

    } else {
        //draw background
        
	    ng_blit_rect_fit(prog_bar->background_image, prog_bar->x + x_off, prog_bar->y + y_off, prog_bar->w, prog_bar->h, 0, opacity);

        //draw foreground
        if ( prog_bar->foreground_image == NULL ) {
            return;
        }
        img_obj_t *foreground = prog_bar->foreground_image;

        int img_w = foreground->w;
        int img_h = foreground->h;
        nema_interpolate_tx_ty( prog_bar->x+x_off            , prog_bar->y+y_off            , 1.f,   0.f,   0.f,
                                prog_bar->x+x_off+prog_bar->w, prog_bar->y+y_off            , 1.f, img_w,   0.f,
                                prog_bar->x+x_off+prog_bar->w, prog_bar->y+y_off+prog_bar->h, 1.f, img_w, img_h,
                                img_w, img_h);


        if ( opacity < 0xff ) {
            nema_set_blend_blit(NEMA_BL_SIMPLE|NEMA_BLOP_MODULATE_A);
            nema_set_const_color(git->color);
        } else {
            nema_set_blend_blit(NEMA_BL_SIMPLE);
        }
        
        //Colorize A8 images
        if ( foreground->format == NEMA_A8 ) {
            nema_set_tex_color(foreground->color);
        }

        nema_bind_src_tex(foreground->bo.base_phys, foreground->w, foreground->h,
                          foreground->format, foreground->stride, foreground->sampling_mode);

        
        //Rect clockwise coordinates are: (A, B, C, D)  => first triangle (A, B, D), second triangle (B, C, D)
        //draw the first triangle
        nema_raster_triangle_fx( nema_i2fx(prog_bar->x + x_off)                   , nema_i2fx(prog_bar->y + y_off), 
                                 nema_i2fx(prog_bar->x + x_off + foreground_width), nema_i2fx(prog_bar->y + y_off),
                                 nema_i2fx(prog_bar->x + x_off)                   , nema_i2fx(prog_bar->y + y_off + prog_bar->h));

        //draw the second triangle
        nema_raster_triangle_fx( nema_i2fx(prog_bar->x + x_off + foreground_width), nema_i2fx(prog_bar->y + y_off),
                                 nema_i2fx(prog_bar->x + x_off + foreground_width), nema_i2fx(prog_bar->y + y_off + prog_bar->h),
                                 nema_i2fx(prog_bar->x + x_off)                   , nema_i2fx(prog_bar->y + y_off + prog_bar->h));
    }
}

DRAW_FUNC(ng_vertical_progress_bar_draw){
    gitem_progress_bar_t *prog_bar = NG_PROGRESS_BAR(git);

    int foreground_height = (int) (prog_bar->h*prog_bar->value);
    uint8_t opacity       = (prog_bar->color & 0xff000000) >> 24;

    if ( prog_bar->flags & GITEMF_FILL_COLOR  ){
        uint32_t color =  git->color;

        if ( opacity == 0xff ) {
            nema_set_blend_fill(NEMA_BL_SRC);
        } else {
            nema_set_blend_fill(NEMA_BL_SIMPLE);
        }

        //draw background
        nema_fill_rect(prog_bar->x + x_off, prog_bar->y + y_off, prog_bar->w, prog_bar->h, color);
        //draw foreground
        nema_fill_rect(prog_bar->x + x_off, prog_bar->y + y_off + prog_bar->h - foreground_height, prog_bar->w, foreground_height, prog_bar->foreground_color);
    } else {
        //draw background
        
	    ng_blit_rect_fit(prog_bar->background_image, prog_bar->x + x_off, prog_bar->y + y_off, prog_bar->w, prog_bar->h, 0, opacity);

        //draw foreground
        if ( prog_bar->foreground_image == NULL ) {
            return;
        }
        img_obj_t *foreground = prog_bar->foreground_image;

        int img_w = foreground->w;
        int img_h = foreground->h;
        nema_interpolate_tx_ty( git->x+x_off       , git->y+y_off       , 1.f,   0.f,   0.f,
                                git->x+x_off+git->w, git->y+y_off       , 1.f, img_w,   0.f,
                                git->x+x_off+git->w, git->y+y_off+git->h, 1.f, img_w, img_h,
                                img_w, img_h);

        if ( opacity == 0xff ) {
            nema_set_blend_blit(NEMA_BL_SIMPLE);
        } else {
            nema_set_blend_blit(NEMA_BL_SIMPLE|NEMA_BLOP_MODULATE_A);
            nema_set_const_color(git->color);
        }
        
        //Colorize A8 images
        if ( foreground->format == NEMA_A8 ) {
            nema_set_tex_color(foreground->color);
        }

        nema_bind_src_tex(foreground->bo.base_phys, foreground->w, foreground->h,
                          foreground->format, foreground->stride, foreground->sampling_mode);

        //Rect clockwise coordinates are: (A, B, C, D)  => first triangle (A, C, D), second triangle (A, B, C)
        //draw the first triangle
        nema_raster_triangle_fx( nema_i2fx(prog_bar->x + x_off)              , nema_i2fx(prog_bar->y + y_off + prog_bar->h - foreground_height), 
                                 nema_i2fx(prog_bar->x + x_off + prog_bar->w), nema_i2fx(prog_bar->y + y_off + prog_bar->h),
                                 nema_i2fx(prog_bar->x + x_off)              , nema_i2fx(prog_bar->y + y_off + prog_bar->h));

        //draw the second triangle
        nema_raster_triangle_fx( nema_i2fx(prog_bar->x + x_off)              , nema_i2fx(prog_bar->y + y_off + prog_bar->h - foreground_height),
                                 nema_i2fx(prog_bar->x + x_off + prog_bar->w), nema_i2fx(prog_bar->y + y_off + prog_bar->h - foreground_height),
                                 nema_i2fx(prog_bar->x + x_off + prog_bar->w), nema_i2fx(prog_bar->y + y_off + prog_bar->h));
    }
}

void ng_progress_bar_set_percent(gitem_base_t *git, float percent) {
    gitem_progress_bar_t *progress_bar = NG_PROGRESS_BAR(git);
    float init_value = progress_bar->value;
    progress_bar->value = CLAMP(percent, 0.f, 1.f);

    if ( ( progress_bar->value != init_value ) && ( progress_bar->event ) ) {
        ng_event_handle(progress_bar->event, EV_TRIGGER_VALUE_CHANGED);
    }
}

void ng_progress_bar_set_background_image(gitem_base_t *git, void *asset_ptr) {
    gitem_progress_bar_t *prog_bar = NG_PROGRESS_BAR(git);
	prog_bar->background_image = (img_obj_t*) asset_ptr;
}

void ng_progress_bar_set_foreground_image(gitem_base_t *git, void *asset_ptr) {
    gitem_progress_bar_t *prog_bar = NG_PROGRESS_BAR(git);
	prog_bar->foreground_image = (img_obj_t*) asset_ptr;
}

void ng_progress_bar_set_foreground_color(gitem_base_t *git, uint32_t rgba) {
    gitem_progress_bar_t *prog_bar = NG_PROGRESS_BAR(git);
	prog_bar->foreground_color = rgba;
}
