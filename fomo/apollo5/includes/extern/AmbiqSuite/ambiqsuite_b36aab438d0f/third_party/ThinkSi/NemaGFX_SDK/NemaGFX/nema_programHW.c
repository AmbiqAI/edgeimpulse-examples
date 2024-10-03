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
#include "nema_raster.h"
#include "nema_regs.h"
#include "nema_blender_intern.h"
#include "nema_programHW.h"
#include "nema_rasterizer.h"
#include "nema_rasterizer_intern.h"
#include "nema_error.h"

TLS_VAR nema_context_t nema_context = {0};
// {.en_tscFB = 0,
//  .en_ZCompr = 0,
//  .en_sw_depth = 0,
//  .surface_tile = 0U,
//  .tri_cul = 0U,
//  .color_grad = 0U,
//  .draw_flags = 0U,
//  .aa = 0,
//  .nema_error = NEMA_ERR_NO_INIT,
//  .prev_clip_xy = {0},
//  .prev_clip_wh = {0},
//  .breakpoint = 0,
//  .texs = {zero_tex},
//  .implicit_submission_lock = 0U,
//  .explicit_submission = 0U,
//  .src_ckey = 0U};


#define CL_GET_SPACE(size)                      \
        cmd_array = (uint32_t *)nema_cl_get_space(size);    \
        if (cmd_array == (NULL)) {                   \
            return;                             \
        }
#define CL_GET_SPACE_U(size)                      \
        cmd_array = (nema_multi_union_t *)nema_cl_get_space(size);    \
        if (cmd_array == NULL) {                   \
            return;                             \
        }

#define EXTRA_HOLD  ( (nema_context.surface_tile != 0U) ? 0x01000000U : 0U)


// -------------------------------- CONFIG -------------------------------------
uint32_t
nema_readHwConfig(void)
{
    static uint32_t config = 0U;

    if ( config == 0U ) {
        config = nema_reg_read(NEMA_CONFIG);
    }

    return config;
}

uint32_t
nema_readHwConfigH(void)
{
    uint32_t   config = nema_reg_read(NEMA_CONFIGH);

    return config;
}

void enable_scaling(uint8_t scaling)
{
    nema_context.scaling = scaling;
}

void
nema_enable_tiling(uint32_t enable)
{
    nema_context.surface_tile = (enable != 0U) ? RAST_TILE : 0U;
    nema_context.draw_flags   = nema_context.surface_tile | nema_context.color_grad | nema_context.tri_cul | nema_context.aa;
}

void
nema_enable_tiling_2x2(uint32_t enable)
{
    nema_context.surface_tile = (enable != 0U) ? RAST_TILE_2X2 : 0U;
    nema_context.draw_flags   = nema_context.surface_tile | nema_context.color_grad | nema_context.tri_cul | nema_context.aa;
}

uint32_t
nema_enable_aa(uint8_t e0, uint8_t e1, uint8_t e2, uint8_t e3)
{
    uint32_t aa = 0;
    if (e0 != 0U) { aa |= RAST_AA_E0;   }
    if (e1 != 0U) { aa |= RAST_AA_E1;   }
    if (e2 != 0U) { aa |= RAST_AA_E2;   }
    if (e3 != 0U) { aa |= RAST_AA_E3;   }

    uint32_t prev_aa = nema_context.aa;
    nema_context.aa = aa;

    nema_context.draw_flags   = nema_context.surface_tile | nema_context.color_grad | nema_context.tri_cul | nema_context.aa;
    return prev_aa;
}

uint32_t
nema_enable_aa_flags(uint32_t aa) {
    uint32_t prev_aa = nema_context.aa;
    nema_context.aa = aa & RAST_AA_MASK;

    nema_context.draw_flags   = nema_context.surface_tile | nema_context.color_grad | nema_context.tri_cul | nema_context.aa;
    return prev_aa;
}

// -------------------------------- LOADCTRL -----------------------------------
void
nema_setLoadCtrlReg(uint32_t val)
{
    nema_cl_add_cmd( NEMA_LOADCTRL, val);
}

// ------------------------------- MATMULT -------------------------------------
void
nema_set_matmul_ctrl(uint32_t val)
{
    nema_cl_add_cmd( NEMA_MATMULT, val);
}

//-------------------------------------------------------------------------------
void
nema_matmul_bypass(int enable) {
    if (enable != 0) {
        nema_set_matmul_ctrl(MMUL_BYPASS | MMUL_NONPERSP);
    }
    else {
        nema_set_matmul_ctrl(0);
    }
}

//-------------------------------------------------------------------------------
void
nema_set_matrix(nema_matrix3x3_t m)
{
    int i = 0;

    nema_multi_union_t *cmd_array;
    #define EPSILON     (0.0000001f)
    #define COMPARE_2_ZERO(a) ((a) < (EPSILON) && (a) > -(EPSILON) )
    if ( COMPARE_2_ZERO(m[2][0]) && COMPARE_2_ZERO(m[2][1]) ) {
        if ( !COMPARE_2_ZERO(m[2][0] - 1.0f) ) {
            m[0][0] /= m[2][2];
            m[0][1] /= m[2][2];
            m[0][2] /= m[2][2];
            m[1][0] /= m[2][2];
            m[1][1] /= m[2][2];
            m[1][2] /= m[2][2];
        }

        CL_GET_SPACE_U(6);

        cmd_array[i].u = NEMA_MM12; i++; cmd_array[i].f = m[1][2]; i++;
        cmd_array[i].u = NEMA_MM02; i++; cmd_array[i].f = m[0][2]; i++;
    } else {
        CL_GET_SPACE_U(9);

        cmd_array[i].u = NEMA_MM12; i++; cmd_array[i].f = m[1][2]; i++;
        cmd_array[i].u = NEMA_MM02; i++; cmd_array[i].f = m[0][2]; i++;

        cmd_array[i].u = NEMA_MM20; i++; cmd_array[i].f = m[2][0]; i++;
        cmd_array[i].u = NEMA_MM21; i++; cmd_array[i].f = m[2][1]; i++;
        cmd_array[i].u = NEMA_MM22; i++; cmd_array[i].f = m[2][2]; i++;
    }
    #undef EPSILON
    #undef COMPARE_2_ZERO

    cmd_array[i].u = NEMA_MM00; i++; cmd_array[i].f = m[0][0]; i++;
    cmd_array[i].u = NEMA_MM01; i++; cmd_array[i].f = m[0][1]; i++;
    cmd_array[i].u = NEMA_MM10; i++; cmd_array[i].f = m[1][0]; i++;
    cmd_array[i].u = NEMA_MM11; i++; cmd_array[i].f = m[1][1]; i++;
}

//-------------------------------------------------------------------------------
void
nema_set_matrix_all(nema_matrix3x3_t m)
{
    nema_multi_union_t *cmd_array;
    CL_GET_SPACE_U(9);

    cmd_array[ 0].u = NEMA_MM12; cmd_array[ 1].f = m[1][2];
    cmd_array[ 2].u = NEMA_MM02; cmd_array[ 3].f = m[0][2];
    cmd_array[ 4].u = NEMA_MM20; cmd_array[ 5].f = m[2][0];
    cmd_array[ 6].u = NEMA_MM21; cmd_array[ 7].f = m[2][1];
    cmd_array[ 8].u = NEMA_MM22; cmd_array[ 9].f = m[2][2];
    cmd_array[10].u = NEMA_MM00; cmd_array[11].f = m[0][0];
    cmd_array[12].u = NEMA_MM01; cmd_array[13].f = m[0][1];
    cmd_array[14].u = NEMA_MM10; cmd_array[15].f = m[1][0];
    cmd_array[16].u = NEMA_MM11; cmd_array[17].f = m[1][1];
}

// ------------------------------- MATMULT -------------------------------------
void
nema_set_matrix_scale(
                      float dst_x,
                      float dst_y,
                      float dst_xres,
                      float dst_yres,
                      float src_x,
                      float src_y,
                      float src_xres,
                      float src_yres
                      )
{
    float W = src_xres/dst_xres;
    float H = src_yres/dst_yres;
    float X = -dst_x*W + src_x;
    float Y = -dst_y*H + src_y;

    nema_multi_union_t *cmd_array;
    CL_GET_SPACE_U(4);

    cmd_array[ 0].u = NEMA_MM12; cmd_array[ 1].f = Y;
    cmd_array[ 2].u = NEMA_MM02; cmd_array[ 3].f = X;
    cmd_array[ 4].u = NEMA_MM00; cmd_array[ 5].f = W;
    cmd_array[ 6].u = NEMA_MM11; cmd_array[ 7].f = H;
}
//-------------------------------------------------------------------------------
void
nema_set_matrix_translate(float dst_x, float dst_y)
{
    float dst_x_ = dst_x;
    float dst_y_ = dst_y;

    dst_x_ = -dst_x_;
    dst_y_ = -dst_y_;

    nema_multi_union_t *cmd_array;
    CL_GET_SPACE_U(2);

    cmd_array[ 0].u = NEMA_MM12; cmd_array[ 1].f = dst_y_;
    cmd_array[ 2].u = NEMA_MM02; cmd_array[ 3].f = dst_x_;

    // NemaP's MatMult interpolator automatically uses fixed point arithmetic
    // for translations. Other affine or perspective use (reduced) floating
    // points. When translating to over 2048, fixed point range is not enough,
    // so force MatMult to use floating point
    if ( (dst_x_ < -2048.f) || (dst_y_ < -2048.f) ) {
        nema_set_matmul_ctrl(MMUL_NONPERSP);
    }
}

