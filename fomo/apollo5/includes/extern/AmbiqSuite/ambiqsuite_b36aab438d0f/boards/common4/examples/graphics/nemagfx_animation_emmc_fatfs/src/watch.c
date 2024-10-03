//*****************************************************************************
//
//! @file watch.c
//!
//! @brief NemaGFX example.
//!
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
#include "watch.h"
#include "am_bsp.h"
#include "am_hal_global.h"
#ifndef DONT_USE_NEMADC
#include "nema_dc.h"
#include "nema_dc_mipi.h"
#endif
#include "hand_0_hour.h"
#include "hand_0_minute.h"
#include "hand_0_second.h"
#include "hand_1_hour.h"
#include "hand_1_minute.h"
#include "hand_1_second.h"
#include "mask_a8.h"
#include "string.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define WATCH_STYLE         2

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
uintptr_t g_pWatchAddress = MSPI_XIP_BASE_ADDRESS;
static const watchPivot_t pivot[WATCH_STYLE] = { {{-13, -109}, {-14, -157}, {-10, -158}},           \
                                                 {{-11, -140}, {-14, -139}, { -9, -148}}};

static img_obj_t hour_hand[WATCH_STYLE] = {{{0},  28, 125, -1, 1, NEMA_RGBA8888, NEMA_FILTER_BL},   \
                                        {{0},  22, 104, -1, 0, NEMA_RGBA8888, NEMA_FILTER_BL}};
static img_obj_t min_hand[WATCH_STYLE] = {{{0},  28, 174, -1, 1, NEMA_RGBA8888, NEMA_FILTER_BL},    \
                                        {{0},  28, 153, -1, 0, NEMA_RGBA8888, NEMA_FILTER_BL}};
static img_obj_t sec_hand[WATCH_STYLE] = {{{0},  21, 182, -1, 1, NEMA_RGBA8888, NEMA_FILTER_BL},    \
                                        {{0},  18, 219, -1, 0, NEMA_RGBA8888, NEMA_FILTER_BL}};

static img_obj_t mask =              {{0},  RESOLUTION_X, RESOLUTION_Y, -1, 1, NEMA_A8, NEMA_FILTER_BL};

static nema_cmdlist_t wf_cl;

static nema_buffer_t
createBuffer(void *ptr, size_t size)
{
    nema_buffer_t bo;
    bo.base_phys = g_pWatchAddress;
    bo.base_virt = (void *)(bo.base_phys);
    bo.size = size;
    memcpy(bo.base_virt, ptr, size);
    g_pWatchAddress += (bo.size + 7) >> 3 << 3;
    return bo;
}

void
g_load_watch_images()
{
    hour_hand[STYLE_ONE].bo = createBuffer((void*)hand_0_hour_rgba, sizeof(hand_0_hour_rgba));
    min_hand[STYLE_ONE].bo = createBuffer((void*)hand_0_minute_rgba, sizeof(hand_0_minute_rgba));
    sec_hand[STYLE_ONE].bo = createBuffer((void*)hand_0_second_rgba, sizeof(hand_0_second_rgba));

    hour_hand[STYLE_TWO].bo = createBuffer((void*)hand_1_hour_rgba, sizeof(hand_1_hour_rgba));
    min_hand[STYLE_TWO].bo = createBuffer((void*)hand_1_minute_rgba, sizeof(hand_1_minute_rgba));
    sec_hand[STYLE_TWO].bo = createBuffer((void*)hand_1_second_rgba, sizeof(hand_1_second_rgba));
    mask.bo = createBuffer((void*)mask_a8, sizeof(mask_a8));
    wf_cl = nema_cl_create();
}
//void
//destroyImagesBuffer(uint8_t style)
//{
//    nema_buffer_destroy(&hour_hand[style].bo);
//    nema_buffer_destroy(&min_hand[style].bo);
//    nema_buffer_destroy(&sec_hand[style].bo);
//}

static inline void
drawHand(img_obj_t *img, float x0, float y0, float angle, int cx, int cy)
{
    float x1 = x0 + img->w,  y1 = y0;
    float x2 = x0 + img->w,  y2 = y0 + img->h;
    float x3 = x0       ,  y3 = y0 + img->h;

    //calculate rotation matrix
    nema_matrix3x3_t m;
    nema_mat3x3_load_identity(m);
    nema_mat3x3_rotate(m, -angle);
    nema_mat3x3_translate(m, cx, cy);

    //rotate points
    nema_mat3x3_mul_vec(m, &x0, &y0);
    nema_mat3x3_mul_vec(m, &x1, &y1);
    nema_mat3x3_mul_vec(m, &x2, &y2);
    nema_mat3x3_mul_vec(m, &x3, &y3);

    //draw hand
    nema_bind_src_tex( img->bo.base_phys, img->w, img->h, img->format, img->stride, NEMA_FILTER_BL);
    nema_blit_quad_fit(x0, y0,
                       x1, y1,
                       x2, y2,
                       x3, y3);
}

nema_cmdlist_t *
g_draw_watch_hands(float time, uint8_t style)
{
    float angle;
    nema_cl_rewind(&wf_cl);
    nema_cl_bind(&wf_cl);

    nema_set_clip(0, 0, RESOLUTION_X, RESOLUTION_Y);
    nema_clear(0);
    nema_set_blend_blit(NEMA_BL_SIMPLE);

    //draw hour hand
    angle =  time / 120.0f; //time * 1 / 60 / 60 / 12.f * 360.f;
    drawHand( &hour_hand[style], pivot[style].hour.hand_x, pivot[style].hour.hand_y, -angle, RESOLUTION_X / 2, RESOLUTION_Y / 2);

    //draw minute hand
    angle =  time / 10.0f;  //time /60 / 60.f * 360.f;
    drawHand( &min_hand[style], pivot[style].min.hand_x, pivot[style].min.hand_y , -angle, RESOLUTION_X / 2, RESOLUTION_Y / 2);

    //draw second hand
    angle = time * 6.0f;    //time / 60.f * 360.f;
    drawHand( &sec_hand[style], pivot[style].sec.hand_x, pivot[style].sec.hand_y, -angle, RESOLUTION_X / 2, RESOLUTION_Y / 2);

    //nema_set_blend_blit(NEMA_BL_SIMPLE);
    //nema_set_tex_color(0xff000000);
    nema_bind_src_tex( mask.bo.base_phys, mask.w, mask.h, mask.format, mask.stride, NEMA_FILTER_BL);
    nema_blit(0, 0);

    nema_cl_return();

    return &wf_cl;
}

