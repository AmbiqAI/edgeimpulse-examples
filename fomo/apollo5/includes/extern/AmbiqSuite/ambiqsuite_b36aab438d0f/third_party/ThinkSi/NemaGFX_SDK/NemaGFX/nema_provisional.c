/* TSI 2023.gen */
// -----------------------------------------------------------------------------
// Copyright (c) 2008-23 Think Silicon Single Member PC
// Think Silicon Single Member PC Confidential Proprietary
// -----------------------------------------------------------------------------
//     All Rights reserved - Unpublished -rights reserved under
//         the Copyright laws of the European Union
//
//  This file includes the Confidential information of Think Silicon Single
//  Member PC The receiver of this Confidential Information shall not disclose
//  it to any third party and shall protect its confidentiality by using the
//  same degree of care, but not less then a reasonable degree of care, as the
//  receiver uses to protect receiver's own Confidential Information. The entire
//  notice must be reproduced on all authorized copies and copies may only be
//  made to the extent permitted by a licensing agreement from Think Silicon
//  Single Member PC.
//
//  The software/data is provided 'as is', without warranty of any kind,
//  expressed or implied, including but not limited to the warranties of
//  merchantability, fitness for a particular purpose and noninfringement. In no
//  event shall Think Silicon Single Member PC be liable for any claim, damages
//  or other liability, whether in an action of contract, tort or otherwise,
//  arising from, out of or in connection with the software.
//
//  For further information please contact:
//
//                    Think Silicon Single Member PC
//                    http://www.think-silicon.com
//                    Patras Science Park
//                    Rion Achaias 26504
//                    Greece
// -----------------------------------------------------------------------------

#include "nema_provisional.h"
#include "nema_raster.h"
#include "nema_math.h"
#include "nema_graphics.h"
#include "nema_rasterizer_intern.h"

#define ORIENTATION_COLLINEAR         0
#define ORIENTATION_CLOCKWISE         1
#define ORIENTATION_COUNTER_CLOCKWISE 2


static float orientation_subformula(float x0, float y0, float x1, float y1, float x2, float y2)
{
    float ret = (x1-x0)*(y2-y0) - (y1-y0)*(x2-x0); //P2-P0, P1-P0
    return ret;
}

// returns the orientation (collinear, clockwise, counter clockwise of three points)
static inline int get_orientation(float x0, float y0, float x1, float y1, float x2, float y2) {
    float val = (y1 - y0)*(x2 - x1) - (y2 - y1)*(x1 - x0);
    int   ret = 0;

    if ( nema_float_is_zero(val) ) {
        ret =  0;
    } else if ( val > 0.f ) {
        ret = 1;
    } else {
        ret = 2;
    }

    return ret;
}

static inline bool is_quad_clockwise(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3) {
    float d0 = (x1-x0)*(y1+y0);
    float d1 = (x2-x1)*(y2+y1);
    float d2 = (x3-x2)*(y3+y2);
    float d3 = (x0-x3)*(y0+y3);

    float sum = d0 + d1 +d2 + d3;

    bool ret = false;
    if ( sum > 0.f ) {
        ret = true;
    }

    return ret;
}

