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

#include "nema_vg_paint.h"
#include "nema_vg_path.h"
#include "nema_vg_p.h"
#include "nema_vg.h"
#include "nema_graphics.h"
#include "nema_sys_defs.h"
#include "nema_rasterizer_intern.h"
#include "nema_vg_context.h"

#ifndef FB_FORMAT
#define FB_FORMAT NEMA_RGBA8888
#endif

TLS_VAR nema_vg_context_t_ context_ = {0};



TLS_VAR nema_vg_context_t_* nemavg_context ;
nema_vg_stencil_t_ stencil_;
nema_vg_stencil_t_* stencil;
nema_buffer_t lut_gradient;

void
context_reset(void)
{
    context_.clipped_segs            = NULL;
    context_.clipped_data            = NULL;

    context_.fill_rule               = NEMA_VG_FILL_NON_ZERO;
    context_.quality                 = NEMA_VG_QUALITY_BETTER;
    context_.drawing_clipped_path    = 0U;
    context_.fb_tsc                  = 0U;
    context_.error                   = NEMA_VG_ERR_NO_ERROR;
    context_.blend                   = NEMA_BL_SRC_OVER;
    context_.aa_flags                = RAST_AA_E1;
    context_.aa_mask                 = RAST_AA_MASK;
    context_.flags                   = 0U;
    context_.clipped_segs_size_bytes = 0U;
    context_.clipped_data_size_bytes = 0U;
    context_.masking_info.mask_obj   = NULL;
    context_.masking_info.trans_x    = 0.0f;
    context_.masking_info.trans_y    = 0.0f;
    context_.masking_info.masking    = 0U;

    nema_mat3x3_load_identity(context_.global_m);
    nema_mat3x3_load_identity(context_.global_m_inv);
    nema_mat3x3_load_identity(context_.path_plus_global_m);

    context_.info.do_fat_stroke             = 0;
    context_.info.do_draw_on_fb             = 0;
    context_.info.use_bevel_joins           = 0;
    context_.info.do_not_transform          = 0;
    context_.info.last_vertex               = -1;
    context_.info.stroke_w                  = 1.f;
    context_.info.stroke_w_x                = 1.f;
    context_.info.stroke_w_y                = 1.f;
    context_.info.paint_color               = 0x0;
    context_.info.num_stroked_line_segments = 0U;
    context_.info.first_line.q0.x           = 0.0f;
    context_.info.first_line.q1.x           = 0.0f;
    context_.info.first_line.q2.x           = 0.0f;
    context_.info.first_line.q3.x           = 0.0f;
    context_.info.first_line.q0.y           = 0.0f;
    context_.info.first_line.q1.y           = 0.0f;
    context_.info.first_line.q2.y           = 0.0f;
    context_.info.first_line.q3.y           = 0.0f;
    context_.info.first_line.length         = 0.0f;
    context_.info.first_line.dx             = 0.0f;
    context_.info.first_line.dy             = 0.0f;
    context_.info.last_line.q0.x            = 0.0f;
    context_.info.last_line.q1.x            = 0.0f;
    context_.info.last_line.q2.x            = 0.0f;
    context_.info.last_line.q3.x            = 0.0f;
    context_.info.last_line.q0.y            = 0.0f;
    context_.info.last_line.q1.y            = 0.0f;
    context_.info.last_line.q2.y            = 0.0f;
    context_.info.last_line.q3.y            = 0.0f;
    context_.info.last_line.length          = 0.0f;
    context_.info.last_line.dx              = 0.0f;
    context_.info.last_line.dy              = 0.0f;
    context_.info.end_line.q0.x             = 0.0f;
    context_.info.end_line.q1.x             = 0.0f;
    context_.info.end_line.q2.x             = 0.0f;
    context_.info.end_line.q3.x             = 0.0f;
    context_.info.end_line.q0.y             = 0.0f;
    context_.info.end_line.q1.y             = 0.0f;
    context_.info.end_line.q2.y             = 0.0f;
    context_.info.end_line.q3.y             = 0.0f;
    context_.info.end_line.length           = 0.0f;
    context_.info.end_line.dx               = 0.0f;
    context_.info.end_line.dy               = 0.0f;
    context_.info.has_transformation        = 0U;
    context_.info.had_transformation        = 0U;
    context_.info.pre_clip_transformation   = 0U;
    context_.info.faster_quality            = 0U;
    context_.info.has_cap                   = 0U;
    context_.info.draw_join                 = 1;
    context_.info.global_sx_factor          = 0.0f;
    context_.info.global_sy_factor          = 0.0f;
    for (int i = 0 ; i< STACK_DEPTH; ++i)
    {
        for (int j = 0 ; j< 6; ++j)
        {
            context_.info.tess_stack[i][j] = 0.0f;
        }
    }
    context_.info.tess_stack_idx             = 0;

    context_.info.fan_center.fanx0_f  = 0.0f;
    context_.info.fan_center.fany0_f  = 0.0f;
    context_.info.fan_center.fanx0  = 0;
    context_.info.fan_center.fanx0  = 0;
    context_.info.fan_center.fany0fx  = 0;
    context_.info.fan_center.fany0fx  = 0;
#ifndef NEMAPVG
#ifdef HAAS
    context_.info.clip_x_orig = 0;
    context_.info.clip_y_orig = 0;
    context_.info.clip_w_orig = 0;
    context_.info.clip_h_orig = 0;
#endif
#endif
    context_.stroke.width = 1.0f;
    context_.stroke.miter_limit = 4.0f;
    context_.stroke.start_cap_style = NEMA_VG_CAP_BUTT;
    context_.stroke.end_cap_style = NEMA_VG_CAP_BUTT;
    context_.stroke.join_style = NEMA_VG_JOIN_BEVEL;
}

