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
#include "nemagfx_planet.h"

#include "nema_vg.h"
#include "nema_hal.h"
#include "nema_math.h"
#include "nema_core.h"
#include "nema_utils.h"
#include "nema_event.h"
#include "nema_programHW.h"

//#include "paint.h"
#include "graphics_power_control.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define EARTH_ORBITS_SPEED (8.0f)
#define VENUS_ORBITS_SPEED (13.0f)
#define RESONANCE    (VENUS_ORBITS_SPEED/EARTH_ORBITS_SPEED)

#define EARTH_ORBIT_RADIUS (165.0f)
#define VENUS_ORBIT_RADIUS (120.0f)

#define EARTH_RADIUS (8.0f)
#define VENUS_RADIUS (5.0f)
#define SUN_RADIUS (16.0f)

#define ORBIT_WIDTH (3.0f)
#define LINE_WIDTH (1.0f)

#define EARTH_ORBIT_STEP (5U)

#define DISPLAY_LOOP (8U)

#define TOTAL_LINE (360/EARTH_ORBIT_STEP * DISPLAY_LOOP)

//*****************************************************************************
//
// Render task handle.
//
//*****************************************************************************
TaskHandle_t RenderTaskHandle;

typedef struct rgb
{
  float r, g, b;
} RGB;

typedef struct hsl
{
  float h, s, l;
} HSL;

struct line_t
{
  float earth_x;
  float earth_y;
  float venus_x;
  float venus_y;
  uint32_t color;
};

// struct line_group_ctl_t
// {
//   uint32_t current;
//   uint32_t total;
//   struct line_t* p_line_data;
// };

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
struct line_t line_data[TOTAL_LINE];

// struct line_group_ctl line_group =
// {
//   .current = 0;
//   .total = 0;
//   .p_line_total = line_data;
// };

//*****************************************************************************
//
// Convert HUE format to RGB format
// Origin: https://gist.github.com/ciembor/1494530
// By: Maciej Ciemborowicz
//
//*****************************************************************************
/*
 * Converts an HUE to r, g or b.
 * returns float in the set [0, 1].
 */
float hue2rgb(float p, float q, float t)
{

  if (t < 0)
  {
    t += 1;
  }
  if (t > 1)
  {
    t -= 1;
  }
  if (t < 1. / 6)
  {
    return p + (q - p) * 6 * t;
  }
  if (t < 1. / 2)
  {
    return q;
  }
  if (t < 2. / 3)
  {
    return p + (q - p) * (2. / 3 - t) * 6;
  }

  return p;

}

//*****************************************************************************
//
// Convert HSL format to RGB format
// Origin: https://gist.github.com/ciembor/1494530
// By: Maciej Ciemborowicz
//
//*****************************************************************************
/*
 * Converts an HSL color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes h, s, and l are contained in the set [0, 1] and
 * returns RGB in the set [0, 255].
 */
RGB hsl2rgb(float h, float s, float l)
{

  RGB result;

  if (0 == s)
  {
    result.r = result.g = result.b = l; // achromatic
  }
  else
  {
    float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
    float p = 2 * l - q;
    result.r = hue2rgb(p, q, h + 1. / 3) * 255;
    result.g = hue2rgb(p, q, h) * 255;
    result.b = hue2rgb(p, q, h - 1. / 3) * 255;
  }

  return result;

}

//*****************************************************************************
//
// Planet orbital Resonance
//
//*****************************************************************************


void draw_bg(img_obj_t* des_img)
{
    float sun_x;
    float sun_y;

    // Sun location
    sun_x = des_img->w*0.5f;
    sun_y = des_img->h*0.5f;

    //Bind framebuffer
    nema_bind_dst_tex(des_img->bo.base_phys,
                      des_img->w,
                      des_img->h,
                      des_img->format,
                      des_img->stride);

    //Set clip
    nema_set_clip(0, 0, des_img->w, des_img->h);

    //Clear
    nema_clear(0x00000000);

    // Set blend mode
    nema_set_blend_fill(NEMA_BL_SIMPLE);

    //Enable anti-aliasing
    nema_enable_aa(1, 1, 1, 1);

    //earth orbit
    nema_draw_circle_aa(des_img->w*0.5f, des_img->h*0.5f, EARTH_ORBIT_RADIUS, ORBIT_WIDTH, 0xffffffff);

    //venus orbit
    nema_draw_circle_aa(des_img->w*0.5f, des_img->h*0.5f, VENUS_ORBIT_RADIUS, ORBIT_WIDTH, 0xffffffff);

    //Draw venus
    nema_fill_circle_aa(sun_x, sun_y,  VENUS_RADIUS, 0xff0000ff);
}