void nema_draw_triangle_aa(float x0, float y0, float x1, float y1, float x2, float y2, float border_width, uint32_t color)
{
    int orientation = get_orientation(x0, y0, x1, y1, x2, y2);

    if ( orientation == ORIENTATION_COLLINEAR ) {
    //    nema_draw_line_aa(x0, y0, x1, y1, border_width, color);
       return;
    }

    float dx0     = x1 - x0;
    float dy0     = y1 - y0;
    float length0 = nema_sqrt(dx0*dx0 + dy0*dy0);
    float dx1     = x2 - x1;
    float dy1     = y2 - y1;
    float length1 = nema_sqrt(dx1*dx1 + dy1*dy1);
    float dx2     = x0 - x2;
    float dy2     = y0 - y2;
    float length2 = nema_sqrt(dx2*dx2 + dy2*dy2);

    if ( nema_float_is_zero(length0) || nema_float_is_zero(length1) || nema_float_is_zero(length2) ) {
        return;
    } else {
        //MISRA
    }

    float perimeter  = length0 + length1 + length2;
    float incenter_x = (length1*x0 + length2*x1 + length0*x2)/perimeter;
    float incenter_y = (length1*y0 + length2*y1 + length0*y2)/perimeter;
    float max_border_width = 2.f*nema_absf( (x1 - x0)*(y0 - incenter_y) - (x0 - incenter_x)*(y1 - y0) )/length0;
    float _border_width = border_width;

    if ( _border_width > max_border_width ) {
        _border_width = max_border_width;
        if (_border_width <= 0.f) {
            return;
        }
    } else if ( _border_width < 0.f ) {
        _border_width = 1.f;
    } else {
        //MISRA
    }

    dx0 = 0.5f*_border_width*dx0/length0;
    dy0 = 0.5f*_border_width*dy0/length0;
    dx1 = 0.5f*_border_width*dx1/length1;
    dy1 = 0.5f*_border_width*dy1/length1;
    dx2 = 0.5f*_border_width*dx2/length2;
    dy2 = 0.5f*_border_width*dy2/length2;

    if ( orientation == ORIENTATION_CLOCKWISE ) {
        dx0 = - dx0;
        dy0 = - dy0;
        dx1 = - dx1;
        dy1 = - dy1;
        dx2 = - dx2;
        dy2 = - dy2;
    }

    //Find the parallel lines e0, e1, e2 (inner and outer)
    float e0_in_x0  = x0 - dy0;
    float e0_in_y0  = y0 + dx0;
    float e0_in_x1  = x1 - dy0;
    float e0_in_y1  = y1 + dx0;
    float e0_out_x0 = x0 + dy0;
    float e0_out_y0 = y0 - dx0;
    float e0_out_x1 = x1 + dy0;
    float e0_out_y1 = y1 - dx0;

    float e1_in_x0  = x1 - dy1;
    float e1_in_y0  = y1 + dx1;
    float e1_in_x1  = x2 - dy1;
    float e1_in_y1  = y2 + dx1;
    float e1_out_x0 = x1 + dy1;
    float e1_out_y0 = y1 - dx1;
    float e1_out_x1 = x2 + dy1;
    float e1_out_y1 = y2 - dx1;

    float e2_in_x0  = x2 - dy2;
    float e2_in_y0  = y2 + dx2;
    float e2_in_x1  = x0 - dy2;
    float e2_in_y1  = y0 + dx2;
    float e2_out_x0 = x2 + dy2;
    float e2_out_y0 = y2 - dx2;
    float e2_out_x1 = x0 + dy2;
    float e2_out_y1 = y0 - dx2;

    float x0_in  = 0.f;
    float y0_in  = 0.f;
    float x1_in  = 0.f;
    float y1_in  = 0.f;
    float x2_in  = 0.f;
    float y2_in  = 0.f;
    float x0_out = 0.f;
    float y0_out = 0.f;
    float x1_out = 0.f;
    float y1_out = 0.f;
    float x2_out = 0.f;
    float y2_out = 0.f;

    //calculate the crossing points
    (void) line_intersection_point(e0_in_x0, e0_in_y0, e0_in_x1, e0_in_y1, e2_in_x0, e2_in_y0, e2_in_x1, e2_in_y1, &x0_in, &y0_in);
    (void) line_intersection_point(e0_in_x0, e0_in_y0, e0_in_x1, e0_in_y1, e1_in_x0, e1_in_y0, e1_in_x1, e1_in_y1, &x1_in, &y1_in);
    (void) line_intersection_point(e1_in_x0, e1_in_y0, e1_in_x1, e1_in_y1, e2_in_x0, e2_in_y0, e2_in_x1, e2_in_y1, &x2_in, &y2_in);

    (void) line_intersection_point(e0_out_x0, e0_out_y0, e0_out_x1, e0_out_y1, e2_out_x0, e2_out_y0, e2_out_x1, e2_out_y1, &x0_out, &y0_out);
    (void) line_intersection_point(e0_out_x0, e0_out_y0, e0_out_x1, e0_out_y1, e1_out_x0, e1_out_y0, e1_out_x1, e1_out_y1, &x1_out, &y1_out);
    (void) line_intersection_point(e1_out_x0, e1_out_y0, e1_out_x1, e1_out_y1, e2_out_x0, e2_out_y0, e2_out_x1, e2_out_y1, &x2_out, &y2_out);

    uint32_t prev_aa = nema_enable_aa(1, 0, 0, 0);
    nema_set_raster_color(color);

    //prefer one of the inner points as the common point of the triangles (x1, y1)
    nema_raster_triangle_f(x0_in , y0_in , x1_in , y1_in , x0_out, y0_out);
    nema_raster_triangle_f(x0_out, y0_out, x1_out, y1_out, x1_in , y1_in );
    nema_raster_triangle_f(x1_in , y1_in , x2_in , y2_in , x1_out, y1_out);
    nema_raster_triangle_f(x1_out, y1_out, x2_out, y2_out, x2_in , y2_in );
    nema_raster_triangle_f(x2_in , y2_in , x0_in , y0_in , x2_out, y2_out);
    nema_raster_triangle_f(x2_out, y2_out, x0_out, y0_out, x0_in , y0_in );

    (void)nema_enable_aa_flags(prev_aa);
}

