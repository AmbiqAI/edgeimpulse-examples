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
#include "lvgl_widget_examples.h"
#include "lv_ambiq_font_align.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
// Animations
#define DRAW_ANIM_IMG   0
#define DRAW_ANIM_PLAYBACK 0

// Events
#define DRAW_EVENT      0

// Layouts
#define DRAW_FLEX       0
#define DRAW_GRID       0

// Scrolling
#define DRAW_SCROLL     0

// Widgets
#define DRAW_ARC        0
#define DRAW_BAR        0
#define DRAW_BUTTON     0
#define DRAW_BTNMATRIX  0
#define DRAW_CALENDAR   0
#define DRAW_CANVAS     0
#define DRAW_CHART      0
#define DRAW_CHECKBOX   0
#define DRAW_CLOCK      0
#define DRAW_COLORWHEEL 0
#define DRAW_DROPDOWN   0
#define DRAW_IMG_RECOLOR 0
#define DRAW_IMG_ROTATE  0
#define DRAW_IMGBTN     0
#define DRAW_KEYBOARD   0
#define DRAW_LABEL      0
#define DRAW_LED        0
#define DRAW_LINE       0
#define DRAW_LIST       0
#define DRAW_METER      0
#define DRAW_MSGBOX     0
#define DRAW_ROLLER     0
#define DRAW_SLIDER     0
#define DRAW_SPAN       0
#define DRAW_SPINNER    0
#define DRAW_SWITCH     0
#define DRAW_TABLE      0
#define DRAW_TABVIEW    0
#define DRAW_TEXTAREA   0
#define DRAW_WIN        0

// 3rd party libs
#define DRAW_BMP        0
#define DRAW_JPG        0 //https://docs.lvgl.io/master/libs/sjpg.html //Currently only 16 bit image format is supported
#define DRAW_PNG        0
#define DRAW_GIF        0
#define DRAW_QRCODE     0

// Others
#define DRAW_SNAPSHOT   1

#if (DRAW_GIF == 1)
#include "third_party/LVGL/lvgl/examples/libs/gif/img_bulb_gif.c"
LV_IMG_DECLARE(img_bulb_gif)

lv_img_dsc_t bulb_gif = {
  .header.always_zero = 0,
  .header.w = 0,
  .header.h = 0,
  .data_size = 0,
  .header.cf = LV_IMG_CF_RAW,
  .data = img_blub_gif_map,
};

#elif (DRAW_PNG == 1)
#include "third_party/LVGL/lvgl/examples/libs/png/img_wing_png.c"
LV_IMG_DECLARE(img_wink_png)

lv_img_dsc_t wink_png = {
  .header.always_zero = 0,
  .header.w = 50,
  .header.h = 50,
  .data_size = 5158,
  .header.cf = LV_IMG_CF_RAW_ALPHA,
  .data = img_wink_png_map,
};

#elif (DRAW_ANIM_IMG == 1)
#include "third_party/LVGL/lvgl/examples/assets/animimg001.c"
#include "third_party/LVGL/lvgl/examples/assets/animimg002.c"
#include "third_party/LVGL/lvgl/examples/assets/animimg003.c"

LV_IMG_DECLARE(animimg001)
LV_IMG_DECLARE(animimg002)
LV_IMG_DECLARE(animimg003)

lv_img_dsc_t animimg1 = {
  .header.always_zero = 0,
  .header.w = 130,
  .header.h = 170,
  .data_size = 22100 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
};

lv_img_dsc_t animimg2 = {
  .header.always_zero = 0,
  .header.w = 130,
  .header.h = 170,
  .data_size = 22100 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
};

lv_img_dsc_t animimg3 = {
  .header.always_zero = 0,
  .header.w = 130,
  .header.h = 170,
  .data_size = 22100 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
};

static const lv_img_dsc_t * anim_imgs[3] = {
    &animimg1,
    &animimg2,
    &animimg3,
};

#elif (DRAW_CLOCK == 1)
#include "third_party/LVGL/lvgl/examples/assets/img_hand.c"
LV_IMG_DECLARE(img_hand)
lv_img_dsc_t clk_hand = {
  .header.always_zero = 0,
  .header.w = 100,
  .header.h = 9,
  .data_size = 900 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
  .data = img_hand_map,
};
#elif (DRAW_IMG_ROTATE == 1) || (DRAW_IMG_RECOLOR == 1)
#include "third_party/LVGL/lvgl/examples/assets/img_cogwheel_argb.c"
LV_IMG_DECLARE(img_cogwheel_argb)
lv_img_dsc_t img_cogwheel = {
  .header.always_zero = 0,
  .header.w = 100,
  .header.h = 100,
  .data_size = 10000 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
  .data = img_cogwheel_argb_map,
};
#elif (DRAW_IMGBTN == 1)
#include "third_party/LVGL/lvgl/examples/assets/imgbtn_left.c"
LV_IMG_DECLARE(imgbtn_left)
lv_img_dsc_t limgbtn = {
  .header.always_zero = 0,
  .header.w = 8,
  .header.h = 50,
  .data_size = 400 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
  .data = imgbtn_left_map,
};
#include "third_party/LVGL/lvgl/examples/assets/imgbtn_right.c"
LV_IMG_DECLARE(imgbtn_right)
lv_img_dsc_t rimgbtn = {
  .header.always_zero = 0,
  .header.w = 8,
  .header.h = 50,
  .data_size = 400 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
  .data = imgbtn_right_map,
};
#include "third_party/LVGL/lvgl/examples/assets/imgbtn_mid.c"
LV_IMG_DECLARE(imgbtn_mid)
lv_img_dsc_t mimgbtn = {
  .header.always_zero = 0,
  .header.w = 5,
  .header.h = 49,
  .data_size = 245 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
  .data = imgbtn_mid_map,
};
#elif (DRAW_SNAPSHOT == 1)
#include "texture/phone_100x100_bgra8888.h"
#include "texture/message_100x100_bgra8888.h"

LV_IMG_DECLARE(phone_100x100_bgra8888)
LV_IMG_DECLARE(message_100x100_bgra8888)

lv_img_dsc_t img_phone_bgra_psram = {
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 100,
  .header.h = 100,
  .data_size = 10000 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .data = phone_100x100_bgra8888_map,
};

lv_img_dsc_t icon_msg_bgra_psram = {
  .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 100,
  .header.h = 100,
  .data_size = 10000 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .data = message_100x100_bgra8888_map,
};
#elif (DRAW_ARC == 1)
#include "texture/color_wheel_300_300_argb8888.h"
lv_img_dsc_t img_color_wheel_argb_psram = {
  .header.cf = LV_IMG_CF_AMBIQ_ARGB8888,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 300,
  .header.h = 300,
  .data_size = 300*300 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .data = color_wheel_300_300_argb8888,
};
#endif

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
//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern am_util_stdio_print_char_t g_pfnCharPrint;


static lv_style_t g_style_btn;
static void style_init(void)
{
    /*Create a simple button style*/
    lv_style_init(&g_style_btn);

    lv_style_set_radius(&g_style_btn, 50);
    lv_style_set_bg_opa(&g_style_btn, LV_OPA_COVER);
    lv_style_set_bg_color(&g_style_btn, lv_palette_main(LV_PALETTE_BLUE));

    /*Add a shadow*/
    lv_style_set_shadow_width(&g_style_btn, 25);
    lv_style_set_shadow_color(&g_style_btn, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_shadow_ofs_x(&g_style_btn, 8);
    lv_style_set_shadow_ofs_y(&g_style_btn, 8);

    lv_style_set_text_color(&g_style_btn, lv_color_white());
}

#if (DRAW_ARC == 1)
static void set_angle(void * obj, int32_t v)
{
    lv_arc_set_value(obj, v);
}
#endif

#if (DRAW_ANIM_PLAYBACK == 1)
static void anim_x_cb(void * var, int32_t v)
{
    lv_obj_set_x(var, v);
}

static void anim_size_cb(void * var, int32_t v)
{
    lv_obj_set_size(var, v, v);
}
#endif

#if (DRAW_METER == 1) || (DRAW_CLOCK == 1)
static lv_obj_t * meter;
static void set_value(void * indic, int32_t v)
{
    lv_meter_set_indicator_end_value(meter, indic, v);
}
#endif

#if DRAW_CALENDAR == 1
#define LV_USE_CALENDAR_HEADER_ARROW    1
static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_current_target(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_calendar_date_t date;
        if(lv_calendar_get_pressed_date(obj, &date)) {
            LV_LOG_USER("Clicked date: %02d.%02d.%d", date.day, date.month, date.year);
        }
    }
}
#endif

#if (DRAW_GRID == 1)
static void row_gap_anim(void * obj, int32_t v)
{
    lv_obj_set_style_pad_row(obj, v, 0);
}

static void column_gap_anim(void * obj, int32_t v)
{
    lv_obj_set_style_pad_column(obj, v, 0);
}
#endif

#if (DRAW_KEYBOARD == 1)
static void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = lv_event_get_user_data(e);
    if(code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }

    if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}
#endif

#if (DRAW_BAR == 1)
static void set_temp(void * bar, int32_t temp)
{
    lv_bar_set_value(bar, temp, LV_ANIM_ON);
}
#endif

#if (DRAW_IMG_ROTATE == 1)
static void set_angle(void * img, int32_t v)
{
    lv_img_set_angle(img, v);
}

static void set_zoom(void * img, int32_t v)
{
    lv_img_set_zoom(img, v);
}
#endif

#if (DRAW_LABEL == 1)
#define MASK_WIDTH 200
#define MASK_HEIGHT 90

static void add_mask_event_cb(lv_event_t * e)
{
    static lv_draw_mask_map_param_t m;
    static int16_t mask_id;

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    lv_opa_t * mask_map = lv_event_get_user_data(e);
    if(code == LV_EVENT_COVER_CHECK) {
        lv_event_set_cover_res(e, LV_COVER_RES_MASKED);
    }
    else if(code == LV_EVENT_DRAW_MAIN_BEGIN) {
        lv_draw_mask_map_init(&m, &obj->coords, mask_map);
        mask_id = lv_draw_mask_add(&m, NULL);

    }
    else if(code == LV_EVENT_DRAW_MAIN_END) {
        lv_draw_mask_free_param(&m);
        lv_draw_mask_remove_id(mask_id);
    }
}
#endif

