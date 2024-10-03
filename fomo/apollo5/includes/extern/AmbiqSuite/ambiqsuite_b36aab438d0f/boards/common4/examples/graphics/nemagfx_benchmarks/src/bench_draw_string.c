//*****************************************************************************
//
//! @file bench_draw_string.c
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
#include <nema_font.h>

#include "DejaVuSerif12pt8b.h"
#include "string.h"
#include "bench.h"

static char str[] = "Think Silicon\nUltra-low power | vivid graphics";
static int w, h;

static int
i32RenderFrame()
{
    int x = 0;
    int y = 0;
    uint32_t col = nema_rand();

    if (FB_RESX > w)
    {
       x = nema_rand() % (FB_RESX - w);
    }

    if (FB_RESY > h)
    {
        y = nema_rand() % (FB_RESY - h);
    }

    nema_print(str, x, y, w, h, col, NEMA_ALIGNX_CENTER | NEMA_TEXT_WRAP | NEMA_ALIGNY_CENTER);

    return sizeof(str) - 1;
}

//*****************************************************************************
//
//! @brief draw strings
//!
//! @param blendmode - blend mode selection.
//!
//! This function load font to memory SSRAM,draw test string with selected blend
//! mode.
//!
//! @return the total count of pixels.
//
//*****************************************************************************
int
bench_draw_string(int blendmode)
{
    g_sDejaVuSerif12pt8b.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, g_sDejaVuSerif12pt8b.bitmap_size);
    //nema_buffer_map(&g_sDejaVuSerif12pt8b.bo);
    memcpy(g_sDejaVuSerif12pt8b.bo.base_virt, g_sDejaVuSerif12pt8b.bitmap, g_sDejaVuSerif12pt8b.bitmap_size);
    nema_buffer_flush(&g_sDejaVuSerif12pt8b.bo);

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
    nema_set_blend_blit(blendmode);
    nema_bind_font(&g_sDejaVuSerif12pt8b);
    nema_string_get_bbox(str, &w, &h, FB_RESX, 1);

    int i32PixCount = 0;
    //nema_bind_font(&g_sDejaVuSerif12pt8b);
    i32PixCount += CL_CHECK_SUBMIT(ITEMS_PER_CL <= 5 ? 1 : ITEMS_PER_CL / 5);

    nema_cl_destroy(&g_sContextCL);
    nema_cl_destroy(&g_sCL0);
    nema_cl_destroy(&g_sCL1);

    nema_buffer_destroy(&g_sDejaVuSerif12pt8b.bo);

    return i32PixCount;
}
