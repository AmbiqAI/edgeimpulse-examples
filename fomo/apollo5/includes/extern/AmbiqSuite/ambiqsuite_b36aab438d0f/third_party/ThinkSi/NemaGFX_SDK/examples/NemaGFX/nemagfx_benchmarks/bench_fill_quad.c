/* TSI 2023.xmp */
/*******************************************************************************
 * Copyright (c) 2023 Think Silicon Single Member PC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this file and/or associated documentation files to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Materials, and to permit persons to whom the Materials are furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * The software is provided 'as is', without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose and noninfringement. In no event shall Think
 * Silicon Single Member PC be liable for any claim, damages or other liability,
 * whether in an action of contract, tort or otherwise, arising from, out of or
 * in connection with the software or the use or other dealings in the software.
 ******************************************************************************/

#include <nema_core.h>

#include "bench.h"

static int premultiply_color;

static int render_frame()
{
    int bbox_w = nema_rand()%(RESX-50) + 50;
    int bbox_h = nema_rand()%(RESY-50) + 50;
    uint32_t col = nema_rand();
    int bbox_x = nema_rand()%(RESX-bbox_w);
    int bbox_y = nema_rand()%(RESY-bbox_h);

    int x0 = bbox_x + (float)(nema_rand()%(100))/100.f*bbox_w;
    int y0 = bbox_y;
    int x1 = bbox_x + bbox_w;
    int y1 = bbox_y + (float)(nema_rand()%(100))/100.f*bbox_h;
    int x2 = bbox_x + (float)(nema_rand()%(100))/100.f*bbox_w;
    int y2 = bbox_y + bbox_h;
    int x3 = bbox_x;
    int y3 = bbox_y + (float)(nema_rand()%(100))/100.f*bbox_h;

    if (premultiply_color != 0) {
        col = nema_premultiply_rgba(col);
    }

    nema_fill_quad(x0, y0, x1, y1, x2, y2, x3, y3, col);

    int area = 0.5f*nema_abs(x0 * (y1 - y3) +
                             x1 * (y2 - y0) +
                             x2 * (y3 - y1) +
                             x3 * (y0 - y2) );
    return area;
}

float bench_fill_quad(int blend)
{
    context_cl = nema_cl_create();
    cl0 = nema_cl_create();
    cl1 = nema_cl_create();
    cl_cur = &cl0;

    nema_cl_bind(&context_cl);
    //Bind Framebuffer
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, -1);
    //Set Clipping Rectangle
    nema_set_clip(0, 0, RESX, RESY);
    //Set Blending Mode
    nema_set_blend_fill(blend ? NEMA_BL_SRC_OVER : NEMA_BL_SRC);
    //-----------------------------------------------------------------------
    premultiply_color = blend;

    float pix_count = 0;
    pix_count += CL_CHECK_SUBMIT(0);

    nema_cl_destroy(&context_cl);
    nema_cl_destroy(&cl0);
    nema_cl_destroy(&cl1);

    return pix_count;
}
