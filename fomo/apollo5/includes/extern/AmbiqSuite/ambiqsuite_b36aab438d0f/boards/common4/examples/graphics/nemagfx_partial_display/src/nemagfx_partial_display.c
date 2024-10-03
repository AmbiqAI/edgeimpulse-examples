//*****************************************************************************
//
//! @file nemagfx_partial_display.c
//!
//! @brief NemaGFX Partial Display Example
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_partial_display NemaGFX Partial Display Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example demostrate the  partial frame update feature not only the
//! horizontal direction but also vertical direction
//! PURE_COLOR_MODE selected for fill color mode
//! TEXTURE_MODE selected for the texture blending mode
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
#include "nemagfx_partial_display.h"
#include "chips_456x456_rgba565.h"
#include "arial48pt1b.h"

#define RESX 456
#define RESY 456

// Divide the rendered scene by:
#define DIV_X 4
#define DIV_Y 4

#define TILE_SIZE_X (RESX/DIV_X)
#define TILE_SIZE_Y (RESY/DIV_Y)

typedef enum
{
    PURE_COLOR_MODE = 0,
    TEXTURE_MODE
}
partial_blend_mode;

//*****************************************************************************
//
// Variables Definition
//
//*****************************************************************************

static img_obj_t g_sFB;

static float get_fps(void)
{
    static float fps   = 0;
    static int   frame = 0;

    ++frame;
    if ( frame%100 == 0 ) {
        static float start_time = 0.f;
        static float stop_time  = 0.f;

        stop_time = nema_get_time();
        fps = 100.f/(stop_time-start_time);
        // am_util_stdio_printf("\nfps: %.02f\n", fps);
        start_time = stop_time;
    }

    return fps;
}

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
    g_sFB.stride = TILE_SIZE_X * nema_format_size(g_sFB.format);
    g_sFB.bo = nema_buffer_create(g_sFB.stride * g_sFB.h / DIV_Y);
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
//! @brief render balls and transfer frame to display.
//!
//! draw all balls move in display region,it can bounce back balls when these out of
//! bounds. the application also printing some interval time,especially fps value.
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
void
partial_display_select_mode(partial_blend_mode ePmode)
{
    partial_blend_mode ePartialBlendMode = ePmode;

    img_obj_t obj_bg = {{0}, 456, 456, -1, 0, NEMA_RGBA5650, NEMA_FILTER_PS};

    obj_bg.bo = nema_buffer_create(chips_456x456_rgba565_length);
    memcpy(obj_bg.bo.base_virt, chips_456x456_rgba565, chips_456x456_rgba565_length);

    nema_cmdlist_t cl  = nema_cl_create();

    // Create a command list for the rendered scene
    // ==============================================
    nema_cmdlist_t rendered_scene  = nema_cl_create();

    nema_cl_bind(&rendered_scene);
    nema_cl_rewind(&rendered_scene);

	if ( ePartialBlendMode == PURE_COLOR_MODE )
	{
		nema_bind_src_tex(obj_bg.bo.base_phys, obj_bg.w, obj_bg.h, obj_bg.format, obj_bg.stride, obj_bg.sampling_mode);
	    nema_set_blend_blit(NEMA_BL_SIMPLE);
	    nema_blit(0, 0);
	
	    nema_set_blend_fill(NEMA_BL_SIMPLE);
	    nema_fill_rect(0     , 0     , RESX/2, RESY/2, nema_rgba(0xff, 0    , 0   , 0x7A));
	    nema_fill_rect(RESX/2, 0     , RESX/2, RESY/2, nema_rgba(0   , 0xff , 0   , 0x7A));
	    nema_fill_rect(0     , RESY/2, RESX/2, RESY/2, nema_rgba(0   , 0    , 0xff, 0x7A));
	    nema_fill_rect(RESX/2, RESY/2, RESX/2, RESY/2, nema_rgba(0   , 0xff , 0xff, 0x7A));
	    nema_fill_rect(RESX/4, RESY/4, RESX/2, RESY/2, nema_rgba(0   , 0    , 0   , 0x37));
	}
	else if ( ePartialBlendMode == TEXTURE_MODE )
	{
	    nema_bind_src_tex(obj_bg.bo.base_phys, obj_bg.w, obj_bg.h, obj_bg.format, obj_bg.stride, obj_bg.sampling_mode);
	    nema_set_blend_blit(NEMA_BL_SIMPLE);
	    nema_blit(0, 0);
	}
	else
	{
		am_util_stdio_printf("Mode not supported\n");
		while(1);
	}	

    nema_cl_unbind();

    for (int offset_clip_y = 0; offset_clip_y < RESY; offset_clip_y += TILE_SIZE_Y)
	{
        for (int offset_clip_x = 0; offset_clip_x < RESX; offset_clip_x += TILE_SIZE_X)
		{
            nema_cl_bind(&cl);
            nema_cl_rewind(&cl);

            uint32_t base_addr;

            base_addr = (g_sFB.bo.base_phys - (offset_clip_y*g_sFB.stride) - (offset_clip_x*nema_format_size(g_sFB.format)));

            nema_bind_dst_tex(base_addr,
							   g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);
			
            nema_set_clip(offset_clip_x, offset_clip_y, TILE_SIZE_X, TILE_SIZE_Y);
			
            uint8_t run_once = 0;

            if (run_once == 0)
            {
                nema_cl_branch(&rendered_scene);
                run_once = 1;
            }
			
            nema_cl_submit(&cl);
            nema_cl_wait(&cl);

            am_devices_display_set_region(TILE_SIZE_X, TILE_SIZE_Y, offset_clip_x, offset_clip_y);
    
            am_devices_display_transfer_frame(g_sFB.w / DIV_X,
                                              g_sFB.h / DIV_Y,
                                              g_sFB.bo.base_phys,
                                              NULL, NULL);

            am_devices_display_wait_transfer_done();
        }
    }

    nema_buffer_destroy(&obj_bg.bo);
    nema_cl_destroy (&rendered_scene);
    nema_cl_destroy (&cl);

#ifndef BAREMETAL
    taskYIELD();
#endif
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
partial_display_area_update(void)
{
    img_obj_t obj_bg = {{0}, 456, 456, -1, 0, NEMA_RGBA5650, NEMA_FILTER_PS};

    obj_bg.bo = nema_buffer_create(chips_456x456_rgba565_length);
    memcpy(obj_bg.bo.base_virt, chips_456x456_rgba565, chips_456x456_rgba565_length);

    uint8_t ui8_alpha = 0;

    nema_cmdlist_t cl  = nema_cl_create();

    // Create a command list for the rendered scene
    // ==============================================
    nema_cmdlist_t rendered_scene  = nema_cl_create();

    nema_cl_bind(&rendered_scene);
    nema_cl_rewind(&rendered_scene);

    nema_bind_src_tex(obj_bg.bo.base_phys, obj_bg.w, obj_bg.h, obj_bg.format, obj_bg.stride, obj_bg.sampling_mode);
    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit(0, 0);

    nema_set_blend_fill(NEMA_BL_SIMPLE);
    nema_fill_rect(0     , 0     , RESX/2, RESY/2, nema_rgba(0xff, 0    , 0   , ui8_alpha));
    nema_fill_rect(RESX/2, 0     , RESX/2, RESY/2, nema_rgba(0   , 0xff , 0   , ui8_alpha));
    nema_fill_rect(0     , RESY/2, RESX/2, RESY/2, nema_rgba(0   , 0    , 0xff, ui8_alpha));
    nema_fill_rect(RESX/2, RESY/2, RESX/2, RESY/2, nema_rgba(0   , 0xff , 0xff, ui8_alpha));
    nema_fill_rect(RESX/4, RESY/4, RESX/2, RESY/2, nema_rgba(0   , 0    , 0   , ui8_alpha));

    nema_cl_unbind();

    char disp_buf[32] = {0};
    int32_t rooling_num = 0;
    int iGetfps;

	while (1)
	{
	    for (int offset_clip_y = TILE_SIZE_Y; offset_clip_y < RESY - TILE_SIZE_Y; offset_clip_y += TILE_SIZE_Y)
		{
	        for (int offset_clip_x = TILE_SIZE_X; offset_clip_x < RESX - TILE_SIZE_X; offset_clip_x += TILE_SIZE_X)
			{
	            nema_cl_bind(&cl);
	            nema_cl_rewind(&cl);

				uint32_t base_addr;

				base_addr = (g_sFB.bo.base_phys - (offset_clip_y*g_sFB.stride) - (offset_clip_x*nema_format_size(g_sFB.format)));

	            nema_bind_dst_tex(base_addr,
								   g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);
				
	            nema_set_clip(offset_clip_x, offset_clip_y, TILE_SIZE_X, TILE_SIZE_Y);
				
				nema_bind_src_tex(obj_bg.bo.base_phys, obj_bg.w, obj_bg.h, obj_bg.format, obj_bg.stride, obj_bg.sampling_mode);
				nema_set_blend_blit(NEMA_BL_SIMPLE);
				nema_blit(0, 0);

				sprintf(disp_buf, "%d", (int)rooling_num);
				if ( ++rooling_num > 999)
				{
					rooling_num = 0;
				}
				nema_bind_font(&g_sArial48pt1b);				
				nema_set_blend_fill(NEMA_BL_SIMPLE);
				nema_print(disp_buf, TILE_SIZE_X, TILE_SIZE_Y, RESX, RESY, 0xFF00FFFF, 0);
				nema_print("FPS:", TILE_SIZE_X, TILE_SIZE_Y + 48 + 52, RESX, RESY, 0x2000FFFF, 0);

				sprintf(disp_buf, "%.3f", get_fps()/4);           // updated 4 rectangle area
				nema_print(disp_buf, TILE_SIZE_X, TILE_SIZE_Y + 96 + 36 + 36, RESX, RESY, 0x2000FFFF, 0);
				
	            nema_cl_submit(&cl);
	            nema_cl_wait(&cl);

				am_devices_display_set_region(TILE_SIZE_X, TILE_SIZE_Y, offset_clip_x, offset_clip_y);
				
				am_devices_display_transfer_frame(g_sFB.w / DIV_X,
												  g_sFB.h / DIV_Y,
												  g_sFB.bo.base_phys,
												  NULL, NULL);
				am_devices_display_wait_transfer_done();
	        }
	    }
		
		nema_calculate_fps();

		#ifndef BAREMETAL
	    	taskYIELD();
		#endif
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
partial_display(void)
{
    partial_display_select_mode(TEXTURE_MODE);
    am_util_delay_ms(1000);
    partial_display_select_mode(PURE_COLOR_MODE);
    partial_display_area_update();
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
    am_util_stdio_printf("nemafgx_partial_display Example\n");
    partial_display();
#else // BAREMETAL
    //
    // Initialize plotting interface.
    //
    am_util_debug_printf("FreeRTOS nemafgx_partial_display Example\n");

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
