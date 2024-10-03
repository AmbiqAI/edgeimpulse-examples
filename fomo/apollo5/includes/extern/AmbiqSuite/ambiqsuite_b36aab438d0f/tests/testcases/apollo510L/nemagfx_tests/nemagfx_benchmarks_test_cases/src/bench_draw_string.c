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

//#define NEMA_FONT_IMPLEMENTATION
#include "DejaVuSerif12pt8b.h"
//#undef NEMA_FONT_IMPLEMENTATION

#include "bench.h"

static char str[] = "Think Silicon\nUltra-low power | vivid graphics";
static int w, h;

//static int _memcpy(uint8_t* dst, const uint8_t* src, uint32_t nbytes)
//{
//    uint32_t i;
//    int count = 0;
//    for (i = 0; i < nbytes; i++)
//    {
//        dst[i] = src[i];
//        count++;
//    }
//
//    return count;
//}

static int render_frame()
{
    int x = 0;
    int y = 0;
    uint32_t col = rand();

    if (FB_RESX > w)
    {
       x = rand() % (FB_RESX - w);
    }

    if (FB_RESY > h)
    {
        y = rand() % (FB_RESY - h);
    }

    nema_print(str, x, y, w, h, col, NEMA_ALIGNX_CENTER | NEMA_TEXT_WRAP | NEMA_ALIGNY_CENTER);

    return sizeof(str) - 1;
}


int bench_draw_string(int blendmode)
{
    if (sCurrentCfg.eTexLocation == TEX_PSRAM)
    {
        DejaVuSerif12pt8b.bo.base_virt = (void *)(MSPI_XIP_BASE_ADDRESS + Ambiq_logo.w*Ambiq_logo.h*4);
        DejaVuSerif12pt8b.bo.base_phys = (uintptr_t)DejaVuSerif12pt8b.bo.base_virt;
        DejaVuSerif12pt8b.bo.size = DejaVuSerif12pt8b.bitmap_size;
    }
    else if (sCurrentCfg.eTexLocation == TEX_SSRAM)
    {
        DejaVuSerif12pt8b.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, DejaVuSerif12pt8b.bitmap_size);
    }
    else
    {
        am_util_stdio_printf("Invalid texture location setting!\n");
    }
    //nema_buffer_map(&DejaVuSerif12pt8b.bo);
    nema_memcpy(DejaVuSerif12pt8b.bo.base_virt, DejaVuSerif12pt8b.bitmap, DejaVuSerif12pt8b.bitmap_size);
    //nema_buffer_flush(&DejaVuSerif12pt8b.bo);

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
    nema_set_blend_blit(blendmode);
    nema_bind_font(&DejaVuSerif12pt8b);
    nema_string_get_bbox(str, &w, &h, FB_RESX, 1);
    //-----------------------------------------------------------------------

    int pix_count = 0;
    nema_bind_font(&DejaVuSerif12pt8b);
    pix_count += CL_CHECK_SUBMIT(ITEMS_PER_CL <= 5 ? 1 : ITEMS_PER_CL / 5);
    nema_buffer_destroy(&DejaVuSerif12pt8b.bo);
    nema_cl_destroy(&context_cl);
    nema_cl_destroy(&cl0);
    nema_cl_destroy(&cl1);

    return pix_count;
}
