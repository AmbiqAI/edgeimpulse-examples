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

#define PI 3.14159

static int SZX;
static int SZY;

static int rotation;

static int render_frame()
{
    int x=0;
    int y=0;

    if (rotation == NEMA_ROT_090_CCW || rotation == NEMA_ROT_270_CCW ||
        rotation == NEMA_ROT_090_CW || rotation == NEMA_ROT_270_CW ) {
        if (RESX-SZY > 0) {
            x = nema_rand()%(RESX-SZY);
        }
        if (RESY-SZX > 0) {
            y = nema_rand()%(RESY-SZX);
        }
    } else {

        if (RESX-SZX > 0) {
            x = nema_rand()%(RESX-SZX);
        } else { }


        if (RESY-SZY > 0) {
            y = nema_rand()%(RESY-SZY);
        }
    }

    if (rotation == 0) {
        nema_blit(x,y);
    }
    else {
        nema_blit_rotate(x, y, rotation);
    }

    return SZX*SZY;
}

float bench_blit(int blendmode, int rotation_)
{
    SZX = TSi_logo.w;
    SZY = TSi_logo.h;
    rotation = rotation_;

    context_cl = nema_cl_create();
    cl0 = nema_cl_create();
    cl1 = nema_cl_create();
    cl_cur = &cl0;

    nema_cl_bind(&context_cl);
    //Set Clipping Rectangle
    nema_set_clip(0, 0, RESX, RESY);
    //Bind Framebuffer
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, -1);
    nema_bind_src_tex(TSi_logo.bo.base_phys, TSi_logo.w, TSi_logo.h, TSi_logo.format, TSi_logo.stride, NEMA_FILTER_BL);
    nema_set_const_color(0xff00);
    nema_set_src_color_key(0x404040);
    nema_set_dst_color_key(0x0);
    nema_set_blend_blit(blendmode);
    //-----------------------------------------------------------------------

    float pix_count = 0;
    pix_count += CL_CHECK_SUBMIT(0);

    nema_cl_destroy(&context_cl);
    nema_cl_destroy(&cl0);
    nema_cl_destroy(&cl1);

    return pix_count;
}
