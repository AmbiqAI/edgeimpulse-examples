//*****************************************************************************
//
//! @file coverflow.c
//! @brief coverflow example.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

// -----------------------------------------------------------------------------
// Copyright (c) 2019 Think Silicon S.A.
// Think Silicon S.A. Confidential Proprietary
// -----------------------------------------------------------------------------
//     All Rights reserved - Unpublished -rights reserved under
//         the Copyright laws of the European Union
//
//  This file includes the Confidential information of Think Silicon S.A.
//  The receiver of this Confidential Information shall not disclose
//  it to any third party and shall protect its confidentiality by
//  using the same degree of care, but not less than a reasonable
//  degree of care, as the receiver uses to protect receiver's own
//  Confidential Information. The entire notice must be reproduced on all
//  authorised copies and copies may only be made to the extent permitted
//  by a licensing agreement from Think Silicon S.A..
//
//  The software is provided 'as is', without warranty of any kind, express or
//  implied, including but not limited to the warranties of merchantability,
//  fitness for a particular purpose and noninfringement. In no event shall
//  Think Silicon S.A. be liable for any claim, damages or other liability, whether
//  in an action of contract, tort or otherwise, arising from, out of or in
//  connection with the software or the use or other dealings in the software.
//
//
//                    Think Silicon S.A.
//                    http://www.think-silicon.com
//                    Patras Science Park
//                    Rion Achaias 26504
//                    Greece
// -----------------------------------------------------------------------------

#include "am_bsp.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef BAREMETAL
#include "nema_event.h"
#endif

#include "coverflow.h"
#include "im01_128x128_rgba.h"
#include "im02_128x128_rgba.h"
#include "im03_128x128_rgba.h"
#include "im04_128x128_rgba.h"

//*****************************************************************************
//
//! *Porting notes
//! * 1)define RESX, RESY, FRAME_BUFFERS, IMG_SIZE, IMG_FORMAT, IMAGE_COUNT
//! * 2)MODE_INTERRACTIVE: coverflow works using mouse input, otherwise draw frames continuously
//! * 3)Modify "load_mult_objects" function (frame-buffer format, stride, input images path)
//
//*****************************************************************************
#define COUNT_FPS
// #define MODE_INTERRACTIVE

#ifndef DEBUG_OVERDRAWS
#define DEBUG_OVERDRAWS 0
#endif

#define FRAME_BUFFERS 2

#define IMG_SIZE      128

#define IMAGE_COUNT   4
#define M_PI          3.14159265358979323846

#define IMAGE_ADDRESS1 (MSPI_XIP_BASE_ADDRESS + ui8Img128x128RGBA01Length)
#define IMAGE_ADDRESS2 (MSPI_XIP_BASE_ADDRESS + ui8Img128x128RGBA01Length + ui8Img128x128RGBA02Length)
#define IMAGE_ADDRESS3 (MSPI_XIP_BASE_ADDRESS + ui8Img128x128RGBA01Length + ui8Img128x128RGBA02Length + ui8Img128x128RGBA03Length)

typedef struct _quad_t
{
    float x0, y0, z0, w0;
    float x1, y1, z1, w1;
    float x2, y2, z2, w2;
    float x3, y3, z3, w3;
}
Quad_t;

static img_obj_t g_sFB[FRAME_BUFFERS];
static img_obj_t g_sImgsRGBA[IMAGE_COUNT];

static float FoV    = 30.f;
static float y_orig = 0;
static float x_orig = 0;
static nema_matrix4x4_t proj_matrix;

const float img_width     = 0.2f;
const float img_angle_deg = 90.f;
const float back_rail_z   = 0.98f;
const float front_img_z   = 0.7f;

