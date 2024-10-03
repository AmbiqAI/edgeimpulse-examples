//*****************************************************************************
//
//! @file bench_draw_stroked_arc_aa.c
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
#include "nema_raster.h"

#define PI 3.14159

static int
i32RenderFrame()
{
    int i32Radius = nema_rand() % 225 + 25;

    int x = nema_rand() % (FB_RESX - 2 * i32Radius) + i32Radius;
    uint32_t col = nema_rand();
    int y = nema_rand() % (FB_RESY - 2 * i32Radius) + i32Radius;

    float w = (float)(nema_rand() % 32);
    float start_angle = (float)(nema_rand() % 180);
    float end_angle = start_angle + (float)(nema_rand() % 180);

    nema_set_raster_color(col);
    nema_raster_stroked_arc_aa(x, y, i32Radius, w, start_angle, end_angle);

    return (int)(PI * 2 * i32Radius);
}

int
bench_draw_stroked_arc_aa(int blendmode)
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
    nema_set_blend_fill(blendmode);

    int i32PixCount = 0;
    i32PixCount += CL_CHECK_SUBMIT(0);

    nema_cl_destroy(&g_sContextCL);
    nema_cl_destroy(&g_sCL0);
    nema_cl_destroy(&g_sCL1);

    return i32PixCount;
}
