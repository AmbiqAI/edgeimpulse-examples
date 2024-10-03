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
#include "nema_core.h"
#include "nema_regs.h"
#include "nema_vg.h"
#include "nema_vg_p.h"
#include "nema_blender_intern.h"
#include "nema_rasterizer_intern.h"
#include "nema_programHW.h"
#include "nema_provisional.h"

#include "nema_raster.h"
#include "nema_matrix3x3.h"

#ifndef NEMAVG_USE_STANDARD_C_MATH
#define NEMAVG_USE_STANDARD_C_MATH 1
#endif

#if NEMAVG_USE_STANDARD_C_MATH

#include <math.h>
#define ATAN2_(y, x)    (atan2f( (y), (x) ) )
#define SQRT_(x)        (sqrtf(x))
#define TAN_(x)         (tanf(x))
#define COS_(x)         (cosf(x))
#define SIN_(x)         (sinf(x))

#else

#define ATAN2_(y, x)    nema_deg_to_rad(nema_atan2((y), (x)))
#define SQRT_(x)        (nema_sqrt(x))
#define TAN_(x)         (nema_tan_r( (x)))
#define COS_(x)         (nema_cos_r( (x)))
#define SIN_(x)         (nema_sin_r( (x)))

#endif

// #define DEBUG_VG
// #define ENABLE_PRINT_FUNC_ENTRY

#define FILLSTENCIL_OPT

#ifdef ENABLE_PRINT_FUNC_ENTRY
#define PRINT_FUNC_ENTRY PRINT_F("%s\n", __func__)
#else
#define PRINT_FUNC_ENTRY
#endif

#ifndef NEMAPVG
#define NEMA_P_IMEM_16
#endif

#ifdef NEMA_P_IMEM_16
    #define SHADER_ADDR_PIXOUT3     0U
    #define SHADER_ADDR_FILLSRCOVER 0U
    #define SHADER_ADDR_FILLSTENCIL 0U
#else
    #define PRECOMPILED_SHADERS
    #define SHADER_ADDR_PIXOUT3     21U
    #define SHADER_ADDR_FILLSRCOVER 20U
    #define SHADER_ADDR_FILLSTENCIL 19U
#endif //NEMA_P_IMEM_16

#define COLLINEAR_OPT


#ifdef NEMAPVG
#define HW_VERTEX_MMUL
#else
#undef HW_VERTEX_MMUL
#endif

#define CHECK_STENCIL_DIMENSIONS()  \
    if((stencil->width % 4 != 0) || (stencil->height %4 != 0)){ \
        nema_vg_set_error(NEMA_VG_ERR_INVALID_STENCIL_SIZE); \
    }

// forward declarations
inline static void
nema_raster_quad_f_(nema_vg_vertex_t_ p0,
                    nema_vg_vertex_t_ p1,
                    nema_vg_vertex_t_ p2,
                    nema_vg_vertex_t_ p3);

// #ifdef nema_abs
// #undef nema_abs
// #endif

// Tried *1.f, *2.f, *4.f, *8.f
// *8.f looks the best
// #define vertex_2fx(a)   (((int)(a*8.f))*0x2000)

int vertex_2fx(float a){
    float a_f = a * 65536.f; // + 0.5f ;
    int a_i = (int)(a_f);
    return a_i;
}

static void get_matrix_scale_factors(nema_matrix3x3_t m, float *sx, float *sy){
    *sx = SQRT_(m[0][0] * m[0][0] + m[0][1] * m[0][1]);
    *sy = SQRT_(m[1][0] * m[1][0] + m[1][1] * m[1][1]);
}

static void tess_stack_push_vertex(nema_vg_vertex_t_ p0, nema_vg_vertex_t_ m, nema_vg_vertex_t_ p1) {
    if ( nemavg_context->info.tess_stack_idx  < STACK_DEPTH) {
        nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][0] = p0.x;
        nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][1] = p0.y;
        nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][2] = m.x;
        nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][3] = m.y;
        nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][4] = p1.x;
        nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][5] = p1.y;
        ++nemavg_context->info.tess_stack_idx;
    }
}

static void tess_stack_pop_vertex(nema_vg_vertex_t_ *p0, nema_vg_vertex_t_ *m, nema_vg_vertex_t_ *p1) {
    if ( nemavg_context->info.tess_stack_idx != 0) {
        --nemavg_context->info.tess_stack_idx;
        p0->x = nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][0] ;
        p0->y = nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][1] ;
        m->x  = nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][2] ;
        m->y  = nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][3] ;
        p1->x = nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][4] ;
        p1->y = nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][5] ;
    }
}

void enable_draw_on_fb(int enable) {
    nemavg_context->info.do_draw_on_fb = enable;
}

void enable_use_bevel_joins(int enable) {
    nemavg_context->info.use_bevel_joins = enable;
}

#define DOING_STROKE  (nemavg_context->fill_rule == NEMA_VG_STROKE)
#define DOING_NONZERO (nemavg_context->fill_rule == NEMA_VG_FILL_NON_ZERO)
#define DOING_EVENODD (nemavg_context->fill_rule == NEMA_VG_FILL_EVEN_ODD)


#ifdef DEBUG_VG
static void print_path_idx(const char *s, nema_vbuf_t_ *vb, int idx) {
    PRINT_F("%s%d: %f, %f | %d\n", s, idx,
                                vb->ptr[idx  ].x,
                                vb->ptr[idx  ].y,
                                vb->ptr[idx  ].flags);
}

void
print_path(nema_vbuf_t_ *vb) {
    // PRINT_FUNC_ENTRY;

    for (int idx = 0; idx < vb->last; ++idx) {
        print_path_idx("", vb, idx);
    }
}
#endif

static uint8_t check_points_coincident(nema_vg_vertex_t_* p1, nema_vg_vertex_t_* p2)\
{
    uint8_t is_coincident = 0U;
    if((nema_floats_equal(p1->x, p2->x)) &&
       (nema_floats_equal(p1->y, p2->y)))
    {
        is_coincident = 1;
    }

    return is_coincident;
}

static uint8_t check_lines_coincident(stroked_line_segment_t_* line1, stroked_line_segment_t_* line2)
{
    uint8_t is_coincident = 0U;

    if((check_points_coincident(&(line1->q0), &(line2->q0)) == 1U ) &&
       (check_points_coincident(&(line1->q1), &(line2->q1)) == 1U ) &&
       (check_points_coincident(&(line1->q2), &(line2->q2)) == 1U ) &&
       (check_points_coincident(&(line1->q3), &(line2->q3)) == 1U ) )
    {
        is_coincident = 1;
    }

    return is_coincident;
}

void set_fan_center(float fanx, float fany)
{
    nemavg_context->info.fan_center.fanx0_f = fanx + 0.5f;
    nemavg_context->info.fan_center.fany0_f = fany + 0.5f;
    nemavg_context->info.fan_center.fanx0   = (int32_t)(nemavg_context->info.fan_center.fanx0_f);
    nemavg_context->info.fan_center.fany0   = (int32_t)(nemavg_context->info.fan_center.fany0_f);
    nemavg_context->info.fan_center.fanx0fx = nema_i2fx(nemavg_context->info.fan_center.fanx0);
    nemavg_context->info.fan_center.fany0fx = nema_i2fx(nemavg_context->info.fan_center.fany0);
}


void set_clip2_or_temp(int clip2, int32_t x, int32_t y, uint32_t w, uint32_t h)
{
    if (clip2 != 0){
        nema_set_clip2(x, y, w, h);
    }else{
        nema_set_clip_temp(x, y, w, h);
    }
}

void reset_clip2_or_temp(int clip2)
{
    if (clip2 != 0){
        nema_set_clip2(0, 0, 0x7fff, 0x7fff);
    }else{
        nema_set_clip_pop();
    }
}


// -------------------------------------------------------------------------------
//                           PATH DRAW/FILL
// -------------------------------------------------------------------------------

#ifndef NEMAPVG
#ifdef HAAS

void
init_clip_from_context(void)
{
    nemavg_context->info.clip_x_orig = nema_context.prev_clip_xy[0];
    nemavg_context->info.clip_y_orig = nema_context.prev_clip_xy[1];
    nemavg_context->info.clip_w_orig = (int32_t)nema_context.prev_clip_wh[0];
    nemavg_context->info.clip_h_orig = (int32_t)nema_context.prev_clip_wh[1];

}

void
init_clip(int32_t x_orig, int32_t y_orig, int32_t w_orig, int32_t h_orig)
{
    nemavg_context->info.clip_x_orig = x_orig;
    nemavg_context->info.clip_y_orig = y_orig;
    nemavg_context->info.clip_w_orig = w_orig;
    nemavg_context->info.clip_h_orig = h_orig;
}
#endif //HAAS

//forward declaration
inline static void
transform_vertex(nema_matrix3x3_t m, nema_vg_vertex_t_* coord);


static void transform_vertices( nema_vg_vertex_t_* p0,
                                nema_vg_vertex_t_* p1,
                                nema_vg_vertex_t_* p2,
                                nema_vg_vertex_t_* p3)
{
    transform_vertex(nemavg_context->path_plus_global_m, p0);
    transform_vertex(nemavg_context->path_plus_global_m, p1);
    transform_vertex(nemavg_context->path_plus_global_m, p2);
    transform_vertex(nemavg_context->path_plus_global_m, p3);
}

void enable_do_not_transform(uint8_t enable)
{
    nemavg_context->info.do_not_transform = (int) enable;
}

#endif //NEMAPVG

static inline void draw_quad(nema_vg_vertex_t_* p0, nema_vg_vertex_t_* p1, nema_vg_vertex_t_* p2, nema_vg_vertex_t_* p3)
{
#ifndef NEMAPVG
    if((nemavg_context->info.do_not_transform == 0) &&
       ((nemavg_context->flags & NEMA_VG_CONTEXT_STROKE_WIDTH_SCREEN_SPACE) == 0U))
    {
        transform_vertices(p0, p1, p2, p3);
    }
#endif //NEMAPVG
    nema_raster_quad_f_ (*p0, *p1, *p2, *p3);
}

static int triangle_area(const nema_vg_vertex_t_* p0,
                         const nema_vg_vertex_t_* p1,
                         const nema_vg_vertex_t_* p2)
{
    nema_vg_float_t area = p0->x * (p1->y - p2->y) + p1->x * (p2->y - p0->y) + p2->x * (p0->y - p1->y);
    float half_area_f = 0.5f* ((float) nema_abs((int)area));
    return (int) (half_area_f);
}


static uint8_t is_point_inside_stroked_line(const stroked_line_segment_t_* line,
                                            const nema_vg_vertex_t_* p)
{
    uint8_t ret = 1U;

    int area01 = triangle_area(p, &(line->q0), &(line->q1));
    int area12 = triangle_area(p, &(line->q1), &(line->q2));
    int area23 = triangle_area(p, &(line->q2), &(line->q3));
    int area30 = triangle_area(p, &(line->q3), &(line->q0));

    int area_quad = triangle_area(&(line->q0), &(line->q1), &(line->q2)) +
                    triangle_area(&(line->q2), &(line->q3), &(line->q0));
    int area_points = area01 + area12 + area23 + area30;

    if(area_points > area_quad)
    {
        ret = 0U;
    }

    return ret;
}

static void find_outer_inner_points(const stroked_line_segment_t_* previous_line,
                                    const stroked_line_segment_t_* current_line,
                                    nema_vg_vertex_t_* previous_line_outer_edge_start, nema_vg_vertex_t_* previous_line_outer_edge_end,
                                    nema_vg_vertex_t_* previous_line_inner_edge_start, nema_vg_vertex_t_* previous_line_inner_edge_end,
                                    nema_vg_vertex_t_* current_line_outer_edge_start,  nema_vg_vertex_t_* current_line_outer_edge_end,
                                    nema_vg_vertex_t_* current_line_inner_edge_start,  nema_vg_vertex_t_* current_line_inner_edge_end)
{
    //from previous line check q1,q2 which lies inside current line geometry
    //from current line check q0,q3 which lies inside previous line geometry
    //q1 is always paired with q0 and q2 with q3
    if((is_point_inside_stroked_line(current_line, &(previous_line->q1)) != 0U) ||
        (is_point_inside_stroked_line(previous_line, &(current_line->q0)) != 0U))
    {
        *previous_line_inner_edge_end   = previous_line->q1;
        *previous_line_outer_edge_end   = previous_line->q2;
        *previous_line_inner_edge_start = previous_line->q0;
        *previous_line_outer_edge_start = previous_line->q3;
        *current_line_inner_edge_start  = current_line->q0;
        *current_line_outer_edge_start  = current_line->q3;
        *current_line_inner_edge_end    = current_line->q1;
        *current_line_outer_edge_end    = current_line->q2;
    }
    else if((is_point_inside_stroked_line(current_line, &(previous_line->q2)) != 0U) ||
            (is_point_inside_stroked_line(previous_line, &(current_line->q3)) != 0U))
    {
        *previous_line_inner_edge_end   = previous_line->q2;
        *previous_line_outer_edge_end   = previous_line->q1;
        *previous_line_inner_edge_start = previous_line->q3;
        *previous_line_outer_edge_start = previous_line->q0;
        *current_line_inner_edge_start  = current_line->q3;
        *current_line_outer_edge_start  = current_line->q0;
        *current_line_inner_edge_end    = current_line->q2;
        *current_line_outer_edge_end    = current_line->q1;
    }
    else
    {
        if(previous_line->q1.y > current_line->q0.y) //previous line is higher
        {
            if(previous_line->q1.y > previous_line->q2.y)
            {
                *previous_line_inner_edge_end   = previous_line->q2;
                *previous_line_outer_edge_end   = previous_line->q1;
                *previous_line_inner_edge_start = previous_line->q3;
                *previous_line_outer_edge_start = previous_line->q0;
                *current_line_inner_edge_start  = current_line->q3;
                *current_line_outer_edge_start  = current_line->q0;
                *current_line_inner_edge_end    = current_line->q2;
                *current_line_outer_edge_end    = current_line->q1;
            }
            else
            {
                *previous_line_inner_edge_end   = previous_line->q1;
                *previous_line_outer_edge_end   = previous_line->q2;
                *previous_line_inner_edge_start = previous_line->q0;
                *previous_line_outer_edge_start = previous_line->q3;
                *current_line_inner_edge_start  = current_line->q0;
                *current_line_outer_edge_start  = current_line->q3;
                *current_line_inner_edge_end    = current_line->q1;
                *current_line_outer_edge_end    = current_line->q2;
            }
        }
        else // previous line is lower
        {
            if(previous_line->q1.y > previous_line->q2.y)
            {
                *previous_line_inner_edge_end   = previous_line->q1;
                *previous_line_outer_edge_end   = previous_line->q2;
                *previous_line_inner_edge_start = previous_line->q0;
                *previous_line_outer_edge_start = previous_line->q3;
                *current_line_inner_edge_start  = current_line->q0;
                *current_line_outer_edge_start  = current_line->q3;
                *current_line_inner_edge_end    = current_line->q1;
                *current_line_outer_edge_end    = current_line->q2;
            }
            else
            {
                *previous_line_inner_edge_end   = previous_line->q2;
                *previous_line_outer_edge_end   = previous_line->q1;
                *previous_line_inner_edge_start = previous_line->q3;
                *previous_line_outer_edge_start = previous_line->q0;
                *current_line_inner_edge_start  = current_line->q3;
                *current_line_outer_edge_start  = current_line->q0;
                *current_line_inner_edge_end    = current_line->q2;
                *current_line_outer_edge_end    = current_line->q1;
            }
        }
    }
}

