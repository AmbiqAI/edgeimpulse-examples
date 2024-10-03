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
#include "nema_interpolators.h"
#include "nema_core.h"
#include "nema_rasterizer.h"
#include "nema_programHW.h"

#define swap(a, b, c)  \
    do {        \
       c = a;   \
       a = b;   \
       b = c;   \
    } while(0)

void
nema_interpolate_rect_colors(int x0, int y0, int w, int h, color_var_t* col0, color_var_t* col1, color_var_t* col2) {
    (void) x0;
    (void) y0;

    float dist_r = col1->r-col0->r;
    float dist_g = col1->g-col0->g;
    float dist_b = col1->b-col0->b;
    float dist_a = col1->a-col0->a;

    float wf = (float)w;
    float r_dx = dist_r/wf;
    float g_dx = dist_g/wf;
    float b_dx = dist_b/wf;
    float a_dx = dist_a/wf;

    dist_r = col2->r-col1->r;
    dist_g = col2->g-col1->g;
    dist_b = col2->b-col1->b;
    dist_a = col2->a-col1->a;

    float hf = (float)h;
    float r_dy = dist_r/hf;
    float g_dy = dist_g/hf;
    float b_dy = dist_b/hf;
    float a_dy = dist_a/hf;

    nema_set_gradient(col0->r, col0->g, col0->b, col0->a, r_dx, r_dy, g_dx, g_dy, b_dx, b_dy, a_dx, a_dy);
}

static inline void linear_interpolate(float x0, float y0,
                                      float x1, float y1,
                                      float x2, float y2,
                                      float w1, float w2, float w3,
                                      float *VS, float *Vdx, float *Vdy,
                                      float DET, float Xstart, float Ystart ) {
    float A, B, C;
    // www1.eonfusion.com/manual/index.php/Formulae_for_interpolation
    A    = ((y1-y2)*w1+(y2-y0)*w2+(y0-y1)*w3);
    B    = ((x2-x1)*w1+(x0-x2)*w2+(x1-x0)*w3);
    C    = ((x1*y2-x2*y1)*w1+(x2*y0-x0*y2)*w2+(x0*y1-x1*y0)*w3);

    //the rasterizer floors RGBA values. Add 0.5f to round to nearest
    // *VS  = (A*Xstart+B*Ystart+C) / DET + 0.5f;
    *VS  = (A*Xstart+B*Ystart+C) / DET;
    *Vdx = A / DET;
    *Vdy = B / DET;
}

static inline void
get_xy_start( float x0, float y0, float x1, float y1, float x2, float y2, float *Xstart, float *Ystart )
{
    float tmpf_;
      tmpf_ = x0+0.5f;
    int x0i = (int)tmpf_;
      tmpf_ = y0+0.5f;
    int y0i = (int)tmpf_;
      tmpf_ = x1+0.5f;
    int x1i = (int)tmpf_;
      tmpf_ = y1+0.5f;
    int y1i = (int)tmpf_;
      tmpf_ = x2+0.5f;
    int x2i = (int)tmpf_;
      tmpf_ = y2+0.5f;
    int y2i = (int)tmpf_;

    *Xstart = (float)x0i;
    *Ystart = (float)y0i;
    if ((float)y1i < *Ystart) {
        *Xstart = (float)x1i;
        *Ystart = (float)y1i;
    }
    if ((float)y1i == *Ystart && (float)x1i < *Xstart ) {
        *Xstart = (float)x1i;
        *Ystart = (float)y1i;
    }
    if ((float)y2i < *Ystart) {
        *Xstart = (float)x2i;
        *Ystart = (float)y2i;
    }
    if ((float)y2i == *Ystart && (float)x2i < *Xstart ) {
        *Xstart = (float)x2i;
        *Ystart = (float)y2i;
    }

    if (*Xstart < 0.f) {
        *Xstart = 0.f;
    }

    if (*Ystart < 0.f) {
        *Ystart = 0.f;
    }
}

