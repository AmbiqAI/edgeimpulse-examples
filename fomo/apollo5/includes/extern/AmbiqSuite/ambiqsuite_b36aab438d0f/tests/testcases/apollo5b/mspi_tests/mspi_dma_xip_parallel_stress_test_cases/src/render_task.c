//*****************************************************************************
//
//! @file render_task.c
//!
//! @brief Task to handle RENDER operations.
//!
//! AM_DEBUG_PRINTF
//! If enabled, debug messages will be sent over ITM.
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
#include "mspi_dma_xip_parallel_stress_test_cases.h"

#include "nema_core.h"
#include "nema_utils.h"
#include "nema_event.h"
#include "nema_hal.h"
#include "nema_vg.h"
#include "nema_vg_tsvg.h"
#include "nema_error.h"

#include "blowing.h"
#include "climate.h"
#include "day.h"

#include "graphics_power_control.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define GPU_POWER_SAVE_ENABLE   (0)
#define GPU_POWER_MODE_SWITCH   (1)

//clock hands location
#define SSEC_PIVOT_X (-10)
#define SSEC_PIVOT_Y (-158)

#define MIN_PIVOT_X  (-14)
#define MIN_PIVOT_Y  (-157)

#define HOUR_PIVOT_X (-13)
#define HOUR_PIVOT_Y (-109)

#define ICON_WIDTH (100)
#define ICON_HEIGHT (100)

#define ICON_POS_X  (145)
#define ICON_POS_Y  (250)

//*****************************************************************************
//
// RENDER task handle.
//
//*****************************************************************************
TaskHandle_t RenderTaskHandle;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
const unsigned char* icon_to_draw = blowing;
const unsigned char* icon_array[3] = {blowing, climate, day};


//*****************************************************************************
//
// Task function.
//
//*****************************************************************************
//*****************************************************************************
//
// Draw needle
//
//*****************************************************************************
static inline void
needle_draw( img_obj_t *img, float x0, float y0, float angle, int cx, int cy )
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

    //draw needle
    nema_bind_src_tex(img->bo.base_phys, img->w, img->h, img->format, img->stride, NEMA_FILTER_BL);
    nema_blit_quad_fit(x0, y0,
                       x1, y1,
                       x2, y2,
                       x3, y3);
}

//*****************************************************************************
//
//! @brief Draw the weather icon.
//!
//! @return None.
//
//*****************************************************************************
static void draw_vg_icon(const uint8_t * icon, int32_t area_pos_x, int32_t area_pos_y,
                        int32_t area_width, int32_t area_hight)
{
    //
    // Set global matrix 
    //
    uint32_t svg_width;
    uint32_t svg_height;
    nema_vg_get_tsvg_resolution(icon, &svg_width, &svg_height);

    float scale_x = area_width  / (float)svg_width;
    float scale_y = area_hight / (float)svg_height;

    nema_matrix3x3_t matrix;
    nema_mat3x3_load_identity(matrix);
    nema_mat3x3_scale(matrix, scale_x, scale_y);
    nema_mat3x3_translate(matrix, area_pos_x, area_pos_y);
    nema_vg_set_global_matrix(matrix);

    nema_vg_set_blend(NEMA_BL_SIMPLE);

    //
    // Draw icon
    //
    nema_vg_draw_tsvg(icon);

    //
    // Reset global matrix
    //
    nema_vg_reset_global_matrix();

}


//*****************************************************************************
//
// Draw watchface
//
//*****************************************************************************
int
watchface_draw(nema_cmdlist_t *cl, float hour, float min, float sec, int parts)
{
    float angle;

    //rewind and bind the CL
    nema_cl_rewind(cl);
    nema_cl_bind_circular(cl);

    //bind the destination buffer
    nema_bind_dst_tex(g_pFrameBufferGPU->bo.base_phys, FB_RESX, FB_RESY, g_pFrameBufferGPU->format, g_pFrameBufferGPU->stride);

    //draw backface
    nema_set_clip(0, 0, FB_RESX, FB_RESY / SMALLFB_STRIPES);

    nema_bind_src_tex(g_pClockDialBufferGPU->bo.base_phys,
                      g_pClockDialBufferGPU->w,
                      g_pClockDialBufferGPU->h,
                      g_pClockDialBufferGPU->format,
                      g_pClockDialBufferGPU->stride,
                      0);

    nema_set_blend_blit(NEMA_BLOP_MODULATE_A | NEMA_BL_SIMPLE);
    nema_blit(0, 0);

    nema_bind_src_tex(g_pSportsDialBufferGPU->bo.base_phys,
                  g_pSportsDialBufferGPU->w,
                  g_pSportsDialBufferGPU->h,
                  g_pSportsDialBufferGPU->format,
                  g_pSportsDialBufferGPU->stride,
                  0);
    nema_blit(0, 0);

    draw_vg_icon(icon_to_draw, ICON_POS_X, ICON_POS_Y - FB_RESY / SMALLFB_STRIPES * parts, ICON_WIDTH, ICON_HEIGHT);

    nema_set_blend_blit(NEMA_BL_SIMPLE);

    //draw needle
    angle =  hour / 12.f * 360.f;
    needle_draw(&g_sHourHand, HOUR_PIVOT_X, HOUR_PIVOT_Y, -angle, FB_RESX / 2, FB_RESY / 2 - FB_RESY / SMALLFB_STRIPES * parts);

    angle =  min / 60.f * 360.f;
    needle_draw(&g_sMinHand, MIN_PIVOT_X, MIN_PIVOT_Y, -angle, FB_RESX / 2, FB_RESY / 2 - FB_RESY / SMALLFB_STRIPES * parts);

    angle = sec / 60.f * 360.f;
    needle_draw(&g_sSecHand, SSEC_PIVOT_X, SSEC_PIVOT_Y, -angle, FB_RESX / 2, FB_RESY / 2 - FB_RESY / SMALLFB_STRIPES * parts);

    return 0;
}

