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
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "nemagfx_masking.h"

#include "nema_hal.h"
#include "nema_math.h"
#include "nema_core.h"
#include "nema_utils.h"
#include "nema_event.h"
#include "nema_programHW.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
typedef enum
{
    ROTATION_EFFECT,
    SCALE_EFFECT,
} visual_effect_e;

//*****************************************************************************
//
// Render task handle.
//
//*****************************************************************************
TaskHandle_t RenderTaskHandle;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
uint32_t g_ui32FrameTime = 0;

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
// Scale the texture with masking effect.
//
//*****************************************************************************
void scale_masking(img_obj_t *des_img, img_obj_t *src_img, float ratio)
{
    float width;
    float hight;
    float start_x;
    float start_y;

    //Bind framebuffer
    nema_bind_dst_tex(des_img->bo.base_phys,
                      des_img->w,
                      des_img->h,
                      des_img->format,
                      des_img->stride);

    //Set clip
    nema_set_clip(0, 0, des_img->w, des_img->h);

    //Set Blending Mode
    nema_set_blend_fill(NEMA_BL_SRC);

    //Clear
    nema_fill_rect(0, 0, des_img->w, des_img->h, 0);

    //Bind source texture
    nema_bind_src_tex(src_img->bo.base_phys,
                      src_img->w,
                      src_img->h,
                      src_img->format,
                      src_img->stride,
                      src_img->sampling_mode);

    // MASK should be loaded in TEX3 slot
    nema_bind_tex(NEMA_TEX3,
                  g_sMask.bo.base_phys,
                  g_sMask.w, g_sMask.h,
                  g_sMask.format,
                  g_sMask.stride,
                  NEMA_FILTER_BL);
    mask_enable();

    width = src_img->w * ratio;
    hight = src_img->h * ratio;
    start_x = ( des_img->w - width ) * 0.5;
    start_y = ( des_img->h - hight ) * 0.5;

    nema_blit_quad_fit(start_x, start_y, start_x + width, start_y, start_x + width, start_y + hight, start_x, start_y + hight);

}


//*****************************************************************************
//
// Rotate an image with any specified angle.
//
//*****************************************************************************
static inline void
texture_rotate( img_obj_t *img, float x0, float y0, float angle, float cx, float cy )
{
    float x1 = x0 + img->w,  y1 = y0;
    float x2 = x0 + img->w,  y2 = y0 + img->h;
    float x3 = x0       ,  y3 = y0 + img->h;

    //calculate rotation matrix
    nema_matrix3x3_t m;
    nema_mat3x3_load_identity(m);
    nema_mat3x3_rotate(m, angle);
    nema_mat3x3_translate(m, cx, cy);

    //rotate points
    nema_mat3x3_mul_vec(m, &x0, &y0);
    nema_mat3x3_mul_vec(m, &x1, &y1);
    nema_mat3x3_mul_vec(m, &x2, &y2);
    nema_mat3x3_mul_vec(m, &x3, &y3);

    //draw texture with morton order
    nema_bind_src_tex(img->bo.base_phys, img->w, img->h, img->format, img->stride, img->sampling_mode);
    nema_blit_quad_fit(x0, y0,
                       x1, y1,
                       x2, y2,
                       x3, y3);
}