#if (DRAW_CHART == 1)
static void draw_event_cb(lv_event_t * e)
{
    lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e);
    if(dsc->part == LV_PART_ITEMS)
    {
        lv_obj_t * obj = lv_event_get_target(e);
        lv_chart_series_t * ser = lv_chart_get_series_next(obj, NULL);
        uint32_t cnt = lv_chart_get_point_count(obj);
        /*Make older value more transparent*/
        dsc->rect_dsc->bg_opa = (LV_OPA_COVER *  dsc->id) / (cnt - 1);

        /*Make smaller values blue, higher values red*/
        lv_coord_t * x_array = lv_chart_get_x_array(obj, ser);
        lv_coord_t * y_array = lv_chart_get_y_array(obj, ser);
        /*dsc->id is the tells drawing order, but we need the ID of the point being drawn.*/
        uint32_t start_point = lv_chart_get_x_start_point(obj, ser);
        uint32_t p_act = (start_point + dsc->id) % cnt; /*Consider start point to get the index of the array*/
        lv_opa_t x_opa = (x_array[p_act] * LV_OPA_50) / 200;
        lv_opa_t y_opa = (y_array[p_act] * LV_OPA_50) / 1000;

        dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_RED),
                                               lv_palette_main(LV_PALETTE_BLUE),
                                               x_opa + y_opa);
    }
}

static void add_data(lv_timer_t * timer)
{
    LV_UNUSED(timer);
    lv_obj_t * chart = timer->user_data;
    lv_chart_set_next_value2(chart, lv_chart_get_series_next(chart, NULL), lv_rand(0, 200), lv_rand(0, 1000));
}
#endif

#if (DRAW_MSGBOX == 1)
static void event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_current_target(e);
    LV_LOG_USER("Button %s clicked", lv_msgbox_get_active_btn_text(obj));
}
#endif

#if (DRAW_ROLLER == 1)
static void mask_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    static int16_t mask_top_id = -1;
    static int16_t mask_bottom_id = -1;

    if(code == LV_EVENT_COVER_CHECK) {
        lv_event_set_cover_res(e, LV_COVER_RES_MASKED);

    }
    else if(code == LV_EVENT_DRAW_MAIN_BEGIN) {
        /* add mask */
        const lv_font_t * font = lv_obj_get_style_text_font(obj, LV_PART_MAIN);
        lv_coord_t line_space = lv_obj_get_style_text_line_space(obj, LV_PART_MAIN);
        lv_coord_t font_h = lv_font_get_line_height(font);

        lv_area_t roller_coords;
        lv_obj_get_coords(obj, &roller_coords);

        lv_area_t rect_area;
        rect_area.x1 = roller_coords.x1;
        rect_area.x2 = roller_coords.x2;
        rect_area.y1 = roller_coords.y1;
        rect_area.y2 = roller_coords.y1 + (lv_obj_get_height(obj) - font_h - line_space) / 2;

        lv_draw_mask_fade_param_t * fade_mask_top = lv_mem_buf_get(sizeof(lv_draw_mask_fade_param_t));
        lv_draw_mask_fade_init(fade_mask_top, &rect_area, LV_OPA_TRANSP, rect_area.y1, LV_OPA_COVER, rect_area.y2);
        mask_top_id = lv_draw_mask_add(fade_mask_top, NULL);

        rect_area.y1 = rect_area.y2 + font_h + line_space - 1;
        rect_area.y2 = roller_coords.y2;

        lv_draw_mask_fade_param_t * fade_mask_bottom = lv_mem_buf_get(sizeof(lv_draw_mask_fade_param_t));
        lv_draw_mask_fade_init(fade_mask_bottom, &rect_area, LV_OPA_COVER, rect_area.y1, LV_OPA_TRANSP, rect_area.y2);
        mask_bottom_id = lv_draw_mask_add(fade_mask_bottom, NULL);

    }
    else if(code == LV_EVENT_DRAW_POST_END) {
        lv_draw_mask_fade_param_t * fade_mask_top = lv_draw_mask_remove_id(mask_top_id);
        lv_draw_mask_fade_param_t * fade_mask_bottom = lv_draw_mask_remove_id(mask_bottom_id);
        lv_draw_mask_free_param(fade_mask_top);
        lv_draw_mask_free_param(fade_mask_bottom);
        lv_mem_buf_release(fade_mask_top);
        lv_mem_buf_release(fade_mask_bottom);
        mask_top_id = -1;
        mask_bottom_id = -1;
    }
}
#endif

#if (DRAW_WIN == 1)
static void event_handler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    //LV_LOG_USER("Button %d clicked", (int)lv_obj_get_index(obj));
}
#endif

#if (DRAW_TABLE == 1)
static void draw_part_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e);
    /*If the cells are drawn...*/
    if(dsc->part == LV_PART_ITEMS) {
        uint32_t row = dsc->id /  lv_table_get_col_cnt(obj);
        uint32_t col = dsc->id - row * lv_table_get_col_cnt(obj);

        /*Make the texts in the first cell center aligned*/
        if(row == 0) {
            dsc->label_dsc->align = LV_TEXT_ALIGN_CENTER;
            dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_BLUE), dsc->rect_dsc->bg_color, LV_OPA_20);
            dsc->rect_dsc->bg_opa = LV_OPA_COVER;
        }
        /*In the first column align the texts to the right*/
        else if(col == 0) {
            dsc->label_dsc->align = LV_TEXT_ALIGN_RIGHT;
        }

        /*MAke every 2nd row grayish*/
        if((row != 0 && row % 2) == 0) {
            dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_GREY), dsc->rect_dsc->bg_color, LV_OPA_10);
            dsc->rect_dsc->bg_opa = LV_OPA_COVER;
        }
    }
}
#endif

#if (DRAW_TABVIEW == 1)
static void scroll_begin_event(lv_event_t * e)
{
    /*Disable the scroll animations. Triggered when a tab button is clicked */
    if(lv_event_get_code(e) == LV_EVENT_SCROLL_BEGIN) {
        lv_anim_t * a = lv_event_get_param(e);
        if(a)  a->time = 0;
    }
}
#endif

#if (DRAW_LIST == 1)
static lv_obj_t * list1;
static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked: %s", lv_list_get_btn_text(list1, obj));
    }
}
#endif

#if (DRAW_SCROLL == 1)
static void scroll_event_cb(lv_event_t * e)
{
    lv_obj_t * cont = lv_event_get_target(e);

    lv_area_t cont_a;
    lv_obj_get_coords(cont, &cont_a);
    lv_coord_t cont_y_center = cont_a.y1 + lv_area_get_height(&cont_a) / 2;

    lv_coord_t r = lv_obj_get_height(cont) * 7 / 10;
    uint32_t i;
    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    for(i = 0; i < child_cnt; i++) {
        lv_obj_t * child = lv_obj_get_child(cont, i);
        lv_area_t child_a;
        lv_obj_get_coords(child, &child_a);

        lv_coord_t child_y_center = child_a.y1 + lv_area_get_height(&child_a) / 2;

        lv_coord_t diff_y = child_y_center - cont_y_center;
        diff_y = LV_ABS(diff_y);

        /*Get the x of diff_y on a circle.*/
        lv_coord_t x;
        /*If diff_y is out of the circle use the last point of the circle (the radius)*/
        if(diff_y >= r) {
            x = r;
        }
        else {
            /*Use Pythagoras theorem to get x from radius and y*/
            uint32_t x_sqr = r * r - diff_y * diff_y;
            lv_sqrt_res_t res;
            lv_sqrt(x_sqr, &res, 0x8000);   /*Use lvgl's built in sqrt root function*/
            x = r - res.i;
        }

        /*Translate the item by the calculated X coordinate*/
        lv_obj_set_style_translate_x(child, x, 0);

        /*Use some opacity with larger translations*/
        lv_opa_t opa = lv_map(x, 0, r, LV_OPA_TRANSP, LV_OPA_COVER);
        lv_obj_set_style_opa(child, LV_OPA_COVER - opa, 0);
    }
}

#endif

#if (DRAW_IMG_RECOLOR == 1)
static lv_obj_t * red_slider, * green_slider, * blue_slider, * intense_slider;
static lv_obj_t * img1;
static void slider_event_cb(lv_event_t * e)
{
    LV_UNUSED(e);

    /*Recolor the image based on the sliders' values*/
    lv_color_t color  = lv_color_make(lv_slider_get_value(red_slider), lv_slider_get_value(green_slider),
                                      lv_slider_get_value(blue_slider));
    lv_opa_t intense = lv_slider_get_value(intense_slider);
    lv_obj_set_style_img_recolor_opa(img1, intense, 0);
    lv_obj_set_style_img_recolor(img1, color, 0);
}

static lv_obj_t * create_slider(lv_color_t color)
{
    lv_obj_t * slider = lv_slider_create(lv_scr_act());
    lv_slider_set_range(slider, 0, 255);
    lv_obj_set_size(slider, 10, 200);
    lv_obj_set_style_bg_color(slider, color, LV_PART_KNOB);
    lv_obj_set_style_bg_color(slider, lv_color_darken(color, LV_OPA_40), LV_PART_INDICATOR);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    return slider;
}
#endif

#if (DRAW_CHECKBOX == 1)
static void anim_x_cb(void * var, int32_t v)
{
    lv_obj_set_x(var, v);
}

static void sw_event_cb(lv_event_t * e)
{
    lv_obj_t * sw = lv_event_get_target(e);
    lv_obj_t * label = lv_event_get_user_data(e);

    if(lv_obj_has_state(sw, LV_STATE_CHECKED)) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, label);
        lv_anim_set_values(&a, lv_obj_get_x(label), 100);
        lv_anim_set_time(&a, 500);
        lv_anim_set_exec_cb(&a, anim_x_cb);
        lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
        lv_anim_start(&a);
    }
    else {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, label);
        lv_anim_set_values(&a, lv_obj_get_x(label), -lv_obj_get_width(label));
        lv_anim_set_time(&a, 500);
        lv_anim_set_exec_cb(&a, anim_x_cb);
        lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
        lv_anim_start(&a);
    }
}
#endif

#if (DRAW_SWITCH == 1)
static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("State: %s\n", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "On" : "Off");
    }
}
#endif

#if (DRAW_LED == 1)
uint32_t g_brightness = LV_LED_BRIGHT_MIN;
uint32_t g_ledcolor = LV_PALETTE_RED;
static void event_handler1(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_PRESSED) {
        lv_led_toggle(obj);
    }
}

