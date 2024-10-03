//*****************************************************************************
//
//! @file nemagfx_breakpoints.c
//!
//! @brief NemaGFX example.
//! In order to further debug applications, NEMA GFX implements support for breakpoints. Breakpoints can
//! be inserted at different positions within a CL in order to pause its execution. When the GPU reaches a
//! breakpoint, it signals the CPU about it and pauses its execution. The GPU will continue its operation when
//! the CPU instructs it so. While the GPU is paused, the user can have an overview of the state of the GPU
//! (by calling respective functions) and the rendering process. In order to use the breakpoints, NemaGFX
//! offers the following functions:
//!
//! void nema_brk_enable(void):    enables the breakpoints.
//! void nema_brk_disable(void):   disables the brakpoints.
//! int nema_brk_add(void):        adds a new breakpoint to the CL and returns its id (positive value).
//! int nema_brk_wait(int brk_id): the CPU waits for the GPU to reach the breakpoint with id
//!                                equal to brk_id. Setting the brk_id = 0 will make the CPU wait until the GPU
//!                                encounters the next breakpoint, regardless of its id. The function returns
//!                                the id of the encountered breakpoint.
//! void nema_brk_continue(void):  instruct the GPU to resume execution.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "nemagfx_breakpoints.h"

#define FB_RESX     200
#define FB_RESY     200
//*****************************************************************************
//
// Variables Definition.
//
//*****************************************************************************
img_obj_t                   g_sFB = {{0}, FB_RESX, FB_RESY, -1, 0, NEMA_RGB24, 0};

//*****************************************************************************
//
//! @brief fill rectangles with breakpoints
//!
//! The following example illustrates the usage of breakpoints.The GPU is programmed
//! to draw three rectangles filled with different colors (red, green and blue ).
//!
//! @return None.
//
//*****************************************************************************
void
fill_rectangles(void)
{
    g_sFB.stride = g_sFB.w * nema_format_size(g_sFB.format);
    g_sFB.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, g_sFB.stride * g_sFB.h);
    if ( g_sFB.bo.base_virt == NULL )
    {
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }
    nema_cmdlist_t cl = nema_cl_create ();
    nema_cl_bind (& cl);

    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, -1);

    nema_set_clip(0, 0, FB_RESX, FB_RESY);

    //
    // Enable breakpoints
    //
    nema_brk_enable ();
    nema_set_blend_fill ( NEMA_BL_SRC );
    //
    // fill red rectangle
    //
    nema_fill_rect (0 , 0, FB_RESX, FB_RESY, nema_rgba (255 , 0, 0, 255) );
    //
    // Add first breakpoint
    //
    int brk1 = nema_brk_add ();
    //
    // fill green rectangle
    //
    nema_fill_rect (0 , 0, FB_RESX / 2, FB_RESX / 2, nema_rgba (0, 255 , 0, 255) );
    //
    // Add second breakpoint
    //
    int brk2 = nema_brk_add ();
    //
    // fill blue rectangle
    //
    nema_fill_rect (0 , 0, FB_RESX / 4, FB_RESX / 4, nema_rgba (0, 0, 255 , 255) );
    //
    // Add third breakpoint
    //
    int brk3 = nema_brk_add ();

    nema_cl_unbind ();
    // Submit Command List for execution
    nema_cl_submit (&cl);

    int brk ;
    //
    // Wait for the GPU to reach the first breakpoint.
    //
    brk = nema_brk_wait ( brk1 );
    //
    // Let the GPU resume execution
    //
    nema_brk_continue ();
    //
    // Wait for the GPU to reach the third breakpoint ( ignore any other breakpoints )
    //
    brk = nema_brk_wait ( brk3 );
    //
    // Let the GPU resume execution
    //
    nema_brk_continue ();
    //
    // Wait for the Command List to finish execution
    //
    nema_cl_wait (& cl);
    nema_cl_destroy (& cl);
    //
    // Disable breakpoints
    //
    nema_brk_disable ();

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
    am_bsp_debug_printf_enable();
#endif

    //
    // Initialize display
    //
    am_devices_display_init(FB_RESX,
                            FB_RESY,
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

    fill_rectangles();

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }

}