// ------------------------------ CONSTREGS ------------------------------------

void nema_set_const_reg_half(uint32_t reg, uint32_t v, float value){

    uint32_t hwreg = reg | (((uint32_t)0x1U<<v)<<4);

    //if (reg > 4) return;
    nema_multi_union_t val = {.f = value};
    nema_cl_add_cmd( NEMA_FRAG_CONADDR, hwreg);
    nema_cl_add_cmd( NEMA_FRAG_CONDATA, val.u);
}

void nema_set_const_reg_single(uint32_t reg, uint32_t v, float value){

    uint32_t hwreg = reg | (((uint32_t)0x3U<<v)<<4);
    //if (hwreg > (NEMA_CMAX_REG + 0x20)) return;
    nema_multi_union_t val = {.f = value};
    nema_cl_add_cmd( NEMA_FRAG_CONADDR, hwreg);
    nema_cl_add_cmd( NEMA_FRAG_CONDATA, val.u);
}

// ------------------------------- UBLENDER ------------------------------------
void
nema_load_frag_shader(const uint32_t *cmd,
                      uint32_t count,
                      uint32_t codeptr)
{
    const uint32_t *cmd_ptr = cmd;
    uint32_t i;
    nema_cl_add_cmd(NEMA_IMEM_ADDR, codeptr);
    for(i=0; i<count; i++)
    {
        nema_cl_add_cmd(NEMA_IMEM_DATAH  , *cmd_ptr); ++cmd_ptr;
        nema_cl_add_cmd(NEMA_IMEM_DATAL  , *cmd_ptr); ++cmd_ptr;
    }
}

//-------------------------------------------------------------------------------
void
nema_set_frag_ptr(uint32_t ptr)
{
    nema_cl_add_cmd(NEMA_CODEPTR, ptr);
}

//-------------------------------------------------------------------------------
void
nema_load_frag_shader_ptr(const uint32_t *cmd,
                          uint32_t count,
                          uint32_t codeptr,
                          uint32_t ptr)
{
    const uint32_t *cmd_ = cmd;

    uint32_t *cmd_array;
    int sz = ((int)count*2)+2;
    CL_GET_SPACE(sz);

    *cmd_array = NEMA_IMEM_ADDR; ++cmd_array;
    *cmd_array = codeptr;        ++cmd_array;

    uint32_t i;
    for(i=0; i<count; i++)
    {
//        nema_cl_add_cmd(NEMA_IMEM_DATAH  , *cmd); ++cmd;
//        nema_cl_add_cmd(NEMA_IMEM_DATAL  , *cmd); ++cmd;
        *cmd_array = NEMA_IMEM_DATAH; ++cmd_array; *cmd_array = cmd_[0]; ++cmd_array;
        *cmd_array = NEMA_IMEM_DATAL; ++cmd_array; *cmd_array = cmd_[1]; ++cmd_array;

        cmd_ += 2;
    }

    *cmd_array = NEMA_CODEPTR; ++cmd_array;
    *cmd_array = ptr;
}

// ------------------------------- ROP_BLENDER ----------------------------------
void
nema_set_rop_blend_mode(uint32_t bl_mode)
{
    nema_cl_add_cmd(NEMA_ROPBLENDER_BLEND_MODE, bl_mode);
}

void
nema_set_rop_dst_color_key(uint32_t rgba)
{
    nema_cl_add_cmd(NEMA_ROPBLENDER_DST_CKEY, rgba);
}

void
nema_set_rop_const_color(uint32_t rgba)
{
    nema_cl_add_cmd(NEMA_ROPBLENDER_CONST_COLOR, rgba);
}

// ------------------------------- Z-BUFFER ------------------------------------
void
nema_set_depth_ctrl(uint32_t val)
{
    nema_cl_add_cmd( NEMA_ZFUNC, val);
}

// -------------------------- RASTERIZER (DEPTH) -------------------------------
void
nema_set_depth_imm(uint32_t startl,
                   uint32_t starth,
                   uint32_t dxl,
                   uint32_t dxh,
                   uint32_t dyl,
                   uint32_t dyh)
{
    // input's range is [0, 0xffffffff]

    uint32_t *cmd_array;
    CL_GET_SPACE(6);

    cmd_array[ 0] = NEMA_DEPTH_START_L; cmd_array[ 1] = startl;
    cmd_array[ 2] = NEMA_DEPTH_START_H; cmd_array[ 3] = starth;
    cmd_array[ 4] = NEMA_DEPTH_DX_L;    cmd_array[ 5] = dxl;
    cmd_array[ 6] = NEMA_DEPTH_DX_H;    cmd_array[ 7] = dxh;
    cmd_array[ 8] = NEMA_DEPTH_DY_L;    cmd_array[ 9] = dyl;
    cmd_array[10] = NEMA_DEPTH_DY_H;    cmd_array[11] = dyh;
}

// -------------------------- RASTERIZER (GRAD) --------------------------------
void nema_set_gradient_fx(int32_t r_init, int32_t g_init, int32_t b_init,
                          int32_t a_init, int32_t r_dx,   int32_t r_dy,
                          int32_t g_dx,   int32_t g_dy,   int32_t b_dx,
                          int32_t b_dy,   int32_t a_dx,   int32_t a_dy)
{

    nema_multi_union_t *cmd_array;
    CL_GET_SPACE_U(12);

    cmd_array[ 0].u = NEMA_RED_INIT; cmd_array[ 1].i=r_init;
    cmd_array[ 2].u = NEMA_GRE_INIT; cmd_array[ 3].i=g_init;
    cmd_array[ 4].u = NEMA_BLU_INIT; cmd_array[ 5].i=b_init;
    cmd_array[ 6].u = NEMA_ALF_INIT; cmd_array[ 7].i=a_init;
    cmd_array[ 8].u = NEMA_RED_DX  ; cmd_array[ 9].i=r_dx;
    cmd_array[10].u = NEMA_RED_DY  ; cmd_array[11].i=r_dy;
    cmd_array[12].u = NEMA_GRE_DX  ; cmd_array[13].i=g_dx;
    cmd_array[14].u = NEMA_GRE_DY  ; cmd_array[15].i=g_dy;
    cmd_array[16].u = NEMA_BLU_DX  ; cmd_array[17].i=b_dx;
    cmd_array[18].u = NEMA_BLU_DY  ; cmd_array[19].i=b_dy;
    cmd_array[20].u = NEMA_ALF_DX  ; cmd_array[21].i=a_dx;
    cmd_array[22].u = NEMA_ALF_DY  ; cmd_array[23].i=a_dy;
}

// -------------------------- RASTERIZER (GEOM) --------------------------------

/** \brief Set the color which will be used when drawing primitives (lines, rectangles etc)
 *
 * \param rgba8888 Color to be used
 * \see nema_rgba()
 *
 */
void
nema_set_raster_color(uint32_t rgba8888)
{
    nema_cl_add_cmd( NEMA_DRAW_COLOR , rgba8888 );
}

//-------------------------------------------------------------------------------

/** \brief Program Rasterizer to draw a line
 *
 * \param x0 x coordinate at the beginning of the line
 * \param y0 y coordinate at the beginning of the line
 * \param x1 x coordinate at the end of the line
 * \param y1 y coordinate at the end of the line
 *
 */
void
nema_raster_line(int x0, int y0, int x1, int y1)
{

    uint32_t *cmd_array;
    CL_GET_SPACE(3);

    cmd_array[ 0] = NEMA_DRAW_STARTXY; cmd_array[ 1] = YX16TOREG32(y0, x0);
    cmd_array[ 2] = NEMA_DRAW_ENDXY  ; cmd_array[ 3] = YX16TOREG32(y1, x1);
    cmd_array[ 4] = NEMA_DRAW_CMD | EXTRA_HOLD;
    cmd_array[ 5] = nema_context.draw_flags | DRAW_LINE;
}
//-------------------------------------------------------------------------------

/** \brief Program Rasterizer to draw a triangle for NemaP
 *
 * \param x0fx x coordinate at the first vertex of the triangle (float)
 * \param y0fx y coordinate at the first vertex of the triangle (float)
 * \param x1fx x coordinate at the second vertex of the triangle (float)
 * \param y1fx y coordinate at the second vertex of the triangle (float)
 * \param x2fx x coordinate at the third vertex of the triangle (float)
 * \param y2fx y coordinate at the third vertex of the triangle (float)
 *
 */
void
nema_raster_triangle_f(float x0, float y0, float x1, float y1, float x2, float y2)
{
    int x0fx = nema_f2fx(x0);    int y0fx = nema_f2fx(y0);
    int x1fx = nema_f2fx(x1);    int y1fx = nema_f2fx(y1);
    int x2fx = nema_f2fx(x2);    int y2fx = nema_f2fx(y2);

    nema_raster_triangle_fx(x0fx, y0fx, x1fx, y1fx, x2fx, y2fx);
}

