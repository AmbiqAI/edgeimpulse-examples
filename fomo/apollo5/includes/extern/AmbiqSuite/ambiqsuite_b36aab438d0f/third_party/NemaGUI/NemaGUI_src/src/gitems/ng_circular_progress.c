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

#include "ng_globals.h"
#include "ng_circular_progress.h"
#include "ng_draw_prim.h"
#include "nema_raster.h"

static GESTURE_FUNC_PRESS(press)
{
    float R       = node->this_->h >> 1;
    float vX      = x - R;
    float vY      = y - R;
    float magV    = nema_sqrt(vX*vX + vY*vY);
    float temp_x  = R + vX / magV * R;
    float temp_y  = R + vY / magV * R;

    float angle   = 0.f;
    float nom     = R - temp_y;
    float denom   = R - temp_x;

    if (denom == 0 )
        angle = 90; //will be mapped to 270 later
    else {
        angle = nema_atan(nom/denom);
    }

    //Map angle to [0, 360]
    //1st quartile [0 , 90]
    if((temp_x <= R) && (temp_y <= R)){
        angle = angle + 270;
    //2nd quartile [90 , 180]
    }else if((temp_x >= R) && (temp_y <= R)){
        angle = angle + 90;
    //3rd quartile [180 , 270]
    }else if((temp_x >= R) && (temp_y >= R)){
        angle = angle + 90;
    //4th quartile [270 , 360]
    }else if ((temp_x <= R) && (temp_y >= R)){
        angle = angle + 270;
    }

    gitem_circular_progress_t *circ_prog = NG_CIRCULAR_PROGRESS(node->this_);
    // circ_prog->angle = angle;

    if(circ_prog->min_angle >= 0 && (angle >= circ_prog->min_angle && angle <= circ_prog->max_angle)){
        circ_prog->value = (angle-circ_prog->min_angle)/(circ_prog->max_angle - circ_prog->min_angle);

        if (node->this_->event){
            ng_event_handle(node->this_->event, EV_TRIGGER_VALUE_CHANGED);
        }

    }else if(circ_prog->min_angle < 0 || circ_prog->max_angle < 0){
        if((angle + 360 >= circ_prog->min_angle + 360 && angle + 360 <= circ_prog->max_angle + 360)){
            circ_prog->value = (angle - circ_prog->min_angle)/(circ_prog->max_angle - circ_prog->min_angle);
        }else if (angle - 360 >= circ_prog->min_angle && angle - 360 <= circ_prog->max_angle){
            circ_prog->value = (float) (angle - circ_prog->min_angle-360)/(circ_prog->max_angle - circ_prog->min_angle);
        }

        if (node->this_->event){
            ng_event_handle(node->this_->event, EV_TRIGGER_VALUE_CHANGED);
        }
    }
}

static GESTURE_FUNC_SWIPE(swipe)
{
    (void)dx;
    (void)dy;
    press(node, x, y);
}

gitem_gestures_t gestures_circular_progress = {press, NULL, swipe, NULL};

