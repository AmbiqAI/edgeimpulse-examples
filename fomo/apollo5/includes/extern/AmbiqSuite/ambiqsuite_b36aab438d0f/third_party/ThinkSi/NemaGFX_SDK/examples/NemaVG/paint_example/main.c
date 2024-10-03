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
#include "nema_matrix3x3.h"
#include "nema_utils.h"
#include "nema_vg.h"
#include "paint.h"
#include "ref.rgba.h"

#ifndef DONT_USE_NEMADC
#include "nema_dc.h"
#endif

#define RESX 800
#define RESY 600

nema_img_obj_t fb = {{0}, RESX, RESY, RESX*4, 0, NEMA_RGBA8888, 0};
nema_img_obj_t ref_img = {{0}, REF_IMG_W, REF_IMG_H, REF_IMG_W*4, 0, NEMA_RGBA8888, NEMA_FILTER_BL};

#ifndef DONT_USE_NEMADC
nemadc_layer_t dc_layer = {(void *)0, 0, RESX, RESY, -1, 0, 0, RESX, RESY, 0xff, NEMADC_BL_SRC, 0, NEMADC_RGBA8888, 0, 0, 0, 0, 0};
#endif

NEMA_VG_PATH_HANDLE path;
NEMA_VG_PAINT_HANDLE paint;
NEMA_VG_GRAD_HANDLE gradient;
nema_matrix3x3_t m_path;
nema_matrix3x3_t m_paint;
float texture_center_x = REF_IMG_W/2;
float texture_center_y = REF_IMG_H/2;
float star_x_off = 10.f, star_y_off = 10.f;
float star_length = 80.f;
float star_height = 100.f;

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


static void
draw_star_texture(float x_pos, float y_pos, float degree)
{
    nema_mat3x3_load_identity(m_path);
    nema_mat3x3_translate(m_path, x_pos, y_pos);
    nema_vg_path_set_matrix(path, m_path);

    nema_mat3x3_load_identity(m_paint);
    calc_matrix(m_paint, degree, texture_center_x, texture_center_y);
    nema_mat3x3_translate(m_paint, x_pos + star_x_off, y_pos + star_y_off);
    nema_vg_paint_set_tex_matrix(paint, m_paint);

    nema_vg_draw_path(path, paint);
}