/** \brief Program Rasterizer to draw a triangle for NemaP
 *
 * \param x0 x coordinate at the first vertex of the triangle (float)
 * \param y0 y coordinate at the first vertex of the triangle (float)
 *
 */
void
nema_raster_triangle_p0_f(float x0, float y0)
{
    int x0fx = nema_f2fx(x0);    int y0fx = nema_f2fx(y0);

    nema_multi_union_t *cmd_array;
    CL_GET_SPACE_U(3);

    cmd_array[0].u = NEMA_DRAW_PT0_X; cmd_array[1].i = x0fx;
    cmd_array[2].u = NEMA_DRAW_PT0_Y; cmd_array[3].i = y0fx;
    cmd_array[4].u = NEMA_DRAW_CMD | EXTRA_HOLD  ;

    uint32_t flags = nema_context.draw_flags;
    if ( (flags & (uint32_t)RAST_AA_E2) != 0U) {
        flags |= RAST_AA_E3;
    } else {
        flags &= ~(RAST_AA_E3);
    }

    cmd_array[5].u = flags | DRAW_TRIANGLE;
}

/** \brief Program Rasterizer to draw a triangle for NemaP
 *
 * \param x1 x coordinate at the second vertex of the triangle (float)
 * \param y1 y coordinate at the second vertex of the triangle (float)
 *
 */
void
nema_raster_triangle_p1_f(float x1, float y1)
{
    int x1fx = nema_f2fx(x1);    int y1fx = nema_f2fx(y1);

    nema_multi_union_t *cmd_array;
    CL_GET_SPACE_U(3);

    cmd_array[0].u = NEMA_DRAW_PT1_X; cmd_array[1].i = x1fx;
    cmd_array[2].u = NEMA_DRAW_PT1_Y; cmd_array[3].i = y1fx;
    cmd_array[4].u = NEMA_DRAW_CMD | EXTRA_HOLD  ;

    uint32_t flags = nema_context.draw_flags;
    if ( (flags & (uint32_t)RAST_AA_E2) != 0U) {
        flags |= RAST_AA_E3;
    } else {
        flags &= ~(RAST_AA_E3);
    }

    cmd_array[5].u = flags | DRAW_TRIANGLE;
}

/** \brief Program Rasterizer to draw a triangle for NemaP
 *
 * \param x2 x coordinate at the third vertex of the triangle (float)
 * \param y2 y coordinate at the third vertex of the triangle (float)
 *
 */
void
nema_raster_triangle_p2_f(float x2, float y2)
{
    int x2fx = nema_f2fx(x2);    int y2fx = nema_f2fx(y2);

    nema_multi_union_t *cmd_array;
    CL_GET_SPACE_U(3);

    cmd_array[0].u = NEMA_DRAW_PT2_X; cmd_array[1].i = x2fx;
    cmd_array[2].u = NEMA_DRAW_PT2_Y; cmd_array[3].i = y2fx;
    cmd_array[4].u = NEMA_DRAW_CMD | EXTRA_HOLD  ;

    uint32_t flags = nema_context.draw_flags;
    if ( (flags & (uint32_t)RAST_AA_E2) != 0U) {
        flags |= RAST_AA_E3;
    } else {
        flags &= ~(RAST_AA_E3);
    }

    cmd_array[5].u = flags | DRAW_TRIANGLE;
}

// #define RASTERIZER_BUG_WA
// #define RASTERIZER_BUG_WA_BITACCURATE

#define SET_BYPASS_REG(addr, data) do {                                        \
    cmd_array[i++].u = NEMA_BYPASS_ADDR, cmd_array[i++].u = (addr);                \
    cmd_array[i++].u = NEMA_BYPASS_DATA | NEMA_HOLDCMD; cmd_array[i++].i = (data); \
} while(false)

#define SET_BYPASS_REG_U(addr, data) do {                                        \
    cmd_array[i++].u = NEMA_BYPASS_ADDR, cmd_array[i++].u = (addr);                \
    cmd_array[i++].u = NEMA_BYPASS_DATA | NEMA_HOLDCMD; cmd_array[i++].u = (data); \
} while(false)

#ifdef RASTERIZER_BUG_WA
static int32_t fxmul(int32_t a, int32_t b) {
    // 16.16 * 16.16 = 32.32
    int64_t c64 = (int64_t)a*(int64_t)b;
    // 32.32 -> 24.8
    int32_t c32 = (int32_t)(c64/0x1000000);

    if ( ((uint32_t)c64 & 0x800000U) != 0U ) {
        if ( (c32>=0) ) {
            c32 += 1;
        }
        else {
            c32 -= 1;
        }
    }

    return c32;
}
#endif


//-------------------------------------------------------------------------------

/** \brief Program Rasterizer to draw a triangle
 *
 * \param x0fx x coordinate at the first vertex of the triangle (fixed point 16.16)
 * \param y0fx y coordinate at the first vertex of the triangle (fixed point 16.16)
 * \param x1fx x coordinate at the second vertex of the triangle (fixed point 16.16)
 * \param y1fx y coordinate at the second vertex of the triangle (fixed point 16.16)
 * \param x2fx x coordinate at the third vertex of the triangle (fixed point 16.16)
 * \param y2fx y coordinate at the third vertex of the triangle (fixed point 16.16)
 *
 */