nema_vg_context_t_*
nema_vg_get_context(void)
{
    return &context_;
}

void
stencil_buffer_destroy(void)
{
    nema_buffer_destroy(&stencil_.bo);
}

nema_buffer_t
stencil_buffer_create(int w, int h, int pool)
{

#ifdef RASTERIZER_BUG_WA
    ++w;
    ++h;
#endif

    return nema_buffer_create_pool(pool, w*h);
}

void
stencil_buffer_set(int w, int h, nema_buffer_t bo)
{
    stencil_.bo = bo;
    stencil_.width = w;
    stencil_.height = h;
    stencil_.dirty_area_p1.x = 0.0f;
    stencil_.dirty_area_p1.y = 0.0f;
    stencil_.dirty_area_p2.x = (nema_vg_float_t)w;
    stencil_.dirty_area_p2.y = 0.0f;
    stencil_.dirty_area_p3.x = (nema_vg_float_t)w;
    stencil_.dirty_area_p3.y = (nema_vg_float_t)h;
    stencil_.dirty_area_p4.x = 0.0f;
    stencil_.dirty_area_p4.y = (nema_vg_float_t)h;
}

void stencil_buffer_set_prealloc(void)
{
    stencil_.flags |= NEMA_VG_STENCIL_ALLOC;
}

nema_vg_stencil_t_*
nema_vg_get_stencil(void)
{
    return &stencil_;
}

#ifndef NEMAPVG
void
lut_buffer_create(void)
{
    if (lut_gradient.base_phys != 0U) {
        return;
    }
    lut_gradient = nema_buffer_create_pool(NEMA_MEM_POOL_FB, LUT_SIZE);

    uint8_t* lut_ = (uint8_t*)lut_gradient.base_virt;

    for (int i = 0; i <= LUT_SIZE-1; ++i) {
        uint32_t idx = ((uint32_t)i>>4) | ((uint32_t)i <<4);
        int idx_i = (int)idx%256;
        int val = i;
        // int val = i*256/LUT_SIZE;
        if (val <= 0x70){
            val = 0xff;
        }
        else if (val >= 0x90){
            val = 0xff;
        }
        else if (val == 0x80){
            val = 0;
        }
        else if (val > 0x80) {
            val = (val-0x80)*16;
        }
        else {
            val = (0x80-val)*16;
        }
        lut_[idx_i] = (uint8_t)val;
    }

    lut_[LUT_SIZE-1] = 255U;

#ifdef NEMA_VG_FLUSH_BUFFERS
NEMA_VG_FLUSH_CACHE;
#endif

}

