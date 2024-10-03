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
#include "nema_vg.h"
#include "nema_vg_p.h"
#include "nema_matrix3x3.h"
#include "nema_sys_defs.h"

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

static TLS_VAR nema_vg_culled_info_t_ culled_info = {0};

#define MARGIN (0.f)
#define BEZ_MARGIN 50.f

#define OUT_OF_RANGE(p)            \
        ( (p).x > culled_info._max_coord || (p).y > culled_info._max_coord || (p).x < culled_info._min_coord || (p).y < culled_info._min_coord )

#define ADD_SEG(seg) do { \
        if( culled_info._dry_run == 0 )  { \
            PRINT_F("NEW SEG: %d - line %d\n", seg, __LINE__); \
            if (culled_info.idx_new_seg < culled_info.new_seg_size) { \
                (culled_info.new_segs[culled_info.idx_new_seg] = (seg)); culled_info.idx_new_seg++;} \
            else{ \
                culled_info.error = NEMA_VG_ERR_BAD_BUFFER;  \
                culled_info.idx_new_seg++; \
                culled_info._dry_run = 1;} \
        } \
        else { \
            culled_info.idx_new_seg++; \
        } \
    } while(false)

#define ADD_DATA(point) do { \
        if ( culled_info._dry_run == 0 ) { \
            PRINT_F("NEW DATA: %f, %f - line %d\n", (point).x, (point).y, __LINE__); \
            if (culled_info.idx_new_data + 1U < culled_info.new_data_size) { \
                culled_info.new_data[culled_info.idx_new_data]= (point).x; culled_info.idx_new_data++; \
                culled_info.new_data[culled_info.idx_new_data]= (point).y; culled_info.idx_new_data++;} \
            else{ \
                culled_info.error = NEMA_VG_ERR_BAD_BUFFER;\
                culled_info.idx_new_data += 2U; \
                 culled_info._dry_run = 1;}\
        } \
        else { \
            culled_info.idx_new_data += 2U; \
        } \
    } while(false)

static inline void
clamp_vertex(nema_vg_vertex_t_ *p_dst, nema_vg_vertex_t_ *p_src)
{
    float x = nema_clamp(p_src->x, culled_info._min_coord, culled_info._max_coord);
    float y = nema_clamp(p_src->y, culled_info._min_coord, culled_info._max_coord);
    p_dst->x = x;
    p_dst->y = y;
}

static void
clear_culled_info_struct(void){
    culled_info.new_seg_size     = 0 ;
    culled_info.new_segs         = NULL ;
    culled_info.new_data_size    = 0;
    culled_info.new_data         = NULL ;
    culled_info.idx_new_data     = 0;
    culled_info.idx_new_seg      = 0;
    culled_info.new_start_data.x = 0.f;
    culled_info.new_start_data.y = 0.f;
    culled_info.save_start_data  = 0U;
    culled_info.prev_cmd_is_move = 0U;
}

static void tess_stack_push(float x0, float y0, float mx, float my, float x1, float y1) {
    if ( nemavg_context->info.tess_stack_idx  < STACK_DEPTH) {
        nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][0] = x0;
        nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][1] = y0;
        nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][2] = mx;
        nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][3] = my;
        nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][4] = x1;
        nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][5] = y1;
        ++nemavg_context->info.tess_stack_idx;
    }
}

static void tess_stack_pop(float *x0, float *y0, float *mx, float *my, float *x1, float *y1) {
    if ( nemavg_context->info.tess_stack_idx != 0) {
        --nemavg_context->info.tess_stack_idx;
        *x0 = nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][0] ;
        *y0 = nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][1] ;
        *mx = nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][2] ;
        *my = nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][3] ;
        *x1 = nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][4] ;
        *y1 = nemavg_context->info.tess_stack[nemavg_context->info.tess_stack_idx][5] ;
    }
}

inline static uint8_t
cubic_outside_of_fb(
    nema_vg_vertex_t_ p0 ,
    nema_vg_vertex_t_ p1,
    nema_vg_vertex_t_ p2,
    nema_vg_vertex_t_ p3
) {
    float min = culled_info._min_coord + BEZ_MARGIN;
    float max = culled_info._max_coord - BEZ_MARGIN;

    if ( p0.x < min &&
         p1.x < min &&
         p2.x < min &&
         p3.x < min ){
         return 1U;
    }

    if ( p0.x > max &&
         p1.x > max &&
         p2.x > max &&
         p3.x > max ){
         return 1U;
    }

    if ( p0.y < min &&
         p1.y < min &&
         p2.y < min &&
         p3.y < min ){
         return 1U;
    }

    if ( p0.y > max &&
         p1.y > max &&
         p2.y > max &&
         p3.y > max ){
         return 1U;
    }

    return 0U;
}

inline static uint8_t
quad_outside_of_fb(
    nema_vg_vertex_t_ p0,
    nema_vg_vertex_t_ p1,
    nema_vg_vertex_t_ p2
) {
    float min = culled_info._min_coord + BEZ_MARGIN;
    float max = culled_info._max_coord - BEZ_MARGIN;

    if ( p0.x < min &&
         p1.x < min &&
         p2.x < min ){
         return 1U;
    }

    if ( p0.x > max &&
         p1.x > max &&
         p2.x > max ){
         return 1U;
    }

    if ( p0.y < min &&
         p1.y < min &&
         p2.y < min ){
         return 1U;
    }

    if ( p0.y > max &&
         p1.y > max &&
         p2.y > max ){
         return 1U;
    }

    return 0U;
}