void
nema_raster_triangle_fx(int x0fx, int y0fx, int x1fx, int y1fx, int x2fx, int y2fx)
{
#ifndef RASTERIZER_BUG_WA
    {
    nema_multi_union_t *cmd_array;
    CL_GET_SPACE_U(7);

    cmd_array[ 0].u = NEMA_DRAW_PT0_X; cmd_array[ 1].i = x0fx;
    cmd_array[ 2].u = NEMA_DRAW_PT0_Y; cmd_array[ 3].i = y0fx;
    cmd_array[ 4].u = NEMA_DRAW_PT1_X; cmd_array[ 5].i = x1fx;
    cmd_array[ 6].u = NEMA_DRAW_PT1_Y; cmd_array[ 7].i = y1fx;
    cmd_array[ 8].u = NEMA_DRAW_PT2_X; cmd_array[ 9].i = x2fx;
    cmd_array[10].u = NEMA_DRAW_PT2_Y; cmd_array[11].i = y2fx;
    cmd_array[12].u = NEMA_DRAW_CMD | EXTRA_HOLD;


    uint32_t flags = nema_context.draw_flags;
    if ( (flags & (uint32_t)RAST_AA_E2) != 0U) {
        flags |= RAST_AA_E3;
    } else {
        flags &= ~(RAST_AA_E3);
    }

    cmd_array[13].u = flags | DRAW_TRIANGLE;
    }
#else
    *(uint32_t *)&x0fx &= 0xfffff000U;
    *(uint32_t *)&y0fx &= 0xfffff000U;
    *(uint32_t *)&x1fx &= 0xfffff000U;
    *(uint32_t *)&y1fx &= 0xfffff000U;
    *(uint32_t *)&x2fx &= 0xfffff000U;
    *(uint32_t *)&y2fx &= 0xfffff000U;

    int x0i = x0fx/0x10000;
    int y0i = y0fx/0x10000;
    int x1i = x1fx/0x10000;
    int y1i = y1fx/0x10000;
    int x2i = x2fx/0x10000;
    int y2i = y2fx/0x10000;

    int Xstart = x0i;
    int Ystart = y0i;
    if ((y1i < Ystart) || (y1i == Ystart && x1i < Xstart) ) {
        Xstart = x1i, Ystart = y1i;
    }

    if ((y2i < Ystart) || (y2i == Ystart && x2i < Xstart)) {
        Xstart = x2i, Ystart = y2i;
    }

    if (Xstart < 0) {
        Xstart = 0;
    }

    if (Ystart < 0) {
        Ystart = 0;
    }
    int E0dx =  (x0fx - x1fx);
    int E0dy =  (y1fx - y0fx);
    int E1dx =  (x1fx - x2fx);
    int E1dy =  (y2fx - y1fx);
    int E2dx =  (x2fx - x0fx);
    int E2dy =  (y0fx - y2fx);

    int Xstart1 = Xstart*0x10000 + 0x8000;
    int Ystart1 = Ystart*0x10000 + 0x8000;

    int E0S, E1S, E2S;

    E0S  = fxmul(E0dx,  Ystart1-y0fx);
    E0S += fxmul(E0dy,  Xstart1-x0fx);
    E1S  = fxmul(E1dx,  Ystart1-y1fx);
    E1S += fxmul(E1dy,  Xstart1-x1fx);
    E2S  = fxmul(E2dx,  Ystart1-y2fx);
    E2S += fxmul(E2dy,  Xstart1-x2fx);

#ifdef RASTERIZER_BUG_WA_BITACCURATE
    // imitate right binary arithmetic shift
    int area = 0;
    area += E0S > 0 ? E0S/2 : (E0S-1)/2;
    area += E1S > 0 ? E1S/2 : (E1S-1)/2;
    area += E2S > 0 ? E2S/2 : (E2S-1)/2;
#else
    int area = E0S/2+E1S/2+E2S/2;
#endif

    if( area < 0 ) {
        if ( 0U != (nema_context.draw_flags & RAST_SETUP_CULL_CW) ) {
            return;
        }
    }
    else {
        if ( 0U != (nema_context.draw_flags & RAST_SETUP_CULL_CCW) ) {
            return;
        }
    }

    E0dx /= 0x100;
    E0dy /= 0x100;
    E1dx /= 0x100;
    E1dy /= 0x100;
    E2dx /= 0x100;
    E2dy /= 0x100;

    {
    // // setup clip
    // int clip_minx_fx = nema_min2(nema_min2(x0fx, x1fx), x2fx);
    // int clip_miny_fx = nema_min2(nema_min2(y0fx, y1fx), y2fx);

    // // floor
    // int clip_minx = (clip_minx_fx/0x10000);
    // int clip_miny = (clip_miny_fx/0x10000);

    int clip_maxx = nema_max2(nema_max2(x0fx, x1fx), x2fx);
    clip_maxx = (clip_maxx /*+0xffff*/)/0x10000;
    ++clip_maxx;
    int clip_maxy = nema_max2(nema_max2(y0fx, y1fx), y2fx);
    clip_maxy = (clip_maxy /*+0xffff*/)/0x10000;
    ++clip_maxy;

    nema_multi_union_t *cmd_array;
    CL_GET_SPACE_U(3+2);
    int i = 0;
    SET_BYPASS_REG(0x06U, 0);
    cmd_array[i++].u = NEMA_DRAW_STARTXY;
    // cmd_array[i++].u = YX16TOREG32(clip_miny, clip_minx);
    cmd_array[i++].u = 0U;
    cmd_array[i++].u = NEMA_DRAW_ENDXY  ;
    cmd_array[i++].u = YX16TOREG32(clip_maxy, clip_maxx);
    cmd_array[i++].u = NEMA_DRAW_CMD | EXTRA_HOLD;
    cmd_array[i++].u = nema_context.draw_flags | RAST_SETUP_CULL_CCW | RAST_SETUP_CULL_CW | DRAW_BOX;
    }

    {
    nema_multi_union_t *cmd_array;
    int commands = 15;
    if (area < 0) {
        ++commands;
    }
    CL_GET_SPACE_U(commands*2);
    int i = 0;
    SET_BYPASS_REG(0x08U , Xstart * 0x10000);      // BYPASS_PT0_X
    SET_BYPASS_REG(0x09U , Ystart * 0x10000);      // BYPASS_PT1_X
    SET_BYPASS_REG(0x12U , E0S   );                // BYPASS_E0S
    SET_BYPASS_REG(0x13U , E0dx  );                // BYPASS_E0dx
    SET_BYPASS_REG(0x14U , E0dy  );                // BYPASS_E0dy
    SET_BYPASS_REG(0x15U , E1S   );                // BYPASS_E1S
    SET_BYPASS_REG(0x16U , E1dx  );                // BYPASS_E1dx
    SET_BYPASS_REG(0x17U , E1dy  );                // BYPASS_E1dy
    SET_BYPASS_REG(0x18U , E2S   );                // BYPASS_E2S
    SET_BYPASS_REG(0x19U , E2dx  );                // BYPASS_E2dx
    SET_BYPASS_REG(0x1aU , E2dy  );                // BYPASS_E2dy
    SET_BYPASS_REG(0x1bU , E2S   );                // BYPASS_E2S
    SET_BYPASS_REG(0x1cU , E2dx  );                // BYPASS_E2dx
    SET_BYPASS_REG(0x1dU , E2dy  );                // BYPASS_E2dy
    if (area < 0) {
    SET_BYPASS_REG(0x1eU , 1     );                //BYPASS_NEG_AREA
    }
    uint32_t flags = nema_context.draw_flags & (~(uint32_t)NEMA_CULL_ALL);

    if ( (flags & (uint32_t)RAST_AA_E2) != 0U) {
        flags |= RAST_AA_E3;
    } else {
        flags &= ~(RAST_AA_E3);
    }

    SET_BYPASS_REG_U(0x2bU, flags | DRAW_TRIANGLE); // BYPASS_CMD
    }
#endif
}

//-------------------------------------------------------------------------------

/** \brief Program Rasterizer to generate a pixel
 *
 * \param x x coordinate of the pixel
 * \param y y coordinate of the pixel
 *
 */
void nema_raster_pixel(int x, int y) {
    nema_raster_rect(x, y, 1, 1);
}

//-------------------------------------------------------------------------------

/** \brief Program Rasterizer to generate a rectangle
 *
 * \param x x coordinate of the upper left vertex of the rectangle
 * \param y y coordinate at the upper left vertex of the rectangle
 * \param w width of the rectangle
 * \param h height of the rectangle
 *
 */
void
nema_raster_rect(int x, int y, int w, int h)
{
    if ( (w <= 0) || (h <= 0)) {
        return;
    }

    uint32_t *cmd_array;
    CL_GET_SPACE(3);

    cmd_array[ 0] = NEMA_DRAW_STARTXY;
    cmd_array[ 1] = YX16TOREG32(y, x);;
    cmd_array[ 2] = NEMA_DRAW_ENDXY  ;
    int y1 = y+h;
    int x1 = x+w;
    cmd_array[ 3] = YX16TOREG32(y1, x1);
    cmd_array[ 4] = NEMA_DRAW_CMD | EXTRA_HOLD;
    cmd_array[ 5] = nema_context.draw_flags | DRAW_BOX;
}


//-------------------------------------------------------------------------------

/** \brief Program Rasterizer to draw a rectangle with rounded edges
 *
 * \param x0 x coordinate of the upper left vertex of the rectangle
 * \param y0 y coordinate at the upper left vertex of the rectangle
 * \param w  width of the rectangle
 * \param h  height of the rectangle
 * \param r  corner radius
 *
 */
void nema_raster_rounded_rect(int x0, int y0, int w, int h, int r) {
    int r_ = r;

    if ( (w/2) < r_) {
        r_ = (w/2);
    }
    if ( (h/2) < r_) {
        r_ = (h/2);
    }

    if (r_ <= 0) {
        nema_raster_rect(x0, y0, w, h);
        return;
    }

    // Calculate cmds needed for operation
    int x=0, y=r_;
    int d=3-(2*r_);

    int x_start = x, y_start = y, d_start = d;
    bool raster_rect_needed =  ((h-(2*r_)) > 0 ) ? true : false ;

    int cmd_needed = 0;

    if(raster_rect_needed){
        cmd_needed += (int)NEMA_RASTER_RECT_SIZE;
    }

    while(x<=y) {
        if (x != 0) {
            cmd_needed += (int)NEMA_RASTER_LINE_SIZE * 2;
        }

        if (d<0) {
            d += (4*x)+6;
        }
        else {
            if (x != y) {
                cmd_needed += (int)NEMA_RASTER_LINE_SIZE * 2;
            } else {
                //don't do anything
            }

            d += (4*(x-y))+10;
            y -= 1;
        }
        x++;
    }

    int ret = nema_cl_enough_space(cmd_needed);
    if(ret < 0){
        return;
    }

    // execute command if there is enough space
    int x_l = x0+r_;       int y_t = y0+r_;
    int x_r = (x0+w-r_)-1; int y_b = (y0+h-r_)-1;

    x = x_start;
    y = y_start;
    d = d_start;

    if (raster_rect_needed) {
        nema_raster_rect(x0, y0+r_, w, h-2*r_);
    }

    while(x<=y) {
        if (x != 0) {
            nema_raster_line(x_l-y, y_t-x, x_r+y, y_t-x);
            nema_raster_line(x_l-y, y_b+x, x_r+y, y_b+x);
        }

        if (d<0) {
            d += (4*x)+6;
        }
        else {
            if (x != y) {
                nema_raster_line(x_l-x, y_t-y, x_r+x, y_t-y);
                nema_raster_line(x_l-x, y_b+y, x_r+x, y_b+y);
            } else {
                //don't do anything
            }

            d += (4*(x-y))+10;
            y -= 1;
        }
        x++;
    }
}

/** \brief Program Rasterizer to generate a rectangle
 *
 * \param x x coordinate of the upper left vertex of the rectangle (float)
 * \param y y coordinate at the upper left vertex of the rectangle (float)
 * \param w width of the rectangle (float)
 * \param h height of the rectangle (float)
 *
 */

void
nema_raster_rect_f(float x, float y, float w, float h)
{
    nema_raster_quad_f(x    , y    , x + w, y,
                       x + w, y + h, x    , y + h);
}

/** \brief Program Rasterizer to generate a rectangle
 *
 * \param x x coordinate of the upper left vertex of the rectangle (fixed point 16.16)
 * \param y y coordinate at the upper left vertex of the rectangle (fixed point 16.16)
 * \param w width of the rectangle (fixed point 16.16)
 * \param h height of the rectangle (fixed point 16.16)
 *
 */
