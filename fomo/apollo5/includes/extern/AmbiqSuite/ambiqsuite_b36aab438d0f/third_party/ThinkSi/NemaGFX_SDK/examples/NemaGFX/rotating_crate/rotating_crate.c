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
#include "nema_matrix4x4.h"
#include "nema_math.h"
#include "nema_utils.h"

#ifndef DONT_USE_NEMADC
#include "nema_dc.h"
#endif

#ifndef RESX
#define RESX 800
#endif

#ifndef RESY
#define RESY 600
#endif

#ifndef COLOR_CUBE
#define COLOR_CUBE 0
#endif

typedef enum {
    PRIM_LINE,
    PRIM_RECT,
    PRIM_TRI,
    PRIM_QUAD,
    PRIM_MAX
} primitive_e;


#define FRAME_BUFFERS 2
img_obj_t fb[FRAME_BUFFERS];

#ifndef DONT_USE_NEMADC
nemadc_layer_t layer[FRAME_BUFFERS] = {{0}};
#endif

#define CRATE_FORMAT NEMA_RGBA8888
// #define CRATE_PATH   "crate.512x512.rgba"

#include "crate.512x512.rgba.h"

img_obj_t crate  = {{0}, 512, 512, -1, 0, CRATE_FORMAT, 0};

void
load_objects(void)
{
    int i;
    for (i = 0; i < FRAME_BUFFERS; ++i) {
        fb[i].w = RESX;
        fb[i].h = RESY;
        fb[i].format = NEMA_RGBA8888;
        fb[i].stride = RESX*4;
        fb[i].bo = nema_buffer_create(fb[i].stride*fb[i].h);
        nema_buffer_map(&fb[i].bo);

#ifndef DONT_USE_NEMADC
        layer[i].sizex = layer[i].resx = fb[i].w;
        layer[i].sizey = layer[i].resy = fb[i].h;
        layer[i].stride = fb[i].stride;
        layer[i].format = NEMADC_RGBA8888;
        layer[i].blendmode = NEMADC_BF_ONE;
        layer[i].baseaddr_phys = fb[i].bo.base_phys;
        layer[i].baseaddr_virt = fb[i].bo.base_virt;
#endif

    }

#if 0
    // crate.bo = nema_load_file(CRATE_PATH, -1, (void *)0);
#else
    crate.bo = nema_buffer_create(crate_512x512_rgba_len);
    nema_memcpy(crate.bo.base_virt, crate_512x512_rgba, crate_512x512_rgba_len);
#endif
}

int mouse_x = 0;
int mouse_y = 0;

void draw_cube_side(float *v, int v0, int v1, int v2, int v3, uint32_t col) {
#if COLOR_CUBE
    //fill with color
    nema_fill_quad(v[v0*3], v[v0*3+1],
                   v[v1*3], v[v1*3+1],
                   v[v2*3], v[v2*3+1],
                   v[v3*3], v[v3*3+1], col);
#else

    //blit
    nema_blit_quad_fit(v[v0*3], v[v0*3+1],
                   v[v1*3], v[v1*3+1],
                   v[v2*3], v[v2*3+1],
                   v[v3*3], v[v3*3+1]);

#endif
}

void innercube(int angle_x, int angle_y, int angle_z)
{
    float box_size_2 = 0.2f;
    float FoV = 28.0724869359f;

                   //x     y    z
    float v[]   = {-box_size_2, -box_size_2, box_size_2,   //0  0
                    box_size_2, -box_size_2, box_size_2,   //1  3
                    box_size_2,  box_size_2, box_size_2,   //2  6
                   -box_size_2,  box_size_2, box_size_2,   //3  9
                   -box_size_2, -box_size_2,-box_size_2,   //4  12
                    box_size_2, -box_size_2,-box_size_2,   //5  15
                    box_size_2,  box_size_2,-box_size_2,   //6  18
                   -box_size_2,  box_size_2,-box_size_2};  //7  21

    //projection
    nema_matrix4x4_t mvp;

    nema_mat4x4_load_perspective(mvp, FoV, (float)RESX/RESY, 0.2f, 100.f);

    nema_matrix4x4_t proj;
    nema_mat4x4_load_identity(proj);
    nema_mat4x4_rotate_X(proj, angle_x);
    nema_mat4x4_rotate_Y(proj, angle_y);
    nema_mat4x4_rotate_Z(proj, angle_z);
    nema_mat4x4_translate(proj, 0, 0, 2.f-box_size_2);

    nema_mat4x4_mul(mvp, mvp, proj);

    int i;

    for (i = 0; i < 24; i+=3) {
        float w = 1.f;
        nema_mat4x4_obj_to_win_coords(mvp, 0.f, 0.f, RESX, RESY,
                                      1.f, 100.f,
                                      &v[i  ], &v[i+1], &v[i+2], &w);
    }

    //blend color with background
#if COLOR_CUBE
    nema_set_blend_fill(NEMA_BL_SIMPLE);
#else
    nema_set_blend_blit(NEMA_BL_SRC);
#endif
    nema_bind_src_tex(crate.bo.base_phys, crate.w, crate.h, crate.format, crate.stride, NEMA_FILTER_BL);

    //remove this to draw back sides also
    nema_tri_cull(NEMA_CULL_CW);
    draw_cube_side(v, 0, 1, 2, 3, 0x60ffffff); //front
    draw_cube_side(v, 4, 0, 3, 7, 0x600000ff); //left
    draw_cube_side(v, 1, 5, 6, 2, 0x60ff00ff); //right
    draw_cube_side(v, 4, 5, 1, 0, 0x60ff0000); //top
    draw_cube_side(v, 3, 2, 6, 7, 0x6000ff00); //bottom
    draw_cube_side(v, 5, 4, 7, 6, 0x60808080); //back
    nema_tri_cull(NEMA_CULL_NONE);
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

nema_cmdlist_t cl;
nema_cmdlist_t cl_clear;

void
nema_gfx_display_function(){
    static int angle_x = 0.f;
    static int angle_y = 0.f;
    static int angle_z = 80.f;

    angle_x = (angle_x+1)%360;
    angle_y = (angle_y+2)%360;
//    angle_z = (angle_z+2)%360;


    nema_cl_bind(&cl_clear);
    nema_cl_rewind(&cl_clear);

    nema_set_clip(0, 0, RESX, RESY);
    nema_bind_dst_tex(cur_fb_base_phys, fb[0].w, fb[0].h, fb[0].format, fb[0].stride);

    nema_clear(0);

    nema_cl_submit(&cl_clear);

    nema_cl_bind(&cl);
    nema_cl_rewind(&cl);

    nema_set_clip(0, 0, RESX, RESY);
    nema_bind_dst_tex(cur_fb_base_phys, fb[0].w, fb[0].h, fb[0].format, fb[0].stride);

    innercube(angle_x, angle_y, angle_z);

    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    swap_buffers();
}

#ifdef STANDALONE
int main()
#else
int rotating_crate()
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
#endif

    cur_fb_base_phys = fb[cur_fb].bo.base_phys;

    //Create Command Lists
    cl  = nema_cl_create();
    cl_clear = nema_cl_create();

    while(1){
        nema_gfx_display_function();
        nema_calculate_fps();
    }

    nema_cl_destroy(&cl);

    return 0;
}
