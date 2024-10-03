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
#include "lvgl_dynamic_watch_demo.h"
#include "lv_ambiq_font_align.h"
//*****************************************************************************
//
// GUI task handle.
//
//*****************************************************************************
TaskHandle_t GuiTaskHandle;

//*****************************************************************************
//
// LVGL Objects
//
//*****************************************************************************
uint32_t SECOND_EVENT;
lv_obj_t *scr1;
lv_obj_t *scr2;

lv_obj_t* tile_to_be_redraw = NULL;
lv_obj_t* tile_dynamic_wallpaper = NULL;
lv_obj_t* image_dynamic_wallpaper = NULL;

//*****************************************************************************
//
// Texture.
//
//*****************************************************************************
typedef struct{
  char *watch_bg;
  char *hour_leftlight;
  char *hour_leftliaght_dot;
  char *hour_rightlight;
  char *hour_rightliaght_dot;
  char *hour_shadow;
  char *minute_leftlight;
  char *minute_leftlight_dot;
  char *minute_rightlight;
  char *minute_rightlight_dot;
  char *minute_shadow;
  char *second_leftlight;
  char *second_leftlight_dot;
  char *second_rightlight;
  char *second_rightlight_dot;
  char *second_shadow;
  char *week;
  char *month;
} material_name_t;

typedef struct{
    lv_img_dsc_t* img_watch_bg_psram;
    lv_img_dsc_t* img_hour_leftlight_psram;
    lv_img_dsc_t* img_hour_leftlight_dot_psram;
    lv_img_dsc_t* img_hour_rightlight_psram;
    lv_img_dsc_t* img_hour_rightlight_dot_psram;
    lv_img_dsc_t* img_hour_shadow_psram;
    lv_img_dsc_t* img_minute_leftlight_psram;
    lv_img_dsc_t* img_minute_leftlight_dot_psram;
    lv_img_dsc_t* img_minute_rightlight_psram;
    lv_img_dsc_t* img_minute_rightlight_dot_psram;
    lv_img_dsc_t* img_minute_shadow_psram;
    lv_img_dsc_t* img_second_leftlight_psram;
    lv_img_dsc_t* img_second_leftlight_dot_psram;
    lv_img_dsc_t* img_second_rightlight_psram;
    lv_img_dsc_t* img_second_rightlight_dot_psram;
    lv_img_dsc_t* img_second_shadow_psram;
    lv_img_dsc_t* img_week_psram;
    lv_img_dsc_t* img_month_psram;
    lv_img_dsc_t** img_gear_psram_array;
} clock_texture_t;

typedef struct
{
    lv_obj_t* second_hand;
    lv_obj_t* second_dot;
    lv_obj_t* second_hand_shadow;
    lv_obj_t* minute_hand;
    lv_obj_t* minute_dot;
    lv_obj_t* minute_hand_shadow;
    lv_obj_t* hour_hand;
    lv_obj_t* hour_dot;
    lv_obj_t* hour_hand_shadow;
    clock_texture_t* texture;
}clock_hands_t;

