//*****************************************************************************
//
//! @file render_raw_ttf.c
//!
//! @brief A demo to show how to draw ttf font without using the nema_vg_print.
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "nemagfx_vg_test.h"

#ifdef RUN_RENDER_RAW_TTF


//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
//! Frame buffer size
#define FB_RESX (200)
#define FB_RESY (200)

//! Select frame buffer format
#define USE_RGB888  1
#define USE_RGB565  0

#if USE_RGB888
    #define DISPLAY_FORMAT            NEMADC_RGB24
    #define FRAME_BUFFER_FORMAT       NEMA_RGB24
#elif USE_RGB565
    #define DISPLAY_FORMAT            NEMADC_RGB565
    #define FRAME_BUFFER_FORMAT       NEMA_RGB565
#else
    #error "Display format not supported!"
#endif

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
//! Frame buffer
static nema_img_obj_t g_sFrameBuffer =
{
    .bo = {0},
    .w = FB_RESX,
    .h = FB_RESY,
    .stride = -1,
    .color =  0,
    .format = FRAME_BUFFER_FORMAT,
    .sampling_mode = NEMA_FILTER_BL
};

//! DC layer
nemadc_layer_t g_sDCLayer =
{
    .startx        = 0,
    .sizex         = FB_RESX,
    .resx          = FB_RESX,
    .starty        = 0,
    .sizey         = FB_RESY,
    .resy          = FB_RESY,
    .stride        = -1,
    .format        = DISPLAY_FORMAT,
    .blendmode     = NEMADC_BL_SRC,
    .buscfg        = 0,
    .alpha         = 0xff,
    .flipx_en      = 0,
    .flipy_en      = 0,
    .extra_bits    = 0
};

//*****************************************************************************
//
//! @brief Allocate buffer from SSRAM
//!
//
//*****************************************************************************
int
SSRAM_buffer_alloc(img_obj_t* img)
{
    uint32_t size;

    size = nema_texture_size(img->format, 0, img->w, img->h);

    img->bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, size);

    if ( img->bo.base_virt == NULL )
    {
      am_util_stdio_printf("TSI buffer Malloc failed!\n");
      return -1;
    }

    return 0;
}

