//*****************************************************************************
//
//! @file nemagfx_watchface.c
//!
//! @brief NemaGFX Watchface Example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_watchface NemaGFX Watchface Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example uses one or two frame buffer demonstrate a digital Quartz clock, with
//! Nema GPU support, the shader effect continue shows while timer passing, the
//! function needs a timer to get the accurate time past.
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

#include "nemagfx_watchface.h"
#include "watch.h"

#define AM_DEBUG_PRINTF
//*****************************************************************************
//
// Variables Definition.
//
//*****************************************************************************
static nema_cmdlist_t   cl;
static uint8_t          ui8CurrentIndex = 0;
//
// generally speaking,we opt GPU format RGB24 ,application implement
// watchface effect with single framebuffer.
//
#define FRAME_BUFFERS   1

#ifdef AM_PART_APOLLO4B
    #define GPU_FORMAT      NEMA_RGBA8888
#else
    #define GPU_FORMAT      NEMA_RGB24
#endif

static img_obj_t        sFB[FRAME_BUFFERS];

float start_time;
float end_time;

//*****************************************************************************
//
//! @brief initialize frame buffer(s)
//!
//! This function  initialize  framebuffer(s) for GPU and layers for DC if have.
//!
//! @return None.
//
//*****************************************************************************
static void
load_objects(void)
{
    for (uint8_t i = 0; i < FRAME_BUFFERS; ++i)
    {
        sFB[i].w = RESX;
        sFB[i].h = RESY;
        sFB[i].format = GPU_FORMAT;
        sFB[i].stride = sFB[i].w * nema_format_size(sFB[i].format);
        sFB[i].bo = nema_buffer_create(sFB[i].stride*sFB[i].h);
        (void)nema_buffer_map(&sFB[i].bo);
    }
}
//*****************************************************************************
//
//! @brief render and display watchface
//!
//! This function used to render & display watchface with Dual-/single- Buffer.
//!
//! @return None.
//
//*****************************************************************************
int
watchface(void)
{
    load_objects();
    init_watch(
        1,          //draw backface (bf)
        1,          //bf is texture
        0x10204080  //if (bf) isn't texture, fill with this color
        );
    cl = nema_cl_create_sized(0x100);
// #### INTERNAL BEGIN ####
#if defined(BAREMETAL) && defined(APOLLO4_FPGA)
    am_hal_stimer_config(AM_HAL_STIMER_HFRC_6MHZ | AM_HAL_STIMER_CFG_RUN);
#endif
// #### INTERNAL END ####
    while(1)
    {
        //
        // transfer frame to the display
        //
        am_devices_display_transfer_frame(sFB[ui8CurrentIndex].w,
                                          sFB[ui8CurrentIndex].h,
                                          sFB[ui8CurrentIndex].bo.base_phys,
                                          NULL, NULL);
        //
        // exchange GPU render and display transfer frame buffers.
        //
        ui8CurrentIndex = (ui8CurrentIndex+1) % FRAME_BUFFERS;

        float time = nema_get_time();
        float sec  = time;
        float min  = time / 60;
        float hour = time / 60 / 60;

        //
        //draw_watchface returns a CL pointer that draws the watchface
        //the destination texture needs to be bound beforehand
        //

        nema_cmdlist_t *wf_cl = draw_watchface(hour, min, sec);

        //
        //rewind and bind the CL
        //
        nema_cl_rewind(&cl);
        nema_cl_bind(&cl);

        //
        //bind the destination buffer
        //
        nema_bind_dst_tex(sFB[ui8CurrentIndex].bo.base_phys,
                          sFB[ui8CurrentIndex].w,
                          sFB[ui8CurrentIndex].h,
                          sFB[ui8CurrentIndex].format,
                          sFB[ui8CurrentIndex].stride);

        //
        //branch to the watchface command list
        //
        nema_cl_branch(wf_cl);
#if (FRAME_BUFFERS == 1)
        //
        // wait transfer done
        //
        am_devices_display_wait_transfer_done();
#endif
        //
        //submit command list
        //
        nema_cl_submit(&cl);
        //
        //wait GPU render completed
        //
        nema_cl_wait(&cl);
        nema_calculate_fps();
#if (FRAME_BUFFERS == 2)
        //
        // wait transfer done
        //
        am_devices_display_wait_transfer_done();
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
    am_bsp_external_pwr_on();
    am_util_delay_ms(100);
    am_bsp_low_power_init();
    //
    // Initialize the printf interface for ITM/SWO output.
    //
    //am_bsp_itm_printf_enable();
    //
    // Clear the terminal and print the banner.
    //
    //am_util_stdio_terminal_clear();
#ifdef AM_DEBUG_PRINTF
    //
    // Initialize a debug printing interface.
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

#ifdef BAREMETAL
    watchface();
#else //!< BAREMETAL
    //
    // Initialize plotting interface.
    //
    am_util_debug_printf("FreeRTOS NemaGFX_SW Transition Effect Example\n");

    //
    // Run the application.
    //
    run_tasks();
#endif //!< BAREMETAL

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