static uint32_t draw_join(nema_vg_vertex_t_* p0, nema_vg_vertex_t_* p1, nema_vg_vertex_t_* p2, nema_vg_vertex_t_* p3, int is_close)
{
    uint32_t prev_aa =0;
    if((nemavg_context->stroke.join_style == NEMA_VG_JOIN_BEVEL)  || (nemavg_context->info.use_bevel_joins != 0)){
        if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
            prev_aa = nema_enable_aa(1, 0, 1, 0);
        }
        draw_quad(p0, p1, p2, p3);
        if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
            (void)nema_enable_aa_flags(prev_aa);
        }
    }
    else if(nemavg_context->stroke.join_style == NEMA_VG_JOIN_ROUND)
    {
#ifndef NEMAPVG
        if((nemavg_context->info.do_not_transform == 0) &&
        ((nemavg_context->flags & NEMA_VG_CONTEXT_STROKE_WIDTH_SCREEN_SPACE) == 0U))
        {
            transform_vertices(p0, p1, p2, p3);
        }
#endif //NEMAPVG

        // find the intersection point of p0-p2 and p1-p3
        nema_vg_vertex_t_ intersection_point = {0.f , 0.f};
        int lines_intersect = line_intersection_point(p0->x, p0->y,
                                                      p2->x, p2->y,
                                                      p1->x, p1->y,
                                                      p3->x, p3->y,
                                                      &(intersection_point.x), &(intersection_point.y));
        if(lines_intersect != 0)
        {
            float len = 0.0f;
#ifdef NEMAPVG
            float sx = 1.f;
            float sy = 1.f;

            get_matrix_scale_factors(nemavg_context->path_plus_global_m, &sx, &sy);
            sx = nema_absf(sx);
            sy = nema_absf(sy);
            int scale = ((sx > 2) || (sy > 2)) ? 1 : 0;
            if(scale == 1)
            {
                enable_scaling(1);
            }
            len = nemavg_context->stroke.width/2.f;
#else
            float dx = p3->x - p1->x;
            float dy = p3->y - p1->y;
            len = nema_sqrt((dx*dx) + (dy*dy))/2.f;
#endif //NEMAPVG


            if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
                nema_raster_circle_aa(intersection_point.x, intersection_point.y, len);
            }
            else{
                nema_raster_circle(intersection_point.x, intersection_point.y, len);
            }

#ifdef NEMAPVG
            if(scale == 1)
            {
                enable_scaling(0);
            }
#endif //NEMAPVG
        }

    }
    else if(nemavg_context->stroke.join_style == NEMA_VG_JOIN_MITER){

        stroked_line_segment_t_ previous_line, current_line;
        if (nemavg_context->info.first_line.length > 0.0f && nemavg_context->info.num_stroked_line_segments == 1U){
            previous_line = nemavg_context->info.first_line;
            current_line = nemavg_context->info.end_line;
        }
        else if ((is_close == 1) && (nemavg_context->info.first_line.length > 0.0f) && (nemavg_context->info.last_line.length > 0.0f)){
            current_line = nemavg_context->info.first_line;
            previous_line = nemavg_context->info.last_line;
        }
        else{
            previous_line = nemavg_context->info.last_line;
            current_line = nemavg_context->info.end_line;
        }

        if( check_lines_coincident(&current_line, &previous_line) != 0U ) //coincident lines don't draw join
        {
            return NEMA_VG_ERR_NO_ERROR;
        }

        //find outer edges of the two drawn lines
        nema_vg_vertex_t_ previous_line_outer_edge_end   = {0.f, 0.f};
        nema_vg_vertex_t_ previous_line_outer_edge_start = {0.f, 0.f};
        nema_vg_vertex_t_ previous_line_inner_edge_end   = {0.f, 0.f};
        nema_vg_vertex_t_ previous_line_inner_edge_start = {0.f, 0.f};
        nema_vg_vertex_t_ current_line_outer_edge_end    = {0.f, 0.f};
        nema_vg_vertex_t_ current_line_outer_edge_start  = {0.f, 0.f};
        nema_vg_vertex_t_ current_line_inner_edge_end    = {0.f, 0.f};
        nema_vg_vertex_t_ current_line_inner_edge_start  = {0.f, 0.f};
        find_outer_inner_points(&previous_line, &current_line,
                                &previous_line_outer_edge_start, &previous_line_outer_edge_end,
                                &previous_line_inner_edge_start, &previous_line_inner_edge_end,
                                &current_line_outer_edge_start,  &current_line_outer_edge_end,
                                &current_line_inner_edge_start,  &current_line_inner_edge_end);

        //find intersection points of the two drawn lines
        nema_vg_vertex_t_ inner_point = {0.f, 0.f};
        nema_vg_vertex_t_ outer_point = {0.f, 0.f};
        //inner intersection point from previous_line inner edges and current line inner edges
        int inner_intersect = line_intersection_point(previous_line_inner_edge_start.x, previous_line_inner_edge_start.y,
                                                      previous_line_inner_edge_end.x,   previous_line_inner_edge_end.y,
                                                      current_line_inner_edge_start.x,  current_line_inner_edge_start.y,
                                                      current_line_inner_edge_end.x,    current_line_inner_edge_end.y,
                                                      &(inner_point.x), &(inner_point.y));

        //outter intersection point from previous_line outer edges and current line outer edges
        int outer_intersect =  line_intersection_point(previous_line_outer_edge_start.x, previous_line_outer_edge_start.y,
                                                        previous_line_outer_edge_end.x,   previous_line_outer_edge_end.y,
                                                        current_line_outer_edge_start.x,  current_line_outer_edge_start.y,
                                                        current_line_outer_edge_end.x,    current_line_outer_edge_end.y,
                                                        &(outer_point.x), &(outer_point.y));

        float dx = outer_point.x - inner_point.x;
        float dy = outer_point.y - inner_point.y;
        float inter_points_len = nema_sqrt((dx*dx) + (dy*dy));
        float miter_product = nemavg_context->stroke.miter_limit*nemavg_context->stroke.width;
        if((inter_points_len > miter_product) || (outer_intersect == 0) || (inner_intersect == 0)) //draw bevel
        {
            if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
                prev_aa = nema_enable_aa(1, 0, 1, 0);
            }
            draw_quad(p0, p1, p2, p3);
        }
        else{ //draw miter
            if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
                prev_aa = nema_enable_aa(0, 1, 1, 0);
            }
            draw_quad(&inner_point, &previous_line_outer_edge_end, &outer_point, &current_line_outer_edge_start);
        }

        if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
            (void)nema_enable_aa_flags(prev_aa);
        }
    }
    else{
        return NEMA_VG_ERR_INVALID_JOIN_STYLE;
    }

    return NEMA_VG_ERR_NO_ERROR;

}

void flush_lines_(int is_close, uint8_t seg){

    uint32_t prev_aa = 0;
    if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) == NEMA_VG_QUALITY_NON_AA){
        prev_aa = nema_enable_aa(0, 0, 0, 0);
    }

    nema_vg_vertex_t_ p0;
    nema_vg_vertex_t_ p1;
    nema_vg_vertex_t_ p2;
    nema_vg_vertex_t_ p3;

    // if we have a close path, we need a bevel edge between last and first lines
    if ((is_close == 1) && (nemavg_context->info.first_line.length > 0.0f) && (nemavg_context->info.last_line.length > 0.0f)){
        p0 = nemavg_context->info.last_line.q1;
        p1 = nemavg_context->info.first_line.q0;
        p2 = nemavg_context->info.last_line.q2;
        p3 = nemavg_context->info.first_line.q3;

        (void) draw_join(&p0, &p1, &p2, &p3, 1);
    }

    if (nemavg_context->info.last_line.length > 0.0f){
        if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
            if ((is_close == 1) || (nemavg_context->info.has_cap != 0U)){
                prev_aa = nema_enable_aa(1, 0, 1, 0);
            }
            else{
                prev_aa = nema_enable_aa(1, 1, 1, 0);
            }
        }

        p0 = nemavg_context->info.last_line.q0;
        p1 = nemavg_context->info.last_line.q1;
        p2 = nemavg_context->info.last_line.q2;
        p3 = nemavg_context->info.last_line.q3;

        draw_quad(&p0, &p1, &p2, &p3);

        nemavg_context->info.last_line.length = 0.0f;

        if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
            (void)nema_enable_aa_flags(prev_aa);
        }
    }

    // if we have stroke(something like a move), don't flush the first line
    if (nemavg_context->info.first_line.length > 0.0f && (seg & NEMA_VG_PRIM_NO_STROKE) == 0U){
        if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
            if (is_close == 1){
                prev_aa = nema_enable_aa(1, 0, 1, 0);
            }
            else if ((nemavg_context->info.num_stroked_line_segments == 1U) ){
                if(nemavg_context->info.has_cap == 0U){ //if there is a cap dont add aa in the outer edge
                    prev_aa = nema_enable_aa(1, 1, 1, 1);
                }
                else{
                    prev_aa = nema_enable_aa(1, 1, 1, 0);
                }
            }
            else{
                if(nemavg_context->info.has_cap == 0U){
                    prev_aa = nema_enable_aa(1, 0, 1, 1);
                }
                else{
                    prev_aa = nema_enable_aa(1, 0, 1, 0);
                }
            }
        }

        p0 = nemavg_context->info.first_line.q0;
        p1 = nemavg_context->info.first_line.q1;
        p2 = nemavg_context->info.first_line.q2;
        p3 = nemavg_context->info.first_line.q3;

        draw_quad(&p0, &p1, &p2, &p3);

        nemavg_context->info.first_line.length = 0.0f;

        if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
            (void)nema_enable_aa_flags(prev_aa);
        }
    }

    // draw the bevel edge
    if (nemavg_context->info.num_stroked_line_segments == 2U){
        p0 = nemavg_context->info.first_line.q1;
        p1 = nemavg_context->info.last_line.q0;
        p2 = nemavg_context->info.first_line.q2;
        p3 = nemavg_context->info.last_line.q3;

        (void) draw_join(&p0, &p1, &p2, &p3, 0);
    }

    (void)nema_enable_aa_flags(prev_aa);

    // although here we set the num stroke line segments to zero,
    // in case of a stroke move, we will have the first line segment
    // to be valid
    nemavg_context->info.num_stroked_line_segments = 0U;
}


void
raster_stroked_line_(nema_vg_vertex_t_ p0,  nema_vg_vertex_t_ p1, int is_join)
{
    nema_vg_float_t dX = p1.x - p0.x;
    nema_vg_float_t dY = p1.y - p0.y;

    nema_vg_float_t len2 = dX*dX + dY*dY;

    if ( nema_float_is_zero(len2) ) { //when points are equal save them so only the caps can be added in the end
        nemavg_context->info.end_line.q1.x = p0.x;
        nemavg_context->info.end_line.q1.y = p0.y - (nemavg_context->info.stroke_w_x*0.5f);
        nemavg_context->info.end_line.q2.x = p0.x;
        nemavg_context->info.end_line.q2.y = p0.y + (nemavg_context->info.stroke_w_x*0.5f);
        nemavg_context->info.end_line.q0 = nemavg_context->info.end_line.q1;
        nemavg_context->info.end_line.q3 = nemavg_context->info.end_line.q2;
        return;
    }

    nema_vg_float_t len = SQRT_(len2);
    nema_vg_vertex_t_ q0, q1, q2, q3;

    nema_vg_float_t one_over_len = 1.0f / len;
    nema_vg_float_t dx = dX * one_over_len; //len is checked above won't be zero
    nema_vg_float_t dy = dY * one_over_len;
    nema_vg_float_t dist_x = nemavg_context->info.stroke_w_x*0.5f;
    nema_vg_float_t dist_y = nemavg_context->info.stroke_w_y*0.5f;

    q0.x = p0.x - dist_x*dy;
    q0.y = p0.y + dist_y*dx;
    q1.x = p1.x - dist_x*dy;
    q1.y = p1.y + dist_y*dx;
    q2.x = p1.x + dist_x*dy;
    q2.y = p1.y - dist_y*dx;
    q3.x = p0.x + dist_x*dy;
    q3.y = p0.y - dist_y*dx;

    nemavg_context->info.end_line.q0 = q0;
    nemavg_context->info.end_line.q1 = q1;
    nemavg_context->info.end_line.q2 = q2;
    nemavg_context->info.end_line.q3 = q3;

    if ( nemavg_context->info.do_fat_stroke == 0 ){ // only need to check the stroke given by the user
        uint32_t prev_aa = nema_enable_aa_flags(nemavg_context->aa_mask);
        nema_raster_quad_f_ (q0, q1, q2, q3);
        (void)nema_enable_aa_flags(prev_aa);
    }else{
        uint32_t prev_aa;
        if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) == NEMA_VG_QUALITY_NON_AA){
            prev_aa = nema_enable_aa(0, 0, 0, 0);
        }
        else{
            prev_aa = nema_enable_aa(1, 0, 1, 0);
        }

        nema_vg_vertex_t_ p0_;
        nema_vg_vertex_t_ p1_;
        nema_vg_vertex_t_ p2_;
        nema_vg_vertex_t_ p3_;

        if (nemavg_context->info.last_line.length > 0.0f){

            p0_ = nemavg_context->info.last_line.q0;
            p1_ = nemavg_context->info.last_line.q1;
            p2_ = nemavg_context->info.last_line.q2;
            p3_ = nemavg_context->info.last_line.q3;

            draw_quad(&p0_, &p1_, &p2_, &p3_);

            #ifdef COLLINEAR_OPT
            // check if current line with the last one are
            // collinear
            float determinant = (nemavg_context->info.last_line.dx * dy - nemavg_context->info.last_line.dy * dx);
            if (nema_abs_(determinant) <= 0.05f){
                q0 = nemavg_context->info.last_line.q1;
                q3 = nemavg_context->info.last_line.q2;
            } else
            #endif // COLLINEAR_OPT
            {
                // draw the bevel edge
                p0_ = nemavg_context->info.last_line.q1;
                p1_ = q0;
                p2_ = nemavg_context->info.last_line.q2;
                p3_ = q3;

                if(is_join == 1)
                {
                    (void) draw_join(&p0_, &p1_, &p2_, &p3_, 0);
                }
                else
                {
                    draw_quad(&p0_, &p1_, &p2_, &p3_);
                }
            }
        }

        if (nemavg_context->info.first_line.length > 0.0f && nemavg_context->info.num_stroked_line_segments == 1U){

            #ifdef COLLINEAR_OPT
            // check if current line with the last one are
            // collinear
            float determinant = (nemavg_context->info.first_line.dx * dy - nemavg_context->info.first_line.dy * dx);
            if (nema_abs_(determinant) <= 0.05f){
                q0 = nemavg_context->info.first_line.q1;
                q3 = nemavg_context->info.first_line.q2;
            } else
            #endif // COLLINEAR_OPT
            {
                // draw the bevel edge
                p0_ = nemavg_context->info.first_line.q1;
                p1_ = q0;
                p2_ = nemavg_context->info.first_line.q2;
                p3_ = q3;

                if(is_join == 1)
                {
                    (void) draw_join(&p0_, &p1_, &p2_, &p3_, 0);
                }
                else
                {
                    draw_quad(&p0_, &p1_, &p2_, &p3_);
                }
            }
        }

        ++nemavg_context->info.num_stroked_line_segments;

        // don't check for num line segments here
        // because in case of a stroke move, the nemavg_context->info.first_line
        // will be valid but num line segments will be
        // zero
        if (nemavg_context->info.first_line.length == 0.0f){
            nemavg_context->info.first_line.q0 = q0;
            nemavg_context->info.first_line.q1 = q1;
            nemavg_context->info.first_line.q2 = q2;
            nemavg_context->info.first_line.q3 = q3;
            nemavg_context->info.first_line.length = len;
            nemavg_context->info.first_line.dx = dx;
            nemavg_context->info.first_line.dy = dy;

            (void)nema_enable_aa_flags(prev_aa);

            return;
        }

        nemavg_context->info.last_line.q0 = q0;
        nemavg_context->info.last_line.q1 = q1;
        nemavg_context->info.last_line.q2 = q2;
        nemavg_context->info.last_line.q3 = q3;
        nemavg_context->info.last_line.length = len;
        nemavg_context->info.last_line.dx = dx;
        nemavg_context->info.last_line.dy = dy;

        (void)nema_enable_aa_flags(prev_aa);
    }
}


void set_bezier_quality(uint8_t quality)
{
    (void) quality;
    nemavg_context->info.faster_quality = nemavg_context->quality & NEMA_VG_QUALITY_FASTER;
}


inline static uint32_t to_rgba(unsigned char R,
                   unsigned char G,
                   unsigned char B,
                   unsigned char A) {
    return ((uint32_t)R | ((uint32_t)G<<8) | ((uint32_t)B<<16) | ((uint32_t)A<<24));
}


