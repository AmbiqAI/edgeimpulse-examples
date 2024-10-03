//*****************************************************************************
//
//! @file nemagfx_rotating_clock.c
//!
//! @brief NemaGFX Rotating Clock Example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_rotating_clock NemaGFX Rotating Clock Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example use two frame buffer demonstrate a digital rotating clock,
//! with Nema GPU support, the shader effect continue shows while timer passing
//! need a timer to get the accurate time past.

//! AM_DEBUG_PRINTF
//! If enabled, debug messages will be sent over ITM.
//
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
// This application has a large number of common include files. For
// convenience, we'll collect them all together in a single header and include
// that everywhere.
//
//*****************************************************************************

#include "nemagfx_rotating_clock.h"
#include "numbers_rgba.h"
//#define AM_DEBUG_PRINTF

//*****************************************************************************
//
// Variables Definition.
//
//*****************************************************************************

static int32_t              VIEWPORT_WIDTH  = RESX;
static int32_t              VIEWPORT_HEIGHT = (RESX / 4 * 3);

static nema_cmdlist_t       g_sCL;

static uint32_t             g_ui32CurFB = 0;

static img_obj_t            g_sNumbersImg = {{0},
                                            TEX_DIM,
                                            TEX_DIM * 10,
                                            TEX_DIM * 4,
                                            0,
                                            NEMA_RGBA8888,
                                            NEMA_FILTER_BL};

#define FRAME_BUFFERS       2

#ifdef AM_PART_APOLLO4B
    #define GPU_FORMAT          NEMA_RGBA8888
#else
    #define GPU_FORMAT          NEMA_RGB24
#endif

static img_obj_t            g_sFB[FRAME_BUFFERS];

static const double         height2 = (double)((double)0.3249196962 * (double)ALTITUDE);

static float                proj_matrix[4][4] = {{ 5.3365272918, 0.0000,  0.0000,  0.0000},
                                                { 0.0000, 7.1153697224,  0.0000,  0.0000},
                                                { 0.0000, 0.0000, -1.0833333333, -1.0000},
                                                { 0.0000, 0.0000, -2.0833333333,  0.0000}};

static float                modelview_matrix[4][4] = {{ 1.0000, 0.0000,  0.0000, 0.0000},
                                                     { 0.0000, 1.0000,  0.0000, 0.0000},
                                                     { 0.0000, 0.0000,  1.0000, 0.0000},
                                                     { 0.0000, -0.50000, -11.8980, 1.0000}};

static float                mvp_matrix[4][4];

static float                v[PLEURES * 2 + 2][4];

static void
load_objects(void)
{
    for (int32_t i = 0; i < FRAME_BUFFERS; ++i)
    {
        g_sFB[i].w = RESX;
        g_sFB[i].h = RESY;
        g_sFB[i].format = GPU_FORMAT;
        g_sFB[i].stride = g_sFB[i].w * nema_format_size(g_sFB[i].format);
        g_sFB[i].bo = nema_buffer_create(g_sFB[i].stride * g_sFB[i].h);
        nema_buffer_map(&g_sFB[i].bo);
        printf("FB: V:%p P:0x%08x\n", (void *)g_sFB[i].bo.base_virt,
                                        g_sFB[i].bo.base_phys);
    }

    g_sNumbersImg.bo = nema_buffer_create(i32NumbersRGBALength);
    memcpy(g_sNumbersImg.bo.base_virt,
           g_ui8NumbersRGBA,
           i32NumbersRGBALength);
}

static void
obj2Window_coords(float dstvec[4], float mvp[4][4], float vec[4])
{
    //
    // http://www.songho.ca/opengl/gl_transform.html
    // vec: Obj Coords
    //

    int32_t y = 0;
    dstvec[3] = mvp[0][3] * vec[0] + mvp[1][3] * vec[1] + mvp[2][3] * vec[2] + mvp[3][3] * vec[3];
    for (y = 0; y < 3; ++y)
    {
        //!< Clip Coords
        dstvec[y] = mvp[0][y] * vec[0] + mvp[1][y] * vec[1] + mvp[2][y] * vec[2] + mvp[3][y] * vec[3];
        //!< NDC Coords
        dstvec[y] /= dstvec[3];
    }

    //!< Window Coords
    dstvec[0] = (VIEWPORT_WIDTH  / 2) * dstvec[0] + (VIEWPORT_WIDTH  / 2);
    dstvec[1] = (VIEWPORT_HEIGHT / 2) * dstvec[1] + (VIEWPORT_HEIGHT / 2);
    dstvec[2] = ((ZFAR-ZNEAR) * dstvec[2] + (ZFAR + ZNEAR)) / 2;
}

static void
rotateX(float angle)
{
    float cos0 = nema_cos(angle);
    float sin0 = nema_sin(angle);

    modelview_matrix[1][1] =  cos0;
    modelview_matrix[1][2] = -sin0;
    modelview_matrix[2][1] =  sin0;
    modelview_matrix[2][2] =  cos0;
}

