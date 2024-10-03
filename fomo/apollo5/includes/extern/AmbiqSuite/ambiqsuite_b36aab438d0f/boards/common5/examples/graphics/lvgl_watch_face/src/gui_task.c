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
#include "lvgl_watch_face.h"
#include "lv_ambiq_font_align.h"
#include "texture/TH_BG_454_bgra8888.h"
#include "texture/TH_HOUR_bgra8888.h"
#include "texture/TH_MINUTE_bgra8888.h"
#include "texture/TH_SECOND_bgra8888.h"
#include "texture/DOT_L_bgra8888.h"
#include "texture/DOT_M_bgra8888.h"
#include "texture/DOT_S_bgra8888.h"
#include "texture/DOT_T_bgra8888.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************


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
lv_img_dsc_t img_watch_bg_psram = {
  .header.always_zero = 0,
  .header.w = 454,
  .header.h = 454,
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
};

lv_img_dsc_t img_hour_psram = {
  .header.always_zero = 0,
  .header.w = 50,
  .header.h = 315,
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
};

lv_img_dsc_t img_minute_psram = {
  .header.always_zero = 0,
  .header.w = 50,
  .header.h = 315,
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
};

lv_img_dsc_t img_second_psram = {
  .header.always_zero = 0,
  .header.w = 50,
  .header.h = 315,
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
};

lv_img_dsc_t img_dot_l_psram = {
  .header.always_zero = 0,
  .header.w = 50,
  .header.h = 50,
  .data_size = 50 * 50 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
};

lv_img_dsc_t img_dot_m_psram = {
  .header.always_zero = 0,
  .header.w = 50,
  .header.h = 50,
  .data_size = 50 * 50 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
};

lv_img_dsc_t img_dot_t_psram = {
  .header.always_zero = 0,
  .header.w = 50,
  .header.h = 50,
  .data_size = 50 * 50 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
};

lv_img_dsc_t img_dot_s_psram = {
  .header.always_zero = 0,
  .header.w = 25,
  .header.h = 28,
  .data_size = 25 * 28 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
};


//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern am_util_stdio_print_char_t g_pfnCharPrint;

//*****************************************************************************
//
// Load texture and font data from MRAM to PSRAM
//
//*****************************************************************************
void
texture_load(void)
{
    img_watch_bg_psram.data_size    = TH_BG_454_bgra8888_len;
    img_hour_psram.data_size        = TH_HOUR_bgra8888_len;
    img_minute_psram.data_size      = TH_MINUTE_bgra8888_len;
    img_second_psram.data_size      = TH_SECOND_bgra8888_len;
    img_dot_l_psram.data_size       = DOT_L_bgra8888_length;
    img_dot_m_psram.data_size       = DOT_M_bgra8888_length;
    img_dot_s_psram.data_size       = DOT_S_bgra8888_length;
    img_dot_t_psram.data_size       = DOT_T_bgra8888_length;

    img_watch_bg_psram.data = (const uint8_t *)lv_mem_ssram_alloc(img_watch_bg_psram.data_size);
    img_hour_psram.data     = (const uint8_t *)lv_mem_ssram_alloc(img_hour_psram.data_size);
    img_minute_psram.data   = (const uint8_t *)lv_mem_ssram_alloc(img_minute_psram.data_size);
    img_second_psram.data   = (const uint8_t *)lv_mem_ssram_alloc(img_second_psram.data_size);
    img_dot_l_psram.data    = (const uint8_t *)lv_mem_ssram_alloc(img_dot_l_psram.data_size);
    img_dot_m_psram.data    = (const uint8_t *)lv_mem_ssram_alloc(img_dot_m_psram.data_size);
    img_dot_s_psram.data    = (const uint8_t *)lv_mem_ssram_alloc(img_dot_s_psram.data_size);
    img_dot_t_psram.data    = (const uint8_t *)lv_mem_ssram_alloc(img_dot_t_psram.data_size);

    nema_memcpy((void*)img_watch_bg_psram.data,  TH_BG_454_bgra8888, TH_BG_454_bgra8888_len);
    nema_memcpy((void*)img_hour_psram.data,      TH_HOUR_bgra8888,   TH_HOUR_bgra8888_len);
    nema_memcpy((void*)img_minute_psram.data,    TH_MINUTE_bgra8888, TH_MINUTE_bgra8888_len);
    nema_memcpy((void*)img_second_psram.data,    TH_SECOND_bgra8888, TH_SECOND_bgra8888_len);
    nema_memcpy((void*)img_dot_l_psram.data,    DOT_L_bgra8888, DOT_L_bgra8888_length);
    nema_memcpy((void*)img_dot_m_psram.data,    DOT_M_bgra8888, DOT_M_bgra8888_length);
    nema_memcpy((void*)img_dot_s_psram.data,    DOT_S_bgra8888, DOT_S_bgra8888_length);
    nema_memcpy((void*)img_dot_t_psram.data,    DOT_T_bgra8888, DOT_T_bgra8888_length);
}