static void event_handler2(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_PRESSED)
    {
        g_ledcolor++;
        if(g_ledcolor >= _LV_PALETTE_LAST)
        {
            g_ledcolor = LV_PALETTE_RED;
        }
        lv_led_set_color(obj, lv_palette_main(g_ledcolor));
    }
}

static void event_handler3(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_PRESSED)
    {
        g_brightness += 50;
        if(g_brightness > LV_LED_BRIGHT_MAX)
        {
            g_brightness = LV_LED_BRIGHT_MIN;
        }
        lv_led_set_brightness(obj, g_brightness);
    }
}
#endif

#if (DRAW_TEXTAREA == 1)
static lv_obj_t * kb;
static void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
        /*Focus on the clicked text area*/
        if(kb != NULL) lv_keyboard_set_textarea(kb, ta);
    }

    else if(code == LV_EVENT_READY) {
        LV_LOG_USER("Ready, current text: %s", lv_textarea_get_text(ta));
    }
}

#endif

#if (DRAW_EVENT == 1)
static void event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * label = lv_event_get_user_data(e);

    switch(code) {
        case LV_EVENT_PRESSED:
            lv_label_set_text(label, "The last button event:\nLV_EVENT_PRESSED");
            break;
        case LV_EVENT_CLICKED:
            lv_label_set_text(label, "The last button event:\nLV_EVENT_CLICKED");
            break;
        case LV_EVENT_LONG_PRESSED:
            lv_label_set_text(label, "The last button event:\nLV_EVENT_LONG_PRESSED");
            break;
        case LV_EVENT_LONG_PRESSED_REPEAT:
            lv_label_set_text(label, "The last button event:\nLV_EVENT_LONG_PRESSED_REPEAT");
            break;
        default:
            break;
    }
}
#endif

#if (DRAW_BTNMATRIX == 1)
static void event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    uint32_t id = lv_btnmatrix_get_selected_btn(obj);
    bool prev = id == 0 ? true : false;
    bool next = id == 6 ? true : false;
    if(prev || next) {
        /*Find the checked button*/
        uint32_t i;
        for(i = 1; i < 7; i++) {
            if(lv_btnmatrix_has_btn_ctrl(obj, i, LV_BTNMATRIX_CTRL_CHECKED)) break;
        }

        if(prev && i > 1) i--;
        else if(next && i < 5) i++;

        lv_btnmatrix_set_btn_ctrl(obj, i, LV_BTNMATRIX_CTRL_CHECKED);
    }
}
#endif

#if (DRAW_SNAPSHOT == 1)
lv_obj_t * snapshot_obj;
lv_obj_t * snapshot_obj_1;

static void snapshot_ofs_x_anim(void * img, int32_t v)
{
    lv_obj_set_pos(snapshot_obj, v, 0);
    lv_obj_set_pos(snapshot_obj_1, v + lv_obj_get_width(snapshot_obj), 0);
}

static void snapshot_set_zoom(void * img, int32_t v)
{
    lv_img_set_zoom(snapshot_obj, (260 + v));
    lv_img_set_zoom(snapshot_obj_1, (290 - v));
}

static void snapshot_set_opa_anim(void * img, int32_t v)
{
    lv_obj_set_style_opa(snapshot_obj, (-v), 0);
    lv_obj_set_style_opa(snapshot_obj_1, v, 0);
}

// Task
void
msg_switch_task(void)
{
    lv_obj_t * root = lv_scr_act();

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_opa(&style, LV_OPA_COVER);//LV_OPA_TRANSP
    lv_style_set_bg_color(&style, lv_color_make(0xFF, 0xFF, 0xFF));//white
    lv_obj_add_style(root, &style, 0);
    lv_obj_set_size(root, (LV_AMBIQ_FB_RESX), (LV_AMBIQ_FB_RESX));
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_OFF);

    //Create 0
    snapshot_obj = lv_img_create(root);
    lv_obj_set_style_bg_color(snapshot_obj, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_bg_opa(snapshot_obj, LV_OPA_100, 0);

    /*Create the container and its children*/
    lv_obj_t * container = lv_obj_create(root);
    static lv_style_t style_c0;
    lv_style_init(&style_c0);
    lv_style_set_bg_opa(&style_c0, LV_OPA_COVER);//LV_OPA_TRANSP
    lv_style_set_bg_color(&style_c0, lv_color_make(0xFF, 0xFF, 0xFF));//white
    lv_obj_add_style(container, &style_c0, 0);
    lv_obj_set_size(container, (LV_AMBIQ_FB_RESX), (LV_AMBIQ_FB_RESX));
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t * img = lv_img_create(container);
    lv_img_set_src(img, &icon_msg_bgra_psram);
    lv_img_set_zoom(img, 150);
    lv_obj_set_pos(img, 0, 100);

    lv_obj_t * label = lv_label_create(container);          /*Add a label to the button*/
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);     /*Break the long lines*/
    lv_obj_set_width(label, 300);  /*Set smaller width to make the lines wrap*/
    //LV_FONT_DECLARE(lv_font_montserrat_40);
    //lv_obj_set_style_text_font(label, &lv_font_montserrat_40, 0);
    //lv_label_set_recolor(label, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_text(label, "New");                       /*Set the labels text*/
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -20);

    lv_obj_t * label1 = lv_label_create(container);          /*Add a label to the button*/
    lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);       /*Break the long lines*/
    lv_obj_set_width(label1, 300);  /*Set smaller width to make the lines wrap*/
    //lv_obj_set_style_text_font(label1, &lv_font_montserrat_40, 0);
    //lv_label_set_recolor(label1, true);                     /*Enable re-coloring by commands in the text*/
    lv_label_set_text(label1, "Message");                       /*Set the labels text*/
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * label2 = lv_label_create(container);          /*Add a label to the button*/
    lv_label_set_long_mode(label2, LV_LABEL_LONG_WRAP);       /*Break the long lines*/
    lv_obj_set_width(label2, 300);  /*Set smaller width to make the lines wrap*/
    //lv_obj_set_style_text_font(label2, &lv_font_montserrat_40, 0);
    //lv_label_set_recolor(label2, true);                     /*Enable re-coloring by commands in the text*/
    lv_label_set_text(label2, "Received");                       /*Set the labels text*/
    lv_obj_set_style_text_align(label2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label2, LV_ALIGN_CENTER, 0, 20);

    lv_img_dsc_t * snapshot = lv_mem_alloc(sizeof(lv_img_dsc_t));
    if(snapshot == NULL) {
        return;
    }

    lv_res_t rtn = -1;
    uint32_t buff_size = lv_snapshot_buf_size_needed(container, LV_IMG_CF_AMBIQ_RGB565);
    uint8_t *buf = (uint8_t *)lv_mem_external_alloc(buff_size);
    rtn = lv_snapshot_take_to_buf(root, LV_IMG_CF_AMBIQ_RGB565, snapshot, buf, buff_size);

    if(rtn != LV_RES_OK)
    {
        return;
    }

    if(snapshot == NULL){
        return;
    }else{
        lv_img_set_src(snapshot_obj, snapshot);
    }

    lv_obj_del(container);

    //Create 1
    snapshot_obj_1 = lv_img_create(root);
    lv_obj_set_style_bg_color(snapshot_obj_1, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_bg_opa(snapshot_obj_1, LV_OPA_100, 0);

    lv_obj_t * container_1 = lv_obj_create(root);
    static lv_style_t style_c1;
    lv_style_init(&style_c1);
    lv_style_set_bg_opa(&style_c1, LV_OPA_COVER);//LV_OPA_TRANSP
    lv_style_set_bg_color(&style_c1, lv_color_make(0xFF, 0xFF, 0xFF));//white
    lv_obj_add_style(container_1, &style_c1, 0);
    lv_obj_set_size(container_1, (LV_AMBIQ_FB_RESX), (LV_AMBIQ_FB_RESX));
    lv_obj_set_scrollbar_mode(container_1, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t * img2_1 = lv_img_create(container_1);
    lv_img_set_src(img2_1, &img_phone_bgra_psram);
    lv_img_set_zoom(img2_1, 170);
    lv_obj_set_pos(img2_1, 0, 100);
    lv_obj_t * label_1 = lv_label_create(container_1);          /*Add a label to the button*/
    //LV_FONT_DECLARE(lv_font_montserrat_40);
    //lv_obj_set_style_text_font(label_1, &lv_font_montserrat_40, 0);
    //lv_label_set_recolor(label_1, true);                     /*Enable re-coloring by commands in the text*/
    lv_label_set_text(label_1, "Incoming call");                       /*Set the labels text*/
    lv_obj_set_style_text_align(label_1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label_1, LV_ALIGN_CENTER, 0, 0);

    lv_img_dsc_t * snapshot_1 = lv_mem_alloc(sizeof(lv_img_dsc_t));
    if(snapshot_1 == NULL) {
        return;
    }

    rtn = -1;
    buff_size = lv_snapshot_buf_size_needed(container_1, LV_IMG_CF_AMBIQ_RGB565);
    buf = (uint8_t *)lv_mem_external_alloc(buff_size);
    rtn = lv_snapshot_take_to_buf(container_1, LV_IMG_CF_AMBIQ_RGB565, snapshot_1, buf, buff_size);
    if(rtn != LV_RES_OK)
    {
        return;
    }

    if(snapshot_1 == NULL){
        return;
    }else{
        lv_img_set_src(snapshot_obj_1, snapshot_1);
    }

    lv_obj_del(container_1);

    // anim
    lv_obj_set_pos(snapshot_obj, 0, 0);
    lv_obj_align(snapshot_obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(snapshot_obj_1, LV_ALIGN_CENTER, 0, 0);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);

#if 0//def SET_SLIDE
    lv_anim_set_exec_cb(&a, snapshot_ofs_x_anim);
    lv_anim_set_values(&a, 0, -454);//-260);
    lv_anim_set_time(&a, 500);
    lv_anim_set_playback_time(&a, 500);
    lv_anim_start(&a);
#endif

#if 1//def SET_ZOOM
    lv_anim_set_exec_cb(&a, snapshot_set_zoom);
    lv_anim_set_values(&a, 0, 30);
    lv_anim_set_time(&a, 500);
    lv_anim_set_playback_time(&a, 500);
    lv_anim_start(&a);
#endif

#if 1//def SET_OPA
    lv_anim_set_exec_cb(&a, snapshot_set_opa_anim);
    lv_anim_set_values(&a, 0, 255);
    lv_anim_set_time(&a, 500);
    lv_anim_set_playback_time(&a, 500);
    lv_anim_start(&a);
#endif

#if 0
    snapshot = (void *)lv_img_get_src(snapshot_obj);
    if(snapshot) {
        lv_snapshot_free(snapshot);
    }
#endif
}
#endif

//*****************************************************************************
//
// Load texture and font data from MRAM to PSRAM
//
//*****************************************************************************
void
texture_load(void)
{
#if (LV_USE_GPU_AMBIQ_NEMA == 1)
#if DRAW_ANIM_IMG == 1
    animimg1.data = (const uint8_t *)lv_mem_external_alloc(animimg001.data_size);
    animimg2.data = (const uint8_t *)lv_mem_external_alloc(animimg002.data_size);
    animimg3.data = (const uint8_t *)lv_mem_external_alloc(animimg003.data_size);

    memcpy((void*)animimg1.data, animimg001.data, animimg001.data_size);
    memcpy((void*)animimg2.data, animimg002.data, animimg002.data_size);
    memcpy((void*)animimg3.data, animimg003.data, animimg003.data_size);
#elif (DRAW_CLOCK == 1)
    clk_hand.data = (const uint8_t *)lv_mem_external_alloc(img_hand.data_size);
    memcpy((void*)clk_hand.data, img_hand.data, img_hand.data_size);
#elif (DRAW_IMG_ROTATE == 1) || (DRAW_IMG_RECOLOR == 1)
    img_cogwheel.data = (const uint8_t *)lv_mem_external_alloc(img_cogwheel_argb.data_size);
    memcpy((void*)img_cogwheel.data, img_cogwheel_argb.data, img_cogwheel_argb.data_size);
#elif (DRAW_IMGBTN == 1)
    limgbtn.data = (const uint8_t *)lv_mem_external_alloc(imgbtn_left.data_size);
    rimgbtn.data = (const uint8_t *)lv_mem_external_alloc(imgbtn_right.data_size);
    mimgbtn.data = (const uint8_t *)lv_mem_external_alloc(imgbtn_mid.data_size);

    memcpy((void*)limgbtn.data, imgbtn_left.data, imgbtn_left.data_size);
    memcpy((void*)rimgbtn.data, imgbtn_right.data, imgbtn_right.data_size);
    memcpy((void*)mimgbtn.data, imgbtn_mid.data, imgbtn_mid.data_size);
#elif (DRAW_SNAPSHOT == 1)
    img_phone_bgra_psram.data = (const uint8_t *)lv_mem_external_alloc(phone_100x100_bgra8888.data_size);
    icon_msg_bgra_psram.data = (const uint8_t *)lv_mem_external_alloc(message_100x100_bgra8888.data_size);

    memcpy((void*)img_phone_bgra_psram.data, phone_100x100_bgra8888.data, phone_100x100_bgra8888.data_size);
    memcpy((void*)icon_msg_bgra_psram.data, message_100x100_bgra8888.data, message_100x100_bgra8888.data_size);
#elif (DRAW_GIF == 1)
    uint32_t data_size = 60 * 80 * LV_IMG_PX_SIZE_ALPHA_BYTE;
    bulb_gif.data = (const uint8_t *)lv_mem_external_alloc(data_size);
    memcpy((void*)bulb_gif.data, img_bulb_gif.data, data_size);
#elif (DRAW_PNG == 1)
    wink_png.data = (const uint8_t *)lv_mem_external_alloc(img_wink_png.data_size);
    memcpy((void*)wink_png.data, img_wink_png.data, img_wink_png.data_size);

#elif (DRAW_ARC == 1)
    img_color_wheel_argb_psram.data = (const uint8_t *)lv_mem_external_alloc(img_color_wheel_argb_psram.data_size);

    memcpy((void*)img_color_wheel_argb_psram.data, color_wheel_300_300_argb8888, img_color_wheel_argb_psram.data_size);
#endif

    // Alloc buffer in PSRAM to hold the font data.
    uint32_t font_length = lv_ambiq_bitmap_length_get(lv_font_montserrat_14.dsc);
    void* font_buffer = lv_mem_external_alloc(font_length);

    // Copy and align the font bitmap
    lv_font_fmt_txt_dsc_t* font = (lv_font_fmt_txt_dsc_t*)lv_font_montserrat_14.dsc;
    lv_ambiq_A4_font_align(font, font_buffer);
#endif
}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
GuiTask(void *pvParameters)
{
    am_util_stdio_printf("GUI task start!\n");

    // // Init LVGL gui driver
    // lv_init();

#if LV_USE_LOG == 1
    lv_log_register_print_cb((lv_log_print_g_cb_t)g_pfnCharPrint);
#endif

    // Load texture.
    texture_load();

#ifdef LV_AMBIQ_TOUCH_USED
    // Init touchpad driver
    lv_indev_drv_init(&indev_drv); /*Descriptor of a input device driver*/
    indev_drv.type = LV_INDEV_TYPE_POINTER; /*Touch pad is a pointer-like device*/
    indev_drv.read_cb = lv_ambiq_touch_read; /*Set your driver function*/
    lv_indev_drv_register(&indev_drv); /*Finally register the driver*/
#endif

#ifdef LV_AMBIQ_FS_USED
    // Init file system
    lv_ambiq_fs_init();
#endif

    /*Initialize the style*/
    style_init();

#if (DRAW_ARC == 1)
    /*Create an Arc*/
    lv_obj_t * arc = lv_arc_create(lv_scr_act());
    lv_obj_center(arc);
    lv_obj_set_size(arc, 300, 300);
    lv_arc_set_rotation(arc, 270);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);   /*Be sure the knob is not displayed*/
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);  /*To not allow adjusting by click*/
    lv_obj_center(arc);
    lv_obj_set_style_arc_img_src(arc, &img_color_wheel_argb_psram, LV_PART_INDICATOR);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, arc);
    lv_anim_set_exec_cb(&a, set_angle);
    lv_anim_set_time(&a, 1000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);    /*Just for the demo*/
    lv_anim_set_repeat_delay(&a, 500);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_start(&a);

