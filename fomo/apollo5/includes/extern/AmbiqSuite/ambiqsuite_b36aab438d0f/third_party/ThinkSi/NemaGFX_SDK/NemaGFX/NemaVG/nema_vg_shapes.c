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
#include "nema_vg_context.h"
#include "nema_vg.h"
#include "nema_vg_p.h"
#include "nema_utils.h"
#include "nema_raster.h"
#include "nema_programHW.h"
#include "nema_rasterizer_intern.h"
#include "nema_regs.h"

static int should_draw_path(nema_vg_paint_t *paint, uint32_t blend)
{
    uint8_t has_stroke_draw = 0U;
    if (nemavg_context->fill_rule == NEMA_VG_STROKE){
        has_stroke_draw = 1U;
    }

    if (( (nemavg_context->quality & NEMA_VG_QUALITY_MAXIMUM) == NEMA_VG_QUALITY_MAXIMUM ) &&
         nemavg_context->fill_rule != NEMA_VG_STROKE) {
        return 1;
    }

    if (blend == NEMA_BL_SRC){
        return 0;
    }

    if((blend & NEMA_BLOP_SRC_PREMULT) != 0U &&
        paint->type != NEMA_VG_PAINT_COLOR) {
        return 1;
    }

    uint8_t is_source_over = 0U;
    if ((blend&0xffffU) == NEMA_BL_SRC_OVER) {
        is_source_over = 1U;
    }

    uint8_t has_transparent_color = 0U;
    if (paint->type == NEMA_VG_PAINT_COLOR) {
        if ( is_source_over == 1U && (paint->paint_color & 0xff000000U) != 0xff000000U ) {
            has_transparent_color = 1U;
        }
    }
    if (is_source_over == 1U && paint->opacity < 1.0f){
        has_transparent_color = 1U;
    }

    if ( (has_stroke_draw == 1U) && (has_transparent_color == 1U)) {
        return 1;
    }

    return 0;
}

static uint8_t should_handle_large_coords(nema_vg_vertex_t_ bbox[4], nema_matrix3x3_t m)
{
    float min_coord, max_coord;
    nema_vg_get_coord_limits(&min_coord, &max_coord);

    if ( (nemavg_context->flags & NEMA_VG_CONTEXT_ENABLE_HANDLE_LARGE_COORDS) == 0U){
        return 0U;
    }

    if (m != NULL){
        for (int i=0; i<4; ++i){
            nema_mat3x3_mul_vec(m, &bbox[i].x, &bbox[i].y);
        }
    }

    for (int i=0; i<4; ++i){
        // Check for out of limit bbox
        if (bbox[i].x > max_coord || bbox[i].x < min_coord){
            return 1U;
        }
        if (bbox[i].y > max_coord || bbox[i].y < min_coord){
            return 1U;
        }
    }

    return 0U;
}

static uint32_t check_shapes(nema_vg_paint_t *paint)
{
    NEMA_VG_ASSERT(paint, NEMA_VG_ERR_BAD_HANDLE);
    if (paint->opacity <= 0.0f){
        return NEMA_VG_ERR_INVALID_OPACITY;
    }

    if (nemavg_context->fill_rule == NEMA_VG_STROKE && nemavg_context->stroke.width <= 0.0f){
        return NEMA_VG_ERR_INVALID_STROKE_WIDTH;
    }
    nemavg_context->info.has_cap = 0;//caps apply only in paths

    return NEMA_VG_ERR_NO_ERROR;
}

static uint32_t shape_bind_blend(nema_vg_paint_t *paint, uint32_t blend, uint32_t paint_color) {
    uint32_t paint_color_ = paint_color;
    uint32_t blend_       = blend;
    bind_tex1_according_paint(paint);

    const uint32_t blit_stencil_cmd_nonpremul_opa[] = { 0x0000110bU, 0x00000000U,
                                                        0x080c0000U, 0x000121c7U,
                                                        0x080c0002U, 0x80046187U};
    const uint32_t blit_nonpremul_opa_src_ckey[] = { 0x000c110bU, 0x00000000U,
                                                     0x081c0000U, 0x000121c7U,
                                                     0x080c0002U, 0x80046187U};

    if (paint->type == NEMA_VG_PAINT_COLOR){

        if ( paint->opacity < 1.f ) {
            uint32_t a = paint_color_ >> 24U;
            float af = (float)a;
            af *= paint->opacity;
            af += 0.5f;
            uint32_t a_new = (uint32_t)af;
            a_new = a_new << 24U;

            paint_color_ &= 0x00ffffffU;
            paint_color_ |= a_new;
        }

        nema_set_blend_fill(blend_);
        nema_set_raster_color(paint_color_);
    }
    else{
        blend_ = (paint->type == NEMA_VG_PAINT_TEXTURE && paint->tex.is_lut_tex == 1U) ? blend_ | NEMA_BLOP_LUT : blend_ ;

        if ( paint->opacity < 1.f && ((blend_ & NEMA_BLOP_SRC_PREMULT) == 0U)) {
            float opaf = paint->opacity*255.f + 0.5f;
            int opa = (int)opaf;
            uint32_t rgba = ((uint32_t)opa << 24U) | ((uint32_t)opa << 16U) | ((uint32_t)opa << 8U) | ((uint32_t)opa);
            nema_set_const_color(rgba);

            blend_ |= NEMA_BLOP_MODULATE_A;
            blend_ |= NEMA_BLOP_MODULATE_RGB;

            nema_set_blend_blit(blend_);
        }
        else if (paint->opacity < 1.f && ((blend_ & NEMA_BLOP_SRC_PREMULT) != 0U)) {
            // ROP BLENDER SETUP
            nema_cl_add_cmd(NEMA_ROPBLENDER_BLEND_MODE, blend_);

            uint32_t cmd       = 2;
            uint32_t cmd_count = 2;
            uint32_t codeptr = 0x941e8000U;

            if (paint->type == NEMA_VG_PAINT_TEXTURE && paint->tex.is_lut_tex == 1U) {
                // opacity + lut
                            cmd = 0;
                            cmd_count = 3;
                            codeptr = 0x941da000U;
            }
            if ( (blend_ & NEMA_BLOP_SRC_CKEY) != 0U ) {
                // we need to do src ckey
                nema_load_frag_shader(&blit_nonpremul_opa_src_ckey[cmd], cmd_count+1U, 0);
            }
            else {
                nema_load_frag_shader(&blit_stencil_cmd_nonpremul_opa[cmd], cmd_count+1U, 0);
            }
            nema_set_frag_ptr(codeptr);

            float opaf = paint->opacity*255.f + 0.5f;
            int opa = (int)opaf;
            uint32_t rgba = ((uint32_t)opa << 24U) | ((uint32_t)opa << 16U) | ((uint32_t)opa << 8U) | ((uint32_t)opa);
            nema_set_const_color(rgba);
        }
        else {
            nema_set_blend_blit(blend_);
        }
    }

    return paint_color_;
}

static void draw_rounded_rect_corner(const nema_vg_vertex_t_* fan_center,
                                     const nema_vg_vertex_t_* p1,
                                     const nema_vg_vertex_t_* ctrl0,
                                     const nema_vg_vertex_t_* ctrl1,
                                     const nema_vg_vertex_t_* p2)
{
    nema_vg_info_t_ path_info;
    int cix, ciy;

    cix = vertex_2fx(fan_center->x);
    ciy = vertex_2fx(fan_center->y);
    reset_last_vertex();
    set_fan_center(fan_center->x, fan_center->y);
    set_raster_point(4, cix, ciy);
    path_info.p1 = *p1;
    path_info.ctrl0 = *ctrl0;
    path_info.ctrl1 = *ctrl1;
    path_info.p2 = *p2;
    enable_draw_on_fb(1);
    enable_use_bevel_joins(1);
    (void)fill_cubic(&path_info);
    enable_draw_on_fb(0);
    enable_use_bevel_joins(0);
}

static void draw_rounded_rect_side(const nema_vg_vertex_t_* p1,
                                   const nema_vg_vertex_t_* p2,
                                   const nema_vg_vertex_t_* p1_,
                                   const nema_vg_vertex_t_* p2_)
{
    if (nemavg_context->fill_rule != NEMA_VG_STROKE){
        uint32_t prev_aa = 0;
        if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
            prev_aa = nema_enable_aa(1, 0, 0, 0);
        }
        nema_raster_quad_f(p1->x, p1->y, p2->x, p2->y, p1_->x, p1_->y, p2_->x, p2_->y);
        (void)nema_enable_aa_flags(prev_aa);
    }else{
        raster_stroked_line_(*p1, *p2, 0);
    }
}

static void draw_rounded_rect_geometry(const rounded_rect_points *points)
{

    draw_rounded_rect_corner(&(points->AH),
                             &(points->H),
                             &(points->AH_ctrl0),
                             &(points->AH_ctrl1),
                             &(points->A));
    draw_rounded_rect_side(&(points->A),
                           &(points->B),
                           &(points->BC),
                           &(points->AH));

    draw_rounded_rect_corner(&(points->BC),
                             &(points->B),
                             &(points->BC_ctrl0),
                             &(points->BC_ctrl1),
                             &(points->C));
    draw_rounded_rect_side(&(points->C),
                           &(points->D),
                           &(points->DE),
                           &(points->BC));

    draw_rounded_rect_corner(&(points->DE),
                             &(points->D),
                             &(points->DE_ctrl0),
                             &(points->DE_ctrl1),
                             &(points->E));
    draw_rounded_rect_side(&(points->E),
                           &(points->F),
                           &(points->FG),
                           &(points->DE));

    draw_rounded_rect_corner(&(points->FG),
                             &(points->F),
                             &(points->FG_ctrl0),
                             &(points->FG_ctrl1),
                             &(points->G));
    draw_rounded_rect_side(&(points->G),
                           &(points->H),
                           &(points->AH),
                           &(points->FG));


    if (nemavg_context->fill_rule == NEMA_VG_STROKE){
        flush_lines_(1, 0);
    }
    else{
        uint32_t prev_aa = 0;
        if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
            prev_aa = nema_enable_aa(0, 0, 0, 0);
        }
        nema_raster_quad_f(points->AH.x, points->AH.y, points->BC.x, points->BC.y, points->DE.x, points->DE.y, points->FG.x, points->FG.y);
        (void)nema_enable_aa_flags(prev_aa);
    }
}