// Returns 1 if the lines intersect, otherwise 0. In addition, if the lines
// intersect the intersection point may be stored in the floats i_x and i_y.
static int get_line_intersection(float p0_x, float p0_y, float p1_x, float p1_y,
    float p2_x, float p2_y, float p3_x, float p3_y, nema_vg_vertex_t_ *p_xy, int idx)
{
    float s1_x, s1_y, s2_x, s2_y;
    s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
    s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

    // ODOT: FIX DIVIDE BY ZERO
    float s, t;
    s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
    t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

    if ((s >= 0.f && s <= 1.f && t >= 0.f && t <= 1.f) && idx < 2)
    {
        //Check if point already exists, as the corner points belong to 2 lines
        if(idx == 1) {
            if(nema_floats_equal(p_xy[0].x ,p0_x + (t * s1_x)) && nema_floats_equal(p_xy[0].y,p0_y + (t * s1_y))){
                return 0;
            }
        }
        // Collision detected
        // if (i_x != NULL)
        p_xy[idx].x = p0_x + (t * s1_x);
        // if (i_y != NULL)
        p_xy[idx].y = p0_y + (t * s1_y);
        return 1;
    }

    return 0; // No collision
}

static int points_coincide(nema_vg_vertex_t_ *p1, nema_vg_vertex_t_ *p2) {
    if ( nema_floats_equal(p1->x, p2->x) && nema_floats_equal(p1->y, p2->y) ) {
        return 1;
    }

    return 0;
}

// returns on how many points the line intersects the range bbox
static inline int calc_line_to_range_intersections(nema_vg_vertex_t_ p1 , nema_vg_vertex_t_ p2, nema_vg_vertex_t_* p, uint8_t start_point)
{
    (void) start_point;
    int ii;
    int idx = 0;

    ii = get_line_intersection(p1.x, p1.y, p2.x, p2.y, culled_info._min_coord, culled_info._min_coord, culled_info._max_coord, culled_info._min_coord, p, idx);
    if (ii != 0){
        if(points_coincide(&p1, p) == 0) {
            if(points_coincide(&p2, p) == 0) {
                ++idx;
            }
        }
    }
    ii = get_line_intersection(p1.x, p1.y, p2.x, p2.y, culled_info._min_coord, culled_info._max_coord, culled_info._max_coord, culled_info._max_coord, p, idx);
    if (ii != 0){
        if(points_coincide(&p1, p) == 0) {
            if(points_coincide(&p2, p) == 0) {
                ++idx;
            }
        }
    }
    ii = get_line_intersection(p1.x, p1.y, p2.x, p2.y, culled_info._min_coord, culled_info._min_coord, culled_info._min_coord, culled_info._max_coord, p, idx);
    if (ii != 0){
        if(points_coincide(&p1, p) == 0) {
            if(points_coincide(&p2, p) == 0) {
                ++idx;
            }
        }
    }
    ii = get_line_intersection(p1.x, p1.y, p2.x, p2.y, culled_info._max_coord, culled_info._min_coord, culled_info._max_coord, culled_info._max_coord, p, idx);
    if (ii != 0){
        if(points_coincide(&p1, p) == 0) {
            if(points_coincide(&p2, p) == 0) {
                ++idx;
            }
        }
    }

    PRINT_F("\t\t%s: idx: %d\n", __func__, idx);
    return idx;
}

#define IMPLICIT_CLOSE_ 0x1ffU

static uint32_t add_last_point( nema_vg_vertex_t_ p, uint8_t flag) {
    if ( culled_info.prev_cmd_is_move == (/*NEMA_VG_PRIM_CLOSE |*/ 0x100U) ) {
        ADD_SEG(/*NEMA_VG_PRIM_CLOSE |*/ flag);
        culled_info.prev_cmd_is_move = 0U;
    }
    else if ( culled_info.prev_cmd_is_move == IMPLICIT_CLOSE_ ) {
        culled_info.prev_cmd_is_move = 0U;
    }
    else {
        ADD_SEG((uint8_t)(NEMA_VG_PRIM_LINE | flag));
        ADD_DATA(p);
    }

    return NEMA_VG_ERR_NO_ERROR;
}

static uint32_t calc_move_geometry(nema_vg_vertex_t_ p)
{
    nema_vg_vertex_t_ p1 = p;
    nema_vg_vertex_t_ p2 = p;
    clamp_vertex(&p1, &p2);
    ADD_SEG(NEMA_VG_PRIM_MOVE);
    ADD_DATA(p1);
    return NEMA_VG_ERR_NO_ERROR;
}

