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
#include "nema_font.h"
#include "nema_event.h"
#include "nema_utils.h"

#define VECTOR_FONT DejaVuSans_ttf_kern

#define FRAME_BUFFERS 2

#define RESX 800
#define RESY 600

static img_obj_t fb[FRAME_BUFFERS];
static nemadc_layer_t layer[FRAME_BUFFERS] = {{0}};

static char str[]  = "Sample text 0\nSample text 1";

void load_objects(void) {
    int i;
    for (i = 0; i < FRAME_BUFFERS; ++i) {
        fb[i].w      = RESX;
        fb[i].h      = RESY;
        fb[i].format = NEMA_RGBA8888;
        fb[i].stride = RESX*4;
        fb[i].bo     = nema_buffer_create(fb[i].stride*fb[i].h);
        nema_buffer_map(&fb[i].bo);

#ifndef DONT_USE_NEMADC
        layer[i].sizex         = layer[i].resx = fb[i].w;
        layer[i].sizey         = layer[i].resy = fb[i].h;
        layer[i].stride        = fb[i].stride;
        layer[i].format        = NEMADC_RGBA8888;
        layer[i].blendmode     = NEMADC_BF_ONE;
        layer[i].baseaddr_phys = fb[i].bo.base_phys;
        layer[i].baseaddr_virt = fb[i].bo.base_virt;
#endif

    }
}


int cur_fb = 0;
uintptr_t cur_fb_base_phys = 0;

void
swap_buffers(void) {

#ifndef DONT_USE_NEMADC
    nemadc_wait_vsync();
    nemadc_set_layer(0, &layer[cur_fb]);
#endif

    cur_fb = (cur_fb+1)%FRAME_BUFFERS;
    cur_fb_base_phys = fb[cur_fb].bo.base_phys;
}

int main(int argc, char **argv)
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
#endif

    cur_fb_base_phys = fb[cur_fb].bo.base_phys;

    //Create Command List
    nema_cmdlist_t cl  = nema_cl_create_sized(10*1024);
    //Bind Command List
    nema_cl_bind_circular(&cl);

    //Bind Framebuffer
    nema_bind_dst_tex(cur_fb_base_phys, fb[0].w, fb[0].h, fb[0].format, fb[0].stride);
    //Set Clipping Rectangle
    nema_set_clip(0, 0, RESX, RESY);

    //Set Blending Mode
    nema_set_blend_fill(NEMA_BL_SRC);
    //Fill Rectangle with Black (Clear)
    nema_fill_rect(0, 0, RESX, RESY, 0);

    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    nema_cl_rewind(&cl);

    //initialize nema_vg
    nema_vg_init(RESX, RESY);

    NEMA_VG_PAINT_HANDLE paint =  nema_vg_paint_create();
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_COLOR);

    float font_size = 32.f;
    uint32_t rgba8888 = nema_rgba(255, 255, 0, 255);
    nema_vg_paint_set_paint_color(paint, rgba8888);

    nema_vg_bind_font(&VECTOR_FONT);

    nema_vg_set_font_size(font_size);
    nema_set_blend_fill(NEMA_BL_SRC_OVER);

    float w,h;
    nema_vg_string_get_bbox(str, &w, &h, RESX, NEMA_ALIGNX_LEFT);

    float x_off = 170.f;
    float y_off = 250.f;
    //transformation origin point is the center location in the string's bound box
    float x_transform = x_off + 0.5f*w;
    float y_transform = y_off + 0.5f*h;

    while(1){
        float time = nema_get_time();
        float sec  = time;
        float min  = time/60;
        float hour = time/60/60;

        //calculate text angle based on current time
        float angle = sec*5.f /60.f*360.f;

        nema_cl_rewind(&cl);

        nema_bind_dst_tex(cur_fb_base_phys, fb[0].w, fb[0].h, fb[0].format, fb[0].stride);
        nema_clear(nema_rgba(0, 0, 0, 0xff));

        //compute the transformation matrix that
        //rotates text around transformation origin point (x_transform, y_transform)
        nema_matrix3x3_t mat;
        nema_mat3x3_load_identity(mat);
        nema_mat3x3_translate(mat, -x_transform, -y_transform);
        nema_mat3x3_rotate(mat, angle);
        nema_mat3x3_translate(mat, x_transform, y_transform);
        nema_vg_print(paint, str, x_off, y_off, RESX, RESY, NEMA_ALIGNX_LEFT, mat);

        //draw transformation origin point
        nema_set_blend_fill(NEMA_BL_SRC);
        nema_fill_rect(x_transform, y_transform, 2, 2, 0xff0000ff);
        nema_cl_submit(&cl);
        nema_cl_wait(&cl);

        swap_buffers();
        nema_calculate_fps();
    }

    nema_cl_destroy(&cl);

    nema_vg_paint_destroy(paint);
    nema_vg_deinit();

    return 0;
}