static void draw_rect_geometry(const rect_points *points)
{
    if (nemavg_context->fill_rule != NEMA_VG_STROKE){
        uint32_t prev_aa = 0;
        if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
            prev_aa = nema_enable_aa(1, 1, 1, 1);
        }
        nema_raster_quad_f(points->p1_up_left.x, points->p1_up_left.y, points->p2_up_right.x, points->p2_up_right.y,
        points->p3_up_left.x, points->p3_up_left.y, points->p4_up_right.x, points->p4_up_right.y);
        (void)nema_enable_aa_flags(prev_aa);
    }else{
        uint32_t prev_aa = 0;
        if ((nemavg_context->quality & NEMA_VG_QUALITY_NON_AA) != NEMA_VG_QUALITY_NON_AA){
            prev_aa = nema_enable_aa(1, 0, 1, 0);
        }
        nema_raster_quad_f(points->p1_up_left.x, points->p1_up_left.y, points->p2_up_right.x, points->p2_up_right.y,
                            points->p2_down_left.x, points->p2_down_left.y, points->p1_down_right.x, points->p1_down_right.y);

        nema_raster_quad_f(points->p2_up_right.x, points->p2_up_right.y, points->p3_down_right.x, points->p3_down_right.y,
                            points->p3_up_left.x, points->p3_up_left.y, points->p2_down_left.x, points->p2_down_left.y);

        nema_raster_quad_f(points->p3_down_right.x, points->p3_down_right.y, points->p4_down_left.x, points->p4_down_left.y,
                            points->p4_up_right.x, points->p4_up_right.y, points->p3_up_left.x, points->p3_up_left.y);

        nema_raster_quad_f(points->p4_down_left.x, points->p4_down_left.y, points->p1_up_left.x, points->p1_up_left.y,
                            points->p1_down_right.x, points->p1_down_right.y, points->p4_up_right.x, points->p4_up_right.y);

        (void)nema_enable_aa_flags(prev_aa);
    }
}

static int ring_caps_intersect(nema_vg_vertex_t_ c1, nema_vg_vertex_t_ c2, float r)
{
    float dx = c1.x - c2.x;
    float dy = c1.y - c2.y;
    float rr = r * 2.0f; // caps have the same radius

    // if length squared of 2 centers is less than length squared of 2 radiuses
    if ((dx * dx + dy * dy) <= (rr * rr)) {
        return 1;
    }
    else {
        return 0;
    }
}

// if rx == ry then then curves correspond to a circle
static void create_ellipse_curves(float cx, float cy, float rx, float ry, ellipse_points *points)
{
    // create the points and control points of 4 cubic beziers with radius 1
    // and center (0, 0). Equations are from "A Primer on Bezier Curves, Chapter 42"

    float k = 0.55228f;
    /*points->right_point = {1.0f, 0.0f};
    points->right_down_ctrl0_point = {1.0f, k * 1.0f};
    points->right_down_ctrl1_point = {k * 1.0f, 1.0f};
    points->down_point = {0.0f, 1.0f};
    points->down_left_ctrl1_point = {-1.0f, k * 1.0f};
    points->down_left_ctrl0_point = {k * (-1.0f), 1.0f};
    points->left_point = {-1.0f, 0.0f};
    points->left_up_ctrl0_point = {-1.0f, k * (-1.0f)};
    points->left_up_ctrl1_point = {k * (-1.0f), -1.0f};
    points->up_point = {0.0f, -1.0f};
    points->up_right_ctrl1_point = {1.0f, k * (-1.0f)};
    points->up_right_ctrl0_point = {k * 1.0f, -1.0f};*/

    points->right_point.x = rx + cx;
    points->right_point.y = cy;
    points->right_down_ctrl0_point.x = rx + cx;
    points->right_down_ctrl0_point.y = k * ry + cy;
    points->right_down_ctrl1_point.x = k * rx + cx;
    points->right_down_ctrl1_point.y = ry + cy;
    points->down_point.x = cx;
    points->down_point.y = ry + cy;
    points->down_left_ctrl0_point.x = -k * rx + cx;
    points->down_left_ctrl0_point.y = ry + cy;
    points->down_left_ctrl1_point.x = -rx + cx;
    points->down_left_ctrl1_point.y = k * ry + cy;
    points->left_point.x = -rx + cx;
    points->left_point.y = cy;
    points->left_up_ctrl0_point.x = -rx + cx;
    points->left_up_ctrl0_point.y = -k * ry + cy;
    points->left_up_ctrl1_point.x = -k * rx + cx;
    points->left_up_ctrl1_point.y = -ry + cy;
    points->up_point.x = cx;
    points->up_point.y = -ry + cy;
    points->up_right_ctrl0_point.x = k * rx + cx;
    points->up_right_ctrl0_point.y = -ry + cy;
    points->up_right_ctrl1_point.x = rx + cx;
    points->up_right_ctrl1_point.y = -k * ry + cy;

}

static uint32_t draw_ellipse_path(const ellipse_points *points, nema_vg_paint_t *paint, nema_matrix3x3_t m)
{
    float coords[26] = {points->right_point.x, points->right_point.y,//move
                                points->right_down_ctrl0_point.x, points->right_down_ctrl0_point.y, //cubic ctrl 0
                                points->right_down_ctrl1_point.x,  points->right_down_ctrl1_point.y, //cubic ctrl 1
                                points->down_point.x, points->down_point.y, // cubic p2
                                points->down_left_ctrl0_point.x,  points->down_left_ctrl0_point.y, //cubic ctrl 0
                                points->down_left_ctrl1_point.x,  points->down_left_ctrl1_point.y, //cubic ctrl 1
                                points->left_point.x, points->left_point.y, // cubic p2
                                points->left_up_ctrl0_point.x,  points->left_up_ctrl0_point.y, //cubic ctrl 0
                                points->left_up_ctrl1_point.x,  points->left_up_ctrl1_point.y, //cubic ctrl 1
                                points->up_point.x, points->up_point.y, // cubic p2
                                points->up_right_ctrl0_point.x,  points->up_right_ctrl0_point.y, //cubic ctrl 0
                                points->up_right_ctrl1_point.x,  points->up_right_ctrl1_point.y, //cubic ctrl 1
                                points->right_point.x, points->right_point.y // cubic p2
                                };

    uint8_t cmds[6] = {NEMA_VG_PRIM_MOVE,
                        NEMA_VG_PRIM_BEZIER_CUBIC,
                        NEMA_VG_PRIM_BEZIER_CUBIC,
                        NEMA_VG_PRIM_BEZIER_CUBIC,
                        NEMA_VG_PRIM_BEZIER_CUBIC,
                        NEMA_VG_PRIM_CLOSE
                        };

    NEMA_VG_PATH_HANDLE path = nema_vg_path_create();
    nema_vg_path_clear(path);
    nema_vg_path_set_shape(path, 6L, cmds, 26L, coords);
    if (m != NULL){
        nema_vg_path_set_matrix(path, m);
    }

    uint32_t error = nema_vg_draw_path(path, paint);
    nema_vg_path_destroy(path);

    return error;
}