static uint32_t calc_line_geometry(nema_vg_vertex_t_ p1, nema_vg_vertex_t_ p2)
{
    int oor_p1 = 0;
    if ( OUT_OF_RANGE(p1) ) {
        oor_p1 = 1;
    }

    int oor_p2 = 0;
    if ( OUT_OF_RANGE(p2) ) {
        oor_p2 = 1;
    }

    nema_vg_vertex_t_ p[2]  = {0};

    if ( oor_p1 == 0 && oor_p2 == 0 )
    {
        PRINT_F("ALL IN\n");
        // everything is within range
        //no new segs/data needed
        NEMA_VG_IF_ERROR(add_last_point(p2, 0));
    }
    else if( oor_p1 == 0 && oor_p2 != 0 )
    {
        PRINT_F("P2 OUT\n");
        // p2 is out of range
        (void) calc_line_to_range_intersections(p1, p2, p, 0);

        uint32_t LINE = NEMA_VG_PRIM_LINE;
        if ( culled_info.prev_cmd_is_move == IMPLICIT_CLOSE_ ) {
            LINE = NEMA_VG_PRIM_LINE | NEMA_VG_PRIM_NO_STROKE;
        }
        ADD_SEG((uint8_t)LINE);
        ADD_DATA(p[0]);

        nema_vg_vertex_t_ pp2  = {0.f, 0.f};
        clamp_vertex(&pp2, &p2);
        NEMA_VG_IF_ERROR(add_last_point(pp2, NEMA_VG_PRIM_NO_STROKE));
    }
    else if( oor_p1 != 0 && oor_p2 == 0 )
    {
        PRINT_F("P1 OUT\n");
        // p1 is out of range
        (void) calc_line_to_range_intersections(p1, p2, p, 1);
        // coming from projection
        ADD_SEG((uint8_t)(NEMA_VG_PRIM_LINE | NEMA_VG_PRIM_NO_STROKE));
        ADD_DATA(p[0]);

        NEMA_VG_IF_ERROR(add_last_point(p2, 0));
    }
    else //if( oor_p1 != 0 && oor_p2 != 0 )
    {
        // p1 and p2 is out of range
        PRINT_F("OUT OF RANGE\n");
        int points = calc_line_to_range_intersections(p2, p1, p, 1);
        PRINT_F("points: %d\n", points);

        nema_vg_vertex_t_ pp1  = {0.f, 0.f};
        clamp_vertex(&pp1, &p1);
        nema_vg_vertex_t_ pp2  = {0.f, 0.f};
        clamp_vertex(&pp2, &p2);

        if ( points != 2 ) {
            if ( !nema_floats_equal(pp1.x,pp2.x) && !nema_floats_equal(pp1.y,pp2.y) ) {
                nema_vg_vertex_t_ corner1 = {pp1.x, pp2.y};
                nema_vg_vertex_t_ corner2 = {pp2.x, pp1.y};

                // equation for distance of a point from a line
                // d=(x−x1)(y2−y1)−(y−y1)(x2−x1)

                float dy = p2.y-p1.y;
                float dx = p2.x-p1.x;

                float d1 = (corner1.x - p1.x)*dy - (corner1.y - p1.y)*dx;
                float d2 = (corner2.x - p1.x)*dy - (corner2.y - p1.y)*dx;

                ADD_SEG((uint8_t)(NEMA_VG_PRIM_LINE | NEMA_VG_PRIM_NO_STROKE));
                if ( nema_absf(d1) < nema_absf(d2) ) {
                    ADD_DATA( corner1 );
                }
                else {
                    ADD_DATA( corner2 );
                }
            }

            NEMA_VG_IF_ERROR(add_last_point(pp2, NEMA_VG_PRIM_NO_STROKE));
        }
        else {
            // 2 intersecting points
            // coming from projection
            ADD_SEG((uint8_t)(NEMA_VG_PRIM_LINE | NEMA_VG_PRIM_NO_STROKE));
            uint32_t LINE = NEMA_VG_PRIM_LINE;
            if ( culled_info.prev_cmd_is_move == IMPLICIT_CLOSE_ ) {
                LINE = NEMA_VG_PRIM_LINE | NEMA_VG_PRIM_NO_STROKE;
            }

            if ( nema_floats_equal(pp1.x, p[0].x) || nema_floats_equal(pp1.y, p[0].y)) {
               ADD_DATA( p[0] );
               ADD_SEG((uint8_t)LINE);
               ADD_DATA( p[1] );
            }
            else {
               ADD_DATA( p[1] );
               ADD_SEG((uint8_t)LINE);
               ADD_DATA( p[0] );
            }

            NEMA_VG_IF_ERROR(add_last_point(pp2, NEMA_VG_PRIM_NO_STROKE));
        }
    }

    return NEMA_VG_ERR_NO_ERROR;
}

