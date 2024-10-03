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

#include "DejaVuSerif_ttf_kern.h"

#include "nema_font.h"
#include "nema_event.h"
#include "nema_utils.h"
#include "nema_easing.h"

#define VECTOR_FONT DejaVuSerif_ttf_kern

#define FRAME_BUFFERS 3

#define RESX 800
#define RESY 600

#define STEADY_STEP
// #define SIN_TRAJECTORY

#define COLOR_STOPS       4
static float stops[COLOR_STOPS] = {0.0f, 0.5f, 0.65f, 0.75f};
static color_var_t colors[COLOR_STOPS] = {
                    { 0xff, 0xff, 0xff, 0xff },
                    { 0x80, 0x80, 0x80, 0xff },
                    { 0x20, 0x20, 0x20, 0xff },
                    { 0x00, 0x00, 0x00, 0xff }
                                    }; //red



static img_obj_t fb[FRAME_BUFFERS];
static nemadc_layer_t layer[FRAME_BUFFERS] = {{0}};

static img_obj_t background = {{0}, RESX, RESY, 800*4, 0, NEMA_RGBA8888, 0};

#if 0
    static char str0[]  = "Good morning";
    static char str1[]  = "Καλημέρα";
    static char str2[]  = "Guten Morgen";
    static char str3[]  = "Bonjour";
    static char str4[]  = "Buongiorno";
    static char str5[]  = "God morgen";
    static char str6[]  = "Nema VG";
#else
    static char str0[]  = "Ultra";     //red
    static char str1[]  = "Low";       //orange
    static char str2[]  = "Power";     //yellow
    static char str3[]  = "Graphics";  //green
    static char str4[]  = "Think";     //blue
    static char str5[]  = "Silicon";   //teal
    static char str6[]  = "Nema VG";   //purple
#endif


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

    background.bo = nema_load_file("gradient_green800x600.rgba", -1, 0);
}

float draw_char(char c, float angle, float start_x, float start_y,
                float end_x, float end_y, NEMA_VG_PAINT_HANDLE paint,
                float (*ez_func)(float p)) {
    float x_advance    = 0.f;
    float cur_x        = 0.f;
    float cur_y        = 0.f;

    float sign  = -1.f;
    float speed = 8.f;

    speed = nema_fmod(speed, 12.f);
    float angle_2 = speed*angle*sign;

    nema_matrix3x3_t mat;
    nema_mat3x3_load_identity(mat);

    if ( angle < 90.f ) {
        //Bring to foreground while rotating
        cur_x = nema_ez(start_x,  end_x, 90.f, angle, ez_func);
        cur_y = nema_ez(start_y,  end_y, 90.f, angle, ez_func);

        nema_mat3x3_translate(mat, -cur_x, -cur_y);
        nema_mat3x3_rotate(mat, angle_2);
        nema_mat3x3_translate(mat, cur_x, cur_y);
    } else if ( angle < 270.f){
        //zoom in
        cur_x = end_x;
        cur_y = end_y;
    } else if (angle < 300.f) {
        //Send to background
        cur_x = nema_ez(end_x,  start_x, 30.f, angle - 270.f, nema_ez_linear);
        cur_y = nema_ez(end_y,  start_y, 30.f, angle - 270.f, nema_ez_linear);
    }

    x_advance += nema_vg_print_char(paint, c, cur_x, cur_y, mat, 0);

    return x_advance;
}


int cur_fb = 0;
uintptr_t cur_fb_base_phys = 0;