inline static uint32_t premultiply_rgba(uint32_t rgba) {
    uint32_t r = (rgba      )&0xffU;
    uint32_t g = (rgba >>  8)&0xffU;
    uint32_t b = (rgba >> 16)&0xffU;
    uint32_t a = (rgba >> 24)     ;

    r =(r*a)/255U;
    g =(g*a)/255U;
    b =(b*a)/255U;

    return to_rgba((unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a);
}

inline static uint32_t modulate_rgba(uint32_t rgba, float val) {
    uint32_t r = (rgba      )&0xffU;
    uint32_t g = (rgba >>  8)&0xffU;
    uint32_t b = (rgba >> 16)&0xffU;
    uint32_t a = (rgba >> 24)     ;

    float cf;
    cf = (float)r*val;
    r = (uint32_t) cf;
    cf = (float)g*val;
    g = (uint32_t) cf;
    cf = (float)b*val;
    b = (uint32_t) cf;
    cf = (float)a*val;
    a = (uint32_t) cf;

    return to_rgba((unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a);
}

uint32_t set_stroked_line_width(float stroke, nema_matrix3x3_t m, uint32_t color)
{
    uint32_t color_ = color;
    // nemavg_context->info.stroke_w here is in object space
    nemavg_context->info.stroke_w = stroke;
    nemavg_context->info.stroke_w_x = nemavg_context->info.stroke_w;
    nemavg_context->info.stroke_w_y = nemavg_context->info.stroke_w;

    float screen_stroke_w = nemavg_context->info.stroke_w;
    float screen_stroke_w_x = nemavg_context->info.stroke_w_x;
    float screen_stroke_w_y = nemavg_context->info.stroke_w_y;

        if ( nemavg_context->fill_rule == NEMA_VG_STROKE ) {
            if (m != NULL)  {
                if ( (nemavg_context->flags & NEMA_VG_CONTEXT_STROKE_WIDTH_SCREEN_SPACE) == 0U){
                    float sx = 1.f;
                    float sy = 1.f;

                    get_matrix_scale_factors(m, &sx, &sy);

                    sx = nema_absf(sx);
                    sy = nema_absf(sy);
                    screen_stroke_w_x = screen_stroke_w * sx;
                    screen_stroke_w_y = screen_stroke_w * sy;
                    screen_stroke_w = nema_max2(screen_stroke_w_x, screen_stroke_w_y);
                }
            }

            if ( nema_floats_equal(screen_stroke_w_x, screen_stroke_w_y) && screen_stroke_w_x < 1.f ) {
                color_ = modulate_rgba(color_, screen_stroke_w_x);
                screen_stroke_w_x = 1.f;
                screen_stroke_w_y = 1.f;
                screen_stroke_w   = 1.f;
            }
        }

    nemavg_context->info.stroke_w    = screen_stroke_w;
    nemavg_context->info.stroke_w_x  = screen_stroke_w_x;
    nemavg_context->info.stroke_w_y  = screen_stroke_w_y;

    if ( (nemavg_context->fill_rule == NEMA_VG_STROKE)
          && (screen_stroke_w_x > 1.f || screen_stroke_w_y > 1.f) ) {
        nemavg_context->info.do_fat_stroke = 1;
    }
    else {
        nemavg_context->info.do_fat_stroke = 0;

        nemavg_context->info.stroke_w_x = 1.f;
        nemavg_context->info.stroke_w_y = 1.f;
        nemavg_context->info.stroke_w   = 1.f;
    }

    return color_;
}

void reset_last_vertex(void)
{
    nemavg_context->info.last_vertex = -1;
}

//rasterize a cubic bezier via tesselation (fixed segment count) - draw with lines
static void
stroke_cubic_decasteljau(nema_vg_vertex_t_ p0,
                         nema_vg_vertex_t_ c0,
                         nema_vg_vertex_t_ c1,
                         nema_vg_vertex_t_ p1)
{
    int iter = 1;
    int force_tess = 1;
    int first_run = 1;
    nema_vg_vertex_t_ p0_ = p0;
    nema_vg_vertex_t_ c0_ = c0;
    nema_vg_vertex_t_ c1_ = c1;
    nema_vg_vertex_t_ p1_ = p1;
    do  {
        // Calculate all the mid-points of the line segments
        //----------------------
        nema_vg_vertex_t_ p01 = {(p0_.x + c0_.x) * 0.5f, (p0_.y + c0_.y) * 0.5f};
        nema_vg_vertex_t_ p12 = {(c0_.x + c1_.x) * 0.5f, (c0_.y + c1_.y) * 0.5f};
        nema_vg_vertex_t_ p23 = {(c1_.x + p1_.x) * 0.5f, (c1_.y + p1_.y) * 0.5f};

        nema_vg_vertex_t_ p012 = {(p01.x + p12.x) * 0.5f, (p01.y + p12.y) * 0.5f};
        nema_vg_vertex_t_ p123 = {(p12.x + p23.x) * 0.5f, (p12.y + p23.y) * 0.5f};

        nema_vg_vertex_t_ m   = {(p012.x + p123.x) * 0.5f, (p012.y + p123.y) * 0.5f};
        nema_vg_vertex_t_ mid = {(p0_.x + p1_.x) * 0.5f, (p0_.y + p1_.y) * 0.5f};

        nema_vg_vertex_t_ d = {nema_abs_(mid.x - m.x), nema_abs_(mid.y - m.y)};

        if (
            (force_tess != 1 &&  d.x < 0.5f &&  d.y < 0.5f)
            || nemavg_context->info.tess_stack_idx >= STACK_DEPTH
            )
        {
            --iter;
            if((first_run == 1) && (nemavg_context->info.draw_join == 1)){ //the first part needs a bevel join with the previous line
                raster_stroked_line_(p0_, m, 1);                   // we need to check if it was from an arc that sends many cubics
                first_run = 0;                                     // to not put intermediate joins
            }
            else{
                raster_stroked_line_(p0_, m, 0);
            }
            raster_stroked_line_(m, p1_, 0);

            if (nemavg_context->info.tess_stack_idx != 0) {
                nema_vg_vertex_t_ p1__ = p1_;
                tess_stack_pop_vertex(&c0_, &c1_, &p1_);
                p0_ = p1__;
            }
            continue;
        }

        force_tess = 0;

        // Continue subdivision
        //----------------------
        tess_stack_push_vertex(p123, p23, p1_);
        ++iter;

        c0_ = p01;
        c1_ = p012;
        p1_ = m;
    } while ( iter != 0 );
}

//rasterize a quadratic bezier via tesselation (fixed segment count) - draw with lines
static void
stroke_quadratic_decasteljau( nema_vg_vertex_t_ p0,
                              nema_vg_vertex_t_ c,
                              nema_vg_vertex_t_ p1)
{
    int iter = 1;
    int first_run = 1;
    nema_vg_vertex_t_ p0_ = p0;
    nema_vg_vertex_t_ c_  = c;
    nema_vg_vertex_t_ p1_ = p1;

    do  {
        // Calculate all the mid-points of the line segments
        //----------------------
        nema_vg_vertex_t_ p01 = {(p0_.x + c_.x ) * 0.5f, (p0_.y + c_.y) * 0.5f};
        nema_vg_vertex_t_ p12 = {(c_.x  + p1_.x) * 0.5f, (c_.y + p1_.y) * 0.5f};

        nema_vg_vertex_t_ p012 = {(p01.x + p12.x) * 0.5f, (p01.y + p12.y) * 0.5f};

        nema_vg_vertex_t_ d = {p1_.x - p0_.x, p1_.y - p0_.y};
        nema_vg_float_t  d1 = nema_abs_(((c_.x - p1_.x) * d.y - (c_.y - p1_.y) * d.x));

        uint8_t subdivide = 0U;

        if ( nemavg_context->info.faster_quality != 0U ) {
            subdivide = ((d1* d1) > (d.x*d.x + d.y*d.y)) ? 1U : 0U;;
        } else {
            subdivide = ((d1* d1) > (d.x*d.x + d.y*d.y)*0.5f) ? 1U : 0U;
        }

        if ( subdivide != 0U ) {
            // Continue subdivision
            //----------------------
            tess_stack_push_vertex(p012, p12, p1_);
            c_  = p01;
            p1_ = p012;
            ++iter;
        } else {
            // Try to approximate the full quadratic curve by two straight lines
            //------------------
            --iter;
            if(first_run == 1){ //the first part needs a bevel join with the previous line
                raster_stroked_line_(p0_, p012, 1);
                first_run = 0;
            }else{
                raster_stroked_line_(p0_, p012, 0);
            }
            raster_stroked_line_(p012, p1_, 0);
            tess_stack_pop_vertex(&p0_, &c_, &p1_);
        }
    } while ( iter != 0 );
}

// forward declarations
static inline uint32_t
process_path_vertex( nema_vbuf_t_ *vb, const uint8_t seg,
                    nema_vg_info_t_* path_info);

static uint32_t
fill_arc(nema_vg_path_t* path, nema_vg_info_t_* path_info);


static inline nema_vg_vertex_t_
find_implicit_vertices(nema_vg_info_t_* path_info);

inline static uint32_t
stroke_path(nema_vg_path_t* path);

#ifdef NEMAPVG
#include "PVG/nema_bezier.cc"
#include "PVG/nema_vg_pvg.cc"
#else
#include "Pico/nema_vg_pico.cc"
#endif // NEMAPVG

static inline void
render_line(nema_vg_vertex_t_* p1, nema_vg_vertex_t_* p2)
{
    if(nemavg_context->fill_rule == NEMA_VG_STROKE){
        line_draw(p1, p2);
    }
    else if(nemavg_context->fill_rule == NEMA_VG_FILL_NON_ZERO){
        line_non_zero(p1, p2);
    }
    else{ //EVEN-ODD
        line_even_odd(p1, p2);
    }
}

#include "process_path_vertex.cc"

// return 1 if geometry needs to be culled
static int
cull_geometry(void) {

    if (    stencil->dirty_area_p1.x <= 0.f
         && stencil->dirty_area_p2.x <= 0.f
         && stencil->dirty_area_p3.x <= 0.f
         && stencil->dirty_area_p4.x <= 0.f
        ) { return 1; }

    if (    stencil->dirty_area_p1.y <= 0.f
         && stencil->dirty_area_p2.y <= 0.f
         && stencil->dirty_area_p3.y <= 0.f
         && stencil->dirty_area_p4.y <= 0.f
        ) { return 1; }

    if (    stencil->dirty_area_p1.x >= (float)stencil->width
         && stencil->dirty_area_p2.x >= (float)stencil->width
         && stencil->dirty_area_p3.x >= (float)stencil->width
         && stencil->dirty_area_p4.x >= (float)stencil->width
        ) { return 1; }

    if (    stencil->dirty_area_p1.y >= (float)stencil->height
         && stencil->dirty_area_p2.y >= (float)stencil->height
         && stencil->dirty_area_p3.y >= (float)stencil->height
         && stencil->dirty_area_p4.y >= (float)stencil->height
        ) { return 1; }


    return 0;
}

// returns NEMA_VG_ERR_COORDS_OUT_OF_RANGE if coords are out of range
static uint32_t
check_range(void) {
    #ifndef NEMA_VG_DONT_CHECK_COORD_RANGE
        if ( stencil->dirty_area_p1.x >  MAX_COORD ) {
            return NEMA_VG_ERR_COORDS_OUT_OF_RANGE; }
        if ( stencil->dirty_area_p1.y >  MAX_COORD ) {
            return NEMA_VG_ERR_COORDS_OUT_OF_RANGE; }
        if ( stencil->dirty_area_p2.x >  MAX_COORD ) {
            return NEMA_VG_ERR_COORDS_OUT_OF_RANGE; }
        if ( stencil->dirty_area_p2.y >  MAX_COORD ) {
            return NEMA_VG_ERR_COORDS_OUT_OF_RANGE; }
        if ( stencil->dirty_area_p3.x >  MAX_COORD ) {
            return NEMA_VG_ERR_COORDS_OUT_OF_RANGE; }
        if ( stencil->dirty_area_p3.y >  MAX_COORD ) {
            return NEMA_VG_ERR_COORDS_OUT_OF_RANGE; }
        if ( stencil->dirty_area_p4.x >  MAX_COORD ) {
            return NEMA_VG_ERR_COORDS_OUT_OF_RANGE; }
        if ( stencil->dirty_area_p4.y >  MAX_COORD ) {
            return NEMA_VG_ERR_COORDS_OUT_OF_RANGE; }

        if ( stencil->dirty_area_p1.x < MIN_COORD ) {
            return NEMA_VG_ERR_COORDS_OUT_OF_RANGE; }
        if ( stencil->dirty_area_p1.y < MIN_COORD ) {
            return NEMA_VG_ERR_COORDS_OUT_OF_RANGE; }
        if ( stencil->dirty_area_p2.x < MIN_COORD ) {
            return NEMA_VG_ERR_COORDS_OUT_OF_RANGE; }
        if ( stencil->dirty_area_p2.y < MIN_COORD ) {
            return NEMA_VG_ERR_COORDS_OUT_OF_RANGE; }
        if ( stencil->dirty_area_p3.x < MIN_COORD ) {
            return NEMA_VG_ERR_COORDS_OUT_OF_RANGE; }
        if ( stencil->dirty_area_p3.y < MIN_COORD ) {
            return NEMA_VG_ERR_COORDS_OUT_OF_RANGE; }
        if ( stencil->dirty_area_p4.x < MIN_COORD ) {
            return NEMA_VG_ERR_COORDS_OUT_OF_RANGE; }
        if ( stencil->dirty_area_p4.y < MIN_COORD ) {
            return NEMA_VG_ERR_COORDS_OUT_OF_RANGE; }
    #endif
    return NEMA_VG_ERR_NO_ERROR;
}

static uint32_t
stencil_set_dirty_area(nema_vg_path_t* path){
    // here half_stroke is in:
    // NemaP: screen space
    // NemaPVG: object space
    float half_stroke_x = nemavg_context->info.stroke_w * 0.5f;
    float half_stroke_y = nemavg_context->info.stroke_w * 0.5f;
    if((nemavg_context->info.do_fat_stroke == 1) && (nemavg_context->info.has_cap == 1U))
    {
        //add the start/end cap stroke size
        half_stroke_x += nemavg_context->info.stroke_w + 1.f;
        half_stroke_y += nemavg_context->info.stroke_w + 1.f;
    }
    nema_vg_float_t incrx = 4.f;
    nema_vg_float_t incry = 1.f;

    // clear them now in order to be used later by calc_tranformed_bbox function
    nemavg_context->info.global_sx_factor = 0.0f;
    nemavg_context->info.global_sy_factor = 0.0f;

    if (nemavg_context->info.has_transformation != 0U) {
        #if 1
        float sx;
        float sy;

        get_matrix_scale_factors(nemavg_context->path_plus_global_m, &sx, &sy);
        sx = nema_absf(sx);
        sy = nema_absf(sy);
        nemavg_context->info.global_sx_factor = sx;
        nemavg_context->info.global_sy_factor = sy;

        if (sx > 0.001f) {
            incrx = 4.f/sx;
#ifndef HW_VERTEX_MMUL
            // since half_stroke is in screen space for NemaP,
            // we need to bring it to object space because,
            // this information will be encoded in the stencil_area
            // which will then be multiplied by the transformation matrix
            half_stroke_x /= sx;
#endif
        }
        if (sy > 0.001f) {
            incry = 4.f/sy;
#ifndef HW_VERTEX_MMUL
            // since half_stroke is in screen space for NemaP,
            // we need to bring it to object space because,
            // this information will be encoded in the stencil_area
            // which will then be multiplied by the transformation matrix
            half_stroke_y /= sy;
#endif
        }
        #else
        incrx = 0.05f*path->shape.bbox.w;
        incry = 0.05f*path->shape.bbox.h;
        #endif
    }

    incrx += half_stroke_x;
    incry += half_stroke_y;

    stencil->dirty_area_p1.x = path->shape.bbox.min.x - incrx;
    stencil->dirty_area_p1.y = path->shape.bbox.min.y - incry;
    stencil->dirty_area_p2.x = path->shape.bbox.min.x + path->shape.bbox.w + incrx;
    stencil->dirty_area_p2.y = path->shape.bbox.min.y - incry;
    stencil->dirty_area_p3.x = path->shape.bbox.min.x + path->shape.bbox.w + incrx;
    stencil->dirty_area_p3.y = path->shape.bbox.min.y + path->shape.bbox.h + incry;
    stencil->dirty_area_p4.x = path->shape.bbox.min.x - incrx;
    stencil->dirty_area_p4.y = path->shape.bbox.min.y + path->shape.bbox.h + incry;

    if(nemavg_context->info.has_transformation != 0U){
        nema_mat3x3_mul_vec_(nemavg_context->path_plus_global_m, &(stencil->dirty_area_p1.x), &(stencil->dirty_area_p1.y));
        nema_mat3x3_mul_vec_(nemavg_context->path_plus_global_m, &(stencil->dirty_area_p2.x), &(stencil->dirty_area_p2.y));
        nema_mat3x3_mul_vec_(nemavg_context->path_plus_global_m, &(stencil->dirty_area_p3.x), &(stencil->dirty_area_p3.y));
        nema_mat3x3_mul_vec_(nemavg_context->path_plus_global_m, &(stencil->dirty_area_p4.x), &(stencil->dirty_area_p4.y));
    }

    return NEMA_VG_ERR_NO_ERROR;
}

static inline void
broaden_stencil_dirty_area(float* upper_quad, float* bottom_quad){
    int points[4] = {0,0,0,0}; // [tr , br , bl, tl]

    nema_vg_vertex_t_ vertices[4] = {{stencil->dirty_area_p1.x,stencil->dirty_area_p1.y},
                                    {stencil->dirty_area_p2.x,stencil->dirty_area_p2.y},
                                    {stencil->dirty_area_p3.x,stencil->dirty_area_p3.y},
                                    {stencil->dirty_area_p4.x,stencil->dirty_area_p4.y}};

    nema_vg_vertex_t_ *smalest_xy = &vertices[0]; //points[0]
    int smalest_xy_index = 0;

    for (int i=1;i<4; i++){
        if(smalest_xy->x > vertices[i].x){
            smalest_xy = &vertices[i];
            smalest_xy_index = i;
        }
        else if(nema_floats_equal(smalest_xy->x,vertices[i].x)){
            if(smalest_xy->y > vertices[i].y){
                smalest_xy = &vertices[i];
                smalest_xy_index = i;
            }
        }
        else{
            //MISRA
        }
    }

    float dotQuad = ((vertices[(smalest_xy_index-1 + 4)%4].x - vertices[smalest_xy_index].x)*
                     (vertices[(smalest_xy_index+1)%4].y -     vertices[smalest_xy_index].y))-
                    ((vertices[(smalest_xy_index+1)%4].x -     vertices[smalest_xy_index].x)*
                     (vertices[(smalest_xy_index-1 + 4)%4].y - vertices[smalest_xy_index].y));

    if (dotQuad > 0.0f){
        vertices[0] = stencil->dirty_area_p4;
        vertices[1] = stencil->dirty_area_p3;
        vertices[2] = stencil->dirty_area_p2;
        vertices[3] = stencil->dirty_area_p1;
    }

    nema_vg_vertex_t_ *tr = &vertices[0]; //points[0]
    nema_vg_vertex_t_ *br = &vertices[0]; //points[1]
    nema_vg_vertex_t_ *bl = &vertices[0]; //points[2]
    nema_vg_vertex_t_ *tl = &vertices[0]; //points[3]

    float broaden_by_pixels = 4.f;

    // find_tr()
    for (int i=1;i<4; i++){
        if(tr->y > vertices[i].y){
            tr = &vertices[i];
            points[0]=i;
        }
        else if(nema_floats_equal(tr->y,vertices[i].y)){
            if(tr->x < vertices[i].x){
                tr = &vertices[i];
                points[0]=i;
            }
        }
        else{
            //MISRA
        }
    }

    // find_tl()
    for (int i=1;i<4; i++){
        if(tl->y > vertices[i].y){
            tl = &vertices[i];
            points[3]=i;
        }
        else if(nema_floats_equal(tl->y,vertices[i].y)){
            if(tl->x > vertices[i].x){
                tl = &vertices[i];
                points[3]=i;
            }
        }
        else{
            //MISRA
        }
    }

    //find_br()
    for (int i=1;i<4; i++){
        if(br->y < vertices[i].y){
            br = &vertices[i];
            points[1]=i;
        }
        else if(nema_floats_equal(br->y,vertices[i].y)){
            if(br->x < vertices[i].x){
                br = &vertices[i];
                points[1]=i;
            }
        }
        else{
            //MISRA
        }
    }

    // find_bl()
    for (int i=1;i<4; i++){
        if(bl->y < vertices[i].y){
            bl = &vertices[i];
            points[2]=i;
        }
        else if(nema_floats_equal(bl->y,vertices[i].y)){
            if(bl->x > vertices[i].x){
                bl = &vertices[i];
                points[2]=i;
            }
        }
        else{
            //MISRA
        }
    }

    //Top left corner == Top right corner
    if(tr == tl){

                upper_quad[0] = (tr->x+broaden_by_pixels)/4.f;
                upper_quad[1] = tr->y;
                upper_quad[2] = (vertices[(points[0]+ 1)%4].x + broaden_by_pixels)/4.f;
                upper_quad[3] = (vertices[(points[0]+ 1)%4].y);
                upper_quad[4] = (vertices[(points[0]+ 3)%4].x - broaden_by_pixels)/4.f;
                upper_quad[5] = (vertices[(points[0]+ 3)%4].y);
                upper_quad[6] = (tr->x-broaden_by_pixels)/4.f;
                upper_quad[7] = tr->y;
    }else{

                upper_quad[0] = (tr->x+broaden_by_pixels)/4.f;
                upper_quad[1] = tr->y;
                upper_quad[2] = (vertices[(points[0]+ 1)%4].x + broaden_by_pixels)/4.f;
                upper_quad[3] = (vertices[(points[0]+ 1)%4].y);
                upper_quad[4] = (vertices[(points[0]+ 2)%4].x - broaden_by_pixels)/4.f;
                upper_quad[5] = (vertices[(points[0]+ 2)%4].y);
                upper_quad[6] = (tl->x-broaden_by_pixels)/4.f;
                upper_quad[7] = tl->y;
    }

    //Bottom left corner == Bottom right corner
    if(br == bl){

                bottom_quad[0] = (br->x+broaden_by_pixels)/4.f;
                bottom_quad[1] = br->y;
                bottom_quad[2] = (br->x-broaden_by_pixels)/4.f;
                bottom_quad[3] = br->y;
                bottom_quad[4] = (vertices[(points[1]+ 1)%4].x - broaden_by_pixels)/4.f;
                bottom_quad[5] = (vertices[(points[1]+ 1)%4].y);
                bottom_quad[6] = (vertices[(points[1]+ 3)%4].x + broaden_by_pixels)/4.f;
                bottom_quad[7] = (vertices[(points[1]+ 3)%4].y);
    }else{

                bottom_quad[0] = (br->x+broaden_by_pixels)/4.f;
                bottom_quad[1] = br->y;
                bottom_quad[2] = (bl->x-broaden_by_pixels)/4.f;
                bottom_quad[3] = bl->y;
                bottom_quad[4] = (vertices[(points[1]+ 2)%4].x - broaden_by_pixels)/4.f;
                bottom_quad[5] = (vertices[(points[1]+ 2)%4].y);
                bottom_quad[6] = (vertices[(points[1]+ 3)%4].x + broaden_by_pixels)/4.f;
                bottom_quad[7] = (vertices[(points[1]+ 3)%4].y);
    }
}

static uint32_t
stencil_clear_dirty_area(void){

    uint32_t w = (uint32_t)stencil->width;
    uint32_t w_ = w + 3U;
    uint32_t shifted_w = ((w_)>>2);
    uint32_t h = (uint32_t)stencil->height;
    int32_t stride = (int32_t)(shifted_w)*4;

    uint32_t clear_color = 0x80808080U;

#ifndef NEMAPVG
    if ( nemavg_context->fill_rule != NEMA_VG_FILL_NON_ZERO ) {
        clear_color = 0U;
    }
#else
    if ( nemavg_context->fill_rule == NEMA_VG_STROKE ) {
        clear_color = 0U;
    }
#endif

#if 1
    {
    const int num_cmds = 8;
    uint32_t *cmd_array = nema_cl_get_space(num_cmds);
    if (cmd_array == NULL) {
        return NEMA_VG_ERR_CL_FULL;
    }
#ifdef NEMA_P_IMEM_16
    static const uint32_t cmd_pix3[] = {0x080c0182U, 0x8a8761c6U};
    nema_load_frag_shader(&cmd_pix3[0], 1, SHADER_ADDR_PIXOUT3);
#endif //NEMA_P_IMEM_16
    int cmd_idx = 0;
    // 1
    cmd_array[cmd_idx] = NEMA_TEX3_FSTRIDE; cmd_idx++;
    cmd_array[cmd_idx] = ((unsigned)NEMA_RGBA8888 << 24) | ((unsigned)stride & 0xffffU); cmd_idx++;
    // 2
    cmd_array[cmd_idx] = NEMA_TEX3_RESXY; cmd_idx++;
    cmd_array[cmd_idx] = (h << 16) | shifted_w; cmd_idx++;
    // 3
    cmd_array[cmd_idx] = NEMA_ROPBLENDER_BLEND_MODE; cmd_idx++;
    cmd_array[cmd_idx] = 1U; cmd_idx++;
    // 4
    cmd_array[cmd_idx] = NEMA_CODEPTR; cmd_idx++;
#ifdef NEMA_P_IMEM_16
    //SHADER_ADDR_PIXOUT3 is 0 and MISRA is complaining
    cmd_array[cmd_idx] = 0x1c1f0000U; cmd_idx++;
#else
    cmd_array[cmd_idx] = 0x1c1f0000U | SHADER_ADDR_PIXOUT3; cmd_idx++;
#endif //NEMA_P_IMEM_16
    // 5
    cmd_array[cmd_idx] = NEMA_DRAW_COLOR; cmd_idx++;
    cmd_array[cmd_idx] = clear_color; cmd_idx++;
    // 6
    cmd_array[cmd_idx] = NEMA_CLIPMIN; cmd_idx++;
    cmd_array[cmd_idx] = 0U; cmd_idx++;
    // 7
    cmd_array[cmd_idx] = NEMA_CLIPMAX; cmd_idx++;
    cmd_array[cmd_idx] = (h << 16) | (shifted_w & 0xffffU); cmd_idx++;
    // X - is this needed?
    cmd_array[cmd_idx] = NEMA_MATMULT; cmd_idx++;
    cmd_array[cmd_idx] = 0x90000000U;
    }
#else
    nema_cl_add_cmd(NEMA_TEX3_FSTRIDE, ((unsigned)NEMA_RGBA8888 << 24) | ((unsigned)stride & 0xffffU));
    nema_cl_add_cmd(NEMA_TEX3_RESXY, (h << 16) | shifted_w);
    nema_set_blend(NEMA_BL_SRC, NEMA_TEX3, NEMA_NOTEX, NEMA_NOTEX);
    nema_set_raster_color(clear_color);
    nema_set_clip_temp(0, 0, shifted_w, h);
#endif

    uint32_t prev_aa = nema_enable_aa_flags(0U);

    float  upper_quad[8] = { 0 };
    float bottom_quad[8] = { 0 };

    //Define the broadened stencil area with 2 quads
    broaden_stencil_dirty_area(upper_quad,bottom_quad);

    nema_vg_vertex_t_ p0, p1, p2, p3;

    //Clear the stencil for the broadened areas
    p0.x = upper_quad[0];
    p0.y = upper_quad[1];
    p1.x = upper_quad[2];
    p1.y = upper_quad[3];
    p2.x = upper_quad[4];
    p2.y = upper_quad[5];
    p3.x = upper_quad[6];
    p3.y = upper_quad[7];
    nema_raster_quad_f_(p0, p1, p2, p3);
    p0.x = bottom_quad[0];
    p0.y = bottom_quad[1];
    p1.x = bottom_quad[2];
    p1.y = bottom_quad[3];
    p2.x = bottom_quad[4];
    p2.y = bottom_quad[5];
    p3.x = bottom_quad[6];
    p3.y = bottom_quad[7];
    nema_raster_quad_f_(p0, p1, p2, p3);

    (void)nema_enable_aa_flags(prev_aa);

    nema_set_clip_pop();
    nema_cl_add_cmd(NEMA_TEX3_FSTRIDE, ((unsigned)NEMA_L8 << 24) | ((unsigned)w & 0xffffU));
    nema_cl_add_cmd(NEMA_TEX3_RESXY, (h << 16) | w);

    return NEMA_VG_ERR_NO_ERROR;
}

// -----------------------------------------------------------------------------------------
// ARC HELPERS START - TO BE MOVED IN A DIFFERENT FILE???
// -----------------------------------------------------------------------------------------

#ifndef NEMA_PI
#define NEMA_PI 3.14159265358979323846f
#endif
/* Given: Points (x0, y0) and (x1, y1)
* Return: TRUE if a solution exists, FALSE otherwise
* Circle centers are written to (cx0, cy0) and (cx1, cy1)
*/
static uint32_t
findUnitCircles(const nema_vg_info_t_* path_info,
                uint32_t CW, uint32_t large,
                float *cx, float *cy,
                float *irx, float *iry,
                float *theta0_deg, float *theta1_deg
                )
{
    float rx  = path_info->ctrl0.x;
    float ry  = path_info->ctrl0.y;
    float rot_deg = path_info->ctrl1.x;
    float x0p  = path_info->previous_ctrl.x;
    float y0p  = path_info->previous_ctrl.y;
    float x1p  = path_info->previous_data.x;
    float y1p  = path_info->previous_data.y;

    // Ensure radii are positive
    rx = nema_absf(rx);
    ry = nema_absf(ry);

    /* Convert rotation angle from degrees to radians */
    float rot_rad = nema_deg_to_rad(rot_deg);
    /* Pre-compute rotation matrix entries */
    float cosine = COS_(rot_rad);
    float sine = SIN_(rot_rad);
    /* Transform (x0, y0) and (x1, y1) into unit space */
    /* using (inverse) rotate, followed by (inverse) scale */
    float x0 = ( x0p*cosine + y0p*sine  )/rx;
    float y0 = (-x0p*sine   + y0p*cosine)/ry;
    float x1 = ( x1p*cosine + y1p*sine  )/rx;
    float y1 = (-x1p*sine   + y1p*cosine)/ry;

    /* Compute differences and averages */
    float dx = x0 - x1;
    float dy = y0 - y1;
    float xm = (x0 + x1)*0.5f;
    float ym = (y0 + y1)*0.5f;

    /* Solve for intersecting unit circles */
    float dsq = dx*dx + dy*dy;
    if (dsq == 0.f) {
        return NEMA_VG_ERR_INVALID_ARC_DATA; /* Points are coincident */
    }

    float disc = 1.f/dsq - 1.f/4.f;

    if (disc < 0.f) {
        disc = 0.f;
        float scale = SQRT_(dsq/4.f);
        rx *= scale;
        ry *= scale;

        x0 = ( x0p*cosine + y0p*sine  )/rx;
        y0 = (-x0p*sine   + y0p*cosine)/ry;
        x1 = ( x1p*cosine + y1p*sine  )/rx;
        y1 = (-x1p*sine   + y1p*cosine)/ry;
        dx = x0 - x1;
        dy = y0 - y1;
        xm = (x0 + x1)*0.5f;
        ym = (y0 + y1)*0.5f;
    }

    *irx = rx;
    *iry = ry;

    float s = SQRT_(disc);
    float sdx = s*dx;
    float sdy = s*dy;

    float ucx = xm + sdy;
    float ucy = ym - sdx;

    float theta00_rad = ATAN2_( y0-ucy, x0-ucx );
    float theta01_rad = ATAN2_( y1-ucy, x1-ucx );
    float theta00_deg = nema_rad_to_deg( theta00_rad );
    float theta01_deg = nema_rad_to_deg( theta01_rad );

    float dtheta0_deg = theta01_deg - theta00_deg;
    if (dtheta0_deg < 0.f) { dtheta0_deg+=360.f; }

    if ( ( (dtheta0_deg <  180.f) && ((CW ^ large) != 0U) ) ||
         ( (dtheta0_deg >= 180.f) && ((CW ^ large) == 0U) ) ) {
        *theta0_deg = theta00_deg;
        *theta1_deg = theta01_deg;
    }
    else {
        ucx = xm - sdy;
        ucy = ym + sdx;
        float theta10_deg = nema_rad_to_deg( ATAN2_(y0-ucy, x0-ucx) );
        float theta11_deg = nema_rad_to_deg( ATAN2_(y1-ucy, x1-ucx) );

        *theta0_deg = theta10_deg;
        *theta1_deg = theta11_deg;
    }

    ucx *= rx;
    ucy *= ry;

    *cx = ucx*cosine - ucy*sine;
    *cy = ucx*sine   + ucy*cosine;

    return NEMA_VG_ERR_NO_ERROR;
}

void transform_arc( nema_vg_vertex_t_ *v, float cosine, float sine, float rx, float ry, float cx, float cy ) {
      nema_vg_vertex_t_ v1;
      v1.x = v->x * rx * cosine - v->y * ry * sine   + cx;
      v1.y = v->x * rx * sine   + v->y * ry * cosine + cy;
      v->x = v1.x;
      v->y = v1.y;
}

static void
render_arc(nema_vg_path_t* path,  nema_vg_info_t_* path_info, float angle0_deg, float angle1_deg, float cx, float cy,
            float rx, float ry, uint8_t force_point ) {
    (void) path;
    float angle0_rad = nema_deg_to_rad( angle0_deg );
    float angle1_rad = nema_deg_to_rad( angle1_deg );
    float angle_rad = angle1_rad-angle0_rad;
    float angle_rad025 = (angle_rad)*0.25f;
    float k = 4.f/3.f * TAN_( angle_rad025 );
    float cosine = COS_(angle_rad);
    float sine   = SIN_(angle_rad);

    nema_vg_info_t_ arc_info;

    arc_info.p1.x    = 1.f;
    arc_info.p1.y    = 0.f;
    arc_info.ctrl0.x = 1.f;
    arc_info.ctrl0.y = k;
    arc_info.ctrl1.x = (cosine + k * sine);
    arc_info.ctrl1.y = (sine - k * cosine);
    arc_info.p2.x    = cosine;
    arc_info.p2.y    = sine;

    float rot_rad = path_info->ctrl1.x/180.f*NEMA_PI;
    // float rx      = path_info->ctrl0.x;
    // float ry      = path_info->ctrl0.y;
    float cos0    = COS_(angle0_rad);
    float sin0    = SIN_(angle0_rad);
    float cos_rot = COS_(rot_rad);
    float sin_rot = SIN_(rot_rad);

    if ( (force_point & 1U) != 0U ) {
        arc_info.p1 = path_info->p1;
    }
    else
    {
        transform_arc( &arc_info.p1   , cos0, sin0, 1.f, 1.f, 0.f, 0.f);
        transform_arc( &arc_info.p1   , cos_rot, sin_rot, rx, ry, cx, cy);
#ifndef HW_VERTEX_MMUL
        if (nemavg_context->info.has_transformation != 0U)  {
            transform_vertex(nemavg_context->path_plus_global_m, &arc_info.p1);
        }
#endif
    }

    transform_arc( &arc_info.ctrl0, cos0, sin0, 1.f, 1.f, 0.f, 0.f);
    transform_arc( &arc_info.ctrl0, cos_rot, sin_rot, rx, ry, cx, cy);

    transform_arc( &arc_info.ctrl1, cos0, sin0, 1.f, 1.f, 0.f, 0.f);
    transform_arc( &arc_info.ctrl1, cos_rot, sin_rot, rx, ry, cx, cy);

    if ( (force_point & 2U) != 0U ) {
            // CW
        arc_info.p2 = path_info->p2;
    } else
    {
        transform_arc( &arc_info.p2   , cos0, sin0, 1.f, 1.f, 0.f, 0.f);
        transform_arc( &arc_info.p2   , cos_rot, sin_rot, rx, ry, cx, cy);
#ifndef HW_VERTEX_MMUL
        if (nemavg_context->info.has_transformation != 0U)  {
            transform_vertex(nemavg_context->path_plus_global_m, &arc_info.p2);
        }
#endif
    }
#ifndef HW_VERTEX_MMUL
    if (nemavg_context->info.has_transformation != 0U)  {
        transform_vertex(nemavg_context->path_plus_global_m, &arc_info.ctrl0);
        transform_vertex(nemavg_context->path_plus_global_m, &arc_info.ctrl1);
    }
#endif

    if ( fill_cubic(&arc_info) != NEMA_VG_ERR_NO_ERROR ) {
        return;
    }

#ifndef NEMAPVG
    if ( (nemavg_context->fill_rule == NEMA_VG_FILL_NON_ZERO)) {
        // this is not the first quadrant
        // if we draw in non-zero
        // fill internal of ARC with a triangle
        nema_raster_triangle_f( path_info->p1.x , path_info->p1.y,
                                arc_info.p1.x   , arc_info.p1.y  ,
                                arc_info.p2.x   , arc_info.p2.y  );
    }
#endif
}


/* Given: Ellipse parameters rh, rv, rot (in degrees),
* endpoints (x0, y0) and (x1, y1)
* Return: TRUE if a solution exists, FALSE otherwise
* Ellipse centers are written to (cx0, cy0) and (cx1, cy1)
*/
static uint32_t
fill_arc(nema_vg_path_t* path, nema_vg_info_t_* path_info)
{
    float rx  = path_info->ctrl0.x;
    float ry  = path_info->ctrl0.y;

    // Ensure radii are non-zero (else treat this as a straight line)
    if ( nema_float_is_zero(rx) || nema_float_is_zero(ry) ) {
        render_line(&path_info->p1, &path_info->p2);
        return NEMA_VG_ERR_NO_ERROR;
    }

    uint32_t CW, large;
    if ((path_info->seg & NEMA_VG_ARC_CW    ) != 0U)
         { CW = 1U; }
    else { CW = 0U; }

    if ((path_info->seg & NEMA_VG_ARC_LARGE ) != 0U)
         { large = 1U; }
    else { large = 0U; }

    float theta0_deg, theta1_deg;
    float cx, cy;
    NEMA_VG_IF_ERROR(findUnitCircles( path_info, CW, large,
                            &cx, &cy, &rx, &ry, &theta0_deg, &theta1_deg));

    if ( CW == 0U ) {
        // rearrange angles so that we render CW
        float tmp = theta0_deg;
        theta0_deg = theta1_deg;
        theta1_deg = tmp;
    }

    {
        if ( theta0_deg > theta1_deg ) {
            // make sure theta1_deg > theta0_deg
            theta1_deg += 360.f;
        }

        int first_run = 1;
        if ( CW == 1U ) {
            // force p1 for initial run
            uint8_t force_point = 1;
            float theta_deg = theta0_deg;
            const float step_deg = 90.f;
            while ( theta_deg < theta1_deg ) {
                // split arcs to up-to 90degrees so that they can be well represented as Cubic Beziers
                float end_theta = (theta_deg+step_deg) < theta1_deg ? (theta_deg+step_deg) : theta1_deg;

                if ( theta_deg + step_deg >= theta1_deg ) {
                    // force p2
                    force_point += 2U;
                }

                if(first_run != 1)
                {
                    nemavg_context->info.draw_join = 0;
                }
                render_arc(path, path_info, theta_deg, end_theta, cx, cy, rx, ry, force_point);
                first_run = 0;
                force_point = 0U;
                theta_deg += step_deg;
            }
        }
        else {

            // force p1 for initial run
            uint8_t force_point = 1U;
            float theta_deg = theta1_deg;
            const float step_deg = -90.f;
            while ( theta_deg > theta0_deg ) {
                // split arcs to up-to 90degrees so that they can be well represented as Cubic Beziers
                float end_theta = (theta_deg+step_deg) > theta0_deg ? (theta_deg+step_deg) : theta0_deg;

                if ( theta_deg + step_deg <= theta0_deg ) {
                    // force p2
                    force_point += 2U;
                }

                if(first_run != 1)
                {
                    nemavg_context->info.draw_join = 0;
                }
                render_arc(path, path_info, theta_deg, end_theta, cx, cy, rx, ry, force_point);
                first_run = 0;

                force_point = 0U;
                theta_deg += step_deg;
            }
        }
        nemavg_context->info.draw_join = 1;
    }

    return NEMA_VG_ERR_NO_ERROR;
}

// -----------------------------------------------------------------------------------------
// ARC HELPERS END - TO BE MOVED IN A DIFFERENT FILE???
// -----------------------------------------------------------------------------------------
void fill_rounded_cap( const nema_vg_vertex_t_ *cap_center,
                       float radius, float angle_cos_theta, float angle_sin_theta,
                       uint8_t is_end)
{
     float angle_cos_theta_ = angle_cos_theta;
     float angle_sin_theta_ = angle_sin_theta;
    nema_vg_vertex_t_ vertices[49]; // this is the max tesselation steps(48) + 1

    int steps = calculate_steps_from_radius(radius) / 2;
    float step_cos_theta = nema_cos(180.0f / (float)steps);
    float step_sin_theta;
    // when we draw the cap at the end. Negative step_sin_theta means
    // we change winding(Reflection Trig identity)
    if(is_end != 0U)
    {
        step_sin_theta = nema_sin(180.0f / (float)steps);
    }
    else
    {
        step_sin_theta = -nema_sin(180.0f / (float)steps);
    }

    vertices[0].x = (cap_center->x + radius * (-angle_cos_theta_));
    vertices[0].y = (cap_center->y + radius * (-angle_sin_theta_));
    int i;
    for (i=1; i<steps; ++i){
        vertices[i].x = cap_center->x + radius * angle_cos_theta_;
        vertices[i].y = cap_center->y + radius * angle_sin_theta_;

        // Advance the the cos and sin theta. This is
        // a trigonometric identity
        float ns = step_sin_theta * angle_cos_theta_ + step_cos_theta * angle_sin_theta_;
        float nc = step_cos_theta * angle_cos_theta_ - step_sin_theta * angle_sin_theta_;
        angle_cos_theta_ = nc;
        angle_sin_theta_ = ns;
    }
    // we create a triangle fan with 1 less triangle
    // because the last triangle needs to have 2 edges AA
    uint32_t prev_aa = 0;
    if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
        prev_aa = nema_enable_aa(0, 1, 0, 0);
    }
    nema_raster_triangle_fan_f((float *)&(vertices[0].x), steps - 1, 2);

    if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
        (void)nema_enable_aa(0, 1, 1, 0);
    }
    nema_raster_triangle_f(vertices[0].x,       vertices[0].y,
                         vertices[steps-2].x, vertices[steps-2].y,
                         vertices[steps-1].x, vertices[steps-1].y
                         );

    (void)nema_enable_aa_flags(prev_aa);
}

