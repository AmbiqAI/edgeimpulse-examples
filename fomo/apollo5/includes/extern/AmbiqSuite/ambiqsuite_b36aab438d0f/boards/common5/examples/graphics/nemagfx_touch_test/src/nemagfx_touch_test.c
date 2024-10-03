//*****************************************************************************
//
//! @file nemagfx_touch_test.c
//!
//! @brief nemagfx_touch_test example
//! This is a demo on drawing line on with touch feature.
//! print out the X and Y touch position on the screen
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "nemagfx_touch_test.h"

#define PATTERN_BUF_SIZE        8
#define TARGET_COLOR            0xFFFFFFFF

nema_event_t g_event;
bool touch_press   = false;
bool touch_release = false;

static img_obj_t fb;
nema_cmdlist_t cl;

static am_devices_tc_tma525_info_t g_sTouchInfo = {0};

static void touch_handler(void *x);

static void
touch_handler(void *x)
{
    if (am_devices_tma525_nonblocking_get_point((am_devices_tc_tma525_info_t *)&g_sTouchInfo) == AM_DEVICES_TMA525_STATUS_SUCCESS)
    {
        g_event.mouse_x = g_sTouchInfo.x0;
        g_event.mouse_y = g_sTouchInfo.y0;
        touch_release = g_sTouchInfo.touch_released;
        
        if (touch_release == true)
        {
            touch_press = false;
        }
        else
        {
            touch_press = true;
            am_util_stdio_printf("(x,y) = (%d, %d)\n", g_event.mouse_x, g_event.mouse_y);
        }
    }
}

void
render_crossline(uint8_t screen_clear)
{
    static int lastX = 0, lastY = 0, flag = 0;

    if (flag == 0)
    {
        cl = nema_cl_create();
    }

    nema_cl_rewind(&cl);
    nema_cl_bind(&cl);
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, fb.stride);

    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_fill(NEMA_BL_SRC);

    if (screen_clear)
    {
        nema_clear(0);
    }

    if (flag == 1)
    {
        nema_draw_line(0, lastY, RESX, lastY, 0);
        nema_draw_line(lastX, 0, lastX, RESY, 0);
        nema_draw_circle(lastX, lastY, 20, 0);
    }

    nema_draw_line(0, g_event.mouse_y, RESX, g_event.mouse_y, TARGET_COLOR);
    nema_draw_line(g_event.mouse_x, 0, g_event.mouse_x, RESY, TARGET_COLOR);
    nema_draw_circle(g_event.mouse_x, g_event.mouse_y, 20, TARGET_COLOR);

    lastX = g_event.mouse_x;
    lastY = g_event.mouse_y;

    if (flag == 1)
    {
        //
        // wait transfer done
        //
        am_devices_display_wait_transfer_done();
    }

    nema_cl_submit(&cl);
    flag = 1;
    nema_cl_wait(&cl);

    am_devices_display_transfer_frame(fb.w,
                                  fb.h,
                                  fb.bo.base_phys,
                                  NULL, NULL);
}

void
fb_release(void)
{
    nema_buffer_destroy(&fb.bo);
}


void
touch_init(void)
{
    //
    // initialize touchpad
    //
    am_devices_tma525_init(AM_BSP_TP_IOM_MODULE, touch_handler, NULL);
}

//*****************************************************************************
//
// Main function
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
    am_util_stdio_printf("\nnemagfx_touch_test\n");

    am_devices_disp_color_e eDispColor = COLOR_FORMAT_RGB888;
    //
    // Initialize display
    //
    am_devices_display_init(RESX,
                            RESY,
                            eDispColor,
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

    //
    // initialize frame buffer
    //
    fb.w = RESX;
    fb.h = RESY;
    fb.format = (eDispColor == COLOR_FORMAT_RGB565) ? NEMA_RGB565 : NEMA_RGB24;
    fb.stride = fb.w * nema_format_size(fb.format);
    fb.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, fb.stride * fb.h);
    if (fb.bo.base_virt == (void *)NULL)
    {
        //
        // Not enough Memories.this check is important!
        //
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }

    touch_init();

    while (1)
    {
        static bool is_released = true;
        if ( touch_press == true )
        {
            render_crossline(1);
            is_released = false;
        }
        else if ( is_released != true )
        {
            render_crossline(0);
            is_released = true;
        }
    }
}

