//*****************************************************************************
//
//! @file bench_draw_rect.c
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

static int premultiply_color;

static int render_frame()
{
    int size = rand() % (min2(FB_RESX, FB_RESY) / 2);

    int x = rand() % (FB_RESX - size);
    uint32_t col = rand();
    int y = rand() % (FB_RESY - size);

    if (premultiply_color != 0)
    {
        col = nema_premultiply_rgba(col);
    }

    nema_draw_rect(x, y, size, size, col);

    return 4*size -4;
}


int bench_draw_rect(int blend)
{
    context_cl = nema_cl_create();
    cl0 = nema_cl_create();
    cl1 = nema_cl_create();
    cl_cur = &cl0;

    nema_cl_bind(&context_cl);
    //Bind Framebuffer
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, (nema_tex_format_t)(fb.format), -1);
    //Set Clipping Rectangle
    nema_set_clip(0, 0, fb.w, fb.h);
    //Set Blending Mode
    nema_set_blend_fill(blend ? NEMA_BL_SRC_OVER : NEMA_BL_SRC);
    //-----------------------------------------------------------------------
    premultiply_color = blend;

    int pix_count = 0;
    pix_count += CL_CHECK_SUBMIT(0);

    nema_cl_destroy(&context_cl);
    nema_cl_destroy(&cl0);
    nema_cl_destroy(&cl1);

    return pix_count;
}