//rasterize a cubic bezier via tesselation (fixed segment count) - draw with lines
static uint32_t
cubic_decasteljau(  nema_vg_vertex_t_ p0 ,
                    nema_vg_vertex_t_ pc0,
                    nema_vg_vertex_t_ pc1,
                    nema_vg_vertex_t_ p1 )
{
    int iter = 1;
    nema_vg_vertex_t_ p0_  = p0;
    nema_vg_vertex_t_ pc0_ = pc0;
    nema_vg_vertex_t_ pc1_ = pc1;
    nema_vg_vertex_t_ p1_  = p1;

    do  {
        uint8_t in_range = 0U;
        if ( !OUT_OF_RANGE(p0_) &&
             !OUT_OF_RANGE(pc0_) &&
             !OUT_OF_RANGE(pc1_) &&
             !OUT_OF_RANGE(p1_)
             ) {
                 // the entire cubic is within range, so don't subdivide
                 in_range = 1U;
             }

        uint8_t in_fb = (cubic_outside_of_fb(p0_, pc0_, pc1_, p1_) == 0U) ? 1U : 0U ;

        uint8_t subdivide = 1U;
        if ( in_range  != 0U ) {
            subdivide = 0U;
        }
        if ( in_fb  == 0U ) {
            subdivide = 0U;
        }

        if ( subdivide != 0U && nemavg_context->info.tess_stack_idx < STACK_DEPTH ) {
            // Continue subdivision
            //----------------------
            // tess_stack_push( x0, y0, x01, y01, mx, my);
            nema_vg_vertex_t_ p01, p12, p23, p123, p012, p0123;
            // Calculate all the mid-points of the line segments
            //----------------------
            p01.x   = (p0_.x  + pc0_.x) * 0.5f;
            p01.y   = (p0_.y  + pc0_.y) * 0.5f;
            p12.x   = (pc0_.x + pc1_.x) * 0.5f;
            p12.y   = (pc0_.y + pc1_.y) * 0.5f;
            p23.x   = (pc1_.x + p1_.x ) * 0.5f;
            p23.y   = (pc1_.y + p1_.y ) * 0.5f;
            p012.x  = (p01.x  + p12.x ) * 0.5f;
            p012.y  = (p01.y  + p12.y ) * 0.5f;
            p123.x  = (p12.x  + p23.x ) * 0.5f;
            p123.y  = (p12.y  + p23.y ) * 0.5f;
            p0123.x = (p012.x + p123.x) * 0.5f;
            p0123.y = (p012.y + p123.y) * 0.5f;

            tess_stack_push(p123.x, p123.y, p23.x, p23.y, p1_.x, p1_.y);
            ++iter;

            pc0_.x = p01.x;
            pc0_.y = p01.y;
            pc1_.x = p012.x;
            pc1_.y = p012.y;
            p1_.x  = p0123.x;
            p1_.y  = p0123.y;
        } else {
            // Stop subdivision
            --iter;

            if ( in_fb == 0U ) {
                // do line

                NEMA_VG_IF_ERROR(calc_line_geometry(p0_, p1_));
            }
            else {
                // it's within range
                // so draw it directly
                ADD_SEG(NEMA_VG_PRIM_BEZIER_CUBIC);
                ADD_DATA(pc0_);
                ADD_DATA(pc1_);
                ADD_DATA(p1_);
            }

            if (nemavg_context->info.tess_stack_idx != 0) {
                float x1_ = p1_.x;
                float y1_ = p1_.y;
                tess_stack_pop(&pc0_.x, &pc0_.y, &pc1_.x, &pc1_.y, &p1_.x, &p1_.y);
                p0_.x = x1_;
                p0_.y = y1_;
            }
        }

    } while ( iter != 0 );

    return NEMA_VG_ERR_NO_ERROR;
}

//rasterize a quadratic bezier via tesselation (fixed segment count) - draw with lines
static uint32_t
quadratic_decasteljau(  nema_vg_vertex_t_ p0,
                        nema_vg_vertex_t_ p1,
                        nema_vg_vertex_t_ p2
)
{
    int iter = 1;
    nema_vg_vertex_t_ p0_ = p0;
    nema_vg_vertex_t_ p1_ = p1;
    nema_vg_vertex_t_ p2_ = p2;

    do  {

        uint8_t in_range = 0U;
        if ( !OUT_OF_RANGE(p0_) &&
             !OUT_OF_RANGE(p1_) &&
             !OUT_OF_RANGE(p2_)
             ) {
                 // the entire cubic is within range, so don't subdivide
                 in_range = 1U;
             }

        uint8_t in_fb = (quad_outside_of_fb(p0_, p1_, p2_) == 0U) ? 1U : 0U;

        uint8_t subdivide = 1U;
        if ( in_range != 0U ) {
            subdivide = 0U;
        }
        if ( in_fb == 0U ) {
            subdivide = 0U;
        }

        if ( subdivide != 0U && nemavg_context->info.tess_stack_idx < STACK_DEPTH ) {
            nema_vg_vertex_t_ p01, p12, p012;
            p01.x   = (p0_.x  + p1_.x ) * 0.5f;
            p01.y   = (p0_.y  + p1_.y ) * 0.5f;
            p12.x   = (p1_.x  + p2_.x ) * 0.5f;
            p12.y   = (p1_.y  + p2_.y ) * 0.5f;
            p012.x  = (p01.x + p12.x)   * 0.5f;
            p012.y  = (p01.y + p12.y)   * 0.5f;
            // Continue subdivision
            //----------------------
            tess_stack_push( p012.x, p012.y, p12.x, p12.y, p2.x, p2.y);
            p1_.x = p01.x;
            p1_.y = p01.y;
            p2_.x = p012.x;
            p2_.y = p012.y;
            ++iter;
        } else {
            // Stop subdivision
            //----------------------
            --iter;

            if ( in_fb == 0U ) {
                // do line
                NEMA_VG_IF_ERROR(calc_line_geometry(p0_, p2_));
            }
            else {
                // it's within range
                // so draw it directly
                ADD_SEG(NEMA_VG_PRIM_BEZIER_QUAD);
                ADD_DATA(p1_);
                ADD_DATA(p2_);
            }

            tess_stack_pop(&p0_.x, &p0_.y, &p1_.x, &p1_.y, &p2_.x, &p2_.y);
        }

    } while ( iter != 0 );

    return NEMA_VG_ERR_NO_ERROR;
}

// forward declaration
static uint32_t calc_move_geometry(nema_vg_vertex_t_ p);