//*****************************************************************************
//
// GUI functions
//
//*****************************************************************************


//*****************************************************************************
//
// Task function.
//
//*****************************************************************************
void
GuiTask(void *pvParameters)
{
    am_util_stdio_printf("GUI task start!\n");

#if LV_USE_LOG == 1
    lv_log_register_print_cb((lv_log_print_g_cb_t)g_pfnCharPrint);
#endif

    // Load texture.
    texture_load();

    /*Now create the actual image*/
    lv_obj_t *watch_bg = lv_img_create(lv_scr_act());
    lv_img_set_src(watch_bg, &img_watch_bg_psram);
    lv_obj_align(watch_bg, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *lv_hour = lv_img_create(watch_bg);
    lv_img_set_src(lv_hour, &img_hour_psram);
    lv_obj_align(lv_hour, LV_ALIGN_TOP_LEFT, 202, 0);
    lv_img_set_pivot(lv_hour, 25, 224);

    lv_obj_t *lv_minute = lv_img_create(watch_bg);
    lv_img_set_src(lv_minute, &img_minute_psram);
    lv_obj_align(lv_minute, LV_ALIGN_TOP_LEFT, 202, 0);
    lv_img_set_pivot(lv_minute, 25, 224);

    lv_obj_t *lv_second = lv_img_create(watch_bg);
    lv_img_set_src(lv_second, &img_second_psram);
    lv_obj_align(lv_second, LV_ALIGN_TOP_LEFT, 202, 0);
    lv_img_set_pivot(lv_second, 25, 224);

    lv_obj_t * lv_dot_l = lv_img_create(watch_bg);
    lv_img_set_src(lv_dot_l, &img_dot_l_psram);
    lv_obj_align(lv_dot_l, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * lv_dot_m = lv_img_create(watch_bg);
    lv_img_set_src(lv_dot_m, &img_dot_m_psram);
    lv_obj_align(lv_dot_m, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * lv_dot_s = lv_img_create(watch_bg);
    lv_img_set_src(lv_dot_s, &img_dot_s_psram);
    lv_obj_align(lv_dot_s, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * lv_dot_t = lv_img_create(watch_bg);
    lv_img_set_src(lv_dot_t, &img_dot_t_psram);
    lv_obj_align(lv_dot_t, LV_ALIGN_CENTER, 0, 0);

    uint32_t last_sec = 0;
    uint32_t current_sec = 0;
    bool watch_update = false;
    uint32_t needle_hour = 0;
    uint32_t needle_minute = 0;
    uint32_t needle_second = 0;
    uint32_t angle;

    while (1)
    {
        current_sec = (uint32_t) nema_get_time();

        if (current_sec != last_sec)
        {
            last_sec = current_sec;
            watch_update = true;

            if (++needle_second > 59)
            {
                needle_second = 0;
                needle_minute++;

                if (needle_minute > 59)
                {
                    needle_minute = 0;
                    needle_hour++;
                    if (needle_hour > 11)
                    {
                        needle_hour = 0;
                    }
                }
            }
        }

        if (watch_update == true)
        {
            angle = needle_hour * 300 + needle_minute * 5;
            lv_img_set_angle(lv_hour, angle);

            angle = needle_minute * 60 + needle_second;
            lv_img_set_angle(lv_minute, angle);

            angle = needle_second * 60;
            lv_img_set_angle(lv_second, angle);

            watch_update = false;
        }

        lv_timer_handler();

        vTaskDelay(100);
    }
}