//*****************************************************************************
//gpu_power_mode_select
//! @brief this function used to GPU power mode select
//!
//! @return true- GPU power select success.
//!         false- GPU power select failed.DISPLAY_TASK_ENABLE
//*****************************************************************************
bool
gpu_power_mode_select(am_hal_pwrctrl_gpu_mode_e mode)
{
    bool status;
    am_hal_pwrctrl_gpu_mode_e current_mode;

    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_GFX, &status);
    if ( status )
    {
        //
        //Power down
        //
        am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);
    } 
    am_hal_pwrctrl_gpu_mode_select(mode);
    am_hal_pwrctrl_gpu_mode_status(&current_mode);
    if(mode != current_mode)
    {
        am_util_stdio_printf("gpu mode selection failed!\n");
        vErrorHandler();
        return false;
    }

    return true;
}

//*****************************************************************************
//  change the icon, this is a callback function in FreeRTOS timer.
//*****************************************************************************
void icon_change(TimerHandle_t timer)
{
    static uint32_t count = 0;

    icon_to_draw = icon_array[count];

    count++;
    if(count >= 3)
    {
        count = 0;
    }
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
    float time;
    float sec;
    float min;
    float hour;

#if GPU_POWER_MODE_SWITCH  
    char* mode_string;
    am_hal_pwrctrl_gpu_mode_e eGPUMode = AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE;    
#endif

    am_util_stdio_printf("Render task start!\n");

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

    //Initialize NemaGFX
    nema_init();
    ret = nema_get_error();
    if (ret != NEMA_ERR_NO_ERROR)
    {
        am_util_stdio_printf("NemaGFX init failed!\n");

        //suspend and delete this task.
        vTaskDelete(NULL);
    }

    //Initialize NemaGFX
    nema_vg_init(FB_RESX, FB_RESY);
    ret = nema_vg_get_error();
    if (ret != NEMA_VG_ERR_NO_ERROR)
    {
        am_util_stdio_printf("NemaDC init failed!\n");

        //suspend and delete this task.
        vTaskDelete(NULL);
    }   

    // Create GPU command list
    cl = nema_cl_create_sized(0x1000);

    //
    // Create timer to change icon type
    //
    TimerHandle_t icon_timer = xTimerCreate("IconTimer", 1000, pdTRUE, NULL, icon_change);
    xTimerStart(icon_timer, 0);

    while(1)
    {
        //Wait start.
        xSemaphoreTake( g_semGPUStart, portMAX_DELAY);

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_clear(24);
        am_hal_gpio_output_clear(27);
#endif

#if (GPU_POWER_SAVE_ENABLE || GPU_POWER_MODE_SWITCH)
        //This API will directly return if GPU is still power on.
        gpu_power_up();
#endif

        if ( g_intRenderPart == 0 )
        {
            //Get time from timer
            time = nema_get_time();

#ifdef USE_CRC_CHECK
            (void)time;     //Avoid warning!
            sec  = 25.0;
            min  = 8.0;
            hour = 16.0;
#else
            sec  = time;
            min  = time / 60;
            hour = time / 60 / 60;
#endif
        }

        //draw watchface
        watchface_draw(&cl, hour, min, sec, g_intRenderPart);

        //start GPU, submit CL
        nema_cl_submit(&cl);
#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_set(22);
#endif

        //Wait GPU
        nema_cl_wait(&cl);

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_clear(22);
#endif

        //Notify the GUI task.
        xSemaphoreGive(g_semGPUEnd);

#if GPU_POWER_SAVE_ENABLE
        //Power Down
        gpu_power_down();
#endif

#if GPU_POWER_MODE_SWITCH
        mode_string = (eGPUMode == AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE) ? "GPU_HP" : "GPU_LP";
        if(gpu_power_mode_select(eGPUMode) != true)
        {
            am_util_stdio_printf("[ERROR]: Enter %s mode failed!\n", mode_string);
            vErrorHandler();
        }

        if ( eGPUMode == AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE )
        {
            eGPUMode = AM_HAL_PWRCTRL_GPU_MODE_LOW_POWER;
        }
        else
        {
            eGPUMode = AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE;
        }   
#endif

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_set(24);
#endif
    }

}

