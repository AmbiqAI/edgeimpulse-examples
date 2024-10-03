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
#include "nema_interpolators.h"

static int render_frame()
{
    int x0 = nema_rand()%(RESX);
    int y2 = nema_rand()%(RESY);
    int x1 = nema_rand()%(RESX);
    int y0 = nema_rand()%(RESY);
    int x2 = nema_rand()%(RESX);
    int y1 = nema_rand()%(RESY);

    nema_blit_tri_fit(x0, y0, 0, x1, y1, 1, x2, y2, 2);

    int area = 0.5f * nema_abs(x0 * (y1 - y2) +
                               x1 * (y2 - y0) +
                               x2 * (y0 - y1));

    return area;
}

float bench_textured_tri(int blendmode, int nema_tex_mode)
{
    context_cl = nema_cl_create();
    cl0 = nema_cl_create();
    cl1 = nema_cl_create();
    cl_cur = &cl0;

    nema_cl_bind(&context_cl);
    //Bind Framebuffer
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, -1);
    nema_bind_src_tex(TSi_logo.bo.base_phys, TSi_logo.w, TSi_logo.h, TSi_logo.format, TSi_logo.stride, nema_tex_mode);
    //Set Clipping Rectangle
    nema_set_clip(0, 0, RESX, RESY);
    //Set Blending Mode
    nema_set_blend_blit(blendmode);
    //-----------------------------------------------------------------------

    float pix_count = 0;
    pix_count += CL_CHECK_SUBMIT(0);

    nema_cl_destroy(&context_cl);
    nema_cl_destroy(&cl0);
    nema_cl_destroy(&cl1);

    return pix_count;
}