void
nema_raster_rect_fx(int xfx, int yfx, int wfx, int hfx)
{
    nema_raster_quad_fx(xfx,       yfx,
                        xfx + wfx, yfx,
                        xfx + wfx, yfx + hfx,
                        xfx,       yfx + hfx);
}

/** \brief Program Rasterizer to draw a quadrilateral for NemaP
 *
 * \param x0fx x coordinate at the first vertex of the quadrilateral (floa)
 * \param y0fx y coordinate at the first vertex of the quadrilateral (floa)
 * \param x1fx x coordinate at the second vertex of the quadrilateral (float)
 * \param y1fx y coordinate at the second vertex of the quadrilateral (float)
 * \param x2fx x coordinate at the third vertex of the quadrilateral (float)
 * \param y2fx y coordinate at the third vertex of the quadrilateral (float)
 * \param x3fx x coordinate at the fourth vertex of the quadrilateral (float)
 * \param y3fx y coordinate at the fourth vertex of the quadrilateral (float)
 *
*/

void
nema_raster_quad_f(float x0, float y0, float x1, float y1,
                   float x2, float y2, float x3, float y3)
{
    int x0fx = nema_f2fx(x0);    int y0fx = nema_f2fx(y0);
    int x1fx = nema_f2fx(x1);    int y1fx = nema_f2fx(y1);
    int x2fx = nema_f2fx(x2);    int y2fx = nema_f2fx(y2);
    int x3fx = nema_f2fx(x3);    int y3fx = nema_f2fx(y3);

    nema_raster_quad_fx(x0fx, y0fx, x1fx, y1fx, x2fx, y2fx, x3fx, y3fx);
}

//-------------------------------------------------------------------------------

/** \brief Program Rasterizer to draw a quadrilateral
 *
 * \param x0fx x coordinate at the first vertex of the quadrilateral (fixed point 16.16)
 * \param y0fx y coordinate at the first vertex of the quadrilateral (fixed point 16.16)
 * \param x1fx x coordinate at the second vertex of the quadrilateral (fixed point 16.16)
 * \param y1fx y coordinate at the second vertex of the quadrilateral (fixed point 16.16)
 * \param x2fx x coordinate at the third vertex of the quadrilateral (fixed point 16.16)
 * \param y2fx y coordinate at the third vertex of the quadrilateral (fixed point 16.16)
 * \param x3fx x coordinate at the fourth vertex of the quadrilateral (fixed point 16.16)
 * \param y3fx y coordinate at the fourth vertex of the quadrilateral (fixed point 16.16)
 *
 */
void
nema_raster_quad_fx(int x0fx, int y0fx,
                    int x1fx, int y1fx, int x2fx, int y2fx, int x3fx, int y3fx)
{
#ifndef RASTERIZER_BUG_WA
    nema_multi_union_t *cmd_array;
    CL_GET_SPACE_U(9);

    cmd_array[ 0].u = NEMA_DRAW_PT0_X; cmd_array[ 1].i = x0fx;
    cmd_array[ 2].u = NEMA_DRAW_PT0_Y; cmd_array[ 3].i = y0fx;
    cmd_array[ 4].u = NEMA_DRAW_PT1_X; cmd_array[ 5].i = x1fx;
    cmd_array[ 6].u = NEMA_DRAW_PT1_Y; cmd_array[ 7].i = y1fx;
    cmd_array[ 8].u = NEMA_DRAW_PT2_X; cmd_array[ 9].i = x2fx;
    cmd_array[10].u = NEMA_DRAW_PT2_Y; cmd_array[11].i = y2fx;
    cmd_array[12].u = NEMA_DRAW_PT3_X; cmd_array[13].i = x3fx;
    cmd_array[14].u = NEMA_DRAW_PT3_Y; cmd_array[15].i = y3fx;
    cmd_array[16].u = NEMA_DRAW_CMD | EXTRA_HOLD  ;
    cmd_array[17].u = nema_context.draw_flags | DRAW_QUAD;
#else
    *(uint32_t *)&x0fx &= 0xfffff000U;
    *(uint32_t *)&y0fx &= 0xfffff000U;
    *(uint32_t *)&x1fx &= 0xfffff000U;
    *(uint32_t *)&y1fx &= 0xfffff000U;
    *(uint32_t *)&x2fx &= 0xfffff000U;
    *(uint32_t *)&y2fx &= 0xfffff000U;
    *(uint32_t *)&x3fx &= 0xfffff000U;
    *(uint32_t *)&y3fx &= 0xfffff000U;

    int x0i = x0fx/0x10000;
    int y0i = y0fx/0x10000;
    int x1i = x1fx/0x10000;
    int y1i = y1fx/0x10000;
    int x2i = x2fx/0x10000;
    int y2i = y2fx/0x10000;
    int x3i = x3fx/0x10000;
    int y3i = y3fx/0x10000;

    int Xstart = x0i;
    int Ystart = y0i;
    if ((y1i < Ystart) || (y1i == Ystart && x1i < Xstart) ) {
        Xstart = x1i, Ystart = y1i;
    }

    if ((y2i < Ystart) || (y2i == Ystart && x2i < Xstart)) {
        Xstart = x2i, Ystart = y2i;
    }

    if ((y3i < Ystart) || (y3i == Ystart && x3i < Xstart)) {
        Xstart = x3i, Ystart = y3i;
    }

    if (Xstart < 0) {
        Xstart = 0;
    }

    if (Ystart < 0) {
        Ystart = 0;
    }
    int E0dx =  (x0fx - x1fx);
    int E0dy =  (y1fx - y0fx);
    int E1dx =  (x1fx - x2fx);
    int E1dy =  (y2fx - y1fx);
    int E2dx =  (x2fx - x3fx);
    int E2dy =  (y3fx - y2fx);
    int E3dx =  (x3fx - x0fx);
    int E3dy =  (y0fx - y3fx);

    int Xstart1 = Xstart*0x10000 + 0x8000;
    int Ystart1 = Ystart*0x10000 + 0x8000;

    int E0S, E1S, E2S, E3S;

    E0S  = fxmul(E0dx,  Ystart1-y0fx);
    E0S += fxmul(E0dy,  Xstart1-x0fx);
    E1S  = fxmul(E1dx,  Ystart1-y1fx);
    E1S += fxmul(E1dy,  Xstart1-x1fx);
    E2S  = fxmul(E2dx,  Ystart1-y2fx);
    E2S += fxmul(E2dy,  Xstart1-x2fx);
    E3S  = fxmul(E3dx,  Ystart1-y3fx);
    E3S += fxmul(E3dy,  Xstart1-x3fx);

#ifdef RASTERIZER_BUG_WA_BITACCURATE
    // imitate right binary arithmetic shift
    int area = 0;
    area += E0S > 0 ? E0S/2 : (E0S-1)/2;
    area += E1S > 0 ? E1S/2 : (E1S-1)/2;
    area += E2S > 0 ? E2S/2 : (E2S-1)/2;
    area += E3S > 0 ? E3S/2 : (E3S-1)/2;
#else
    int area = E0S/2+E1S/2+E2S/2+E3S/2;
#endif

    if( area < 0 ) {
        if ( 0U != (nema_context.draw_flags & RAST_SETUP_CULL_CW) ) {
            return;
        }
    }
    else {
        if ( 0U != (nema_context.draw_flags & RAST_SETUP_CULL_CCW) ) {
            return;
        }
    }

    E0dx /= 0x100;
    E0dy /= 0x100;
    E1dx /= 0x100;
    E1dy /= 0x100;
    E2dx /= 0x100;
    E2dy /= 0x100;
    E3dx /= 0x100;
    E3dy /= 0x100;

    {
    // // setup clip
    // int clip_minx_fx = nema_min2(nema_min2(x0fx, x1fx), nema_min2(x2fx, x3fx));
    // int clip_miny_fx = nema_min2(nema_min2(y0fx, y1fx), nema_min2(y2fx, y3fx));

    // // floor
    // int clip_minx = (clip_minx_fx/0x10000);
    // int clip_miny = (clip_miny_fx/0x10000);

    int clip_maxx = nema_max2(nema_max2(x0fx, x1fx), nema_max2(x2fx, x3fx));
    clip_maxx = (clip_maxx /*+0xffff*/)/0x10000;
    ++clip_maxx;
    int clip_maxy = nema_max2(nema_max2(y0fx, y1fx), nema_max2(y2fx, y3fx));
    clip_maxy = (clip_maxy /*+0xffff*/)/0x10000;
    ++clip_maxy;

    nema_multi_union_t *cmd_array;
    CL_GET_SPACE_U(3+2);
    int i = 0;
    SET_BYPASS_REG(0x06U, 0);
    cmd_array[i++].u = NEMA_DRAW_STARTXY;
    // cmd_array[i++].u = YX16TOREG32(clip_miny, clip_minx);
    cmd_array[i++].u = 0U;
    cmd_array[i++].u = NEMA_DRAW_ENDXY  ;
    cmd_array[i++].u = YX16TOREG32(clip_maxy, clip_maxx);
    cmd_array[i++].u = NEMA_DRAW_CMD | EXTRA_HOLD;
    cmd_array[i++].u = nema_context.draw_flags | RAST_SETUP_CULL_CCW | RAST_SETUP_CULL_CW | DRAW_BOX;
    }

    {
    nema_multi_union_t *cmd_array;
    int commands = 15;
    if ( area < 0 ) {
        ++commands;
    }
    CL_GET_SPACE_U(commands*2);
    int i = 0;
    SET_BYPASS_REG(0x08U , Xstart * 0x10000);      // BYPASS_PT0_X
    SET_BYPASS_REG(0x09U , Ystart * 0x10000);      // BYPASS_PT1_X
    SET_BYPASS_REG(0x12U , E0S   );                // BYPASS_E0S
    SET_BYPASS_REG(0x13U , E0dx  );                // BYPASS_E0dx
    SET_BYPASS_REG(0x14U , E0dy  );                // BYPASS_E0dy
    SET_BYPASS_REG(0x15U , E1S   );                // BYPASS_E1S
    SET_BYPASS_REG(0x16U , E1dx  );                // BYPASS_E1dx
    SET_BYPASS_REG(0x17U , E1dy  );                // BYPASS_E1dy
    SET_BYPASS_REG(0x18U , E2S   );                // BYPASS_E2S
    SET_BYPASS_REG(0x19U , E2dx  );                // BYPASS_E2dx
    SET_BYPASS_REG(0x1aU , E2dy  );                // BYPASS_E2dy
    SET_BYPASS_REG(0x1bU , E3S   );                // BYPASS_E2S
    SET_BYPASS_REG(0x1cU , E3dx  );                // BYPASS_E2dx
    SET_BYPASS_REG(0x1dU , E3dy  );                // BYPASS_E2dy
    if (area < 0) {
    SET_BYPASS_REG(0x1eU , 1     );                //BYPASS_NEG_AREA
    }
    uint32_t flags = nema_context.draw_flags & (~(uint32_t)NEMA_CULL_ALL);
    SET_BYPASS_REG_U(0x2bU, flags | DRAW_TRIANGLE); // BYPASS_CMD
    }
#endif
}