static uint32_t draw_stroked_circle_path(const ellipse_points *outer_points, const ellipse_points *inner_points, nema_vg_paint_t *paint, nema_matrix3x3_t m)
{
    float coords[52] = {
                        // the outer circle points
                        outer_points->right_point.x, outer_points->right_point.y,//move
                        outer_points->right_down_ctrl0_point.x, outer_points->right_down_ctrl0_point.y, //cubic ctrl 0
                        outer_points->right_down_ctrl1_point.x,  outer_points->right_down_ctrl1_point.y, //cubic ctrl 1
                        outer_points->down_point.x, outer_points->down_point.y, // cubic p2
                        outer_points->down_left_ctrl0_point.x,  outer_points->down_left_ctrl0_point.y, //cubic ctrl 0
                        outer_points->down_left_ctrl1_point.x,  outer_points->down_left_ctrl1_point.y, //cubic ctrl 1
                        outer_points->left_point.x, outer_points->left_point.y, // cubic p2
                        outer_points->left_up_ctrl0_point.x,  outer_points->left_up_ctrl0_point.y, //cubic ctrl 0
                        outer_points->left_up_ctrl1_point.x,  outer_points->left_up_ctrl1_point.y, //cubic ctrl 1
                        outer_points->up_point.x, outer_points->up_point.y, // cubic p2
                        outer_points->up_right_ctrl0_point.x,  outer_points->up_right_ctrl0_point.y, //cubic ctrl 0
                        outer_points->up_right_ctrl1_point.x,  outer_points->up_right_ctrl1_point.y, //cubic ctrl 1
                        outer_points->right_point.x, outer_points->right_point.y, // cubic p2
                        //move to the inner circle
                        inner_points->right_point.x, inner_points->right_point.y,//move
                        inner_points->up_right_ctrl1_point.x,  inner_points->up_right_ctrl1_point.y, //cubic ctrl 1
                        inner_points->up_right_ctrl0_point.x,  inner_points->up_right_ctrl0_point.y, //cubic ctrl 0
                        inner_points->up_point.x, inner_points->up_point.y, // cubic p2
                        inner_points->left_up_ctrl1_point.x,  inner_points->left_up_ctrl1_point.y, //cubic ctrl 1
                        inner_points->left_up_ctrl0_point.x,  inner_points->left_up_ctrl0_point.y, //cubic ctrl 0
                        inner_points->left_point.x, inner_points->left_point.y, // cubic p2
                        inner_points->down_left_ctrl1_point.x,  inner_points->down_left_ctrl1_point.y, //cubic ctrl 1
                        inner_points->down_left_ctrl0_point.x,  inner_points->down_left_ctrl0_point.y, //cubic ctrl 0
                        inner_points->down_point.x, inner_points->down_point.y, // cubic p2
                        inner_points->right_down_ctrl1_point.x,  inner_points->right_down_ctrl1_point.y, //cubic ctrl 1
                        inner_points->right_down_ctrl0_point.x, inner_points->right_down_ctrl0_point.y, //cubic ctrl 0
                        inner_points->right_point.x, inner_points->right_point.y //move
                        };

    uint8_t cmds[11] = {
                        NEMA_VG_PRIM_MOVE,
                        NEMA_VG_PRIM_BEZIER_CUBIC,
                        NEMA_VG_PRIM_BEZIER_CUBIC,
                        NEMA_VG_PRIM_BEZIER_CUBIC,
                        NEMA_VG_PRIM_BEZIER_CUBIC,
                        NEMA_VG_PRIM_MOVE,
                        NEMA_VG_PRIM_BEZIER_CUBIC,
                        NEMA_VG_PRIM_BEZIER_CUBIC,
                        NEMA_VG_PRIM_BEZIER_CUBIC,
                        NEMA_VG_PRIM_BEZIER_CUBIC,
                        NEMA_VG_PRIM_CLOSE
                        };

    NEMA_VG_PATH_HANDLE path = nema_vg_path_create();
    nema_vg_path_clear(path);
    nema_vg_path_set_shape(path, 11L, cmds, 52L, coords);
    if (m != NULL){
        nema_vg_path_set_matrix(path, m);
    }

    nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);

    uint32_t error = nema_vg_draw_path(path, paint);
    nema_vg_path_destroy(path);

    nema_vg_set_fill_rule(NEMA_VG_STROKE);

    return error;
}

