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
#include "nemagfx_vg_shape.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

#define SHAPE_DRAW_AREA (120)
#define SHAPE_GAP       (10)
#define SHAPE_CIRCLE    ((SHAPE_DRAW_AREA - SHAPE_GAP * 2) / 2)

#define SHAPE_ROW_1 (120)
#define SHAPE_ROW_2 (SHAPE_ROW_1 + SHAPE_DRAW_AREA)
#define SHAPE_COL_1 (65)
#define SHAPE_COL_2 (SHAPE_COL_1 + SHAPE_DRAW_AREA)
#define SHAPE_COL_3 (SHAPE_COL_2 + SHAPE_DRAW_AREA)

#define SHAPE_CIRCLE_CX (SHAPE_COL_1)
#define SHAPE_CIRCLE_CY (SHAPE_ROW_1)
#define SHAPE_CIRCLE_R  (SHAPE_CIRCLE)

#define SHAPE_ELLIPSE_CX (SHAPE_COL_2)
#define SHAPE_ELLIPSE_CY (SHAPE_ROW_1)
#define SHAPE_ELLIPSE_RX (SHAPE_CIRCLE)
#define SHAPE_ELLIPSE_RY (SHAPE_ELLIPSE_RX / 2)

#define SHAPE_RING_CX          (SHAPE_COL_3)
#define SHAPE_RING_CY          (SHAPE_ROW_1)
#define SHAPE_RING_R           (SHAPE_CIRCLE)
#define SHAPE_RING_ANGLE_START (10)
#define SHAPE_RING_ANGLE_END   (330)

#define SHAPE_LINE_X0    (SHAPE_COL_1 - SHAPE_CIRCLE)
#define SHAPE_LINE_Y0    (SHAPE_ROW_2 - SHAPE_CIRCLE)
#define SHAPE_LINE_X1    (SHAPE_COL_1 + SHAPE_CIRCLE)
#define SHAPE_LINE_Y1    (SHAPE_ROW_2 + SHAPE_CIRCLE)
#define SHAPE_LINE_WIDTH (10)

#define SHAPE_ROUNDED_RECT_X     (SHAPE_COL_2 - SHAPE_CIRCLE)
#define SHAPE_ROUNDED_RECT_Y     (SHAPE_ROW_2 - SHAPE_CIRCLE)
#define SHAPE_ROUNDED_RECT_WIDTH ((SHAPE_DRAW_AREA - SHAPE_GAP * 2))
#define SHAPE_ROUNDED_RECT_HIGHT ((SHAPE_DRAW_AREA - SHAPE_GAP * 2))
#define SHAPE_ROUNDED_RECT_RX    (20)
#define SHAPE_ROUNDED_RECT_RY    (30)

#define SHAPE_RECT_X     (SHAPE_COL_3 - SHAPE_CIRCLE)
#define SHAPE_RECT_Y     (SHAPE_ROW_2 - SHAPE_CIRCLE)
#define SHAPE_RECT_WIDTH ((SHAPE_DRAW_AREA - SHAPE_GAP * 2))
#define SHAPE_RECT_HIGHT ((SHAPE_DRAW_AREA - SHAPE_GAP * 2))

#define TOTAL_SHAPES   (6)
#define REFRESH_PERIOD (5000) // In unit of ms

//*****************************************************************************
//
// Render task handle.
//
//*****************************************************************************
TaskHandle_t RenderTaskHandle;

//*****************************************************************************
//
// Function to show different paint.
//
//*****************************************************************************
typedef enum
{
    PAINT_FILL,
    PAINT_LINEAR_GRADIENT,
    PAINT_CONIC_GRADIENT,
    PAINT_RADIAL_GRADIENT,
    PAINT_TEXTURE,
} paint_type_t;

typedef struct
{
    float x;
    float y;
} point_t;