#define CALC_BBOX
#define CLIPPING

#ifdef HW_VERTEX_MMUL
#define REDEF_HW_VERTEX_MMUL
#undef HW_VERTEX_MMUL
#endif

#include "process_path_vertex.cc"

#ifdef REDEF_HW_VERTEX_MMUL
#define HW_VERTEX_MMUL
#endif

#undef CALC_BBOX
#undef CLIPPING

//=========================================================================
// Handling of Arcs
static uint32_t arc_to_cubic(nema_vg_info_t_* path_info, float angle0_deg, float angle1_deg, float cx, float cy,
            float rx, float ry, uint8_t force_point)
{
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
        if (culled_info.has_transformation != 0U)  {
            transform_vertex(culled_info.path_plus_global_m, &arc_info.p1);
        }
    }

    transform_arc( &arc_info.ctrl0, cos0, sin0, 1.f, 1.f, 0.f, 0.f);
    transform_arc( &arc_info.ctrl0, cos_rot, sin_rot, rx, ry, cx, cy);

    transform_arc( &arc_info.ctrl1, cos0, sin0, 1.f, 1.f, 0.f, 0.f);
    transform_arc( &arc_info.ctrl1, cos_rot, sin_rot, rx, ry, cx, cy);

    if ( (force_point & 2U) != 0U ) {
            // CW
        arc_info.p2 = path_info->p2;
    }
    else
    {
        transform_arc( &arc_info.p2   , cos0, sin0, 1.f, 1.f, 0.f, 0.f);
        transform_arc( &arc_info.p2   , cos_rot, sin_rot, rx, ry, cx, cy);
        if (culled_info.has_transformation != 0U)  {
            transform_vertex(culled_info.path_plus_global_m, &arc_info.p2);
        }
    }

    if (culled_info.has_transformation != 0U)  {
        transform_vertex(culled_info.path_plus_global_m, &arc_info.ctrl0);
        transform_vertex(culled_info.path_plus_global_m, &arc_info.ctrl1);
    }

    NEMA_VG_IF_ERROR(cubic_decasteljau(arc_info.p1, arc_info.ctrl0, arc_info.ctrl1, arc_info.p2));
    return NEMA_VG_ERR_NO_ERROR;
}

/*
 * SVG arc representation uses "endpoint parameterisation" where we specify the start and endpoint of the arc.
 * This is to be consistent with the other path commands. However we need to convert this to "centre point
 * parameterisation" in order to calculate the arc. Luckily, the SVG spec provides all the required maths
 * in section "F.6 Elliptical arc implementation notes".
 * The following code resembles the code in fill_arc, findUnitCircles and render_arc functions in nema_vg.c
 */
static uint32_t calc_arc_geometry(nema_vg_info_t_* path_info)
{
    float x0p  = path_info->previous_ctrl.x;
    float y0p  = path_info->previous_ctrl.y;
    float x1p  = path_info->previous_data.x;
    float y1p  = path_info->previous_data.y;

    if (nema_floats_equal(x0p,x1p) && nema_floats_equal(y0p,y1p)) {
        // If the endpoints (x1, y1) and (x0, y0) are identical, then this
        // is equivalent to omitting the elliptical arc segment entirely.
        // (behaviour specified by the spec)
        return NEMA_VG_ERR_NO_ERROR;
    }

    float rx  = path_info->ctrl0.x;
    float ry  = path_info->ctrl0.y;

    // Handle degenerate case (behaviour specified by the spec)
    // Ensure radii are non-zero (else treat this as a straight line)
    if (nema_float_is_zero(rx) || nema_float_is_zero(ry)) {
         NEMA_VG_IF_ERROR(calc_line_geometry(path_info->p1, path_info->p2));
         return NEMA_VG_ERR_NO_ERROR;
    }

    // Code from findUnitCircles follows

    float cx, cy;
    uint32_t CW, large;
    float theta0_deg, theta1_deg;
    if ((path_info->seg & NEMA_VG_ARC_CW) != 0U)
         { CW = 1U; }
    else { CW = 0U; }

    if ((path_info->seg & NEMA_VG_ARC_LARGE) != 0U)
         { large = 1U; }
    else { large = 0U; }

    // Ensure radii are positive
    // Sign of the radii is ignored (behaviour specified by the spec)
    rx = nema_absf(rx);
    ry = nema_absf(ry);

    float rot_deg = path_info->ctrl1.x;

    // Convert rotation angle from degrees to radians
    float rot_rad = nema_deg_to_rad(rot_deg);
    // Pre-compute rotation matrix entries */
    float cosine = COS_(rot_rad);
    float sine = SIN_(rot_rad);
    // Transform (x0, y0) and (x1, y1) into unit space
    // using (inverse) rotate, followed by (inverse) scale
    float x0 = ( x0p*cosine + y0p*sine  )/rx;
    float y0 = (-x0p*sine   + y0p*cosine)/ry;
    float x1 = ( x1p*cosine + y1p*sine  )/rx;
    float y1 = (-x1p*sine   + y1p*cosine)/ry;

    // Compute differences and averages
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
        theta0_deg = theta00_deg;
        theta1_deg = theta01_deg;
    }
    else {
        ucx = xm - sdy;
        ucy = ym + sdx;
        float theta10_deg = nema_rad_to_deg( ATAN2_(y0-ucy, x0-ucx) );
        float theta11_deg = nema_rad_to_deg( ATAN2_(y1-ucy, x1-ucx) );

        theta0_deg = theta10_deg;
        theta1_deg = theta11_deg;
    }

    ucx *= rx;
    ucy *= ry;

    cx = ucx*cosine - ucy*sine;
    cy = ucx*sine   + ucy*cosine;

    // Code from fill_arc follows
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

        // break up to 4 cubics
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

                NEMA_VG_IF_ERROR(arc_to_cubic(path_info, theta_deg, end_theta, cx, cy, rx, ry, force_point));

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

                NEMA_VG_IF_ERROR(arc_to_cubic(path_info, theta_deg, end_theta, cx, cy, rx, ry, force_point));

                force_point = 0U;
                theta_deg += step_deg;
            }
        }
    }

    return NEMA_VG_ERR_NO_ERROR;
}