void
lut_buffer_destroy(void)
{
    nema_buffer_destroy(&lut_gradient);
}

#endif //NEMAPVG

nema_buffer_t* nema_vg_get_lut(void)
{
    return &lut_gradient;
}

uint8_t
get_fill_rule(void)
{
    return context_.fill_rule;
}

void
enable_screen_space_stroking(void)
{
    context_.flags |= NEMA_VG_CONTEXT_STROKE_WIDTH_SCREEN_SPACE;
}

void
disable_screen_space_stroking(void)
{
    context_.flags = context_.flags & (~NEMA_VG_CONTEXT_STROKE_WIDTH_SCREEN_SPACE);
}

void
nema_vg_set_fill_rule(uint8_t fill_rule)
{
    context_.fill_rule = fill_rule;
}

void
nema_vg_stroke_set_width(float width)
{
    context_.stroke.width = width;
}

void
nema_vg_stroke_set_cap_style(uint8_t start_cap_style, uint8_t end_cap_style)
{
    if((start_cap_style >= NEMA_VG_CAP_MAX) ||(end_cap_style >= NEMA_VG_CAP_MAX)){
        nema_vg_set_error(NEMA_VG_ERR_INVALID_CAP_STYLE);
        return;
    }
    context_.stroke.start_cap_style = start_cap_style;
    context_.stroke.end_cap_style   = end_cap_style;
}

void
nema_vg_stroke_set_join_style(uint8_t join_style)
{
    if(join_style >= NEMA_VG_JOIN_MAX){
        nema_vg_set_error(NEMA_VG_ERR_INVALID_JOIN_STYLE);
        return;
    }
    context_.stroke.join_style = join_style;
}

void
nema_vg_stroke_set_miter_limit(float miter_limit)
{
    context_.stroke.miter_limit = miter_limit;
}

void
nema_vg_masking(uint8_t masking)
{
    context_.masking_info.masking = masking;
}

uint32_t
nema_vg_set_mask(nema_img_obj_t *mask_obj)
{
    uint32_t error = NEMA_VG_ERR_NO_ERROR;

    if(mask_obj == NULL){
        error = NEMA_VG_ERR_INVALID_MASKING_FORMAT;
        nema_vg_set_error(error);
        context_.masking_info.mask_obj = NULL;
        return error;
    }

    if(!(mask_obj->format == NEMA_A8 || mask_obj->format == NEMA_A4 ||
         mask_obj->format == NEMA_A2 || mask_obj->format == NEMA_A1 )){
        error = NEMA_VG_ERR_INVALID_MASKING_FORMAT;
        nema_vg_set_error(error);
        context_.masking_info.mask_obj = NULL;
        return error;
    }
    context_.masking_info.mask_obj = mask_obj;
    return error;
}

void
nema_vg_set_mask_translation(float x, float y)
{
    context_.masking_info.trans_x = x;
    context_.masking_info.trans_y = y;
}

uint32_t
nema_vg_get_error(void)
{
    uint32_t error = context_.error;
    context_.error = NEMA_VG_ERR_NO_ERROR;
    return error;
}

void
nema_vg_set_error(uint32_t error)
{
    context_.error = error;
}

void nema_vg_set_blend(uint32_t blend)
{
    context_.blend = blend;
}

void
nema_vg_set_quality(uint8_t quality)
{
    context_.quality = quality;

    if((quality & NEMA_VG_QUALITY_NON_AA) == NEMA_VG_QUALITY_NON_AA){
        context_.aa_flags = 0U;
        context_.aa_mask = 0U;
    }
    else{
        context_.aa_flags = RAST_AA_E1;
        context_.aa_mask = RAST_AA_MASK;
    }
}