//*****************************************************************************
//
//! @brief Demo of predefined shapes with different paint setting.
//!
//
//*****************************************************************************
void vg_shapes_demo(img_obj_t *des_img, NEMA_VG_PAINT_HANDLE *paint,
                    NEMA_VG_GRAD_HANDLE gradient, paint_type_t paint_type)
{
    // clear framebuffer
    nema_bind_dst_tex(des_img->bo.base_phys, des_img->w, des_img->h,
                      des_img->format, des_img->stride);
    nema_set_clip(0, 0, des_img->w, des_img->h);
    nema_clear(nema_rgba(0xff, 0xff, 0x00, 0xff)); // yellow

    // set vg context
    nema_vg_set_fill_rule(NEMA_VG_FILL_EVEN_ODD);
    nema_vg_set_blend(NEMA_BL_SRC_OVER);

    float       stops[5]  = {0.0f, 0.25f, 0.50f, 0.75f, 1.0f};
    color_var_t colors[5] =
    {
        {0, 0, 255, 255},     // blue
        {0, 255, 0, 255},     // green
        {255, 255, 255, 255}, // white
        {255, 165, 0, 255},   // orange
        {255, 0, 0, 255}      // red
    };

    nema_vg_grad_set(gradient, 5, stops, colors);

    point_t center_point[TOTAL_SHAPES] =
    {
        {SHAPE_COL_1, SHAPE_ROW_1}, {SHAPE_COL_2, SHAPE_ROW_1},
        {SHAPE_COL_3, SHAPE_ROW_1}, {SHAPE_COL_1, SHAPE_ROW_2},
        {SHAPE_COL_2, SHAPE_ROW_2}, {SHAPE_COL_3, SHAPE_ROW_2},
    };

    nema_matrix3x3_t m_paint;

    // Set up paint parameter
    switch (paint_type)
    {
        case PAINT_FILL:
            for (int i = 0; i < TOTAL_SHAPES; i++)
            {
                nema_vg_paint_set_type(paint[i], NEMA_VG_PAINT_COLOR);
                nema_vg_paint_set_stroke_width(paint[i], 15.0f);
                nema_vg_paint_set_paint_color(paint[i],
                                              nema_rgba(0xff, 0x00, 0xff, 0x80));
            }
            break;
        case PAINT_LINEAR_GRADIENT:
            for (int i = 0; i < TOTAL_SHAPES; i++)
            {
                nema_vg_paint_set_type(paint[i], NEMA_VG_PAINT_GRAD_LINEAR);
                nema_vg_paint_set_stroke_width(paint[i], 15.0f);
                nema_vg_paint_set_grad_linear(
                    paint[i], gradient, 0, center_point[i].y - SHAPE_DRAW_AREA / 2,
                    0, center_point[i].y + SHAPE_DRAW_AREA / 2,
                    NEMA_TEX_CLAMP | NEMA_FILTER_BL);
            }

            break;
        case PAINT_CONIC_GRADIENT:
            for (int i = 0; i < TOTAL_SHAPES; i++)
            {
                nema_vg_paint_set_type(paint[i], NEMA_VG_PAINT_GRAD_CONICAL);
                nema_vg_paint_set_stroke_width(paint[i], 15.0f);
                nema_vg_paint_set_grad_conical(paint[i], gradient,
                                               center_point[i].x, center_point[i].y,
                                               NEMA_TEX_CLAMP | NEMA_FILTER_BL);
            }
            break;
        case PAINT_RADIAL_GRADIENT:
            for (int i = 0; i < TOTAL_SHAPES; i++)
            {
                nema_vg_paint_set_type(paint[i], NEMA_VG_PAINT_GRAD_RADIAL);
                nema_vg_paint_set_stroke_width(paint[i], 15.0f);
                nema_vg_paint_set_grad_radial(paint[i], gradient, center_point[i].x,
                                              center_point[i].y, SHAPE_CIRCLE,
                                              NEMA_TEX_CLAMP | NEMA_FILTER_BL);
            }
            break;
        case PAINT_TEXTURE:
            for (int i = 0; i < TOTAL_SHAPES; i++)
            {
                nema_mat3x3_load_identity(m_paint);
                nema_mat3x3_translate(m_paint, center_point[i].x - g_sRef.w / 2.f,
                                      center_point[i].y - g_sRef.h / 2.f);

                nema_vg_paint_set_type(paint[i], NEMA_VG_PAINT_TEXTURE);
                nema_vg_paint_set_tex(paint[i], &g_sRef);
                nema_vg_paint_set_stroke_width(paint[i], 15.0f);
                nema_vg_paint_set_tex_matrix(paint[i], m_paint);
            }
            break;
        default:
            break;
    }

    // Draw shapes
    nema_vg_draw_circle(SHAPE_CIRCLE_CX, SHAPE_CIRCLE_CY, SHAPE_CIRCLE_R, NULL,
                        paint[0]);

    nema_vg_draw_ellipse(SHAPE_ELLIPSE_CX, SHAPE_ELLIPSE_CY, SHAPE_ELLIPSE_RX,
                         SHAPE_ELLIPSE_RY, NULL, paint[1]);

    nema_vg_draw_ring(SHAPE_RING_CX, SHAPE_RING_CY, SHAPE_RING_R,
                      SHAPE_RING_ANGLE_START, SHAPE_RING_ANGLE_END, paint[2]);

    nema_vg_draw_line(SHAPE_LINE_X0, SHAPE_LINE_Y0, SHAPE_LINE_X1,
                      SHAPE_LINE_Y1, NULL, paint[3]);

    nema_vg_draw_rounded_rect(SHAPE_ROUNDED_RECT_X, SHAPE_ROUNDED_RECT_Y,
                              SHAPE_ROUNDED_RECT_WIDTH,
                              SHAPE_ROUNDED_RECT_HIGHT, SHAPE_ROUNDED_RECT_RX,
                              SHAPE_ROUNDED_RECT_RY, NULL, paint[4]);

    nema_vg_draw_rect(SHAPE_RECT_X, SHAPE_RECT_Y, SHAPE_RECT_WIDTH,
                      SHAPE_RECT_HIGHT, NULL, paint[5]);
}