static void find_perpedicular(nema_vg_vertex_t_* perp1, nema_vg_vertex_t_* perp2, float dist, uint8_t end_point)
{
    nema_vg_vertex_t_ p1;
    nema_vg_vertex_t_ p2;
    stroked_line_segment_t_ line;
    uint8_t use_left_point = 0U;

    if(end_point == 1U)
    {
        line = nemavg_context->info.end_line;
        p1 = line.q1;
        p2 = line.q2;
        if((line.q1.x <= line.q0.x) && (line.q2.x <= line.q3.x))
        {
            use_left_point = 1U;
        }
    }
    else
    {
        line = nemavg_context->info.first_line;
        p1 = line.q0;
        p2 = line.q3;
        if((line.q0.x < line.q1.x) && (line.q3.x < line.q2.x))
        {
            use_left_point = 1U;
        }
    }

    float vx = p2.x - p1.x;
    float vy = p2.y - p1.y;
    float len = nema_sqrt( vx*vx + vy*vy );
    float ux = -vy/len;
    float uy = vx/len;

    //choose the right one depending  the line direction
    if (use_left_point == 1U  )// we need the left perpediculars
    {
        if (ux > 0.f )
        {
            perp1->x = p2.x - dist * ux;
            perp1->y = p2.y - dist * uy;
            perp2->x = p1.x - dist * ux;
            perp2->y = p1.y - dist * uy;
        }
        else{
            perp1->x = p2.x + dist * ux;
            perp1->y = p2.y + dist * uy;
            perp2->x = p1.x + dist * ux;
            perp2->y = p1.y + dist * uy;
        }
    }
    else
    {
        if (ux <= 0.f )
        {
            perp1->x = p2.x - dist * ux;
            perp1->y = p2.y - dist * uy;
            perp2->x = p1.x - dist * ux;
            perp2->y = p1.y - dist * uy;
        }
        else{
            perp1->x = p2.x + dist * ux;
            perp1->y = p2.y + dist * uy;
            perp2->x = p1.x + dist * ux;
            perp2->y = p1.y + dist * uy;
        }
    }

}