static clock_hands_t tile0_needle =
{
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

static clock_hands_t tile1_needle =
{
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

material_name_t watch_aod_material =
{
    .watch_bg           = "E:BG(8)_47b5b808-557a-4aca-9b24-219189f5337a.rgba565",

    .hour_shadow        = "E:H3_d8e21464-8832-461f-bab1-4a01d159347e.rgba4444",
    .minute_shadow      = "E:M3-1_c89970a5-9e90-4e9f-b474-3918bbb99577.rgba4444",
    .second_shadow      = NULL,

    .week               = "E:W4_48646473-e9dd-444c-ab52-cddb1bb27c76.rgba4444",
    .month              = "E:M11_7cafbf08-c5e8-49c6-84e1-61ee749d9021.rgba4444",

    .hour_leftlight         = "E:hand_aod_hour_left_5_71_22_196_11_161.bgra8888",
    .hour_leftliaght_dot    = "E:dot_aod_hour_left_0_220_32_24_16_12.bgra8888",
    .hour_rightlight        = "E:hand_aod_hour_right_5_71_22_196_11_161.bgra8888",
    .hour_rightliaght_dot   = "E:dot_aod_hour_right_0_220_32_24_16_12.bgra8888",

    .minute_leftlight       = "E:hand_aod_minute_left_5_21_22_246_11_211.bgra8888",
    .minute_leftlight_dot   = "E:dot_aod_minute_left_2_223_28_18_14_9.bgra8888",
    .minute_rightlight      = "E:hand_aod_minute_right_5_21_22_246_11_211.bgra8888",
    .minute_rightlight_dot  = "E:dot_aod_minute_right_2_223_28_18_14_9.bgra8888",

    .second_leftlight       = NULL,
    .second_leftlight_dot   = NULL,
    .second_rightlight      = NULL,
    .second_rightlight_dot  = NULL,

};

material_name_t watch_normal_material =
{
    .watch_bg           = "E:BG(1)_0f19350b-630f-4f88-ac5a-6181fcda9b79.rgba565",

    .hour_shadow        = "E:H3_7daf8f40-0c7b-4054-af40-fff72de59730.rgba4444",
    .minute_shadow      = "E:M3-1_c89970a5-9e90-4e9f-b474-3918bbb99577.rgba4444",
    .second_shadow      = "E:S3_a852ba51-23ae-45a4-aa9e-985c164e029c.rgba4444",

    .week               = "E:W4_f0b8bc48-84d7-4cf4-8915-5921eda6059d.rgba4444",
    .month              = "E:M11_7cafbf08-c5e8-49c6-84e1-61ee749d9021.rgba4444",

    .hour_leftlight         = "E:hand_noraml_hour_left_5_71_22_196_11_161.bgra8888",
    .hour_leftliaght_dot    = "E:dot_noraml_hour_left_0_220_32_24_16_12.bgra8888",
    .hour_rightlight        = "E:hand_noraml_hour_right_5_71_22_196_11_161.bgra8888",
    .hour_rightliaght_dot   = "E:dot_noraml_hour_right_0_220_32_24_16_12.bgra8888" ,

    .minute_leftlight       = "E:hand_noraml_minute_left_5_21_22_246_11_211.bgra8888",
    .minute_leftlight_dot   = "E:dot_noraml_minute_left_2_223_28_18_14_9.bgra8888" ,
    .minute_rightlight      = "E:hand_noraml_minute_right_5_21_22_246_11_211.bgra8888",
    .minute_rightlight_dot  = "E:dot_noraml_minute_right_2_223_28_18_14_9.bgra8888" ,

    .second_leftlight       = "E:hand_noraml_second_left_13_7_6_252_3_225.bgra8888",
    .second_leftlight_dot   = "E:dot_noraml_second_left_6_223_20_18_10_9.bgra8888"  ,
    .second_rightlight      = "E:hand_noraml_second_right_13_7_6_252_3_225.bgra8888",
    .second_rightlight_dot  = "E:dot_noraml_second_right_6_223_20_18_10_9.bgra8888" ,

};

lv_img_dsc_t* img_gear_psram[90];
lv_img_dsc_t img_gear_psram_arry_default[90];

lv_img_dsc_t img_gear_default ={
  .header.always_zero = 0,
  .header.w = 185,
  .header.h = 185,
  .data_size = 185 * 185 * 2,
  .header.cf = LV_IMG_CF_AMBIQ_RGBA4444,
};

lv_img_dsc_t img_normal_bg_psram = 
{
  .header.always_zero = 0,
  .header.w = 464,
  .header.h = 464,
  .header.cf = LV_IMG_CF_AMBIQ_RGB565,
  .data_size = 464*464*2,
};

lv_img_dsc_t img_normal_left_hour_psram = 
{
  .header.always_zero = 0,
  .header.w = 22,
  .header.h = 196,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 22*196*4,
};

lv_img_dsc_t img_normal_right_hour_psram =
{
  .header.always_zero = 0,
  .header.w = 22,
  .header.h = 196,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 22*196*4,
};

lv_img_dsc_t img_normal_left_hour_dot_psram = 
{
  .header.always_zero = 0,
  .header.w = 32,
  .header.h = 24,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 32*24*4,
};

lv_img_dsc_t img_normal_right_hour_dot_psram =
{
  .header.always_zero = 0,
  .header.w = 32,
  .header.h = 24,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 32*24*4,
};

lv_img_dsc_t img_normal_shadow_hour_psram =
{
  .header.always_zero = 0,
  .header.w = 32,
  .header.h = 464,
  .header.cf = LV_IMG_CF_AMBIQ_RGBA4444,
  .data_size = 32*464*2,
};

lv_img_dsc_t img_normal_left_minute_psram = 
{
  .header.always_zero = 0,
  .header.w = 22,
  .header.h = 246,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 22*246*4,
};

lv_img_dsc_t img_normal_right_minute_psram =
{
  .header.always_zero = 0,
  .header.w = 22,
  .header.h = 246,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 22*246*4,
};

lv_img_dsc_t img_normal_left_minute_dot_psram = 
{
  .header.always_zero = 0,
  .header.w = 28,
  .header.h = 18,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 28*18*4,
};

lv_img_dsc_t img_normal_right_minute_dot_psram =
{
  .header.always_zero = 0,
  .header.w = 28,
  .header.h = 18,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 28*18*4,
};

lv_img_dsc_t img_normal_shadow_minute_psram =
{
  .header.always_zero = 0,
  .header.w = 32,
  .header.h = 464,
  .header.cf = LV_IMG_CF_AMBIQ_RGBA4444,
  .data_size = 32*464*2,
};

lv_img_dsc_t img_normal_left_second_psram = 
{
  .header.always_zero = 0,
  .header.w = 6,
  .header.h = 252,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 6*252*4,
};

lv_img_dsc_t img_normal_right_second_psram =
{
  .header.always_zero = 0,
  .header.w = 6,
  .header.h = 252,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 6*252*4,
};

lv_img_dsc_t img_normal_left_second_dot_psram = 
{
  .header.always_zero = 0,
  .header.w = 20,
  .header.h = 18,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 20*18*4,
};

lv_img_dsc_t img_normal_right_second_dot_psram =
{
  .header.always_zero = 0,
  .header.w = 20,
  .header.h = 18,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 20*18*4,
};

lv_img_dsc_t img_normal_shadow_second_psram =
{
  .header.always_zero = 0,
  .header.w = 32,
  .header.h = 464,
  .header.cf = LV_IMG_CF_AMBIQ_RGBA4444,
  .data_size = 32*464*2,
};

lv_img_dsc_t img_normal_week_psram =
{
  .header.always_zero = 0,
  .header.w = 208,
  .header.h = 35,
  .header.cf = LV_IMG_CF_AMBIQ_RGBA4444,
  .data_size = 208*35*2,
};

lv_img_dsc_t img_normal_month_psram =
{
  .header.always_zero = 0,
  .header.w = 96,
  .header.h = 20,
  .header.cf = LV_IMG_CF_AMBIQ_RGBA4444,
  .data_size = 96*20*2,
};

lv_img_dsc_t img_aod_bg_psram = 
{
  .header.always_zero = 0,
  .header.w = 464,
  .header.h = 464,
  .header.cf = LV_IMG_CF_AMBIQ_RGB565,
  .data_size = 464*464*2,
};

lv_img_dsc_t img_aod_left_hour_psram = 
{
  .header.always_zero = 0,
  .header.w = 22,
  .header.h = 196,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 22*196*4,
};

lv_img_dsc_t img_aod_right_hour_psram =
{
  .header.always_zero = 0,
  .header.w = 22,
  .header.h = 196,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 22*196*4,
};

lv_img_dsc_t img_aod_left_hour_dot_psram = 
{
  .header.always_zero = 0,
  .header.w = 32,
  .header.h = 24,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 32*24*4,
};

lv_img_dsc_t img_aod_right_hour_dot_psram =
{
  .header.always_zero = 0,
  .header.w = 32,
  .header.h = 24,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 32*24*4,
};

lv_img_dsc_t img_aod_shadow_hour_psram =
{
  .header.always_zero = 0,
  .header.w = 32,
  .header.h = 464,
  .header.cf = LV_IMG_CF_AMBIQ_RGBA4444,
  .data_size = 32*464*2,
};

lv_img_dsc_t img_aod_left_minute_psram = 
{
  .header.always_zero = 0,
  .header.w = 22,
  .header.h = 246,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 22*246*4,
};

lv_img_dsc_t img_aod_right_minute_psram =
{
  .header.always_zero = 0,
  .header.w = 22,
  .header.h = 246,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 22*246*4,
};

lv_img_dsc_t img_aod_left_minute_dot_psram = 
{
  .header.always_zero = 0,
  .header.w = 28,
  .header.h = 18,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 28*18*4,
};

lv_img_dsc_t img_aod_right_minute_dot_psram =
{
  .header.always_zero = 0,
  .header.w = 28,
  .header.h = 18,
  .header.cf = LV_IMG_CF_AMBIQ_BGRA8888,
  .data_size = 28*18*4,
};

lv_img_dsc_t img_aod_shadow_minute_psram =
{
  .header.always_zero = 0,
  .header.w = 32,
  .header.h = 464,
  .header.cf = LV_IMG_CF_AMBIQ_RGBA4444,
  .data_size = 32*464*2,
};

lv_img_dsc_t img_aod_week_psram =
{
  .header.always_zero = 0,
  .header.w = 208,
  .header.h = 35,
  .header.cf = LV_IMG_CF_AMBIQ_RGBA4444,
  .data_size = 208*35*2,
};

lv_img_dsc_t img_aod_month_psram =
{
  .header.always_zero = 0,
  .header.w = 96,
  .header.h = 20,
  .header.cf = LV_IMG_CF_AMBIQ_RGBA4444,
  .data_size = 96*20*2,
};

lv_img_dsc_t img_wallpaper_psram = {
  .header.always_zero = 0,
  .header.w = 456,
  .header.h = 456,
  .data_size = 456 * 456 * 6 / 8,
  .header.cf = LV_IMG_CF_AMBIQ_TSC6,
};

lv_img_dsc_t img_arc_psram = {
  .header.always_zero = 0,
  .header.w = 464,
  .header.h = 464,
  .data_size = 464 * 464 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_RGB565,
};

lv_img_dsc_t img_watch0_psram = {
  .header.always_zero = 0,
  .header.w = 250,
  .header.h = 250,
  .data_size = 250 * 250 * LV_IMG_PX_SIZE_ALPHA_BYTE,
  .header.cf = LV_IMG_CF_AMBIQ_RGB565,
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
  .header.cf = LV_IMG_CF_AMBIQ_RGB565,
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

clock_texture_t watch_default_cfg = 
{
    .img_watch_bg_psram              =    &img_normal_bg_psram,       
    .img_hour_leftlight_psram        =    &img_normal_left_hour_psram,         
    .img_hour_rightlight_psram       =    &img_normal_right_hour_psram,
    .img_hour_leftlight_dot_psram    =    &img_normal_left_hour_dot_psram,         
    .img_hour_rightlight_dot_psram   =    &img_normal_right_hour_dot_psram,    
    .img_hour_shadow_psram           =    &img_normal_shadow_hour_psram,           
    .img_minute_leftlight_psram      =    &img_normal_left_minute_psram ,     
    .img_minute_rightlight_psram     =    &img_normal_right_minute_psram ,
    .img_minute_leftlight_dot_psram  =    &img_normal_left_minute_dot_psram ,     
    .img_minute_rightlight_dot_psram =    &img_normal_right_minute_dot_psram ,      
    .img_minute_shadow_psram         =    &img_normal_shadow_minute_psram,         
    .img_second_leftlight_psram      =    &img_normal_left_second_psram ,     
    .img_second_rightlight_psram     =    &img_normal_right_second_psram, 
    .img_second_leftlight_dot_psram  =    &img_normal_left_second_dot_psram ,     
    .img_second_rightlight_dot_psram =    &img_normal_right_second_dot_psram,     
    .img_second_shadow_psram         =    &img_normal_shadow_second_psram ,        
    .img_week_psram                  =    &img_normal_week_psram  ,
    .img_month_psram                 =    &img_normal_month_psram ,
    .img_gear_psram_array            =    (lv_img_dsc_t**)(&img_gear_psram),
};

clock_texture_t watch_aod_cfg = 
{
    .img_watch_bg_psram              =    &img_aod_bg_psram,       
    .img_hour_leftlight_psram        =    &img_aod_left_hour_psram,         
    .img_hour_rightlight_psram       =    &img_aod_right_hour_psram,
    .img_hour_leftlight_dot_psram    =    &img_aod_left_hour_dot_psram,         
    .img_hour_rightlight_dot_psram   =    &img_aod_right_hour_dot_psram,    
    .img_hour_shadow_psram           =    &img_aod_shadow_hour_psram,           
    .img_minute_leftlight_psram      =    &img_aod_left_minute_psram ,     
    .img_minute_rightlight_psram     =    &img_aod_right_minute_psram ,
    .img_minute_leftlight_dot_psram  =    &img_aod_left_minute_dot_psram ,     
    .img_minute_rightlight_dot_psram =    &img_aod_right_minute_dot_psram ,      
    .img_minute_shadow_psram         =    &img_aod_shadow_minute_psram,         
    .img_second_leftlight_psram      =    NULL,
    .img_second_rightlight_psram     =    NULL,
    .img_second_leftlight_dot_psram  =    NULL,    
    .img_second_rightlight_dot_psram =    NULL,    
    .img_second_shadow_psram         =    NULL,     
    .img_week_psram                  =    &img_aod_week_psram  ,                
    .img_month_psram                 =    &img_aod_month_psram  , 
    .img_gear_psram_array            =    NULL,              
};

char* gear1_file[90] = {
"E:one_0_9b58c33e-de3b-4b07-806e-aa9be779d667.rgba4444",
"E:one_1_f6be43f9-7264-4193-8c0c-78fb4fed1c50.rgba4444",
"E:one_2_bfa79087-ac8d-4648-acaf-6b6b389c137e.rgba4444",
"E:one_3_fa4d9dcd-18f6-4ab4-989f-f0b841a2a22f.rgba4444",
"E:one_4_5abeb3d8-42b8-46bf-9fd8-bda23b5969a8.rgba4444",
"E:one_5_4209e0c7-b976-4ad9-bb2f-e0b0d55cc05a.rgba4444",
"E:one_6_09123de5-b0c1-4399-8ecc-02411c66b900.rgba4444",
"E:one_7_2a7ba382-a207-4a5a-9995-02433f719a69.rgba4444",
"E:one_8_0063cca6-e23c-4395-8db2-8f97325f38b8.rgba4444",
"E:one_9_9279a219-e38d-4174-8da5-6f6baa3f0938.rgba4444",
"E:one_10_9965dc32-6b25-4104-b4c9-00777322f62e.rgba4444",
"E:one_11_d86a088d-b69d-496c-a57e-ffd6c401f4ad.rgba4444",
"E:one_12_d51f44c6-efc5-4622-9ee1-d1be2447bf16.rgba4444",
"E:one_13_1d071713-c91c-415b-9daa-4000669e691d.rgba4444",
"E:one_14_fe8633da-1400-4f4f-bfeb-365961bd1067.rgba4444",
"E:one_15_65669921-70e2-47c8-9cd7-b6eb9f727e91.rgba4444",
"E:one_16_70920491-8001-44f6-bc2f-df66ba62f1a3.rgba4444",
"E:one_17_c78b26a1-9291-4f99-84a6-15f64503c7d1.rgba4444",
"E:one_18_a850ac1d-084f-40ad-bd9b-46afff0d902a.rgba4444",
"E:one_19_ebaf5a23-f9fe-4dda-b2e8-e3ffc365022e.rgba4444",
"E:one_20_29b5e293-e79c-4a07-9d38-c78105cd537f.rgba4444",
"E:one_21_c83fe7e2-5246-47ea-a99f-450aff573325.rgba4444",
"E:one_22_403faa06-7d58-4436-aa2a-012dc76b84fc.rgba4444",
"E:one_23_90c408c8-3f2b-4374-92d7-7993d2c49283.rgba4444",
"E:one_24_ee979248-357d-4a38-a7b6-6d498ab81758.rgba4444",
"E:one_25_66f1dca8-5691-4f7b-86c2-5651cf471a02.rgba4444",
"E:one_26_bd679c76-bd71-4e78-9997-3c197b89f67a.rgba4444",
"E:one_27_09045199-01de-4e90-9c80-1779106a37f9.rgba4444",
"E:one_28_9f8ee08d-306f-4e80-853a-618a4f6cfc45.rgba4444",
"E:one_29_8d5e0c2f-4721-41c8-a625-57fc20f98586.rgba4444",
"E:one_30_29d92e70-db7a-4730-8853-822241bd542f.rgba4444",
"E:one_31_3eb24949-636d-42f5-8930-35f1e916a710.rgba4444",
"E:one_32_86b5aeeb-b65a-4eed-9ac0-4b1d300e5d82.rgba4444",
"E:one_33_c85e0045-5f86-43f9-9d41-112645e7f35c.rgba4444",
"E:one_34_2a82ea4c-7b93-4cdc-8f6d-4ed658511741.rgba4444",
"E:one_35_5f92c560-f021-458e-8f24-764c69031bb0.rgba4444",
"E:one_36_cc0e3664-3f93-4030-988b-79e8531ab525.rgba4444",
"E:one_37_b4a16a6f-e698-43e3-b9aa-44bf83ecd7f0.rgba4444",
"E:one_38_55a38623-c52a-4222-8a2f-b0f2291e6d43.rgba4444",
"E:one_39_164be0f6-7886-4b7f-a4ba-6ee78c8e45a8.rgba4444",
"E:one_40_d2d70d3c-408c-4a36-817e-9b0d27089ac8.rgba4444",
"E:one_41_1330fcf7-e74e-4e79-8e44-25857f2c244e.rgba4444",
"E:one_42_14e84994-9d25-457e-aaa1-db317277b219.rgba4444",
"E:one_43_4e147593-b152-4060-81ec-e4ef1c30e14e.rgba4444",
"E:one_44_a4cd4459-15d0-4e4b-9cec-91db946daeb9.rgba4444",
"E:one_45_b974fad2-4cec-4194-b55e-1989a8d14739.rgba4444",
"E:one_46_b67aa438-03e9-43d8-989e-46c85f69c3c4.rgba4444",
"E:one_47_4995ca11-9454-4b29-b31d-5bb29597887e.rgba4444",
"E:one_48_f41c1a36-2352-4c24-a63b-50a208284da8.rgba4444",
"E:one_49_81b1fc51-e1b1-406d-87d7-524188bccf25.rgba4444",
"E:one_50_aa70963c-4da1-4d97-9472-233c2edefde0.rgba4444",
"E:one_51_07f67090-d8a1-4998-91f8-3aada9fcb4e4.rgba4444",
"E:one_52_edafaaea-be35-42ef-b620-96cdca87f79f.rgba4444",
"E:one_53_8d726bf2-fb30-4b96-a4ee-45aae254c68d.rgba4444",
"E:one_54_b1a51475-12ba-4777-8a87-9ac1535e211b.rgba4444",
"E:one_55_bab07296-081e-4cb7-b66c-dc5817acd50e.rgba4444",
"E:one_56_a6782882-4e1b-4ecd-888c-26f2a402c8bc.rgba4444",
"E:one_57_8e082ddc-4106-4a5e-bc31-956acecaf559.rgba4444",
"E:one_58_ad0d4cdd-f0ad-48cd-bd88-5e9da941e1c3.rgba4444",
"E:one_59_ef7aa3ef-b9ea-43b0-a5aa-8d9bad71c259.rgba4444",
"E:one_60_508635c5-c3f0-4e1b-9bff-c0280829ac18.rgba4444",
"E:one_61_2b0cf8d3-e787-4df7-a447-6afacaf0cd97.rgba4444",
"E:one_62_73d36c69-4ffc-4b93-af4b-d3749a616a2e.rgba4444",
"E:one_63_1a391c58-b22b-45ee-bcaf-b807da9c972e.rgba4444",
"E:one_64_8a97a806-bb75-4d76-aa95-da769bc4cf56.rgba4444",
"E:one_65_29ae8d05-6133-491b-84d5-4bc87bced2e3.rgba4444",
"E:one_66_ac0f3014-1e40-4b8c-b2a9-6a396e84ae8c.rgba4444",
"E:one_67_a71d2fc1-4f88-47d8-82e7-39e327f3d4bc.rgba4444",
"E:one_68_d767facf-f9b6-42f4-945d-ab4aaaab771d.rgba4444",
"E:one_69_a4f29be6-9e39-4db8-8fb4-cf59222652f5.rgba4444",
"E:one_70_7824a981-39da-4754-a881-ad0abe03d279.rgba4444",
"E:one_71_cb6ae407-13b2-4ff3-b675-1212923474e0.rgba4444",
"E:one_72_9299d37a-07c4-4390-b16d-ee461fe5bb96.rgba4444",
"E:one_73_2d999773-5134-41c8-bb09-ce59fee8e4b1.rgba4444",
"E:one_74_06694746-7854-4554-bd73-79f8796f49dc.rgba4444",
"E:one_75_6d9be718-da6d-41b0-88fe-b8e1f8ff73a5.rgba4444",
"E:one_76_8627e3c9-d469-46d0-8f83-eb14178766aa.rgba4444",
"E:one_77_d0080e67-af3b-498a-ac57-42fd0fa9b947.rgba4444",
"E:one_78_f8c770a5-c68d-4630-8cea-b6535ec38b99.rgba4444",
"E:one_79_27a74f20-949f-49a6-b744-db2710d89463.rgba4444",
"E:one_80_2741e7a3-672e-49b4-8564-10f41897d4fa.rgba4444",
"E:one_81_f454a55b-6a9d-4d1d-baa6-e946e6a4404a.rgba4444",
"E:one_82_9018694d-30f7-4add-a919-a6e21ac34df6.rgba4444",
"E:one_83_eb3c149f-45c0-4e3c-8426-ee2f0ce03ef1.rgba4444",
"E:one_84_77a9c176-5eb3-4df4-888c-74b047e9556a.rgba4444",
"E:one_85_d07b2d62-6d35-428a-a613-8e2f15e769cb.rgba4444",
"E:one_86_5a63e13f-264c-4d52-b66b-139a742c16f1.rgba4444",
"E:one_87_220dd438-7a52-496a-ab00-f6afa2ff13ff.rgba4444",
"E:one_88_6c2599e9-debe-49d5-a40a-f3857a77067b.rgba4444",
"E:one_89_c475929e-67a1-4a1a-b9d1-c8486f24b76f.rgba4444",
};

lv_font_t * ambiq_font_test;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

static lv_indev_drv_t indev_drv;

static void switch_in_screen_handler(lv_event_t* e);
static void switch_out_screen_handler(lv_event_t* e);
static void seconds_update_handler(lv_event_t* event);
static void main_watchface_init(lv_obj_t* base_obj, clock_texture_t* watch_img);
static void dynamic_wallpaper_timer_cb(lv_timer_t * timer);
static void watch_face_tile_change(lv_event_t e);
static void timer_cb(lv_timer_t* timer);

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
//*****************************************************************************
//
// Load texture and font data from MRAM to PSRAM
//
//*****************************************************************************
void
texture_load(void)
{
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
    if(res == LV_FS_RES_OK)
    {
         res = lv_fs_read(&f, buf, len, &read_cnt);
         if((res != LV_FS_RES_OK) || (read_cnt == 0))
         {
           am_util_stdio_printf("File Read Error!\n");
           return res;
         }

         res = lv_fs_close(&f);
         if(res != LV_FS_RES_OK)
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

static void load_texture_item(lv_img_dsc_t* dsc, const char* name)
{
    if(dsc && name)
    {
        LV_LOG_USER("Loading texture: %s\n", name);
        dsc->data = (const uint8_t *)lv_mem_external_alloc(dsc->data_size);
        LV_LOG_USER("location: %8X\n", dsc->data);
        load_emmc_file((char*)name,   (uint8_t*)dsc->data           ,  dsc->data_size           ); 
    }
}

static void load_texture_theme(clock_texture_t *theme,  material_name_t* file_name)
{
    load_texture_item(theme->img_watch_bg_psram             ,  file_name->watch_bg);
    load_texture_item(theme->img_hour_leftlight_psram       ,  file_name->hour_leftlight);
    load_texture_item(theme->img_hour_leftlight_dot_psram   ,  file_name->hour_leftliaght_dot); 
    load_texture_item(theme->img_hour_rightlight_psram      ,  file_name->hour_rightlight);
    load_texture_item(theme->img_hour_rightlight_dot_psram  ,  file_name->hour_rightliaght_dot); 
    load_texture_item(theme->img_hour_shadow_psram          ,  file_name->hour_shadow);
    load_texture_item(theme->img_minute_leftlight_psram     ,  file_name->minute_leftlight); 
    load_texture_item(theme->img_minute_leftlight_dot_psram ,  file_name->minute_leftlight_dot); 
    load_texture_item(theme->img_minute_rightlight_psram    ,  file_name->minute_rightlight); 
    load_texture_item(theme->img_minute_rightlight_dot_psram,  file_name->minute_rightlight_dot); 
    load_texture_item(theme->img_minute_shadow_psram        ,  file_name->minute_shadow); 
    load_texture_item(theme->img_second_leftlight_psram     ,  file_name->second_leftlight); 
    load_texture_item(theme->img_second_leftlight_dot_psram ,  file_name->second_leftlight_dot); 
    load_texture_item(theme->img_second_rightlight_psram    ,  file_name->second_rightlight); 
    load_texture_item(theme->img_second_rightlight_dot_psram,  file_name->second_rightlight_dot); 
    load_texture_item(theme->img_second_shadow_psram        ,  file_name->second_shadow); 
    load_texture_item(theme->img_week_psram                 ,  file_name->week); 
    load_texture_item(theme->img_month_psram                ,  file_name->month); 
}

static void load_texture_to_psram(void)
{
    img_wallpaper_psram.data = (const uint8_t *)lv_mem_external_alloc(img_wallpaper_psram.data_size);

    img_arc_psram.data = (const uint8_t *)lv_mem_external_alloc(img_arc_psram.data_size);
    img_weather_psram.data = (const uint8_t *)lv_mem_external_alloc(img_weather_psram.data_size);
    img_music_psram.data = (const uint8_t *)lv_mem_external_alloc(img_music_psram.data_size);

    img_watch0_psram.data = (const uint8_t *)lv_mem_external_alloc(img_watch0_psram.data_size);
    img_watch1_psram.data = (const uint8_t *)lv_mem_external_alloc(img_watch1_psram.data_size);
    img_watch2_psram.data = (const uint8_t *)lv_mem_external_alloc(img_watch2_psram.data_size);
    img_watch3_psram.data = (const uint8_t *)lv_mem_external_alloc(img_watch3_psram.data_size);
    img_watch4_psram.data = (const uint8_t *)lv_mem_external_alloc(img_watch4_psram.data_size);

    // Load texture from EMMC to PSRAM
    load_emmc_file("E:weather_454.argb8888", (void*)img_weather_psram.data, img_weather_psram.data_size);
    load_emmc_file("E:music_454.argb8888", (void*)img_music_psram.data, img_music_psram.data_size);

    load_emmc_file("E:merged_tsc6_file.bin", (void*)img_wallpaper_psram.data, img_wallpaper_psram.data_size);

    load_emmc_file("E:arc.rgba565", (void*)img_arc_psram.data, img_arc_psram.data_size);

    load_emmc_file("E:market-preview.rgba565", (void*)img_watch0_psram.data, img_watch0_psram.data_size);
    load_emmc_file("E:watch_250_1.argb8888", (void*)img_watch1_psram.data, img_watch1_psram.data_size);
    load_emmc_file("E:watch_250_2.argb8888", (void*)img_watch2_psram.data, img_watch2_psram.data_size);
    load_emmc_file("E:watch_250_3.argb8888", (void*)img_watch3_psram.data, img_watch3_psram.data_size);
    load_emmc_file("E:aod-preview.rgba565", (void*)img_watch4_psram.data, img_watch4_psram.data_size);

    // Load font file from EMMC
    ambiq_font_test = lv_font_load("E:ambiq_font_test.bin");
}

void load_texture_gear(lv_img_dsc_t** gear_img, char** file)
{
    uint32_t i = 0;
    for(;i<90;i++)
    {
        gear_img[i]->data = (const uint8_t *)lv_mem_external_alloc(gear_img[i]->data_size);
        LV_LOG_USER("gear_img: %8X", gear_img[i]->data);
        load_emmc_file(file[i], (uint8_t *)(gear_img[i]->data), gear_img[i]->data_size);
    }
}

//*****************************************************************************
//
// LVGL application functions
//
//*****************************************************************************

void gear_img_default_init(lv_img_dsc_t *gear_img)
{
    uint32_t i;
    for(i = 0; i < 90; i++)
    {
      gear_img[i] = img_gear_default;
    }
}

void gear_img_init(lv_img_dsc_t **gear_img, lv_img_dsc_t *gear_img_default)
{
    uint32_t i;
    for(i = 0; i < 90; i++)
    {
      gear_img[i] = &gear_img_default[i];
    }
}

static void main_watchface_init(lv_obj_t* base_obj, clock_texture_t* watch_img)
{
    lv_obj_set_scrollbar_mode(base_obj, LV_SCROLLBAR_MODE_OFF);

    /*Now create the actual image*/
    lv_obj_t* watch_bg = lv_img_create(base_obj);
    lv_img_set_src(watch_bg, watch_img->img_watch_bg_psram);
    lv_obj_align(watch_bg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_scrollbar_mode(watch_bg, LV_SCROLLBAR_MODE_OFF);

    if(watch_img->img_gear_psram_array)
    {
        lv_obj_t* lv_gear = lv_animimg_create(watch_bg);
        lv_obj_align(lv_gear, LV_ALIGN_CENTER, 0, 118);
        lv_animimg_set_src(lv_gear, (lv_img_dsc_t **)watch_img->img_gear_psram_array, 90);
        lv_animimg_set_duration(lv_gear, 9000);
        lv_animimg_set_repeat_count(lv_gear, LV_ANIM_REPEAT_INFINITE);
        lv_animimg_start(lv_gear);
    }

    lv_obj_t* lv_week = lv_img_create(watch_bg);
    lv_img_set_src(lv_week, watch_img->img_week_psram);
    lv_obj_set_pos(lv_week,130,125);

    lv_obj_t* lv_month = lv_img_create(watch_bg);
    lv_img_set_src(lv_month, watch_img->img_month_psram);
    lv_obj_set_pos(lv_month,186,157);

    lv_obj_t* lv_hour_shadow = lv_img_create(watch_bg);
    lv_img_set_src(lv_hour_shadow, watch_img->img_hour_shadow_psram);
    lv_obj_align(lv_hour_shadow, LV_ALIGN_CENTER, 0, 8);
    lv_img_set_pivot(lv_hour_shadow, 16, 232);
    tile0_needle.hour_hand_shadow =  lv_hour_shadow;

    lv_obj_t* lv_hour_dot = lv_img_create(watch_bg);
    lv_img_set_src(lv_hour_dot, watch_img->img_hour_leftlight_dot_psram);
    lv_obj_align(lv_hour_dot, LV_ALIGN_CENTER, 0, 0);
    lv_img_set_pivot(lv_hour_dot, 16, 12);
    tile0_needle.hour_dot =  lv_hour_dot;

    lv_obj_t* lv_hour = lv_img_create(watch_bg);
    lv_img_set_src(lv_hour, watch_img->img_hour_leftlight_psram);
    lv_obj_align(lv_hour, LV_ALIGN_CENTER, 0, -63);
    lv_img_set_pivot(lv_hour, 11, 161);
    tile0_needle.hour_hand =  lv_hour;

    lv_obj_t* lv_minute_shadow = lv_img_create(watch_bg);
    lv_img_set_src(lv_minute_shadow, watch_img->img_minute_shadow_psram);
    lv_obj_align(lv_minute_shadow, LV_ALIGN_CENTER, 0, 9);
    lv_img_set_pivot(lv_minute_shadow, 16, 232);
    tile0_needle.minute_hand_shadow =  lv_minute_shadow;

    lv_obj_t* lv_minute_dot = lv_img_create(watch_bg);
    lv_img_set_src(lv_minute_dot, watch_img->img_minute_leftlight_dot_psram);
    lv_obj_align(lv_minute_dot,  LV_ALIGN_CENTER, 0, 0);
    lv_img_set_pivot(lv_minute_dot, 14, 9);
    tile0_needle.minute_dot =  lv_minute_dot;

    lv_obj_t* lv_minute = lv_img_create(watch_bg);
    lv_img_set_src(lv_minute, watch_img->img_minute_leftlight_psram);
    lv_obj_align(lv_minute,  LV_ALIGN_CENTER, 0, -88);
    lv_img_set_pivot(lv_minute, 11, 211);
    tile0_needle.minute_hand =  lv_minute;

    if(watch_img->img_second_shadow_psram)
    {
        lv_obj_t* lv_second_shadow = lv_img_create(watch_bg);
        lv_img_set_src(lv_second_shadow, watch_img->img_second_shadow_psram);
        lv_obj_align(lv_second_shadow, LV_ALIGN_CENTER, 0, 7);
        lv_img_set_pivot(lv_second_shadow, 16, 232);
        tile0_needle.second_hand_shadow =  lv_second_shadow; 
    }
    else
    {
        tile0_needle.second_hand_shadow =  NULL;
    }

    if(watch_img->img_second_leftlight_dot_psram)
    {
        lv_obj_t* lv_second_dot = lv_img_create(watch_bg);
        lv_img_set_src(lv_second_dot, watch_img->img_second_leftlight_dot_psram);
        lv_obj_align(lv_second_dot, LV_ALIGN_CENTER, 0, 0);
        lv_img_set_pivot(lv_second_dot, 10, 9);
        tile0_needle.second_dot =  lv_second_dot;
    }
    else
    {
        tile0_needle.second_dot =  NULL;
    }

    if(watch_img->img_second_leftlight_psram)
    {
        lv_obj_t* lv_second = lv_img_create(watch_bg);
        lv_img_set_src(lv_second, watch_img->img_second_leftlight_psram);
        lv_obj_align(lv_second, LV_ALIGN_CENTER, 0, -99);
        lv_img_set_pivot(lv_second, 3, 225);
        tile0_needle.second_hand =  lv_second;
    }
    else
    {
        tile0_needle.second_hand =  NULL;
    }

    tile0_needle.texture =  watch_img;
}

static void dynamic_watchface_init(lv_obj_t* base_obj)
{
    lv_obj_t* dynamic_wallpaper = lv_img_create(base_obj);
    lv_img_set_src(dynamic_wallpaper, &img_wallpaper_psram);
    lv_obj_align(dynamic_wallpaper, LV_ALIGN_CENTER, 0, 0);
    tile_dynamic_wallpaper = base_obj;
    image_dynamic_wallpaper = dynamic_wallpaper;
}

static void switch_watch_face_tileview(lv_obj_t* base_obj)
{
    lv_obj_t * tv_face = lv_tileview_create(base_obj);
    lv_obj_set_style_bg_color(tv_face, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(tv_face, LV_SCROLLBAR_MODE_OFF);
    lv_obj_t * tile_face0 = lv_tileview_add_tile(tv_face, 0, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    lv_obj_t * tile_face1 = lv_tileview_add_tile(tv_face, 1, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    lv_obj_t * tile_face2 = lv_tileview_add_tile(tv_face, 2, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    lv_obj_t * tile_face3 = lv_tileview_add_tile(tv_face, 3, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    lv_obj_t * tile_face4 = lv_tileview_add_tile(tv_face, 4, 0, LV_DIR_LEFT | LV_DIR_RIGHT);

    lv_obj_t * watchface0 = lv_img_create(tile_face0);
    lv_img_set_src(watchface0, &img_watch0_psram);
    lv_obj_align(watchface0, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(tile_face0, switch_in_screen_handler, LV_EVENT_CLICKED, (void*)&watch_default_cfg);

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
    lv_obj_add_event_cb(tile_face4, switch_in_screen_handler, LV_EVENT_CLICKED, (void*)&watch_aod_cfg);

    lv_obj_set_tile(tv_face, tile_face0, LV_ANIM_OFF);

}

void switch_in_screen_handler(lv_event_t* e)
{
    static clock_texture_t* current_selection = &watch_default_cfg;
    clock_texture_t* param = (clock_texture_t*)lv_event_get_user_data(e);

    if(param != current_selection)
    {
        lv_obj_clean(tile_to_be_redraw);
        main_watchface_init(tile_to_be_redraw, param);
        current_selection = param;
    }

    lv_scr_load(scr1);
}

void switch_out_screen_handler(lv_event_t* e)
{
    lv_scr_load(scr2);
}

void dynamic_wall_paper_start_stop(lv_event_t* e)
{

    lv_obj_t* tv = lv_event_get_target(e);
    lv_timer_t* timer = (lv_timer_t*)lv_event_get_user_data(e);
    if(lv_tileview_get_tile_act(tv) == tile_dynamic_wallpaper)
    {
        lv_timer_resume(timer); 
    }
    else
    {
        lv_timer_pause(timer); 
    }

}

void main_watch_face_tileview(lv_obj_t* base_obj)
{
    // Create tileview for watch demo
    lv_obj_t * tv = lv_tileview_create(base_obj);
    lv_obj_set_style_bg_color(tv, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(tv, LV_SCROLLBAR_MODE_OFF);
    lv_obj_t * tile0 = lv_tileview_add_tile(tv, 0, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    lv_obj_t * tile1 = lv_tileview_add_tile(tv, 1, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    lv_obj_t * tile2 = lv_tileview_add_tile(tv, 2, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    lv_obj_t * tile3 = lv_tileview_add_tile(tv, 3, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    lv_obj_t * tile4 = lv_tileview_add_tile(tv, 4, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
    lv_obj_t * tile5 = lv_tileview_add_tile(tv, 5, 0, LV_DIR_LEFT | LV_DIR_RIGHT);

    main_watchface_init(tile0, &watch_default_cfg);
    lv_obj_add_event_cb(tile0, seconds_update_handler, SECOND_EVENT, (void*)&tile0_needle);
    lv_obj_add_event_cb(tile0, switch_out_screen_handler, LV_EVENT_LONG_PRESSED, (void*)tile0);
    tile_to_be_redraw = tile0;

    dynamic_watchface_init(tile1);
    //lv_obj_add_event_cb(tile1, seconds_update_handler, SECOND_EVENT, (void*)&tile1_needle);

    //register timer for dynamic wallpaper, it should start with the operation of switch in tile1
    //Create update timer
    static lv_fs_file_t f;
    lv_fs_res_t res;
    res = lv_fs_open(&f, "E:merged_tsc6_file.bin", LV_FS_MODE_RD);
    if(res != LV_FS_RES_OK)
    {
        am_util_stdio_printf("File Read Error!\n");
    }
    lv_timer_t * timer_dynamic = lv_timer_create(dynamic_wallpaper_timer_cb, 10, &f);
    lv_timer_set_repeat_count(timer_dynamic, -1);
    lv_timer_pause(timer_dynamic);
    lv_obj_add_event_cb(tv, dynamic_wall_paper_start_stop, LV_EVENT_VALUE_CHANGED, timer_dynamic);

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

    c_menu_init(tile5);

    lv_obj_set_tile(tv, tile0, LV_ANIM_OFF);
    lv_obj_add_event_cb(tv, (lv_event_cb_t)watch_face_tile_change, LV_EVENT_VALUE_CHANGED, NULL);
}

void send_second_update(bool force)
{
    static uint32_t last_sec = 0;
    uint32_t current_sec = 0;

    current_sec = lv_tick_get()/1000;
    LV_LOG_USER("current seconds:%d", current_sec);

    lv_obj_t* act_scr = lv_scr_act();

    if((force == true) || (last_sec == 0) || (current_sec != last_sec))
    {    
        lv_event_send(act_scr, SECOND_EVENT, (void*)current_sec);        
    }

    last_sec = current_sec;
}

void watch_face_tile_change(lv_event_t e)
{
    send_second_update(true);
}

void timer_cb(lv_timer_t* timer)
{
    send_second_update(false);
}

void dynamic_wallpaper_timer_cb(lv_timer_t * timer)
{
    static uint32_t i=0;
    lv_fs_res_t res;
    uint32_t read_cnt;
    lv_fs_file_t* fp = (lv_fs_file_t*)timer->user_data;
    if( i < WALLPAPER_FRAME_NUM - 1 )
    {
        i++;
    }
    else
    {
        i = 0;
    }

    res = lv_fs_seek(fp, i * img_wallpaper_psram.data_size, LV_FS_SEEK_SET);
    if(res != LV_FS_RES_OK)
    {
        am_util_stdio_printf("Fail to move file pointer!\n");
    }

    res = lv_fs_read(fp, (uint8_t *)img_wallpaper_psram.data, img_wallpaper_psram.data_size, &read_cnt);
    if((res != LV_FS_RES_OK) || (read_cnt == 0))
    {
        am_util_stdio_printf("File Read Error!\n");
    }
    lv_img_set_src(image_dynamic_wallpaper, &img_wallpaper_psram);
}

void seconds_update_handler(lv_event_t* event)
{
    uint32_t seconds = (uint32_t)lv_event_get_param(event);
    clock_hands_t * needles = (clock_hands_t *)lv_event_get_user_data(event);

    uint32_t needle_second = seconds%60;
    uint32_t needle_minute = (seconds/60)%60;
    uint32_t needle_hour = (seconds/60/60)%12;

    uint16_t angle;

    angle = needle_hour * 300 + needle_minute * 5;
    if(needles->hour_hand) lv_img_set_angle(needles->hour_hand, angle);
    if(needles->hour_hand_shadow) lv_img_set_angle(needles->hour_hand_shadow, angle);
    if(needles->hour_dot) lv_img_set_angle(needles->hour_dot, angle);

    clock_texture_t* textures = needles->texture;

    if(needles->hour_hand)
    {    
        lv_img_dsc_t* current_hour_texture = (lv_img_dsc_t* )lv_img_get_src(needles->hour_hand);
        if((angle > 1800) && (current_hour_texture == textures->img_hour_leftlight_psram))
        {
            lv_img_set_src(needles->hour_hand, textures->img_hour_rightlight_psram);
            lv_img_set_src(needles->hour_dot, textures->img_hour_rightlight_dot_psram);
            lv_img_set_pivot(needles->hour_hand, 11, 161);
            lv_img_set_pivot(needles->hour_dot, 16, 12);
        }
        else if((angle <= 1800) && (current_hour_texture == textures->img_hour_rightlight_psram))
        {
            lv_img_set_src(needles->hour_hand, textures->img_hour_leftlight_psram);
            lv_img_set_src(needles->hour_dot, textures->img_hour_leftlight_dot_psram);
            lv_img_set_pivot(needles->hour_hand, 11, 161);
            lv_img_set_pivot(needles->hour_dot, 16, 12);
        }
    }

    angle = needle_minute * 60 + needle_second;
    if(needles->minute_hand) lv_img_set_angle(needles->minute_hand, angle);
    if(needles->minute_hand_shadow) lv_img_set_angle(needles->minute_hand_shadow, angle);
    if(needles->minute_dot) lv_img_set_angle(needles->minute_dot, angle);

    if(needles->minute_hand)
    {
        lv_img_dsc_t* current_minute_texture = (lv_img_dsc_t* )lv_img_get_src(needles->minute_hand);
        if((angle > 1800) && (current_minute_texture == textures->img_minute_leftlight_psram))
        {
            lv_img_set_src(needles->minute_hand, textures->img_minute_rightlight_psram);
            lv_img_set_src(needles->minute_dot, textures->img_minute_rightlight_dot_psram);
            lv_img_set_pivot(needles->minute_hand, 11, 211);
            lv_img_set_pivot(needles->minute_dot, 14, 9);
        }
        else if((angle <= 1800) && (current_minute_texture == textures->img_minute_rightlight_psram))
        {
            lv_img_set_src(needles->minute_hand, textures->img_minute_leftlight_psram);
            lv_img_set_src(needles->minute_dot, textures->img_minute_leftlight_dot_psram);
            lv_img_set_pivot(needles->minute_hand, 11, 211);
            lv_img_set_pivot(needles->minute_dot, 14, 9);
        }
    }

    angle = needle_second * 60;
    if(needles->second_hand) lv_img_set_angle(needles->second_hand, angle);
    if(needles->second_hand_shadow) lv_img_set_angle(needles->second_hand_shadow, angle);
    if(needles->second_dot) lv_img_set_angle(needles->second_dot, angle);

    if(needles->second_hand)
    {
        lv_img_dsc_t* current_second_texture = (lv_img_dsc_t* )lv_img_get_src(needles->second_hand);
        if((angle > 1800) && (current_second_texture == textures->img_second_leftlight_psram))
        {
            lv_img_set_src(needles->second_hand, textures->img_second_rightlight_psram);
            lv_img_set_src(needles->second_dot, textures->img_second_rightlight_dot_psram);
            lv_img_set_pivot(needles->second_dot, 10, 9);
            lv_img_set_pivot(needles->second_hand, 3, 225);
        }
        else if((angle <= 1800) && (current_second_texture == textures->img_second_rightlight_psram))
        {
            lv_img_set_src(needles->second_hand, textures->img_second_leftlight_psram);
            lv_img_set_src(needles->second_dot, textures->img_second_leftlight_dot_psram);
            lv_img_set_pivot(needles->second_dot, 10, 9);
            lv_img_set_pivot(needles->second_hand, 3, 225);
        }
    }
}

void main_screen_handle_clock_event(lv_event_t* event)
{
    lv_obj_t* parent = lv_event_get_target(event);
    lv_obj_t* tileview = lv_obj_get_child(parent, 0);
    lv_obj_t* current_tile = lv_tileview_get_tile_act(tileview);

    lv_event_code_t code = lv_event_get_code(event);
    void* param = lv_event_get_param(event);

    lv_event_send(current_tile, code, param);
}

static void start_gui(void)
{
    //register timer for time update, this timer send event to all objects to update time.
    lv_timer_t * t = lv_timer_create(timer_cb, 1000, NULL);
    lv_timer_set_repeat_count(t, -1);
    SECOND_EVENT = lv_event_register_id();

    scr1 = lv_obj_create(NULL);
    scr2 = lv_obj_create(NULL);

    switch_watch_face_tileview(scr2);

    main_watch_face_tileview(scr1);
    lv_obj_add_event_cb(scr1, (lv_event_cb_t)watch_face_tile_change, LV_EVENT_SCREEN_LOADED, NULL);
    lv_obj_add_event_cb(scr1, (lv_event_cb_t)main_screen_handle_clock_event, SECOND_EVENT, NULL);

    lv_scr_load(scr1);
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

#if LV_USE_LOG == 1
    lv_log_register_print_cb((lv_log_print_g_cb_t)g_pfnCharPrint);
#endif
    //init img_gear_psram
    gear_img_default_init(img_gear_psram_arry_default);
    gear_img_init(img_gear_psram,img_gear_psram_arry_default);
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

    // Load textures from eMMC to PSRAM
    load_texture_c_menu();
    load_texture_to_psram();
    load_texture_theme(&watch_default_cfg, &watch_normal_material);
    load_texture_theme(&watch_aod_cfg, &watch_aod_material);
    load_texture_gear((lv_img_dsc_t**)watch_default_cfg.img_gear_psram_array,(char**)&gear1_file);

    // // Load textures to SSRAM
    // load_texture_to_ssram();

    start_gui();

    while (1)
    {
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

