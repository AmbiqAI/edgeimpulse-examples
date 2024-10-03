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

#include "nema_matrix3x3.h"
#include "nema_utils.h"
#include "nema_vg.h"
#include "ref.rgba.h"

#ifndef DONT_USE_NEMADC
#include "nema_dc.h"
#endif

#define RESX 800
#define RESY 600

#define COLOR_STOPS       5
#define REF_IMG_W         80
#define REF_IMG_H         100

float stops[COLOR_STOPS] = {0.0f, 0.25f, 0.50f, 0.75f, 1.0f};
color_var_t colors[COLOR_STOPS] = {{0  , 0  , 255  , 255}, //blue
                                   {0, 255  , 0  , 255}, //green
                                   {255  , 255, 255  , 255}, //white
                                   {255  , 165, 0, 255}, //orange
                                   {255, 0, 0, 255}}; //red

nema_img_obj_t fb = {{0}, RESX, RESY, RESX*4, 0, NEMA_RGBA8888, 0};
nema_img_obj_t ref_img = {{0}, REF_IMG_W, REF_IMG_H, REF_IMG_W*4, 0, NEMA_RGBA8888, NEMA_FILTER_BL};

#ifndef DONT_USE_NEMADC
nemadc_layer_t dc_layer = {(void *)0, 0, RESX, RESY, -1, 0, 0, RESX, RESY, 0xff, NEMADC_BL_SRC, 0, NEMADC_RGBA8888, 0, 0, 0, 0, 0};
#endif

NEMA_VG_PAINT_HANDLE paint;
nema_matrix3x3_t m_paint;
float texture_center_x = REF_IMG_W/2;
float texture_center_y = REF_IMG_H/2;

static void load_objects(void)
{
    //Load memory objects
    fb.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, fb.stride*fb.h);
    nema_buffer_map(&fb.bo);
    printf("FB: V:%p P:0x%08x\n", (void *)fb.bo.base_virt, fb.bo.base_phys);

    ref_img.bo = nema_buffer_create(ref_rgba_len);
    nema_memcpy(ref_img.bo.base_virt, ref_rgba, ref_rgba_len);

#ifndef DONT_USE_NEMADC
    dc_layer.baseaddr_phys = fb.bo.base_phys;
    dc_layer.baseaddr_virt = fb.bo.base_virt;
#endif

    printf("FB: V:%p P:0x%08x\n", (void *)fb.bo.base_virt, fb.bo.base_phys);
}

static void destroy_objects(void)
{
    //Destroy memory objects
    nema_buffer_destroy(&ref_img.bo);

    nema_buffer_unmap(&fb.bo);
    nema_buffer_destroy(&fb.bo);
}

static void
calc_matrix(nema_matrix3x3_t m, float angle_degrees, float x, float y)
{
    float cosa = nema_cos(angle_degrees);
    float sina = nema_sin(angle_degrees);

    m[0][0] = cosa;
    m[0][1] = sina;
    m[0][2] = x - (x*cosa) - (y*sina);
    m[1][0] = -sina;
    m[1][1] = cosa;
    m[1][2] = y + (x*sina) - (y*cosa);
}