static void draw_round_cap(uint8_t is_end_cap)
{
    nema_vg_vertex_t_ p0, p1, cap_center;
    float dx, dy, len, angle_cos_theta, angle_sin_theta;
    if((nemavg_context->info.end_line.q0.x == nemavg_context->info.end_line.q1.x) &&
        (nemavg_context->info.end_line.q0.y == nemavg_context->info.end_line.q1.y) &&
        (nemavg_context->info.end_line.q2.x == nemavg_context->info.end_line.q3.x) &&
        (nemavg_context->info.end_line.q2.y == nemavg_context->info.end_line.q3.y)) //NO line was drawn we just add caps
    {

        p1 = nemavg_context->info.end_line.q0;
        p0 = nemavg_context->info.end_line.q2;

#ifndef NEMAPVG
        if((nemavg_context->info.do_not_transform == 0) &&
        ((nemavg_context->flags & NEMA_VG_CONTEXT_STROKE_WIDTH_SCREEN_SPACE) == 0U))
        {
            transform_vertex(nemavg_context->path_plus_global_m, &p0);
            transform_vertex(nemavg_context->path_plus_global_m, &p1);
        }
#endif //NEMAPVG
        dx = p1.x - p0.x;
        dy = p1.y - p0.y;
        len = nemavg_context->stroke.width;
        cap_center.x =  (p1.x + p0.x)*0.5f;
        cap_center.y =  (p1.y + p0.y)*0.5f;
        angle_cos_theta = dx/len;
        angle_sin_theta = dy/len;

        if(is_end_cap == 1U)
        {
            //end cap
            fill_rounded_cap(&cap_center,
                            len*0.5f,
                            angle_cos_theta, angle_sin_theta,
                            1 );
        }else{
            //start cap
            fill_rounded_cap(&cap_center,
                            len*0.5f,
                            angle_cos_theta, angle_sin_theta,
                            0 );
        }
    }
    else
    {
        if(is_end_cap == 1U)
        {
            //end cap
            p0 = nemavg_context->info.end_line.q1;
            p1 = nemavg_context->info.end_line.q2;

#ifndef NEMAPVG
            if((nemavg_context->info.do_not_transform == 0) &&
            ((nemavg_context->flags & NEMA_VG_CONTEXT_STROKE_WIDTH_SCREEN_SPACE) == 0U))
            {
                transform_vertex(nemavg_context->path_plus_global_m, &p0);
                transform_vertex(nemavg_context->path_plus_global_m, &p1);
            }
#endif //NEMAPVG
            dx = p1.x - p0.x;
            dy = p1.y - p0.y;
            len = nemavg_context->stroke.width;
            cap_center.x =  (p1.x + p0.x)*0.5f;
            cap_center.y =  (p1.y + p0.y)*0.5f;
            angle_cos_theta = dx/len;
            angle_sin_theta = dy/len;

            fill_rounded_cap(&cap_center,
                              len*0.5f,
                              angle_cos_theta, angle_sin_theta,
                              1 );
        }
        else
        {
            //start cap
            p0 = nemavg_context->info.first_line.q0;
            p1 = nemavg_context->info.first_line.q3;

#ifndef NEMAPVG
            if((nemavg_context->info.do_not_transform == 0) &&
            ((nemavg_context->flags & NEMA_VG_CONTEXT_STROKE_WIDTH_SCREEN_SPACE) == 0U))
            {
                transform_vertex(nemavg_context->path_plus_global_m, &p0);
                transform_vertex(nemavg_context->path_plus_global_m, &p1);
            }
#endif //NEMAPVG
            dx = p1.x - p0.x;
            dy = p1.y - p0.y;
            len = nemavg_context->stroke.width;
            cap_center.x =  (p1.x + p0.x)*0.5f;
            cap_center.y =  (p1.y + p0.y)*0.5f;
            angle_cos_theta = dx/len;
            angle_sin_theta = dy/len;

            fill_rounded_cap(&cap_center,
                             len*0.5f,
                             angle_cos_theta, angle_sin_theta,
                             0 );
        }
    }
}

static void draw_square_cap(uint8_t is_end_cap)
{
    nema_vg_vertex_t_ p0, p1, p2, p3;
    uint32_t prev_aa = 0;
    if((nemavg_context->info.end_line.q0.x == nemavg_context->info.end_line.q1.x) &&
       (nemavg_context->info.end_line.q0.y == nemavg_context->info.end_line.q1.y) &&
       (nemavg_context->info.end_line.q2.x == nemavg_context->info.end_line.q3.x) &&
       (nemavg_context->info.end_line.q2.y == nemavg_context->info.end_line.q3.y)) //NO line was drawn we just add caps
    {
        if(is_end_cap == 1U)
        {
            p0.x = nemavg_context->info.end_line.q0.x;
            p0.y = nemavg_context->info.end_line.q0.y;
            p1.x = nemavg_context->info.end_line.q0.x + (nemavg_context->info.stroke_w*0.5f);
            p1.y = nemavg_context->info.end_line.q0.y;
            p2.x = nemavg_context->info.end_line.q2.x + (nemavg_context->info.stroke_w*0.5f);
            p2.y = nemavg_context->info.end_line.q2.y;
            p3.x = nemavg_context->info.end_line.q2.x;
            p3.y = nemavg_context->info.end_line.q2.y;

            if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
                prev_aa = nema_enable_aa(1, 1, 1, 0);
            }
            else{
                prev_aa = nema_enable_aa(0, 0, 0, 0);
            }
        }
        else
        {
            p0.x = nemavg_context->info.end_line.q0.x - (nemavg_context->info.stroke_w*0.5f);
            p0.y = nemavg_context->info.end_line.q0.y;
            p1.x = nemavg_context->info.end_line.q0.x;
            p1.y = nemavg_context->info.end_line.q0.y;
            p2.x = nemavg_context->info.end_line.q2.x;
            p2.y = nemavg_context->info.end_line.q2.y;
            p3.x = nemavg_context->info.end_line.q2.x - (nemavg_context->info.stroke_w*0.5f);
            p3.y = nemavg_context->info.end_line.q2.y;

            if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
                prev_aa = nema_enable_aa(1, 0, 1, 1);
            }
            else{
                prev_aa = nema_enable_aa(0, 0, 0, 0);
            }

        }

        draw_quad(&p0, &p1, &p2, &p3); // fix for different end start
        (void)nema_enable_aa_flags(prev_aa);
    }
    else
    {
        if(is_end_cap == 1U)
        {
            //end cap
            p0 = nemavg_context->info.end_line.q1;
            p1 = nemavg_context->info.end_line.q2;
            find_perpedicular(&p2, &p3, nemavg_context->info.stroke_w*0.5f, 1);

            if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
                prev_aa = nema_enable_aa(0, 1, 1, 1);
            }
            else{
                prev_aa = nema_enable_aa(0, 0, 0, 0);
            }

            draw_quad(&p0, &p1, &p2, &p3);
        }
        else
        {
            //start cap
            p0 = nemavg_context->info.first_line.q0;
            p1 = nemavg_context->info.first_line.q3;
            find_perpedicular(&p2, &p3, nemavg_context->info.stroke_w*0.5f, 0);

            if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
                prev_aa = nema_enable_aa(0, 1, 1, 1);
            }
            else{
                prev_aa = nema_enable_aa(0, 0, 0, 0);
            }

            draw_quad(&p0, &p1, &p2, &p3);

        }

        if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
            (void)nema_enable_aa_flags(prev_aa);
        }
    }
}

uint32_t draw_caps(void)
{
    //start cap
    if (nemavg_context->stroke.start_cap_style == NEMA_VG_CAP_BUTT)
    {
        //do nothing
    }
    else if (nemavg_context->stroke.start_cap_style == NEMA_VG_CAP_SQUARE)
    {
        draw_square_cap(0);
    }
    else if (nemavg_context->stroke.start_cap_style == NEMA_VG_CAP_ROUND)
    {
        draw_round_cap(0);
    }
    else{
        return NEMA_VG_ERR_INVALID_CAP_STYLE;
    }

    //end cap
    if (nemavg_context->stroke.end_cap_style== NEMA_VG_CAP_BUTT)
    {
        //do nothing
    }
    else if (nemavg_context->stroke.end_cap_style == NEMA_VG_CAP_SQUARE)
    {
        draw_square_cap(1);
    }
    else if (nemavg_context->stroke.end_cap_style == NEMA_VG_CAP_ROUND)
    {
        draw_round_cap(1);
    }
    else{
        return NEMA_VG_ERR_INVALID_CAP_STYLE;
    }

    return NEMA_VG_ERR_NO_ERROR;
}

