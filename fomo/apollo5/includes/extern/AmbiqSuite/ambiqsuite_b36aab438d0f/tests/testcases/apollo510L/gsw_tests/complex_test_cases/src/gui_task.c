//*****************************************************************************
//
//! @file gui_task.c
//!
//! @brief Task to show an interactive watchface on the display.
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
#include "complex_test_cases.h"
#include "lv_ambiq_touch.h"
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
// Global Variables
//
//*****************************************************************************

static lv_indev_drv_t indev_drv;
lv_indev_t *indev;

lv_obj_t *watch_bg;
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

    memcpy((void*)img_watch_bg_psram.data,  TH_BG_454_bgra8888, TH_BG_454_bgra8888_len);
    memcpy((void*)img_hour_psram.data,      TH_HOUR_bgra8888,   TH_HOUR_bgra8888_len);
    memcpy((void*)img_minute_psram.data,    TH_MINUTE_bgra8888, TH_MINUTE_bgra8888_len);
    memcpy((void*)img_second_psram.data,    TH_SECOND_bgra8888, TH_SECOND_bgra8888_len);
    memcpy((void*)img_dot_l_psram.data,    DOT_L_bgra8888, DOT_L_bgra8888_length);
    memcpy((void*)img_dot_m_psram.data,    DOT_M_bgra8888, DOT_M_bgra8888_length);
    memcpy((void*)img_dot_s_psram.data,    DOT_S_bgra8888, DOT_S_bgra8888_length);
    memcpy((void*)img_dot_t_psram.data,    DOT_T_bgra8888, DOT_T_bgra8888_length);
    uint32_t font_length = lv_ambiq_bitmap_length_get(lv_font_montserrat_28.dsc);
    void* font_buffer = lv_mem_external_alloc(font_length);

    // Copy and align the font bitmap
    lv_font_fmt_txt_dsc_t* font = (lv_font_fmt_txt_dsc_t*)lv_font_montserrat_28.dsc;
    lv_ambiq_A4_font_align(font, font_buffer);

    font_length = lv_ambiq_bitmap_length_get(lv_font_montserrat_14.dsc);
    font_buffer = lv_mem_external_alloc(font_length);

    // Copy and align the font bitmap
    font = (lv_font_fmt_txt_dsc_t*)lv_font_montserrat_14.dsc;
    lv_ambiq_A4_font_align(font, font_buffer);
}

//*****************************************************************************
//
// GUI functions
//
//*****************************************************************************

//*****************************************************************************
//
// Perform initial setup for the GUI task
//
//*****************************************************************************
void
GuiTaskSetup(void)
{
    am_util_stdio_printf("GUI task: setup\r\n");
#if LV_USE_LOG == 1
    lv_log_register_print_cb((lv_log_print_g_cb_t)g_pfnCharPrint);
#endif
    // Load texture.
    texture_load();
}

static void watch_bg_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);

    lv_indev_t * indev = lv_indev_get_act();
    if(indev == NULL) {
        am_util_stdio_printf("Can't get indev handle");
        return;
    }

    lv_point_t vect;
    lv_indev_get_vect(indev, &vect);

    lv_coord_t x = lv_obj_get_x(obj) + vect.x;
    lv_coord_t y = lv_obj_get_y(obj) + vect.y;
    lv_obj_set_pos(obj, x, y);
    am_util_stdio_printf("Drag event %d, %d\n", x, y);
}

static void btn_event_cb(lv_event_t * e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    lv_indev_t *indev = lv_indev_get_act();
    if(indev == NULL) {
        am_util_stdio_printf("Can't get indev handle");
        return;
    }

    lv_point_t point;
    lv_indev_get_point(indev, &point);
    lv_label_set_text_fmt(label, "(%d,%d)", point.x, point.y);
}

//*****************************************************************************
//
// Task function.
//
//*****************************************************************************
void
GuiTask(void *pvParameters)
{
    am_util_stdio_printf("GUI task start!\n");

// Init touchpad driver

    lv_indev_drv_init(&indev_drv); /*Descriptor of a input device driver*/
    indev_drv.long_press_time = 2000; //set long press time
    indev_drv.type = LV_INDEV_TYPE_POINTER; /*Touch pad is a pointer-like device*/
    indev_drv.read_cb = lv_ambiq_touch_read; /*Set your driver function*/
    indev = lv_indev_drv_register(&indev_drv); /*Finally register the driver*/

    /*Now create the actual image*/

    lv_obj_t *tv = lv_tileview_create(lv_scr_act());
    lv_obj_set_style_bg_color(tv, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(tv, LV_SCROLLBAR_MODE_OFF);
    lv_obj_t *tile0 = lv_tileview_add_tile(tv, 0, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    lv_obj_t *tile1 = lv_tileview_add_tile(tv, 1, 0, LV_DIR_LEFT | LV_DIR_RIGHT);

    watch_bg = lv_img_create(tile0);
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

    lv_obj_t *btn = lv_btn_create(tile1);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(btn, 250, 100);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_PRESSING, NULL);


    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "CLICK ME");
    lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(btn_label, lv_color_white(), 0);
    lv_obj_align(btn_label, LV_ALIGN_CENTER, 0, 0);

    uint32_t last_sec = 0;
    uint32_t current_sec = 0;
    bool watch_update = false;
    uint32_t needle_hour = 0;
    uint32_t needle_minute = 0;
    uint32_t needle_second = 0;
    uint32_t angle;
    lv_point_t vect;

    while (1)
    {
        // Update clock hands
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