static float front_rail_z;
static float shift_par0, shift_par3;
static float r_big, r_small;
static float centre_right_x, img_x_proj;
//*****************************************************************************
//
//! @brief initialize objects
//!
//! This function initialize frame buffers,it will loop forever when create frame
//! buffer failure. suggestion do not use format RGB565 when Drive display with
//! MSPI interface on Apollo4P or earlier chips.
//!
//! @return None
//
//*****************************************************************************
static void
load_mult_objects(void)
{
    //
    // Allocate Framebuffers
    //
    for (uint8_t i = 0; i < FRAME_BUFFERS; ++i)
    {
        g_sFB[i].w      = RESX;
        g_sFB[i].h      = RESY;
#if defined (AM_PART_APOLLO4B)
        g_sFB[i].format = NEMA_RGB565;
#else
        g_sFB[i].format = NEMA_RGB24;
#endif
        g_sFB[i].stride = nema_stride_size(g_sFB[i].format, 0, g_sFB[i].w);
        g_sFB[i].bo     = nema_buffer_create(nema_texture_size(g_sFB[i].format, 0, g_sFB[i].w, g_sFB[i].h));
        if(g_sFB[i].bo.base_phys == NULL)
        {
            am_util_stdio_printf("Failed to create FB!\n");
            while(1);
        }
        nema_buffer_map(&g_sFB[i].bo);
        am_util_stdio_printf("FB: V:%p P:0x%08x\n", (void *)g_sFB[i].bo.base_virt, g_sFB[i].bo.base_phys);
    }
#if (DISP_CTRL_IP == DISP_CTRL_IP_DC)
    nemadc_set_bgcolor(0x40404040);
#endif
    uint32_t ui32Color = 0x50505050;

    //
    // Texture is already loaded to memory, use it directly
    // No need to used nema_buffer_create()
    // nema_buffer_create() - like malloc() - only allocates an empty buffer
    // When using nema_buffer_create, make sure to memcpy the original texture to the
    // allocated buffer
    //
#ifdef LOAD_FROM_PSRAM
    g_sImgsRGBA[0].bo.base_virt = (void *)MSPI_XIP_BASE_ADDRESS;
    g_sImgsRGBA[1].bo.base_virt = (void *)IMAGE_ADDRESS1;
    g_sImgsRGBA[2].bo.base_virt = (void *)IMAGE_ADDRESS2;
    g_sImgsRGBA[3].bo.base_virt = (void *)IMAGE_ADDRESS3;

    g_sImgsRGBA[0].bo.base_phys = (uintptr_t)g_sImgsRGBA[0].bo.base_virt;
    g_sImgsRGBA[1].bo.base_phys = (uintptr_t)g_sImgsRGBA[1].bo.base_virt;
    g_sImgsRGBA[2].bo.base_phys = (uintptr_t)g_sImgsRGBA[2].bo.base_virt;
    g_sImgsRGBA[3].bo.base_phys = (uintptr_t)g_sImgsRGBA[3].bo.base_virt;
#else
    g_sImgsRGBA[0].bo = nema_buffer_create(sizeof(ui8Img128x128RGBA01));
    if(g_sImgsRGBA[0].bo.base_phys == NULL)
    {
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }

    g_sImgsRGBA[1].bo = nema_buffer_create(sizeof(ui8Img128x128RGBA02));
    if(g_sImgsRGBA[1].bo.base_phys == NULL)
    {
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }

    g_sImgsRGBA[2].bo = nema_buffer_create(sizeof(ui8Img128x128RGBA03));
    if(g_sImgsRGBA[2].bo.base_phys == NULL)
    {
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }

    g_sImgsRGBA[3].bo = nema_buffer_create(sizeof(ui8Img128x128RGBA04));
    if(g_sImgsRGBA[3].bo.base_phys == NULL)
    {
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }
#endif

    memcpy(g_sImgsRGBA[0].bo.base_virt, (void *)ui8Img128x128RGBA01, sizeof(ui8Img128x128RGBA01));
    g_sImgsRGBA[0].format = NEMA_RGBA8888;
    g_sImgsRGBA[0].w      = IMG_SIZE;
    g_sImgsRGBA[0].h      = IMG_SIZE;
    g_sImgsRGBA[0].stride = nema_stride_size(g_sImgsRGBA[0].format, 0, g_sImgsRGBA[0].w);
    g_sImgsRGBA[0].sampling_mode =  0;
#if (DISP_CTRL_IP == DISP_CTRL_IP_DC)
    nemadc_set_bgcolor(ui32Color);
#endif
    ui32Color += 0x10101010;

    memcpy(g_sImgsRGBA[1].bo.base_virt, (void *)ui8Img128x128RGBA02, sizeof(ui8Img128x128RGBA02));
    g_sImgsRGBA[1].format = NEMA_RGBA8888;
    g_sImgsRGBA[1].w      = IMG_SIZE;
    g_sImgsRGBA[1].h      = IMG_SIZE;
    g_sImgsRGBA[1].stride = nema_stride_size(g_sImgsRGBA[1].format, 0, g_sImgsRGBA[1].w);
    g_sImgsRGBA[1].sampling_mode =  0;
#if (DISP_CTRL_IP == DISP_CTRL_IP_DC)
    nemadc_set_bgcolor(ui32Color);
#endif
    ui32Color += 0x10101010;

    memcpy(g_sImgsRGBA[2].bo.base_virt, (void *)ui8Img128x128RGBA03, sizeof(ui8Img128x128RGBA03));
    g_sImgsRGBA[2].format = NEMA_RGBA8888;
    g_sImgsRGBA[2].w      = IMG_SIZE;
    g_sImgsRGBA[2].h      = IMG_SIZE;
    g_sImgsRGBA[2].stride = nema_stride_size(g_sImgsRGBA[2].format, 0, g_sImgsRGBA[2].w);
    g_sImgsRGBA[2].sampling_mode =  0;
#if (DISP_CTRL_IP == DISP_CTRL_IP_DC)
    nemadc_set_bgcolor(ui32Color);
#endif
    ui32Color += 0x10101010;

    memcpy(g_sImgsRGBA[3].bo.base_virt, (void *)ui8Img128x128RGBA04, sizeof(ui8Img128x128RGBA04));
    g_sImgsRGBA[3].format = NEMA_RGBA8888;
    g_sImgsRGBA[3].w      = IMG_SIZE;
    g_sImgsRGBA[3].h      = IMG_SIZE;
    g_sImgsRGBA[3].stride = nema_stride_size(g_sImgsRGBA[3].format, 0, g_sImgsRGBA[3].w);
    g_sImgsRGBA[3].sampling_mode =  0;
#if (DISP_CTRL_IP == DISP_CTRL_IP_DC)
    nemadc_set_bgcolor(ui32Color);
#endif
    ui32Color += 0x10101010;
}