DRAW_FUNC(ng_circular_progress_draw)
{
    gitem_circular_progress_t *circ_prog = NG_CIRCULAR_PROGRESS(git);

    // Draw background texture
    if (circ_prog->background_image != NULL) {

        //TODO: Draw prim image
        nema_bind_src_tex(circ_prog->background_image->bo.base_phys, circ_prog->background_image->w,      circ_prog->background_image->h,
                          circ_prog->background_image->format,       circ_prog->background_image->stride, circ_prog->background_image->sampling_mode);

        uint8_t opacity = (circ_prog->color & 0xff000000) >> 24;
        if(opacity < 0xff){
            nema_set_blend_blit(NEMA_BL_SIMPLE | NEMA_BLOP_MODULATE_A);
            //Set const color needed for the modulation (needed by the fragment processor)
            nema_set_const_color((circ_prog->background_image->color & 0x00ffffff) | (opacity << 24));
        }else{
            nema_set_blend_blit(NEMA_BL_SIMPLE);
        }

        //Colorize A8 texture
        if(circ_prog->background_image->format == NEMA_A8){
            nema_set_tex_color((circ_prog->background_image->color & 0x00ffffff) | (opacity << 24));
        }

        ng_blit_rect_fit(circ_prog->background_image, git->x + x_off, git->y + y_off, git->w, git->h, 0, opacity);

    }

    //Draw foreground texture
    if (circ_prog->foreground_image == NULL){
        return;
    }

    int img_w = circ_prog->foreground_image->w;
    int img_h = circ_prog->foreground_image->h;


    nema_interpolate_tx_ty( git->x+x_off       , git->y+y_off       , 1.f,   0.f,   0.f,
                            git->x+x_off+git->w, git->y+y_off       , 1.f, img_w,   0.f,
                            git->x+x_off+git->w, git->y+y_off+git->h, 1.f, img_w, img_h,
                            img_w, img_h);

    uint8_t opacity = (circ_prog->color & 0xff000000) >> 24;
    if(opacity < 0xff){
        nema_set_blend_blit(NEMA_BL_SIMPLE | NEMA_BLOP_MODULATE_A);
        //Set const color needed for the modulation (needed by the fragment processor)
        nema_set_const_color((circ_prog->foreground_image->color & 0x00ffffff) | (opacity << 24));
    }else{
        nema_set_blend_blit(NEMA_BL_SIMPLE);
    }

    if(circ_prog->foreground_image->format == NEMA_A8){
        nema_set_tex_color(circ_prog->foreground_image->color);
    }

    nema_bind_src_tex(circ_prog->foreground_image->bo.base_phys, circ_prog->foreground_image->w,      circ_prog->foreground_image->h,
                      circ_prog->foreground_image->format,       circ_prog->foreground_image->stride, circ_prog->foreground_image->sampling_mode);

    int start_angle = circ_prog->min_angle + 90;
    int end_angle   = circ_prog->value*(circ_prog->max_angle - circ_prog->min_angle) + circ_prog->min_angle + 90;

    //Works also without if-sttement
    if(start_angle < 0 || end_angle < 0){
        start_angle += 360;
        end_angle   += 360;
    }

    if (end_angle < start_angle){
        end_angle += 360;
    }

    int start_quartile = ((start_angle+360+45))/90;
    int end_quartile   = ((end_angle +360+45))/90;

    if (start_quartile == end_quartile && end_angle > (start_quartile*90)+45 )
        end_quartile += 4;

    if (end_quartile < start_quartile)
        end_quartile += 4;

    int start_x, start_y;
    int end_x, end_y;

    switch (start_quartile%4) {
    case 0:
        start_x = 0;
        start_y = 0.5*(git->h - nema_tan(start_angle)*git->w);
        break;
    case 1:
        start_x = 0.5f*(git->w + nema_tan(start_angle-90)*git->h);
        start_y = 0;
        break;
    case 2:
       start_x = git->w;
       start_y = 0.5f*(git->h + nema_tan(start_angle)*git->w);
       break;
    case 3:
    default:
       start_x = 0.5f*(git->w - nema_tan(start_angle-90)*git->h);
       start_y = git->h;
       break;
    }

    switch (end_quartile%4) {
    case 0:
        end_x = 0;
        end_y = 0.5*(git->h - nema_tan(end_angle)*git->w);
       break;
    case 1:
        end_x = 0.5f*(git->w + nema_tan(end_angle-90)*git->h);
        end_y = 0;
       break;
    case 2:
       end_x = git->w;
       end_y = 0.5f*(git->h + nema_tan(end_angle)*git->w);
       break;
    case 3:
    default:
       end_x = 0.5f*(git->w - nema_tan(end_angle-90)*git->h);
       end_y = git->h;
       break;
    }

    int i  = start_quartile;

    int x0 = start_x;
    int y0 = start_y;

    int x1 = 0;
    int y1 = 0;

    for ( ; i <= end_quartile; ++i ) {
        if (i != end_quartile) {
            switch (i%4) {
            case 0:
                x1 = 0;
                y1 = 0;
                break;
            case 1:
                x1 = git->w;
                y1 = 0;
                break;
            case 2:
                x1 = git->w;
                y1 = git->h;
                break;
            case 3:
            default:
                x1 = 0;
                y1 = git->h;
                break;
            }
        } else {
           x1 = end_x;
           y1 = end_y;
        }


        nema_raster_triangle_fx( nema_i2fx(git->x + x_off + git->w/2), nema_i2fx(git->y + y_off + git->h/2),
                                 nema_i2fx(git->x + x_off + x0),       nema_i2fx(git->y + y_off + y0),
                                 nema_i2fx(git->x + x_off + x1),       nema_i2fx(git->y + y_off + y1));

        x0 = x1;
        y0 = y1;
    }
}

void ng_circular_progress_set_percent(gitem_base_t *git, float percent) {
    gitem_circular_progress_t *circ_progress = NG_CIRCULAR_PROGRESS(git);
    float init_value = circ_progress->value;
    float value =  CLAMP(percent, 0.f, 1.f);

    circ_progress->value = value;

    if ( (init_value != value) && ( circ_progress->event ) ) {    
        ng_event_handle(circ_progress->event, EV_TRIGGER_VALUE_CHANGED);
    }
}

void ng_circular_progress_set_background_image(gitem_base_t *git, void *asset_ptr) {
    gitem_circular_progress_t *circ_progress = NG_CIRCULAR_PROGRESS(git);
    circ_progress->background_image = asset_ptr;
}

void ng_circular_progress_set_foreground_image(gitem_base_t *git, void *asset_ptr) {
    gitem_circular_progress_t *circ_progress = NG_CIRCULAR_PROGRESS(git);
    circ_progress->foreground_image = asset_ptr;
}
