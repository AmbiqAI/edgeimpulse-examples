//*****************************************************************************
//
//! @file bench_fill_rect.c
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

#define min2(a, b) ((a) < (b) ? (a) : (b))

static int g_i32PremultiplyColor;

static int
i32RenderFrame()
{
    int i32Size = rand() % (min2(FB_RESX, FB_RESY) / 2);

    int x = rand() % (FB_RESX - i32Size);
    uint32_t col = rand();

    if (g_i32PremultiplyColor != 0)
    {
        col = nema_premultiply_rgba(col);
    }

    int y = rand() % (FB_RESY - i32Size);

    nema_fill_rect(x, y, i32Size, i32Size, col);

    return i32Size * i32Size;
}

//*****************************************************************************
//
//! @brief fill patterns with rectangles
//!
//! @param i32Blend - blend mode selection.
//!
//! This function fill large number of rectangles with blend mode NEMA_BL_SRC_OVER
//! or NEMA_BL_SRC
//!
//! @return the total count of pixels.
//
//*****************************************************************************
int
bench_fill_rect(int i32Blend)
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
    nema_cl_add_cmd(NEMA_BURST_SIZE, nema_burst_reg_value);
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