uint32_t
nema_vg_set_global_matrix(nema_matrix3x3_t m)
{
    if ( m == NULL ) {
        context_.flags = context_.flags & (~NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION);
        return NEMA_VG_ERR_NO_ERROR;
    }

#if 0
    if (
            m[0][0] == 1.f
        &&  m[0][1] == 0.f
        &&  m[0][2] == 0.f
        &&  m[1][0] == 0.f
        &&  m[1][1] == 1.f
        &&  m[1][2] == 0.f
        &&  m[2][0] == 0.f
        &&  m[2][1] == 0.f
        &&  m[2][2] == 1.f
        ) {
        // Identity Matrix
        context_.flags = = context_.flags & (~NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION);
        return NEMA_VG_ERR_NO_ERROR;

    }
#endif

    int x, y;
    for (y = 0; y < 3; ++y) {
        for (x = 0; x < 3; ++x) {
            context_.global_m[y][x] = m[y][x];
            context_.global_m_inv[y][x] = m[y][x];
        }
    }

    if ( nema_mat3x3_invert(context_.global_m_inv) != 0 ) {
        return NEMA_VG_ERR_NON_INVERTIBLE_MATRIX;
    }

    context_.flags |= NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION;

    return NEMA_VG_ERR_NO_ERROR;
}

void
nema_vg_reset_global_matrix(void) {
    context_.info.had_transformation = (uint8_t)(context_.flags & NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION);
    context_.flags = context_.flags & (~NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION);
    nema_mat3x3_load_identity(context_.global_m);
    nema_mat3x3_load_identity(context_.global_m_inv);
}

void reenable_global_matrix(void){
    if (context_.info.had_transformation != 0U) {
        context_.flags |= NEMA_VG_CONTEXT_HAS_GLOBAL_TRANSFORMATION;
        context_.info.had_transformation = 0U;
    }
}

void nema_vg_handle_large_coords(uint8_t enable, uint8_t allow_internal_alloc) {
    if(enable != 0U)
    {
        context_.flags |= NEMA_VG_CONTEXT_ENABLE_HANDLE_LARGE_COORDS;
    }
    else
    {
        context_.flags &= ~NEMA_VG_CONTEXT_ENABLE_HANDLE_LARGE_COORDS;
    }
    if(allow_internal_alloc != 0U)
    {
        context_.flags |= NEMA_VG_CONTEXT_ALLOW_INTERNAL_ALLOC;
    }
    else
    {
        context_.flags &= ~NEMA_VG_CONTEXT_ALLOW_INTERNAL_ALLOC;
    }
}

uint32_t nema_vg_bind_clip_coords_buf(void *segs, uint32_t segs_size_bytes, void *data, uint32_t data_size_bytes)
{
    if(segs == NULL){
        return NEMA_VG_ERR_BAD_BUFFER;
    }
    if(segs_size_bytes <= 0U ){
        return NEMA_VG_ERR_INVALID_ARGUMENTS;
    }
    if(data == NULL){
        return NEMA_VG_ERR_BAD_BUFFER;
    }
    if(data_size_bytes <= 0U ){
        return NEMA_VG_ERR_INVALID_ARGUMENTS;
    }

    context_.clipped_data = (nema_buffer_t*)data;
    context_.clipped_segs = (nema_buffer_t*)segs;
    context_.clipped_data_size_bytes = data_size_bytes;
    context_.clipped_segs_size_bytes = segs_size_bytes;

    return NEMA_VG_ERR_NO_ERROR;
}

void nema_vg_unbind_clip_coords_buf(void)
{
    context_.clipped_data = NULL;
    context_.clipped_segs = NULL;
    context_.clipped_data_size_bytes = 0U;
    context_.clipped_segs_size_bytes = 0U;
}

float
get_stroke_width(void)
{
    return context_.stroke.width;
}
