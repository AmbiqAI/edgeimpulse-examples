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
#include "nema_dc.h"
#include "nema_vg.h"
#include "nema_vg_font.h"
#include "DejaVuSans_ttf_kern.h"

#define VECTOR_FONT DejaVuSans_ttf_kern

#define RESX 800
#define RESY 600

nema_img_obj_t fb = {{0}, RESX, RESY, RESX*4, NEMA_RGBA8888};
nemadc_layer_t dc_layer = {(void *)0, 0, RESX, RESY, RESX*4, 0, 0, RESX, RESY, 0xff, NEMADC_BL_SRC, 0, NEMADC_RGBA8888, 0, 0, 0, 0, 0};


void load_objects(void) {
    //Load memory objects
    fb.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, fb.stride*fb.h);
    nema_buffer_map(&fb.bo);

    dc_layer.baseaddr_phys = fb.bo.base_phys;
    dc_layer.baseaddr_virt = fb.bo.base_virt;
}

char str[] = "NemaVG - Hello World!!!\n|-------------------------------|\nThe quick brown fox jumps     over the lazy dog!\n\
ThisIsAVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryLongString\nH NemaGFX υποστηρίζει ΚΑΙ Unicode χαρακτήρες!!!";

// char str[] = "T\0";

int main(int argc, char **argv) {
    //Initialize NemaGFX
    int ret = nema_init();
    if (ret) return ret;
    //Initialize Nema|dc
    ret = nemadc_init();
    if (ret) return ret;

    load_objects();

    //Format        | Pixclock | RESX | FP | SYNC | BP | RESY | FP | SYNC | BP
    //800x600, 60Hz | 40.000   | 800  | 40 | 128  | 88 | 600  | 1  | 4    | 23
    nemadc_timing(800, 40, 128, 88, 600, 1, 4, 23);
    nemadc_set_layer(0, &dc_layer);

    //Create Command List
    nema_cmdlist_t cl  =  nema_cl_create_sized(10*1024);
    //Bind Command List
    nema_cl_bind_circular(&cl);

    //Bind Framebuffer
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, fb.stride);
    //Set Clipping Rectangle
    nema_set_clip(0, 0, RESX, RESY);

    //Set Blending Mode
    nema_set_blend_fill(NEMA_BL_SRC);
    //Fill Rectangle with Black (Clear)
    nema_fill_rect(0, 0, RESX, RESY, 0);

    //initialize nema_vg
    nema_vg_init(RESX, RESY);

    NEMA_VG_PAINT_HANDLE paint =  nema_vg_paint_create();
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_COLOR);
    nema_vg_paint_set_stroke_width(paint, 0.f);

    float font_size = 12.f;
    int x_off = 10;
    int y_off = 100;
    int idx = 0;
    uint32_t rgba8888 = nema_rgba(255, 255, 0, 255 );

    nema_vg_bind_font(&VECTOR_FONT);

    int spacing = 10;
    x_off = 0;
    y_off = 0;
    int w = RESX/2 - spacing/2;
    int h = RESY/2 - spacing/2;
    nema_set_blend_fill(NEMA_BL_SIMPLE);
    nema_vg_set_font_size(font_size);

    nema_vg_paint_set_paint_color(paint, rgba8888);
    nema_set_clip(x_off, y_off, w, h);
    nema_fill_rect(x_off, y_off, w, h, 0x70707070U);
    nema_vg_print(paint, str, (float)x_off, (float)y_off, (float)w, (float)h, NEMA_VG_ALIGNX_LEFT | NEMA_VG_TEXT_WRAP | NEMA_VG_ALIGNY_TOP, NULL);

    x_off = w + spacing/2;
    rgba8888 = nema_rgba(255, 0, 0, 255 );
    nema_vg_paint_set_paint_color(paint, rgba8888);
    nema_set_blend_fill(NEMA_BL_SIMPLE);
    nema_set_clip(x_off, y_off, w, h);
    nema_fill_rect(x_off, y_off, w, h, 0x70707070U);
    nema_vg_print(paint, str, (float)x_off, (float)y_off, (float)w, (float)h, NEMA_VG_ALIGNX_RIGHT | NEMA_VG_TEXT_WRAP | NEMA_VG_ALIGNY_BOTTOM, NULL);

    x_off = w + spacing/2;
    y_off = h + spacing/2;
    rgba8888 = nema_rgba(0, 255, 255, 255 );
    nema_vg_paint_set_paint_color(paint, rgba8888);
    nema_set_blend_fill(NEMA_BL_SIMPLE);
    nema_set_clip(x_off, y_off, w, h);
    nema_fill_rect(x_off, y_off, w, h, 0x70707070U);
    nema_vg_print(paint, str, (float)x_off, (float)y_off, (float)w, (float)h, NEMA_VG_ALIGNX_CENTER | NEMA_VG_TEXT_WRAP | NEMA_VG_ALIGNY_CENTER, NULL);

    x_off = 0;
    y_off = h + spacing/2;
    rgba8888 = nema_rgba(0, 255, 0, 255 );
    nema_vg_paint_set_paint_color(paint, rgba8888);
    nema_set_blend_fill(NEMA_BL_SIMPLE);
    nema_set_clip(x_off, y_off, w, h);
    nema_fill_rect(x_off, y_off, w, h, 0x70707070U);
    nema_vg_print(paint, str, (float)x_off, (float)y_off, (float)w, (float)h, NEMA_VG_ALIGNX_JUSTIFY | NEMA_VG_TEXT_WRAP | NEMA_VG_ALIGNY_JUSTIFY, NULL);

    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    //Destroy (free) Command List
    nema_cl_destroy(&cl);

    //de-initialize nema_vg
    nema_vg_paint_destroy(paint);
    nema_vg_deinit();

    return 0;
}
