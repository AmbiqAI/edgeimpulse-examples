//*****************************************************************************
//
//! @file nemagfx_balls_bench.c
//!
//! @brief NemaGFX example.
//! This example created some random balls with alpha blending on the screen,
//! use
//! #define MAX_CIRCLE_NUM               (15)
//! to choose how many balls rendering on the screen
//!
//! AM_DEBUG_PRINTF
//! If enabled, debug messages will be sent over ITM.
//! @addtogroup graphics
//! @ingroup apollo5 graphics
//! @{
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
#include "nemagfx_balls_bench.h"

#define RESX 360
#define RESY 360

#define MAX_CIRCLE_NUM  (15)
//*****************************************************************************
//
//! define number of framebuffer,could define FRAME_BUFFERS equivalent to 2 for
//! ping-pong mode demonstate graphics performance if have enough space.
//
//*****************************************************************************
#define FRAME_BUFFERS   2

//*****************************************************************************
//
// Variables Definition
//
//*****************************************************************************

static img_obj_t        g_sFB[FRAME_BUFFERS];

//
// ball structure.
//
typedef struct
{
    int32_t i32LastX;
    int32_t i32LastY;

    int32_t i32CenterX;
    int32_t i32CenterY;

    int32_t i32Radius;

    int32_t i32SpeedX;
    int32_t i32SpeedY;

    uint8_t ui8Red;
    uint8_t ui8Green;
    uint8_t ui8Blue;
    uint8_t ui8Aplha;
}
Circle_t;

Circle_t g_sCircle[MAX_CIRCLE_NUM];

//*****************************************************************************
//
//! @brief initialize framebuffer and DC's layer if have.
//!
//! This function initialize framebuffer(s) and layer(s)
//!
//! @return status.
//
//*****************************************************************************
static void
init_framebuffer(const am_devices_disp_color_e eDispColor)
{
    for (uint8_t i = 0; i < FRAME_BUFFERS; ++i)
    {
        g_sFB[i].w = RESX;
        g_sFB[i].h = RESY;
        if ( eDispColor == COLOR_FORMAT_RGB888 )
        {
            g_sFB[i].format = NEMA_RGB24;
        }
        else if ( eDispColor == COLOR_FORMAT_RGB565 )
        {
            g_sFB[i].format = NEMA_RGB565;
        }
        else
        {
            am_util_stdio_printf("Invalid color format!\n");
            while(1);
        }
        g_sFB[i].stride = g_sFB[i].w * nema_format_size(g_sFB[i].format);
        g_sFB[i].bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, g_sFB[i].stride * g_sFB[i].h);
        if ( g_sFB[i].bo.base_virt == (void *)NULL )
        {
            //
            // have no enough space.this check is important!
            //
            am_util_stdio_printf("Failed to create FB!\n");
            while(1);
        }
        nema_buffer_map(&g_sFB[i].bo);

    }
}

//*****************************************************************************
//
//! @brief update balls position and speed direction
//!
//! This function make sure all the balls in panel visible region.it can bounce
//! back balls when out of bounds.
//!
//! @return None.
//
//*****************************************************************************
void
update_circle(void)
{
    int16_t i16CurrentY;
    int16_t i16CurrentX;
    for (uint16_t i = 0; i < MAX_CIRCLE_NUM; i++)
    {
        i16CurrentY = g_sCircle[i].i32CenterY + g_sCircle[i].i32SpeedY;
        if (i16CurrentY > RESY - g_sCircle[i].i32Radius || i16CurrentY < g_sCircle[i].i32Radius)
        {
            if (g_sCircle[i].i32SpeedY > 0)
            {
                g_sCircle[i].i32CenterY = RESY - g_sCircle[i].i32Radius;
                g_sCircle[i].i32SpeedY = -g_sCircle[i].i32SpeedY;
            }
            else
            {
                g_sCircle[i].i32CenterY = g_sCircle[i].i32Radius;
                g_sCircle[i].i32SpeedY = abs(g_sCircle[i].i32SpeedY);
            }
        }
        else
        {
            g_sCircle[i].i32CenterY = i16CurrentY;
        }

        i16CurrentX = g_sCircle[i].i32CenterX + g_sCircle[i].i32SpeedX;
        if (i16CurrentX > RESX - g_sCircle[i].i32Radius || i16CurrentX < g_sCircle[i].i32Radius)
        {
            if (g_sCircle[i].i32SpeedX > 0)
            {
                g_sCircle[i].i32CenterX = RESX - g_sCircle[i].i32Radius;
                g_sCircle[i].i32SpeedX = -g_sCircle[i].i32SpeedX;
            }
            else
            {
                g_sCircle[i].i32CenterX = g_sCircle[i].i32Radius;
                g_sCircle[i].i32SpeedX = abs(g_sCircle[i].i32SpeedX);
            }
        }
        else
        {
            g_sCircle[i].i32CenterX = i16CurrentX;
        }

        nema_fill_circle(g_sCircle[i].i32CenterX, g_sCircle[i].i32CenterY, g_sCircle[i].i32Radius,
                         nema_rgba(g_sCircle[i].ui8Red, g_sCircle[i].ui8Green, g_sCircle[i].ui8Blue, g_sCircle[i].ui8Aplha));
    }

}