static void
calculate_vertices(int32_t i32Maximum, float position,float time)
{
    float vec0[4] = { WIDTH2, -height2, ALTITUDE, 1.0};
    float vec1[4] = {-WIDTH2, -height2, ALTITUDE, 1.0};

    modelview_matrix[3][0] = position;

    for (int32_t i32TimeIndex = 0; i32TimeIndex <= i32Maximum + 1; ++i32TimeIndex)
    {
        rotateX(-(time - i32TimeIndex)*CHAR_ANGLE);

        nema_mat4x4_mul(mvp_matrix, modelview_matrix,proj_matrix);

        obj2Window_coords(v[i32TimeIndex * 2    ], mvp_matrix, vec0);
        obj2Window_coords(v[i32TimeIndex * 2 + 1], mvp_matrix, vec1);
    }
}

//*****************************************************************************
//
//! @brief draw a single number
//!
//! @param  time                    - current time
//! @param  i32TimeIndex            - drawn time index
//!
//! This function used to draw single time number with target background color.
//!
//! @return None.
//
//*****************************************************************************
static void
draw_single_number(float time, int32_t i32TimeIndex)
{
    i32TimeIndex = i32TimeIndex % PLEURES;
    int32_t i32NextTimeIndex = (i32TimeIndex + 1) % PLEURES;
    int32_t i32PreviousTimeIndex = (i32TimeIndex + PLEURES - 1) % PLEURES;

    float cos_angle = nema_cos(-(time - i32TimeIndex) * CHAR_ANGLE);

    //
    // configure default time number background color.
    //
    uint32_t ui32Col = (uint32_t)(50 * cos_angle + 125);

    if ((int32_t)time == i32TimeIndex)
    {
        //
        // configure current time number background color.
        //
        ui32Col += (uint32_t)(80 * (double)(1 + i32TimeIndex - time));
    }
    else if ((int32_t)time == i32PreviousTimeIndex)
    {
        //
        // configure previous time number background color.
        //
        ui32Col += (uint32_t)(80 * (double)(time - i32PreviousTimeIndex));
    }

    ui32Col = ((ui32Col) << 24) | ((ui32Col) << 16) | ((ui32Col) << 8) | ((ui32Col) << 0);

    nema_set_const_color(ui32Col);
    nema_blit_subrect_quad_fit( v[i32TimeIndex * 2 + 1][0], v[i32TimeIndex * 2 + 1][1],
                                v[i32TimeIndex * 2    ][0], v[i32TimeIndex * 2    ][1],
                                v[i32NextTimeIndex * 2    ][0], v[i32NextTimeIndex * 2    ][1],
                                v[i32NextTimeIndex * 2 + 1][0], v[i32NextTimeIndex * 2 + 1][1],
                                0, i32TimeIndex * TEX_DIM, TEX_DIM, TEX_DIM);
}

//*****************************************************************************
//
//! @brief draw rolling numbers
//!
//! @param  i32Maximum                       - maximum value
//! @param  position                         - numbers position
//! @param  currentTime                      - current time
//!
//! This function used to draw rolling numbers,it's start with zero,end with nine.
//!
//! @return None.
//
//*****************************************************************************
static void
draw_rolling_numbers(int32_t i32Maximum, float position, float currentTime)
{
    int32_t i32FrontNum = (int32_t)currentTime  % PLEURES;
    int32_t i32BackNum = (int32_t)(i32FrontNum + 0.5 * PLEURES) % PLEURES;

    calculate_vertices(i32Maximum, position,currentTime);

    //
    // draw back time numbers
    //
    int32_t i32Time = i32BackNum;
    do
    {
        if (i32Time <= i32Maximum)
        {
            draw_single_number(currentTime, i32Time);
        }
        i32Time = (i32Time + 1) % PLEURES;
    }
    while (i32Time != i32FrontNum);

    //
    // draw front time numbers
    //
    i32Time = i32BackNum;
    do
    {
        i32Time = (i32Time + PLEURES - 1) % PLEURES;

        if (i32Time <= i32Maximum)
        {
            draw_single_number(currentTime, i32Time);
        }
    }
    while (i32Time != i32FrontNum);
}