#elif (DRAW_ANIM_PLAYBACK == 1)
    lv_obj_t * obj = lv_obj_create(lv_scr_act());
    lv_obj_set_style_bg_color(obj, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0);

    lv_obj_align(obj, LV_ALIGN_LEFT_MID, 10, 0);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, 10, 50);
    lv_anim_set_time(&a, 1000);
    lv_anim_set_playback_delay(&a, 100);
    lv_anim_set_playback_time(&a, 300);
    lv_anim_set_repeat_delay(&a, 500);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);

    lv_anim_set_exec_cb(&a, anim_size_cb);
    lv_anim_start(&a);
    lv_anim_set_exec_cb(&a, anim_x_cb);
    lv_anim_set_values(&a, 10, 240);
    lv_anim_start(&a);

#elif (DRAW_SPINNER == 1)
    /*Create a spinner*/
    lv_obj_t * spinner = lv_spinner_create(lv_scr_act(), 1000, 60);
    lv_obj_set_size(spinner, 200, 200);
    lv_obj_center(spinner);

#elif (DRAW_METER == 1)
    meter = lv_meter_create(lv_scr_act());
    lv_obj_center(meter);
    lv_obj_set_size(meter, 300, 300);

    /*Add a scale first*/
    lv_meter_scale_t * scale = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale, 41, 2, 10, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_major_ticks(meter, scale, 8, 4, 15, lv_color_black(), 10);

    lv_meter_indicator_t * indic;

    /*Add a blue arc to the start*/
    indic = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_meter_set_indicator_start_value(meter, indic, 0);
    lv_meter_set_indicator_end_value(meter, indic, 20);

    /*Make the tick lines blue at the start of the scale*/
    indic = lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_BLUE),
                                     false, 0);
    lv_meter_set_indicator_start_value(meter, indic, 0);
    lv_meter_set_indicator_end_value(meter, indic, 20);

    /*Add a red arc to the end*/
    indic = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_RED), 0);
    lv_meter_set_indicator_start_value(meter, indic, 80);
    lv_meter_set_indicator_end_value(meter, indic, 100);

    /*Make the tick lines red at the end of the scale*/
    indic = lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_RED), lv_palette_main(LV_PALETTE_RED), false,
                                     0);
    lv_meter_set_indicator_start_value(meter, indic, 80);
    lv_meter_set_indicator_end_value(meter, indic, 100);

    /*Add a needle line indicator*/
    indic = lv_meter_add_needle_line(meter, scale, 4, lv_palette_main(LV_PALETTE_GREY), -10);

    /*Create an animation to set the value*/
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_exec_cb(&a, set_value);
    lv_anim_set_var(&a, indic);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_set_time(&a, 2000);
    lv_anim_set_repeat_delay(&a, 100);
    lv_anim_set_playback_time(&a, 500);
    lv_anim_set_playback_delay(&a, 100);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

#elif (DRAW_ANIM_IMG == 1)
    lv_obj_t * animimg0 = lv_animimg_create(lv_scr_act());
    lv_obj_center(animimg0);

    lv_animimg_set_src(animimg0, (lv_img_dsc_t **) anim_imgs, 3);
    lv_animimg_set_duration(animimg0, 1000);
    lv_animimg_set_repeat_count(animimg0, LV_ANIM_REPEAT_INFINITE);
    lv_animimg_start(animimg0);

