//*****************************************************************************
//
//! @file render_task.c
//!
//! @brief Task to handle GPU render operations.
//!
//!
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

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "nemagfx_vg_masking.h"

#include "nema_core.h"
#include "nema_event.h"
#include "nema_hal.h"
#include "nema_math.h"
#include "nema_programHW.h"
#include "nema_utils.h"

#include "nema_vg.h"
#include "nema_vg_context.h"
#include "nema_vg_font.h"
#include "nema_vg_paint.h"
#include "nema_vg_path.h"
#include "nema_vg_tsvg.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define REFRESH_PERIOD (2000UL) // uint ms

//*****************************************************************************
//
// Render task handle.
//
//*****************************************************************************
TaskHandle_t RenderTaskHandle;

typedef enum
{
    CIRCLE_MASKING,
    ROUNDED_RECT_MASKING,
    ELLIPSE_MASKING,
} masking_type_e;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Create VG mask
//!
//! @param des_img - image object to hold the mask image.
//! @param type - Make type.
//!
//! @return None.
//
//*****************************************************************************

void masking_create(img_obj_t *des_img, masking_type_e type)
{
    // Bind framebuffer
    nema_bind_dst_tex(des_img->bo.base_phys, des_img->w, des_img->h, NEMA_L8,
                      des_img->stride);

    // Set clip
    nema_set_clip(0, 0, des_img->w, des_img->h);

    // Set Blending Mode
    nema_set_blend_fill(NEMA_BL_SRC);

    // Clear
    nema_fill_rect(0, 0, des_img->w, des_img->h, 0);

    // set vg context
    nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
    nema_vg_set_blend(NEMA_BL_SRC_OVER);

    // Create paint.
    NEMA_VG_PAINT_HANDLE paint = nema_vg_paint_create();

    // set paint
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_COLOR);
    nema_vg_paint_set_paint_color(paint, nema_rgba(0xff, 0xff, 0xff, 0xff));

    switch (type)
    {
        case CIRCLE_MASKING:
            nema_vg_draw_circle(des_img->w / 2, des_img->h / 2,
                                nema_min2(des_img->w / 2, des_img->h / 2), NULL,
                                paint);
            break;
        case ROUNDED_RECT_MASKING:
            nema_vg_draw_rounded_rect(0, 0, des_img->w, des_img->h,
                                      des_img->w * 0.20f, des_img->h * 0.20f, NULL,
                                      paint);
            break;
        case ELLIPSE_MASKING:
            nema_vg_draw_ellipse(des_img->w / 2, des_img->h / 2, des_img->w * 0.45f,
                                 des_img->h * 0.35f, NULL, paint);
            break;
    }

    nema_vg_paint_destroy(paint);
}

static void
mask_enable(void)
{
    static const uint32_t cmd[] =
    {
        0x100c118bU, 0x00002000U,
        0x000c0000U, 0x0081a042U,
        0x080c0003U, 0x0a012007U,
        0x004e0002U, 0x804b1286U
    };

    nema_set_blend_blit(NEMA_BL_SIMPLE | NEMA_BLOP_NO_USE_ROPBL);
    nema_load_frag_shader(cmd, 4, 0U);
    nema_set_frag_ptr(0x141c8000U);
}
//*****************************************************************************
//
//! @brief Apply VG mask
//!
//! @param des_img - frame buffer.
//! @param src_img - texture image.
//! @param masking - mask image
//! @param ratio - scale ratio.
//!
//! @return None.
//
//*****************************************************************************
void masking_apply(img_obj_t *des_img, img_obj_t *src_img, img_obj_t *masking,
                   float ratio)
{
    // Bind framebuffer
    nema_bind_dst_tex(des_img->bo.base_phys, des_img->w, des_img->h,
                      des_img->format, des_img->stride);

    // Set clip
    nema_set_clip(0, 0, des_img->w, des_img->h);

    // Set Blending Mode
    nema_set_blend_fill(NEMA_BL_SRC);

    // Clear
    nema_fill_rect(0, 0, des_img->w, des_img->h, 0);

    // Bind source texture
    nema_bind_src_tex(src_img->bo.base_phys, src_img->w, src_img->h,
                      src_img->format, src_img->stride, src_img->sampling_mode);

    // MASK should be loaded in TEX3 slot
    nema_bind_tex(NEMA_TEX3, masking->bo.base_phys, masking->w, masking->h,
                  masking->format, masking->stride, NEMA_FILTER_BL);

    mask_enable();
    // nema_set_blend_blit_compose(NEMA_BL_SIMPLE);
    //nema_set_blend_blit_compose(NEMA_BL_SRC);

    int width   = src_img->w * ratio;
    int hight   = src_img->h * ratio;
    int start_x = (des_img->w - width) * 0.5;
    int start_y = (des_img->h - hight) * 0.5;

    // blit and fit
    nema_blit_quad_fit(start_x, start_y, start_x + width, start_y,
                       start_x + width, start_y + hight, start_x,
                       start_y + hight);
}