static void set_quad_clip2(int quad, int supports_clip2, nema_vg_paint_t *paint)
{
    uint32_t max_width;
    if (supports_clip2 != 0){
        max_width = 0x7FFFU;
    }else{
         nema_vg_stencil_t_ *stencil_obj = nema_vg_get_stencil();
         max_width = (uint32_t)stencil_obj->width;
    }

    switch(quad)
    {
        case 0:
        {
            int32_t clip_center_x = (int32_t)paint->grad.cx;
            int32_t clip_center_y = (int32_t)paint->grad.cy;
            uint32_t clip_width = max_width - (uint32_t)(clip_center_x);
            uint32_t clip_height = max_width - (uint32_t)(clip_center_y);
            set_clip2_or_temp(supports_clip2, clip_center_x, clip_center_y,
                            clip_width, clip_height);
            if (supports_clip2 == 0){
                init_clip(clip_center_x, clip_center_y, (int32_t)clip_width, (int32_t)clip_height);
            }
            break;
        }
        case 1:
        {
            int32_t clip_center_x = (int32_t)0;
            int32_t clip_center_y = (int32_t)paint->grad.cy;
            uint32_t clip_width = (uint32_t)(paint->grad.cx);
            uint32_t clip_height = max_width - (uint32_t)(clip_center_y);
            set_clip2_or_temp(supports_clip2, clip_center_x, clip_center_y,
                            clip_width, clip_height);
            if (supports_clip2 == 0){
                init_clip(clip_center_x, clip_center_y, (int32_t)clip_width, (int32_t)clip_height);
            }
            break;
        }
        case 2:
        {
            int32_t clip_center_x = (int32_t)0;
            int32_t clip_center_y = (int32_t)0;
            uint32_t clip_width = (uint32_t)(paint->grad.cx);
            uint32_t clip_height = (uint32_t)(paint->grad.cy);
            set_clip2_or_temp(supports_clip2, clip_center_x, clip_center_y,
                            clip_width, clip_height);
            if (supports_clip2 == 0){
                init_clip(clip_center_x, clip_center_y, (int32_t)clip_width, (int32_t)clip_height);
            }
            break;
        }
        case 3:
        {
            int32_t clip_center_x = (int32_t)paint->grad.cx;
            int32_t clip_center_y = (int32_t)0;
            uint32_t clip_width = max_width - (uint32_t)(clip_center_x);
            uint32_t clip_height = (uint32_t)(paint->grad.cy);
            set_clip2_or_temp(supports_clip2, clip_center_x, clip_center_y,
                            clip_width, clip_height);
            if (supports_clip2 == 0){
                init_clip(clip_center_x, clip_center_y, (int32_t)clip_width, (int32_t)clip_height);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

uint32_t nema_vg_draw_line(float x1, float y1, float x2, float y2,
                            nema_matrix3x3_t m,
                            NEMA_VG_PAINT_HANDLE paint)
{
    uint32_t error = NEMA_VG_ERR_NO_ERROR;
    float x1_ = x1;
    float x2_ = x2;
    float y1_ = y1;
    float y2_ = y2;

    GET_PAINT;
    NEMA_VG_IF_ERROR(check_shapes(_paint));

    GET_CONTEXT;

    uint32_t blend = nemavg_context->blend;

    uint32_t paint_color = _paint->paint_color;

    uint8_t pre_fill_rule = nemavg_context->fill_rule;
    nema_vg_set_fill_rule(NEMA_VG_STROKE);

    uint8_t has_transformation = (uint8_t)(nemavg_context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION);
    if (m != NULL){
        has_transformation = 1U;
    }

    nema_matrix3x3_t shape_plus_global_m;
    if (has_transformation != 0U){

        nema_mat3x3_load_identity(shape_plus_global_m);

        if ( (nemavg_context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION   ) != 0U ) {
            nema_mat3x3_mul(shape_plus_global_m, nemavg_context->global_m);
        }

        if (m != NULL){
            nema_mat3x3_mul(shape_plus_global_m, m);
        }
    }

    // check for out of range
    {
        nema_vg_vertex_t_ bbox[2] = {
            {x1_, y1_},
            {x2_, y2_},
        };

        float min_coord, max_coord;
        nema_vg_get_coord_limits(&min_coord, &max_coord);

        int with_path = 0;
        if ( (nemavg_context->flags & NEMA_VG_CONTEXT_ENABLE_HANDLE_LARGE_COORDS) != 0U){
            if (has_transformation != 0U){
                nema_mat3x3_mul_vec(shape_plus_global_m, &bbox[0].x, &bbox[0].y);
                nema_mat3x3_mul_vec(shape_plus_global_m, &bbox[1].x, &bbox[1].y);
            }

            for (int i=0; i<2; ++i){
                // Check for out of limit bbox
                if ( (bbox[i].x > max_coord || bbox[i].x < min_coord) ||
                     (bbox[i].y > max_coord || bbox[i].y < min_coord)
                ) {
                    with_path = 1;
                    break;
                }
            }
        }
        if((nemavg_context->stroke.start_cap_style != NEMA_VG_CAP_BUTT) || nemavg_context->stroke.end_cap_style != NEMA_VG_CAP_BUTT)
        {
            with_path = 1;
        }

        if (with_path == 1) {

            float coords[4] = {x1_, y1_, x2_, y2_};

            uint8_t cmds[2] = {NEMA_VG_PRIM_MOVE,
                                NEMA_VG_PRIM_LINE,
                                };

            NEMA_VG_PATH_HANDLE path = nema_vg_path_create();
            nema_vg_path_clear(path);
            nema_vg_path_set_shape(path, 2L, cmds, 4L, coords);
            if (m != NULL){
                nema_vg_path_set_matrix(path, m);
            }

            error = nema_vg_draw_path(path, paint);
            nema_vg_path_destroy(path);

            nema_vg_set_fill_rule(pre_fill_rule);

            return error;
        }
    }

    if ( nema_context.en_tscFB == 1U ) {
        nemavg_context->aa_flags |= (nema_context.surface_tile & RAST_TILE_MASK);
    }

    if (has_transformation != 0U){
        nema_mat3x3_mul_vec(shape_plus_global_m, &x1_, &y1_);
        nema_mat3x3_mul_vec(shape_plus_global_m, &x2_, &y2_);
    }

    if (has_transformation != 0U){
        paint_color = set_stroked_line_width(nemavg_context->stroke.width, shape_plus_global_m, paint_color);
    }else{
        paint_color = set_stroked_line_width(nemavg_context->stroke.width, NULL, paint_color);
    }

    (void)shape_bind_blend(_paint, blend, paint_color);

#ifndef NEMAPVG
    enable_do_not_transform(1);
#endif //NEMAPVG

    nema_vg_vertex_t_ p1 = {x1_, y1_};
    nema_vg_vertex_t_ p2 = {x2_, y2_};
    if (_paint->type != NEMA_VG_PAINT_GRAD_CONICAL){
        set_matrix_according_paint(_paint, m, 0);
        raster_stroked_line_(p1, p2, 0);
        flush_lines_(0, 0);
    }else{
        int supports_clip2 = nema_supports_clip2();

        for (int i=0; i<4; ++i){
            set_quad_clip2(i, supports_clip2, _paint);
            set_matrix_according_paint(_paint, m, i);

            raster_stroked_line_(p1, p2, 0);
            flush_lines_(0, 0);

            reset_clip2_or_temp(supports_clip2);
        }
    }

    draw_caps();

    nema_vg_set_fill_rule(pre_fill_rule);
#ifndef NEMAPVG
    enable_do_not_transform(0U);
#endif //NEMAPVG

    if ( nema_context.en_tscFB == 1U ) {
        nemavg_context->aa_flags &= ~(nema_context.surface_tile & RAST_TILE_MASK);
    }

    return error;
}

uint32_t nema_vg_draw_rect(float x, float y, float width, float height,
                                nema_matrix3x3_t m,
                                NEMA_VG_PAINT_HANDLE paint)
{
    uint32_t error = NEMA_VG_ERR_NO_ERROR;

    GET_CONTEXT;

    GET_PAINT;
    NEMA_VG_IF_ERROR(check_shapes(_paint));

    uint32_t blend = nemavg_context->blend;
    uint32_t paint_color = _paint->paint_color;

    // the 4 corners of the rect.
    // we'll use the raster_quad instead of the raster_rect
    // because the transformation matrix might have rotation
    // or shear
    nema_vg_vertex_t_ p1, p2, p3, p4;
    p1.x = x;
    p1.y = y;
    p2.x = x + width;
    p2.y = y;
    p3.x = x + width;
    p3.y = y + height;
    p4.x = x;
    p4.y = y + height;

    int with_path = 0;
    if ((nemavg_context->quality & NEMA_VG_QUALITY_MAXIMUM) == NEMA_VG_QUALITY_MAXIMUM &&
        nemavg_context->fill_rule != NEMA_VG_STROKE){
        with_path = 1;
    }

    rect_points points;
    if (nemavg_context->fill_rule == NEMA_VG_STROKE){

        nema_vg_vertex_t_ p12_dir = {p2.x - p1.x, p2.y - p1.y};
        nema_vg_vertex_t_ p23_dir = {p3.x - p2.x, p3.y - p2.y};
        nema_vg_vertex_t_ p34_dir = {p4.x - p3.x, p4.y - p3.y};
        nema_vg_vertex_t_ p41_dir = {p1.x - p4.x, p1.y - p4.y};

        p12_dir.x /= width;
        p12_dir.y /= width;
        p23_dir.x /= height;
        p23_dir.y /= height;
        p34_dir.x /= width;
        p34_dir.y /= width;
        p41_dir.x /= height;
        p41_dir.y /= height;

        nema_vg_vertex_t_ p12_normal = {-p12_dir.y, p12_dir.x};
        nema_vg_vertex_t_ p23_normal = {-p23_dir.y, p23_dir.x};
        nema_vg_vertex_t_ p34_normal = {-p34_dir.y, p34_dir.x};
        nema_vg_vertex_t_ p41_normal = {-p41_dir.y, p41_dir.x};

        float stroke_w_x = nemavg_context->stroke.width * 0.5f;
        float stroke_w_y = stroke_w_x;

        p12_normal.x = p12_normal.x * stroke_w_x;
        p12_normal.y = p12_normal.y * stroke_w_y;
        p23_normal.x = p23_normal.x * stroke_w_x;
        p23_normal.y = p23_normal.y * stroke_w_y;
        p34_normal.x = p34_normal.x * stroke_w_x;
        p34_normal.y = p34_normal.y * stroke_w_y;
        p41_normal.x = p41_normal.x * stroke_w_x;
        p41_normal.y = p41_normal.y * stroke_w_y;

        points.p1_up_left.x = p1.x + (-p12_normal.x - p41_normal.x);
        points.p1_up_left.y = p1.y + (-p12_normal.y - p41_normal.y);
        points.p1_down_right.x = p1.x + (p12_normal.x + p41_normal.x);
        points.p1_down_right.y = p1.y + (p12_normal.y + p41_normal.y);

        points.p2_up_right.x = p2.x + (-p12_normal.x - p23_normal.x);
        points.p2_up_right.y = p2.y + (-p12_normal.y - p23_normal.y);
        points.p2_down_left.x = p2.x + (p12_normal.x + p23_normal.x);
        points.p2_down_left.y = p2.y + (p12_normal.y + p23_normal.y);

        points.p3_up_left.x = p3.x + (p34_normal.x + p23_normal.x);
        points.p3_up_left.y = p3.y + (p34_normal.y + p23_normal.y);
        points.p3_down_right.x = p3.x + (-p34_normal.x - p23_normal.x);
        points.p3_down_right.y = p3.y + (-p34_normal.y - p23_normal.y);

        points.p4_up_right.x = p4.x + (p34_normal.x + p41_normal.x);
        points.p4_up_right.y = p4.y + (p34_normal.y + p41_normal.y);
        points.p4_down_left.x = p4.x + (-p34_normal.x - p41_normal.x);
        points.p4_down_left.y = p4.y + (-p34_normal.y - p41_normal.y);
    }else{
        points.p1_up_left  = p1;
        points.p2_up_right = p2;
        points.p3_up_left  = p3;
        points.p4_up_right = p4;
    }

    uint8_t has_transformation = (uint8_t)(nemavg_context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION);
    if (m != NULL){
        has_transformation = 1U;
    }

    nema_matrix3x3_t shape_plus_global_m;
    if (has_transformation != 0U){
        nema_mat3x3_load_identity(shape_plus_global_m);

        if ( (nemavg_context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION   ) != 0U ) {
            nema_mat3x3_mul(shape_plus_global_m, nemavg_context->global_m);
        }

        if (m != NULL){
            nema_mat3x3_mul(shape_plus_global_m, m);
        }
    }

    // create the bbox to check for out of range coords
    nema_vg_vertex_t_ bbox[4] = {
        {x, y},
        {x + width, y},
        {x + width, y + height},
        {x, y + height}
    };
    if (nemavg_context->fill_rule == NEMA_VG_STROKE){
        float half_stroke = nemavg_context->stroke.width * 0.5f;
        bbox[0].x -= half_stroke;
        bbox[0].y -= half_stroke;
        bbox[1].x += half_stroke;
        bbox[1].y -= half_stroke;
        bbox[2].x += half_stroke;
        bbox[2].y += half_stroke;
        bbox[3].x -= half_stroke;
        bbox[3].y += half_stroke;
    }

    uint8_t out_of_range = 0U;
    if (has_transformation != 0U){
        out_of_range = should_handle_large_coords(bbox, shape_plus_global_m);
    } else {
        out_of_range = should_handle_large_coords(bbox, NULL);
    }
    if (out_of_range == 1U){
        with_path = 1;
    }

    if (with_path == 1) {
        NEMA_VG_PATH_HANDLE path = nema_vg_path_create();
        nema_vg_path_clear(path);

        uint8_t pre_fill_rule = nemavg_context->fill_rule;

        if (m != NULL){
            nema_vg_path_set_matrix(path, m);
        }

        if (nemavg_context->fill_rule == NEMA_VG_STROKE){

            float coords[16] = {points.p1_up_left.x, points.p1_up_left.y,//move
                                points.p2_up_right.x, points.p2_up_right.y,
                                points.p3_down_right.x, points.p3_down_right.y,
                                points.p4_down_left.x, points.p4_down_left.y,
                                points.p1_down_right.x, points.p1_down_right.y, // move
                                points.p4_up_right.x, points.p4_up_right.y,
                                points.p3_up_left.x, points.p3_up_left.y,
                                points.p2_down_left.x, points.p2_down_left.y,
                            };

            uint8_t cmds[10] = {NEMA_VG_PRIM_MOVE,
                                NEMA_VG_PRIM_LINE,
                                NEMA_VG_PRIM_LINE,
                                NEMA_VG_PRIM_LINE,
                                NEMA_VG_PRIM_CLOSE,
                                NEMA_VG_PRIM_MOVE,
                                NEMA_VG_PRIM_LINE,
                                NEMA_VG_PRIM_LINE,
                                NEMA_VG_PRIM_LINE,
                                NEMA_VG_PRIM_CLOSE,
                                };

            nema_vg_path_set_shape(path, 10L, cmds, 16L, coords);

            nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);

            error = nema_vg_draw_path(path, paint);
        } else {
            float coords[8] = {points.p1_up_left.x, points.p1_up_left.y,//move
                                points.p2_up_right.x, points.p2_up_right.y,
                                points.p3_up_left.x, points.p3_up_left.y,
                                points.p4_up_right.x, points.p4_up_right.y,
                            };

            uint8_t cmds[5] = {NEMA_VG_PRIM_MOVE,
                                NEMA_VG_PRIM_LINE,
                                NEMA_VG_PRIM_LINE,
                                NEMA_VG_PRIM_LINE,
                                NEMA_VG_PRIM_CLOSE,
                                };

            nema_vg_path_set_shape(path, 5L, cmds, 8L, coords);

            error = nema_vg_draw_path(path, paint);
        }

        nema_vg_set_fill_rule(pre_fill_rule);

        nema_vg_path_destroy(path);
    } else {

        if ( nema_context.en_tscFB == 1U ) {
            nemavg_context->aa_flags |= (nema_context.surface_tile & RAST_TILE_MASK);
        }

#ifndef NEMAPVG
        enable_do_not_transform(1U);
#endif //NEMAPVG
        //Stroked rect is drawn a bit differently.
        // Call set_stroked_line_width to get a modulated color
        if (nemavg_context->fill_rule == NEMA_VG_STROKE){
            if (has_transformation != 0U){
                paint_color = set_stroked_line_width(nemavg_context->stroke.width, shape_plus_global_m, paint_color);
            }else{
                paint_color = set_stroked_line_width(nemavg_context->stroke.width, NULL, paint_color);
            }
        }

        if (has_transformation != 0U){

            if (nemavg_context->fill_rule == NEMA_VG_STROKE){
                nema_mat3x3_mul_vec(shape_plus_global_m, &points.p1_up_left.x, &points.p1_up_left.y);
                nema_mat3x3_mul_vec(shape_plus_global_m, &points.p1_down_right.x, &points.p1_down_right.y);

                nema_mat3x3_mul_vec(shape_plus_global_m, &points.p2_up_right.x, &points.p2_up_right.y);
                nema_mat3x3_mul_vec(shape_plus_global_m, &points.p2_down_left.x, &points.p2_down_left.y);

                nema_mat3x3_mul_vec(shape_plus_global_m, &points.p3_up_left.x, &points.p3_up_left.y);
                nema_mat3x3_mul_vec(shape_plus_global_m, &points.p3_down_right.x, &points.p3_down_right.y);

                nema_mat3x3_mul_vec(shape_plus_global_m, &points.p4_up_right.x, &points.p4_up_right.y);
                nema_mat3x3_mul_vec(shape_plus_global_m, &points.p4_down_left.x, &points.p4_down_left.y);
            } else {
                nema_mat3x3_mul_vec(shape_plus_global_m, &points.p1_up_left.x, &points.p1_up_left.y);
                nema_mat3x3_mul_vec(shape_plus_global_m, &points.p2_up_right.x, &points.p2_up_right.y);
                nema_mat3x3_mul_vec(shape_plus_global_m, &points.p3_up_left.x, &points.p3_up_left.y);
                nema_mat3x3_mul_vec(shape_plus_global_m, &points.p4_up_right.x, &points.p4_up_right.y);
            }
        }

        (void)shape_bind_blend(_paint, blend, paint_color);

        if (_paint->type != NEMA_VG_PAINT_GRAD_CONICAL){
            set_matrix_according_paint(_paint, m, 0);
            draw_rect_geometry(&points);
        }else{
            int supports_clip2 = nema_supports_clip2();

            for (int i=0; i<4; ++i){
                set_quad_clip2(i, supports_clip2, _paint);
                set_matrix_according_paint(_paint, m, i);

                draw_rect_geometry(&points);

                reset_clip2_or_temp(supports_clip2);
            }
        }
#ifndef NEMAPVG
        enable_do_not_transform(0U);
#endif //NEMAPVG

        if ( nema_context.en_tscFB == 1U ) {
            nemavg_context->aa_flags &= ~(nema_context.surface_tile & RAST_TILE_MASK);
        }
    }

    return error;
}

uint32_t nema_vg_draw_rounded_rect(float x, float y, float width, float height,
                                float rx, float ry,
                                nema_matrix3x3_t m,
                                NEMA_VG_PAINT_HANDLE paint)
{
    float rx_ = rx;
    float ry_ = ry;
    uint32_t error = NEMA_VG_ERR_NO_ERROR;

    if ( (width * 0.5f) < rx_) {
        rx_ = width * 0.5f;
    }
    if ( (height * 0.5f) < ry_) {
        ry_ = height * 0.5f;
    }

    if ( (rx_ <= 0.0f) || (ry_ <= 0.0f) ) {
        return nema_vg_draw_rect(x, y, width, height, m, paint);
    }

    GET_CONTEXT;
    GET_PAINT;
    NEMA_VG_IF_ERROR(check_shapes(_paint));

    uint8_t pre_fill_rule = nemavg_context->fill_rule;
    if (pre_fill_rule != NEMA_VG_STROKE){
        nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
    }

    uint32_t blend = nemavg_context->blend;
    uint32_t paint_color = _paint->paint_color;

    set_bezier_quality(nemavg_context->quality);

    uint8_t has_transformation = (uint8_t)(nemavg_context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION);
    if (m != NULL){
        has_transformation = 1U;
    }

    nema_matrix3x3_t shape_plus_global_m;
    if (has_transformation != 0U){
        nema_mat3x3_load_identity(shape_plus_global_m);

        if ( (nemavg_context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION   ) != 0U ) {
            nema_mat3x3_mul(shape_plus_global_m, nemavg_context->global_m);
        }

        if (m != NULL){
            nema_mat3x3_mul(shape_plus_global_m, m);
        }
    }

    rounded_rect_points points;

    // we'll draw 4 bezier curves for the rounded corners and then
    // connect the straight lines
    float k = 0.55228f;

    // First calculate the centers of the beziers
    points.AH.x = x + (rx_)               ; points.AH.y = y + (ry_);
    points.BC.x = x + (width - rx_)       ; points.BC.y = y + (ry_);
    points.DE.x = x + (width - rx_)       ; points.DE.y = y + (height - ry_);
    points.FG.x = x + (rx_)               ; points.FG.y = y + (height - ry_);

    // calculate the points
    points.A.x = 0.0f                                       ; points.A.y = -1.0f;
    points.B.x = 0.0f                                       ; points.B.y = -1.0f;
    points.C.x = 1.0f                                       ; points.C.y = 0.0f;
    points.D.x = 1.0f                                       ; points.D.y = 0.0f;
    points.E.x = 0.0f                                       ; points.E.y = 1.0f;
    points.F.x = 0.0f                                       ; points.F.y = 1.0f;
    points.G.x = -1.0f                                      ; points.G.y = 0.0f;
    points.H.x = -1.0f                                      ; points.H.y = 0.0f;

    points.A.x = points.A.x * rx_ + points.AH.x              ; points.A.y = points.A.y * ry_ + points.AH.y;
    points.B.x = points.B.x * rx_ + points.BC.x              ; points.B.y = points.B.y * ry_ + points.BC.y;
    points.C.x = points.C.x * rx_ + points.BC.x              ; points.C.y = points.C.y * ry_ + points.BC.y;
    points.D.x = points.D.x * rx_ + points.DE.x              ; points.D.y = points.D.y * ry_ + points.DE.y;
    points.E.x = points.E.x * rx_ + points.DE.x              ; points.E.y = points.E.y * ry_ + points.DE.y;
    points.F.x = points.F.x * rx_ + points.FG.x              ; points.F.y = points.F.y * ry_ + points.FG.y;
    points.G.x = points.G.x * rx_ + points.FG.x              ; points.G.y = points.G.y * ry_ + points.FG.y;
    points.H.x = points.H.x * rx_ + points.AH.x              ; points.H.y = points.H.y * ry_ + points.AH.y;

    // calculate the control points
    points.AH_ctrl0.x = -1.0f                               ; points.AH_ctrl0.y = k * (-1.0f);
    points.AH_ctrl1.x = k * (-1.0f)                         ; points.AH_ctrl1.y = -1.0f;
    points.BC_ctrl0.x = k * 1.0f                            ; points.BC_ctrl0.y = -1.0f;
    points.BC_ctrl1.x = 1.0f                                ; points.BC_ctrl1.y = k * (-1.0f);
    points.DE_ctrl0.x = 1.0f                                ; points.DE_ctrl0.y = k * 1.0f;
    points.DE_ctrl1.x = k * 1.0f                            ; points.DE_ctrl1.y = 1.0f;
    points.FG_ctrl0.x = k * (-1.0f)                         ; points.FG_ctrl0.y = 1.0f;
    points.FG_ctrl1.x = -1.0f                               ; points.FG_ctrl1.y = k * 1.0f;

    points.AH_ctrl0.x = points.AH_ctrl0.x * rx_ + points.AH.x; points.AH_ctrl0.y = points.AH_ctrl0.y * ry_ + points.AH.y;
    points.AH_ctrl1.x = points.AH_ctrl1.x * rx_ + points.AH.x; points.AH_ctrl1.y = points.AH_ctrl1.y * ry_ + points.AH.y;
    points.BC_ctrl0.x = points.BC_ctrl0.x * rx_ + points.BC.x; points.BC_ctrl0.y = points.BC_ctrl0.y * ry_ + points.BC.y;
    points.BC_ctrl1.x = points.BC_ctrl1.x * rx_ + points.BC.x; points.BC_ctrl1.y = points.BC_ctrl1.y * ry_ + points.BC.y;
    points.DE_ctrl0.x = points.DE_ctrl0.x * rx_ + points.DE.x; points.DE_ctrl0.y = points.DE_ctrl0.y * ry_ + points.DE.y;
    points.DE_ctrl1.x = points.DE_ctrl1.x * rx_ + points.DE.x; points.DE_ctrl1.y = points.DE_ctrl1.y * ry_ + points.DE.y;
    points.FG_ctrl0.x = points.FG_ctrl0.x * rx_ + points.FG.x; points.FG_ctrl0.y = points.FG_ctrl0.y * ry_ + points.FG.y;
    points.FG_ctrl1.x = points.FG_ctrl1.x * rx_ + points.FG.x; points.FG_ctrl1.y = points.FG_ctrl1.y * ry_ + points.FG.y;


    int with_path = should_draw_path(_paint, blend);

    nema_vg_vertex_t_ bbox[4] = {
        {x, y},
        {x + width, y},
        {x + width, y + height},
        {x, y + height}
    };
    if (nemavg_context->fill_rule == NEMA_VG_STROKE){
        float half_stroke = nemavg_context->stroke.width * 0.5f;
        bbox[0].x -= half_stroke;
        bbox[0].y -= half_stroke;
        bbox[1].x += half_stroke;
        bbox[1].y -= half_stroke;
        bbox[2].x += half_stroke;
        bbox[2].y += half_stroke;
        bbox[3].x -= half_stroke;
        bbox[3].y += half_stroke;
    }

    uint8_t out_of_range = 0;
    if (has_transformation != 0U){
        out_of_range = should_handle_large_coords(bbox, shape_plus_global_m);
    } else {
        out_of_range = should_handle_large_coords(bbox, NULL);
    }
    if (out_of_range == 1U){
        with_path = 1;
    }

    //if RASTERIZER BUG force drawing with stencil
#ifdef RASTERIZER_BUG_WA
    with_path = 1;
#endif

    if (with_path == 1){

        float coords[34] = {points.A.x, points.A.y, //move

                            points.B.x, points.B.y, // line
                            points.BC_ctrl0.x, points.BC_ctrl0.y, //cubic ctrl 0
                            points.BC_ctrl1.x, points.BC_ctrl1.y, //cubic ctrl 1
                            points.C.x, points.C.y, // cubic p2

                            points.D.x, points.D.y, // line
                            points.DE_ctrl0.x, points.DE_ctrl0.y, //cubic ctrl 0
                            points.DE_ctrl1.x, points.DE_ctrl1.y, //cubic ctrl 1
                            points.E.x, points.E.y, // cubic p2

                            points.F.x, points.F.y, // line
                            points.FG_ctrl0.x, points.FG_ctrl0.y, //cubic ctrl 0
                            points.FG_ctrl1.x, points.FG_ctrl1.y, //cubic ctrl 1
                            points.G.x, points.G.y, // cubic p2

                            points.H.x, points.H.y, // line
                            points.AH_ctrl0.x, points.AH_ctrl0.y, //cubic ctrl 0
                            points.AH_ctrl1.x, points.AH_ctrl1.y, //cubic ctrl 1
                            points.A.x, points.A.y, // cubic p2
                        };

        uint8_t cmds[10] = {NEMA_VG_PRIM_MOVE,
                            NEMA_VG_PRIM_LINE,
                            NEMA_VG_PRIM_BEZIER_CUBIC,
                            NEMA_VG_PRIM_LINE,
                            NEMA_VG_PRIM_BEZIER_CUBIC,
                            NEMA_VG_PRIM_LINE,
                            NEMA_VG_PRIM_BEZIER_CUBIC,
                            NEMA_VG_PRIM_LINE,
                            NEMA_VG_PRIM_BEZIER_CUBIC,
                            NEMA_VG_PRIM_CLOSE
                            };

        NEMA_VG_PATH_HANDLE path = nema_vg_path_create();
        nema_vg_path_clear(path);
        nema_vg_path_set_shape(path, 10L, cmds, 34L, coords);
        if (m != NULL){
            nema_vg_path_set_matrix(path, m);
        }

        error = nema_vg_draw_path(path, paint);
        nema_vg_path_destroy(path);

        return error;
    }

#ifndef NEMAPVG
    enable_do_not_transform(1);
#endif //NEMAPVG


    if (nemavg_context->fill_rule == NEMA_VG_STROKE){
        if (has_transformation != 0U){
            paint_color = set_stroked_line_width(nemavg_context->stroke.width, shape_plus_global_m, paint_color);
        }else{
            paint_color = set_stroked_line_width(nemavg_context->stroke.width, NULL, paint_color);
        }
    }

    if ( nema_context.en_tscFB == 1U ) {
        nemavg_context->aa_flags |= (nema_context.surface_tile & RAST_TILE_MASK);
    }

    if (has_transformation != 0U){
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.AH.x, &points.AH.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.BC.x, &points.BC.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.DE.x, &points.DE.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.FG.x, &points.FG.y);

        nema_mat3x3_mul_vec(shape_plus_global_m, &points.A.x, &points.A.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.B.x, &points.B.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.C.x, &points.C.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.D.x, &points.D.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.E.x, &points.E.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.F.x, &points.F.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.G.x, &points.G.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.H.x, &points.H.y);

        nema_mat3x3_mul_vec(shape_plus_global_m, &points.AH_ctrl0.x, &points.AH_ctrl0.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.AH_ctrl1.x, &points.AH_ctrl1.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.BC_ctrl0.x, &points.BC_ctrl0.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.BC_ctrl1.x, &points.BC_ctrl1.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.DE_ctrl0.x, &points.DE_ctrl0.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.DE_ctrl1.x, &points.DE_ctrl1.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.FG_ctrl0.x, &points.FG_ctrl0.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.FG_ctrl1.x, &points.FG_ctrl1.y);
    }

    (void)shape_bind_blend(_paint, blend, paint_color);

    if (_paint->type != NEMA_VG_PAINT_GRAD_CONICAL){
        init_clip_from_context();

        set_matrix_according_paint(_paint, m, 0);
        draw_rounded_rect_geometry(&points);
    }else{
        int supports_clip2 = nema_supports_clip2();

        for (int i=0; i<4; ++i){
            set_quad_clip2(i, supports_clip2, _paint);
            set_matrix_according_paint(_paint, m, i);

            draw_rounded_rect_geometry(&points);

            reset_clip2_or_temp(supports_clip2);
        }
    }

    nema_vg_set_fill_rule(pre_fill_rule);
#ifndef NEMAPVG
    enable_do_not_transform(0U);
#endif //NEMAPVG

    if ( nema_context.en_tscFB == 1U ) {
        nemavg_context->aa_flags &= ~(nema_context.surface_tile & RAST_TILE_MASK);
    }

    return error;
}

static uint32_t
draw_ellipse(ellipse_points* points)
{
    nema_vg_info_t_ path_info;

    path_info.p1 = points->right_point;
    path_info.ctrl0 = points->right_down_ctrl0_point;
    path_info.ctrl1 = points->right_down_ctrl1_point;
    path_info.p2 = points->down_point;
    NEMA_VG_IF_ERROR(fill_cubic(&path_info));

    path_info.p1 = points->down_point;
    path_info.ctrl0 = points->down_left_ctrl0_point;
    path_info.ctrl1 = points->down_left_ctrl1_point;
    path_info.p2 = points->left_point;
    NEMA_VG_IF_ERROR(fill_cubic(&path_info));

    path_info.p1 = points->left_point;
    path_info.ctrl0 = points->left_up_ctrl0_point;
    path_info.ctrl1 = points->left_up_ctrl1_point;
    path_info.p2 = points->up_point;
    NEMA_VG_IF_ERROR(fill_cubic(&path_info));

    path_info.p1 = points->up_point;
    path_info.ctrl0 = points->up_right_ctrl0_point;
    path_info.ctrl1 = points->up_right_ctrl1_point;
    path_info.p2 = points->right_point;
    NEMA_VG_IF_ERROR(fill_cubic(&path_info));

    return NEMA_VG_ERR_NO_ERROR;
}


uint32_t nema_vg_draw_ellipse(float cx, float cy, float rx, float ry,
                            nema_matrix3x3_t m,
                            NEMA_VG_PAINT_HANDLE paint)
{
    uint32_t error = NEMA_VG_ERR_NO_ERROR;

    GET_PAINT;
    NEMA_VG_IF_ERROR(check_shapes(_paint));

    GET_CONTEXT;

    uint8_t pre_fill_rule = nemavg_context->fill_rule;
    if (pre_fill_rule != NEMA_VG_STROKE){
        nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
    }

    uint32_t blend = nemavg_context->blend;
    uint32_t paint_color = _paint->paint_color;

    set_bezier_quality(nemavg_context->quality);

    uint8_t has_transformation = (uint8_t)(nemavg_context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION);
    if (m != NULL){
        has_transformation = 1U;
    }

    nema_matrix3x3_t shape_plus_global_m;
    if (has_transformation != 0U){
        nema_mat3x3_load_identity(shape_plus_global_m);

        if ( (nemavg_context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION   ) != 0U ) {
            nema_mat3x3_mul(shape_plus_global_m, nemavg_context->global_m);
        }

        if (m != NULL){
            nema_mat3x3_mul(shape_plus_global_m, m);
        }
    }

    ellipse_points points;
    create_ellipse_curves(cx, cy, rx, ry, &points);

    int with_path = should_draw_path(_paint, blend);

    nema_vg_vertex_t_ bbox[4] = {
        {points.left_point.x, points.up_point.y},
        {points.right_point.x, points.up_point.y},
        {points.right_point.x, points.down_point.y},
        {points.left_point.x, points.down_point.y}
    };
    if (nemavg_context->fill_rule == NEMA_VG_STROKE){
        float half_stroke = nemavg_context->stroke.width * 0.5f;
        bbox[0].x -= half_stroke;
        bbox[0].y -= half_stroke;
        bbox[1].x += half_stroke;
        bbox[1].y -= half_stroke;
        bbox[2].x += half_stroke;
        bbox[2].y += half_stroke;
        bbox[3].x -= half_stroke;
        bbox[3].y += half_stroke;
    }

    uint8_t out_of_range = 0;
    if (has_transformation != 0U){
        out_of_range = should_handle_large_coords(bbox, shape_plus_global_m);
    } else {
        out_of_range = should_handle_large_coords(bbox, NULL);
    }
    if (out_of_range == 1U){
        with_path = 1;
    }

    //if RASTERIZER BUG force drawing with stencil
#ifdef RASTERIZER_BUG_WA
    with_path = 1;
#endif

    if (with_path == 1){
        return draw_ellipse_path(&points, _paint, m);
    }

#ifndef NEMAPVG
    enable_do_not_transform(1);
#endif //NEMAPVG

    if ( nema_context.en_tscFB == 1U ) {
        nemavg_context->aa_flags |= (nema_context.surface_tile & RAST_TILE_MASK);
    }

    if (has_transformation != 0U){
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.right_point.x, &points.right_point.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.right_down_ctrl0_point.x, &points.right_down_ctrl0_point.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.right_down_ctrl1_point.x, &points.right_down_ctrl1_point.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.down_point.x, &points.down_point.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.down_left_ctrl0_point.x, &points.down_left_ctrl0_point.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.down_left_ctrl1_point.x, &points.down_left_ctrl1_point.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.left_point.x, &points.left_point.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.left_up_ctrl0_point.x, &points.left_up_ctrl0_point.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.left_up_ctrl1_point.x, &points.left_up_ctrl1_point.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.up_point.x, &points.up_point.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.up_right_ctrl0_point.x, &points.up_right_ctrl0_point.y);
        nema_mat3x3_mul_vec(shape_plus_global_m, &points.up_right_ctrl1_point.x, &points.up_right_ctrl1_point.y);
    }

    enable_draw_on_fb(1);
    enable_use_bevel_joins(1);

    set_matrix_according_paint(_paint, m, 0);

    // Set up the center
    nema_vg_vertex_t_ bezier_center = {cx, cy};
    if (has_transformation != 0U){
        nema_mat3x3_mul_vec(shape_plus_global_m, &bezier_center.x, &bezier_center.y);
    }
    int cix = vertex_2fx(bezier_center.x);
    int ciy = vertex_2fx(bezier_center.y);
    set_raster_point(4, cix, ciy);
    set_fan_center(bezier_center.x, bezier_center.y);

    reset_last_vertex();

    if (nemavg_context->fill_rule == NEMA_VG_STROKE){
        if (has_transformation != 0U){
            paint_color = set_stroked_line_width(nemavg_context->stroke.width, shape_plus_global_m, paint_color);
        }else{
            paint_color = set_stroked_line_width(nemavg_context->stroke.width, NULL, paint_color);
        }
    }

    (void)shape_bind_blend(_paint, blend, paint_color);

    if (_paint->type != NEMA_VG_PAINT_GRAD_CONICAL){
        init_clip_from_context();

        NEMA_VG_IF_ERROR(draw_ellipse(&points));
        if (nemavg_context->fill_rule == NEMA_VG_STROKE){
            flush_lines_(1, 0);
        }
    }else{
        int supports_clip2 = nema_supports_clip2();

        for (int i=0; i<4; ++i){
            set_quad_clip2(i, supports_clip2, _paint);
            set_matrix_according_paint(_paint, m, i);
            NEMA_VG_IF_ERROR(draw_ellipse(&points));
            if (nemavg_context->fill_rule == NEMA_VG_STROKE){
                flush_lines_(1, 0);
            }

        }
        reset_clip2_or_temp(supports_clip2);
    }

    nema_vg_set_fill_rule(pre_fill_rule);
    enable_draw_on_fb(0);
    enable_use_bevel_joins(0);

#ifndef NEMAPVG
    enable_do_not_transform(0U);
#endif //NEMAPVG

    if ( nema_context.en_tscFB == 1U ) {
        nemavg_context->aa_flags &= ~(nema_context.surface_tile & RAST_TILE_MASK);
    }
    return error;
}

uint32_t nema_vg_draw_circle(float cx, float cy, float r, nema_matrix3x3_t m, NEMA_VG_PAINT_HANDLE paint)
{
    float cx_ = cx;
    float cy_ = cy;

    uint32_t error = NEMA_VG_ERR_NO_ERROR;

    GET_PAINT;
    NEMA_VG_IF_ERROR(check_shapes(_paint));

    GET_CONTEXT;

    uint32_t blend = nemavg_context->blend;

    uint8_t has_transformation = (uint8_t)(nemavg_context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION);
    if (m != NULL){
        has_transformation = 1U;
    }

    nema_matrix3x3_t shape_plus_global_m = {0};
    if (has_transformation != 0U){
        nema_mat3x3_load_identity(shape_plus_global_m);

        if ( (nemavg_context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION   ) != 0U ) {
            nema_mat3x3_mul(shape_plus_global_m, nemavg_context->global_m);
        }

        if (m != NULL){
            nema_mat3x3_mul(shape_plus_global_m, m);
        }
    }

    int with_path = should_draw_path(_paint, blend);

    nema_vg_vertex_t_ bbox[4] = {
        {cx_ - r, cy_ - r},
        {cx_ + r, cy_ - r},
        {cx_ + r, cy_ + r},
        {cx_ - r, cy_ + r}
    };
    if (nemavg_context->fill_rule == NEMA_VG_STROKE){
        float half_stroke = nemavg_context->stroke.width * 0.5f;
        bbox[0].x -= half_stroke;
        bbox[0].y -= half_stroke;
        bbox[1].x += half_stroke;
        bbox[1].y -= half_stroke;
        bbox[2].x += half_stroke;
        bbox[2].y += half_stroke;
        bbox[3].x -= half_stroke;
        bbox[3].y += half_stroke;
    }

    if (has_transformation != 0U){
        // if we have scale, rotate or shear draw an ellipse
        if ((shape_plus_global_m[0][0] == 1.0f) && (shape_plus_global_m[0][1] == 0.0f) &&
            (shape_plus_global_m[1][0] == 0.0f) && (shape_plus_global_m[1][1] == 1.0f)){

                for (int i=0; i<4; ++i) {
                    bbox[i].x += shape_plus_global_m[0][2];
                    bbox[i].y += shape_plus_global_m[1][2];
                }
        }
    }

    uint8_t out_of_range = should_handle_large_coords(bbox, NULL);
    if (out_of_range == 1U){
        with_path = 1;
    }

    //if RASTERIZER BUG force drawing with stencil
#ifdef RASTERIZER_BUG_WA
    with_path = 1;
#endif

    if (with_path == 1){
        if (nemavg_context->fill_rule == NEMA_VG_STROKE){

            ellipse_points outer_points;
            ellipse_points inner_points;
            create_ellipse_curves(cx_, cy_, r + nemavg_context->stroke.width * 0.5f, r +nemavg_context->stroke.width * 0.5f, &outer_points);
            create_ellipse_curves(cx_, cy_, r - nemavg_context->stroke.width * 0.5f, r -nemavg_context->stroke.width * 0.5f, &inner_points);

            return draw_stroked_circle_path(&outer_points, &inner_points, _paint, m);
        } else {
            ellipse_points points;
            create_ellipse_curves(cx_, cy_, r, r, &points);
            return draw_ellipse_path(&points, _paint, m);
        }
    }

#ifndef NEMAPVG
    enable_do_not_transform(1);
#endif //NEMAPVG
    if ( nema_context.en_tscFB == 1U ) {
        nemavg_context->aa_flags |= (nema_context.surface_tile & RAST_TILE_MASK);
    }

    if (has_transformation != 0U){
        // if we have scale, rotate or shear draw an ellipse
        if ((shape_plus_global_m[0][0] != 1.0f) || (shape_plus_global_m[0][1] != 0.0f) ||
            (shape_plus_global_m[1][0] != 0.0f) || (shape_plus_global_m[1][1] != 1.0f)){

            return nema_vg_draw_ellipse(cx_, cy_, r, r, m, paint);
        }
        // else just translate the center
        cx_ += shape_plus_global_m[0][2];
        cy_ += shape_plus_global_m[1][2];
    }

    uint32_t paint_color = _paint->paint_color;

    if (nemavg_context->fill_rule == NEMA_VG_STROKE){
        if (has_transformation != 0U){
            paint_color = set_stroked_line_width(nemavg_context->stroke.width, shape_plus_global_m, paint_color);
        }else{
            paint_color = set_stroked_line_width(nemavg_context->stroke.width, NULL, paint_color);
        }
    }

    (void)shape_bind_blend(_paint, blend, paint_color);

    set_matrix_according_paint(_paint, m, 0);

    if (_paint->type != NEMA_VG_PAINT_GRAD_CONICAL){
        set_matrix_according_paint(_paint, m, 0);
        if (nemavg_context->fill_rule == NEMA_VG_STROKE){
            nema_raster_stroked_circle_aa(cx_, cy_, r, nemavg_context->stroke.width);
        }else{
            nema_raster_circle_aa(cx_, cy_, r);
        }
    }else{
        int supports_clip2 = nema_supports_clip2();

        if (nemavg_context->fill_rule == NEMA_VG_STROKE){
            for (int i=0; i<4; ++i){
                set_quad_clip2(i, supports_clip2, _paint);
                set_matrix_according_paint(_paint, m, i);

                nema_raster_stroked_circle_aa(cx_, cy_, r, nemavg_context->stroke.width);

                reset_clip2_or_temp(supports_clip2);
            }
        } else {
            for (int i=0; i<4; ++i){
                set_quad_clip2(i, supports_clip2, _paint);
                set_matrix_according_paint(_paint, m, i);

                nema_raster_circle_aa(cx_, cy_, r);

                reset_clip2_or_temp(supports_clip2);
            }
        }
    }
#ifndef NEMAPVG
    enable_do_not_transform(0U);
#endif //NEMAPVG
    if ( nema_context.en_tscFB == 1U ) {
        nemavg_context->aa_flags &= ~(nema_context.surface_tile & RAST_TILE_MASK);
    }

    return error;
}

uint32_t nema_vg_draw_ring(float cx, float cy, float ring_radius, float angle_start, float angle_end,
                                            NEMA_VG_PAINT_HANDLE paint)
{
    return nema_vg_draw_ring_generic(cx, cy, ring_radius, angle_start, angle_end, paint, 1U);
}

uint32_t nema_vg_draw_ring_generic(float cx, float cy, float ring_radius, float angle_start, float angle_end,
                                            NEMA_VG_PAINT_HANDLE paint, uint8_t has_caps)
{
    uint32_t error = NEMA_VG_ERR_NO_ERROR;
    float angle_start_ = angle_start;
    float angle_end_   = angle_end;
    float ring_radius_ = ring_radius;
    GET_PAINT;
    NEMA_VG_IF_ERROR(check_shapes(_paint));

    GET_CONTEXT;

    uint8_t has_transformation = (uint8_t)(nemavg_context->flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION);
    nema_matrix3x3_t global_matrix;

    if (has_transformation != 0U){
        global_matrix[0][0] = nemavg_context->global_m[0][0];
        global_matrix[0][1] = nemavg_context->global_m[0][1];
        global_matrix[0][2] = nemavg_context->global_m[0][2];
        global_matrix[1][0] = nemavg_context->global_m[1][0];
        global_matrix[1][1] = nemavg_context->global_m[1][1];
        global_matrix[1][2] = nemavg_context->global_m[1][2];
        global_matrix[2][0] = nemavg_context->global_m[2][0];
        global_matrix[2][1] = nemavg_context->global_m[2][1];
        global_matrix[2][2] = nemavg_context->global_m[2][2];

        nema_vg_reset_global_matrix();
    }

    uint32_t blend = nemavg_context->blend;

    uint32_t paint_color = _paint->paint_color;

    if (nemavg_context->fill_rule == NEMA_VG_STROKE){
        paint_color = set_stroked_line_width(nemavg_context->stroke.width, NULL, paint_color);
    }

    angle_start_ = nema_fmod_(angle_start_, 360.0f);
    angle_end_ = nema_clamp(angle_end_, angle_start_, angle_start_ + 360.0f);
    // bring the angles from [-360, 360] to [0, 720] space. Just for convinience
    angle_start_ += 360.0f;
    angle_end_   += 360.0f;

    float ring_width = nema_max2(nemavg_context->stroke.width, 1.0f);
    ring_radius_     = nema_max2(ring_radius_, 0.0001f);
    ring_radius_     = nema_max2(ring_radius_, (ring_width * 0.5f));

    nema_vg_vertex_t_ ring_center = {cx, cy};

    float angle_start_cos_theta = nema_cos(angle_start_);
    float angle_start_sin_theta = nema_sin(angle_start_);
    nema_vg_vertex_t_ cap_center1 = {ring_center.x + ring_radius_ * angle_start_cos_theta, ring_center.y + ring_radius_ * angle_start_sin_theta};

    float angle_end_cos_theta = nema_cos(angle_end_);
    float angle_end_sin_theta = nema_sin(angle_end_);
    nema_vg_vertex_t_ cap_center2 = {ring_center.x + ring_radius_ * angle_end_cos_theta, ring_center.y + ring_radius_ * angle_end_sin_theta};

    float angle_diff = nema_abs_(angle_end_ - angle_start_);

    float r = ring_radius_ + ring_width * 0.5f;
    nema_vg_vertex_t_ bbox[4] = {
        {cx - r, cy - r},
        {cx + r, cy - r},
        {cx + r, cy + r},
        {cx - r, cy + r}
    };

    int with_path = 0;
    if ((nemavg_context->quality & NEMA_VG_QUALITY_MAXIMUM) == NEMA_VG_QUALITY_MAXIMUM) {
        with_path = 1;
    }

    uint8_t out_of_range = should_handle_large_coords(bbox, NULL);
    if (out_of_range == 1U){
        with_path = 1;
    }

    if ((angle_diff > 180.0f) && (ring_caps_intersect(cap_center1, cap_center2, ring_width * 0.5f) == 1) ){
        with_path = 0;
    }

    if (with_path == 1){
        uint8_t pre_fill_rule = nemavg_context->fill_rule;
        nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);

        float ring_half_width = ring_width * 0.5f;

        uint8_t exterior_arc_cmd = (uint8_t)(NEMA_VG_PRIM_ARC | NEMA_VG_ARC_CW);
        uint8_t interior_arc_cmd = (uint8_t)NEMA_VG_PRIM_ARC;

        if (angle_diff > 180.0f) {
            exterior_arc_cmd |= NEMA_VG_ARC_LARGE;
            interior_arc_cmd |= NEMA_VG_ARC_LARGE;
        }

        nema_vg_vertex_t_ cap_exterior1 = {
            cap_center1.x + ring_half_width * angle_start_cos_theta,
            cap_center1.y + ring_half_width * angle_start_sin_theta,
        };
        nema_vg_vertex_t_ cap_interior1 = {
            cap_center1.x - ring_half_width * angle_start_cos_theta,
            cap_center1.y - ring_half_width * angle_start_sin_theta,
        };
        nema_vg_vertex_t_ cap_exterior2 = {
            cap_center2.x + ring_half_width * angle_end_cos_theta,
            cap_center2.y + ring_half_width * angle_end_sin_theta,
        };
        nema_vg_vertex_t_ cap_interior2 = {
            cap_center2.x - ring_half_width * angle_end_cos_theta,
            cap_center2.y - ring_half_width * angle_end_sin_theta,
        };

        NEMA_VG_PATH_HANDLE path = nema_vg_path_create();
        nema_vg_path_clear(path);

        if(has_caps == 0U){
            float coords[16] = {
                            // outer large arc
                            cap_exterior1.x, cap_exterior1.y,//move
                            ring_radius_ + ring_half_width, ring_radius_ + ring_half_width, // rx, ry
                            0.0f, //rot
                            cap_exterior2.x, cap_exterior2.y, // arc endpoint
                            cap_interior2.x, cap_interior2.y, // line endpoint
                            // inner arc
                            ring_radius_ - ring_half_width, ring_radius_ - ring_half_width, // rx, ry
                            0.0f, //rot
                            cap_interior1.x, cap_interior1.y, // arc endpoint
                            cap_exterior1.x, cap_exterior1.y // line endpoint
                        };

            uint8_t cmds[6] = {(uint8_t)NEMA_VG_PRIM_MOVE,
                            exterior_arc_cmd,
                            (uint8_t)NEMA_VG_PRIM_LINE,
                            interior_arc_cmd,
                            (uint8_t)NEMA_VG_PRIM_LINE,
                            NEMA_VG_PRIM_CLOSE
                            };

            nema_vg_path_set_shape(path, 6L, cmds, 16L, coords);
        }else {
            float coords[22] = {
                                // outer large arc
                                cap_exterior1.x, cap_exterior1.y,//move
                                ring_radius_ + ring_half_width, ring_radius_ + ring_half_width, // rx, ry
                                0.0f, //rot
                                cap_exterior2.x, cap_exterior2.y, // arc endpoint
                                // ring cap
                                ring_half_width, ring_half_width, // rx, ry
                                0.0f, //rot
                                cap_interior2.x, cap_interior2.y, // arc endpoint
                                // inner arc
                                ring_radius_ - ring_half_width, ring_radius_ - ring_half_width, // rx, ry
                                0.0f, //rot
                                cap_interior1.x, cap_interior1.y, // arc endpoint
                                // second cap
                                ring_half_width, ring_half_width, // rx, ry
                                0.0f, //rot
                                cap_exterior1.x, cap_exterior1.y //arc endpoint
                            };

            uint8_t cmds[6] = {(uint8_t)NEMA_VG_PRIM_MOVE,
                                exterior_arc_cmd,
                                (uint8_t)NEMA_VG_PRIM_SCWARC,
                                interior_arc_cmd,
                                (uint8_t)NEMA_VG_PRIM_SCWARC,
                                NEMA_VG_PRIM_CLOSE
                                };

            nema_vg_path_set_shape(path, 6L, cmds, 22L, coords);
        }

        error = nema_vg_draw_path(path, paint);

        nema_vg_path_destroy(path);

        nema_vg_set_fill_rule(pre_fill_rule);
    } else {

        (void) shape_bind_blend(_paint, blend, paint_color);

        if ( nema_context.en_tscFB == 1U ) {
            nemavg_context->aa_flags |= (nema_context.surface_tile & RAST_TILE_MASK);
        }
        // now draw the rounded caps. The one in the start first
        if (_paint->type == NEMA_VG_PAINT_GRAD_CONICAL){
            int start_quad = (int)(nema_floor_(angle_start_ / 90.0f)) % 4;
            set_matrix_according_paint(_paint, NULL, start_quad);
        }else{
            set_matrix_according_paint(_paint, NULL, 0);
        }

        if(has_caps == 1U){
            fill_rounded_cap(&cap_center1,
                            ring_width * 0.5f, angle_start_cos_theta, angle_start_sin_theta,
                            0);
        }
        else{
            //MISRA
        }

        if (_paint->type != NEMA_VG_PAINT_GRAD_CONICAL){
            nema_raster_stroked_arc_aa_mask(cx, cy, ring_radius_, ring_width, angle_start_, angle_end_,(has_caps == 1U) ? 0U : RAST_AA_MASK);
        }else{
            // find how many quadrants the angle_diff occupies
            // this is done is we divide the angle_diff by 90
            // and ceil the result
            int occupied_quads = nema_ceil_(angle_diff / 90.0f);

            // This should be written in a better way
            // if we have an angle diff bigger or equal than 270 deg
            // and angle start does not coincide on one of the axes,
            // then we draw in the first quad 2 times
            if (angle_diff >= 270.0f && (nema_fmod_(angle_start_, 90.0f) > 0.0f)){
                ++occupied_quads;
            }

            float quad_angle_start = angle_start_;
            for (int q = 0; q < occupied_quads; ++q){
                int current_quad = nema_floor_(quad_angle_start / 90.0f);
                float quad_angle_end = nema_min2(angle_end_, ((float)current_quad + 1.0f) * 90.0f);

                set_matrix_according_paint(_paint, NULL, current_quad % 4);
                nema_raster_stroked_arc_aa_mask(cx, cy, ring_radius_, ring_width, quad_angle_start, quad_angle_end,(has_caps == 1U) ? 0U : RAST_AA_MASK);

                quad_angle_start = nema_min2(quad_angle_end, angle_end_);
            }
        }

        if (_paint->type == NEMA_VG_PAINT_GRAD_CONICAL){
            int end_quad = nema_floor_(angle_end_ / 90.0f) % 4;
            set_matrix_according_paint(_paint, NULL, end_quad);
        }

        if(has_caps == 1U){
            fill_rounded_cap(&cap_center2,
                            ring_width * 0.5f, angle_end_cos_theta, angle_end_sin_theta,
                            1);
        }
        else{
            //MISRA
        }

        if ( nema_context.en_tscFB == 1U ) {
            nemavg_context->aa_flags &= ~(nema_context.surface_tile & RAST_TILE_MASK);
        }
    }

    if (has_transformation != 0U){
        (void)nema_vg_set_global_matrix(global_matrix);
    }

    return error;
}
