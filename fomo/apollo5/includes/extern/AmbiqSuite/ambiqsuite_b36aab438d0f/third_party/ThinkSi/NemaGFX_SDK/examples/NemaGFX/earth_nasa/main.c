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
#include "nema_utils.h"
#include "nema_event.h"

#include "earth_nasa.h"

#ifndef DONT_USE_NEMADC
#include "nema_dc.h"
#endif

#define RESX 500
#define RESY 500


#define FRAME_BUFFERS 3
static img_obj_t fb[3] = {{0}};

#ifndef DONT_USE_NEMADC
static nemadc_layer_t dc_layer[3] = {{0}};
#endif


img_obj_t earth_img = {
    .bo             = {0},
    .w              = 1024,
    .h              = 1024,
    .stride         = 1024*4,
    .color          = 0,
    .format         = NEMA_RGBA8888,
    .sampling_mode  = NEMA_FILTER_BL
};

void
load_objects(void)
{
    //Allocate Framebuffers
    for (int i = 0; i < FRAME_BUFFERS; ++i) {
        fb[i].w      = RESX;
        fb[i].h      = RESY;
        fb[i].format = NEMA_RGBA8888;
        fb[i].stride = RESX*4;
        fb[i].bo     = nema_buffer_create(fb[i].stride*fb[i].h);
        nema_buffer_map(&fb[i].bo);


        dc_layer[i].format        = NEMADC_RGBA8888;
        dc_layer[i].sizex         = dc_layer[i].resx = fb[i].w;
        dc_layer[i].sizey         = dc_layer[i].resy = fb[i].h;
        dc_layer[i].stride        = fb[i].stride;
        dc_layer[i].blendmode     = NEMADC_BL_SRC;
        dc_layer[i].baseaddr_phys = fb[i].bo.base_phys;
        dc_layer[i].baseaddr_virt = fb[i].bo.base_virt;

        printf("FB: V:%p P:0x%08x\n", (void *)fb[i].bo.base_virt, fb[i].bo.base_phys);
    }

    earth_img.bo = nema_load_file("earth.small_img0.rgba", -1, 0);
}

static float angle_x = 0.f;
static float angle_y = 0.f;
static float trans_z = 0.f;
static float angle_dx = 0.f;
static float angle_dy = 0.f;
static float trans_dz = 0.f;

static int draw_wireframe = 0;

void draw_earth() {
    nema_tri_cull(NEMA_CULL_CW);
    nema_bind_src_tex(earth_img.bo.base_phys, earth_img.w, earth_img.h, earth_img.format, earth_img.stride, earth_img.sampling_mode);
    nema_set_blend_blit(NEMA_BL_SRC);

    nema_matrix4x4_t mvp;
    nema_mat4x4_load_perspective(mvp, 45.f, (float)RESX/(float)RESY, 0.1f, 1000.f);

    nema_matrix4x4_t proj;
    nema_mat4x4_load_identity(proj);
    nema_mat4x4_scale(proj, 0.1f, -0.1f, -0.1f);
    nema_mat4x4_rotate_Y(proj, angle_y);
    nema_mat4x4_rotate_X(proj, angle_x);
    nema_mat4x4_translate(proj, 0.f, 0.f, 200.f+trans_z);
    // nema_mat4x4_translate(proj, 250.f, 250.f, 0.f);


    nema_mat4x4_mul(mvp, mvp, proj);

    for (int idx_id = 0; idx_id < EARTH_INDICES_NUM; ) {
        int idx0 = earth_indices[idx_id++];
        int idx1 = earth_indices[idx_id++];
        int idx2 = earth_indices[idx_id++];

        float *v0 = &earth_vertices[idx0*3];
        float *v1 = &earth_vertices[idx1*3];
        float *v2 = &earth_vertices[idx2*3];

        float *uv0 = &earth_uv[idx0*2];
        float *uv1 = &earth_uv[idx1*2];
        float *uv2 = &earth_uv[idx2*2];

        float vv0[4] = {
               v0[0],
               v0[1],
               v0[2],
                1.f
        };
        float vv1[4] = {
               v1[0],
               v1[1],
               v1[2],
                 1.f
        };
        float vv2[4] = {
               v2[0],
               v2[1],
               v2[2],
                 1.f
        };

        int clip;
        clip = nema_mat4x4_obj_to_win_coords(mvp, 0.f, 0.f, RESX, RESY, 0.1f, 1000.f, &vv0[0], &vv0[1], &vv0[2], &vv0[3]);
        if (clip) continue;
        clip = nema_mat4x4_obj_to_win_coords(mvp, 0.f, 0.f, RESX, RESY, 0.1f, 1000.f, &vv1[0], &vv1[1], &vv1[2], &vv1[3]);
        if (clip) continue;
        clip = nema_mat4x4_obj_to_win_coords(mvp, 0.f, 0.f, RESX, RESY, 0.1f, 1000.f, &vv2[0], &vv2[1], &vv2[2], &vv2[3]);
        if (clip) continue;

        float area2 = 0.f;
        {
            area2 =
            vv0[0] * ( vv1[1] - vv2[1]) +
            vv1[0] * ( vv2[1] - vv0[1]) +
            vv2[0] * ( vv0[1] - vv1[1]);

            if ( area2 >= 0.f ) continue;
        }

        if ( draw_wireframe != 0 ) {
            // if draw_wireframe, we need to set the blending function on each triangle
            nema_set_blend_blit(NEMA_BL_SRC);
        }
        nema_blit_tri_uv(
            vv0[0], vv0[1], vv0[2],
            vv1[0], vv1[1], vv1[2],
            vv2[0], vv2[1], vv2[2],
            uv0[0]*earth_img.w, uv0[1]*earth_img.h,
            uv1[0]*earth_img.w, uv1[1]*earth_img.h,
            uv2[0]*earth_img.w, uv2[1]*earth_img.h
        );

        if ( draw_wireframe != 0 ) {
            nema_set_blend_fill(NEMA_BL_SRC);
            nema_draw_line(vv0[0], vv0[1], vv1[0], vv1[1], 0xff0000ffU);
            nema_draw_line(vv1[0], vv1[1], vv2[0], vv2[1], 0xff0000ffU);
            nema_draw_line(vv2[0], vv2[1], vv0[0], vv0[1], 0xff0000ffU);
        }
    }

    nema_tri_cull(NEMA_CULL_NONE);
}