//*****************************************************************************
//
//! @brief draw rotating clock
//!
//! @param  ui32Hour                    - time hour value
//! @param  ui32Min                     - time minute value
//! @param  ui32Sec                     - time second value
//! @param  msec                        - time milisecond value
//!
//! This function draw clock with rotating effect with dual-buffer or single buffer.
//!
//! @return None.
//
//*****************************************************************************
static void
draw_clock(uint32_t ui32Hour, uint32_t ui32Min, uint32_t ui32Sec, float msec)
{
    uint32_t ui32Sec01  = ui32Sec % 10;
    uint32_t ui32Sec10  = ui32Sec / 10;
    uint32_t ui32Min01  = ui32Min % 10;
    uint32_t ui32Min10  = ui32Min / 10;
    uint32_t ui32Hour01 = ui32Hour % 10;
    uint32_t ui32Hour10 = ui32Hour / 10;

    //
    // calulate time in float format
    //
    float sec01_f  = (msec + ui32Sec01);
    float sec10_f  = ui32Sec01 != 9  ? ui32Sec10 :
                     ui32Sec10 != 5  ? (EASE(msec)*1.f + (float)ui32Sec10):
                                       (EASE(msec)*5.f + (float)ui32Sec10);

    float min01_f  = ui32Sec   != 59 ? ui32Min01 : (EASE(msec) + ui32Min01);
    float min10_f  = ui32Min01 != 9 || ui32Sec != 59 ? ui32Min10 :
                             ui32Min10 != 5  ? (EASE(msec)*1.f + ui32Min10):
                                               (EASE(msec)*5.f + ui32Min10);

    float hour01_f = ui32Sec   != 59 || ui32Min != 59 ? ui32Hour01 :
                            ui32Hour == 23 ? (EASE(msec)*7.f + ui32Hour01) :
                                             (EASE(msec) + ui32Hour01);
    float hour10_f = ui32Sec   != 59 || ui32Min != 59 || (ui32Hour01 != 9 && ui32Hour != 23)
                          ? ui32Hour10 :
                            ui32Hour01 == 9 ? (EASE(msec) + ui32Hour10):
                                              (EASE(msec)*8.f + ui32Hour10);
    //
    // transfer frame to the display
    //
    am_devices_display_transfer_frame(g_sFB[g_ui32CurFB].w,
                                      g_sFB[g_ui32CurFB].h,
                                      g_sFB[g_ui32CurFB].bo.base_phys,
                                      NULL, NULL);
    g_ui32CurFB = (g_ui32CurFB+1) % FRAME_BUFFERS;

    nema_cl_rewind(&g_sCL);

    nema_bind_dst_tex(g_sFB[g_ui32CurFB].bo.base_phys,
                      g_sFB[0].w,
                      g_sFB[0].h,
                      g_sFB[0].format,
                      g_sFB[0].stride);

    nema_bind_src_tex(g_sNumbersImg.bo.base_phys,
                      g_sNumbersImg.w,
                      g_sNumbersImg.h,
                      g_sNumbersImg.format,
                      g_sNumbersImg.stride,
                      g_sNumbersImg.sampling_mode);

    nema_set_clip(0, 0, RESX, RESY);

    nema_clear(0);
    nema_set_blend_blit(NEMA_BL_SIMPLE | NEMA_BLOP_MODULATE_RGB);
    //
    // draw hour time
    //
    draw_rolling_numbers(2, -1.56, hour10_f);
    draw_rolling_numbers(9, -1.00, hour01_f);
    //
    // draw minute time
    //
    draw_rolling_numbers(5, -0.28, min10_f);
    draw_rolling_numbers(9, 0.28, min01_f);
    //
    // draw second time
    //
    draw_rolling_numbers(9, 1.56, sec01_f);
    draw_rolling_numbers(5, 1.00, sec10_f);

    nema_cl_submit(&g_sCL);
    //
    //wait GPU render completed after transfer frame when using dualbuffers.
    //
    nema_cl_wait(&g_sCL);
    //
    // wait transfer done
    //
    am_devices_display_wait_transfer_done();

}

int32_t
rotating_clock()
{
    load_objects();
    //
    // Create Command Lists
    //
    g_sCL  = nema_cl_create();

    nema_cl_bind(&g_sCL);
#ifdef CUSTOM_TIME
    static float time0 = 23.f*60.f*60.f + 59.f*60.f + 30.f;
#endif

    while (1)
    {

#ifdef CUSTOM_TIME  //!< calculate current time
        float time = time0;
        time0 += 2.f / 60.f;

        if (time0 >= 24.f * 60.f * 60.f)
        {
            time0 = 0.f;
        }
#else
        float time = nema_get_time();
#endif

        int32_t ui32Sec  = (int32_t)time % 60;
        int32_t ui32Hour = (int32_t)(time / 60 / 60);
        int32_t ui32Min  = ((int32_t)(time / 60)) % 60;
        float msec = (time - (float)(int32_t)time);

        draw_clock(ui32Hour, ui32Min, ui32Sec, msec);

        nema_calculate_fps();
    }
}

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{
    //
    // External power on
    //
    am_bsp_external_pwr_on();
    am_util_delay_ms(100);
    am_bsp_low_power_init();
    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();
    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

#ifdef AM_DEBUG_PRINTF
    //
    // Enable debug print to the console.
    //
    am_bsp_debug_printf_enable();
#endif

    //
    // Initialize display
    //
    am_devices_display_init(RESX,
                            RESY,
                            COLOR_FORMAT_RGB888,
                            false);

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Initialize NemaGFX
    //
    nema_init();

    am_hal_timer_config_t sTimerConfig;
    uint32_t ui32Status;
    ui32Status = am_hal_timer_default_config_set(&sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to initialize a timer configuration structure with default values!\n");
    }
    sTimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    sTimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    ui32Status = am_hal_timer_config(0, &sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure a timer!\n");
    }
    ui32Status = am_hal_timer_start(0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to start a timer!\n");
    }

    rotating_clock();
    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