#elif (DRAW_CALENDAR == 1)
    lv_obj_t  * calendar = lv_calendar_create(lv_scr_act());
    lv_obj_set_size(calendar, 380, 380);
    lv_obj_align(calendar, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(calendar, event_handler, LV_EVENT_ALL, NULL);

    lv_calendar_set_today_date(calendar, 2022, 07, 06);
    lv_calendar_set_showed_date(calendar, 2022, 07);

    /*Highlight a few days*/
    static lv_calendar_date_t highlighted_days[3];       /*Only its pointer will be saved so should be static*/
    highlighted_days[0].year = 2022;
    highlighted_days[0].month = 07;
    highlighted_days[0].day = 6;

    highlighted_days[1].year = 2022;
    highlighted_days[1].month = 07;
    highlighted_days[1].day = 11;

    highlighted_days[2].year = 2022;
    highlighted_days[2].month = 07;
    highlighted_days[2].day = 22;

    lv_calendar_set_highlighted_dates(calendar, highlighted_days, 3);

#if LV_USE_CALENDAR_HEADER_DROPDOWN
    lv_calendar_header_dropdown_create(calendar);
#elif LV_USE_CALENDAR_HEADER_ARROW
    lv_calendar_header_arrow_create(calendar);
#endif
    lv_calendar_set_showed_date(calendar, 2022, 10);

#elif (DRAW_COLORWHEEL == 1)
    lv_obj_t * cw;

    cw = lv_colorwheel_create(lv_scr_act(), true);
    lv_obj_set_size(cw, 300, 300);
    lv_obj_center(cw);

#elif (DRAW_SPAN == 1)
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_border_width(&style, 1);
    lv_style_set_border_color(&style, lv_palette_main(LV_PALETTE_ORANGE));
    lv_style_set_pad_all(&style, 2);

    lv_obj_t * spans = lv_spangroup_create(lv_scr_act());
    lv_obj_set_width(spans, 300);
    lv_obj_set_height(spans, 300);
    lv_obj_center(spans);
    lv_obj_add_style(spans, &style, 0);

    lv_spangroup_set_align(spans, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(spans, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_indent(spans, 20);
    lv_spangroup_set_mode(spans, LV_SPAN_MODE_BREAK);

    lv_span_t * span = lv_spangroup_new_span(spans);
    lv_span_set_text(span, "China is a beautiful country.");
    lv_style_set_text_color(&span->style, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_text_decor(&span->style, LV_TEXT_DECOR_STRIKETHROUGH | LV_TEXT_DECOR_UNDERLINE);
    lv_style_set_text_opa(&span->style, LV_OPA_50);

    span = lv_spangroup_new_span(spans);
    lv_span_set_text_static(span, "good good study, day day up.");
#if LV_FONT_MONTSERRAT_24
    lv_style_set_text_font(&span->style,  &lv_font_montserrat_24);
#endif
    lv_style_set_text_color(&span->style, lv_palette_main(LV_PALETTE_GREEN));

    span = lv_spangroup_new_span(spans);
    lv_span_set_text_static(span, "LVGL is an open-source graphics library.");
    lv_style_set_text_color(&span->style, lv_palette_main(LV_PALETTE_BLUE));

    span = lv_spangroup_new_span(spans);
    lv_span_set_text_static(span, "the boy no name.");
    lv_style_set_text_color(&span->style, lv_palette_main(LV_PALETTE_GREEN));
#if LV_FONT_MONTSERRAT_20
    lv_style_set_text_font(&span->style, &lv_font_montserrat_20);
#endif
    lv_style_set_text_decor(&span->style, LV_TEXT_DECOR_UNDERLINE);

    span = lv_spangroup_new_span(spans);
    lv_span_set_text(span, "I have a dream that hope to come true.");

    lv_spangroup_refr_mode(spans);

#elif (DRAW_CLOCK == 1)
    meter = lv_meter_create(lv_scr_act());
    lv_obj_set_size(meter, 320, 320);
    lv_obj_center(meter);

    /*Create a scale for the minutes*/
    /*61 ticks in a 360 degrees range (the last and the first line overlaps)*/
    lv_meter_scale_t * scale_min = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale_min, 61, 1, 10, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_range(meter, scale_min, 0, 60, 360, 270);

    /*Create another scale for the hours. It's only visual and contains only major ticks*/
    lv_meter_scale_t * scale_hour = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale_hour, 12, 0, 0, lv_palette_main(LV_PALETTE_GREY));               /*12 ticks*/
    lv_meter_set_scale_major_ticks(meter, scale_hour, 1, 2, 20, lv_color_black(), 10);    /*Every tick is major*/
    lv_meter_set_scale_range(meter, scale_hour, 1, 12, 330, 300);       /*[1..12] values in an almost full circle*/

    /*Add a the hands from images*/
    lv_meter_indicator_t * indic_min = lv_meter_add_needle_img(meter, scale_min, &clk_hand, 5, 5);
    lv_meter_indicator_t * indic_hour = lv_meter_add_needle_img(meter, scale_min, &clk_hand, 5, 5);

    /*Create an animation to set the value*/
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_exec_cb(&a, set_value);
    lv_anim_set_values(&a, 0, 60);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_time(&a, 2000);     /*2 sec for 1 turn of the minute hand (1 hour)*/
    lv_anim_set_var(&a, indic_min);
    lv_anim_start(&a);

    lv_anim_set_var(&a, indic_hour);
    lv_anim_set_time(&a, 24000);    /*24 sec for 1 turn of the hour hand*/
    lv_anim_set_values(&a, 0, 60);
    lv_anim_start(&a);

#elif (DRAW_GRID == 1)
    /*60x60 cells*/
    static lv_coord_t col_dsc[] = {60, 60, 60, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {45, 45, 45, LV_GRID_TEMPLATE_LAST};

    /*Create a container with grid*/
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 300, 220);
    lv_obj_center(cont);
    lv_obj_set_grid_dsc_array(cont, col_dsc, row_dsc);

    lv_obj_t * label;
    lv_obj_t * obj;
    uint32_t i;
    for(i = 0; i < 9; i++) {
        uint8_t col = i % 3;
        uint8_t row = i / 3;

        obj = lv_obj_create(cont);
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, col, 1,
                             LV_GRID_ALIGN_STRETCH, row, 1);
        label = lv_label_create(obj);
        lv_label_set_text_fmt(label, "%d,%d", col, row);
        lv_obj_center(label);
    }

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, cont);
    lv_anim_set_values(&a, 0, 10);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);

    lv_anim_set_exec_cb(&a, row_gap_anim);
    lv_anim_set_time(&a, 500);
    lv_anim_set_playback_time(&a, 500);
    lv_anim_start(&a);

    lv_anim_set_exec_cb(&a, column_gap_anim);
    lv_anim_set_time(&a, 3000);
    lv_anim_set_playback_time(&a, 3000);
    lv_anim_start(&a);
#elif (DRAW_KEYBOARD == 1)
    /*Create a keyboard to use it with an of the text areas*/
    lv_obj_t * kb = lv_keyboard_create(lv_scr_act());

    /*Create a text area. The keyboard will write here*/
    lv_obj_t * ta;
    ta = lv_textarea_create(lv_scr_act());
    lv_obj_align(ta, LV_ALIGN_TOP_LEFT, 50, 50);
    lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb);
    lv_textarea_set_placeholder_text(ta, "Hello");
    lv_obj_set_size(ta, 160, 120);

    ta = lv_textarea_create(lv_scr_act());
    lv_obj_align(ta, LV_ALIGN_TOP_RIGHT, -50, 50);
    lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb);
    lv_obj_set_size(ta, 160, 120);

    lv_keyboard_set_textarea(kb, ta);
    lv_obj_align(kb, LV_ALIGN_CENTER, 0, 50);

#elif (DRAW_BAR == 1)
    static lv_style_t style_indic;

    lv_style_init(&style_indic);
    lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_bg_grad_color(&style_indic, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_bg_grad_dir(&style_indic, LV_GRAD_DIR_VER);

    lv_obj_t * bar = lv_bar_create(lv_scr_act());
    lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);
    lv_obj_set_size(bar, 20, 200);
    lv_obj_center(bar);
    lv_bar_set_range(bar, -20, 40);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_exec_cb(&a, set_temp);
    lv_anim_set_time(&a, 3000);
    lv_anim_set_playback_time(&a, 3000);
    lv_anim_set_var(&a, bar);
    lv_anim_set_values(&a, -20, 40);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

#elif (DRAW_IMG_ROTATE == 1)

    /*Now create the actual image*/
    lv_obj_t * img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &img_cogwheel);
    lv_obj_align(img, LV_ALIGN_CENTER, 50, 50);
    lv_img_set_pivot(img, 0, 0);    /*Rotate around the top left corner*/

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, img);
    lv_anim_set_exec_cb(&a, set_angle);
    lv_anim_set_values(&a, 0, 3600);
    lv_anim_set_time(&a, 5000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    lv_anim_set_exec_cb(&a, set_zoom);
    lv_anim_set_values(&a, 128, 512);
    lv_anim_set_playback_time(&a, 3000);
    lv_anim_start(&a);

#elif (DRAW_LABEL == 1)
    /* Create the mask of a text by drawing it to a canvas*/
    lv_opa_t *mask_map = lv_mem_ssram_alloc(MASK_WIDTH * MASK_HEIGHT);

    /*Create a "8 bit alpha" canvas and clear it*/
    lv_obj_t * canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(canvas, mask_map, MASK_WIDTH, MASK_HEIGHT, LV_IMG_CF_ALPHA_8BIT);
    lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_TRANSP);

    /*Draw a label to the canvas. The result "image" will be used as mask*/
    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.color = lv_color_white();
    label_dsc.align = LV_TEXT_ALIGN_CENTER;
    lv_canvas_draw_text(canvas, 5, 40, MASK_WIDTH, &label_dsc, "Text with gradient");

    /*The mask is reads the canvas is not required anymore*/
    lv_obj_del(canvas);

    /* Create an object from where the text will be masked out.
     * Now it's a rectangle with a gradient but it could be an image too*/
    lv_obj_t * grad = lv_obj_create(lv_scr_act());
    lv_obj_set_size(grad, MASK_WIDTH, MASK_HEIGHT);
    lv_obj_center(grad);
    lv_obj_set_style_bg_color(grad, lv_color_hex(0xff0000), 0);
    lv_obj_set_style_bg_grad_color(grad, lv_color_hex(0x0000ff), 0);
    lv_obj_set_style_bg_grad_dir(grad, LV_GRAD_DIR_HOR, 0);
    lv_obj_add_event_cb(grad, add_mask_event_cb, LV_EVENT_ALL, mask_map);

#elif (DRAW_CHART == 1)
    lv_obj_t * chart = lv_chart_create(lv_scr_act());
    lv_obj_set_size(chart, 300, 200);
    lv_obj_align(chart, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(chart, draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
    lv_obj_set_style_line_width(chart, 0, LV_PART_ITEMS);   /*Remove the lines*/

    lv_chart_set_type(chart, LV_CHART_TYPE_SCATTER);

    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 5, 5, 5, 1, true, 30);
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 10, 5, 6, 5, true, 50);

    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_X, 0, 200);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 1000);

    lv_chart_set_point_count(chart, 50);

    lv_chart_series_t * ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    uint32_t i;
    for(i = 0; i < 50; i++) {
        lv_chart_set_next_value2(chart, ser, lv_rand(0, 200), lv_rand(0, 1000));
    }

    lv_timer_create(add_data, 100, chart);