static uint32_t calc_new_geometry(nema_vg_path_t* path)
{
    size_t idx_seg = 0;
    nema_vg_info_t_ path_info = {0};
    nema_vbuf_t_ *vb = &path->shape;

    path_info.no_move = 1U;
    path_info.new_path = 1U;

    uint8_t seg;
    uint8_t seg_no_rel;
    while (idx_seg < vb->seg_size)  {
        culled_info.prev_cmd_is_move = 0U;
        seg = (vb->seg)[idx_seg]; idx_seg++;
        seg_no_rel = (uint8_t)(seg & NEMA_VG_NO_REL_MASK); //remove rel from seg to avoid multi checking
        NEMA_VG_IF_ERROR(process_path_vertex(vb, seg, &path_info));

        if( path_info.no_move == 1U ) {
            PRINT_F("# IMPLICIT MOVE, %f, %f\n", path_info.p1.x, path_info.p1.y);
            NEMA_VG_IF_ERROR(calc_move_geometry(path_info.p1));
            path_info.no_move = 0U;
        }

        if ( seg_no_rel == NEMA_VG_PRIM_BEZIER_QUAD || seg_no_rel == NEMA_VG_PRIM_BEZIER_SQUAD ) {
            PRINT_F("#  QUAD, %f, %f\n", path_info.ctrl0.x, path_info.ctrl0.y);
            PRINT_F("#      , %f, %f\n", path_info.p2.x, path_info.p2.y);
            NEMA_VG_IF_ERROR(quadratic_decasteljau(path_info.p1, path_info.ctrl0, path_info.p2));
        }
        else if ( seg_no_rel == NEMA_VG_PRIM_BEZIER_CUBIC || seg_no_rel == NEMA_VG_PRIM_BEZIER_SCUBIC ) {
            PRINT_F("# CUBIC, %f, %f\n", path_info.ctrl0.x, path_info.ctrl0.y);
            PRINT_F("#      , %f, %f\n", path_info.ctrl1.x, path_info.ctrl1.y);
            PRINT_F("#      , %f, %f\n", path_info.p2.x, path_info.p2.y);
            NEMA_VG_IF_ERROR(cubic_decasteljau(path_info.p1, path_info.ctrl0, path_info.ctrl1, path_info.p2));
        }
        else if ( (seg & NEMA_VG_PRIM_MASK) == NEMA_VG_PRIM_ARC ) {
            PRINT_F("#  ARC\n");
            PRINT_F("# rx %f, ry %f\n", path_info.ctrl0.x, path_info.ctrl0.y);
            PRINT_F("# rot_deg  %f\n", path_info.ctrl1.x);
            PRINT_F("# x0 %f, y0 %f\n", path_info.previous_ctrl.x, path_info.previous_ctrl.y);
            PRINT_F("# x1 %f, y1 %f\n", path_info.previous_data.x, path_info.previous_data.y);
            NEMA_VG_IF_ERROR(calc_arc_geometry(&path_info));
        }
        else if( seg_no_rel == NEMA_VG_PRIM_POLYGON || seg_no_rel == NEMA_VG_PRIM_POLYLINE ) {
            //calc_line_geometry() is called for each line inside process_path_vertex
        }
        else if( seg_no_rel == NEMA_VG_PRIM_LINE  ||
                 seg_no_rel == NEMA_VG_PRIM_VLINE ||
                 seg_no_rel == NEMA_VG_PRIM_HLINE
        ) {
            PRINT_F("#  LINE, %f, %f\n", path_info.p2.x, path_info.p2.y);
            NEMA_VG_IF_ERROR(calc_line_geometry(path_info.p1, path_info.p2));
        }
        else if( seg_no_rel == NEMA_VG_PRIM_MOVE) {
            PRINT_F("#  MOVE, %f, %f\n", path_info.p2.x, path_info.p2.y);
            NEMA_VG_IF_ERROR(calc_move_geometry(path_info.p2));
        }
        else{
            //MISRA
            PRINT_F("# CLOSE\n");

            culled_info.prev_cmd_is_move = /*NEMA_VG_PRIM_CLOSE |*/ 0x100U;
            // Probably not correct, check if close is out of range!!!
            NEMA_VG_IF_ERROR(calc_line_geometry(path_info.p1, path_info.p2));


            // ADD_SEG(seg_no_rel);
            // no ADD_DATA needed
        }
    } // while (idx_seg < vb->seg_size)

#if 0
    if(idx_seg == vb->seg_size && OUT_OF_RANGE(path_info.p2)) //if last point is out of range an implicit line is added to the start point
    {
        (void)calc_line_geometry(culled_info, path_info.p1, path_info.p2);
        // ADD_SEG(NEMA_VG_PRIM_CLOSE);
    }
#else
    if(path_info.no_close == 1U){
        nema_vg_vertex_t_ implicit_p2 = find_implicit_vertices(&path_info);
        culled_info.prev_cmd_is_move = IMPLICIT_CLOSE_;
        NEMA_VG_IF_ERROR(calc_line_geometry(path_info.p2, implicit_p2));
    }
#endif

    return NEMA_VG_ERR_NO_ERROR;
}