static inline void
bind_img(img_obj_t *psImg)
{
    nema_set_tex_color(0);
    nema_bind_src_tex(psImg->bo.base_phys, psImg->w, psImg->h, psImg->format, psImg->stride, NEMA_FILTER_BL | NEMA_TEX_BORDER);
}

//
// Transform coordinates from [-1,1] to [0, RESX] and [0, RESY]
//
static void
transform_quad_coords(Quad_t *psQ)
{
    nema_mat4x4_obj_to_win_coords(proj_matrix, x_orig, y_orig, RESX, RESY, -1, 1, &psQ->x0, &psQ->y0, &psQ->z0, &psQ->w0);
    nema_mat4x4_obj_to_win_coords(proj_matrix, x_orig, y_orig, RESX, RESY, -1, 1, &psQ->x1, &psQ->y1, &psQ->z1, &psQ->w1);
    nema_mat4x4_obj_to_win_coords(proj_matrix, x_orig, y_orig, RESX, RESY, -1, 1, &psQ->x2, &psQ->y2, &psQ->z2, &psQ->w2);
    nema_mat4x4_obj_to_win_coords(proj_matrix, x_orig, y_orig, RESX, RESY, -1, 1, &psQ->x3, &psQ->y3, &psQ->z3, &psQ->w3);
}