#elif (DRAW_MSGBOX == 1)
    static const char * btns[] = {"Apply", "Close", ""};

    lv_obj_t * mbox1 = lv_msgbox_create(lv_scr_act(), "Hello", "This is a message box with two buttons.", btns, true);
    lv_obj_set_size(mbox1, 320, 300);
    lv_obj_add_event_cb(mbox1, event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_center(mbox1);

#elif (DRAW_ROLLER == 1)
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_bg_color(&style, lv_color_black());
    lv_style_set_text_color(&style, lv_color_white());
    lv_style_set_border_width(&style, 0);
    lv_style_set_pad_all(&style, 0);
    lv_obj_add_style(lv_scr_act(), &style, 0);

    lv_obj_t * roller1 = lv_roller_create(lv_scr_act());
    lv_obj_add_style(roller1, &style, 0);
    lv_obj_set_style_bg_opa(roller1, LV_OPA_TRANSP, LV_PART_SELECTED);

    lv_roller_set_options(roller1,
                          "January\n"
                          "February\n"
                          "March\n"
                          "April\n"
                          "May\n"
                          "June\n"
                          "July\n"
                          "August\n"
                          "September\n"
                          "October\n"
                          "November\n"
                          "December",
                          LV_ROLLER_MODE_NORMAL);

    lv_obj_center(roller1);
    lv_roller_set_visible_row_count(roller1, 3);
    lv_obj_add_event_cb(roller1, mask_event_cb, LV_EVENT_ALL, NULL);
#elif (DRAW_WIN == 1)
    lv_obj_t * win = lv_win_create(lv_scr_act(), 40);
    lv_obj_set_size(win, 300, 300);
    lv_obj_align(win, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t * btn;
    btn = lv_win_add_btn(win, LV_SYMBOL_LEFT, 40);
    lv_obj_set_size(btn, 40, 20);
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    lv_win_add_title(win, "A title");

    btn = lv_win_add_btn(win, LV_SYMBOL_RIGHT, 40);
    lv_obj_set_size(btn, 40, 20);
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    btn = lv_win_add_btn(win, LV_SYMBOL_CLOSE, 60);
    lv_obj_set_size(btn, 60, 20);
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t * cont = lv_win_get_content(win);  /*Content can be added here*/
    lv_obj_t * label = lv_label_create(cont);
    lv_label_set_text(label, "This is\n"
                      "a pretty\n"
                      "long text\n"
                      "to see how\n"
                      "the window\n"
                      "becomes\n"
                      "scrollable.\n"
                      "\n"
                      "\n"
                      "Some more\n"
                      "text to be\n"
                      "sure it\n"
                      "overflows. :)");

#elif (DRAW_TABLE == 1)
    lv_obj_t * table = lv_table_create(lv_scr_act());

    /*Fill the first column*/
    lv_table_set_cell_value(table, 0, 0, "Name");
    lv_table_set_cell_value(table, 1, 0, "Apple");
    lv_table_set_cell_value(table, 2, 0, "Banana");
    lv_table_set_cell_value(table, 3, 0, "Lemon");
    lv_table_set_cell_value(table, 4, 0, "Grape");
    lv_table_set_cell_value(table, 5, 0, "Melon");
    lv_table_set_cell_value(table, 6, 0, "Peach");
    lv_table_set_cell_value(table, 7, 0, "Nuts");

    /*Fill the second column*/
    lv_table_set_cell_value(table, 0, 1, "Price");
    lv_table_set_cell_value(table, 1, 1, "$7");
    lv_table_set_cell_value(table, 2, 1, "$4");
    lv_table_set_cell_value(table, 3, 1, "$6");
    lv_table_set_cell_value(table, 4, 1, "$2");
    lv_table_set_cell_value(table, 5, 1, "$5");
    lv_table_set_cell_value(table, 6, 1, "$1");
    lv_table_set_cell_value(table, 7, 1, "$9");

    /*Set a smaller height to the table. It'll make it scrollable*/
    lv_obj_set_width(table, 320);
    lv_obj_set_height(table, 320);
    lv_table_set_col_width(table, 0, 180);
    lv_table_set_col_width(table, 1, 160);
    lv_obj_center(table);

    /*Add an event callback to to apply some custom drawing*/
    lv_obj_add_event_cb(table, draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);

#elif (DRAW_TABVIEW == 1)
    /*Create a Tab view object*/
    lv_obj_t * tabview;
    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_LEFT, 80);
    lv_obj_add_event_cb(lv_tabview_get_content(tabview), scroll_begin_event, LV_EVENT_SCROLL_BEGIN, NULL);

    lv_obj_set_style_bg_color(tabview, lv_palette_lighten(LV_PALETTE_RED, 2), 0);

    lv_obj_t * tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_style_bg_color(tab_btns, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    lv_obj_set_style_text_color(tab_btns, lv_palette_lighten(LV_PALETTE_GREY, 5), 0);
    lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_RIGHT, LV_PART_ITEMS | LV_STATE_CHECKED);


    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t * tab1 = lv_tabview_add_tab(tabview, "Tab 1");
    lv_obj_t * tab2 = lv_tabview_add_tab(tabview, "Tab 2");
    lv_obj_t * tab3 = lv_tabview_add_tab(tabview, "Tab 3");
    lv_obj_t * tab4 = lv_tabview_add_tab(tabview, "Tab 4");
    lv_obj_t * tab5 = lv_tabview_add_tab(tabview, "Tab 5");

    lv_obj_set_style_bg_color(tab2, lv_palette_lighten(LV_PALETTE_AMBER, 3), 0);
    lv_obj_set_style_bg_opa(tab2, LV_OPA_COVER, 0);

    /*Add content to the tabs*/
    lv_obj_t * label = lv_label_create(tab1);
    lv_label_set_text(label, "First tab");

    label = lv_label_create(tab2);
    lv_label_set_text(label, "Second tab");

    label = lv_label_create(tab3);
    lv_label_set_text(label, "Third tab");

    label = lv_label_create(tab4);
    lv_label_set_text(label, "Forth tab");

    label = lv_label_create(tab5);
    lv_label_set_text(label, "Fifth tab");

    lv_obj_clear_flag(lv_tabview_get_content(tabview), LV_OBJ_FLAG_SCROLLABLE);

#elif (DRAW_LIST == 1)
    /*Create a list*/
    list1 = lv_list_create(lv_scr_act());
    lv_obj_set_size(list1, 300, 300);
    lv_obj_center(list1);

    /*Add buttons to the list*/
    lv_obj_t * btn;

    lv_list_add_text(list1, "File");
    btn = lv_list_add_btn(list1, LV_SYMBOL_FILE, "New");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_DIRECTORY, "Open");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_SAVE, "Save");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_CLOSE, "Delete");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_EDIT, "Edit");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    lv_list_add_text(list1, "Connectivity");
    btn = lv_list_add_btn(list1, LV_SYMBOL_BLUETOOTH, "Bluetooth");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_GPS, "Navigation");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_USB, "USB");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_BATTERY_FULL, "Battery");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    lv_list_add_text(list1, "Exit");
    btn = lv_list_add_btn(list1, LV_SYMBOL_OK, "Apply");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_CLOSE, "Close");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