//*****************************************************************************
//
//! @brief Render task.
//!
//
//*****************************************************************************
void RenderTask(void *pvParameters)
{
    int ret;

#ifdef GPU_WORK_TIME_PRINT
    uint32_t time_start;
    uint32_t time_end;
#endif

    am_util_stdio_printf("Render task start!\n");

    // Power on GPU
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

    // Initialize NemaGFX
    ret = nema_init();
    if (ret != 0)
    {
        am_util_stdio_printf("GPU init failed!\n");

        // suspend and delete this task.
        vTaskDelete(NULL);
    }

    // Init vg
    nema_vg_init(g_pFrameBufferGPU->w, g_pFrameBufferGPU->h);

    // Create paint
    NEMA_VG_PAINT_HANDLE paint[TOTAL_SHAPES];

    for (int i = 0; i < TOTAL_SHAPES; i++)
    {
        paint[i] = nema_vg_paint_create();
    }

    // Create gradient, this gradient will be reused by different paint.
    NEMA_VG_GRAD_HANDLE gradient = nema_vg_grad_create();

    // Create GPU command list
    nema_cmdlist_t cl = nema_cl_create();
    nema_cl_bind(&cl);

    uint32_t start_tick         = xTaskGetTickCount();
    uint32_t paint_type_pointer = 0;

    paint_type_t shapes_paint[] =
    {
        PAINT_FILL,
        PAINT_LINEAR_GRADIENT,
        PAINT_CONIC_GRADIENT,
        PAINT_RADIAL_GRADIENT,
        PAINT_TEXTURE,
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

        // Change the paint type at each second.
        uint32_t current_tick = xTaskGetTickCount();
        if (current_tick - start_tick > REFRESH_PERIOD)
        {
            start_tick = current_tick;
            paint_type_pointer++;
            if ( paint_type_pointer >=
                 (sizeof(shapes_paint) / sizeof(shapes_paint[0])) )
            {
                paint_type_pointer = 0;
            }
        }

        // Draw the shapes.
        vg_shapes_demo(g_pFrameBufferGPU, paint, gradient,
                       shapes_paint[paint_type_pointer]);

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

    // Release resource
    for (int i = 0; i < TOTAL_SHAPES; i++)
    {
        nema_vg_paint_destroy(paint[i]);
    }
    nema_vg_grad_destroy(gradient);

    nema_vg_deinit();
}