void nema_draw_rounded_rect_aa(float x, float y, float w, float h, float r, float border_width, uint32_t rgba8888) {
    float r_ = r;
    if ( 0.5f*w < r_) {
        r_ = 0.5f*w;
    }

    if ( 0.5f*h < r_) {
        r_ = 0.5f*h;
    }

    if ( (r_ <= 0.f) || (w <= 2.f) || (h <= 2.f)) {
        int xi = (int) x;
        int yi = (int) y;
        int wi = (int) w;
        int hi = (int) h;

        nema_draw_rect(xi, yi, wi, hi, rgba8888);
        return;
    }

    const float max_border_width = 0.5f*nema_max2(w, h);
    float _border_width = border_width;

    if ( border_width > max_border_width ) {
        _border_width = max_border_width;
    } else if ( border_width < 0.f ) {
        _border_width = 1.f;
    } else {
        //misra
    }

    if ( _border_width > 2.f*r_ ) {
        _border_width = 2.f*r_;
    }

    if ( r_ > 2.f*max_border_width ) {
        r_ = 2.f*max_border_width;
    }

    nema_set_raster_color(rgba8888);

    (void) nema_enable_aa(0, 1, 0, 1);
    //Draw left side
    float tx = x - 0.5f*_border_width;
    float ty = y + r_;
    nema_raster_rect_f(tx, ty, _border_width, h - 2.f*r_);

    //draw right side
    tx += w;
    nema_raster_rect_f(tx, ty, _border_width, h - 2.f*r_);

    (void) nema_enable_aa(1, 0, 1, 0);
    //draw top side
    tx = x + r_;
    ty = y - 0.5f*_border_width;
    nema_raster_rect_f(tx, ty, w - 2.f*r_, _border_width);
    //draw bottom side
    ty += h;
    nema_raster_rect_f(tx, ty, w - 2.f*r_, _border_width);

    //top left arc
    nema_raster_stroked_arc_aa( x + r_, y + r_, r_, _border_width, 180.f, 270.f);
    //top right arc
    nema_raster_stroked_arc_aa( x + w -  r_, y + r_, r_, _border_width, 270.f, 360.f);
    //bottom left arc
    nema_raster_stroked_arc_aa( x + r_, y + h - r_, r_, _border_width, 90.f, 180.f);
    //bottom right arc
    nema_raster_stroked_arc_aa( x + w - r_, y + h - r_, r_, _border_width, 0.f, 90.f);
}

static void fill_rounded_rect_corner(float center_x, float center_y,
                                     float angle_cos_theta, float angle_sin_theta, int steps,
                                     float step_cos_theta, float step_sin_theta, float radius
                                    )
{

    float v_x = center_x + radius * angle_cos_theta;
    float v_y = center_y + radius * angle_sin_theta;

    float ns = step_sin_theta * angle_cos_theta + step_cos_theta * angle_sin_theta;
    float nc = step_cos_theta * angle_cos_theta - step_sin_theta * angle_sin_theta;
    float angle_cos_theta_ = nc;
    float angle_sin_theta_ = ns;

    float v1_x = center_x + radius * angle_cos_theta_;
    float v1_y = center_y + radius * angle_sin_theta_;

    // print the first triangle
    nema_raster_triangle_f(center_x, center_y,
                        v_x, v_y,
                        v1_x, v1_y);


    // We create a triangle fan here.
    // basically, we copied the triangle_fan code here to avoid
    // allocating memory on the stack to pass as argument.
    // In triangle fan adjacent triangles have two vertices equal.
    // Vertice 0 is always the same in all triangles
    // Vertices 1 and 2 change alternately
    int count = 0;
    for(int i = 1; i < steps; i++){
        // advance the angle with the trig identity
        ns = step_sin_theta * angle_cos_theta_ + step_cos_theta * angle_sin_theta_;
        nc = step_cos_theta * angle_cos_theta_ - step_sin_theta * angle_sin_theta_;
        angle_cos_theta_ = nc;
        angle_sin_theta_ = ns;
#ifdef RASTERIZER_BUG_WA
        v1_x = v_x;
        v1_y = v_y;
#endif
        v_x = center_x + radius * angle_cos_theta_;
        v_y = center_y + radius * angle_sin_theta_;
#ifdef RASTERIZER_BUG_WA
        if(count == 0){
            nema_raster_triangle_f(center_x, center_y,
                        v_x, v_y,
                        v1_x, v1_y);
            count++;
        }
        else{
            nema_raster_triangle_f(center_x, center_y,
                        v1_x, v1_y,
                        v_x, v_y);
            count = 0;
        }
#else
        if(count == 0){
            nema_raster_triangle_p1_f(v_x, v_y);
            count++;
        }
        else{
            nema_raster_triangle_p2_f(v_x, v_y);
            count = 0;
        }
#endif
    }
}

