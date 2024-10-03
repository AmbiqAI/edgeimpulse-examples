//*****************************************************************************
//
//! @file nemagfx_grad.c
//!
//! @brief NemaGFX Color Gradient Example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_grad NemaGFX Color Gradient Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: In computer graphics, a color gradient specifies a range of position-dependent
//! colors, usually used to fill a region. For example, many window managers
//! allow the screen background to be specified as a gradient. The colors
//! produced by a gradient vary continuously with the position, producing smooth
//! color transitions.

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
#include "nemagfx_grad.h"

//*****************************************************************************
//
// Variables Definition.
//
//*****************************************************************************
#ifdef AM_PART_APOLLO4B
//
// AXI Scratch buffer Need to allocate 20 Words even though we only need 16, to ensure we have 16 Byte alignment
//
AM_SHARED_RW uint32_t       axiScratchBuf[20];
#endif
AM_SHARED_RW nema_cmdlist_t g_sContextCL;

#if defined (AM_PART_APOLLO4B)
    img_obj_t                   g_sFB = {{0}, RESX, RESY, -1, 0, NEMA_RGBA8888, 0};
#else
    img_obj_t                   g_sFB = {{0}, RESX, RESY, -1, 0, NEMA_RGB24, 0};
#endif

//*****************************************************************************
//
//! @brief set vertex color
//!
//! @param  sColor                  - pointer of color structure
//! @param  ui8Value                - value for different color.
//!
//! This function just set alpha value to zero,and red ,blue and green three colors.
//!
//! @return None.
//
//*****************************************************************************
void
set_color(color_var_t *sColor,uint8_t ui8Value)
{
    sColor->a = 0;
    switch(ui8Value%GRIDNUM)
    {
        case 0:
            sColor->r = 0xFF;
            sColor->g = 0x00;
            sColor->b = 0x00;
            break;
        case 1:
            sColor->r = 0x00;
            sColor->g = 0xFF;
            sColor->b = 0x00;
            break;
        case 2:
            sColor->r = 0x00;
            sColor->g = 0x00;
            sColor->b = 0xFF;
            break;
        default:
            break;
    }
}

//*****************************************************************************
//
//! @brief fill 3x3 rectangles gradient
//!
//! This function fill 3x3 ractangles with different gradient types.
//!
//! @return None.
//
//*****************************************************************************
int32_t
fillrect_grad()
{
    int32_t x1=0, y1=0;

    color_var_t col0, col1, col2;

    g_sFB.stride = g_sFB.w * nema_format_size(g_sFB.format);
    g_sFB.bo = nema_buffer_create(g_sFB.stride * g_sFB.h);
    if(g_sFB.bo.base_virt == NULL)
    {
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }
    memset(g_sFB.bo.base_virt, 0, g_sFB.stride * g_sFB.h);

    g_sContextCL = nema_cl_create();

    nema_cl_bind(&g_sContextCL);

    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, -1);

    nema_set_clip(0, 0, RESX, RESY);

    nema_enable_gradient(1);

    nema_set_blend_fill(NEMA_BL_SRC);

    for(uint8_t ui8Row=0;ui8Row < GRIDNUM;ui8Row ++)
    {
        y1 = GRIDOFFSET * ui8Row;

        for(uint8_t ui8Col=0;ui8Col < GRIDNUM;ui8Col ++)
        {
            x1 = GRIDOFFSET * ui8Col;

            set_color(&col0,ui8Row * GRIDNUM + ui8Col);
            set_color(&col1,ui8Col * GRIDNUM + ui8Col + 1);
            set_color(&col2,ui8Row * GRIDNUM + ui8Row + 2);

            nema_interpolate_rect_colors(x1, y1, COLORSIZE,COLORSIZE, &col0, &col1, &col2);

            nema_fill_rect(x1, y1, COLORSIZE,COLORSIZE, 0x00000000);
        }
    }

    nema_cl_submit(&g_sContextCL);

    nema_cl_wait(&g_sContextCL);

    //
    // transfer frame to the display
    //
    am_devices_display_transfer_frame(g_sFB.w,
                                      g_sFB.h,
                                      g_sFB.bo.base_phys,
                                      NULL, NULL);
    //
    // wait transfer done
    //
    am_devices_display_wait_transfer_done();

    nema_enable_gradient(0);

    nema_cl_unbind();

    nema_cl_destroy(&g_sContextCL);

    return 0;
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
#ifdef AM_DEBUG_PRINTF
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

    fillrect_grad();

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