//-------------------------------------------------------------------------------

int
calculate_steps_from_radius(float r) {
    float stepsf = 0.3f*NEMA_PI*r+0.5f;

    int steps = (int)stepsf;

    if ( steps > 96 ) {
        steps = 96;
    }
    else if (steps < 32) {
        steps = 32;
    } else {
        steps  = (steps+15)/16;
        steps  *= 16;
    }

    return steps;
}

int line_intersection_point(float xa0, float ya0, float xa1, float ya1, float xb0, float yb0, float xb1, float yb1, float *x_out, float *y_out){
    int ret = 1;//lines intersect
    // Line AB represented as a1x + b1y = c1
    float a1 = ya1 - ya0;
    float b1 = xa0 - xa1;
    float c1 = a1*(xa0) + b1*(ya0);

    // Line CD represented as a2x + b2y = c2
    float a2 = yb1 - yb0;
    float b2 = xb0 - xb1;
    float c2 = a2*(xb0)+ b2*(yb0);

    float determinant = a1*b2 - a2*b1;

    if ( !nema_float_is_zero(determinant) ) {
        *x_out = (b2*c1 - b1*c2)/determinant;
        *y_out = (a1*c2 - a2*c1)/determinant;
    }
    else
    {
        ret = 0;
    }

    return ret;
}

static const float sins[6] = {
    0.382683432f,
    0.195090322f,
    0.130526192f,
    0.09801714f,
    0.078459096f,
    0.065403129f
};

static const float coss[6] = {
    0.923879533f,
    0.98078528f,
    0.991444861f,
    0.995184727f,
    0.996917334f,
    0.997858923f
};

void
nema_raster_circle(float x, float y, float r) {
    int steps = calculate_steps_from_radius(r);

#if 1
    // use precalculated cos/sin for more accuracy
    steps /= 16;

    float cc = coss[steps-1];
    float ss = sins[steps-1];
    steps *= 4;
#else
    float angle_step = 360.f/steps;

    float cc = nema_cos(angle_step);
    float ss = nema_sin(angle_step);

    steps /= 8;
#endif

    // Calculate steps needed for the operation
    int cmd_needed = 0;
    for (int s = 1; s < steps; ++s) {
        cmd_needed += (int)NEMA_RASTER_QUAD_FX_SIZE * 2;
    }
    cmd_needed += (int)NEMA_RASTER_TRIANGLE_FX_SIZE * 2;
    int ret = nema_cl_enough_space(cmd_needed);
    if(ret < 0){
        return ;
    }

    // Execute command if there is enough space
    float prev_x     = r;   //nema_cos(0)*r;
    float prev_y     = 0.f; //nema_sin(0)*r;
    int prev_xfx     = nema_f2fx(r);
    int prev_yfx     = 0;

    int xfx = nema_f2fx(x);
    int yfx = nema_f2fx(y);

    for (int s = 1; s < steps; ++s) {
        float cur_x = cc * prev_x - ss * prev_y;
        float cur_y = ss * prev_x + cc * prev_y;

        if ((cur_y-prev_y < 0.5f) && (nema_context.scaling == 0)) {
            prev_x   = cur_x;
            prev_y   = cur_y;
            continue;
        }

        int cur_xfx = nema_f2fx(cur_x);
        int cur_yfx = nema_f2fx(cur_y);

        nema_raster_quad_fx(
                            xfx-prev_xfx, yfx+prev_yfx,
                            xfx- cur_xfx, yfx+ cur_yfx,
                            xfx+ cur_xfx, yfx+ cur_yfx,
                            xfx+prev_xfx, yfx+prev_yfx
                            );
        nema_raster_quad_fx(
                            xfx- cur_xfx, yfx- cur_yfx,
                            xfx-prev_xfx, yfx-prev_yfx,
                            xfx+prev_xfx, yfx-prev_yfx,
                            xfx+ cur_xfx, yfx- cur_yfx
                            );

        prev_xfx = cur_xfx;
        prev_yfx = cur_yfx;
        prev_x   = cur_x;
        prev_y   = cur_y;
    }

    nema_raster_triangle_fx(
            xfx         , nema_f2fx(y+r),
            xfx-prev_xfx, yfx+prev_yfx,
            xfx+prev_xfx, yfx+prev_yfx
        );

    nema_raster_triangle_fx(
            xfx         , nema_f2fx(y-r),
            xfx-prev_xfx, yfx-prev_yfx,
            xfx+prev_xfx, yfx-prev_yfx
        );
}

void
nema_raster_circle_aa(float x, float y, float r) {

    uint32_t prev_aa = nema_enable_aa(1, 0, 1, 0);
    nema_raster_circle(x, y, r);
    (void)nema_enable_aa_flags(prev_aa);
}