inline static uint32_t
stroke_path(nema_vg_path_t* path) {
    PRINT_FUNC_ENTRY;

    nema_vbuf_t_ *vb = &path->shape;

    size_t idx_seg = 0;

    nema_vg_info_t_ path_info = {0};

    path_info.no_move = 1;
    path_info.new_path = 1;
    path_info.no_close = 1;
    uint8_t seg;
    uint8_t seg_no_rel;

#ifndef NEMAPVG
    if((nemavg_context->info.do_fat_stroke == 1) &&
      ((nemavg_context->flags & NEMA_VG_CONTEXT_STROKE_WIDTH_SCREEN_SPACE) == 0U) &&
       (nemavg_context->info.do_not_transform == 0 ))
    {
        nemavg_context->info.has_transformation = 0U;
    }
#endif

    while (idx_seg < vb->seg_size)  {

        seg = (vb->seg)[idx_seg]; idx_seg++;
        seg_no_rel = (uint8_t)(seg & NEMA_VG_NO_REL_MASK); //remove rel from seg to avoid multi checking
        NEMA_VG_IF_ERROR(process_path_vertex(vb, seg, &path_info));

        if( (path_info.new_path == 0U) && ((seg & NEMA_VG_PRIM_NO_STROKE) == 0U) ) {
            if ( seg_no_rel == NEMA_VG_PRIM_BEZIER_QUAD || seg_no_rel == NEMA_VG_PRIM_BEZIER_SQUAD ) {
                NEMA_VG_IF_ERROR(fill_quad(&path_info));
            }
            else if ( seg_no_rel == NEMA_VG_PRIM_BEZIER_CUBIC || seg_no_rel == NEMA_VG_PRIM_BEZIER_SCUBIC ) {
                NEMA_VG_IF_ERROR(fill_cubic(&path_info));
            }
            else if ( (seg & NEMA_VG_PRIM_MASK) == NEMA_VG_PRIM_ARC ) {
                NEMA_VG_IF_ERROR(fill_arc(path, &path_info));
            }
            else if( seg_no_rel == NEMA_VG_PRIM_POLYGON || seg_no_rel == NEMA_VG_PRIM_POLYLINE ) {
                // The polygon/polyline segments are already handled inside process_path_vertex...
            }
            else {
                line_draw(&path_info.p1, &path_info.p2);
            }
        }else{ //MOVE
            flush_lines_(0, seg);
            if ((path_info.no_close != 0U) && (idx_seg != 1U)) //when there a second move in the path draw the previous caps
            {
                (void) draw_caps(); //draws both end and start cap
            }
        }

    }

    int is_close = (path_info.no_close == 0U) ? (int)1 : (int)0;
    flush_lines_(is_close, 0);

    if (is_close == 0)
    {
        (void) draw_caps(); //draws both end and start cap
    }

    return NEMA_VG_ERR_NO_ERROR;
}

static uint32_t
calc_transformed_bbox(nema_vg_path_t* path)
{
    // here half_stroke is in:
    // NemaP: screen space
    // NemaPVG: object space
    float half_stroke_x = nemavg_context->info.stroke_w * 0.5f;
    float half_stroke_y = nemavg_context->info.stroke_w * 0.5f;
    if((nemavg_context->info.do_fat_stroke == 1) && (nemavg_context->info.has_cap == 1U))
    {
        //add the cap start/end stroke size
        half_stroke_x += nemavg_context->info.stroke_w + 1.f;
        half_stroke_y += nemavg_context->info.stroke_w + 1.f;
    }
    float incrx = 1.0f;
    float incry = 1.0f;

    if (nemavg_context->info.global_sx_factor > 0.001f) {
        incrx = 1.0f/nemavg_context->info.global_sx_factor;
#ifndef HW_VERTEX_MMUL
            // since half_stroke is in screen space for NemaP,
            // we need to bring it to object space because,
            // this information will be encoded in the stencil_area
            // which will then be multiplied by the transformation matrix
            half_stroke_x /= nemavg_context->info.global_sx_factor;
#endif
    }
    if (nemavg_context->info.global_sy_factor > 0.001f) {
        incry = 1.0f/nemavg_context->info.global_sy_factor;
#ifndef HW_VERTEX_MMUL
            // since half_stroke is in screen space for NemaP,
            // we need to bring it to object space because,
            // this information will be encoded in the stencil_area
            // which will then be multiplied by the transformation matrix
            half_stroke_y /= nemavg_context->info.global_sy_factor;
#endif
    }

    incrx += half_stroke_x;
    incry += half_stroke_y;

    path->shape.bbox.transformed_p1.x = path->shape.bbox.min.x - incrx;
    path->shape.bbox.transformed_p1.y = path->shape.bbox.min.y - incry;
    path->shape.bbox.transformed_p2.x = path->shape.bbox.min.x + path->shape.bbox.w + incrx;
    path->shape.bbox.transformed_p2.y = path->shape.bbox.min.y - incry;
    path->shape.bbox.transformed_p3.x = path->shape.bbox.min.x + path->shape.bbox.w + incrx;
    path->shape.bbox.transformed_p3.y = path->shape.bbox.min.y + path->shape.bbox.h + incry;
    path->shape.bbox.transformed_p4.x = path->shape.bbox.min.x - incrx;
    path->shape.bbox.transformed_p4.y = path->shape.bbox.min.y + path->shape.bbox.h + incry;

    path->shape.bbox.transformed_min.x = path->shape.bbox.min.x - incrx;
    path->shape.bbox.transformed_min.y = path->shape.bbox.min.y - incry;
    path->shape.bbox.transformed_max.x = path->shape.bbox.max.x + incrx;
    path->shape.bbox.transformed_max.y = path->shape.bbox.max.y + incry;
    path->shape.bbox.transformed_w = path->shape.bbox.w + 2.0f*incrx;
    path->shape.bbox.transformed_h = path->shape.bbox.h + 2.0f*incry;

    // if identity matrix no transformation is needed
    if(nemavg_context->info.has_transformation == 0U){
        return NEMA_VG_ERR_NO_ERROR;
    }

    nema_mat3x3_mul_vec_(nemavg_context->path_plus_global_m, &(path->shape.bbox.transformed_p1.x), &(path->shape.bbox.transformed_p1.y));
    nema_mat3x3_mul_vec_(nemavg_context->path_plus_global_m, &(path->shape.bbox.transformed_p2.x), &(path->shape.bbox.transformed_p2.y));
    nema_mat3x3_mul_vec_(nemavg_context->path_plus_global_m, &(path->shape.bbox.transformed_p3.x), &(path->shape.bbox.transformed_p3.y));
    nema_mat3x3_mul_vec_(nemavg_context->path_plus_global_m, &(path->shape.bbox.transformed_p4.x), &(path->shape.bbox.transformed_p4.y));

#ifdef NEMAPVG
    nema_vg_float_t minx1 = nema_min2(path->shape.bbox.transformed_p1.x, path->shape.bbox.transformed_p2.x);
    nema_vg_float_t minx2 = nema_min2(path->shape.bbox.transformed_p3.x, path->shape.bbox.transformed_p4.x);
    nema_vg_float_t minx  = nema_min2(minx1, minx2);

    nema_vg_float_t miny1 = nema_min2(path->shape.bbox.transformed_p1.y, path->shape.bbox.transformed_p2.y);
    nema_vg_float_t miny2 = nema_min2(path->shape.bbox.transformed_p3.y, path->shape.bbox.transformed_p4.y);
    nema_vg_float_t miny  = nema_min2(miny1, miny2);

    nema_vg_float_t maxx1 = nema_max2(path->shape.bbox.transformed_p1.x, path->shape.bbox.transformed_p2.x);
    nema_vg_float_t maxx2 = nema_max2(path->shape.bbox.transformed_p3.x, path->shape.bbox.transformed_p4.x);
    nema_vg_float_t maxx  = nema_max2(maxx1, maxx2);

    nema_vg_float_t maxy1 = nema_max2(path->shape.bbox.transformed_p1.y, path->shape.bbox.transformed_p2.y);
    nema_vg_float_t maxy2 = nema_max2(path->shape.bbox.transformed_p3.y, path->shape.bbox.transformed_p4.y);
    nema_vg_float_t maxy  = nema_max2(maxy1, maxy2);

    path->shape.bbox.transformed_min.x = minx;
    path->shape.bbox.transformed_min.y = miny;
    path->shape.bbox.transformed_max.x = maxx;
    path->shape.bbox.transformed_max.y = maxy;

    path->shape.bbox.transformed_w = maxx - minx;
    path->shape.bbox.transformed_h = maxy - miny;
#endif

    return NEMA_VG_ERR_NO_ERROR;
}

static void
vg_set_blend(uint32_t blending_mode, int doing_blit, int has_lut)
{
    static const uint32_t fill_cmd0[2]  = {0x1a581001U, 0x050481a0U};
    static const uint32_t fill_cmd1[2]  = {0x000c0000U, 0x0a802006U};
    static const uint32_t fill_cmd2[2]  = {0x00000000U, 0x0a581606U};
    static const uint32_t fill_cmd3[2]  = {0x18001000U, 0x0a916426U};
    static const uint32_t fill_cmd4[2]  = {0x10000002U, 0x8a5b56a6U};
    static const uint32_t blit_cmd0[2]  = {0x0a18108bU, 0x00000000U};
    static const uint32_t blit_cmd1[2]  = {0x0800110bU, 0x00000000U};
    static const uint32_t blit_cmd2[2]  = {0x004c0000U, 0x01016204U};
    static const uint32_t blit_cmd33[2] = {0x184c1001U, 0x00000000U};
    static const uint32_t blit_cmd3[2]  = {0x184c1000U, 0x0a882206U};
    static const uint32_t blit_cmd4[2]  = {0x00000000U, 0x0a581606U};
    static const uint32_t blit_cmd5[2]  = {0x18001000U, 0x0a912406U};
    static const uint32_t blit_cmd6[2]  = {0x10000002U, 0x8a5b1686U};

    uint32_t src_bf =  blending_mode&0xfU;
    uint32_t dst_bf = (blending_mode>>8)&0xfU;

    uint32_t cmd[8][2];
    uint32_t cmd_idx = 0;

    uint32_t src_ckey = 0U;
    if ( (blending_mode & NEMA_BLOP_SRC_CKEY) != 0U ) {
        // we need to do src ckey
        src_ckey = 0x100000U;
    }

    if ( doing_blit == 0 ) {

        static const uint32_t bf[16] = {
            (unsigned)R1_000    << 14 | (unsigned)A1_0      << 5,
            (unsigned)R1_111    << 14 | (unsigned)A1_1      << 5,
            (unsigned)R1_T0RGB  << 14 | (unsigned)A1_T0A    << 5,
            (unsigned)R1_iT0RGB << 14 | (unsigned)A1_iT0A   << 5,
            (unsigned)R1_T0AAA  << 14 | (unsigned)A1_T0A    << 5,
            (unsigned)R1_iT0AAA << 14 | (unsigned)A1_iT0A   << 5,
            (unsigned)R1_T3AAA  << 14 | (unsigned)A1_T3A    << 5,
            (unsigned)R1_iT3AAA << 14 | (unsigned)A1_iT3A   << 5,
            (unsigned)R1_T3RGB  << 14 | (unsigned)A1_T3A    << 5,
            (unsigned)R1_iT3RGB << 14 | (unsigned)A1_iT3A   << 5,
            (unsigned)R1_C1RGB  << 14 | (unsigned)A1_C1A    << 5,
            (unsigned)R1_C1AAA  << 14 | (unsigned)A1_C1A    << 5,
            0U,
            0U,
            0U,
            0U
        };

        cmd[cmd_idx][0] = fill_cmd0[0]; cmd[cmd_idx][1] = fill_cmd0[1]; ++cmd_idx; // read dst
        cmd[cmd_idx][0] = fill_cmd1[0] | src_ckey; cmd[cmd_idx][1] = fill_cmd1[1] | bf[src_bf]; ++cmd_idx; // src*bf
        if (dst_bf != NEMA_BF_ZERO) {
            cmd[cmd_idx][0] = fill_cmd2[0]; cmd[cmd_idx][1] = fill_cmd2[1] | bf[dst_bf]; ++cmd_idx; // dst*bf
        }
        cmd[cmd_idx][0] = fill_cmd3[0]; cmd[cmd_idx][1] = fill_cmd3[1]; ++cmd_idx; // src*cov
        cmd[cmd_idx][0] = fill_cmd4[0]; cmd[cmd_idx][1] = fill_cmd4[1]; ++cmd_idx; // dst*icov
        nema_set_frag_ptr( PREFETCH_TEXEL | PRE_IMG3 | PRE_XY | PRE_T1 );
    }
    else {
        static const uint32_t bf_blit[16] = {
            (unsigned)R1_000    << 14 | (unsigned)A1_0      << 5,
            (unsigned)R1_111    << 14 | (unsigned)A1_1      << 5,
            (unsigned)R1_T1RGB  << 14 | (unsigned)A1_T1A    << 5,
            (unsigned)R1_iT1RGB << 14 | (unsigned)A1_iT1A   << 5,
            (unsigned)R1_T1AAA  << 14 | (unsigned)A1_T1A    << 5,
            (unsigned)R1_iT1AAA << 14 | (unsigned)A1_iT1A   << 5,
            (unsigned)R1_T3AAA  << 14 | (unsigned)A1_T3A    << 5,
            (unsigned)R1_iT3AAA << 14 | (unsigned)A1_iT3A   << 5,
            (unsigned)R1_T3RGB  << 14 | (unsigned)A1_T3A    << 5,
            (unsigned)R1_iT3RGB << 14 | (unsigned)A1_iT3A   << 5,
            (unsigned)R1_C1RGB  << 14 | (unsigned)A1_C1A    << 5,
            (unsigned)R1_C1AAA  << 14 | (unsigned)A1_C1A    << 5,
            0U,
            0U,
            0U,
            0U
        };

        cmd[cmd_idx][0] = blit_cmd0[0]; cmd[cmd_idx][1] = blit_cmd0[1]; ++cmd_idx; // read src or idx if lut
        if ( has_lut != 0 ) {
            cmd[cmd_idx][0] = blit_cmd1[0]; cmd[cmd_idx][1] = blit_cmd1[1]; ++cmd_idx; // read palette
        }

        if((blending_mode & NEMA_BLOP_SRC_PREMULT) != 0U) {
            cmd[cmd_idx][0] = blit_cmd2[0] | src_ckey; cmd[cmd_idx][1] = blit_cmd2[1]; ++cmd_idx; // premult
        }

        uint32_t readdst = P_READTEXC;
        if (src_bf >= NEMA_BF_DESTALPHA && src_bf <= NEMA_BF_INVDESTCOLOR) {
            // dst is used for src blending. need to fetch
            cmd[cmd_idx][0] = blit_cmd33[0] | src_ckey;cmd[cmd_idx][1] = blit_cmd33[1];++cmd_idx; // read dst
            readdst = 0U;
        }
        // if (src_bf != NEMA_BF_ONE && dst_bf != NEMA_BF_ZERO ) { // need to read dst
        cmd[cmd_idx][0] = blit_cmd3[0] | readdst | src_ckey; cmd[cmd_idx][1] = blit_cmd3[1] | bf_blit[src_bf]; ++cmd_idx; // src*bf - read dst
        // }
        if (dst_bf != NEMA_BF_ZERO) {
            cmd[cmd_idx][0] = blit_cmd4[0]; cmd[cmd_idx][1] = blit_cmd4[1] | bf_blit[dst_bf]; ++cmd_idx; // dst*bf
        }
        cmd[cmd_idx][0] = blit_cmd5[0]; cmd[cmd_idx][1] = blit_cmd5[1]; ++cmd_idx; // src*cov
        cmd[cmd_idx][0] = blit_cmd6[0]; cmd[cmd_idx][1] = blit_cmd6[1]; ++cmd_idx; // dst*icov
        nema_set_frag_ptr( PREFETCH_TEXEL | PRE_IMG3 | PRE_XY | PRE_T0 );
    }

    nema_set_rop_blend_mode(NEMA_BL_SRC);
    nema_load_frag_shader(&cmd[0][0], cmd_idx, 0);
}

