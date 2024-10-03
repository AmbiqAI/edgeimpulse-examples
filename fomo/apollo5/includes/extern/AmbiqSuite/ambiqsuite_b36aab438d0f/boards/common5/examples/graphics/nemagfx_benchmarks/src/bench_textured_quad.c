//*****************************************************************************
//
//! @file bench_textured_quad.c
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

#include "nema_core.h"
#include "nema_matrix4x4.h"

#include "bench.h"

static int
i32RenderFrame()
{
    int i32BBoxW = rand() % (FB_RESX - 50) + 50;
    int i32BBoxH = rand() % (FB_RESY - 50) + 50;
    int i32BBoxX = rand() % (FB_RESX - i32BBoxW);
    int i32BBoxY = rand() % (FB_RESY - i32BBoxH);

    int x0 = (int)(i32BBoxX + (float)(rand() % (100)) / 100.f * i32BBoxW);
    int y0 = i32BBoxY;
    int x1 = i32BBoxX + i32BBoxW;
    int y1 = (int)(i32BBoxY + (float)(rand() % (100)) / 100.f * i32BBoxH);
    int x2 = (int)(i32BBoxX + (float)(rand() % (100)) / 100.f * i32BBoxW);
    int y2 = i32BBoxY + i32BBoxH;
    int x3 = i32BBoxX;
    int y3 = (int)(i32BBoxY + (float)(rand() % (100)) / 100.f * i32BBoxH);

    nema_blit_quad_fit(x0, y0, x1, y1, x2, y2, x3, y3);

    int area = (int)(0.5f*nema_abs(x0 * (y1 - y3) +
                             x1 * (y2 - y0) +
                             x2 * (y3 - y1) +
                             x3 * (y0 - y2) ));
    return area;
}

//*****************************************************************************
//
//! @brief draw and fit numerous company logos with quadrilaterals
//!
//! @param i32BlendMode     - blend mode selection.
//! @param i32NemaTexMode   - GPU with NEMA_FILTER_PS or NEMA_FILTER_BL.
//!
//! This function could draw and fit numerous company logos with quadrilateral and
//! random positions and size.
//!
//! @return the total count of pixels.
//
//*****************************************************************************
int
bench_textured_quad(int i32BlendMode, int i32NemaTexMode)
{
    g_sContextCL = nema_cl_create();
    g_sCL0 = nema_cl_create();
    g_sCL1 = nema_cl_create();
    g_psCLCur = &g_sCL0;

    nema_cl_bind(&g_sContextCL);
    //
    // Bind Framebuffer
    //
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, (nema_tex_format_t)(g_sFB.format), -1);
    nema_bind_src_tex(g_sAmbiqLogo.bo.base_phys, g_sAmbiqLogo.w, g_sAmbiqLogo.h, (nema_tex_format_t)(g_sAmbiqLogo.format), g_sAmbiqLogo.stride, (nema_tex_mode_t)i32NemaTexMode);
        nema_cl_add_cmd(NEMA_BURST_SIZE, nema_burst_reg_value);
    //
    // Set Clipping Rectangle
    //
    nema_set_clip(0, 0, FB_RESX, FB_RESY);
    //
    // Set Blending Mode
    //
    nema_set_blend_blit(i32BlendMode);
    //-----------------------------------------------------------------------

    int i32PixCount = 0;
    i32PixCount += CL_CHECK_SUBMIT(0);

    nema_cl_destroy(&g_sContextCL);
    nema_cl_destroy(&g_sCL0);
    nema_cl_destroy(&g_sCL1);

    return i32PixCount;
}
