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
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
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

typedef struct _quad_t
{
    float x0, y0;
    float x1, y1;
    float x2, y2;
    float x3, y3;
}
Quad_t;

static void
transform_quad(Quad_t *psQ, nema_matrix3x3_t sM)
{
    psQ->x0 = 0;          psQ->y0 = 0;
    psQ->x1 = g_sAmbiqLogo.w; psQ->y1 = 0;
    psQ->x2 = g_sAmbiqLogo.w; psQ->y2 = g_sAmbiqLogo.h;
    psQ->x3 = 0;          psQ->y3 = g_sAmbiqLogo.h;

    nema_mat3x3_mul_vec(sM, &psQ->x0, &psQ->y0);
    nema_mat3x3_mul_vec(sM, &psQ->x1, &psQ->y1);
    nema_mat3x3_mul_vec(sM, &psQ->x2, &psQ->y2);
    nema_mat3x3_mul_vec(sM, &psQ->x3, &psQ->y3);
}

static int
i32RenderFrame()
{
    static int i32Rotation = 0;
    Quad_t sQuad;
    nema_matrix3x3_t sM;
    nema_mat3x3_load_identity(sM);
    nema_mat3x3_scale(sM, scale, scale);
    nema_mat3x3_rotate(sM, --i32Rotation);
    nema_mat3x3_translate(sM, FB_RESX / 2, FB_RESY / 2);

    transform_quad(&sQuad, sM);

    nema_blit_quad_fit(sQuad.x0, sQuad.y0, sQuad.x1, sQuad.y1, sQuad.x2, sQuad.y2, sQuad.x3, sQuad.y3);

    return SZX * SZY;
}

//*****************************************************************************
//
//! @brief stretch and rotate numerous company logos with selected blend mode
//!
//! @param i32BlendMode         - blend mode selection.
//! @param fscale               - target zoom scale.
//! @param i32NemaTexMode       - GPU with NEMA_FILTER_PS or NEMA_FILTER_BL.
//!
//! This function could rotate & stretch numerous logos with random positions.
//!
//! @return the total count of pixels.
//
//*****************************************************************************
int
bench_stretch_blit_rotate(int i32BlendMode, float fscale, int i32NemaTexMode)
{

    SZX = (int)(g_sAmbiqLogo.w * fscale);
    SZY = (int)(g_sAmbiqLogo.h * fscale);

    scale = fscale;

    g_sContextCL = nema_cl_create();
    g_sCL0 = nema_cl_create();
    g_sCL1 = nema_cl_create();
    g_psCLCur = &g_sCL0;

    nema_cl_bind(&g_sContextCL);
    //
    // Set Clipping Rectangle
    //
    nema_set_clip(0, 0, FB_RESX, FB_RESY);
    //
    // Bind Framebuffer
    //
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, (nema_tex_format_t)(g_sFB.format), -1);
    //
    // Set Blending Mode
    //
    nema_bind_src_tex(g_sAmbiqLogo.bo.base_phys, g_sAmbiqLogo.w, g_sAmbiqLogo.h, (nema_tex_format_t)(g_sAmbiqLogo.format), g_sAmbiqLogo.stride, (nema_tex_mode_t)i32NemaTexMode);
    nema_cl_add_cmd(NEMA_BURST_SIZE, nema_burst_reg_value);
    nema_set_blend_blit(i32BlendMode);

    int i32PixCount = 0;
    i32PixCount += CL_CHECK_SUBMIT(0);

    nema_cl_destroy(&g_sContextCL);
    nema_cl_destroy(&g_sCL0);
    nema_cl_destroy(&g_sCL1);

    return i32PixCount;
}