static uint32_t
stencil_blit(nema_vg_path_t* path, nema_vg_paint_t* paint)
{
    //Enable tiling in case we have dissabled it for stencil blit
    if ( ( (uint32_t)nemavg_context->fb_tsc & FB_TSC_MASK) == (FB_TSC6 | FB_TSC4) ) {
        nema_enable_tiling(1);
    } else if ( (nemavg_context->fb_tsc & FB_TSC_MASK) == FB_TSC12 ) {
        nema_enable_tiling_2x2(1);
    } else {
        //MISRA
    }

    uint32_t blend = nemavg_context->blend;

    if (paint->type != NEMA_VG_PAINT_COLOR) {
        if ( (blend&0xffffU) == NEMA_BL_SRC_OVER ) {
            nema_cl_add_cmd(NEMA_ROPBLENDER_BLEND_MODE, blend);

            uint32_t src_ckey = 0U;
            if ( (blend & NEMA_BLOP_SRC_CKEY) != 0U ) {
                // we need to do src ckey
                src_ckey = 0x100000U;
            }

            uint32_t blit_stencil_cmd_premul[] = {  0x0000110bU, 0x00000000U,
                                                    0x100c1183U, 0x00092004U,
                                                    0x080c0002U, 0x8081a046U};

            uint32_t blit_stencil_cmd_nonpremul[] = {   0x0000110bU, 0x00000000U,
                                                        0x100c1183U, 0x00092000U,
                                                        0x080c0002U, 0x8081a046U};

            uint32_t blit_stencil_cmd_premul_opa[] = {  0x0000110bU, 0x00000000U,
                                                        0x000c0000U, 0x00812182U,
                                                        0x100c1183U, 0x00092004U,
                                                        0x080c0002U, 0x8081a046U};

            uint32_t blit_stencil_cmd_nonpremul_opa[] = {   0x0000110bU, 0x00000000U,
                                                            0x100c1183U, 0x00812182U,
                                                            0x080c0002U, 0x8081a046U};

            if ( paint->opacity >= 1.f ) {
                uint32_t cmd       = 2;
                uint32_t cmd_count = 2;
                uint32_t codeptr = 0x941e8000U;

                if (paint->type == NEMA_VG_PAINT_TEXTURE && paint->tex.is_lut_tex == 1U) {
                                cmd = 0;
                                cmd_count = 3;
                                codeptr = 0x941da000U;
                }
                if((blend & NEMA_BLOP_SRC_PREMULT) == 0U) {
                    blit_stencil_cmd_nonpremul[2] |= src_ckey;
                    nema_load_frag_shader(&blit_stencil_cmd_nonpremul[cmd], cmd_count, 0);
                }
                else{
                    blit_stencil_cmd_premul[2] |= src_ckey;
                    nema_load_frag_shader(&blit_stencil_cmd_premul[cmd], cmd_count, 0);
                }
                nema_set_frag_ptr(codeptr | ((PREFETCH_TEXEL | PRE_TEX[NEMA_TEX0] | PRE_XY /*| PRE_T0*/ | (PRELOAD_ADDR+1U-cmd_count)) << 16));
            }
            else {
                uint32_t cmd       = 2;
                uint32_t cmd_count = 2;
                uint32_t codeptr = 0x941e8000U;

                if (paint->type == NEMA_VG_PAINT_TEXTURE && paint->tex.is_lut_tex == 1U) {
                    // opacity + lut
                                cmd = 0;
                                cmd_count = 3;
                                codeptr = 0x941da000U;
                }

                if((blend & NEMA_BLOP_SRC_PREMULT) == 0U) {
                    blit_stencil_cmd_nonpremul_opa[2] |= src_ckey;
                    nema_load_frag_shader(&blit_stencil_cmd_nonpremul_opa[cmd], cmd_count, 0);
                }
                else{
                    ++cmd_count;
                    blit_stencil_cmd_premul_opa[4] |= src_ckey;
                    nema_load_frag_shader(&blit_stencil_cmd_premul_opa[cmd], cmd_count, 0);
                }
                nema_set_frag_ptr(codeptr | ((PREFETCH_TEXEL | PRE_TEX[NEMA_TEX0] | PRE_XY /*| PRE_T0*/ | (PRELOAD_ADDR+1U-cmd_count)) << 16));

                float opaf = paint->opacity*255.f + 0.5f;
                int opa = (int)opaf;
                uint32_t rgba = (uint32_t)opa << 24U;
                nema_set_const_color(rgba);
            }
        }
        else {
            int is_lut = 0;
            if (paint->type == NEMA_VG_PAINT_TEXTURE && paint->tex.is_lut_tex == 1U) {
                is_lut = 1;
            }
            vg_set_blend(blend, 1, is_lut);
        }
    }
    else {
        // FILL with COLOR
        if ( (blend&(0xffffU | NEMA_BLOP_SRC_CKEY)) == NEMA_BL_SRC_OVER ) {
            const int num_cmds = 3;
            uint32_t *cmd_array = nema_cl_get_space(num_cmds);
            if (cmd_array == NULL) {
                return NEMA_VG_ERR_CL_FULL;
            }
#ifdef NEMA_P_IMEM_16
            #ifdef FILLSTENCIL_OPT
            const uint32_t cmd_fill_stencil[] = {0x0a980002U, 0x8081a046U};
            #else
            const uint32_t cmd_fill_stencil[] = {0x080c0002U, 0x8081a046U};
            #endif
            nema_load_frag_shader(cmd_fill_stencil, 1, SHADER_ADDR_FILLSTENCIL);
#endif //NEMA_P_IMEM_16
            int cmd_idx = 0;
            // 1
            cmd_array[cmd_idx] = NEMA_ROPBLENDER_BLEND_MODE; cmd_idx++;
            cmd_array[cmd_idx] = blend; cmd_idx++;
            // 2
            cmd_array[cmd_idx] = NEMA_CODEPTR; cmd_idx++;
#ifdef NEMA_P_IMEM_16
            // SHADER_ADDR_FILLSTENCIL is 0 and MISRA is complaining
            const uint32_t bg_shader_flags = ((PREFETCH_TEXEL | PRE_TEX[NEMA_TEX0] | PRE_XY /*| PRE_T0*/ | (PRELOAD_ADDR+1U-1U)) << 16U);
            cmd_array[cmd_idx] = (bg_shader_flags| PREFETCH_TEXEL | PRE_T2 | PRE_XY | PRE_IMG3); cmd_idx++;
#else
            const uint32_t bg_shader_flags = ((PREFETCH_TEXEL | PRE_TEX[NEMA_TEX0] | PRE_XY /*| PRE_T0*/ | (PRELOAD_ADDR+1U-1U)) << 16);
            cmd_array[cmd_idx] = (bg_shader_flags| PREFETCH_TEXEL | PRE_T2 | PRE_XY | PRE_IMG3 | SHADER_ADDR_FILLSTENCIL); cmd_idx++;
#endif //NEMA_P_IMEM_16
            // 3
            cmd_array[cmd_idx] = NEMA_DRAW_COLOR; cmd_idx++;
            cmd_array[cmd_idx] = nemavg_context->info.paint_color;
        }
        else {
            vg_set_blend(blend, 0, 0);
            nema_set_raster_color(nemavg_context->info.paint_color);
        }
    }


    if (paint->type != NEMA_VG_PAINT_GRAD_CONICAL){
        nema_raster_quad_f_ (path->shape.bbox.transformed_p1,
                             path->shape.bbox.transformed_p2,
                             path->shape.bbox.transformed_p3,
                             path->shape.bbox.transformed_p4);
    }
    else{

        int supports_clip2 = nema_supports_clip2();

        nema_vg_path_bbox_t *bbox = &path->shape.bbox;

        nema_vg_vertex_t_ c;

        c.x = nema_clamp(paint->grad.cx, 0.f, (float)stencil->width);
        c.y = nema_clamp(paint->grad.cy, 0.f, (float)stencil->height);

        //quadrant 0 cw
        float w = (float)stencil->width  - c.x + 0.5f;
        float h = (float)stencil->height - c.y + 0.5f;

        if (w > 0.f && h > 0.f) {
            nema_set_matrix(paint->grad.m);
            set_clip2_or_temp(supports_clip2, (int32_t)c.x, (int32_t)c.y, (uint32_t)w, (uint32_t)h);
            nema_raster_quad_f_ (bbox->transformed_p1,
                                 bbox->transformed_p2,
                                 bbox->transformed_p3,
                                 bbox->transformed_p4);
        }

        //quadrant 1 cw
        w = c.x + 0.5f;
        h = (float)stencil->height - c.y + 0.5f;
        if (w > 0.f && h > 0.f) {
            nema_set_matrix(paint->grad.m1);
            set_clip2_or_temp(supports_clip2, 0, (int32_t)c.y, (uint32_t)w, (uint32_t)h);
            nema_raster_quad_f_ (bbox->transformed_p1,
                                 bbox->transformed_p2,
                                 bbox->transformed_p3,
                                 bbox->transformed_p4);
        }

        //quadrant 2 cw
        w = c.x + 0.5f;
        h = c.y + 0.5f;
        if (w > 0.f && h > 0.f) {
            nema_set_matrix(paint->grad.m2);
            set_clip2_or_temp(supports_clip2, 0, 0, (uint32_t)w, (uint32_t)h);
            nema_raster_quad_f_ (bbox->transformed_p1,
                                 bbox->transformed_p2,
                                 bbox->transformed_p3,
                                 bbox->transformed_p4);
        }

        //quadrant 3 cw
        w = (float)stencil->width - c.x + 0.5f;
        h = c.y + 0.5f;
        if (w > 0.f && h > 0.f) {
            nema_set_matrix(paint->grad.m3);
            set_clip2_or_temp(supports_clip2, (int32_t)c.x, 0, (uint32_t)w, (uint32_t)h);
            nema_raster_quad_f_ (bbox->transformed_p1,
                                 bbox->transformed_p2,
                                 bbox->transformed_p3,
                                 bbox->transformed_p4);
        }
        // reset clip2
        reset_clip2_or_temp(supports_clip2);
    }

    return NEMA_VG_ERR_NO_ERROR;
}

static uint32_t
stencil_paint(nema_vg_path_t* path_in, nema_vg_paint_t* paint)
{
    // TEX1 paint/TEX3 stencil for each paint type
    switch(paint->type)
    {
        case NEMA_VG_PAINT_GRAD_LINEAR:
        case NEMA_VG_PAINT_GRAD_RADIAL:
        {
            if ( FLAG_IS_SET(paint->flags, NEMA_VG_PAINT_TRANS_LOCKED_TO_PATH) ) {
                uint8_t has_transf_ =
                       (uint8_t)(path_in->flags & NEMA_VG_PATH_HAS_TRANSFORMATION )
                    |  (uint8_t)(nemavg_context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION );

                if ( has_transf_ == 0U ) {
                    nema_set_matrix(paint->grad.m);
                }
                else {
                    nema_matrix3x3_t p_plus_g_m_inv;
                    nema_mat3x3_copy(p_plus_g_m_inv, nemavg_context->path_plus_global_m);

                    if ( nema_mat3x3_invert(p_plus_g_m_inv) != 0 ) {
                        return NEMA_VG_ERR_NON_INVERTIBLE_MATRIX;
                    }

                    nema_matrix3x3_t m;
                    nema_mat3x3_copy(m, paint->grad.m);
                    nema_mat3x3_mul(m, p_plus_g_m_inv);
                    nema_set_matrix(m);
                }
            }
            else {
                if ( (  nemavg_context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION ) == 0U ) {
                    nema_set_matrix(paint->grad.m);
                }
                else {
                    nema_matrix3x3_t m;
                    nema_mat3x3_copy(m, paint->grad.m);
                    nema_mat3x3_mul(m, nemavg_context->global_m_inv);
                    nema_set_matrix(m);
                }
            }
            break;
        }
        case NEMA_VG_PAINT_TEXTURE:
        {
            if ( FLAG_IS_SET(paint->flags, NEMA_VG_PAINT_TRANS_LOCKED_TO_PATH) ) {
                uint8_t has_transf_ =
                       (uint8_t)(path_in->flags & NEMA_VG_PATH_HAS_TRANSFORMATION )
                    |  (uint8_t)(nemavg_context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION );

                if ( has_transf_ == 0U ) {
                    nema_set_matrix(paint->tex.m);
                }
                else {
                    nema_matrix3x3_t p_plus_g_m_inv;
                    nema_mat3x3_copy(p_plus_g_m_inv, nemavg_context->path_plus_global_m);

                    if ( nema_mat3x3_invert(p_plus_g_m_inv) != 0 ) {
                        return NEMA_VG_ERR_NON_INVERTIBLE_MATRIX;
                    }

                    nema_matrix3x3_t m;
                    nema_mat3x3_copy(m, paint->tex.m);
                    nema_mat3x3_mul(m, p_plus_g_m_inv);
                    nema_set_matrix(m);
                }
            }
            else {
                if ( (  nemavg_context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION ) == 0U ) {
                    nema_set_matrix(paint->tex.m);
                }
                else {
                    nema_matrix3x3_t m;
                    nema_mat3x3_copy(m, paint->tex.m);
                    nema_mat3x3_mul(m, nemavg_context->global_m_inv);
                    nema_set_matrix(m);
                }
            }

            break;
        }
        default:
        {
            break;
        }
    }

    NEMA_VG_IF_ERROR(stencil_blit(path_in, paint));
    return NEMA_VG_ERR_NO_ERROR;
}

static uint32_t
stencil_mask(nema_vg_path_t* path)
{
    NEMA_VG_ASSERT(nemavg_context , NEMA_VG_ERR_BAD_HANDLE);
    uint32_t error = NEMA_VG_ERR_NO_ERROR;

    if(nemavg_context->masking_info.mask_obj == NULL ){
        nema_vg_set_error(NEMA_VG_ERR_INVALID_MASKING_OBJ);
        return NEMA_VG_ERR_INVALID_MASKING_OBJ;
    }

    nema_set_tex_color(0);
    // use TEX2 to load mask obj
    nema_bind_tex(NEMA_TEX2, nemavg_context->masking_info.mask_obj->bo.base_phys,
                             nemavg_context->masking_info.mask_obj->w,
                             nemavg_context->masking_info.mask_obj->h,
                             nemavg_context->masking_info.mask_obj->format,
                             nemavg_context->masking_info.mask_obj->stride, NEMA_TEX_BORDER);

#ifdef NEMAPVG
    // if NonZero or EvenOdd
    // need to convert  stencil from NZS/EOS to A8
    int change_tex3_to_A8 = 0;

    if (  (nemavg_context->fill_rule ==  NEMA_VG_FILL_NON_ZERO)
       || (nemavg_context->fill_rule ==  NEMA_VG_FILL_EVEN_ODD) ) {

        change_tex3_to_A8 = 1;

        //bind stencil to TEX1 as A8
        nema_bind_tex(NEMA_TEX1,
                    stencil->bo.base_phys,
                    (uint32_t)stencil->width,
                    (uint32_t)stencil->height,
                    NEMA_A8,
                    stencil->width,
                    NEMA_FILTER_PS);

        nema_set_blend(NEMA_BL_DST_IN, NEMA_TEX1, NEMA_TEX2, NEMA_TEX3);
    }
    else {
        nema_set_blend(NEMA_BL_DST_IN, NEMA_TEX3, NEMA_TEX2, NEMA_NOTEX);
    }
#else
    nema_cl_add_cmd(NEMA_TEX3_FSTRIDE, ((unsigned)NEMA_L8 << 24) | ((unsigned)stencil->width & 0xffffU));
    nema_set_blend(NEMA_BL_DST_IN, NEMA_TEX3, NEMA_TEX2, NEMA_NOTEX);
#endif

    nema_set_matrix_translate(nemavg_context->masking_info.trans_x, nemavg_context->masking_info.trans_y);

    nema_raster_quad_f_ (path->shape.bbox.transformed_p1,
                         path->shape.bbox.transformed_p2,
                         path->shape.bbox.transformed_p3,
                         path->shape.bbox.transformed_p4);

#ifdef NEMAPVG
    if ( change_tex3_to_A8 != 0 ) {
        // Leave TEX3 ready to be read by next stages
        nema_cl_add_cmd(NEMA_TEX3_FSTRIDE, ((unsigned)NEMA_A8 << 24) | ((unsigned)stencil->width & 0xffffU));
    }
#else
        nema_cl_add_cmd(NEMA_TEX3_FSTRIDE, ((unsigned)NEMA_A8 << 24) | ((unsigned)stencil->width & 0xffffU));
#endif

    return error;
}

static uint32_t
draw_directly_to_fb(nema_vg_path_t* path, nema_vg_paint_t* paint)
{
    (void) paint;
    // currently doing only stroking
    uint32_t error = NEMA_VG_ERR_NO_ERROR;

    // bind default render target
    if ( NEMA_BL_SRC_OVER == (nemavg_context->blend&(0xffffU | NEMA_BLOP_SRC_CKEY) ) ) {
        const int num_cmds = 4;
        uint32_t *cmd_array = nema_cl_get_space(num_cmds);
        if (cmd_array == NULL) {
            return NEMA_VG_ERR_CL_FULL;
        }
#ifdef NEMA_P_IMEM_16
        static const uint32_t cmd_fillsrcover[] = {0x004e0002U, 0x800b0286U};
        nema_load_frag_shader(&cmd_fillsrcover[0], 1, SHADER_ADDR_FILLSRCOVER);
#endif //NEMA_P_IMEM_16
        int cmd_idx = 0;
        // 1
        cmd_array[cmd_idx] = NEMA_ROPBLENDER_BLEND_MODE; cmd_idx++;
        cmd_array[cmd_idx] = 1U; cmd_idx++;
        // 2
        cmd_array[cmd_idx] = NEMA_CODEPTR; cmd_idx++;
#ifdef NEMA_P_IMEM_16
        // SHADER_ADD_FILLSRCOVER is 0 and MISRA is complaining
        const uint32_t bg_shader_flags = ((PREFETCH_TEXEL | PRE_TEX[NEMA_TEX0] | PRE_XY /*| PRE_T0*/ | (PRELOAD_ADDR+1U-1U)) << 16U);
        cmd_array[cmd_idx] = bg_shader_flags | 0xb400U; cmd_idx++;
#else
        const uint32_t bg_shader_flags = ((PREFETCH_TEXEL | PRE_TEX[NEMA_TEX0] | PRE_XY /*| PRE_T0*/ | (PRELOAD_ADDR+1U-1U)) << 16);
        cmd_array[cmd_idx] = bg_shader_flags | 0xb400U | SHADER_ADDR_FILLSRCOVER; cmd_idx++;
#endif //NEMA_P_IMEM_16
        // 3
        cmd_array[cmd_idx] = NEMA_DRAW_COLOR; cmd_idx++;
        cmd_array[cmd_idx] = nemavg_context->info.paint_color; cmd_idx++;
        // X - is this needed?
        cmd_array[cmd_idx] = NEMA_MATMULT; cmd_idx++;
        cmd_array[cmd_idx] = 0x90000000U;
    }
    else {
        nema_set_blend_fill(nemavg_context->blend | NEMA_BLOP_NO_USE_ROPBL);
        nema_set_raster_color( nemavg_context->info.paint_color );
    }

    set_vertex_matrix_(nemavg_context->path_plus_global_m);

#ifdef NEMAPVG
    if(nemavg_context->info.do_fat_stroke == 0)
    {
        NEMA_VG_IF_ERROR(stroke_thin_path(path));
    }
    else
    {
        NEMA_VG_IF_ERROR(stroke_path(path));
    }
#else
    NEMA_VG_IF_ERROR(stroke_path(path));
#endif
    revert_vertex_matrix_();

    return error;
}

