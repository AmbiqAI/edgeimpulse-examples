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

#include "nema_core.h"
#include "nema_vg.h"
#include "nema_vg_font.h"
#include "DejaVuSans_ttf_kern.h"

#ifndef DONT_USE_NEMADC
#include "nema_dc.h"
#endif

#define VECTOR_FONT DejaVuSans_ttf_kern

#define RESX 800
#define RESY 600

nema_img_obj_t fb = {{0}, RESX, RESY, RESX*4, 0, NEMA_RGBA8888, NEMA_FILTER_PS};

#ifndef DONT_USE_NEMADC
nemadc_layer_t dc_layer = {(void *)0, 0, RESX, RESY, RESX*4, 0, 0, RESX, RESY, 0xff, NEMADC_BL_SRC, 0, NEMADC_RGBA8888, 0, 0, 0, 0, 0};
#endif

#define COLOR_STOPS       5
float stops[COLOR_STOPS] = {0.0f, 0.25f, 0.50f, 0.75f, 1.0f};
color_var_t colors[COLOR_STOPS] = {{0  , 0  , 0  , 255}, //black
                                   {255, 0  , 0  , 255}, //red
                                   {0  , 255, 0  , 255}, //green
                                   {0  , 0  , 255, 255}, //blue
                                   {255, 255, 255, 255}}; //white
NEMA_VG_PAINT_HANDLE paint;
NEMA_VG_GRAD_HANDLE gradient;

void load_objects(void) {
    //Load memory objects
    fb.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, fb.stride*fb.h);
    nema_buffer_map(&fb.bo);

#ifndef DONT_USE_NEMADC
    dc_layer.baseaddr_phys = fb.bo.base_phys;
    dc_layer.baseaddr_virt = fb.bo.base_virt;
#endif
}

static void destroy_objects(void)
{
    //Destroy memory objects
    nema_buffer_unmap(&fb.bo);
    nema_buffer_destroy(&fb.bo);
}

#ifdef STANDALONE
int main()
#else
int nema_vg_font_paint()
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
    gradient = nema_vg_grad_create();

    nema_vg_grad_set(gradient, COLOR_STOPS, stops, colors);
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_GRAD_LINEAR);
    nema_vg_paint_set_stroke_width(paint, 1);

    float font_size = 32.f;
    float x_off = 10.f;
    float y_off = 100.f;
    float w,h;
    char str[] = "Gradient on fill";
    nema_set_blend_fill(NEMA_BL_SIMPLE);

    nema_vg_bind_font(&VECTOR_FONT);
    nema_vg_set_font_size(font_size);

    nema_vg_string_get_bbox(str, &w, &h, RESX, NEMA_VG_ALIGNX_LEFT);
    nema_vg_paint_set_grad_linear(paint, gradient, x_off, y_off, x_off + w, y_off + h, NEMA_TEX_CLAMP);
    nema_vg_print(paint, str, x_off, y_off, w, h, NEMA_VG_ALIGNX_LEFT, NULL);

    x_off = 350.f;
    char str2[] = "Gradient on stroke";
    nema_vg_set_fill_rule(NEMA_VG_STROKE);
    nema_vg_string_get_bbox(str2, &w, &h, RESX, NEMA_VG_ALIGNX_LEFT);
    nema_vg_paint_set_grad_linear(paint, gradient, x_off, y_off, x_off + w, y_off + h, NEMA_TEX_CLAMP);
    nema_vg_print(paint, str2, x_off, y_off, w, h, NEMA_VG_ALIGNX_LEFT, NULL);

    y_off = 200.f;
    x_off = 10.f;
    nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
    nema_vg_string_get_bbox(str, &w, &h, RESX, NEMA_VG_ALIGNX_LEFT);
    nema_vg_paint_set_grad_linear(paint, gradient, x_off, y_off, x_off + w/2, y_off + h/2, NEMA_TEX_REPEAT);
    nema_vg_print(paint, str, x_off, y_off, w, h, NEMA_VG_ALIGNX_LEFT, NULL);

    x_off = 350.f;
    nema_vg_set_fill_rule(NEMA_VG_STROKE);
    nema_vg_string_get_bbox(str2, &w, &h, RESX, NEMA_VG_ALIGNX_LEFT);
    nema_vg_paint_set_grad_linear(paint, gradient, x_off, y_off, x_off + w/2, y_off + h/2, NEMA_TEX_REPEAT);
    nema_vg_print(paint, str2, x_off, y_off, w, h, NEMA_VG_ALIGNX_LEFT, NULL);

    y_off = 300.f;
    x_off = 10.f;
    nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
    nema_vg_string_get_bbox(str, &w, &h, RESX, NEMA_VG_ALIGNX_LEFT);
    nema_vg_paint_set_grad_linear(paint, gradient, x_off, y_off, x_off + w/2, y_off + h/2, NEMA_TEX_MIRROR);
    nema_vg_print(paint, str, x_off, y_off, w, h, NEMA_VG_ALIGNX_LEFT, NULL);

    x_off = 350.f;
    nema_vg_set_fill_rule(NEMA_VG_STROKE);
    nema_vg_string_get_bbox(str2, &w, &h, RESX, NEMA_VG_ALIGNX_LEFT);
    nema_vg_paint_set_grad_linear(paint, gradient, x_off, y_off, x_off + w/2, y_off + h/2, NEMA_TEX_MIRROR);
    nema_vg_print(paint, str2, x_off, y_off, w, h, NEMA_VG_ALIGNX_LEFT, NULL);

    char str3[] = "Color on fill";
    y_off = 400.f;
    x_off = 10.f;
    nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_COLOR);
    nema_vg_paint_set_paint_color(paint, nema_rgba(0xff, 0, 0, 0xff));
    nema_vg_string_get_bbox(str3, &w, &h, RESX, NEMA_VG_ALIGNX_LEFT);
    nema_vg_print(paint, str3, x_off, y_off, w, h, NEMA_VG_ALIGNX_LEFT, NULL);

    char str4[] = "Color on stroke";
    x_off = 350.f;
    nema_vg_set_fill_rule(NEMA_VG_STROKE);
    nema_vg_string_get_bbox(str4, &w, &h, RESX, NEMA_VG_ALIGNX_LEFT);
    nema_vg_print(paint, str4, x_off, y_off, w, h, NEMA_VG_ALIGNX_LEFT, NULL);

    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    //Destroy (free) Command List
    nema_cl_destroy(&cl);

    //de-initialize nema_vg
    nema_vg_paint_destroy(paint);
    nema_vg_deinit();
    destroy_objects();

    return 0;
}