static void
calc_transformation(nema_vg_path_t *_path) {

    culled_info.has_transformation =    (uint8_t)(_path->flags   & NEMA_VG_PATH_HAS_TRANSFORMATION   )
                         |  (uint8_t)(nemavg_context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION );
    if (culled_info.has_transformation != 0U) {

        if ( (nemavg_context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION   ) != 0U ) {
            nema_mat3x3_copy(culled_info.path_plus_global_m, nemavg_context->global_m);
            if ( (_path->flags   & NEMA_VG_PATH_HAS_TRANSFORMATION     ) != 0U ) {
                nema_mat3x3_mul(culled_info.path_plus_global_m, _path->m);
            }
        }
        else {
            if ( (_path->flags   & NEMA_VG_PATH_HAS_TRANSFORMATION     ) != 0U ) {
                nema_mat3x3_copy(culled_info.path_plus_global_m, _path->m);
            }
            else {
                nema_mat3x3_load_identity(culled_info.path_plus_global_m);
            }
        }
    }
}

uint32_t calc_clipped_path_size(NEMA_VG_PATH_HANDLE path, NEMA_VG_PAINT_HANDLE paint,
                                        size_t* new_seg_size, size_t* new_data_size)
{
    (void) paint;
    culled_info._dry_run = 1;

    GET_CONTEXT
    float half_stroke = nemavg_context->stroke.width * 0.5f;

    culled_info._min_coord = MIN_COORD + 4.f + half_stroke;
    culled_info._max_coord = MAX_COORD - 4.f - half_stroke;
    GET_PATH


    calc_transformation(_path);
    clear_culled_info_struct();
    culled_info.new_seg_size  = *new_seg_size;
    culled_info.new_data_size = *new_data_size;

    *new_seg_size  = 0U;
    *new_data_size = 0U;

    //find new segments and vertices
    NEMA_VG_IF_ERROR(calc_new_geometry(_path));
    *new_seg_size  = culled_info.idx_new_seg;
    *new_data_size = culled_info.idx_new_data;

    return NEMA_VG_ERR_NO_ERROR;
}

uint32_t calc_clipped_path(NEMA_VG_PATH_HANDLE path, NEMA_VG_PAINT_HANDLE paint,
                                   size_t *new_seg_size, uint8_t* new_segs,
                                   size_t *new_data_size, nema_vg_float_t* new_data)
{
    (void) paint;
    culled_info._dry_run = 0;
    uint32_t err = NEMA_VG_ERR_NO_ERROR;

    if(new_segs == NULL || new_data == NULL)//if invalid pointers calculate only size
    {
        err =  NEMA_VG_ERR_BAD_BUFFER;
    }
;
    GET_CONTEXT
    float half_stroke = nemavg_context->stroke.width * 0.5f;

    culled_info._min_coord = MIN_COORD + 4.f + half_stroke;
    culled_info._max_coord = MAX_COORD - 4.f - half_stroke;
    GET_PATH


    calc_transformation(_path);
    clear_culled_info_struct();
    culled_info.new_segs = new_segs;
    culled_info.new_data = new_data;
    culled_info.new_seg_size  = *new_seg_size;
    culled_info.new_data_size = *new_data_size;

    //find new segments and vertices
    NEMA_VG_IF_ERROR(calc_new_geometry(_path));

    *new_seg_size  = culled_info.idx_new_seg;
    *new_data_size = culled_info.idx_new_data;

    return err;
}

void clipped_path_free(nema_buffer_t *seg_buffer, nema_buffer_t *data_buffer) {
    //Destroy the other buffer in case only one was created, we need both or none
    if ( seg_buffer->base_virt != NULL ) {
        nema_buffer_destroy(seg_buffer);
    }

    if ( data_buffer->base_virt != NULL ) {
        nema_buffer_destroy(data_buffer);
    }
}

uint32_t clipped_path_alloc(int seg_size, nema_buffer_t *seg_buffer, int data_size, nema_buffer_t *data_buffer) {
    *seg_buffer  = nema_buffer_create_pool(NEMA_MEM_POOL_CLIPPED_PATH, seg_size);
    (void)nema_buffer_map(seg_buffer);
    *data_buffer = nema_buffer_create_pool(NEMA_MEM_POOL_CLIPPED_PATH, data_size*4); //float, thus data_size*4
    (void)nema_buffer_map(data_buffer);

    if ( (seg_buffer->base_virt == NULL) || ((data_buffer->base_virt == NULL)) ) {
        clipped_path_free(seg_buffer, data_buffer);
        return NEMA_VG_ERR_BAD_BUFFER;
    } else {
        return NEMA_VG_ERR_NO_ERROR;
    }
}


