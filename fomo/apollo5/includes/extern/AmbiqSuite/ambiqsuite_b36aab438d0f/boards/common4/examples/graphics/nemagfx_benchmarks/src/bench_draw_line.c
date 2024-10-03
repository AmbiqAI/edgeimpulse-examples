//*****************************************************************************
//
//! @file bench_draw_line.c
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

static int g_i32PremultiplyColor;

static int
i32RenderFrame()
{
    int x0 = nema_rand() % (FB_RESX);
    int x1 = nema_rand() % (FB_RESX);
    uint32_t col = nema_rand();
    int y0 = nema_rand() % (FB_RESY);
    int y1 = nema_rand() % (FB_RESY);

    if (g_i32PremultiplyColor != 0)
    {
        col = nema_premultiply_rgba(col);
    }

    nema_draw_line(x0, y0, x1, y1, col);

    if ( nema_abs(y0-y1) > nema_abs(x0-x1) )
    {
        return nema_abs(y0-y1);
    }
    else
    {
        return nema_abs(x0-x1);
    }
}

//*****************************************************************************
//
//! @brief draw lines
//!
//! @param i32Blend - blend mode selection.
//!
//! This function could draw numerous lines with selected blend mode.
//!
//! @return the total count of pixels.
//
//*****************************************************************************
int
bench_draw_line(int i32Blend)
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
    //
    // Set Clipping Rectangle
    //
    nema_set_clip(0, 0, FB_RESX, FB_RESY);
    //
    // Set Blending Mode
    //
    nema_set_blend_fill(i32Blend ? NEMA_BL_SRC_OVER : NEMA_BL_SRC);
    g_i32PremultiplyColor = i32Blend;

    int i32PixCount = 0;
    i32PixCount += CL_CHECK_SUBMIT(0);

    nema_cl_destroy(&g_sContextCL);
    nema_cl_destroy(&g_sCL0);
    nema_cl_destroy(&g_sCL1);

    return i32PixCount;
}