//*****************************************************************************
//
//! @brief Implement mask effect with vg paint
//!
//! @param des_img - frame buffer.
//! @param tex_img - texture image.
//! @param ratio - scale ratio.
//! @param type - Make type.
//!
//! @return None.
//
//*****************************************************************************
void vg_mask(img_obj_t *des_img, img_obj_t *tex_img, float ratio,
             masking_type_e type)
{
    // Bind framebuffer
    nema_bind_dst_tex(des_img->bo.base_phys, des_img->w, des_img->h,
                      des_img->format, des_img->stride);

    // Set clip
    nema_set_clip(0, 0, des_img->w, des_img->h);

    // Set Blending Mode
    nema_set_blend_fill(NEMA_BL_SRC);

    // Clear
    nema_fill_rect(0, 0, des_img->w, des_img->h, 0);

    // Set up VG context
    nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
    nema_vg_set_blend(NEMA_BL_SRC_OVER);

    // Create paint.
    NEMA_VG_PAINT_HANDLE paint = nema_vg_paint_create();

    // Set paint to texture paint
    nema_vg_paint_set_type(paint, NEMA_VG_PAINT_TEXTURE);
    nema_vg_paint_set_tex(paint, tex_img);

    // set paint matrix
    nema_matrix3x3_t m_paint;
    nema_mat3x3_load_identity(m_paint);
    nema_mat3x3_translate(m_paint, -tex_img->w * 0.5f, -tex_img->h * 0.5f);
    nema_mat3x3_scale(m_paint, ratio, ratio);
    nema_mat3x3_translate(m_paint, des_img->w * 0.5f, des_img->h * 0.5f);
    nema_vg_paint_set_tex_matrix(paint, m_paint);

    float width = tex_img->w * ratio;
    float hight = tex_img->h * ratio;

    switch (type)
    {
        case CIRCLE_MASKING:
            nema_vg_draw_circle(des_img->w * 0.5f, des_img->h * 0.5f,
                                nema_min2(width, hight) * 0.5f, NULL, paint);
            break;
        case ROUNDED_RECT_MASKING:
            nema_vg_draw_rounded_rect((des_img->w - width) * 0.5f,
                                      (des_img->h - hight) * 0.5f, width, hight,
                                      width * 0.20f, hight * 0.20f, NULL, paint);
            break;
        case ELLIPSE_MASKING:
            nema_vg_draw_ellipse(des_img->w * 0.5f, des_img->h * 0.5f,
                                 width * 0.45f, hight * 0.35f, NULL, paint);
            break;
    }

    nema_vg_paint_destroy(paint);
}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void RenderTask(void *pvParameters)
{
#ifdef GPU_WORK_TIME_PRINT
    uint32_t time_start;
    uint32_t time_end;
#endif

    am_util_stdio_printf("Render task start!\n");

    // Power on GPU
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

    // Initialize NemaGFX
    int ret = nema_init();
    if (ret != 0)
    {
        am_util_stdio_printf("GPU init failed!\n");

        // suspend and delete this task.
        vTaskDelete(NULL);
    }

    // Init vg
    nema_vg_init(g_sFrameBuffer.w, g_sFrameBuffer.h);

    // Create GPU command list
    nema_cmdlist_t cl = nema_cl_create_sized(16 * 1024);

    uint32_t start_tick   = xTaskGetTickCount();
    uint32_t type_pointer = 0;

    masking_type_e masking_shape[] =
    {
        CIRCLE_MASKING,
        ROUNDED_RECT_MASKING,
        ELLIPSE_MASKING,
    };

    while (1)
    {
        // Wait start.
        xSemaphoreTake(g_semGPUStart, portMAX_DELAY);

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_clear(DEBUG_PIN_3);
        am_hal_gpio_output_clear(DEBUG_PIN_6);
#endif

        // rewind and bind the CL
        nema_cl_rewind(&cl);
        nema_cl_bind_circular(&cl);

        // Change the paint type at each second.
        uint32_t current_tick = xTaskGetTickCount();
        if (current_tick - start_tick > REFRESH_PERIOD)
        {
            start_tick = current_tick;
            type_pointer++;
            if ( type_pointer >=
                 (sizeof(masking_shape) / sizeof(masking_shape[0])) )
            {
                type_pointer = 0;
            }
        }

        //Option A: Create mask with NemaVG and apply it to a RGB image
        // masking_create(&g_sMask, masking_shape[type_pointer]);
        // masking_apply(&g_sFrameBuffer, &g_sBoyImage, &g_sMask, 0.4f);

        //Option B: Draw a NemaVG shape with texture paint.
        vg_mask(&g_sFrameBuffer, &g_sBoyImage, 0.4f,
                masking_shape[type_pointer]);

        // start GPU, submit CL
        nema_cl_submit(&cl);
#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_set(DEBUG_PIN_1);
#endif

#ifdef GPU_WORK_TIME_PRINT
        time_start = xTaskGetTickCount();
#endif

        // Wait GPU
        nema_cl_wait(&cl);

#ifdef GPU_WORK_TIME_PRINT
        time_end = xTaskGetTickCount();

        am_util_stdio_printf("%d\n", time_end - time_start);
#endif

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_clear(DEBUG_PIN_1);
#endif

        // Notify the GUI task.
        xSemaphoreGive(g_semGPUEnd);

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_set(DEBUG_PIN_3);
#endif
    }

    nema_vg_deinit();
}