void
nema_raster_stroked_circle_aa(float x, float y, float r, float w) {    // radius needs to be at least 1.f
    float r_ = r;
    float w_ = w;

    if (r_ < 1.f) {
        r_ = 1.f;
    }

    // width needs to be at least 1.f
    if (w_ < 1.f) {
        w_ = 1.f;
    }

    // half width inside the circle, half outside
    w_ *= 0.5f;

    // if width is >= radius, just fill the circle with color
    if (w_ + 0.5f >= r_) {
        nema_raster_circle_aa(x, y, r_ + w_);
        return;
    }

    int steps = calculate_steps_from_radius(r_);

    // use precalculated cos/sin for more accuracy
    steps /= 16;

    float cc = coss[steps-1];
    float ss = sins[steps-1];
    steps *= 2;

    // Calculate cmds needed for the operation
    int cmd_needed = 0;
    for (int s = 0; s < steps; ++s) {
        cmd_needed += (int)NEMA_RASTER_QUAD_FX_SIZE * 8;
    }
    int ret = nema_cl_enough_space(cmd_needed);
    if(ret < 0){
        return ;
    }


    // Execute operation if there is enogh space left
    float ri = r_ - w_;
    float ro = r_ + w_;

    float prev_xo    = ro;  // nema_cos(0)*ro;
    float prev_yo    = 0.f; // nema_sin(0)*ro;
    float prev_xi    = ri;  // nema_cos(0)*ri;
    float prev_yi    = 0.f; // nema_sin(0)*ri;

    uint32_t prev_aa = nema_enable_aa(0, 1, 0, 1);

    int xfx       = nema_f2fx(x);
    int yfx       = nema_f2fx(y);
    int prev_xofx = nema_f2fx(prev_xo);
    int prev_yofx = nema_f2fx(prev_yo);
    int prev_xifx = nema_f2fx(prev_xi);
    int prev_yifx = nema_f2fx(prev_yi);

    for (int s = 0; s < steps; ++s) {
        float cur_xo = cc * prev_xo - ss * prev_yo;
        float cur_xi = cc * prev_xi - ss * prev_yi;

        float cur_yo, cur_yi;

        if ( s == steps ) {
            cur_yo = cur_xo;
            cur_yi = cur_xi;
        } else {
            cur_yo = ss * prev_xo + cc * prev_yo;
            cur_yi = ss * prev_xi + cc * prev_yi;
        }

        int cur_xofx = nema_f2fx(cur_xo);
        int cur_yofx = nema_f2fx(cur_yo);
        int cur_xifx = nema_f2fx(cur_xi);
        int cur_yifx = nema_f2fx(cur_yi);

        nema_raster_quad_fx((xfx+prev_xifx), (yfx+prev_yifx),
                            (xfx+prev_xofx), (yfx+prev_yofx),
                            (xfx+ cur_xofx), (yfx+ cur_yofx),
                            (xfx+ cur_xifx), (yfx+ cur_yifx));
        nema_raster_quad_fx((xfx+prev_xifx), (yfx-prev_yifx),
                            (xfx+prev_xofx), (yfx-prev_yofx),
                            (xfx+ cur_xofx), (yfx- cur_yofx),
                            (xfx+ cur_xifx), (yfx- cur_yifx));
        nema_raster_quad_fx((xfx-prev_xifx), (yfx+prev_yifx),
                            (xfx-prev_xofx), (yfx+prev_yofx),
                            (xfx- cur_xofx), (yfx+ cur_yofx),
                            (xfx- cur_xifx), (yfx+ cur_yifx));
        nema_raster_quad_fx((xfx-prev_xifx), (yfx-prev_yifx),
                            (xfx-prev_xofx), (yfx-prev_yofx),
                            (xfx- cur_xofx), (yfx- cur_yofx),
                            (xfx- cur_xifx), (yfx- cur_yifx));
        nema_raster_quad_fx((xfx+prev_yifx), (yfx+prev_xifx),
                            (xfx+prev_yofx), (yfx+prev_xofx),
                            (xfx+ cur_yofx), (yfx+ cur_xofx),
                            (xfx+ cur_yifx), (yfx+ cur_xifx));
        nema_raster_quad_fx((xfx+prev_yifx), (yfx-prev_xifx),
                            (xfx+prev_yofx), (yfx-prev_xofx),
                            (xfx+ cur_yofx), (yfx- cur_xofx),
                            (xfx+ cur_yifx), (yfx- cur_xifx));
        nema_raster_quad_fx((xfx-prev_yifx), (yfx+prev_xifx),
                            (xfx-prev_yofx), (yfx+prev_xofx),
                            (xfx- cur_yofx), (yfx+ cur_xofx),
                            (xfx- cur_yifx), (yfx+ cur_xifx));
        nema_raster_quad_fx((xfx-prev_yifx), (yfx-prev_xifx),
                            (xfx-prev_yofx), (yfx-prev_xofx),
                            (xfx- cur_yofx), (yfx- cur_xofx),
                            (xfx- cur_yifx), (yfx- cur_xifx));

        prev_xo   = cur_xo;
        prev_yo   = cur_yo;
        prev_xi   = cur_xi;
        prev_yi   = cur_yi;
        prev_xofx = cur_xofx;
        prev_yofx = cur_yofx;
        prev_xifx = cur_xifx;
        prev_yifx = cur_yifx;
    }

    (void)nema_enable_aa_flags(prev_aa);
}

//-------------------------------------------------------------------------------
void
nema_set_bypass_reg( uint32_t addr, uint32_t data ) {
    nema_cl_add_cmd( NEMA_BYPASS_ADDR, addr);

    if ( addr == BYPASS_CMD ) {
        nema_cl_add_cmd( NEMA_BYPASS_DATA | 0x80000000U, data);
    }
    else {
        nema_cl_add_cmd( NEMA_BYPASS_DATA, data);
    }
}

// ------------------------------ INTERRUPT ------------------------------------
void
nema_set_interrupt_ctrl(uint32_t val)
{
    nema_cl_add_cmd( NEMA_INTERRUPT, val);
}

void
nema_set_interrupt_ctrl_imm(uint32_t val)
{
    nema_reg_write(NEMA_INTERRUPT, val);
}

// -------------------------------- UTILS --------------------------------------
//-------------------------------------------------------------------------------

void
nema_set_depth_range(float min_depth, float max_depth)
{

    nema_multi_union_t val0 = {.f = min_depth};
    nema_cl_add_cmd(NEMA_DEPTH_MIN, val0.u);
    nema_multi_union_t val1 = {.f = max_depth};
    nema_cl_add_cmd(NEMA_DEPTH_MAX, val1.u);
}


static int nema_ceil2(float a){
    int a_i = (int)a;
    float a_if = (float)a_i;
    int a_i2 = 0;
    if (a_if < a){
        a_i2 = 1;

    }else{
        //MISRA
    }

    return a_i + a_i2;
}

void nema_raster_stroked_arc_aa( float x0, float y0, float r, float w, float start_angle, float end_angle){
    nema_raster_stroked_arc_aa_mask(x0, y0, r, w, start_angle, end_angle, 0);
}

void nema_raster_stroked_arc_aa_mask( float x0, float y0, float r, float w, float start_angle, float end_angle, uint32_t aa_mask){
    //Generic nema_raster_stroked_arc with aa_mask
    //User can choose between having or not aa on each of the arc endings (Ideal for arcs with no caps or joints)
    //aa_mask && RAST_AA_E2 == 1 AA on last ending
    //aa_mask && RAST_AA_E0 == 1 AA on first ending
    //edges E1,E2 will always have AA

    //Must always be: start_angle < end_angle
    float r_ = r;
    float w_ = w;

    // radius needs to be at least 1.f
    if (r_ < 1.f) {
        r_ = 1.f;
    }

    // width needs to be at least 1.f
    if (w_ < 1.f) {
        w_ = 1.f;
    }

    // half width inside the circle, half outside
    w_ *= 0.5f;

    // if width is >= radius, saturate r
    if (w_ + 0.5f >= r) {
        r_ = w_;
    }

    if (start_angle == end_angle){
        return;
    }

    int steps  = calculate_steps_from_radius(r_); //steps to draw a whole circle

    float span = nema_absf(end_angle - start_angle);

    float steps_f = (float)steps*span/360.f; //steps to draw an arc
    steps = (int)nema_ceil2(steps_f);

    float angle_step = span/steps_f;
    float ro = r_ + w_;
    float ri = r_ - w_;

    // Calculate cmds needed for the operation
    int cmd_needed = 0;


    float angle = start_angle;

    while (angle <= end_angle ) {
        cmd_needed += (int)NEMA_RASTER_QUAD_FX_SIZE;
        angle += angle_step;
    }

    int ret = nema_cl_enough_space(cmd_needed);
    if(ret < 0){
        return ;
    }


    float prev_xo = ro*nema_cos(start_angle);
    float prev_yo = ro*nema_sin(start_angle);
    float prev_xi = ri*nema_cos(start_angle);
    float prev_yi = ri*nema_sin(start_angle);

    int xfx       = nema_f2fx(x0);
    int yfx       = nema_f2fx(y0);
    int prev_xofx = nema_f2fx(prev_xo);
    int prev_yofx = nema_f2fx(prev_yo);
    int prev_xifx = nema_f2fx(prev_xi);
    int prev_yifx = nema_f2fx(prev_yi);

    uint32_t prev_aa = nema_enable_aa(0, 1, 0, 1);

    //Enable AA for first ending edge if requested
    if ( (aa_mask & RAST_AA_E0) != 0U ) {
        (void) nema_enable_aa(1, 1, 0, 1);
    }

    angle = start_angle;
    for (int s = 0; s < steps; ++s) {
        angle += angle_step;

        float cur_xo;
        float cur_xi;

        float cur_yo;
        float cur_yi;

        //if this is the last step, point is derived by the 'end_angle'
        if ( s == steps - 1 ) {
            //Enable AA for last ending edge if requested
            if ( (aa_mask & RAST_AA_E2) != 0U) {
                //Special case Exactly one quad
                if ( ((aa_mask & RAST_AA_E0) != 0U) && (steps == 1) ) {
                    (void) nema_enable_aa(1, 1, 1, 1);
                }
                else{
                    (void) nema_enable_aa(0, 1, 1, 1);
                }
            }
            cur_xo = ro*nema_cos(end_angle);
            cur_xi = ri*nema_cos(end_angle);

            cur_yo = ro*nema_sin(end_angle);
            cur_yi = ri*nema_sin(end_angle);
        } else {
            cur_xo = ro*nema_cos(angle);
            cur_xi = ri*nema_cos(angle);

            cur_yo = ro*nema_sin(angle);
            cur_yi = ri*nema_sin(angle);
        }

        int cur_xofx = nema_f2fx(cur_xo);
        int cur_yofx = nema_f2fx(cur_yo);
        int cur_xifx = nema_f2fx(cur_xi);
        int cur_yifx = nema_f2fx(cur_yi);

        nema_raster_quad_fx((xfx+prev_xifx), (yfx+prev_yifx),
                            (xfx+prev_xofx), (yfx+prev_yofx),
                            (xfx+ cur_xofx), (yfx+ cur_yofx),
                            (xfx+ cur_xifx), (yfx+ cur_yifx));

        prev_xofx = cur_xofx;
        prev_yofx = cur_yofx;
        prev_xifx = cur_xifx;
        prev_yifx = cur_yifx;
        //Enable aa for outer arc edges
        (void) nema_enable_aa(0, 1, 0, 1);
    }

    (void)nema_enable_aa_flags(prev_aa);
}