void nema_fill_rounded_rect_aa(float x, float y, float w, float h, float r, uint32_t rgba8888)
{
    float r_ = r;
    if ( 0.5f*w < r_) {
        r_ = 0.5f*w;
    }

    if ( 0.5f*h < r_) {
        r_ = 0.5f*h;
    }

    if ( (r_ <= 0.f) || (w <= 2.f) || (h <= 2.f)) {
        int xi = (int) x;
        int yi = (int) y;
        int wi = (int) w;
        int hi = (int) h;

        nema_fill_rect(xi, yi, wi, hi, rgba8888);
        return;
    }

    nema_fill_rect_f(x, y + r_, w, h - 2.0f * r_, rgba8888);
    nema_fill_rect_f(x + r_, y, w - 2.0f * r_, r_, rgba8888);
    nema_fill_rect_f(x + r_, y + h - r_, w - 2.0f * r_, r_, rgba8888);

    uint32_t prev_aa = nema_enable_aa(0, 1, 0, 0);

    nema_set_raster_color(rgba8888);

    int steps = calculate_steps_from_radius(r_) / 4;
    float step_cos_theta = nema_cos(90.0f / (float)steps);
    float step_sin_theta = -nema_sin(90.0f / (float)steps);

    // the up right rounded corner
    float angle_cos_theta = 1.0f;
    float angle_sin_theta = 0.0f;
    float center_x = x + w - r_;
    float center_y = y + r_;
    fill_rounded_rect_corner(center_x, center_y,
                            angle_cos_theta, angle_sin_theta, steps,
                            step_cos_theta, step_sin_theta, r_ );

    // the down right rounded corner
    angle_cos_theta = 1.0f;
    angle_sin_theta = 0.0f;
    center_x = x + w - r_;
    center_y = y + h - r_;
    fill_rounded_rect_corner(center_x, center_y,
                            angle_cos_theta, angle_sin_theta, steps,
                            step_cos_theta, -step_sin_theta, r_ );

    // the up left rounded corner
    angle_cos_theta = 0.0f;
    angle_sin_theta = 1.0f;
    center_x = x + r_;
    center_y = y + r_;
    fill_rounded_rect_corner(center_x, center_y,
                            angle_cos_theta, -angle_sin_theta, steps,
                            step_cos_theta, step_sin_theta, r_ );

    angle_cos_theta = -1.0f;
    angle_sin_theta = 0.0f;
    center_x = x + r_;
    center_y = y + h - r_;
    fill_rounded_rect_corner(center_x, center_y,
                            angle_cos_theta, -angle_sin_theta, steps,
                            step_cos_theta, step_sin_theta, r_ );

    (void) nema_enable_aa_flags(prev_aa);
}

static float d_p_from_l(float x0, float y0, float x1, float y1, float x2, float y2) {
    float a = (x2-x1)*(y1-y0) - (x1-x0)*(y2-y1);
    float b = (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);
    float c = nema_absf(a)/nema_sqrt(b);
    return c;
}

