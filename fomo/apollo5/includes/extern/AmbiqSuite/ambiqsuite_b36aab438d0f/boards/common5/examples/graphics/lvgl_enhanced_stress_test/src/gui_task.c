//*****************************************************************************
//
//! @file gui_task.c
//!
//! @brief Task to handle GUI operations.
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
#include "lvgl_enhanced_stress_test.h"
#include "lv_ambiq_font_align.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define GPU_POWER_MODE_SWITCH   (1)
#define DRAW_LABEL  0

//*****************************************************************************
//
// GUI task handle.
//
//*****************************************************************************
TaskHandle_t GuiTaskHandle;

//*****************************************************************************
//
// Texture.
//
//*****************************************************************************


//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern am_util_stdio_print_char_t g_pfnCharPrint;

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
    if ( am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX) )
    {
        am_util_stdio_printf("gpu power on failed!\n");
        return false;
    }
    am_hal_pwrctrl_gpu_mode_status(&current_mode);
    if ( mode != current_mode )
    {
        am_util_stdio_printf("gpu mode selection failed!\n");
        return false;
    }

    return true;
}

//*****************************************************************************
//
// Load texture and font data from MRAM to PSRAM
//
//*****************************************************************************
void
texture_load(void)
{
    // Alloc buffer in PSRAM to hold the font data.
    uint32_t font_length = lv_ambiq_bitmap_length_get(lv_font_montserrat_14.dsc);
    void* font_buffer = lv_mem_external_alloc(font_length);

    // Copy and align the font bitmap
    lv_font_fmt_txt_dsc_t* font = (lv_font_fmt_txt_dsc_t*)lv_font_montserrat_14.dsc;
    lv_ambiq_A4_font_align(font, font_buffer);
}

#define MAX_NUM (5)


//*****************************************************************************
//
// Task function.
//
//*****************************************************************************
void
GuiTask(void *pvParameters)
{
    // bool bg_switch = true;
    uint32_t i;
    TickType_t tick_last;
    TickType_t tick_cur;
    TickType_t tick_elps;

#if GPU_POWER_MODE_SWITCH
    char* mode_string;
    am_hal_pwrctrl_gpu_mode_e eGPUMode = AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE;
#endif

#if LV_USE_LOG == 1
    lv_log_register_print_cb((lv_log_print_g_cb_t)g_pfnCharPrint);
#endif

    // Load texture.
    texture_load();

    lv_style_t style_btn;

    /*Create a simple button style*/
    lv_style_init(&style_btn);

    lv_style_set_radius(&style_btn, 30);
    lv_style_set_bg_opa(&style_btn, LV_OPA_60);
    lv_style_set_bg_color(&style_btn, lv_palette_main(LV_PALETTE_BLUE));


    lv_obj_t * btn[MAX_NUM];


    for ( i = 0; i < MAX_NUM; i++ )
    {
        btn[i] = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
        lv_obj_remove_style_all(btn[i]);
        lv_obj_set_pos(btn[i], 0, 0);                            /*Set its position*/
        lv_obj_set_size(btn[i], 120, 100);                          /*Set its size*/

        lv_obj_add_style(btn[i], &style_btn, 0);

        // lv_obj_t * label = lv_label_create(btn[i]);          /*Add a label to the button*/
        // lv_label_set_text(label, "Button");                     /*Set the labels text*/
        // lv_obj_center(label);
    }

    /*Create a label style*/
    lv_obj_t * label1 = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(label1, LV_LABEL_LONG_SCROLL_CIRCULAR);     /*Circular scroll*/
    lv_obj_set_width(label1, 100);
    lv_label_set_text(label1, "lvgl enhanced stress test. ");
    lv_obj_align(label1, LV_ALIGN_TOP_MID, 0, 40);

    am_util_stdio_printf("GUI task start!\n");
    while (1)
    {
        am_util_stdio_printf("[TASK] : GUI\n");
        tick_last = xTaskGetTickCount();

#if GPU_POWER_MODE_SWITCH
        //
        //Power up
        //
        am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
        if ( nema_init() != 0 )
        {
            am_util_stdio_printf("GPU init failed!\n");
        }
#endif
        while(1)
        {
            for ( i = 0; i < MAX_NUM; i++ )
            {
                uint32_t pos_x = rand() % (LV_AMBIQ_FB_RESX - 120);
                uint32_t pos_y = rand() % (LV_AMBIQ_FB_RESY - 100);

                lv_obj_set_pos(btn[i], pos_x, pos_y);
            }

            lv_timer_handler();

            vTaskDelay(5);

            tick_cur = xTaskGetTickCount();
            if ( tick_cur >= tick_last )
            {
                tick_elps = tick_cur - tick_last;
            }
            else
            {
                tick_elps = tick_cur + portMAX_DELAY - tick_last;
            }

            if ( tick_elps > 10000 )
            {
                break;
            }
        }

#if GPU_POWER_MODE_SWITCH
        //
        //Power down
        //
        am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);
        //
        //Switch to next power mode.
        //
        mode_string = (eGPUMode == AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE) ? "GPU_HP" : "GPU_LP";
        if ( gpu_power_mode_select(eGPUMode) != true )
        {
            am_util_stdio_printf("[ERROR]: Enter %s mode failed!\n", mode_string);
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
        vTaskDelay(2000);
    }
}

