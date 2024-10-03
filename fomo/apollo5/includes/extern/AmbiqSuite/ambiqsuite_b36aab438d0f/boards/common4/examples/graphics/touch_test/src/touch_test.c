//*****************************************************************************
//
//! @file touch_test.c
//!
//! @brief Touch Interface Test Example
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup touch_test Touch Interface Test Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This is a demo on drawing line on Apollo4b_bga_evb_disp_shld board with touch
//! screen.
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

#include "am_bsp.h"
#include "am_util.h"
#include "touch_test.h"
#include "am_devices_display_generic.h"

#ifdef AM_PART_APOLLO4B
AM_SHARED_RW uint32_t axiScratchBuf[20];
#endif

static const unsigned long RED       = 0xFF0000FF;
static am_devices_tc_chsc5816_info_t g_sTouchInfo = {0};
static void fill_round_rect_test(uint8_t screen_clear);
static void touch_handler(void *);

bool touch_press   = false;
bool touch_release = false;

static void touch_handler(void *x)
{
    if (g_sDispCfg.eIC == DISP_IC_RM69330 || g_sDispCfg.eIC == DISP_IC_RM67162)
    {
        if (am_devices_tma525_nonblocking_get_point((am_devices_tc_tma525_info_t *)&g_sTouchInfo) == AM_DEVICES_TMA525_STATUS_SUCCESS)
        {
            touch_release = g_sTouchInfo.touch_released;

            touch_press = !touch_release;

            if (touch_press == true)
            {
                am_util_stdio_printf("x = %d, y = %d\n", g_sTouchInfo.x0, g_sTouchInfo.y0);
            }

        }
    }
    else if (g_sDispCfg.eIC == DISP_IC_CO5300)
    {
        if (am_devices_chsc5816_nonblocking_get_point(&g_sTouchInfo) == AM_DEVICES_CHSC5816_STATUS_SUCCESS)
        {
            touch_press = !g_sTouchInfo.touch_released;

            if (touch_press == true)
            {
                if (g_sTouchInfo.finger_number == 1)
                {
                    am_util_stdio_printf("x0 = %d, y0 = %d\n",
                                          g_sTouchInfo.x0, g_sTouchInfo.y0);

                }
                else if (g_sTouchInfo.finger_number == 2)
                {
                    am_util_stdio_printf("x0 = %d, y0 = %d, x1 = %d, y1 = %d\n",
                                          g_sTouchInfo.x0, g_sTouchInfo.y0, g_sTouchInfo.x1, g_sTouchInfo.y1);
                }
                else
                {
                    am_util_stdio_printf("finger_number not supported!\n");
                }
            }
        }
    }
    else
    {
        am_util_stdio_printf("Error touch IC not supported!\n");
        return;
    }
}

void test_touch_panel(void)
{
    while(1)
    {
        if(touch_press == true)
        {
            // am_util_stdio_printf("Touch pressed!\n");
            fill_round_rect_test(1);
        }
        else
        {
            fill_round_rect_test(0);
        }
    }
}

static img_obj_t fb;
void fb_reload(void)
{
    fb.w = RESX;
    fb.h = RESY;
    fb.format = NEMA_RGB24;
    fb.stride = fb.w * nema_format_size(fb.format);
    fb.bo = nema_buffer_create(fb.stride * fb.h);
    if(fb.bo.base_virt == (void *)NULL)
    {
        //
        // have no enough space.this check is important!
        //
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }
    fb.color = 0;

    fb.sampling_mode = 0;

    am_util_stdio_printf("FB: V:%p P:0x%08x\n", (void *)fb.bo.base_virt, fb.bo.base_phys);
}

void fb_release(void)
{
    nema_buffer_destroy(&fb.bo);
}

nema_cmdlist_t cl;
void fill_round_rect_test(uint8_t screen_clear)
{
    char buf[12];
    int x1, y1;
    x1 = RESX / 2 - 40;
    y1 = 50;

    static uint8_t flag = 0;
    if (flag == 0)
    {
        cl = nema_cl_create();
        flag = 1;
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

    nema_draw_line(0, g_sTouchInfo.y0, RESX, g_sTouchInfo.y0, RED);
    nema_draw_line(g_sTouchInfo.x0, 0, g_sTouchInfo.x0, RESY, RED);
    nema_draw_circle(g_sTouchInfo.x0, g_sTouchInfo.y0, 20, RED);

    if (g_sTouchInfo.finger_number == 2)
    {
        nema_draw_line(0, g_sTouchInfo.y1, RESX, g_sTouchInfo.y1, RED);
        nema_draw_line(g_sTouchInfo.x1, 0, g_sTouchInfo.x1, RESY, RED);
        nema_draw_circle(g_sTouchInfo.x1, g_sTouchInfo.y1, 20, RED);
    }

    am_util_stdio_sprintf(buf, "x%d, y%d", g_sTouchInfo.x0, g_sTouchInfo.y0);
    nema_bind_font(&DejaVuSerif8pt1b);

    nema_print(buf, x1, y1, 100, 32, RED, NEMA_ALIGNX_LEFT);

    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    am_devices_display_transfer_frame(fb.w,
                                      fb.h,
                                      fb.bo.base_phys,
                                      NULL, NULL);
    //
    // wait transfer done
    //
    am_devices_display_wait_transfer_done();
}

int
touch_test(void)
{
    if (g_sDispCfg.eIC == DISP_IC_RM69330 || g_sDispCfg.eIC == DISP_IC_RM67162)
    {
        am_devices_tma525_init(AM_BSP_TP_IOM_MODULE, touch_handler, NULL);
    }
    else if (g_sDispCfg.eIC == DISP_IC_CO5300)
    {
        am_devices_chsc5816_init(AM_BSP_TP_IOM_MODULE, touch_handler, NULL);
    }
    else
    {
        am_util_stdio_printf("Error touch IC not supported!\n");
        return -1;
    }

    fb_reload();
    test_touch_panel();

    am_util_delay_ms(1000);

    return 0;
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
    // Set up scratch AXI buf (needs 64B - aligned to 16 Bytes)
    //
#ifdef AM_PART_APOLLO4B
    am_hal_daxi_control(AM_HAL_DAXI_CONTROL_AXIMEM, (uint8_t *)((uint32_t)(axiScratchBuf + 3) & ~0xF));
#endif
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

    am_util_stdio_printf("Restart ********************************************************************\n");

#ifdef BAREMETAL
    touch_test();
#else /* BAREMETAL */
    // Run the application.
    run_tasks();
#endif /* BAREMETAL*/

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

