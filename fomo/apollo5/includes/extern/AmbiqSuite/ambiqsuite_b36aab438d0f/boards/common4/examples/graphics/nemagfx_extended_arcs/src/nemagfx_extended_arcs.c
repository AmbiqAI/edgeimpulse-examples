//*****************************************************************************
//
//! @file nemagfx_extended_arcs.c
//!
//! @brief NemaGFX Extended Arcs Example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_extended_arcs NemaGFX Extended Arcs Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example demonstrates the round line end functionality included in the
//! NemaGFX_Extended library. The round line end arc will blit with an
//! alpha channel picture, with full Blending Factors. The start arc angle and
//! end arc angle increases by 1 degree after each iteration in the endless
//! loop.
//!
//! This example has been demonstrated with those devices and configurations
//! 1. Apollo4 and Apollo4P devices use default the MIPI DSI interface
//! 2. Apollo4 Lite devices use MSPI2 for the display interface
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
#include "nemagfx_extended_arcs.h"
#include "ambiq_alpha_240x76_rgba.h"
#include "nemagfx_extensions.h"

#define RESX (g_sDispCfg.ui16ResX)
#define RESY (g_sDispCfg.ui16ResY)

//*****************************************************************************
//
// Variables Definition
//
//*****************************************************************************

static img_obj_t g_sFB;

//*****************************************************************************
//
//! @brief initialize framebuffer
//!
//! This function initialize framebuffer(s)
//!
//! @return status.
//
//*****************************************************************************
static void
init_framebuffer(const am_devices_disp_color_e eDispColor)
{
    g_sFB.w = RESX;
    g_sFB.h = RESY;
    if(eDispColor == COLOR_FORMAT_RGB888)
    {
        g_sFB.format = NEMA_RGB24;
    }
    else if(eDispColor == COLOR_FORMAT_RGB565)
    {
        g_sFB.format = NEMA_RGB565;
    }
    else
    {
        am_util_stdio_printf("Invalid color format!\n");
        while(1);
    }
    g_sFB.stride = RESX * nema_format_size(g_sFB.format);
    g_sFB.bo = nema_buffer_create(g_sFB.stride * g_sFB.h);
    if(g_sFB.bo.base_virt == (void *)NULL)
    {
        //
        // have no enough space.this check is important!
        //
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }
}

//*****************************************************************************
//
//! @brief nemagfx_extended_arc_round_ending_display
//!
//!
//! @return status      - None
//
//*****************************************************************************
void
nemagfx_extended_arc_round_ending_display(void)
{
    nema_cmdlist_t sCL;
	uint32_t col;
	uint32_t x = RESX/2;
	uint32_t y = RESY/2;
	uint32_t w = 20;
	uint32_t i32Radius;

	float start_angle = 30;
	float end_angle = 180;

	img_obj_t sAmbiqLogo = {{0}, 240, 76, -1, 0, NEMA_RGBA8888, 0};

	sAmbiqLogo.bo = nema_buffer_create(sizeof(ambiq_alpha_240x76_rgba));
	if ( (uintptr_t)NULL == sAmbiqLogo.bo.base_phys )
	{
		am_util_debug_printf("Fail to create the assets space\n");
		while(1);			//error
	}
	memcpy((void*)sAmbiqLogo.bo.base_phys, ambiq_alpha_240x76_rgba, sizeof(ambiq_alpha_240x76_rgba));

	while (1)
	{
	    sCL = nema_cl_create();
	    nema_cl_bind(&sCL);
		
		nema_clear(0xFFFFFFFF);

	    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);

	    nema_set_clip(0, 0, RESX, RESY);
	    nema_set_blend_fill(NEMA_BL_SIMPLE);

	    start_angle += 1;
	    end_angle += 1 ;
	
	    col = nema_rgba(0xff, 0x00, 0x00, 0xFF);
	    i32Radius = 80;
		nema_raster_stroked_arc_aa_ex(x, y, i32Radius, w, start_angle, 0, end_angle, 1, col);
		
	    col = nema_rgba(0xff, 0x99, 0x00, 0xFF);
	    i32Radius = 105;
		nema_raster_stroked_arc_aa_ex(x, y, i32Radius, w, start_angle, 1, end_angle, 0, col);
		
	    col = nema_rgba(0x6d, 0x9e, 0xeb, 0xA0);
	    i32Radius = 130;
		nema_raster_stroked_arc_aa_ex(x, y, i32Radius, w, start_angle, 1, end_angle, 1, col);

	    nema_bind_src_tex(sAmbiqLogo.bo.base_phys,
	                      sAmbiqLogo.w,
	                      sAmbiqLogo.h,
	                      sAmbiqLogo.format,
	                      sAmbiqLogo.stride,
	                      NEMA_FILTER_PS);

	    nema_set_blend_blit(NEMA_BL_SIMPLE|NEMA_BLOP_MODULATE_A);
	    nema_blit(104, 90);

	    nema_cl_submit(&sCL);
	    nema_cl_wait(&sCL);

		am_devices_display_transfer_frame(g_sFB.w,
										  g_sFB.h,
										  g_sFB.bo.base_phys,
										  NULL, NULL);
		am_devices_display_wait_transfer_done();
		
	    nema_cl_destroy(&sCL);

	    nema_calculate_fps();
    }

	//
	// remember to destory when out of the loop
	//
    // nema_buffer_destroy(&g_sGreekIslandRGBA8888.bo);

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

// #### INTERNAL BEGIN ####
#if !defined(APOLLO4_FPGA)
// #### INTERNAL END ####
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();
// #### INTERNAL BEGIN ####
#endif
// #### INTERNAL END ####

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // color format
    //
    am_devices_disp_color_e eDispColor = COLOR_FORMAT_RGB565;

#if defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    //
    // DSI output frequency
    //
    g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X12;
#endif

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

#ifdef BAREMETAL
    am_util_stdio_printf("nemafgx_extended_arcs Example\n");
    nemagfx_extended_arc_round_ending_display();
#else // BAREMETAL
    //
    // Initialize plotting interface.
    //
    am_util_debug_printf("FreeRTOS nemafgx_extended_arcs Example\n");

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