#elif (DRAW_SCROLL == 1)
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 350, 350);
    lv_obj_center(cont);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_event_cb(cont, scroll_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_set_style_radius(cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(cont, true, 0);
    lv_obj_set_scroll_dir(cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

    uint32_t i;
    for(i = 0; i < 20; i++) {
        lv_obj_t * btn = lv_btn_create(cont);
        lv_obj_remove_style_all(btn );
        lv_obj_add_style(btn , &g_style_btn, 0);
        lv_obj_set_size(btn, 100, 50);

        lv_obj_t * label = lv_label_create(btn);
        lv_label_set_text_fmt(label, "Button %", i);
        lv_obj_center(label);
    }

    /*Update the buttons position manually for first*/
    lv_event_send(cont, LV_EVENT_SCROLL, NULL);

    /*Be sure the fist button is in the middle*/
    lv_obj_scroll_to_view(lv_obj_get_child(cont, 0), LV_ANIM_OFF);

#elif (DRAW_DROPDOWN == 1)
    static const char * opts = "Apple\n"
                               "Banana\n"
                               "Orange\n"
                               "Melon";

    lv_obj_t * dd;
    dd = lv_dropdown_create(lv_scr_act());
    lv_obj_set_size(dd, 150, 100);
    lv_dropdown_set_options_static(dd, opts);
    lv_obj_align(dd, LV_ALIGN_TOP_MID, 0, 10);

    dd = lv_dropdown_create(lv_scr_act());
    lv_obj_set_size(dd, 150, 100);
    lv_dropdown_set_options_static(dd, opts);
    lv_dropdown_set_dir(dd, LV_DIR_BOTTOM);
    lv_dropdown_set_symbol(dd, LV_SYMBOL_UP);
    lv_obj_align(dd, LV_ALIGN_BOTTOM_MID, 0, -10);

    dd = lv_dropdown_create(lv_scr_act());
    lv_obj_set_size(dd, 150, 100);
    lv_dropdown_set_options_static(dd, opts);
    lv_dropdown_set_dir(dd, LV_DIR_RIGHT);
    lv_dropdown_set_symbol(dd, LV_SYMBOL_RIGHT);
    lv_obj_align(dd, LV_ALIGN_LEFT_MID, 10, 0);

    dd = lv_dropdown_create(lv_scr_act());
    lv_obj_set_size(dd, 150, 100);
    lv_dropdown_set_options_static(dd, opts);
    lv_dropdown_set_dir(dd, LV_DIR_LEFT);
    lv_dropdown_set_symbol(dd, LV_SYMBOL_LEFT);
    lv_obj_align(dd, LV_ALIGN_RIGHT_MID, -10, 0);

#elif (DRAW_IMG_RECOLOR == 1)

    /*Create 4 sliders to adjust RGB color and re-color intensity*/
    red_slider = create_slider(lv_palette_main(LV_PALETTE_RED));
    green_slider = create_slider(lv_palette_main(LV_PALETTE_GREEN));
    blue_slider = create_slider(lv_palette_main(LV_PALETTE_BLUE));
    intense_slider = create_slider(lv_palette_main(LV_PALETTE_GREY));

    lv_slider_set_value(red_slider, LV_OPA_20, LV_ANIM_OFF);
    lv_slider_set_value(green_slider, LV_OPA_90, LV_ANIM_OFF);
    lv_slider_set_value(blue_slider, LV_OPA_60, LV_ANIM_OFF);
    lv_slider_set_value(intense_slider, LV_OPA_50, LV_ANIM_OFF);

    lv_obj_align(red_slider, LV_ALIGN_LEFT_MID, 40, 0);
    lv_obj_align_to(green_slider, red_slider, LV_ALIGN_OUT_RIGHT_MID, 40, 0);
    lv_obj_align_to(blue_slider, green_slider, LV_ALIGN_OUT_RIGHT_MID, 40, 0);
    lv_obj_align_to(intense_slider, blue_slider, LV_ALIGN_OUT_RIGHT_MID, 40, 0);

    /*Now create the actual image*/
    img1 = lv_img_create(lv_scr_act());
    lv_img_set_src(img1, &img_cogwheel);
    lv_obj_align(img1, LV_ALIGN_RIGHT_MID, -40, 0);

    lv_event_send(intense_slider, LV_EVENT_VALUE_CHANGED, NULL);

#elif (DRAW_CHECKBOX == 1)
    lv_obj_t * label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "This is a checkbox");
    lv_obj_set_pos(label, 100, 10);

    lv_obj_t * sw = lv_switch_create(lv_scr_act());
    lv_obj_center(sw);
    lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw, sw_event_cb, LV_EVENT_VALUE_CHANGED, label);

#elif (DRAW_SWITCH == 1)
    lv_obj_set_flex_flow(lv_scr_act(), LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(lv_scr_act(), LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * sw;

    sw = lv_switch_create(lv_scr_act());
    lv_obj_set_size(sw, 100, 50);
    lv_obj_add_event_cb(sw, event_handler, LV_EVENT_ALL, NULL);

    sw = lv_switch_create(lv_scr_act());
    lv_obj_set_size(sw, 100, 50);
    lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw, event_handler, LV_EVENT_ALL, NULL);

    sw = lv_switch_create(lv_scr_act());
    lv_obj_set_size(sw, 100, 50);
    lv_obj_add_state(sw, LV_STATE_DISABLED);
    lv_obj_add_event_cb(sw, event_handler, LV_EVENT_ALL, NULL);

    sw = lv_switch_create(lv_scr_act());
    lv_obj_set_size(sw, 100, 50);
    lv_obj_add_state(sw, LV_STATE_CHECKED | LV_STATE_DISABLED);
    lv_obj_add_event_cb(sw, event_handler, LV_EVENT_ALL, NULL);

#elif (DRAW_TEXTAREA == 1)
    /*Create the password box*/
    lv_obj_t * pwd_ta = lv_textarea_create(lv_scr_act());
    lv_textarea_set_text(pwd_ta, "");
    lv_textarea_set_password_mode(pwd_ta, true);
    lv_textarea_set_one_line(pwd_ta, true);
    lv_obj_set_width(pwd_ta, lv_pct(40));
    lv_obj_set_pos(pwd_ta, 50, 100);
    lv_obj_add_event_cb(pwd_ta, ta_event_cb, LV_EVENT_ALL, NULL);

    /*Create a label and position it above the text box*/
    lv_obj_t * pwd_label = lv_label_create(lv_scr_act());
    lv_label_set_text(pwd_label, "Password:");
    lv_obj_align_to(pwd_label, pwd_ta, LV_ALIGN_OUT_TOP_LEFT, 0, 0);

    /*Create the one-line mode text area*/
    lv_obj_t * text_ta = lv_textarea_create(lv_scr_act());
    lv_textarea_set_one_line(text_ta, true);
    lv_textarea_set_password_mode(text_ta, false);
    lv_obj_set_width(text_ta, lv_pct(40));
    lv_obj_add_event_cb(text_ta, ta_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_pos(text_ta, 250, 100);

    /*Create a label and position it above the text box*/
    lv_obj_t * oneline_label = lv_label_create(lv_scr_act());
    lv_label_set_text(oneline_label, "Text:");
    lv_obj_align_to(oneline_label, text_ta, LV_ALIGN_OUT_TOP_LEFT, 0, 0);

    /*Create a keyboard*/
    kb = lv_keyboard_create(lv_scr_act());
    lv_obj_set_size(kb,  400, 150);
    lv_obj_align(kb, LV_ALIGN_CENTER, 0, 50);

    lv_keyboard_set_textarea(kb, pwd_ta); /*Focus it on one of the text areas to start*/

#elif (DRAW_EVENT == 1)
    lv_obj_t * btn = lv_btn_create(lv_scr_act());
    lv_obj_remove_style_all(btn );
    lv_obj_add_style(btn , &g_style_btn, 0);
    lv_obj_set_size(btn, 200, 100);
    lv_obj_center(btn);

    lv_obj_t * btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Click me!");
    lv_obj_center(btn_label);

    lv_obj_t * info_label = lv_label_create(lv_scr_act());
    lv_label_set_text(info_label, "The last button event:\nNone");
    lv_obj_set_pos(info_label, 100, 100);

    lv_obj_add_event_cb(btn, event_cb, LV_EVENT_ALL, info_label);

#elif (DRAW_BUTTON == 1)
    /*Init the style for the default state*/
    static lv_style_t style;
    lv_style_init(&style);

    lv_style_set_radius(&style, 3);

    lv_style_set_bg_opa(&style, LV_OPA_100);
    lv_style_set_bg_color(&style, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_bg_grad_color(&style, lv_palette_darken(LV_PALETTE_BLUE, 2));
    lv_style_set_bg_grad_dir(&style, LV_GRAD_DIR_VER);

    lv_style_set_border_opa(&style, LV_OPA_40);
    lv_style_set_border_width(&style, 2);
    lv_style_set_border_color(&style, lv_palette_main(LV_PALETTE_GREY));

    lv_style_set_shadow_width(&style, 8);
    lv_style_set_shadow_color(&style, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_shadow_ofs_y(&style, 8);

    lv_style_set_outline_opa(&style, LV_OPA_COVER);
    lv_style_set_outline_color(&style, lv_palette_main(LV_PALETTE_BLUE));

    lv_style_set_text_color(&style, lv_color_white());
    lv_style_set_pad_all(&style, 10);

    /*Init the pressed style*/
    static lv_style_t style_pr;
    lv_style_init(&style_pr);

    /*Add a large outline when pressed*/
    lv_style_set_outline_width(&style_pr, 30);
    lv_style_set_outline_opa(&style_pr, LV_OPA_TRANSP);

    lv_style_set_translate_y(&style_pr, 5);
    lv_style_set_shadow_ofs_y(&style_pr, 3);
    lv_style_set_bg_color(&style_pr, lv_palette_darken(LV_PALETTE_BLUE, 2));
    lv_style_set_bg_grad_color(&style_pr, lv_palette_darken(LV_PALETTE_BLUE, 4));

    /*Add a transition to the outline*/
    static lv_style_transition_dsc_t trans;
    static lv_style_prop_t props[] = {LV_STYLE_OUTLINE_WIDTH, LV_STYLE_OUTLINE_OPA, 0};
    lv_style_transition_dsc_init(&trans, props, lv_anim_path_linear, 300, 0, NULL);

    lv_style_set_transition(&style_pr, &trans);

    lv_obj_t * btn1 = lv_btn_create(lv_scr_act());
    lv_obj_remove_style_all(btn1);                          /*Remove the style coming from the theme*/
    lv_obj_add_style(btn1, &style, 0);
    lv_obj_add_style(btn1, &style_pr, LV_STATE_PRESSED);
    lv_obj_set_size(btn1, 200, 100);
    lv_obj_center(btn1);

    lv_obj_t * label = lv_label_create(btn1);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);

#elif (DRAW_BTNMATRIX == 1)
    static lv_style_t style_bg;
    lv_style_init(&style_bg);
    lv_style_set_pad_all(&style_bg, 0);
    lv_style_set_pad_gap(&style_bg, 0);
    lv_style_set_clip_corner(&style_bg, true);
    lv_style_set_radius(&style_bg, LV_RADIUS_CIRCLE);
    lv_style_set_border_width(&style_bg, 0);


    static lv_style_t style_btn;
    lv_style_init(&style_btn);
    lv_style_set_radius(&style_btn, 0);
    lv_style_set_border_width(&style_btn, 1);
    lv_style_set_border_opa(&style_btn, LV_OPA_50);
    lv_style_set_border_color(&style_btn, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_border_side(&style_btn, LV_BORDER_SIDE_INTERNAL);
    lv_style_set_radius(&style_btn, 0);

    static const char * map[] = {LV_SYMBOL_LEFT, "1", "2", "3", "4", "5", LV_SYMBOL_RIGHT, ""};

    lv_obj_t * btnm = lv_btnmatrix_create(lv_scr_act());
    lv_btnmatrix_set_map(btnm, map);
    lv_obj_add_style(btnm, &style_bg, 0);
    lv_obj_add_style(btnm, &style_btn, LV_PART_ITEMS);
    lv_obj_add_event_cb(btnm, event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_size(btnm, 250, 50);

    /*Allow selecting on one number at time*/
    lv_btnmatrix_set_btn_ctrl_all(btnm, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_btnmatrix_clear_btn_ctrl(btnm, 0, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_btnmatrix_clear_btn_ctrl(btnm, 6, LV_BTNMATRIX_CTRL_CHECKABLE);

    lv_btnmatrix_set_one_checked(btnm, true);
    lv_btnmatrix_set_btn_ctrl(btnm, 1, LV_BTNMATRIX_CTRL_CHECKED);

    lv_obj_center(btnm);

#elif (DRAW_IMGBTN == 1)

    /*Create a transition animation on width transformation and recolor.*/
    static lv_style_prop_t tr_prop[] = {LV_STYLE_TRANSFORM_WIDTH, LV_STYLE_IMG_RECOLOR_OPA, 0};
    static lv_style_transition_dsc_t tr;
    lv_style_transition_dsc_init(&tr, tr_prop, lv_anim_path_linear, 200, 0, NULL);

    static lv_style_t style_def;
    lv_style_init(&style_def);
    lv_style_set_text_color(&style_def, lv_color_white());
    lv_style_set_transition(&style_def, &tr);

    /*Darken the button when pressed and make it wider*/
    static lv_style_t style_pr;
    lv_style_init(&style_pr);
    lv_style_set_img_recolor_opa(&style_pr, LV_OPA_30);
    lv_style_set_img_recolor(&style_pr, lv_color_black());
    lv_style_set_transform_width(&style_pr, 20);

    /*Create an image button*/
    lv_obj_t * imgbtn1 = lv_imgbtn_create(lv_scr_act());
    lv_imgbtn_set_src(imgbtn1, LV_IMGBTN_STATE_RELEASED, &limgbtn, &mimgbtn, &rimgbtn);
    lv_obj_add_style(imgbtn1, &style_def, 0);
    lv_obj_add_style(imgbtn1, &style_pr, LV_STATE_PRESSED);
    lv_obj_set_width(imgbtn1, 250);
    lv_obj_center(imgbtn1);

    /*Create a label on the image button*/
    lv_obj_t * label = lv_label_create(imgbtn1);
    lv_label_set_text(label, "Button");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -4);

#elif (DRAW_FLEX == 1)
    /*Create a container with ROW flex direction*/
    lv_obj_t * cont_row = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont_row, 300, 200);
    lv_obj_align(cont_row, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW);

    /*Create a container with COLUMN flex direction*/
    lv_obj_t * cont_col = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont_col, 200, 150);
    lv_obj_align_to(cont_col, cont_row, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_obj_set_flex_flow(cont_col, LV_FLEX_FLOW_COLUMN);

    uint32_t i;
    for(i = 0; i < 10; i++)
    {
        lv_obj_t * obj;
        lv_obj_t * label;

        /*Add items to the row*/
        obj = lv_btn_create(cont_row);
        lv_obj_remove_style_all(obj);
        lv_obj_add_style(obj, &g_style_btn, 0);
        lv_obj_set_size(obj, 100, LV_PCT(100));

        label = lv_label_create(obj);
        lv_label_set_text_fmt(label, "Item: %u", i);
        lv_obj_center(label);

        /*Add items to the column*/
        obj = lv_btn_create(cont_col);
        lv_obj_remove_style_all(obj);
        lv_obj_add_style(obj, &g_style_btn, 0);
        lv_obj_set_size(obj, LV_PCT(100), LV_SIZE_CONTENT);

        label = lv_label_create(obj);
        lv_label_set_text_fmt(label, "Item: %"LV_PRIu32, i);
        lv_obj_center(label);
    }

#elif (DRAW_SNAPSHOT == 1)

    msg_switch_task();

#elif (DRAW_QRCODE == 1)
    lv_obj_t * qr = lv_qrcode_create(lv_scr_act(), 250, lv_color_hex(0), lv_color_hex(0xffffff));

    /*Set data*/
    const char * data = "https://ambiq.com/";
    lv_qrcode_update(qr, data, strlen(data));
    lv_obj_center(qr);

#elif (DRAW_GIF == 1)
    lv_obj_t * img;
    img = lv_gif_create(lv_scr_act());
    lv_gif_set_src(img, &bulb_gif);
    lv_obj_align(img, LV_ALIGN_CENTER, 100, 0);

    img = lv_gif_create(lv_scr_act());
    lv_gif_set_src(img, "E:bulb.gif");
    lv_obj_align(img, LV_ALIGN_CENTER, -100, 0);

#elif (DRAW_BMP == 1) || (DRAW_JPG == 1) || (DRAW_PNG == 1)
    lv_obj_t * img = lv_img_create(lv_scr_act());
#if (DRAW_BMP == 1)
#if LV_COLOR_DEPTH == 32
    lv_img_set_src(img, "E:example_32bit.bmp");
#elif LV_COLOR_DEPTH == 24
    lv_img_set_src(img, "E:example_24bit.bmp");
#elif LV_COLOR_DEPTH == 16
    lv_img_set_src(img, "E:example_16bit.bmp");
#endif
#elif (DRAW_JPG == 1)
    lv_img_set_src(img, "E:small_image.sjpg");
#elif (DRAW_PNG == 1)
    lv_img_set_src(img, &wink_png);
    lv_obj_align(img, LV_ALIGN_CENTER, 100, 0);

    img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, "E:wink.png");
#endif
    lv_obj_center(img);

#elif (DRAW_CANVAS == 1)
    /*Create a buffer for the canvas*/
    lv_color_t *cbuf = lv_mem_ssram_alloc(LV_CANVAS_BUF_SIZE_INDEXED_1BIT(100, 100) * sizeof(lv_color_t));

    /*Create a canvas and initialize its palette*/
    lv_obj_t * canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(canvas, cbuf, 100, 100, LV_IMG_CF_INDEXED_1BIT);
    lv_canvas_set_palette(canvas, 0, LV_COLOR_CHROMA_KEY);
    lv_canvas_set_palette(canvas, 1, lv_palette_main(LV_PALETTE_RED));
    lv_obj_align(canvas, LV_ALIGN_CENTER, 0, 0);

    /*Create colors with the indices of the palette*/
    lv_color_t c0;
    lv_color_t c1;

    c0.full = 0;
    c1.full = 1;

    /*Red background (There is no dedicated alpha channel in indexed images so LV_OPA_COVER is ignored)*/
    lv_canvas_fill_bg(canvas, c1, LV_OPA_COVER);

    /*Create hole on the canvas*/
    uint32_t x;
    uint32_t y;
    for(y = 10; y < 30; y++) {
        for(x = 5; x < 20; x++) {
            lv_canvas_set_px_color(canvas, x, y, c0);
        }
    }

#elif (DRAW_LED == 1)
    /*Create a LED and switch it OFF*/
    lv_obj_t * led1  = lv_led_create(lv_scr_act());
    lv_obj_align(led1, LV_ALIGN_CENTER, -120, 0);
    lv_led_off(led1);
    lv_obj_add_event_cb(led1, event_handler1, LV_EVENT_ALL, NULL);

    /*Copy the previous LED and set a brightness*/
    lv_obj_t * led2  = lv_led_create(lv_scr_act());
    lv_obj_align(led2, LV_ALIGN_CENTER, 0, 0);
    lv_led_set_brightness(led2, 150);
    lv_led_set_color(led2, lv_palette_main(g_ledcolor));
    lv_obj_add_event_cb(led2, event_handler2, LV_EVENT_ALL, NULL);

    /*Copy the previous LED and switch it ON*/
    lv_obj_t * led3  = lv_led_create(lv_scr_act());
    lv_obj_align(led3, LV_ALIGN_CENTER, 120, 0);
    lv_led_set_brightness(led3, LV_LED_BRIGHT_MIN);
    lv_led_on(led3);
    lv_obj_add_event_cb(led3, event_handler3, LV_EVENT_ALL, NULL);

#elif (DRAW_LINE == 1)
    /*Create an array for the points of the line*/
    static lv_point_t line_points[] = { {5, 5}, {70, 70}, {120, 10}, {180, 60}, {240, 10} };

    /*Create style*/
    static lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 8);
    lv_style_set_line_color(&style_line, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_line_rounded(&style_line, true);

    /*Create a line and apply the new style*/
    lv_obj_t * line1;
    line1 = lv_line_create(lv_scr_act());
    lv_line_set_points(line1, line_points, 5);     /*Set the points*/
    lv_obj_add_style(line1, &style_line, 0);
    lv_obj_center(line1);

#elif (DRAW_SLIDER == 1)
    /*Create a transition*/
    static const lv_style_prop_t props[] = {LV_STYLE_BG_COLOR, 0};
    static lv_style_transition_dsc_t transition_dsc;
    lv_style_transition_dsc_init(&transition_dsc, props, lv_anim_path_linear, 300, 0, NULL);

    static lv_style_t style_main;
    static lv_style_t style_indicator;
    static lv_style_t style_knob;
    static lv_style_t style_pressed_color;
    lv_style_init(&style_main);
    lv_style_set_bg_opa(&style_main, LV_OPA_COVER);
    lv_style_set_bg_color(&style_main, lv_color_hex3(0xbbb));
    lv_style_set_radius(&style_main, LV_RADIUS_CIRCLE);
    lv_style_set_pad_ver(&style_main, -2); /*Makes the indicator larger*/

    lv_style_init(&style_indicator);
    lv_style_set_bg_opa(&style_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indicator, lv_palette_main(LV_PALETTE_CYAN));
    lv_style_set_radius(&style_indicator, LV_RADIUS_CIRCLE);
    lv_style_set_transition(&style_indicator, &transition_dsc);

    lv_style_init(&style_knob);
    lv_style_set_bg_opa(&style_knob, LV_OPA_COVER);
    lv_style_set_bg_color(&style_knob, lv_palette_main(LV_PALETTE_CYAN));
    lv_style_set_border_color(&style_knob, lv_palette_darken(LV_PALETTE_CYAN, 3));
    lv_style_set_border_width(&style_knob, 2);
    lv_style_set_radius(&style_knob, LV_RADIUS_CIRCLE);
    lv_style_set_pad_all(&style_knob, 6); /*Makes the knob larger*/
    lv_style_set_transition(&style_knob, &transition_dsc);

    lv_style_init(&style_pressed_color);
    lv_style_set_bg_color(&style_pressed_color, lv_palette_darken(LV_PALETTE_CYAN, 2));

    /*Create a slider and add the style*/
    lv_obj_t * slider = lv_slider_create(lv_scr_act());
    lv_obj_remove_style_all(slider);        /*Remove the styles coming from the theme*/

    lv_obj_add_style(slider, &style_main, LV_PART_MAIN);
    lv_obj_add_style(slider, &style_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(slider, &style_pressed_color, LV_PART_INDICATOR | LV_STATE_PRESSED);
    lv_obj_add_style(slider, &style_knob, LV_PART_KNOB);
    lv_obj_add_style(slider, &style_pressed_color, LV_PART_KNOB | LV_STATE_PRESSED);
    lv_obj_set_width(slider, 250);

    lv_obj_center(slider);

#endif

    while (1)
    {
        lv_timer_handler();

        vTaskDelay(5);
    }
}