int main(int argc, char **argv)
{
int ret = 0;

    /* Initialize NemaGFX */
    ret = nema_init();
    if (ret) return ret;
#ifndef DONT_USE_NEMADC
    /* Initialize NemaDC */
    ret = nemadc_init();
    if (ret) return ret;

    nemadc_cursor_enable(0);
#endif
    ret = nema_event_init(0, 0, 0, RESX, RESY);
    if (ret) return ret;

    load_objects();

#ifndef DONT_USE_NEMADC
    //Format        | Pixclock | RESX | FP | SYNC | BP | RESY | FP | SYNC | BP
    //800x600, 60Hz | 40.000   | 800  | 40 | 128  | 88 | 600  | 1  | 4    | 23
    nemadc_timing(800, 40, 128, 88, 600, 1, 4, 23);
    nemadc_set_layer(0, &layer[0]);
#endif

    cur_fb_base_phys = nema_init_triple_fb(0, fb[0].bo.base_phys,
                                              fb[1].bo.base_phys,
                                              fb[2].bo.base_phys);

    uint32_t red    = nema_rgba(255, 0, 0, 255);
    uint32_t orange = nema_rgba(243, 156, 18, 255);
    uint32_t yellow = nema_rgba(243, 225, 107, 255);
    uint32_t green  = nema_rgba(30, 130, 76, 255);
    uint32_t blue   = nema_rgba(45, 130, 210, 255);
    uint32_t teal   = nema_rgba(28, 71, 77, 255);
    uint32_t purple = nema_rgba(102, 51, 153, 255);

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

    //initialize nema_vg
    nema_vg_init(RESX, RESY);

    NEMA_VG_PAINT_HANDLE paint =  nema_vg_paint_create();
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_COLOR);

    NEMA_VG_GRAD_HANDLE gradient = nema_vg_grad_create();

    nema_vg_set_fill_rule(NEMA_VG_STROKE);
    nema_vg_paint_set_stroke_width(paint, 0.5f);

    float font_size = 0.5f*RESY;
    uint32_t rgba8888 = nema_rgba(255, 255, 0, 255);
    nema_vg_paint_set_paint_color(paint, rgba8888);

    nema_vg_bind_font(&VECTOR_FONT);

    nema_vg_set_font_size(font_size);
    nema_set_blend_fill(NEMA_BL_SRC_OVER);

    float x_off = RESX/2;
    float y_off = RESY/2;

    float angle       = 0.f;
    float step_angle  = 1.f;
    bool zoom_in      = true;
    const float min_size = RESY/20.f;
    const float max_size = RESY/10.f;

    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    while(1){
        nema_cl_rewind(&cl);
    #ifndef STEADY_STEP
        float cur_time = nema_get_time()*30.f;
        angle = nema_fmod(cur_time, 360.f);
        float font_size_progress = nema_fmod(cur_time, 720.f);
        font_size_progress = nema_abs(font_size_progress - 360.f);
        font_size = nema_ez(min_size, max_size, 360.f, font_size_progress, nema_ez_bounce_in_out);
    #else
        float progress = angle/360.f;
        if ( zoom_in == true ) {
            font_size = nema_ez(min_size, max_size, 360.f, angle, nema_ez_bounce_in_out);
        } else {
            font_size = nema_ez(max_size, min_size, 360.f, angle, nema_ez_bounce_in_out);
        }
         nema_vg_set_font_size(font_size);
    #endif

        nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
        nema_vg_paint_set_stroke_width(paint, nema_min2(1.f, 0.02f*font_size));
        nema_vg_set_font_size(font_size);

        float w0,h0;
        nema_vg_string_get_bbox(str0, &w0, &h0, RESX, NEMA_ALIGNX_LEFT);

        x_off = RESX/2;
        y_off = RESY/2;
        //transformation origin point is the center location in the string's bound box
        float x_anim = 10.f + 50.f*nema_absf(nema_cos(2.f*angle));
        float y_anim = -0.5f*h0;

        nema_bind_dst_tex(cur_fb_base_phys, fb[0].w, fb[0].h, fb[0].format, fb[0].stride);
    #if 1
        // nema_clear(nema_rgba(0, 0, 0, 0xff));
        nema_set_blend_blit(NEMA_BL_SRC);
        nema_bind_src_tex(background.bo.base_phys, background.w, background.h, background.format, background.stride, NEMA_FILTER_BL);
        nema_blit(0, 0);
    #else
        nema_vg_grad_set(gradient, COLOR_STOPS, stops, colors);
        nema_vg_paint_set_type(paint, NEMA_VG_PAINT_GRAD_RADIAL);
        nema_vg_paint_set_grad_radial(paint, gradient, RESX/2, RESY/2, nema_max2(RESX, RESY), NEMA_TEX_CLAMP | NEMA_FILTER_BL);
        nema_vg_draw_rect(0, 0, RESX, RESY, NULL, paint);
    #endif

        nema_vg_set_fill_rule(NEMA_VG_STROKE);
        nema_vg_paint_set_type(paint, NEMA_VG_PAINT_COLOR);

        //Draw rotating strings
        //str0 transformation
        rgba8888 = red;
        nema_matrix3x3_t mat;
        nema_mat3x3_load_identity(mat);
        nema_mat3x3_load_identity(mat);
        nema_mat3x3_translate(mat, -x_off, -y_off);
        nema_mat3x3_rotate(mat, angle);
        nema_mat3x3_translate(mat, x_off, y_off);
        nema_vg_paint_set_paint_color(paint, rgba8888);
        nema_vg_print(paint, str0, x_off + x_anim, y_off + y_anim, RESX, RESY, NEMA_ALIGNX_LEFT, mat);

        //str1 transformation
        rgba8888 = orange;
        nema_mat3x3_load_identity(mat);
        nema_mat3x3_translate(mat, -x_off, -y_off);
        nema_mat3x3_rotate(mat, angle + 60.f);
        nema_mat3x3_translate(mat, x_off, y_off);
        nema_vg_paint_set_paint_color(paint, rgba8888);
        nema_vg_print(paint, str1, x_off + x_anim, y_off + y_anim, RESX, RESY, NEMA_ALIGNX_LEFT, mat);

        //str2 transformation
        rgba8888 = yellow;
        nema_mat3x3_load_identity(mat);
        nema_mat3x3_translate(mat, -x_off, -y_off);
        nema_mat3x3_rotate(mat, angle + 120.f);
        nema_mat3x3_translate(mat, x_off, y_off);
        nema_vg_paint_set_paint_color(paint, rgba8888);
        nema_vg_print(paint, str2, x_off + x_anim, y_off + y_anim, RESX, RESY, NEMA_ALIGNX_LEFT, mat);

        //str3 transformation
        rgba8888 = green;
        nema_mat3x3_load_identity(mat);
        nema_mat3x3_translate(mat, -x_off, -y_off);
        nema_mat3x3_rotate(mat, angle + 180.f);
        nema_mat3x3_translate(mat, x_off, y_off);
        nema_vg_paint_set_paint_color(paint, rgba8888);
        nema_vg_print(paint, str3, x_off + x_anim, y_off + y_anim, RESX, RESY, NEMA_ALIGNX_LEFT, mat);

        //str3 transformation
        rgba8888 = blue;
        nema_mat3x3_load_identity(mat);
        nema_mat3x3_translate(mat, -x_off, -y_off);
        nema_mat3x3_rotate(mat, angle + 240.f);
        nema_mat3x3_translate(mat, x_off, y_off);
        nema_vg_paint_set_paint_color(paint, rgba8888);
        nema_vg_print(paint, str4, x_off + x_anim, y_off + y_anim, RESX, RESY, NEMA_ALIGNX_LEFT, mat);

        //str3 transformation
        rgba8888 = teal;
        nema_mat3x3_load_identity(mat);
        nema_mat3x3_translate(mat, -x_off, -y_off);
        nema_mat3x3_rotate(mat, angle + 300.f);
        nema_mat3x3_translate(mat, x_off, y_off);
        nema_vg_paint_set_paint_color(paint, rgba8888);
        nema_vg_print(paint, str5, x_off + x_anim, y_off + y_anim, RESX, RESY, NEMA_ALIGNX_LEFT, mat);

        //Draw text inline
#if 1
        rgba8888 = purple;
        nema_vg_paint_set_paint_color(paint, rgba8888);

    #ifdef SIN_TRAJECTORY
        nema_vg_set_font_size(58.f);
    #else

        if ( (angle > 90.f) && (angle < 270.f) ) {
            font_size = nema_ez(RESX/21.f, RESX/10.f, 180.f, angle - 90.f, nema_ez_bounce_in_out);
            nema_vg_set_font_size(font_size);
        } else if ( (angle >= 270.f) && (angle < 300.f)) {
            font_size = nema_ez(RESX/10.f,  RESX/21.f, 30.f, angle - 270.f, nema_ez_linear);
            nema_vg_set_font_size(font_size);
        } else {
            nema_vg_set_font_size(RESX/21.f);
        }
    #endif

        nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
        nema_vg_string_get_bbox(str6, &w0, &h0, RESX, NEMA_ALIGNX_LEFT);

        float x_advance    = 0.f;
        static float cur_x = 0.f;
        float cur_y        = 0.f;
        float start_x;
        float start_y;
        float end_x;
        float end_y;



    #ifdef SIN_TRAJECTORY
        x_advance = -w0;

        if ( cur_x > RESX + w0) {
            cur_x = 0.f;
        }

        cur_y = (RESY - h0) + h0*nema_sin(cur_x + x_advance); x_advance += nema_vg_print_char(paint, 'N', cur_x + x_advance, cur_y, NULL, 0);
        cur_y = (RESY - h0) + h0*nema_sin(cur_x + x_advance); x_advance += nema_vg_print_char(paint, 'e', cur_x + x_advance, cur_y, NULL, 0);
        cur_y = (RESY - h0) + h0*nema_sin(cur_x + x_advance); x_advance += nema_vg_print_char(paint, 'm', cur_x + x_advance, cur_y, NULL, 0);
        cur_y = (RESY - h0) + h0*nema_sin(cur_x + x_advance); x_advance += nema_vg_print_char(paint, 'a', cur_x + x_advance, cur_y, NULL, 0);
        cur_y = (RESY - h0) + h0*nema_sin(cur_x + x_advance); x_advance += nema_vg_print_char(paint, ' ', cur_x + x_advance, cur_y, NULL, 0);
        cur_y = (RESY - h0) + h0*nema_sin(cur_x + x_advance); x_advance += nema_vg_print_char(paint, 'V', cur_x + x_advance, cur_y, NULL, 0);
        cur_y = (RESY - h0) + h0*nema_sin(cur_x + x_advance); x_advance += nema_vg_print_char(paint, 'G', cur_x + x_advance, cur_y, NULL, 0);
        cur_x += 8.f;
    #else

        start_x = -50.f; end_x = x_off - 0.5f*w0 + x_advance;
        start_y =   RESY; end_y = RESY-50.f;
        x_advance += draw_char('N', angle, start_x, start_y, end_x, end_y, paint, nema_ez_bounce_in_out);

        start_x = 850.f; end_x = x_off - 0.5f*w0 + x_advance;
        start_y =   RESY; end_y = RESY-50.f;
        x_advance += draw_char('e', angle, start_x, start_y, end_x, end_y, paint, nema_ez_linear);

        start_x = 0; end_x = x_off - 0.5f*w0 + x_advance;
        start_y = 0; end_y = RESY-50.f;
        x_advance += draw_char('m', angle, start_x, start_y, end_x, end_y, paint, nema_ez_back_out);

        start_x = 800; end_x = x_off - 0.5f*w0 + x_advance;
        start_y =   0; end_y = RESY-50.f;
        x_advance += draw_char('a', angle, start_x, start_y, end_x, end_y, paint, nema_ez_back_out);

        x_advance += nema_vg_print_char(paint, ' ', x_off - 0.5f*w0 + x_advance, 550, NULL, 0);

        start_x =   0; end_x = x_off - 0.5f*w0 + x_advance;
        start_y = RESY/2.f; end_y = RESY-50.f;
        x_advance += draw_char('V', angle, start_x, start_y, end_x, end_y, paint, nema_ez_bounce_in);

        start_x =   0; end_x = x_off - 0.5f*w0 + x_advance;
        start_y = RESY/3.f; end_y = RESY-50.f;
        x_advance += draw_char('G', angle, start_x, start_y, end_x, end_y, paint, nema_ez_bounce_out);
    #endif //SIN_TRAJECTORY
#endif

        //draw transformation origin point
    #if 0
        nema_set_blend_fill(NEMA_BL_SRC);
        nema_fill_rect(x_off, y_off, 2, 2, 0xff0000ff);
    #endif

        nema_cl_submit(&cl);
        nema_cl_wait(&cl);

        cur_fb_base_phys = nema_swap_fb(0);
        nema_calculate_fps();

    #ifdef STEADY_STEP
        angle += step_angle;
        angle = nema_fmod(angle, 360.f);
        if( nema_float_is_zero(angle) ) {
            zoom_in = !zoom_in;
        }
    #endif

    }

    nema_cl_destroy(&cl);

    nema_vg_paint_destroy(paint);
    nema_vg_deinit();

    return 0;
}