#ifdef STANDALONE
int main()
#else
int shapes()
#endif
{
    //Initialize NemaGFX
    if ( nema_init() != 0 ) {
        return -1;
    }

#ifndef DONT_USE_NEMADC
    //Initialize NemaDC
    if ( nemadc_init() != 0 ) {
        return -2;
    }
#endif

    load_objects();

 #ifndef DONT_USE_NEMADC
    //Format        | Pixclock | RESX | FP | SYNC | BP | RESY | FP | SYNC | BP
    //800x600, 60Hz | 40.000   | 800  | 40 | 128  | 88 | 600  | 1  | 4    | 23
    nemadc_timing(800, 40, 128, 88, 600, 1, 4, 23);
    nemadc_set_layer(0, &dc_layer);
#endif

#if 0
    // Use circular cl
    nema_cmdlist_t cl  = nema_cl_create_sized(1024);
    nema_cl_bind_circular(&cl);
#else
    nema_cmdlist_t cl  = nema_cl_create();
    nema_cl_bind(&cl);
#endif

    //clear fb
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, fb.stride);
    nema_set_clip(0, 0, RESX, RESY);
    nema_clear(nema_rgba(0xff,0xff,0x00,0xff)); //yellow

    nema_vg_init(RESX, RESY);

    paint =  nema_vg_paint_create();
    nema_vg_paint_set_stroke_width(paint, 15.0f);
    nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
    nema_vg_set_blend(NEMA_BL_SRC_OVER);

    NEMA_VG_GRAD_HANDLE gradient = nema_vg_grad_create();
    nema_vg_grad_set(gradient, COLOR_STOPS, stops, colors);
    float y_start = 55.0f;

    //--------------FILL---------------------------
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_COLOR);
    nema_vg_paint_set_paint_color(paint, nema_rgba(0xff, 0x00, 0xff, 0x80));

    nema_vg_draw_circle(60.0f, y_start, 50.0f, NULL, paint);

    nema_vg_draw_ellipse(220.0f, y_start, 70.0f, 30.0f, NULL, paint);

    nema_vg_draw_ring(380.0f, y_start, 40.0f, 0.0f, 330.0f, paint);

    nema_vg_draw_line(450.0f, y_start - 40.0f, 550.0f, y_start + 40.0f, NULL, paint);

    nema_vg_draw_rounded_rect(580.0f, y_start - 40.0f, 80.0f, 80.0f, 20.0f, 30.0f, NULL, paint);

    nema_vg_draw_rect(700.0f, y_start - 40.0f, 80.0f, 80.0f, NULL, paint);

    //--------------LINEAR GRADIENT--------------------------

    y_start += 120.0f;

    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_GRAD_LINEAR);
    nema_vg_paint_set_grad_linear(paint, gradient, 100.0f, y_start - 35.0f, 100.0f, y_start + 35.0f, NEMA_TEX_CLAMP | NEMA_FILTER_BL);

    nema_vg_draw_circle(60.0f, y_start, 50.0f, NULL, paint);

    nema_vg_draw_ellipse(220.0f, y_start, 70.0f, 30.0f, NULL, paint);

    nema_vg_draw_ring(380.0f, y_start, 40.0f, 0.0f, 330.0f, paint);

    nema_vg_draw_line(450.0f, y_start - 40.0f, 550.0f, y_start + 40.0f, NULL, paint);

    nema_vg_draw_rounded_rect(580.0f, y_start - 40.0f, 80.0f, 80.0f, 20.0f, 30.0f, NULL, paint);

    nema_vg_draw_rect(700.0f, y_start - 40.0f, 80.0f, 80.0f, NULL, paint);

    //--------------CONIC GRADIENT--------------------------

    y_start += 120.0f;

    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_GRAD_CONICAL);

    nema_vg_paint_set_grad_conical(paint, gradient, 60.0f, y_start, NEMA_TEX_CLAMP  | NEMA_FILTER_BL);
    nema_vg_draw_circle(60.0f, y_start, 50.0f, NULL, paint);

    nema_vg_paint_set_grad_conical(paint, gradient, 220.0f, y_start, NEMA_TEX_CLAMP  | NEMA_FILTER_BL);
    nema_vg_draw_ellipse(220.0f, y_start, 70.0f, 30.0f, NULL, paint);

    nema_vg_paint_set_grad_conical(paint, gradient, 380.0f, y_start, NEMA_TEX_CLAMP  | NEMA_FILTER_BL);
    nema_vg_draw_ring(380.0f, y_start, 40.0f, 0.0f, 330.0f, paint);

    nema_vg_paint_set_grad_conical(paint, gradient, 500.0f, y_start, NEMA_TEX_CLAMP  | NEMA_FILTER_BL);
    nema_vg_draw_line(450.0f, y_start - 40.0f, 550.0f, y_start + 40.0f, NULL, paint);

    nema_vg_paint_set_grad_conical(paint, gradient, 620.0f, y_start, NEMA_TEX_CLAMP  | NEMA_FILTER_BL);
    nema_vg_draw_rounded_rect(580.0f, y_start - 40.0f, 80.0f, 80.0f, 20.0f, 30.0f, NULL, paint);

    nema_vg_paint_set_grad_conical(paint, gradient, 740.0f, y_start, NEMA_TEX_CLAMP  | NEMA_FILTER_BL);
    nema_vg_draw_rect(700.0f, y_start - 40.0f, 80.0f, 80.0f, NULL, paint);


    //--------------RADIAL GRADIENT--------------------------

    y_start += 120.0f;

    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_GRAD_RADIAL);

    nema_vg_paint_set_grad_radial(paint, gradient, 60.0f, y_start, 50, NEMA_TEX_CLAMP | NEMA_FILTER_BL);
    nema_vg_draw_circle(60.0f, y_start, 50.0f, NULL, paint);

    nema_vg_paint_set_grad_radial(paint, gradient, 220.0f, y_start, 50, NEMA_TEX_CLAMP | NEMA_FILTER_BL);
    nema_vg_draw_ellipse(220.0f, y_start, 70.0f, 30.0f, NULL, paint);

    nema_vg_paint_set_grad_radial(paint, gradient, 380.0f, y_start, 50, NEMA_TEX_CLAMP | NEMA_FILTER_BL);
    nema_vg_draw_ring(380.0f, y_start, 40.0f, 0.0f, 330.0f, paint);

    nema_vg_paint_set_grad_radial(paint, gradient, 500.0f, y_start, 50, NEMA_TEX_CLAMP | NEMA_FILTER_BL);
    nema_vg_draw_line(450.0f, y_start - 40.0f, 550.0f, y_start + 40.0f, NULL, paint);

    nema_vg_paint_set_grad_radial(paint, gradient, 620.0f, y_start, 50, NEMA_TEX_CLAMP | NEMA_FILTER_BL);
    nema_vg_draw_rounded_rect(580.0f, y_start - 40.0f, 80.0f, 80.0f, 20.0f, 30.0f, NULL, paint);

    nema_vg_paint_set_grad_radial(paint, gradient, 740.0f, y_start, 50, NEMA_TEX_CLAMP | NEMA_FILTER_BL);
    nema_vg_draw_rect(700.0f, y_start - 40.0f, 80.0f, 80.0f, NULL, paint);

    //--------------TEXTURE--------------------------
    y_start += 120.0f;

    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_TEXTURE);
    nema_vg_paint_set_tex(paint, &ref_img);

    nema_mat3x3_load_identity(m_paint);
    calc_matrix(m_paint, 0.0f, texture_center_x, texture_center_y);
    nema_mat3x3_translate(m_paint, 20.0f, y_start - 50.0f);
    nema_vg_paint_set_tex_matrix(paint, m_paint);
    nema_vg_draw_circle(60.0f, y_start, 50.0f, NULL, paint);

    nema_mat3x3_load_identity(m_paint);
    calc_matrix(m_paint, 90.0f, texture_center_x, texture_center_y);
    nema_mat3x3_translate(m_paint, 180.0f, y_start - 50.0f);
    nema_vg_paint_set_tex_matrix(paint, m_paint);
    nema_vg_draw_ellipse(220.0f, y_start, 70.0f, 30.0f, NULL, paint);

    nema_mat3x3_load_identity(m_paint);
    calc_matrix(m_paint, 180.0f, texture_center_x, texture_center_y);
    nema_mat3x3_translate(m_paint, 330.0f, y_start - 50.0f);
    nema_vg_paint_set_tex_matrix(paint, m_paint);
    nema_vg_draw_ring(380.0f, y_start, 40.0f, 10.0f, 330.0f, paint);

    nema_mat3x3_load_identity(m_paint);
    calc_matrix(m_paint, 180.0f, texture_center_x, texture_center_y);
    nema_mat3x3_translate(m_paint, 440.0f, y_start - 50.0f);
    nema_vg_paint_set_tex_matrix(paint, m_paint);
    nema_vg_draw_line(450.0f, y_start - 40.0f, 550.0f, y_start + 40.0f, NULL, paint);

    nema_mat3x3_load_identity(m_paint);
    calc_matrix(m_paint, 180.0f, texture_center_x, texture_center_y);
    nema_mat3x3_translate(m_paint, 580.0f, y_start - 50.0f);
    nema_vg_paint_set_tex_matrix(paint, m_paint);
    nema_vg_draw_rounded_rect(580.0f, y_start - 40.0f, 80.0f, 80.0f, 20.0f, 30.0f, NULL, paint);

    nema_mat3x3_load_identity(m_paint);
    calc_matrix(m_paint, 270.0f, texture_center_x, texture_center_y);
    nema_mat3x3_translate(m_paint, 700.0f, y_start - 50.0f);
    nema_vg_paint_set_tex_matrix(paint, m_paint);
    nema_vg_draw_rect(700.0f, y_start - 40.0f, 80.0f, 80.0f, NULL, paint);


    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    nema_vg_grad_destroy(gradient);
    nema_vg_paint_destroy(paint);
    nema_vg_deinit();
    destroy_objects();

    return 0;
}