static void
calc_circle(float *x_off, float *z_off, float angle, float r)
{
    *x_off = nema_cos(angle)*r;
    *z_off = nema_sin(angle)*r;
}

static void
cover_flow_init(void)
{
    nema_mat4x4_load_perspective(proj_matrix, FoV, (float)RESX / RESY, 1, 100);

    img_x_proj = nema_cos(img_angle_deg) * img_width;
    float img_z_proj = nema_sin(img_angle_deg) * img_width;

    front_rail_z = back_rail_z - img_z_proj;

    r_big   = (back_rail_z  - front_img_z) / nema_sin(img_angle_deg);
    r_small = (front_rail_z - front_img_z) / nema_sin(img_angle_deg);

    float x0_front_x = -img_width / 2;

    centre_right_x = x0_front_x + r_big;

    float right_critical_x0 = centre_right_x - nema_cos(img_angle_deg) * r_big;

    shift_par0 = -(right_critical_x0 + img_x_proj - 1) * 0.5f;
    shift_par3 = 1.f - shift_par0;

#if 0
    float centre_right_z = front_img_z;

    float centre_left_x  = x0_front_x - r_small;
    float centre_left_z  = front_img_z;
    float right_critical_z0 = back_rail_z;
    float right_critical_x1 = centre_right_x-nema_cos(img_angle_deg)*r_small;
    float right_critical_z1 = front_rail_z;

    float x0_front_z = front_img_z;

    float x1_front_x = img_width/2;
    float x1_front_z = front_img_z;
    printf("%f\n", img_width    );
    printf("%f\n", img_angle_deg);
    printf("%f\n", img_x_proj);
    printf("%f\n", img_z_proj);
    printf("%f\n", back_rail_z  );
    printf("%f\n", front_img_z  );
    printf("%f\n", front_rail_z);
    printf("%f\n", r_big  );
    printf("%f\n", r_small);

    printf("\n");

    printf("%f\t%f\n", x0_front_x       , x0_front_z       );
    printf("%f\t%f\n", x1_front_x       , x1_front_z       );
    printf("%f\t%f\n", centre_right_x   , centre_right_z   );
    printf("%f\t%f\n", centre_left_x    , centre_left_z    );
    printf("%f\t%f\n", right_critical_x0, right_critical_z0);
    printf("%f\t%f\n", right_critical_x1, right_critical_z1);

    printf("\n");

    printf("%f\n", shift_par0);
    printf("%f\n", shift_par3);
#endif
}

static void
map_quad_points(Quad_t *psQ, float shift_par)
{
    if (shift_par < shift_par0)
    {
        //0 - 0.3
        psQ->x0 = -2 * shift_par + 1 - img_x_proj;
        psQ->z0 = back_rail_z;

        psQ->x1 = psQ->x0 + img_x_proj;
        psQ->z1 = front_rail_z;
    }
    else if (shift_par < 0.5f)
    {
        //shift_par0 - 0.5
        //angle 71.6 - 0
        float angle = nema_ez( img_angle_deg, 0.f, 0.5f-shift_par0, shift_par-shift_par0, &nema_ez_linear);
        calc_circle(&psQ->x0, &psQ->z0, angle, r_big);

        psQ->x0 = centre_right_x - psQ->x0;
        psQ->z0 = front_img_z + psQ->z0;

        calc_circle(&psQ->x1, &psQ->z1, angle, r_small);

        psQ->x1 = centre_right_x - psQ->x1;
        psQ->z1 = front_img_z + psQ->z1;
    }
    else if (shift_par < shift_par3)
    {
        //0.5 - shift_par3
        //angle 0 - 71.6
        float angle = nema_ez( 0.f, img_angle_deg, shift_par3-0.5f, shift_par-0.5f, &nema_ez_linear);
        calc_circle(&psQ->x0, &psQ->z0, angle, r_small);

        psQ->x0 = -centre_right_x + psQ->x0;
        psQ->z0 = front_img_z + psQ->z0;

        calc_circle(&psQ->x1, &psQ->z1, angle, r_big);

        psQ->x1 = -centre_right_x + psQ->x1;
        psQ->z1 = front_img_z + psQ->z1;
    }
    else
    {
        //0.7 - 1.0
        psQ->x0 = -2 * shift_par + 1;
        psQ->z0 = front_rail_z;


        psQ->x1 = psQ->x0 + img_x_proj;
        psQ->z1 = back_rail_z;
    }

    // X
    psQ->x2 = psQ->x1;
    psQ->x3 = psQ->x0;

    // Y
    psQ->y0 = psQ->y1 = -0.5f*img_width;
    psQ->y2 = psQ->y3 =  0.5f*img_width;

    // Z
    psQ->z2 = psQ->z1;
    psQ->z3 = psQ->z0;

    // W
    psQ->w0 = psQ->w1 = psQ->w2 = psQ->w3 = 1.f;
}