void
nema_interpolate_tri_colors(float x0, float y0, float x1, float y1, float x2, float y2, color_var_t* col0, color_var_t* col1, color_var_t* col2) {

    float DET  = x0*y1-x1*y0+x1*y2-x2*y1+x2*y0-x0*y2;

    if ( nema_float_is_zero(DET) ) {
        nema_set_gradient(col0->r, col0->g, col0->b, col0->a,  0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
        return;
    }

    float Xstart, Ystart;
    get_xy_start(x0, y0, x1, y1, x2, y2, &Xstart, &Ystart);

    float r_start, r_dx, r_dy;
    float g_start, g_dx, g_dy;
    float b_start, b_dx, b_dy;
    float a_start, a_dx, a_dy;

    linear_interpolate(x0, y0, x1, y1, x2, y2, col0->r, col1->r, col2->r, &r_start, &r_dx, &r_dy, DET, Xstart, Ystart);
    linear_interpolate(x0, y0, x1, y1, x2, y2, col0->g, col1->g, col2->g, &g_start, &g_dx, &g_dy, DET, Xstart, Ystart);
    linear_interpolate(x0, y0, x1, y1, x2, y2, col0->b, col1->b, col2->b, &b_start, &b_dx, &b_dy, DET, Xstart, Ystart);
    linear_interpolate(x0, y0, x1, y1, x2, y2, col0->a, col1->a, col2->a, &a_start, &a_dx, &a_dy, DET, Xstart, Ystart);

    nema_set_gradient(r_start+0.5f, g_start+0.5f, b_start+0.5f, a_start+0.5f, r_dx, r_dy, g_dx, g_dy, b_dx, b_dy, a_dx, a_dy);
}


void nema_interpolate_tri_depth(float x0, float y0, float z0,
                                float x1, float y1, float z1,
                                float x2, float y2, float z2) {

    float Xstart, Ystart;
    get_xy_start(x0, y0, x1, y1, x2, y2, &Xstart, &Ystart);

    float start, dx, dy;

    float DET  = x0*y1-x1*y0+x1*y2-x2*y1+x2*y0-x0*y2;

    if ( nema_float_is_zero(DET) ){
        start = z0;
        dx = 0.f;
        dy = 0.f;
    }
    else {
        linear_interpolate(x0, y0, x1, y1, x2, y2, z0, z1, z2, &start, &dx, &dy, DET, Xstart, Ystart);
    }
    uint64_t start_d = (uint64_t)start << 32;
    uint32_t starth  = (uint32_t)(start_d >> 32);
    uint32_t startl  = (uint32_t)(start_d & 0xffffffffU);

    uint64_t dx_d = (uint64_t)dx << 32;
    uint32_t dxh  = (uint32_t)(dx_d >> 32);
    uint32_t dxl  = (uint32_t)(dx_d & 0xffffffffU);

    uint64_t dy_d = (uint64_t)dy << 32;
    uint32_t dyh  = (uint32_t)(dy_d >> 32);
    uint32_t dyl  = (uint32_t)(dy_d & 0xffffffffU);

    nema_set_depth_imm(startl, starth, dxl, dxh, dyl, dyh);
}

void nema_interpolate_tx_ty(float x0, float y0, float w0, float tx0, float ty0,
                            float x1, float y1, float w1, float tx1, float ty1,
                            float x2, float y2, float w2, float tx2, float ty2,
                            int tex_width, int tex_height ) {
    (void) tex_width;
    (void) tex_height;

    if ( nema_float_is_zero(w0) || nema_float_is_zero(w1) || nema_float_is_zero(w2) ) {
        return;
    }

    float iz0   = 1.f / w0;
    float iz1   = 1.f / w1;
    float iz2   = 1.f / w2;
    float uiz0x = tx0 * iz0;
    float uiz1x = tx1 * iz1;
    float uiz2x = tx2 * iz2;
    float uiz0y = ty0 * iz0;
    float uiz1y = ty1 * iz1;
    float uiz2y = ty2 * iz2;

    nema_matrix3x3_t _m = {
        {uiz0x , uiz1x , uiz2x},
        {uiz0y, uiz1y, uiz2y},
        { iz0 ,  iz1 ,  iz2 }
    };

    nema_matrix3x3_t m = {
        { x0,  x1,  x2},
        { y0,  y1,  y2},
        {1.f, 1.f, 1.f}
    };

    nema_mat3x3_adj(m);

    nema_mat3x3_mul(_m, m);

    nema_set_matrix(_m);
}