//*****************************************************************************
//
//! @brief Render raw TTF font
//
//*****************************************************************************
int render_raw_ttf(void)
{
    // Initialize NemaGFX
    if ( nema_init() != 0 )
    {
        return -1;
    }

    // Init NemaVG
    nema_vg_init(FB_RESX, FB_RESY);

    // Initialize NemaDC
    if ( display_setup(FB_RESX, FB_RESY) != 0 )
    {
        return -2;
    }

    // Create frame buffer
    if ( SSRAM_buffer_alloc(&g_sFrameBuffer) != 0 )
    {
        return -3;
    }

    // Set layer
    g_sDCLayer.baseaddr_phys = g_sFrameBuffer.bo.base_phys;
    g_sDCLayer.baseaddr_virt = g_sFrameBuffer.bo.base_virt;
    nemadc_set_layer(0, &g_sDCLayer);

    //Create Command List
    nema_cmdlist_t cl  =  nema_cl_create_sized(1*1024);
    //Bind Command List
    //Use Circular CL to Parallelize CPU and GPU
    nema_cl_bind_circular(&cl);

    //Set Framebuffer
    nema_bind_dst_tex(g_sFrameBuffer.bo.base_phys,
                      g_sFrameBuffer.w,
                      g_sFrameBuffer.h,
                      g_sFrameBuffer.format,
                      g_sFrameBuffer.stride);

    //Set Clipping Rectangle
    nema_set_clip(0, 0, FB_RESX, FB_RESY);

    //Clear frame buffer
    nema_clear(0x00000000);
    nema_cl_submit(&cl);
    nema_cl_wait(&cl);
    nema_cl_rewind(&cl);

#if 0
    uint8_t segs[] = {NEMA_VG_PRIM_MOVE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_LINE,
                      NEMA_VG_PRIM_CLOSE};
    float points[] = {8.67f,  -0.00f, 7.23f,  -0.00f,  7.23f,   -5.27f, 2.66f,   -5.27f, 2.66f,
                      -0.00f, 1.23f,  -0.00f, 1.23f,   -11.38f, 2.66f,  -11.38f, 2.66f,  -6.48f,
                      7.23f,  -6.48f, 7.23f,  -11.38f, 8.67f,   -11.38, 8.67f,   -0.00f};
#else
     uint8_t segs[] = {NEMA_VG_PRIM_MOVE,        NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_LINE,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_LINE,
                       NEMA_VG_PRIM_LINE,        NEMA_VG_PRIM_LINE,        NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_LINE,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_MOVE,        NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_LINE,        NEMA_VG_PRIM_LINE,        NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD, NEMA_VG_PRIM_BEZIER_QUAD,
                       NEMA_VG_PRIM_CLOSE};
    float points[] = {
        4.06f, 0.16f,  3.30f, 0.16f,  2.69f, -0.06f, 2.08f, -0.30f, 1.66f, -0.77f, 1.23f, -1.23f,
        1.00f, -1.94f, 0.78f, -2.66f, 0.78f, -3.62f, 0.78f, -4.58f, 0.78f, -5.69f, 1.05f, -6.45f,
        1.31f, -7.23f, 1.73f, -7.70f, 2.17f, -8.19f, 2.73f, -8.39f, 3.30f, -8.61f, 3.91f, -8.61f,
        4.70f, -8.61f, 5.27f, -8.34f, 5.83f, -8.09f, 6.19f, -7.58f, 6.55f, -7.08f, 6.70f, -6.33f,
        6.88f, -5.59f, 6.88f, -4.62f, 6.88f, -3.81f, 2.16f, -3.81f, 2.16f, -3.62f, 2.16f, -2.27f,
        2.66f, -1.64f, 3.17f, -1.03,  4.14f, -1.03f, 4.48f, -1.03f, 4.77f, -1.09f, 5.05f, -1.17f,
        5.28f, -1.30f, 5.52f, -1.42f, 5.70f, -1.59f, 5.91f, -1.78f, 6.08f, -1.98f, 6.80f, -1.11f,
        6.62f, -0.88f, 6.38f, -0.64f, 6.14f, -0.42f, 5.80f, -0.23f, 5.47f, -0.06f, 5.03f, 0.05f,
        4.61f, 0.16f,  4.06f, 0.16f,  3.91f, -7.42f, 3.55f, -7.42f, 3.23f, -7.31f, 2.94f, -7.20f,
        2.70f, -6.92f, 2.47f, -6.66f, 2.33f, -6.19f, 2.19f, -5.72f, 2.16f, -5.00f, 5.48f, -5.00f,
        5.48f, -5.19f, 5.47f, -5.69f, 5.38f, -6.09f, 5.30f, -6.50f, 5.12f, -6.80f, 4.95f, -7.09f,
        4.66f, -7.25f, 4.36f, -7.42f, 3.91f, -7.42f};
#endif

    // Create PATH
    NEMA_VG_PATH_HANDLE path = nema_vg_path_create();
    nema_vg_path_set_shape(path, sizeof(segs) / sizeof(segs[0]), segs, sizeof(points) / sizeof(points[0]), points);

    // Set fill rule
    nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);

    // Set quality
    static volatile uint8_t aa = NEMA_VG_QUALITY_MAXIMUM;
    nema_vg_set_quality(aa);

    // Set blend mode
    nema_vg_set_blend(NEMA_BL_SRC_OVER);

    // Create paint
    NEMA_VG_PATH_HANDLE paint = nema_vg_paint_create();
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_COLOR);
    nema_vg_paint_set_paint_color(paint, 0xFFFFFFFF);

    float time_start = nema_get_time();

    // Draw character
    nema_matrix3x3_t matrix;

    for (int x = 0; x < 20; x++)
    {
      nema_mat3x3_load_identity(matrix);
      nema_mat3x3_translate(matrix, x * 10, 0);

      for (int i = 0; i < 10; i++)
      {
        nema_mat3x3_translate(matrix, 0, 20);
        nema_vg_set_global_matrix(matrix);
        nema_vg_draw_path(path, paint);
      }
    }

    // Submit and wait CL
    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    // Print time used.
    float time_end = nema_get_time();
    float time_used = (time_end - time_start) * 1000.0;
    am_util_stdio_printf("Run time: %.3fms\n", time_used);

    // Start DC
    display_refresh_start();

    // Wait DC complete interrupt.
    nemadc_wait_vsync();

    // Do follow-up operations required by hardware.
    display_refresh_end();

    // Clean up
    nema_vg_paint_destroy(paint);
    nema_vg_path_destroy(path);

    // Destroy (free) Command List
    nema_cl_destroy(&cl);

    // De-initialize nema_vg
    nema_vg_deinit();

    return 0;
}

#endif