#if ENABLE_CLIPPED_PATH2 != 0
uint32_t calc_clipped_path_size2(NEMA_VG_PATH_HANDLE path,
                                        size_t* new_seg_size, size_t* new_data_size,
                                        float min, float max)
{
    culled_info._dry_run = 1;
    culled_info._min_coord = min;
    culled_info._max_coord = max;
    *new_data_size = 0;
    *new_seg_size = 0;
    GET_PATH

    GET_CONTEXT

    calc_transformation(_path);
    clear_culled_info_struct();

    //dry run to calculate new data and seg size
    NEMA_VG_IF_ERROR(calc_new_geometry(_path, &culled_info));
    *new_data_size = culled_info.idx_new_data;
    *new_seg_size = culled_info.idx_new_seg;

    return NEMA_VG_ERR_NO_ERROR;
}

uint32_t calc_clipped_path2(NEMA_VG_PATH_HANDLE path, NEMA_VG_PAINT_HANDLE paint,
                                   size_t *new_seg_size, uint8_t* new_segs,
                                   size_t *new_data_size, nema_vg_float_t* new_data,
                                   float min, float max)
{
    culled_info._min_coord = min;
    culled_info._max_coord = max;
    culled_info._dry_run = 0;
    (void) paint;
    GET_PATH
    GET_CONTEXT

    calc_transformation(_path);
    clear_culled_info_struct();
    culled_info.new_segs = new_segs;
    culled_info.new_data = new_data;
    culled_info.new_seg_size  = *new_seg_size;
    culled_info.new_data_size = *new_data_size;

    //find new segments and vertices
    NEMA_VG_IF_ERROR(calc_new_geometry(_path, &culled_info));

    *new_seg_size  = culled_info.idx_new_seg;
    *new_data_size = culled_info.idx_new_data;

    return NEMA_VG_ERR_NO_ERROR;
}
#endif

uint32_t draw_clipped_path(NEMA_VG_PATH_HANDLE path, NEMA_VG_PAINT_HANDLE paint)
{
    GET_CONTEXT
    culled_info._max_coord = MAX_COORD;
    culled_info._min_coord = MIN_COORD;

    size_t clipped_segs_size = nemavg_context->clipped_segs_size_bytes;
    size_t clipped_data_size = nemavg_context->clipped_data_size_bytes/sizeof(nema_vg_float_t);

    uint32_t err = NEMA_VG_ERR_NO_ERROR;
    uint8_t buffer_enough = 0U;
    if(nemavg_context->clipped_segs == NULL || nemavg_context->clipped_data == NULL)//if invalid pointers calculate only size
    {
        buffer_enough = 0;
        (void) calc_clipped_path_size(path, paint, &clipped_segs_size, &clipped_data_size);
    }
    else
    {
        err = calc_clipped_path(path, paint, &clipped_segs_size, nemavg_context->clipped_segs->base_virt, &clipped_data_size, nemavg_context->clipped_data->base_virt);
        if(culled_info.error != NEMA_VG_ERR_BAD_BUFFER && err == NEMA_VG_ERR_NO_ERROR)
        {
            buffer_enough = 1;
        }
        else{
            buffer_enough = 0;
        }
    }
#ifndef NEMAPVG
    enable_do_not_transform(1);
#endif //NEMAPVG
    if ( buffer_enough != 0U) {
        GET_PATH
        nema_vg_path_t local_path = *_path;
        nema_vg_path_set_shape((void *)&local_path, clipped_segs_size, nemavg_context->clipped_segs->base_virt, clipped_data_size, nemavg_context->clipped_data->base_virt);

        nemavg_context->drawing_clipped_path = 1U;
        err =  nema_vg_draw_path((void *)&local_path, paint);
        nemavg_context->drawing_clipped_path = 0U;
    }
    else if(((nemavg_context->flags & NEMA_VG_CONTEXT_ALLOW_INTERNAL_ALLOC) != 0U)) //it can't fit to predefined size for clipped data/seg allocate space
    {
        nema_buffer_t segs_buffer;
        nema_buffer_t data_buffer;

        int i_segs_size = (int)clipped_segs_size;
        int i_data_size = (int)clipped_data_size;
        err = clipped_path_alloc(i_segs_size, &segs_buffer, i_data_size, &data_buffer);

        if ( err == NEMA_VG_ERR_NO_ERROR ) {
            err = calc_clipped_path(path, paint, &clipped_segs_size, segs_buffer.base_virt,
                                                         &clipped_data_size, data_buffer.base_virt);

            if ( err == NEMA_VG_ERR_NO_ERROR ) {
                GET_PATH
                nema_vg_path_t local_path = *_path;
                nema_vg_path_set_shape((void *)&local_path, clipped_segs_size, segs_buffer.base_virt,
                                             clipped_data_size, data_buffer.base_virt);

                nemavg_context->drawing_clipped_path = 1U;
                err =  nema_vg_draw_path((void *)&local_path, paint);
                nemavg_context->drawing_clipped_path = 0U;
            }

            clipped_path_free(&segs_buffer, &data_buffer);
        }
    }
    else{
       err = NEMA_VG_ERR_BAD_BUFFER;
    }
#ifndef NEMAPVG
    enable_do_not_transform(0U);
#endif //NEMAPVG
    return err;// if here error is NEMA_VG_ERR_NO_ERROR
}