static uint32_t
draw_with_stencil(nema_vg_path_t* path, nema_vg_paint_t* paint)
{
    NEMA_VG_ASSERT(stencil->bo.base_virt, NEMA_VG_ERR_BAD_BUFFER)

#ifndef NEMAPVG
    //bind TEX2(lut)
    GET_LUT;
    if ( (nemavg_context->fill_rule) ==  NEMA_VG_FILL_NON_ZERO ) {
        nema_bind_tex(NEMA_TEX2,
                      lut->base_phys,
                      LUT_SIZE,
                      1,
                      NEMA_L8,
                      0,
                      NEMA_TEX_REPEAT);
    }
#endif

    //bind TEX3(stencil)
    nema_bind_tex(NEMA_TEX3,
                  stencil->bo.base_phys,
                  (uint32_t)stencil->width,
                  (uint32_t)stencil->height,
                  NEMA_A8,
                  stencil->width,
                  NEMA_FILTER_PS);

#ifdef NEMA_VG_INVALIDATE_CACHE
NEMA_VG_DISABLE_CACHE;
#endif

    // clear and bind stencil buffer (TEX3), as render target
    NEMA_VG_IF_ERROR(stencil_clear_dirty_area());

    // Now TEX3 is bound as L8

    // program HW Vertex MMUL if available
    set_vertex_matrix_(nemavg_context->path_plus_global_m);

    // transformed bbox is used in fan mask, stencil_blit and nzstencil
    NEMA_VG_IF_ERROR(calc_transformed_bbox(path));
    // draw path on stencil (TEX3)
    // bind TEX3 to be read by next stages
    NEMA_VG_IF_ERROR(stencil_draw(path, paint));

    // Now TEX3 is bound as A8 (or EOS/NZS)

    if ( (nemavg_context->masking_info.masking) ==  1U ) {
        NEMA_VG_IF_ERROR(stencil_mask(path));
    }

#ifdef NEMA_VG_INVALIDATE_CACHE
NEMA_VG_ENABLE_INVALIDATE_CACHE;
#endif

    if ( paint->type != NEMA_VG_PAINT_COLOR ) {
        //bind TEX1(paint)
        bind_tex1_according_paint(paint);
    }

    NEMA_VG_IF_ERROR(stencil_paint(path, paint));

    // Bypass HW Vertex MMUL if available
    revert_vertex_matrix_();

    return NEMA_VG_ERR_NO_ERROR;
}

uint32_t
nema_vg_draw_path(NEMA_VG_PATH_HANDLE path, NEMA_VG_PAINT_HANDLE paint)
{
    init_clip_from_context();

    if(nema_cl_get_bound() == NULL)
    {
        nema_vg_set_error(NEMA_VG_ERR_NO_BOUND_CL);
        return NEMA_VG_ERR_NO_BOUND_CL;
    }

    GET_PATH;
    NEMA_VG_ASSERT(path, NEMA_VG_ERR_BAD_HANDLE);
    GET_PAINT;
    NEMA_VG_ASSERT(paint, NEMA_VG_ERR_BAD_HANDLE);
    NEMA_VG_ASSERT(nemavg_context, NEMA_VG_ERR_BAD_HANDLE);

#ifndef NEMAPVG
    GET_LUT;
    NEMA_VG_ASSERT(lut, NEMA_VG_ERR_BAD_BUFFER);
#endif

    //Check for src_ckey
    if ( (nemavg_context->blend & NEMA_BLOP_SRC_CKEY) != 0U && _paint->type == NEMA_VG_PAINT_COLOR) {
        if (nema_context.src_ckey == _paint->paint_color){
            return NEMA_VG_ERR_NO_ERROR;
        }
    }

    if((nemavg_context->stroke.start_cap_style != NEMA_VG_CAP_BUTT) || nemavg_context->stroke.end_cap_style != NEMA_VG_CAP_BUTT)
    {
        nemavg_context->info.has_cap = 1;
    }
    else{
        nemavg_context->info.has_cap = 0;
    }

    // Do some initial error checking
     nema_vbuf_t_ *vb = &_path->shape;

    if (vb->data_size == 0U){
        nema_vg_set_error(NEMA_VG_ERR_INVALID_VERTEX_DATA);
        return NEMA_VG_ERR_INVALID_VERTEX_DATA;
    }

    if(vb->bbox.w  < 0.0f || vb->bbox.h < 0.0f)
    {   nema_vg_set_error(NEMA_VG_ERR_DRAW_OUT_OF_BOUNDS);
        return NEMA_VG_ERR_DRAW_OUT_OF_BOUNDS;
    }

    nemavg_context->info.stroke_w = nemavg_context->stroke.width;
    if ( nemavg_context->fill_rule == NEMA_VG_STROKE && nemavg_context->info.stroke_w <= 0.f ) {
        return NEMA_VG_ERR_INVALID_STROKE_WIDTH;
    }

    set_bezier_quality(nemavg_context->quality);
    nemavg_context->info.has_transformation =    (uint8_t)(_path->flags   & NEMA_VG_PATH_HAS_TRANSFORMATION   )
                         |  (uint8_t)(nemavg_context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION );

    uint8_t pre_clip_transformation = nemavg_context->info.has_transformation;
    if( nemavg_context->drawing_clipped_path != 0U ) {
        nemavg_context->info.has_transformation = 0U;
    }

    nema_mat3x3_load_identity(nemavg_context->path_plus_global_m);
    if (nemavg_context->info.has_transformation != 0U || pre_clip_transformation != 0U ) {

        if ( (nemavg_context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION   ) != 0U ) {
            nema_mat3x3_copy(nemavg_context->path_plus_global_m, nemavg_context->global_m);
            if ( (_path->flags   & NEMA_VG_PATH_HAS_TRANSFORMATION     ) != 0U ) {
                nema_mat3x3_mul(nemavg_context->path_plus_global_m, _path->m);
            }
        }
        else {
            if ( (_path->flags   & NEMA_VG_PATH_HAS_TRANSFORMATION     ) != 0U ) {
                nema_mat3x3_copy(nemavg_context->path_plus_global_m, _path->m);
            }
            // else {
                // nema_mat3x3_load_identity(nemavg_context->path_plus_global_m);
            // }
        }
    }

    nemavg_context->info.stroke_w_x = nemavg_context->info.stroke_w;
    nemavg_context->info.stroke_w_y = nemavg_context->info.stroke_w;

    nemavg_context->info.paint_color = _paint->paint_color;

    if ( _paint->opacity < 1.f ) {
        uint32_t au = (nemavg_context->info.paint_color >> 24U);
        int a = (int)au;
        float af = (float)a;
        af *= _paint->opacity;
        af += 0.5f;
        int a_new = (int)(af);
        a_new = a_new * 0x1000000; // <<24

        nemavg_context->info.paint_color &= 0x00ffffffU;
        nemavg_context->info.paint_color |= (uint32_t)a_new;
    }

    if((nemavg_context->blend & NEMA_BLOP_SRC_PREMULT) != 0U) {
        nemavg_context->info.paint_color = premultiply_rgba(nemavg_context->info.paint_color);
    }

//object space
    float screen_stroke_w = nemavg_context->info.stroke_w;
    float screen_stroke_w_x = nemavg_context->info.stroke_w_x;
    float screen_stroke_w_y = nemavg_context->info.stroke_w_y;

    if ( nemavg_context->fill_rule == NEMA_VG_STROKE ) {
        if (nemavg_context->info.has_transformation != 0U || pre_clip_transformation != 0U )  {
            float sx = 1.f;
            float sy = 1.f;
            if ( (nemavg_context->flags & NEMA_VG_CONTEXT_STROKE_WIDTH_SCREEN_SPACE) == 0U){
                // bring stroke width from object space to screen space

#ifndef NEMAPVG
                if(nemavg_context->info.stroke_w <= 1.f || nemavg_context->info.do_not_transform == 1)
                {
#endif
                    get_matrix_scale_factors(nemavg_context->path_plus_global_m, &sx, &sy);
#ifndef NEMAPVG
                }
#endif
                sx = nema_absf(sx);
                sy = nema_absf(sy);
                screen_stroke_w_x = screen_stroke_w * sx;
                screen_stroke_w_y = screen_stroke_w * sy;
                screen_stroke_w = nema_max2(screen_stroke_w_x, screen_stroke_w_y);
#ifdef HW_VERTEX_MMUL
            } else if ( (nemavg_context->flags & NEMA_VG_CONTEXT_STROKE_WIDTH_SCREEN_SPACE) != 0U) {
                // if stroke width was given by the user in screen space,
                // we need to bring it to object space
                if ( nemavg_context->info.has_transformation != 0U ) {
                    get_matrix_scale_factors(nemavg_context->path_plus_global_m, &sx, &sy);
                    sx = 1.0f / nema_absf(sx);
                    sy = 1.0f / nema_absf(sy);
                    nemavg_context->info.stroke_w_x = screen_stroke_w * sx;
                    nemavg_context->info.stroke_w_y = screen_stroke_w * sy;
                    nemavg_context->info.stroke_w   = nema_max2(screen_stroke_w_x, screen_stroke_w_y);
                }
            } else {
                // MISRA
#endif
            }
        }

        if ( nema_floats_equal(screen_stroke_w_x, screen_stroke_w_y) && screen_stroke_w_x < 1.f ) {
            nemavg_context->info.paint_color = modulate_rgba(nemavg_context->info.paint_color, screen_stroke_w_x);
            screen_stroke_w_x = 1.f;
            screen_stroke_w_y = 1.f;
            screen_stroke_w   = 1.f;
        }
    }

#ifndef HW_VERTEX_MMUL
    nemavg_context->info.stroke_w    = screen_stroke_w;
    nemavg_context->info.stroke_w_x  = screen_stroke_w_x;
    nemavg_context->info.stroke_w_y  = screen_stroke_w_y;
#else
    if ( nemavg_context->info.has_transformation == 0U && pre_clip_transformation != 0U ) {
        nemavg_context->info.stroke_w    = screen_stroke_w;
        nemavg_context->info.stroke_w_x  = screen_stroke_w_x;
        nemavg_context->info.stroke_w_y  = screen_stroke_w_y;
    }
#endif
    // here nemavg_context->info.stroke_w is in:
    // NemaP: screen space
    // NemaPVG: object space

    if ( (nemavg_context->fill_rule == NEMA_VG_STROKE)
          && (screen_stroke_w_x > 1.f || screen_stroke_w_y > 1.f) ) {
        nemavg_context->info.do_fat_stroke = 1;
#ifndef NEMAPVG
        if( nemavg_context->stroke.width <= 1.f)
        {
            nemavg_context->info.stroke_w_x = nemavg_context->stroke.width;
            nemavg_context->info.stroke_w_y = nemavg_context->stroke.width;
            nemavg_context->info.stroke_w   = nemavg_context->stroke.width;
        }
#endif
    }
    else {
        nemavg_context->info.do_fat_stroke = 0;

        nemavg_context->info.stroke_w_x = 1.f;
        nemavg_context->info.stroke_w_y = 1.f;
        nemavg_context->info.stroke_w   = 1.f;
    }

    uint32_t blend = nemavg_context->blend;

    nemavg_context->info.do_draw_on_fb = 0;

    uint8_t should_always_do_stencil = 0U;
    if ( (nemavg_context->quality & NEMA_VG_QUALITY_MAXIMUM) == NEMA_VG_QUALITY_MAXIMUM ) {
        should_always_do_stencil = 1U;
    }
    if (nemavg_context->fill_rule == NEMA_VG_STROKE) {
        should_always_do_stencil = 0U;
    }

    if (should_always_do_stencil == 0U) {

        if ( (nemavg_context->fill_rule == NEMA_VG_STROKE && _paint->type == NEMA_VG_PAINT_COLOR) &&
             (nemavg_context->info.do_fat_stroke == 0 || blend == NEMA_BL_SRC || ( (blend&0xffffU) == NEMA_BL_SRC_OVER && (nemavg_context->info.paint_color & 0xff000000U) == 0xff000000U) ) &&
             (nemavg_context->masking_info.masking != 1U) ) {

            nemavg_context->info.do_draw_on_fb = 1;
        }
    }

    NEMA_VG_IF_ERROR(stencil_set_dirty_area(path));
    if ( 1 == cull_geometry() ) {
        // path is outside stencil limits
        return NEMA_VG_ERR_NO_ERROR;
    }

    // Check if path bbox's range is supported
    // return NEMA_VG_ERR_COORDS_OUT_OF_RANGE if not
    uint32_t out_of_range = 0U;

    if ( nemavg_context->drawing_clipped_path == 0U ) {
        out_of_range = check_range();
    }

    if( out_of_range == NEMA_VG_ERR_COORDS_OUT_OF_RANGE)
    {
       if((nemavg_context->flags & NEMA_VG_CONTEXT_ENABLE_HANDLE_LARGE_COORDS) != 0U )
       {
            NEMA_VG_IF_ERROR(draw_clipped_path(path, paint));
            return NEMA_VG_ERR_NO_ERROR;
       }
       else
       {
            nema_vg_set_error(NEMA_VG_ERR_COORDS_OUT_OF_RANGE);
            return NEMA_VG_ERR_COORDS_OUT_OF_RANGE;
       }
    }

    if (nemavg_context->info.do_draw_on_fb == 1) {
        NEMA_VG_IF_ERROR(draw_directly_to_fb(_path, _paint));
    } else {
        if ( nema_context.en_tscFB == 1U ) {
            //Dissable tiling for stencil blit
            if ( nema_context.surface_tile == RAST_TILE ) {
                nema_enable_tiling(0);
                nemavg_context->fb_tsc = (uint8_t) (FB_TSC6 | FB_TSC4);
            } else if ( nema_context.surface_tile == RAST_TILE_2X2 ) {
                nema_enable_tiling_2x2(0);
                nemavg_context->fb_tsc = (uint8_t) FB_TSC12;
            } else {
                //MISRA
            }
        }

        NEMA_VG_IF_ERROR(draw_with_stencil(_path, _paint));
    }

    return NEMA_VG_ERR_NO_ERROR;
}

#ifndef NEMA_P_IMEM_16
static void
init_shaders(void) {
    static const uint32_t cmd_pix3[] = {0x080c0182U, 0x8a8761c6U};
    nema_load_frag_shader(&cmd_pix3[0], 1, SHADER_ADDR_PIXOUT3);
    static const uint32_t cmd_fillsrcover[] = {0x004e0002U, 0x800b0286U};
    nema_load_frag_shader(&cmd_fillsrcover[0], 1, SHADER_ADDR_FILLSRCOVER);

    #ifdef FILLSTENCIL_OPT
    const uint32_t cmd_fill_stencil[] = {0x0a980002U, 0x8081a046U};
    #else
    const uint32_t cmd_fill_stencil[] = {0x080c0002U, 0x8081a046U};
    #endif
    nema_load_frag_shader(cmd_fill_stencil, 1, SHADER_ADDR_FILLSTENCIL);
}
#endif

static void
load_shaders(void)
{
#ifdef NEMAPVG
    bezier_init_shaders();
    bypass_vertex_matrix();
#endif

#ifndef NEMA_P_IMEM_16
    init_shaders();
#endif
}

void
nema_vg_thread_init(void)
{
    context_reset();
    CHECK_STENCIL_DIMENSIONS();
    tsvg_init();
    uint32_t ui_width  = (uint32_t) stencil->width;
    uint32_t ui_height = (uint32_t) stencil->height;
    set_tsvg_text_max_size(ui_width, ui_height);
    GET_CONTEXT;
}

static void
init(int width, int height, nema_buffer_t bo)
{
    load_shaders();
    //allocate buffers
    stencil_buffer_set(width, height, bo);
    GET_STENCIL;
#ifndef NEMAPVG
    lut_buffer_create();
#endif
    //set context - thread specific
    nema_vg_thread_init();

}

void
nema_vg_init_stencil_prealloc(int width, int height, nema_buffer_t stencil_bo)
{
    init(width, height, stencil_bo);
}

void
nema_vg_init_stencil_pool(int width, int height, int pool)
{
    nema_buffer_t bo = stencil_buffer_create(width, height, pool);
    nema_vg_init_stencil_prealloc(width, height, bo);
    stencil_buffer_set_prealloc();
}

void
nema_vg_init(int width, int height)
{
    int w = ((width+3)/4)*4;
    int h = ((height+3)/4)*4;
    nema_vg_init_stencil_pool(w, h, NEMA_MEM_POOL_FB);
}

void
nema_vg_reinit(void)
{
    load_shaders();
    GET_CONTEXT;
    GET_STENCIL;
}

void
nema_vg_deinit(void)
{
    if((stencil->flags & NEMA_VG_STENCIL_ALLOC) != 0U)
    {
        stencil_buffer_destroy();
    }
#ifndef NEMAPVG
    lut_buffer_destroy();
#endif
    tsvg_deinit();
    context_reset();
    nema_vg_set_error(NEMA_VG_ERR_NO_INIT);
}

void nema_vg_get_coord_limits(float *min_coord, float *max_coord) {
    *min_coord = (float) (MIN_COORD);
    *max_coord = (float) (MAX_COORD);
}