#ifdef STANDALONE
int main()
#else
int nema_paint_stars()
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
    path = nema_vg_path_create();
    gradient = nema_vg_grad_create();
    nema_vg_path_set_shape(path, 6, cmds_star, 18, data_star_small);
    nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
    nema_vg_set_blend(NEMA_BL_SRC_OVER);


    float x_pos, y_pos;
    //--------------LINEAR GRADIENT--------------------------

    nema_vg_grad_set(gradient, COLOR_STOPS, stops, colors);
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_GRAD_LINEAR);

    // Use the same paint with different grad each time
    // use the same path with offset each time

    for(int i = 0; i < STARS_IN_ONE_LINE ; i++)
    {

        x_pos= STAR_DIST_X*i;

        y_pos = 0.f;
        nema_mat3x3_load_identity(m_path);
        nema_mat3x3_translate(m_path, x_pos, y_pos);
        nema_vg_path_set_matrix(path, m_path);

        nema_vg_paint_set_grad_linear(paint,gradient, grad[i][0] + x_pos, grad[i][1] + y_pos, grad[i][2] + x_pos, grad[i][3] + y_pos, NEMA_TEX_CLAMP | NEMA_FILTER_BL);
        nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
        nema_vg_draw_path(path, paint);

        y_pos = STAR_DIST_Y;
        nema_mat3x3_load_identity(m_path);
        nema_mat3x3_translate(m_path, x_pos, y_pos);
        nema_vg_path_set_matrix(path, m_path);

        nema_vg_paint_set_grad_linear(paint,gradient, grad[i][0] + x_pos, grad[i][1] + y_pos, grad[i][2] + x_pos, grad[i][3] + y_pos, NEMA_TEX_CLAMP | NEMA_FILTER_BL);
        nema_vg_set_fill_rule(NEMA_VG_FILL_NON_ZERO);
        nema_vg_draw_path(path, paint);
    }

    //--------------TEXTURE--------------------------

    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_TEXTURE);
    nema_vg_paint_set_tex(paint, &ref_img);
    float degree[STARS_IN_ONE_LINE] = {0.f,   45.f,
                                       90.f,  135.f,
                                       180.f, 225.f,
                                       270.f, 315.f};

    for(int i = 0; i < STARS_IN_ONE_LINE ; i++)
    {
        x_pos= STAR_DIST_X*i;

        y_pos = STAR_DIST_Y*2;
        nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
        draw_star_texture(x_pos, y_pos, degree[i]);

        y_pos = STAR_DIST_Y*3;
        nema_vg_set_fill_rule(NEMA_VG_FILL_NON_ZERO);
        draw_star_texture(x_pos, y_pos, degree[i]);
    }

    y_pos = STAR_DIST_Y*4;

    //----------------CONIC GRADIENT---------------------

    float x_star_center;
    float y_star_center;
    x_pos = 0.f;
    x_star_center = x_pos + star_length/2 + star_x_off;
    y_star_center = y_pos + star_height/2 + star_y_off + 12.f;
    nema_mat3x3_load_identity(m_path);
    nema_mat3x3_translate(m_path, x_pos, y_pos);
    nema_vg_path_set_matrix(path, m_path);
    nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_GRAD_CONICAL);
    nema_vg_paint_set_grad_conical(paint,gradient, x_star_center, y_star_center, NEMA_TEX_CLAMP  | NEMA_FILTER_BL);

    nema_vg_draw_path(path, paint);

    x_pos = STAR_DIST_X;
    x_star_center = x_pos + star_length/2 + star_x_off;
    y_star_center = y_pos + star_height/2 + star_y_off + 12.f;
    nema_mat3x3_load_identity(m_path);
    nema_mat3x3_translate(m_path, x_pos, y_pos);
    nema_vg_path_set_matrix(path, m_path);
    nema_vg_set_fill_rule(NEMA_VG_FILL_NON_ZERO);
    nema_vg_paint_set_grad_conical(paint,gradient, x_star_center, y_star_center, NEMA_TEX_CLAMP | NEMA_FILTER_BL);
    nema_vg_draw_path(path, paint);

    //--------------RADIAL GRADIENT--------------------------

    x_pos = STAR_DIST_X*2;
    x_star_center = x_pos + star_length/2 + star_x_off;
    y_star_center = y_pos + star_height/2 + star_y_off + 12.f;
    nema_mat3x3_load_identity(m_path);
    nema_mat3x3_translate(m_path, x_pos, y_pos);
    nema_vg_path_set_matrix(path, m_path);
    nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_GRAD_RADIAL);
    nema_vg_paint_set_grad_radial(paint, gradient, x_star_center, y_star_center, star_length/2, NEMA_TEX_CLAMP | NEMA_FILTER_BL);
    nema_vg_draw_path(path, paint);

    uint32_t error = nema_vg_get_error();
    if (error == NEMA_VG_ERR_NO_RADIAL_ENABLED){
        printf("error = %x \n", error);
    }

    x_pos = STAR_DIST_X*3;
    x_star_center = x_pos + star_length/2 + star_x_off;
    y_star_center = y_pos + star_height/2 + star_y_off + 12.f;
    nema_mat3x3_load_identity(m_path);
    nema_mat3x3_translate(m_path, x_pos, y_pos);
    nema_vg_path_set_matrix(path, m_path);
    nema_vg_set_fill_rule(NEMA_VG_FILL_NON_ZERO);
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_GRAD_RADIAL);
    nema_vg_paint_set_grad_radial(paint, gradient, x_star_center, y_star_center, star_length/2, NEMA_TEX_CLAMP | NEMA_FILTER_BL);
    nema_vg_draw_path(path, paint);

    //--------------COLOR FILL--------------------------


    x_pos = STAR_DIST_X*4;
    nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
    nema_mat3x3_load_identity(m_path);
    nema_mat3x3_translate(m_path, x_pos, y_pos);
    nema_vg_path_set_matrix(path, m_path);
    nema_vg_paint_set_paint_color(paint, nema_rgba(0x00, 0xff, 0x000, 0xff)); // green
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_COLOR);
    nema_vg_draw_path(path, paint);

    x_pos = STAR_DIST_X*5;
    nema_mat3x3_load_identity(m_path);
    nema_mat3x3_translate(m_path, x_pos, y_pos);
    nema_vg_path_set_matrix(path, m_path);
    nema_vg_set_fill_rule(NEMA_VG_FILL_NON_ZERO);
    nema_vg_paint_set_paint_color(paint, nema_rgba(0x00, 0xff, 0x000, 0xff)); // green
    nema_vg_draw_path(path, paint);

    //--------------DRAW OUTLINE--------------------------

    x_pos = STAR_DIST_X*6;
    nema_mat3x3_load_identity(m_path);
    nema_mat3x3_translate(m_path, x_pos, y_pos);
    nema_vg_path_set_matrix(path, m_path);
    nema_vg_set_fill_rule(NEMA_VG_STROKE);
    nema_vg_paint_set_paint_color(paint, nema_rgba(0xff, 0x00, 0x000, 0xff)); // red
    nema_vg_draw_path(path, paint);

    //stroke >1
    x_pos = STAR_DIST_X*7;
    nema_mat3x3_load_identity(m_path);
    nema_mat3x3_translate(m_path, x_pos, y_pos);
    nema_vg_path_set_matrix(path, m_path);
    nema_vg_paint_set_stroke_width(paint, 5.f);
    nema_vg_draw_path(path, paint);

    //-----------------------------------------------

    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    nema_vg_paint_destroy(paint);
    nema_vg_path_destroy(path);
    nema_vg_grad_destroy(gradient);

    nema_vg_deinit();
    destroy_objects();

    return 0;
}
