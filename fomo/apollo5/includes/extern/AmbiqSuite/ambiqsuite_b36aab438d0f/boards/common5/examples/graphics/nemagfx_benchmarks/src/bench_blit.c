//*****************************************************************************
//
//! @file bench_blit.c
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

static int rotation;

static int
i32RenderFrame()
{
    int x = 0;
    int y = 0;

    if (rotation == NEMA_ROT_090_CCW || rotation == NEMA_ROT_270_CCW ||
        rotation == NEMA_ROT_090_CW || rotation == NEMA_ROT_270_CW )
    {
        if (FB_RESX-SZY > 0)
        {
            x = rand() % (FB_RESX - SZY);
        }
        if (FB_RESY-SZX > 0)
        {
            y = rand() % (FB_RESY - SZX);
        }
    }
    else
    {
        if (FB_RESX - SZX > 0)
        {
            x = rand() % (FB_RESX-SZX);
        }
        else
        { }

        if (FB_RESY - SZY > 0)
        {
            y = rand() % (FB_RESY - SZY);
        }
    }

    if (rotation == 0)
    {
        nema_blit(x, y);
    }
    else
    {
        nema_blit_rotate(x, y, rotation);
    }

    return SZX * SZY;
}

//*****************************************************************************
//
//! @brief draw numerous company logos with selected blend mode
//!
//! @param i32BlendMode - blend mode selection.
//! @param i32Rotation  - degrees rotation with counter-clockwise or clockwise.
//!
//! This function could draw numerous logos with selected blend mode.
//!
//! @return the total count of pixels.
//
//*****************************************************************************
int
bench_blit(int i32BlendMode, int i32Rotation)
{
    SZX = g_sAmbiqLogo.w;
    SZY = g_sAmbiqLogo.h;
    rotation = i32Rotation;

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
    nema_bind_src_tex(g_sAmbiqLogo.bo.base_phys, g_sAmbiqLogo.w, g_sAmbiqLogo.h, (nema_tex_format_t)(g_sAmbiqLogo.format), g_sAmbiqLogo.stride, NEMA_FILTER_BL);
    nema_cl_add_cmd(NEMA_BURST_SIZE, nema_burst_reg_value);
    nema_set_const_color(0xff00);
    nema_set_src_color_key(0x404040);
    nema_set_dst_color_key(0x0);
    nema_set_blend_blit(i32BlendMode);

    int i32PixCount = 0;
    i32PixCount += CL_CHECK_SUBMIT(0);

    nema_cl_destroy(&g_sContextCL);
    nema_cl_destroy(&g_sCL0);
    nema_cl_destroy(&g_sCL1);

    return i32PixCount;
}