static float
map_to_ratio(float val, float break_point, float laska)
{
    float ret_val = 0.f;

    if (val <= break_point)
    {
        return val / (1.f + laska);
    }

    ret_val += break_point;
    val     -= break_point;

    float before_laska = (0.5f - break_point) * 2.f;
    float after_laska  = (0.5f - break_point) * 2.f + laska;

    if (val < before_laska)
    {
        ret_val += val * after_laska / before_laska;
        return ret_val / (1.f + laska);
    }

    ret_val += after_laska;

    val     -= before_laska;
    ret_val += val;

    return ret_val / (1.f + laska);
}

int32_t
cover_flow()
{
    static uint8_t ui8CurrentIndex = 0;
    float shift_par;
    int32_t ui32Frames = 0;
    double secs = 0.5;
    int32_t i32CurImg = 0;
    float sec_init     = 0.f;
    double cur_sec      = 0.f;

#ifdef COUNT_FPS
    float sec0 = nema_get_time();
#endif
#ifndef BAREMETAL
    nema_event_init(1, 0, 0, RESX, RESY);
#endif

    nema_cmdlist_t sCLBg = nema_cl_create();
    nema_cmdlist_t sCL = nema_cl_create();
    nema_cmdlist_t *psCurCL = &sCL;

    load_mult_objects();

    cover_flow_init();

    nema_cl_bind(&sCLBg);

#if (DEBUG_OVERDRAWS == 0)
    nema_enable_gradient(0);
#endif
    nema_set_blend_fill(NEMA_BL_SRC);

    uint32_t ui32ColStep = 0x20000;
    uint32_t ui32ColInit = 0x080000;
    nema_set_gradient_fx(ui32ColInit, ui32ColInit, ui32ColInit, 0xff0000,
                         0, ui32ColStep,
                         0, ui32ColStep,
                         0, ui32ColStep,
                         0, 0);

    nema_fill_rect(0, RESY>>1, RESX, RESY>>1, 0x0);
    nema_enable_gradient(0);
    nema_fill_rect(0, 0, RESX, RESY>>1, 0x0);

#ifdef MODE_INTERRACTIVE
    nema_event_t event = {0};
#endif

    nema_cl_bind(psCurCL);
// #### INTERNAL BEGIN ####
#if defined(BAREMETAL) && defined(APOLLO4_FPGA)
    am_hal_stimer_config(AM_HAL_STIMER_HFRC_6MHZ | AM_HAL_STIMER_CFG_RUN);
#endif
// #### INTERNAL END ####
    while (1)
    {
        //
        // transfer frame to the display
        //
        am_devices_display_transfer_frame(g_sFB[ui8CurrentIndex].w,
                                          g_sFB[ui8CurrentIndex].h,
                                          g_sFB[ui8CurrentIndex].bo.base_phys,
                                          NULL, NULL);

        ui8CurrentIndex = (ui8CurrentIndex + 1)%FRAME_BUFFERS;

        nema_cl_rewind(psCurCL);

        nema_enable_aa(1, 1, 1, 1);

        nema_set_clip(0, 0, RESX, RESY);
        //
        // Bind Framebuffer
        //
        nema_bind_dst_tex(g_sFB[ui8CurrentIndex].bo.base_phys,
                          g_sFB[ui8CurrentIndex].w,
                          g_sFB[ui8CurrentIndex].h,
                          g_sFB[ui8CurrentIndex].format,
                          g_sFB[ui8CurrentIndex].stride);
        //
        // Clear screen before printing items on updated positions
        //
        nema_cl_branch(&sCLBg);

#if (DEBUG_OVERDRAWS != 0)
    nema_debug_overdraws(1U);
#endif
        //
        // Redraw gradient on Synopsys, when using the gyroscope
        //
#if defined(MODE_INTERRACTIVE) && defined(PLATFORM_SYNOPSYS)
        nema_set_blend_fill(NEMA_BL_SRC);
        nema_enable_gradient(0);
        nema_set_gradient_fx(ui32ColInit, ui32ColInit, ui32ColInit, 0xff0000,
                             0, ui32ColStep,
                             0, ui32ColStep,
                             0, ui32ColStep,
                             0, 0 );

        nema_fill_rect(0, (RESY >> 1) - (int)roll, RESX, (RESY >> 1) + (int)roll, 0x0);
        nema_enable_gradient(0);
        nema_fill_rect(0, 0, RESX, (RESY>>1)-(int)roll, 0x0);
#endif

        nema_set_const_color( nema_rgba(50, 50, 50, 0x80) );
        //
        // STOP_COUNT: number of images inside frustum
        //
#define STOP_COUNT (40)

        const float break_point = 0.5f - 1.f / STOP_COUNT;
        float clip_left  = RESX;
        float clip_right = 0;

        for (int32_t i32Stop = STOP_COUNT-1; i32Stop >= 0; --i32Stop)
        {
            int i32StopIdx = (i32Stop & 0x1) ? STOP_COUNT - ((i32Stop + 1) >> 1) : i32Stop >> 1 ;
            int img_idx  = (i32CurImg + i32StopIdx + IMAGE_COUNT * 10 - (STOP_COUNT >> 1)) % IMAGE_COUNT;

            if (i32StopIdx > STOP_COUNT >> 1)
            {
                if (clip_right >= RESX)
                {
                    continue;
                }
                //
                // right side pictures
                //
                nema_set_clip((int32_t)clip_right, 0, (uint32_t)(RESX - clip_right), RESY);
            }
            else
            { //if ( i32StopIdx > STOP_COUNT>>1)
                if (clip_left < 0)
                {
                    continue;
                }
                //
                // left side pictures
                //
                nema_set_clip(0, 0, (uint32_t)clip_left, RESY);
            }

            float shift_target, shift_prev;

            shift_target = nema_ez(.5f, 0.f, STOP_COUNT >> 1, i32StopIdx,     nema_ez_linear) + 0.5f;
            shift_prev   = nema_ez(.5f, 0.f, STOP_COUNT >> 1, i32StopIdx + 1, nema_ez_linear) + 0.5f;

            shift_target = map_to_ratio(shift_target, break_point, 0.19f);

            shift_prev   = map_to_ratio(shift_prev, break_point, 0.19f);

            shift_par    = nema_ez( shift_prev, shift_target, 1.0, cur_sec, &nema_ez_linear);

            Quad_t q1;
            map_quad_points(&q1, shift_par);

            transform_quad_coords(&q1);
            //
            // make sides of quads integers so that clipping is done right
            //
            q1.x0 = q1.x3 = (int32_t) q1.x0;
            q1.x1 = q1.x2 = (int32_t) q1.x1;
            //
            // Reflections
            //
            nema_set_blend_blit(NEMA_BL_SIMPLE | NEMA_BLOP_MODULATE_A);
            bind_img(&g_sImgsRGBA[(img_idx) % IMAGE_COUNT]);

            float screen_img_height_0 = q1.y3 - q1.y0;
            float screen_img_height_1 = q1.y2 - q1.y1;

            nema_blit_quad_fit(q1.x0, q1.y0 + 2.f * screen_img_height_0,
                               q1.x1, q1.y1 + 2.f * screen_img_height_1,
                               q1.x2, q1.y2,
                               q1.x3, q1.y3);
            //
            // Images
            //
            nema_set_blend_blit(NEMA_BL_SIMPLE);
            nema_blit_quad_fit(q1.x0, q1.y0,
                               q1.x1, q1.y1,
                               q1.x2, q1.y2,
                               q1.x3, q1.y3);

            if (i32StopIdx > STOP_COUNT >> 1)
            {
                //
                // right side pictures
                //
                clip_right = q1.x1;
            }
            else if (i32StopIdx < STOP_COUNT >> 1)
            {
                //
                // left side pictures
                //
                clip_left = q1.x0;
            }
            else
            {
                clip_left  = q1.x0;
                clip_right = q1.x1;
            }

            if (clip_right >= RESX && clip_left < 0)
            {
                break;
            }

        }

        nema_cl_submit(psCurCL);

#ifdef COUNT_FPS
        if (++ui32Frames % 100 == 0)
        {
            am_util_stdio_printf("fps: %.2f\n", (100.f / (nema_get_time() - sec0)));
            sec0 = nema_get_time();
            ui32Frames = 0;
        }
#endif

#ifdef MODE_INTERRACTIVE
        static float d_cur_sec = 0.f;
        static float d_FoV     = 0.f;

        nema_event_wait(&event, 0);

        //Fixed-numders tuned for Synopsys platform
        if ( event.mouse_event == MOUSE_EVENT_NONE && (event.mouse_state == MOUSE_STATE_LEFT_CLICKED))
        {
            d_cur_sec -= event.mouse_dx / 90.f;
            if (d_cur_sec > 0.9f)
            {
                d_cur_sec = 0.9f;
            }
            else if (d_cur_sec < -0.9f)
            {
                d_cur_sec = -0.9f;
            }

        }
        else if (event.mouse_event == MOUSE_EVENT_SCROLL_UP)
        {
            d_FoV -= 1.f;
        }
        else if (event.mouse_event == MOUSE_EVENT_SCROLL_DOWN)
        {
             d_FoV += 1.f;
        }

        cur_sec   += d_cur_sec;
        d_cur_sec *= 0.8f;

        FoV   += d_FoV;

        //Swipe momentum
        d_FoV *= 0.8f;

        if (FoV < 20.f)
        {
            FoV   = 20.f;
            d_FoV = 0.f;
        }
        else if (FoV > 35.f)
        {
            FoV   = 35.f;
            d_FoV = 0.f;
        }

        nema_mat4x4_load_perspective(proj_matrix, FoV, (float)RESX / RESY, 1, 100);

        //Continuous mode (not interractive)
#else
        //
        // Small cur_sec => slow transition (does NOT affect the actual fps)
        //
        cur_sec += 0.05;
#endif


        if (cur_sec > secs + 0.5)
        {
            cur_sec  -= secs + 0.5;
            sec_init += secs + 0.5;
            i32CurImg   = (i32CurImg + 1) % IMAGE_COUNT;
        }
        else if (cur_sec < 0.f)
        {
            cur_sec  += secs + 0.5;
            sec_init -= secs + 0.5;
            i32CurImg   = (i32CurImg + IMAGE_COUNT - 1) % IMAGE_COUNT;
        }

        nema_cl_wait(psCurCL);
        //
        // wait transfer done
        //
        am_devices_display_wait_transfer_done();
    }
}
