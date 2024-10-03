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

static float scale;

typedef struct _quad_t {
    float x0, y0;
    float x1, y1;
    float x2, y2;
    float x3, y3;
} quad_t;

static void transform_quad(quad_t *q, nema_matrix3x3_t m)
{
    q->x0 = 0;          q->y0 = 0;
    q->x1 = TSi_logo.w; q->y1 = 0;
    q->x2 = TSi_logo.w; q->y2 = TSi_logo.h;
    q->x3 = 0;          q->y3 = TSi_logo.h;

    nema_mat3x3_mul_vec(m, &q->x0, &q->y0);
    nema_mat3x3_mul_vec(m, &q->x1, &q->y1);
    nema_mat3x3_mul_vec(m, &q->x2, &q->y2);
    nema_mat3x3_mul_vec(m, &q->x3, &q->y3);
}

static int render_frame()
{
    static int rotation = 0;
    quad_t quad;
    nema_matrix3x3_t m;
    nema_mat3x3_load_identity(m);
    nema_mat3x3_scale(m, scale, scale);
    nema_mat3x3_rotate(m, --rotation);
    nema_mat3x3_translate(m, RESX/2, RESY/2);

    transform_quad(&quad, m);

    nema_blit_quad_fit(quad.x0, quad.y0, quad.x1, quad.y1, quad.x2, quad.y2, quad.x3, quad.y3);

    return SZX*SZY;
}

float bench_stretch_blit_rotate(int blendmode, float _scale, int nema_tex_mode)
{

    SZX = TSi_logo.w * _scale;
    SZY = TSi_logo.h * _scale;

    scale = _scale;

    context_cl = nema_cl_create();
    cl0 = nema_cl_create();
    cl1 = nema_cl_create();
    cl_cur = &cl0;

    nema_cl_bind(&context_cl);
    //Set Clipping Rectangle
    nema_set_clip(0, 0, RESX, RESY);
    //Bind Framebuffer
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, -1);
    //Set Blending Mode
    nema_bind_src_tex(TSi_logo.bo.base_phys, TSi_logo.w, TSi_logo.h, TSi_logo.format, TSi_logo.stride, nema_tex_mode);
    nema_set_blend_blit(blendmode);
    //-----------------------------------------------------------------------

    float pix_count = 0;
    pix_count += CL_CHECK_SUBMIT(0);

    nema_cl_destroy(&context_cl);
    nema_cl_destroy(&cl0);
    nema_cl_destroy(&cl1);

    return pix_count;
}
