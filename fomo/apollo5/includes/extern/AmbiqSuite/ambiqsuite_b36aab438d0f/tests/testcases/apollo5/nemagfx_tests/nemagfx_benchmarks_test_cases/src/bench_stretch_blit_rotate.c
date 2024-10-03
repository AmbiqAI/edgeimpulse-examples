//*****************************************************************************
//
//! @file bench_stretch_blit_rotate.c
//!
//! @brief NemaGFX example.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

// -----------------------------------------------------------------------------
// Copyright (c) 2019 Think Silicon S.A.
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

#include <nema_core.h>

#include "bench.h"

#define PI 3.14159

static int SZX;
static int SZY;

static float scale;
static int rotation = 0;

typedef struct _quad_t
{
    float x0, y0;
    float x1, y1;
    float x2, y2;
    float x3, y3;
} quad_t;

static void transform_quad(quad_t *q, nema_matrix3x3_t m)
{
    q->x0 = 0;          q->y0 = 0;
    q->x1 = Ambiq_logo.w; q->y1 = 0;
    q->x2 = Ambiq_logo.w; q->y2 = Ambiq_logo.h;
    q->x3 = 0;          q->y3 = Ambiq_logo.h;

    nema_mat3x3_mul_vec(m, &q->x0, &q->y0);
    nema_mat3x3_mul_vec(m, &q->x1, &q->y1);
    nema_mat3x3_mul_vec(m, &q->x2, &q->y2);
    nema_mat3x3_mul_vec(m, &q->x3, &q->y3);
}

static int render_frame()
{
    quad_t quad;
    nema_matrix3x3_t m;
    nema_mat3x3_load_identity(m);
    nema_mat3x3_scale(m, scale, scale);
    nema_mat3x3_rotate(m, --rotation);
    nema_mat3x3_translate(m, FB_RESX / 2, FB_RESY / 2);

    transform_quad(&quad, m);

    nema_blit_quad_fit(quad.x0, quad.y0, quad.x1, quad.y1, quad.x2, quad.y2, quad.x3, quad.y3);

    return SZX*SZY;
}

int bench_stretch_blit_rotate(int blendmode, float _scale, int nema_tex_mode)
{

    SZX = (int)(Ambiq_logo.w * _scale);
    SZY = (int)(Ambiq_logo.h * _scale);

    scale = _scale;
    rotation = 0;

    context_cl = nema_cl_create();
    cl0 = nema_cl_create();
    cl1 = nema_cl_create();
    cl_cur = &cl0;

    nema_cl_bind(&context_cl);
    //Set Clipping Rectangle
    nema_set_clip(0, 0, fb.w, fb.h);
    //Bind Framebuffer
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, (nema_tex_format_t)(fb.format), -1);
    //Set Blending Mode
    nema_bind_src_tex(Ambiq_logo.bo.base_phys, Ambiq_logo.w, Ambiq_logo.h, (nema_tex_format_t)(Ambiq_logo.format), Ambiq_logo.stride, (nema_tex_mode_t)nema_tex_mode/* | NEMA_TEX_MORTON_ORDER*/);
    nema_enable_tiling(sCurrentCfg.eTiling);
    nema_set_blend_blit(blendmode);
    //-----------------------------------------------------------------------

    int pix_count = 0;
    pix_count += CL_CHECK_SUBMIT(0);
    nema_enable_tiling(0);

    nema_cl_destroy(&context_cl);
    nema_cl_destroy(&cl0);
    nema_cl_destroy(&cl1);

    return pix_count;
}