//*****************************************************************************
//
//! @brief render balls and transfer frame to display.
//!
//! draw all balls move in display region,it can bounce back balls when these out of
//! bounds. the application also printing some interval time,especially fps value.
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
void
balls_bench(void)
{
    float start_time;
    float end_time;
    static uint8_t ui8CurrentIndex = 0;
    uint32_t ui32PrintTimer = 0;
    nema_cmdlist_t sCLCircles;

    srand(1);

    for (uint16_t i = 0; i < MAX_CIRCLE_NUM; i++)
    {
        g_sCircle[i].i32Radius = rand() % 100;
        g_sCircle[i].i32CenterX = rand() % (RESX - g_sCircle[i].i32Radius * 2) + g_sCircle[i].i32Radius;
        g_sCircle[i].i32CenterY = rand() % (RESY - g_sCircle[i].i32Radius * 2) + g_sCircle[i].i32Radius;

        g_sCircle[i].ui8Red = rand() % 256;
        g_sCircle[i].ui8Green = rand() % 256;
        g_sCircle[i].ui8Blue = rand() % 256;
        g_sCircle[i].ui8Aplha = rand() % 256;

        g_sCircle[i].i32SpeedX = rand() % 5 + 1;
        g_sCircle[i].i32SpeedY = rand() % 5 + 1;
    }

    sCLCircles = nema_cl_create();
    nema_cl_bind(&sCLCircles);
    while (1)
    {
        //
        // transfer frame to the display
        //
        am_devices_display_transfer_frame(g_sFB[ui8CurrentIndex].w,
                                          g_sFB[ui8CurrentIndex].h,
                                          g_sFB[ui8CurrentIndex].bo.base_phys,
                                          NULL, NULL);

        ui8CurrentIndex = (ui8CurrentIndex + 1) % FRAME_BUFFERS;

        nema_bind_dst_tex(g_sFB[ui8CurrentIndex].bo.base_phys,
                          g_sFB[ui8CurrentIndex].w,
                          g_sFB[ui8CurrentIndex].h,
                          g_sFB[ui8CurrentIndex].format,
                          g_sFB[ui8CurrentIndex].stride);
        nema_set_clip(0, 0, RESX, RESY);
        nema_clear(nema_rgba(0x00, 0x00, 0x00, 0xff));
        nema_set_blend_fill(NEMA_BL_SIMPLE);

        start_time = nema_get_time();
        update_circle();
        end_time = nema_get_time();
        if (++ui32PrintTimer % 100 == 0)
        {
            am_util_stdio_printf("update_circle used %f s\n", end_time - start_time);
        }

        start_time = nema_get_time();
        nema_cl_submit(&sCLCircles);
        nema_calculate_fps();
        nema_cl_wait(&sCLCircles);
        end_time = nema_get_time();
        if (ui32PrintTimer % 100 == 0)
        {
            am_util_stdio_printf("nema_cl_wait used %f s\n", end_time - start_time);
        }
        nema_cl_rewind(&sCLCircles);
        //
        // wait transfer done
        //
        am_devices_display_wait_transfer_done();
#ifndef BAREMETAL
        taskYIELD();
#endif
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
    am_bsp_low_power_init();

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();
    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // color format
    //
    am_devices_disp_color_e eDispColor = COLOR_FORMAT_RGB888;
    //
    // Initialize display
    //
    am_devices_display_init(RESX,
                            RESY,
                            eDispColor,
                            false);

    //
    // Initialize GPU
    //
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

    nema_init();

    init_framebuffer(eDispColor);

#ifdef AM_DEBUG_PRINTF
    //
    // Enable debug printing to the console.
    //
    am_bsp_debug_printf_enable();
#endif

#ifdef BAREMETAL
    am_util_stdio_printf("nemafgx_balls_bench Example\n");
    balls_bench();
#else // BAREMETAL
    //
    // Initialize plotting interface.
    //
    am_util_debug_printf("FreeRTOS nemafgx_balls_bench Example\n");

    //
    // Run the application.
    //
    run_tasks();
#endif // BAREMETAL

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