void draw_planet(img_obj_t* des_img, uint32_t line_idx, uint32_t draw_line_total)
{
    float earth_x;
    float earth_y;
    float venus_x;
    float venus_y;
    uint32_t color;

    // // Is there any limitation for nema_sin nema_con function?
    // float earth_degree = deg + 270.0f;
    // float venus_degree = deg * RESONANCE + 270.0f;

    // HSL hsl_line_color;
    // RGB rgb_line_color;

    //Bind framebuffer
    nema_bind_dst_tex(des_img->bo.base_phys,
                      des_img->w,
                      des_img->h,
                      des_img->format,
                      des_img->stride);

    //Set clip
    nema_set_clip(0, 0, des_img->w, des_img->h);

    nema_clear(0x00000000);

    nema_set_blend_fill(NEMA_BL_SIMPLE);

    //Enable anti-aliasing
    nema_enable_aa(1, 1, 1, 1);

    float alpha_f = 1.0f;
    int index = line_idx;
    uint32_t alpha_uint;

    while(draw_line_total > 0 )
    {

        earth_x = line_data[index].earth_x;
        earth_y = line_data[index].earth_y;
        venus_x = line_data[index].venus_x;
        venus_y = line_data[index].venus_y;

        color = line_data[index].color;
        alpha_uint = ((uint32_t)(alpha_f * 255.0f) << 24) & 0xff000000;
        color |= alpha_uint;

        //Draw the connection line between earth and venus
        nema_draw_line_aa(earth_x, earth_y, venus_x, venus_y, LINE_WIDTH, color);

        draw_line_total --;
        index --;
        if ( index < 0 )
        {
          index += TOTAL_LINE;
        }

        alpha_f -= (1.0f / TOTAL_LINE / 2);
        if ( alpha_f < 0 )
        {
            alpha_f = 0.f;
        }
    }

    //earth orbit
    nema_draw_circle_aa(des_img->w*0.5f, des_img->h*0.5f, EARTH_ORBIT_RADIUS, ORBIT_WIDTH, 0xffffffff);

    //venus orbit
    nema_draw_circle_aa(des_img->w*0.5f, des_img->h*0.5f, VENUS_ORBIT_RADIUS, ORBIT_WIDTH, 0xffffffff);

    // //earth location
    // earth_x = des_img->w*0.5f + EARTH_ORBIT_RADIUS*nema_cos(earth_degree);
    // earth_y = des_img->h*0.5f + EARTH_ORBIT_RADIUS*nema_sin(earth_degree);

    // //venus location
    // venus_x = des_img->w*0.5f + VENUS_ORBIT_RADIUS*nema_cos(venus_degree);
    // venus_y = des_img->h*0.5f + VENUS_ORBIT_RADIUS*nema_sin(venus_degree);

    // //Enable anti-aliasing
    // nema_enable_aa(1, 1, 1, 1);

    earth_x = line_data[line_idx].earth_x;
    earth_y = line_data[line_idx].earth_y;
    venus_x = line_data[line_idx].venus_x;
    venus_y = line_data[line_idx].venus_y;

    //Draw earth
    nema_fill_circle_aa(earth_x, earth_y,  EARTH_RADIUS, 0x80ff0000);

    //Draw venus
    nema_fill_circle_aa(venus_x, venus_y,  VENUS_RADIUS, 0x80ff00ff);

    //Draw sun
    nema_fill_circle_aa(FB_RESX*0.5f, FB_RESY*0.5f,  SUN_RADIUS, 0xff0000ff);

    // int temp = earth_degree/360.0f;

    // hsl_line_color.h = (earth_degree - temp * 360.0f) / 360.0f;

    // hsl_line_color.s = 0.5f;
    // hsl_line_color.l = 0.5f;

    // rgb_line_color = hsl2rgb(hsl_line_color.h, hsl_line_color.s, hsl_line_color.l);

    // uint32_t color = nema_rgba((uint8_t)rgb_line_color.r, (uint8_t)rgb_line_color.g, (uint8_t)rgb_line_color.b, 0x00);



}

void init_line_group(struct line_t* data)
{
    uint32_t i;
    float earth_degree;
    float venus_degree;
    int temp;
    uint32_t color;

    HSL hsl_line_color;
    RGB rgb_line_color;

    for ( i = 0; i < TOTAL_LINE; i++ )
    {
      earth_degree = i*EARTH_ORBIT_STEP + 270.0f;
      venus_degree = i*EARTH_ORBIT_STEP * RESONANCE + 270.0f;

      //earth location
      data[i].earth_x = FB_RESX*0.5f + EARTH_ORBIT_RADIUS*nema_cos(earth_degree);
      data[i].earth_y = FB_RESY*0.5f + EARTH_ORBIT_RADIUS*nema_sin(earth_degree);

      //venus location
      data[i].venus_x = FB_RESX*0.5f + VENUS_ORBIT_RADIUS*nema_cos(venus_degree);
      data[i].venus_y = FB_RESY*0.5f + VENUS_ORBIT_RADIUS*nema_sin(venus_degree);

      temp = earth_degree / 360.0f;

      hsl_line_color.h = (earth_degree - temp * 360.0f) / 360.0f;

      hsl_line_color.s = 0.5f;
      hsl_line_color.l = 0.5f;

      rgb_line_color = hsl2rgb(hsl_line_color.h, hsl_line_color.s, hsl_line_color.l);

      data[i].color = nema_rgba((uint8_t)rgb_line_color.r, (uint8_t)rgb_line_color.g, (uint8_t)rgb_line_color.b, 0x00);
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
    void *vb;
    bool init = false;
    uint32_t idx = 0;
    uint32_t display_line = 1;

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

    init_line_group(line_data);

    while(1)
    {
        //Wait start.
        xSemaphoreTake( g_semGPUStart, portMAX_DELAY);

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_clear(DEBUG_PIN_3);
        am_hal_gpio_output_clear(DEBUG_PIN_6);
#endif

        //This API will directly return if GPU is still power on.
        gpu_power_up();

        //rewind and bind the CL
        nema_cl_rewind(&cl);
        nema_cl_bind(&cl);

        draw_planet(g_pFrameBufferGPU, idx, display_line);

        idx ++;
        if ( idx >= TOTAL_LINE )
        {
            idx = 0;
        }

        display_line ++;
        if ( display_line >= TOTAL_LINE )
        {
            display_line = TOTAL_LINE;
        }

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

        //Power Down
        gpu_power_down();

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_set(DEBUG_PIN_3);
#endif
    }

}