void handle_events() {

    static nema_event_t event = {0};
    nema_event_wait(&event, 0);

    angle_dx *= 0.9f;
    angle_dy *= 0.9f;
    trans_dz *= 0.9f;

    if ( event.mouse_state == MOUSE_STATE_LEFT_CLICKED ) {
#if 1
        angle_dy = ( (-0.5f*event.mouse_dx) + angle_dy ) * 0.5f;
        angle_dx = ( ( 0.5f*event.mouse_dy) + angle_dx ) * 0.5f;
#else
        angle_dy = -event.mouse_dx*0.5f;
        angle_dx =  event.mouse_dy*0.5f;
#endif
        // printf("%f, %f, %f\n", angle_x, angle_y, angle_z);
    }
    if ( event.mouse_event == MOUSE_EVENT_SCROLL_UP ) {
        trans_dz -= 10.f;
    }
    if ( event.mouse_event == MOUSE_EVENT_SCROLL_DOWN ) {
        trans_dz += 10.f;
    }
    if ( event.mouse_event == MOUSE_EVENT_MIDDLE_RELEASE) {
        draw_wireframe = 1 - draw_wireframe;
    }

    angle_x += angle_dx;
    angle_y += angle_dy;
    trans_z += trans_dz;
    trans_z = nema_clamp(trans_z, -50.f, 750.f);
}

#ifdef STANDALONE
int
main()
#else
int earth_nasa()
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
    (void)nema_event_init(0, 0, 0, RESX, RESY);

    load_objects();

#ifndef DONT_USE_NEMADC
    //Format        | Pixclock | RESX | FP | SYNC | BP | RESY | FP | SYNC | BP
    //800x600, 60Hz | 40.000   | 800  | 40 | 128  | 88 | 600  | 1  | 4    | 23
    nemadc_timing(800, 40, 128, 88, 600, 1, 4, 23);
    nemadc_set_layer(0, &dc_layer[0]);

    uintptr_t cur_fb_base_phys = nema_init_triple_fb(0, dc_layer[0].baseaddr_phys,
                                                        dc_layer[1].baseaddr_phys,
                                                        dc_layer[2].baseaddr_phys);
#endif

    //Create Command List
    nema_cmdlist_t cl  = nema_cl_create_sized(4096);
    //Bind Command List
    nema_cl_bind_circular(&cl);


    do {
        nema_cl_rewind(&cl);

        //Bind Framebuffer
        nema_bind_dst_tex(cur_fb_base_phys, fb[0].w, fb[0].h, fb[0].format, fb[0].stride);
        //Set Clipping Rectangle
        nema_set_clip(0, 0, RESX, RESY);

        //Set Blending Mode
        nema_set_blend_fill(NEMA_BL_SRC);
        //Fill Rectangle with Black (Clear)
        nema_fill_rect(0, 0, RESX, RESY, 0);

        draw_earth();

        //Submit Command List
        nema_cl_submit(&cl);
        //Wait for submitted Command List to finish
        nema_cl_wait(&cl);
        cur_fb_base_phys = nema_swap_fb(0);
        nema_calculate_fps();

        handle_events();
    } while(1);

    //Destroy (free) Command List
    nema_cl_destroy(&cl);

    return 0;
}