//-------------------------------------------------------------------------------

#ifdef NEMA_ENABLE_BREAKPOINTS

void nema_brk_enable(void) {
    nema_reg_write(NEMA_BREAKPOINT_MASK, 0xFFFFU);
}

void nema_brk_disable(void) {
    nema_reg_write(NEMA_BREAKPOINT_MASK, 0);
}

int  nema_brk_add(void) {
    if ( nema_context.breakpoint >= 0x7fff ) {
        nema_context.breakpoint = 1;
    } else {
        ++nema_context.breakpoint;
    }

    nema_cl_add_cmd(NEMAP_HOLDCMD | NEMA_BREAKPOINT, (uint32_t)(int32_t) nema_context.breakpoint);
    nema_cl_add_cmd(0xff800000U | NEMA_INTERRUPT, 1U);

    return (int)nema_context.breakpoint;
}

int nema_brk_wait(int brk_id) {
    int irq = 0;
    int incoming_brk;
    int brk_id_ = brk_id;

    if ( nema_reg_read(NEMA_BREAKPOINT_MASK) == 0U ) {
        //breakpoints are disabled
        return 0;
    }

    if (brk_id_ < 0) {
        brk_id_ = 0;
    }

    bool force_exit = false;
    do {
        incoming_brk = (int)nema_reg_read(NEMA_BREAKPOINT);

        if (incoming_brk == 0) {
            irq = nema_wait_irq_brk(brk_id_);
        }

        if ( brk_id_ > 0 ) {
            if ( incoming_brk == brk_id_ ) {
                return incoming_brk;
            } else {
                if ( incoming_brk != 0) {
                    nema_brk_continue();
                }
            }
        }
        else if ( incoming_brk != 0) {
            force_exit = true;
            // return incoming_brk;
        } else if ( irq < 0 ) {
            force_exit = true;
            // return irq;
        } else {
            //misra
        }
    } while ( !force_exit );

    if ( incoming_brk != 0) {
        return incoming_brk;
    } else if ( irq < 0 ) {
        return irq;
    } else {
        return 0;
    }
}

void nema_brk_continue(void) {
    int incoming_brk = (int)nema_reg_read(NEMA_BREAKPOINT);

    if (incoming_brk != 0) {
        nema_reg_write(NEMA_BREAKPOINT, 0U);
    }
}

static void gpflags_assert(uint32_t hold_mask) {
    uint32_t holdbits = nema_reg_read(NEMA_GP_FLAGS);

    // assert hold mask bits
    holdbits |= hold_mask;
    nema_reg_write(NEMA_GP_FLAGS, holdbits);
}

static void gpflags_deassert(uint32_t hold_mask) {
    uint32_t holdbits = nema_reg_read(NEMA_GP_FLAGS);

    // deassert hold mask bits
    holdbits &= (~hold_mask);
    nema_reg_write(NEMA_GP_FLAGS, holdbits);
}

void nema_ext_hold_irq_enable(uint32_t hold_id) {
    uint32_t hold_mask = ((uint32_t)1U) << hold_id;
    hold_mask &= 0xfU;
    hold_mask <<= 8;

    gpflags_assert(hold_mask);
}

void nema_ext_hold_irq_disable(uint32_t hold_id) {
    uint32_t hold_mask = ((uint32_t)1U) << hold_id;
    hold_mask &= 0xfU;
    hold_mask <<= 8;

    gpflags_deassert(hold_mask);
}

void nema_ext_hold_enable(uint32_t hold_id) {
    uint32_t hold_mask = ((uint32_t)1U) << hold_id;
    hold_mask &= 0xfU;
    hold_mask <<= 4;

    gpflags_assert(hold_mask);
}

void nema_ext_hold_disable(uint32_t hold_id) {
    uint32_t hold_mask = ((uint32_t)1U) << hold_id;
    hold_mask &= 0xfU;
    hold_mask <<= 4;

    gpflags_deassert(hold_mask);
}

void nema_ext_hold_assert_imm(uint32_t hold_id) {
    uint32_t hold_mask = ((uint32_t)1u) << hold_id;
    hold_mask &= 0xfu;

    gpflags_assert(hold_mask);
}

void nema_ext_hold_deassert_imm(uint32_t hold_id) {
    uint32_t hold_mask = ((uint32_t)1u) << hold_id;
    hold_mask &= 0xfu;

    gpflags_deassert(hold_mask);
}

void nema_ext_hold_assert(uint32_t hold_id, int stop) {
    uint32_t hold_flags = 0;
    if ( stop != 0 ) {
        hold_flags = NEMAP_HOLDGPFLAG;
    }

    uint32_t gp_flag = ((uint32_t)1U << hold_id);
    gp_flag &= 0xfu;

    // high bits used as a mask to indicate which GP_FLAG to be forced
    nema_cl_add_cmd( NEMA_GP_FLAGS | hold_flags, (gp_flag << 16) | gp_flag );
}

void nema_ext_hold_deassert(uint32_t hold_id) {
    uint32_t gp_flag = ((uint32_t)1U << hold_id);
    gp_flag &= 0xfu;

    // high bits used as a mask to indicate which GP_FLAG to be forced
    nema_cl_add_cmd( NEMA_GP_FLAGS, (gp_flag << 16U) );
}

#endif // NEMA_ENABLE_BREAKPOINTS

void nema_set_burst_size( int blender_burst_size, int tex_burst_size)
{
    int     tex_burst_size_ = tex_burst_size;
    int blender_burst_size_ = blender_burst_size;

    if((tex_burst_size_ < 0) || (tex_burst_size_ > (int)NEMA_MAX_BURST_SIZE))
    {
        tex_burst_size_ = 0;
    }
    if((blender_burst_size_ < 0) || (blender_burst_size_ > (int)NEMA_MAX_BURST_SIZE))
    {
        blender_burst_size_ = 0;
    }
    //Cast to unsigned for MISRA
    uint32_t unsigned_tex_burst_size     = (uint32_t) tex_burst_size_;
    uint32_t unsigned_blender_burst_size = (uint32_t) blender_burst_size_;
    nema_cl_add_cmd(NEMA_BURST_SIZE,  (unsigned_blender_burst_size << 4U) | unsigned_tex_burst_size);
}

void nema_raster_triangle_strip_f(float* vertices, int num_vertices, int stride)
{
    if(num_vertices < 3){
        return;
    }

    int triangle_count = num_vertices - 2;

    // Print the first triangle
#ifndef RASTERIZER_BUG_WA
    nema_raster_triangle_f(vertices[0],        vertices[1],
                           vertices[stride],   vertices[stride + 1],
                           vertices[2*stride], vertices[2*stride + 1]);

    // In triangle strip adjacent triangles have two vertices equal.
    // Vertices 1, 2, 3 change alternately
    int count = 0;
    int idx;
    for(int i = 1; i < triangle_count; i++){
        idx = i + 2; // First triangle takes the first three vertices.
        if(count == 0){
            nema_raster_triangle_p0_f(vertices[idx*stride], vertices[idx*stride + 1]);
            count++;
        }else if(count == 1){
            nema_raster_triangle_p1_f(vertices[idx*stride], vertices[idx*stride + 1]);
            count++;
        }
        else{
            nema_raster_triangle_p2_f(vertices[idx*stride], vertices[idx*stride + 1]);
            count = 0;
        }
    }
#else
    for(int i = 0; i < triangle_count; i++) {
        nema_raster_triangle_f(vertices[(i  )*stride], vertices[(i  )*stride + 1],
                               vertices[(i+1)*stride], vertices[(i+1)*stride + 1],
                               vertices[(i+2)*stride], vertices[(i+2)*stride + 1]);
    }
#endif
}

void nema_raster_triangle_fan_f(float* vertices, int num_vertices, int stride)
{
    if(num_vertices < 3){
        return;
    }

    int triangle_count = num_vertices - 2;

#ifndef RASTERIZER_BUG_WA
    nema_raster_triangle_f(vertices[0],        vertices[1],
                           vertices[stride],   vertices[stride + 1],
                           vertices[2*stride], vertices[2*stride + 1]);

    // In triangle fan adjacent triangles have two vertices equal.
    // Vertice 0 is always the same in all triangles
    // Vertices 1 and 2 change alternately
    int count = 0;
    int idx;
    for(int i = 1; i < triangle_count; i++){
        idx = i + 2; // First triangle takes the first three vertices.
        if(count == 0){
            nema_raster_triangle_p1_f(vertices[idx*stride], vertices[idx*stride + 1]);
            count++;
        }
        else{
            nema_raster_triangle_p2_f(vertices[idx*stride], vertices[idx*stride + 1]);
            count = 0;
        }
    }
#else
    for(int i = 0; i < triangle_count; i++) {
        nema_raster_triangle_f(vertices[0]           , vertices[1]               ,
                               vertices[(i+1)*stride], vertices[(i+1)*stride + 1],
                               vertices[(i+2)*stride], vertices[(i+2)*stride + 1]);
    }
#endif
}

#ifdef UNIT_TEST
void nema_brk_reset_id(void)
{
    nema_context.breakpoint = 0;
}
#endif // UNIT_TEST
