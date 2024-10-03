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
#include "ng_gauge.h"
#include "ng_needle.h"
#include "ng_draw_prim.h"

// void press_gauge(tree_node_t *node, int x, int y)
GESTURE_FUNC_PRESS(press_gauge)
{
    gitem_gauge_t   *gauge = NG_GAUGE(node->this_);
    gitem_needle_t *needle = NG_NEEDLE(gauge->needle);

    //Map needle x,y to the closest circle point
    float R       = needle->y_rot;
    float vX      = x - gauge->x_rot;
    float vY      = y - gauge->y_rot;
    float magV    = nema_sqrt(vX*vX + vY*vY);
    float temp_x  = gauge->x_rot + vX / magV * R;
    float temp_y  = gauge->y_rot + vY / magV * R;

    float angle   = 0.f;
    float nom     = gauge->y_rot - temp_y;
    float denom   = gauge->x_rot - temp_x;

    //**************    TODO    ********************
    //a) optimize if-statements => merge equivalent conditions
    //b) avoid divisions with const (i.e. ..../(max_angle-min_angle)), precalculate denominator and multiply with the inverse of it
    if(denom == 0 && (temp_y <= gauge->y_rot)) {
        angle = 90;
    }
    else if(denom == 0 && (temp_y > gauge->y_rot)) {
        angle = 90; //will be mapped to 270 later
    }
    else {
        // angle = (int) nema_rad_to_deg(atan(nom/denom));
        angle = nema_atan(nom/denom);
    }

    //Map angle to [0, 360]
    //1st quartile [0 , 90]
    if((temp_x <= gauge->x_rot) && (temp_y <= gauge->y_rot)){
        angle = angle + 270;
    //2nd quartile [90 , 180]
    }else if((temp_x >= gauge->x_rot) && (temp_y <= gauge->y_rot)){
        angle = angle + 90;
    //3rd quartile [180 , 270]
    }else if((temp_x >= gauge->x_rot) && (temp_y >= gauge->y_rot)){
        angle = angle + 90;
    //4th quartile [270 , 360]
    }else if ((temp_x <= gauge->x_rot) && (temp_y >= gauge->y_rot)){
        angle = angle + 270;
    }

    // Transform angles in a form conformant with: min_angle < angle < max_angle
    // and set gauge/needle arguments (angle, val) only when the previous condition holds
    if ( (gauge->min_angle >= 0) && (gauge->max_angle >=0) ) {

        if ( (angle >= gauge->min_angle) && (angle <= gauge->max_angle) ) {
            needle->angle = angle;
            gauge->value = gauge->max_value*(needle->angle-gauge->min_angle)/(gauge->max_angle - gauge->min_angle);

            if (node->this_->event) {
                ng_event_handle(node->this_->event, EV_TRIGGER_VALUE_CHANGED);
            }
        }

    } else { // if(needle->min_angle < 0 || needle->max_angle < 0){

        if((angle >= gauge->min_angle       && angle <= gauge->max_angle) ||
           (angle >= gauge->min_angle + 360 && angle <= gauge->max_angle + 360)){
            needle->angle = angle;

            if(angle >= gauge->min_angle && angle <= gauge->max_angle) {
                gauge->value = (gauge->max_value*(needle->angle - gauge->min_angle) + gauge->min_value*(gauge->max_angle - needle->angle))/(gauge->max_angle - gauge->min_angle);
            }

            // phase shift by 360 degrees required: min_angle += 360, max_angle += 360
            else {
                gauge->value = (gauge->max_value*(needle->angle - gauge->min_angle - 360) + gauge->min_value*(gauge->max_angle + 360 - needle->angle))/(gauge->max_angle - gauge->min_angle);
            }

            if (node->this_->event) {
                ng_event_handle(node->this_->event, EV_TRIGGER_VALUE_CHANGED);
            }
        }
    }
}


// void swipe_gauge(tree_node_t *node, int x, int y, int dx, int dy)
GESTURE_FUNC_SWIPE(swipe_gauge)
{
    (void)dx;
    (void)dy;
    press_gauge(node, x, y);
}

gitem_gestures_t gestures_gauge = {press_gauge, NULL, swipe_gauge, NULL};

DRAW_FUNC(ng_gauge_draw){
    gitem_gauge_t *gauge = NG_GAUGE(git);

    if ( gauge->flags & GITEMF_FILL_IMAGE ) {
        ng_blit_rect_fit(gauge->image, gauge->x + x_off, gauge->y + y_off, gauge->w, gauge->h, 0, (gauge->color & 0xff000000) >> 24);
    } else {
        if ( gauge->flags & GITEMF_FILL_COLOR ) {
            ng_fill_rect(gauge->x + x_off, gauge->y + y_off, gauge->w, gauge->h, gauge->color, 0);
        } else {
            ng_draw_primitive_rect(gauge->x + x_off, gauge->y + y_off, gauge->w, gauge->h, gauge->color, gauge->pen_width, 0);
        }
    }
}

static void ng_gauge_change_value(gitem_gauge_t *gauge, float value) {
    float init_value = gauge->value;
    gauge->value = value;

    if ( value != init_value ) {    
        gitem_needle_t *needle = NG_NEEDLE(gauge->needle);
        needle->angle = gauge->min_angle  + (gauge->max_angle - gauge->min_angle) * (value - gauge->min_value)/(gauge->max_value - gauge->min_value); // min + range*percentage

        if (gauge->event) {
            ng_event_handle(gauge->event, EV_TRIGGER_VALUE_CHANGED);
        }
    }
}

void ng_gauge_set_value(gitem_base_t *git, float value) {
    gitem_gauge_t *gauge = NG_GAUGE(git);
    ng_gauge_change_value(gauge, value);
}

void ng_gauge_set_percent(gitem_base_t *git, float percent) {
    gitem_gauge_t *gauge = NG_GAUGE(git);
    ng_gauge_change_value(gauge, percent*(gauge->max_value - gauge->min_value) + gauge->min_value);
}

void ng_gauge_set_image(gitem_base_t *git, void *asset_ptr) {
    gitem_gauge_t *gauge = NG_GAUGE(git);
	gauge->image = (img_obj_t*) asset_ptr;
}
