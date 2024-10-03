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
#include "lvgl_watch_demo.h"
#include "lv_ambiq_font_align.h"
//*****************************************************************************
//
// GUI task handle.
//
//*****************************************************************************
TaskHandle_t GuiTaskHandle;

SemaphoreHandle_t g_semBLEAdv = NULL;
SemaphoreHandle_t g_semBLECon = NULL;

//*****************************************************************************
//
// Texture.
//
//*****************************************************************************
lv_img_dsc_t img_watch_bg_psram = {
  .header.always_zero = 0,
  .header.w = 454,
  .header.h = 454,
  .data_size = 454 * 454 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_heart_rate0_psram = {
  .header.always_zero = 0,
  .header.w = 142,
  .header.h = 142,
  .data_size = 142 * 142 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_friday_psram = {
  .header.always_zero = 0,
  .header.w = 42,
  .header.h = 20,
  .data_size = 42 * 20 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_ble_psram = {
  .header.always_zero = 0,
  .header.w = 40,
  .header.h = 40,
  .data_size = 40 * 40 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_hour_psram = {
  .header.always_zero = 0,
  .header.w = 50,
  .header.h = 315,
  .data_size = 50 * 315 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_minute_psram = {
  .header.always_zero = 0,
  .header.w = 50,
  .header.h = 315,
  .data_size = 50 * 315 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_second_psram = {
  .header.always_zero = 0,
  .header.w = 50,
  .header.h = 315,
  .data_size = 50 * 315 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_dot_l_psram = {
  .header.always_zero = 0,
  .header.w = 50,
  .header.h = 50,
  .data_size = 50 * 50 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_dot_m_psram = {
  .header.always_zero = 0,
  .header.w = 50,
  .header.h = 50,
  .data_size = 50 * 50 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_dot_t_psram = {
  .header.always_zero = 0,
  .header.w = 50,
  .header.h = 50,
  .data_size = 50 * 50 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_dot_s_psram = {
  .header.always_zero = 0,
  .header.w = 25,
  .header.h = 28,
  .data_size = 25 * 28 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_watch_bg1_psram = {
  .header.always_zero = 0,
  .header.w = 454,
  .header.h = 454,
  .data_size = 454 * 454 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_hour1_psram = {
  .header.always_zero = 0,
  .header.w = 50,
  .header.h = 315,
  .data_size = 50 * 315 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_minute1_psram = {
  .header.always_zero = 0,
  .header.w = 50,
  .header.h = 315,
  .data_size = 50 * 315 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_second1_psram = {
  .header.always_zero = 0,
  .header.w = 50,
  .header.h = 315,
  .data_size = 50 * 315 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_arc_psram = {
  .header.always_zero = 0,
  .header.w = 454,
  .header.h = 454,
  .data_size = 454 * 454 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_weather_psram = {
  .header.always_zero = 0,
  .header.w = 454,
  .header.h = 454,
  .data_size = 454 * 454 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_music_psram = {
  .header.always_zero = 0,
  .header.w = 454,
  .header.h = 454,
  .data_size = 454 * 454 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_wallpaper_psram = {
  .header.always_zero = 0,
  .header.w = 456,
  .header.h = 456,
  .data_size = 456 * 456 * 6 / 8,
  .header.cf = LV_IMG_CF_AMBIQ_TSC6,
};

lv_img_dsc_t img_watch0_psram = {
  .header.always_zero = 0,
  .header.w = 250,
  .header.h = 250,
  .data_size = 250 * 250 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_watch1_psram = {
  .header.always_zero = 0,
  .header.w = 250,
  .header.h = 250,
  .data_size = 250 * 250 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_watch2_psram = {
  .header.always_zero = 0,
  .header.w = 250,
  .header.h = 250,
  .data_size = 250 * 250 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_watch3_psram = {
  .header.always_zero = 0,
  .header.w = 250,
  .header.h = 250,
  .data_size = 250 * 250 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_watch4_psram = {
  .header.always_zero = 0,
  .header.w = 250,
  .header.h = 250,
  .data_size = 250 * 250 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_hour_ssram = {
  .header.always_zero = 0,
  .header.w = 50,
  .header.h = 315,
  .data_size = 50 * 315 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_minute_ssram = {
  .header.always_zero = 0,
  .header.w = 50,
  .header.h = 315,
  .data_size = 50 * 315 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_second_ssram = {
  .header.always_zero = 0,
  .header.w = 50,
  .header.h = 315,
  .data_size = 50 * 315 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
};

lv_img_dsc_t img_world_map_psram = {
  .header.cf = LV_IMG_CF_AMBIQ_TSVG,
  .header.w = 454,
  .header.h = 454,
  .header.always_zero = 0,
  .header.reserved = 0,
  .data_size = 40356,
};


lv_font_t * ambiq_font_test;
//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

static lv_indev_drv_t indev_drv;


//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern am_util_stdio_print_char_t g_pfnCharPrint;

//*****************************************************************************
//
// Task function.
//
//*****************************************************************************

static img_obj_t g_sHourHand =
{
    {0},  50, 315, -1, 1, NEMA_ARGB8888, NEMA_FILTER_BL,
};

static img_obj_t g_sMinHand =
{
    {0},  50, 315, -1, 1, NEMA_ARGB8888, NEMA_FILTER_BL,
};

static img_obj_t g_sSecHand =
{
    {0},  50, 315, -1, 1, NEMA_ARGB8888, NEMA_FILTER_BL,
};
//*****************************************************************************
//
// Allocate buffer in SSRAM using tsi_malloc
//
//*****************************************************************************
static int
buffer_create_in_ssram(img_obj_t* img)
{
    uint32_t size;

    size = nema_texture_size(img->format, 0, img->w, img->h);

    img->bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, size);

    if ( img->bo.base_virt == NULL )
    {
      am_util_stdio_printf("buffer Malloc failed!\n");
      return -1;
    }

    memset(img->bo.base_virt, 0, size);

    return 0;
}

int
texture_ssram_init(void)
{
    int ret;

    ret = buffer_create_in_ssram(&g_sHourHand);
    if ( ret < 0 )
    {
       return ret;
    }

    ret = buffer_create_in_ssram(&g_sMinHand);
    if ( ret < 0 )
    {
       return ret;
    }

    ret = buffer_create_in_ssram(&g_sSecHand);
    if ( ret < 0 )
    {
       return ret;
    }

    return 0;
}
//*****************************************************************************
//
// Load texture and font data from MRAM to PSRAM
//
//*****************************************************************************
void
texture_load(void)
{
    texture_ssram_init();

    uint32_t font_length = lv_ambiq_bitmap_length_get(lv_font_montserrat_28.dsc);
    void* font_buffer = lv_mem_external_alloc(font_length);

    // Copy and align the font bitmap
    lv_font_fmt_txt_dsc_t* font = (lv_font_fmt_txt_dsc_t*)lv_font_montserrat_28.dsc;
    lv_ambiq_A4_font_align(font, font_buffer);

    // Flush the Cache area with may contain the font data
    am_hal_cachectrl_range_t range;
    range.ui32StartAddr = (uint32_t)font_buffer;
    range.ui32Size = font_length;
    am_hal_cachectrl_dcache_clean(&range);    

    font_length = lv_ambiq_bitmap_length_get(lv_font_montserrat_14.dsc);
    font_buffer = lv_mem_external_alloc(font_length);

    // Copy and align the font bitmap
    font = (lv_font_fmt_txt_dsc_t*)lv_font_montserrat_14.dsc;
    lv_ambiq_A4_font_align(font, font_buffer);

    range.ui32StartAddr = (uint32_t)font_buffer;
    range.ui32Size = font_length;
    am_hal_cachectrl_dcache_clean(&range);    
}

//*****************************************************************************
//
// Read files from EMMC
//
//*****************************************************************************
lv_fs_res_t
load_emmc_file(char *str, uint8_t *buf, uint32_t len)
{
    lv_fs_file_t f;
    lv_fs_res_t res;
    uint32_t read_cnt = 0;

    res = lv_fs_open(&f, str, LV_FS_MODE_RD);
    if ( res == LV_FS_RES_OK )
    {
         res = lv_fs_read(&f, buf, len, &read_cnt);
         if ( (res != LV_FS_RES_OK) || (read_cnt == 0) )
         {
           am_util_stdio_printf("File Read Error!\n");
           return res;
         }

         res = lv_fs_close(&f);
         if ( res != LV_FS_RES_OK )
         {
           am_util_stdio_printf("Fail to close file\n");
         }
    }
    else
    {
        am_util_stdio_printf("Fail to open file!\n");
    }
    return res;
}

//*****************************************************************************
//
// LVGL Objects
//
//*****************************************************************************
lv_obj_t * tv;
lv_obj_t * tile0;
lv_obj_t * tile1;
lv_obj_t * tile2;
lv_obj_t * tile3;
lv_obj_t * tile4;
lv_obj_t * tile5;

lv_obj_t * lv_hour;
lv_obj_t * lv_minute;
lv_obj_t * lv_second;
lv_obj_t * watch_bg = NULL;

lv_obj_t * lv_hour1;
lv_obj_t * lv_minute1;
lv_obj_t * lv_second1;
lv_obj_t * watch_bg1 = NULL;

lv_obj_t * tv_face;
lv_obj_t * tile_face0;
lv_obj_t * tile_face1;
lv_obj_t * tile_face2;
lv_obj_t * tile_face3;
lv_obj_t * tile_face4;

lv_obj_t * dynamic_wallpaper;
lv_obj_t * lv_hour2;
lv_obj_t * lv_minute2;
lv_obj_t * lv_second2;

lv_obj_t * map;
lv_anim_t map_zoom;
lv_anim_t map_rotate;

static uint32_t watchface_num = 0;
static lv_style_t style_btn;

//*****************************************************************************
//
// LVGL application functions
//
//*****************************************************************************
static void style_init(void);
static void face0_btn_event_cb(lv_event_t * e);
static void face4_btn_event_cb(lv_event_t * e);
static void btn0_event_cb(lv_event_t * e);
static void btn4_event_cb(lv_event_t * e);
static void watch_face_tileview();
static void watchface0_init(void);
static void watchface1_init(void);


static void style_init(void)
{
    /*Create a simple button style*/
    lv_style_init(&style_btn);

    lv_style_set_bg_opa(&style_btn, LV_OPA_TRANSP);

}

static void face0_btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if ( code == LV_EVENT_CLICKED )
    {
        lv_obj_del(tv_face);
        watchface_num = 0;
        if ( watch_bg != NULL )
        {
            lv_obj_clear_flag(watch_bg, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void face4_btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if ( code == LV_EVENT_CLICKED )
    {
        lv_obj_del(tv_face);
        watchface_num = 4;
        if ( watch_bg1 != NULL )
        {
            lv_obj_clear_flag(watch_bg1, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void btn0_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if ( code == LV_EVENT_LONG_PRESSED )
    {
        lv_obj_add_flag(watch_bg, LV_OBJ_FLAG_HIDDEN);
        watch_face_tileview();
    }
}

static void btn4_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if ( code == LV_EVENT_LONG_PRESSED )
    {
        lv_obj_add_flag(watch_bg1, LV_OBJ_FLAG_HIDDEN);
        watch_face_tileview();
    }
}

// static void zoom_in_event_cb(lv_event_t * e)
// {
//     lv_event_code_t code = lv_event_get_code(e);

//     if ( code == LV_EVENT_CLICKED )
//     {
//         uint16_t zoom_ratio = lv_img_get_zoom(map);
//         if ( zoom_ratio < (256*4) )
//         {
//             lv_img_set_zoom(map, zoom_ratio + 4);
//         }
//     }
// }

// static void zoom_out_event_cb(lv_event_t * e)
// {
//     lv_event_code_t code = lv_event_get_code(e);

//     if ( code == LV_EVENT_CLICKED )
//     {
//         uint16_t zoom_ratio = lv_img_get_zoom(map);
//         if ( zoom_ratio > (256/2) )
//         {
//             lv_img_set_zoom(map, zoom_ratio - 4);
//         }
//     }
// }

// static void set_angle(void * img, int32_t v)
// {
//     lv_img_set_angle(img, v);
// }

// static void set_zoom(void * img, int32_t v)
// {
//     lv_img_set_zoom(img, v);
// }

// static void restart_map_animation(lv_event_t * e)
// {
    // lv_obj_t * current_tile = lv_tileview_get_tile_act(tv);
    // if (current_tile == tile5)
    // {
    //     lv_anim_start(&map_zoom);
    //     lv_anim_start(&map_rotate);
    // }
    // else
    // {
    //     lv_anim_stop(&map_zoom);
    //     lv_anim_stop(&map_rotate);
    // }
// }

static void world_map_init(void)
{
    map = lv_img_create(tile5);
    lv_img_set_src(map, &img_world_map_psram);
    lv_obj_align(map, LV_ALIGN_CENTER, 0, 0);
    lv_img_set_pivot(map, img_world_map_psram.header.w / 2, img_world_map_psram.header.h / 2);    /*Rotate around the top left corner*/

    // lv_obj_add_event_cb(tile5, restart_map_animation, LV_EVENT_VALUE_CHANGED, NULL);

    // lv_anim_init(&map_rotate);
    // lv_anim_set_var(&map_rotate, map);
    // lv_anim_set_exec_cb(&map_rotate, set_angle);
    // lv_anim_set_values(&map_rotate, 0, 150);
    // lv_anim_set_time(&map_rotate, 80);
    // lv_anim_set_repeat_count(&map_rotate, LV_ANIM_REPEAT_INFINITE);
    // lv_anim_set_playback_time(&map_rotate, 80);

    // lv_anim_init(&map_zoom);
    // lv_anim_set_var(&map_zoom, map);
    // lv_anim_set_exec_cb(&map_zoom, set_zoom);
    // lv_anim_set_values(&map_zoom, 256, 400);
    // lv_anim_set_time(&map_zoom, 500);
    // lv_anim_set_playback_time(&map_zoom, 500);
}


static void watch_face_tileview(void)
{
    tv_face = lv_tileview_create(lv_scr_act());
    lv_obj_set_style_bg_color(tv_face, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(tv_face, LV_SCROLLBAR_MODE_OFF);
    tile_face0 = lv_tileview_add_tile(tv_face, 0, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    tile_face1 = lv_tileview_add_tile(tv_face, 1, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    tile_face2 = lv_tileview_add_tile(tv_face, 2, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    tile_face3 = lv_tileview_add_tile(tv_face, 3, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    tile_face4 = lv_tileview_add_tile(tv_face, 4, 0, LV_DIR_LEFT | LV_DIR_RIGHT);

    lv_obj_t * watchface0 = lv_img_create(tile_face0);
    lv_img_set_src(watchface0, &img_watch0_psram);
    lv_obj_align(watchface0, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * watchface1 = lv_img_create(tile_face1);
    lv_img_set_src(watchface1, &img_watch1_psram);
    lv_obj_align(watchface1, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * watchface2 = lv_img_create(tile_face2);
    lv_img_set_src(watchface2, &img_watch2_psram);
    lv_obj_align(watchface2, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * watchface3 = lv_img_create(tile_face3);
    lv_img_set_src(watchface3, &img_watch3_psram);
    lv_obj_align(watchface3, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * watchface4 = lv_img_create(tile_face4);
    lv_img_set_src(watchface4, &img_watch4_psram);
    lv_obj_align(watchface4, LV_ALIGN_CENTER, 0, 0);

    /*Initialize the style*/
    style_init();

    lv_obj_t * btn = lv_btn_create(tile_face0);
    lv_obj_remove_style_all(btn);
    lv_obj_set_pos(btn, 102, 102);
    lv_obj_set_size(btn, 250, 250);
    lv_obj_add_event_cb(btn, face0_btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_style(btn, &style_btn, 0);

    lv_obj_t * btn4 = lv_btn_create(tile_face4);
    lv_obj_remove_style_all(btn4);
    lv_obj_set_pos(btn4, 102, 102);
    lv_obj_set_size(btn4, 250, 250);
    lv_obj_add_event_cb(btn4, face4_btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_style(btn4, &style_btn, 0);

}

#if defined(apollo4l_blue_eb)
lv_obj_t * lv_ble = NULL;
lv_anim_t ble_anim;
lv_anim_timeline_t * ble_anim_timeline = NULL;
static void ble_set_opa_anim(void * img, int32_t v)
{
    lv_obj_set_style_opa(lv_ble, v, 0);
}
#endif
static void watchface0_init(void)
{
    if ( watch_bg == NULL )
    {
        /*Now create the actual image*/
        watch_bg = lv_img_create(tile0);
        lv_img_set_src(watch_bg, &img_watch_bg_psram);
        lv_obj_align(watch_bg, LV_ALIGN_TOP_LEFT, 0, 0);

        lv_obj_t * heart_rate = lv_img_create(watch_bg);
        lv_img_set_src(heart_rate, &img_heart_rate0_psram);
        lv_obj_align(heart_rate, LV_ALIGN_TOP_LEFT, 156, 256);

        lv_obj_t * lv_week = lv_img_create(watch_bg);
        lv_img_set_src(lv_week, &img_friday_psram);
        lv_obj_align(lv_week, LV_ALIGN_TOP_LEFT, 191, 128);

#if ENABLE_LABEL

        lv_obj_t * label_date = lv_label_create(watch_bg);
        lv_label_set_text(label_date, "6");
        lv_obj_set_style_text_font(label_date, &lv_font_montserrat_28, 0);
        lv_obj_set_style_text_color(label_date, lv_color_white(), 0);
        lv_obj_align(label_date, LV_ALIGN_TOP_LEFT, 237, 124);

        lv_obj_t * label_step = lv_label_create(watch_bg);
        lv_label_set_text(label_step, "1234");
        lv_obj_set_style_text_font(label_step, &lv_font_montserrat_28, 0);
        lv_obj_set_style_text_color(label_step, lv_color_white(), 0);
        lv_obj_align(label_step, LV_ALIGN_TOP_LEFT, 87, 226);

        lv_obj_t * label_bat = lv_label_create(watch_bg);
        lv_label_set_text(label_bat, "56");
        lv_obj_set_style_text_font(label_bat, &lv_font_montserrat_28, 0);
        lv_obj_set_style_text_color(label_bat, lv_color_white(), 0);
        lv_obj_align(label_bat, LV_ALIGN_TOP_LEFT, 330, 226);

        lv_obj_t * label_heart = lv_label_create(watch_bg);
        lv_label_set_text(label_heart, "80");
        lv_obj_set_style_text_font(label_heart, &lv_font_montserrat_28, 0);
        lv_obj_set_style_text_color(label_heart, lv_color_white(), 0);
        lv_obj_align(label_heart, LV_ALIGN_TOP_LEFT, 210, 326);
#endif

        lv_hour = lv_img_create(watch_bg);
        lv_img_set_src(lv_hour, &img_hour_psram);
        lv_obj_align(lv_hour, LV_ALIGN_TOP_LEFT, 202, 0);
        lv_img_set_pivot(lv_hour, 25, 224);

        lv_minute = lv_img_create(watch_bg);
        lv_img_set_src(lv_minute, &img_minute_psram);
        lv_obj_align(lv_minute, LV_ALIGN_TOP_LEFT, 202, 0);
        lv_img_set_pivot(lv_minute, 25, 224);

        lv_second = lv_img_create(watch_bg);
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

        /*Initialize the style*/
        style_init();

        lv_obj_t * btn = lv_btn_create(watch_bg);
        lv_obj_remove_style_all(btn);
        lv_obj_set_pos(btn, 0, 0);
        lv_obj_set_size(btn, 454, 454);
        lv_obj_add_event_cb(btn, btn0_event_cb, LV_EVENT_ALL, NULL);
        lv_obj_add_style(btn, &style_btn, 0);
    }

}

static void watchface1_init(void)
{
   if ( watch_bg1 == NULL )
   {
        /*Now create the actual image*/
        watch_bg1 = lv_img_create(tile0);
        lv_img_set_src(watch_bg1, &img_watch_bg1_psram);
        lv_obj_align(watch_bg1, LV_ALIGN_CENTER, 0, 0);

        lv_hour1 = lv_img_create(watch_bg1);
        lv_img_set_src(lv_hour1, &img_hour1_psram);
        lv_obj_align(lv_hour1, LV_ALIGN_TOP_LEFT, 202, 0);
        lv_img_set_pivot(lv_hour1, 25, 224);

        lv_minute1 = lv_img_create(watch_bg1);
        lv_img_set_src(lv_minute1, &img_minute1_psram);
        lv_obj_align(lv_minute1, LV_ALIGN_TOP_LEFT, 202, 0);
        lv_img_set_pivot(lv_minute1, 25, 224);

        lv_second1 = lv_img_create(watch_bg1);
        lv_img_set_src(lv_second1, &img_second1_psram);
        lv_obj_align(lv_second1, LV_ALIGN_TOP_LEFT, 202, 0);
        lv_img_set_pivot(lv_second1, 25, 224);

        /*Initialize the style*/
        style_init();

        lv_obj_t * btn = lv_btn_create(watch_bg1);
        lv_obj_remove_style_all(btn);
        lv_obj_set_pos(btn, 0, 0);
        lv_obj_set_size(btn, 454, 454);
        lv_obj_add_event_cb(btn, btn4_event_cb, LV_EVENT_ALL, NULL);
        lv_obj_add_style(btn, &style_btn, 0);

   }
}

#ifdef USE_DYNAMIC_WALLPAPER
static void dynamic_watchface_init(void)
{
    dynamic_wallpaper = lv_img_create(tile1);
    lv_img_set_src(dynamic_wallpaper, &img_wallpaper_psram);
    lv_obj_align(dynamic_wallpaper, LV_ALIGN_CENTER, 0, 0);

    img_hour_ssram.data   = g_sHourHand.bo.base_virt;
    img_minute_ssram.data = g_sMinHand.bo.base_virt;
    img_second_ssram.data = g_sSecHand.bo.base_virt;

    lv_hour2 = lv_img_create(dynamic_wallpaper);
    lv_img_set_src(lv_hour2, &img_hour_ssram);
    lv_obj_align(lv_hour2, LV_ALIGN_TOP_LEFT, 202, 0);
    lv_img_set_pivot(lv_hour2, 25, 224);
    lv_img_set_angle(lv_hour2, 2700);


    lv_minute2 = lv_img_create(dynamic_wallpaper);
    lv_img_set_src(lv_minute2, &img_minute_ssram);
    lv_obj_align(lv_minute2, LV_ALIGN_TOP_LEFT, 202, 0);
    lv_img_set_pivot(lv_minute2, 25, 224);
    lv_img_set_angle(lv_minute2, 100);


    lv_second2 = lv_img_create(dynamic_wallpaper);
    lv_img_set_src(lv_second2, &img_second_ssram);
    lv_obj_align(lv_second2, LV_ALIGN_TOP_LEFT, 202, 0);
    lv_img_set_pivot(lv_second2, 25, 224);
    lv_img_set_angle(lv_second2, 1500);
}
#endif

static void load_texture_to_psram(void)
{
    img_watch_bg_psram.data = (const uint8_t *)lv_mem_external_alloc(img_watch_bg_psram.data_size);
    img_heart_rate0_psram.data = (const uint8_t *)lv_mem_external_alloc(img_heart_rate0_psram.data_size);
    img_friday_psram.data = (const uint8_t *)lv_mem_external_alloc(img_friday_psram.data_size);
    img_ble_psram.data = (const uint8_t *)lv_mem_external_alloc(img_ble_psram.data_size);
    img_hour_psram.data = (const uint8_t *)lv_mem_external_alloc(img_hour_psram.data_size);
    img_minute_psram.data = (const uint8_t *)lv_mem_external_alloc(img_minute_psram.data_size);
    img_second_psram.data = (const uint8_t *)lv_mem_external_alloc(img_second_psram.data_size);
    img_dot_l_psram.data = (const uint8_t *)lv_mem_external_alloc(img_dot_l_psram.data_size);
    img_dot_m_psram.data = (const uint8_t *)lv_mem_external_alloc(img_dot_m_psram.data_size);
    img_dot_t_psram.data = (const uint8_t *)lv_mem_external_alloc(img_dot_t_psram.data_size);
    img_dot_s_psram.data = (const uint8_t *)lv_mem_external_alloc(img_dot_s_psram.data_size);

    img_watch_bg1_psram.data = (const uint8_t *)lv_mem_external_alloc(img_watch_bg1_psram.data_size);
    img_hour1_psram.data = (const uint8_t *)lv_mem_external_alloc(img_hour1_psram.data_size);
    img_minute1_psram.data = (const uint8_t *)lv_mem_external_alloc(img_minute1_psram.data_size);
    img_second1_psram.data = (const uint8_t *)lv_mem_external_alloc(img_second1_psram.data_size);

    img_arc_psram.data = (const uint8_t *)lv_mem_external_alloc(img_arc_psram.data_size);
    img_weather_psram.data = (const uint8_t *)lv_mem_external_alloc(img_weather_psram.data_size);
    img_music_psram.data = (const uint8_t *)lv_mem_external_alloc(img_music_psram.data_size);
    img_wallpaper_psram.data = (const uint8_t *)lv_mem_external_alloc(img_wallpaper_psram.data_size);

    img_watch0_psram.data = (const uint8_t *)lv_mem_external_alloc(img_watch0_psram.data_size);
    img_watch1_psram.data = (const uint8_t *)lv_mem_external_alloc(img_watch1_psram.data_size);
    img_watch2_psram.data = (const uint8_t *)lv_mem_external_alloc(img_watch2_psram.data_size);
    img_watch3_psram.data = (const uint8_t *)lv_mem_external_alloc(img_watch3_psram.data_size);
    img_watch4_psram.data = (const uint8_t *)lv_mem_external_alloc(img_watch4_psram.data_size);
    // Load texture from EMMC to PSRAM
    load_emmc_file("E:watch_bg_454.argb8888", (void*)img_watch_bg_psram.data, img_watch_bg_psram.data_size);
    load_emmc_file("E:heart_rate0.argb8888", (void*)img_heart_rate0_psram.data, img_heart_rate0_psram.data_size);
    load_emmc_file("E:FRI.argb8888", (void*)img_friday_psram.data,  img_friday_psram.data_size);
//    load_emmc_file("E:ble_logo.argb8888", (void*)img_ble_psram.data,  img_ble_psram.data_size);
    load_emmc_file("E:hour.argb8888", (void*)img_hour_psram.data,  img_hour_psram.data_size);
    load_emmc_file("E:minute.argb8888", (void*)img_minute_psram.data,  img_minute_psram.data_size);
    load_emmc_file("E:second.argb8888", (void*)img_second_psram.data,  img_second_psram.data_size);
    load_emmc_file("E:DOT_L.argb8888", (void*)img_dot_l_psram.data, img_dot_l_psram.data_size);
    load_emmc_file("E:DOT_M.argb8888", (void*)img_dot_m_psram.data,  img_dot_m_psram.data_size);
    load_emmc_file("E:DOT_T.argb8888", (void*)img_dot_t_psram.data, img_dot_t_psram.data_size);
    load_emmc_file("E:DOT_S.argb8888", (void*)img_dot_s_psram.data,  img_dot_s_psram.data_size);

    load_emmc_file("E:TH_BG_454.argb8888", (void*)img_watch_bg1_psram.data, img_watch_bg1_psram.data_size);
    load_emmc_file("E:TH_HOUR.argb8888", (void*)img_hour1_psram.data,  img_hour1_psram.data_size);
    load_emmc_file("E:TH_MINUTE.argb8888", (void*)img_minute1_psram.data,  img_minute1_psram.data_size);
    load_emmc_file("E:TH_SECOND.argb8888", (void*)img_second1_psram.data,  img_second1_psram.data_size);

    load_emmc_file("E:arc_454.argb8888", (void*)img_arc_psram.data, img_arc_psram.data_size);
    load_emmc_file("E:weather_454.argb8888", (void*)img_weather_psram.data, img_weather_psram.data_size);
    load_emmc_file("E:music_454.argb8888", (void*)img_music_psram.data, img_music_psram.data_size);
#ifdef USE_DYNAMIC_WALLPAPER
    load_emmc_file("E:merged_tsc6_file.bin", (void*)img_wallpaper_psram.data, img_wallpaper_psram.data_size);
#endif

    load_emmc_file("E:watch_250_0.argb8888", (void*)img_watch0_psram.data, img_watch0_psram.data_size);
    load_emmc_file("E:watch_250_1.argb8888", (void*)img_watch1_psram.data, img_watch1_psram.data_size);
    load_emmc_file("E:watch_250_2.argb8888", (void*)img_watch2_psram.data, img_watch2_psram.data_size);
    load_emmc_file("E:watch_250_3.argb8888", (void*)img_watch3_psram.data, img_watch3_psram.data_size);
    load_emmc_file("E:watch_250_4.argb8888", (void*)img_watch4_psram.data, img_watch4_psram.data_size);

    // Load font file from EMMC
    ambiq_font_test = lv_font_load("E:ambiq_font_test.bin");

    img_world_map_psram.data = (const uint8_t *)lv_mem_external_alloc(img_world_map_psram.data_size);
    load_emmc_file("E:usa_2.tsvg", (void*)img_world_map_psram.data, img_world_map_psram.data_size);
    //nema_memcpy((void*)img_world_map_psram.data, tiger_tsvg, img_world_map_psram.data_size);
}

static void load_texture_to_ssram(void)
{
    nema_memcpy(g_sHourHand.bo.base_virt, (void*)img_hour1_psram.data, img_hour1_psram.data_size);
    nema_memcpy(g_sMinHand.bo.base_virt, (void*)img_minute1_psram.data, img_minute1_psram.data_size);
    nema_memcpy(g_sSecHand.bo.base_virt, (void*)img_second1_psram.data, img_second1_psram.data_size);

}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
GuiTask(void *pvParameters)
{
    uint16_t needle_hour = 0;
    uint16_t needle_minute = 0;
    uint16_t needle_second = 0;
    uint32_t current_sec = 0;
    uint32_t last_sec = 0;
    uint16_t angle;
#ifdef USE_DYNAMIC_WALLPAPER
    uint32_t i = 0;
#endif

    bool watch_update = false;

    am_util_stdio_printf("GUI task start!\n");

#if LV_USE_LOG == 1
    lv_log_register_print_cb((lv_log_print_g_cb_t)g_pfnCharPrint);
#endif

    // Load texture.
    texture_load();

    // Init touchpad driver
#ifdef LV_AMBIQ_TOUCH_USED
    lv_indev_drv_init(&indev_drv); /*Descriptor of a input device driver*/
    indev_drv.long_press_time = 2000; //set long press time
    indev_drv.type = LV_INDEV_TYPE_POINTER; /*Touch pad is a pointer-like device*/
    indev_drv.read_cb = lv_ambiq_touch_read; /*Set your driver function*/
    lv_indev_drv_register(&indev_drv); /*Finally register the driver*/
#endif

#ifdef LV_AMBIQ_FS_USED
    // Init file system
    lv_ambiq_fs_init();
#endif

    // Load textures from eMMC to PSRAM
    load_texture_to_psram();

    // Load textures to SSRAM
    load_texture_to_ssram();

    // Create tileview for watch demo
    tv = lv_tileview_create(lv_scr_act());
    lv_obj_set_style_bg_color(tv, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(tv, LV_SCROLLBAR_MODE_OFF);
    tile0 = lv_tileview_add_tile(tv, 0, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
#ifdef USE_DYNAMIC_WALLPAPER
    tile1 = lv_tileview_add_tile(tv, 1, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    tile2 = lv_tileview_add_tile(tv, 2, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    tile3 = lv_tileview_add_tile(tv, 3, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    tile4 = lv_tileview_add_tile(tv, 4, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    tile5 = lv_tileview_add_tile(tv, 5, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
#else
    tile2 = lv_tileview_add_tile(tv, 1, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    tile3 = lv_tileview_add_tile(tv, 2, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    tile4 = lv_tileview_add_tile(tv, 3, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    tile5 = lv_tileview_add_tile(tv, 4, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
#endif

    lv_obj_t * arcs = lv_img_create(tile2);
    lv_img_set_src(arcs, &img_arc_psram);
    lv_obj_align(arcs, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * weather = lv_img_create(tile3);
    lv_img_set_src(weather, &img_weather_psram);
    lv_obj_align(weather, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * label_ambiq = lv_label_create(tile3);
    lv_label_set_text(label_ambiq, "Ambiq Micro, Inc.");
    lv_obj_set_style_text_font(label_ambiq, ambiq_font_test, 0);
    lv_obj_set_style_text_color(label_ambiq, lv_color_white(), 0);
    lv_obj_align(label_ambiq, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * music = lv_img_create(tile4);
    lv_img_set_src(music, &img_music_psram);
    lv_obj_align(music, LV_ALIGN_CENTER, 0, 0);

    world_map_init();

#ifdef USE_DYNAMIC_WALLPAPER
    dynamic_watchface_init();
#endif

#ifdef USE_DYNAMIC_WALLPAPER
    lv_fs_file_t f;
    lv_fs_res_t res;
    res = lv_fs_open(&f, "E:merged_tsc6_file.bin", LV_FS_MODE_RD);
    if ( res != LV_FS_RES_OK )
    {
        am_util_stdio_printf("File Read Error!\n");
    }
#endif

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

        lv_obj_t * current_tile = lv_tileview_get_tile_act(tv);

#ifdef USE_DYNAMIC_WALLPAPER
        uint32_t read_cnt = 0;
        if ( current_tile == tile1 )
        {
            if ( i < WALLPAPER_FRAME_NUM - 1 )
            {
                i++;
            }
            else
            {
                i = 0;
            }

            res = lv_fs_seek(&f, i * img_wallpaper_psram.data_size, LV_FS_SEEK_SET);
            if ( res != LV_FS_RES_OK )
            {
                am_util_stdio_printf("Fail to move file pointer!\n");
            }

            res = lv_fs_read(&f, (uint8_t *)img_wallpaper_psram.data, img_wallpaper_psram.data_size, &read_cnt);
            if ( (res != LV_FS_RES_OK) || (read_cnt == 0) )
            {
                am_util_stdio_printf("File Read Error!\n");
            }

            lv_img_set_src(dynamic_wallpaper, &img_wallpaper_psram);

            if (watch_update == true)
            {
                angle = needle_hour * 300 + needle_minute * 5;
                lv_img_set_angle(lv_hour2, angle);

                angle = needle_minute * 60 + needle_second;
                lv_img_set_angle(lv_minute2, angle);

                angle = needle_second * 60;
                lv_img_set_angle(lv_second2, angle);

                watch_update = false;
            }

        }
#endif

        if ( current_tile == NULL || current_tile == tile0 )
        {
            if ( watchface_num == 0 )
            {
                watchface0_init();

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
            }
            else
            {
                watchface1_init();

                if (watch_update == true)
                {
                    angle = needle_hour * 300 + needle_minute * 5;
                    lv_img_set_angle(lv_hour1, angle);

                    angle = needle_minute * 60 + needle_second;
                    lv_img_set_angle(lv_minute1, angle);

                    angle = needle_second * 60;
                    lv_img_set_angle(lv_second1, angle);

                    watch_update = false;
                }
            }
        }
        else if ( current_tile == tile5 )
        {
            uint16_t zoom_ratio = lv_img_get_zoom(map);
            if ( zoom_ratio < (256 * 4) )
            {
                lv_img_set_zoom(map, zoom_ratio + 4);
            }
            else
            {
                lv_img_set_zoom(map, 256);
            }
        }

        lv_timer_handler();

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
    am_hal_gpio_output_set(DEBUG_PIN_6);
#endif

        vTaskDelay(5);

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
    am_hal_gpio_output_clear(DEBUG_PIN_6);
#endif
    }
}