void nema_draw_quad_aa(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3,
                             float border_width, uint32_t color) {
    float cross012 = orientation_subformula(x0, y0, x1, y1, x2, y2);
    float cross123 = orientation_subformula(x1, y1, x2, y2, x3, y3);
    float cross230 = orientation_subformula(x2, y2, x3, y3, x0, y0);
    float cross301 = orientation_subformula(x3, y3, x0, y0, x1, y1);

    float allcross = cross012+cross123+cross230+cross301;
    if ( nema_absf(cross012 / allcross) < 0.01f) {
        cross012 = 0.f;
    }
    if ( nema_absf(cross123 / allcross) < 0.01f) {
        cross123 = 0.f;
    }
    if ( nema_absf(cross230 / allcross) < 0.01f) {
        cross230 = 0.f;
    }
    if ( nema_absf(cross301 / allcross) < 0.01f) {
        cross301 = 0.f;
    }

    if ((cross012 >= 0.f) &&
        (cross123 >= 0.f) &&
        (cross230 >= 0.f) &&
        (cross301 >= 0.f)
         ) {
            // MISRA
         }
    else if (
        (cross012 <= 0.f) &&
        (cross123 <= 0.f) &&
        (cross230 <= 0.f) &&
        (cross301 <= 0.f)
        ) {
            // MISRA
         }
    else {
        return;
    }

    float dx0     = x1 - x0;
    float dy0     = y1 - y0;
    float length0 = nema_sqrt(dx0*dx0 + dy0*dy0);
    float dx1     = x2 - x1;
    float dy1     = y2 - y1;
    float length1 = nema_sqrt(dx1*dx1 + dy1*dy1);
    float dx2     = x3 - x2;
    float dy2     = y3 - y2;
    float length2 = nema_sqrt(dx2*dx2 + dy2*dy2);
    float dx3     = x0 - x3;
    float dy3     = y0 - y3;
    float length3 = nema_sqrt(dx3*dx3 + dy3*dy3);

    //max border width
    float d0_sq = d_p_from_l(x0, y0, x1, y1, x3, y3);
    float d1_sq = d_p_from_l(x2, y2, x1, y1, x3, y3);
    float d2_sq = d_p_from_l(x1, y1, x0, y0, x2, y2);
    float d3_sq = d_p_from_l(x3, y3, x0, y0, x2, y2);

    float max_border = nema_min2(d0_sq, d1_sq);
    max_border = nema_min2(max_border, d2_sq);
    max_border = nema_min2(max_border, d3_sq);

    float _border_width = border_width;

    if ( _border_width <= 0.04f ) {
        return;
    }

    if ( _border_width > max_border ) {
        _border_width = max_border;
    }

    dx0 = 0.5f*_border_width*dx0/length0;
    dy0 = 0.5f*_border_width*dy0/length0;
    dx1 = 0.5f*_border_width*dx1/length1;
    dy1 = 0.5f*_border_width*dy1/length1;
    dx2 = 0.5f*_border_width*dx2/length2;
    dy2 = 0.5f*_border_width*dy2/length2;
    dx3 = 0.5f*_border_width*dx3/length3;
    dy3 = 0.5f*_border_width*dy3/length3;

    if ( is_quad_clockwise(x0, y0, x1, y1, x2, y2, x3, y3) ) {
        dx0 = - dx0;
        dy0 = - dy0;
        dx1 = - dx1;
        dy1 = - dy1;
        dx2 = - dx2;
        dy2 = - dy2;
        dx3 = - dx3;
        dy3 = - dy3;
    }

    //Find the parallel lines e0, e1, e2 (inner and outer)
    //e0
    //inner line points
    float e0_in_x0 = x0 - dy0;
    float e0_in_y0 = y0 + dx0;
    float e0_in_x1 = x1 - dy0;
    float e0_in_y1 = y1 + dx0;
    //outer line points
    float e0_out_x0 = x0 + dy0;
    float e0_out_y0 = y0 - dx0;
    float e0_out_x1 = x1 + dy0;
    float e0_out_y1 = y1 - dx0;

    //e1
    //inner line points
    float e1_in_x0 = x1 - dy1;
    float e1_in_y0 = y1 + dx1;
    float e1_in_x1 = x2 - dy1;
    float e1_in_y1 = y2 + dx1;
    //outer line points
    float e1_out_x0 = x1 + dy1;
    float e1_out_y0 = y1 - dx1;
    float e1_out_x1 = x2 + dy1;
    float e1_out_y1 = y2 - dx1;

    //e2
    //inner line points
    float e2_in_x0 = x2 - dy2;
    float e2_in_y0 = y2 + dx2;
    float e2_in_x1 = x3 - dy2;
    float e2_in_y1 = y3 + dx2;
    //outer line points
    float e2_out_x0 = x2 + dy2;
    float e2_out_y0 = y2 - dx2;
    float e2_out_x1 = x3 + dy2;
    float e2_out_y1 = y3 - dx2;

    //e3
    //inner line points
    float e3_in_x0 = x3 - dy3;
    float e3_in_y0 = y3 + dx3;
    float e3_in_x1 = x0 - dy3;
    float e3_in_y1 = y0 + dx3;
    //outer line points
    float e3_out_x0 = x3 + dy3;
    float e3_out_y0 = y3 - dx3;
    float e3_out_x1 = x0 + dy3;
    float e3_out_y1 = y0 - dx3;

    float x0_in  = 0.f;
    float y0_in  = 0.f;
    float x1_in  = 0.f;
    float y1_in  = 0.f;
    float x2_in  = 0.f;
    float y2_in  = 0.f;
    float x3_in  = 0.f;
    float y3_in  = 0.f;
    float x0_out = 0.f;
    float y0_out = 0.f;
    float x1_out = 0.f;
    float y1_out = 0.f;
    float x2_out = 0.f;
    float y2_out = 0.f;
    float x3_out = 0.f;
    float y3_out = 0.f;

    //calculate the crossing points of the inner lines
    (void) line_intersection_point(e0_in_x0, e0_in_y0, e0_in_x1, e0_in_y1, e3_in_x0, e3_in_y0, e3_in_x1, e3_in_y1, &x0_in, &y0_in);
    (void) line_intersection_point(e0_in_x0, e0_in_y0, e0_in_x1, e0_in_y1, e1_in_x0, e1_in_y0, e1_in_x1, e1_in_y1, &x1_in, &y1_in);
    (void) line_intersection_point(e1_in_x0, e1_in_y0, e1_in_x1, e1_in_y1, e2_in_x0, e2_in_y0, e2_in_x1, e2_in_y1, &x2_in, &y2_in);
    (void) line_intersection_point(e2_in_x0, e2_in_y0, e2_in_x1, e2_in_y1, e3_in_x0, e3_in_y0, e3_in_x1, e3_in_y1, &x3_in, &y3_in);

    //calculate the crossing points of the outer lines
    (void) line_intersection_point(e0_out_x0, e0_out_y0, e0_out_x1, e0_out_y1, e3_out_x0, e3_out_y0, e3_out_x1, e3_out_y1, &x0_out, &y0_out);
    (void) line_intersection_point(e0_out_x0, e0_out_y0, e0_out_x1, e0_out_y1, e1_out_x0, e1_out_y0, e1_out_x1, e1_out_y1, &x1_out, &y1_out);
    (void) line_intersection_point(e1_out_x0, e1_out_y0, e1_out_x1, e1_out_y1, e2_out_x0, e2_out_y0, e2_out_x1, e2_out_y1, &x2_out, &y2_out);
    (void) line_intersection_point(e2_out_x0, e2_out_y0, e2_out_x1, e2_out_y1, e3_out_x0, e3_out_y0, e3_out_x1, e3_out_y1, &x3_out, &y3_out);

    uint32_t prev_aa = nema_enable_aa(1, 0, 0, 0);

    nema_set_raster_color(color);
    nema_raster_triangle_f(x0_in , y0_in , x1_in , y1_in , x0_out, y0_out);
    nema_raster_triangle_f(x0_out, y0_out, x1_out, y1_out, x1_in , y1_in );
    nema_raster_triangle_f(x1_in , y1_in , x2_in , y2_in , x1_out, y1_out);
    nema_raster_triangle_f(x1_out, y1_out, x2_out, y2_out, x2_in , y2_in );
    nema_raster_triangle_f(x2_in , y2_in , x3_in , y3_in , x2_out, y2_out);
    nema_raster_triangle_f(x2_out, y2_out, x3_out, y3_out, x3_in , y3_in );
    nema_raster_triangle_f(x3_in , y3_in , x0_in , y0_in , x3_out, y3_out);
    nema_raster_triangle_f(x3_out, y3_out, x0_out, y0_out, x0_in , y0_in );

    (void)nema_enable_aa_flags(prev_aa);
}