//*****************************************************************************
//
// Rotate the texture with masking effect.
//
//*****************************************************************************
void
rotate_masking(img_obj_t *des_img, img_obj_t *src_img, float angle)
{
    // Bind framebuffer
    nema_bind_dst_tex(des_img->bo.base_phys,
                      des_img->w,
                      des_img->h,
                      des_img->format,
                      des_img->stride);

    //Enable tiled rendering feature
    nema_enable_tiling(true);

    //Set clip
    nema_set_clip(0, 0, FB_RESX, FB_RESY);

    //Set Blending Mode
    nema_set_blend_fill(NEMA_BL_SRC);

    //Clear
    nema_fill_rect(0, 0, FB_RESX, FB_RESY, 0);

    // MASK should be loaded in TEX3 slot
    nema_bind_tex(NEMA_TEX3,
                  g_sMask.bo.base_phys,
                  g_sMask.w, g_sMask.h,
                  g_sMask.format,
                  g_sMask.stride,
                  NEMA_FILTER_BL);
    mask_enable();

    //Rotate texture
    float src_center_x = src_img->w * 0.5f;
    float src_center_y = src_img->h * 0.5f;
    float des_center_x = des_img->w * 0.5f;
    float des_center_y = des_img->h * 0.5f;
    texture_rotate(src_img, -src_center_x, -src_center_y, angle, des_center_x, des_center_y );
}


//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
RenderTask(void *pvParameters)
{
    int ret;
    static nema_cmdlist_t cl;
    visual_effect_e eCurrentEffect = SCALE_EFFECT;
    uint32_t ui32LastFrameTime = 0;
    float angle;
    float ratio;

#ifdef GPU_WORK_TIME_PRINT
    uint32_t time_start;
    uint32_t time_end;
#endif

    am_util_stdio_printf("Render task start!\n");

    //Power on GPU
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

    //Initialize NemaGFX
    ret = nema_init();
    if (ret != 0)
    {
        am_util_stdio_printf("GPU init failed!\n");

        //suspend and delete this task.
        vTaskDelete(NULL);
    }

    // Create GPU command list
    cl = nema_cl_create();

    while(1)
    {
        //Wait start.
        xSemaphoreTake( g_semGPUStart, portMAX_DELAY);

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_clear(DEBUG_PIN_3);
        am_hal_gpio_output_clear(DEBUG_PIN_6);
#endif

        //rewind and bind the CL
        nema_cl_rewind(&cl);
        nema_cl_bind(&cl);

        //If the frame time rollback, we need to toggle the display effect between scale and rotation.
        if ( g_ui32FrameTime < ui32LastFrameTime )
        {
            eCurrentEffect = ( eCurrentEffect == ROTATION_EFFECT ) ? SCALE_EFFECT : ROTATION_EFFECT;
        }

        switch( eCurrentEffect )
        {
            case ROTATION_EFFECT:

                angle = 180.0f * nema_absf(nema_sin(ANIMATION_SPEED * g_ui32FrameTime));
                rotate_masking(g_pFrameBufferA, &g_sBoyMortonImage, angle);

                angle = -180.0f * nema_absf(nema_cos(ANIMATION_SPEED * g_ui32FrameTime));
                rotate_masking(g_pFrameBufferB,  &g_sGirlMortonImage, angle);
                break;
            case SCALE_EFFECT:

                ratio = 1.f - 0.4f * nema_absf(nema_sin(ANIMATION_SPEED * g_ui32FrameTime));
                scale_masking(g_pFrameBufferA, &g_sBoyImage, ratio);

                ratio = 1.f - 0.4f * nema_absf(nema_cos(ANIMATION_SPEED * g_ui32FrameTime));
                scale_masking(g_pFrameBufferB,  &g_sGirlImage, ratio);
                break;
            default:
                break;
        }

        //Record the frame time
        ui32LastFrameTime = g_ui32FrameTime;

        //start GPU, submit CL
        nema_cl_submit(&cl);
#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_set(DEBUG_PIN_1);
#endif

#ifdef GPU_WORK_TIME_PRINT
        time_start = xTaskGetTickCount();
#endif

        //Wait GPU
        nema_cl_wait(&cl);

#ifdef GPU_WORK_TIME_PRINT
        time_end = xTaskGetTickCount();

        am_util_stdio_printf("%d\n", time_end - time_start);
#endif

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_clear(DEBUG_PIN_1);
#endif

        //Notify the GUI task.
        xSemaphoreGive(g_semGPUEnd);

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_set(DEBUG_PIN_3);
#endif
    }

}
