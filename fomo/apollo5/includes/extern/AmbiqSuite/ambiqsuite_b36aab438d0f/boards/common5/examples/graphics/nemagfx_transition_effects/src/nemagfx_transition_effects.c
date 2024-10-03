//*****************************************************************************
//
//! @file nemagfx_transition_effects.c
//!
//! @brief this example use one frame buffer demonstrate two picture trasition
//! effect, with Nema GPU support, the effect include
//! NEMA_TRANS_LINEAR_H,
//! NEMA_TRANS_CUBE_H,
//! NEMA_TRANS_INNERCUBE_H,
//! NEMA_TRANS_STACK_H,
//! NEMA_TRANS_LINEAR_V,
//! NEMA_TRANS_CUBE_V,
//! NEMA_TRANS_INNERCUBE_V,
//! NEMA_TRANS_STACK_V,
//! NEMA_TRANS_FADE,
//! NEMA_TRANS_FADE_ZOOM,
//! NEMA_TRANS_MAX,
//! NEMA_TRANS_NONE
//!
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
#include "nemagfx_transition_effects.h"
#include "ant_bgr24.h"
#include "beach_bgr24.h"
//*****************************************************************************
//
// Variables Definition.
//
//*****************************************************************************
static img_obj_t            g_sScreen0 = {{0}, 256, 256, -1, 0, (uint8_t)NEMA_RGB24, 0};
static img_obj_t            g_sScreen1 = {{0}, 256, 256, -1, 0, (uint8_t)NEMA_RGB24, 0};

static img_obj_t            g_sFB[FRAME_BUFFERS];
static nema_cmdlist_t       g_sCLDrawEntireScene;
static nema_transition_t    g_eEffect = NEMA_TRANS_LINEAR_H;
static nema_cmdlist_t       g_sCL;

static void
load_objects(void)
{
    for (int32_t i = 0; i < FRAME_BUFFERS; ++i)
    {
        g_sFB[i].w = RESX;
        g_sFB[i].h = RESY / SMALLFB_STRIPES;
        g_sFB[i].format = NEMA_RGB24;

        g_sFB[i].stride = nema_format_size(g_sFB[i].format) * g_sFB[i].w;

        g_sFB[i].bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, g_sFB[i].stride * g_sFB[i].h);

        (void)nema_buffer_map(&g_sFB[i].bo);

    }

// #### INTERNAL BEGIN ####
#if defined(LOAD_FROM_MRAM)
    //
    // Set the textures address in MRAM for GPU
    //
    g_sScreen0.bo.base_phys = (uintptr_t)g_ui8Ant_bgr24;
    g_sScreen0.bo.base_virt = (void*)g_sScreen0.bo.base_phys;
    g_sScreen1.bo.base_phys = (uintptr_t)g_ui8Beach_bgr24;
    g_sScreen1.bo.base_virt = (void*)g_sScreen1.bo.base_phys;
#elif defined(LOAD_FROM_PSRAM)
    g_sScreen0.bo.base_phys = (uintptr_t)MSPI_XIP_BASE_ADDRESS;
    g_sScreen0.bo.base_virt = (void*)g_sScreen0.bo.base_phys;
    g_sScreen1.bo.base_phys = (uintptr_t)(MSPI_XIP_BASE_ADDRESS + sizeof(g_ui8Ant_bgr24));
    g_sScreen1.bo.base_virt = (void*)g_sScreen1.bo.base_phys;


    nema_memcpy((void*)g_sScreen0.bo.base_phys, g_ui8Ant_bgr24, sizeof(g_ui8Ant_bgr24));
    nema_memcpy((void*)g_sScreen1.bo.base_phys, g_ui8Beach_bgr24, sizeof(g_ui8Beach_bgr24));

#else
// #### INTERNAL END ####
    //
    // Preload the textures from MRAM to SSRAM
    // Set the textures address in SSRAM for GPU
    //

    g_sScreen0.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, sizeof(g_ui8Ant_bgr24));
    nema_memcpy((void*)g_sScreen0.bo.base_phys, g_ui8Ant_bgr24, sizeof(g_ui8Ant_bgr24));
    g_sScreen1.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, sizeof(g_ui8Beach_bgr24));
    nema_memcpy((void*)g_sScreen1.bo.base_phys, g_ui8Beach_bgr24, sizeof(g_ui8Beach_bgr24));

// #### INTERNAL BEGIN ####
#endif
// #### INTERNAL END ####
}

//*****************************************************************************
//
//! @brief render frame buffers and display transition effects.
//!
//! @param  step            - transition effects step.
//!
//! this function could render and display concurrently through Dual-buffers,
//! or render and display serially through single-buffer.
//!
//! @return None.
//
//*****************************************************************************
static void
display(float step)
{
    static bool is_first_frame = true;
    uint8_t ui8CurrentIndex = 0;
    for (int32_t stripe = 0; stripe < SMALLFB_STRIPES; ++stripe)
    {
        nema_cl_bind(&g_sCLDrawEntireScene);
        nema_cl_rewind(&g_sCLDrawEntireScene);
        nema_bind_tex(NEMA_TEX1,
                  g_sScreen0.bo.base_phys + stripe * g_sScreen0.bo.size / SMALLFB_STRIPES,
                  g_sScreen0.w,
                  g_sScreen0.h,
                  g_sScreen0.format,
                  g_sScreen0.stride,
                  NEMA_FILTER_BL | NEMA_TEX_BORDER);

        nema_bind_tex(NEMA_TEX2,
                  g_sScreen1.bo.base_phys + stripe * g_sScreen1.bo.size / SMALLFB_STRIPES,
                  g_sScreen1.w,
                  g_sScreen1.h,
                  g_sScreen1.format,
                  g_sScreen1.stride,
                  NEMA_FILTER_BL | NEMA_TEX_BORDER);

        nema_set_tex_color(0);

        nema_transition(g_eEffect,
                        NEMA_TEX1,
                        NEMA_TEX2,
                        NEMA_BL_SRC,
                        step,
                        RESX,
                        RESY / SMALLFB_STRIPES);

        nema_cl_bind(&g_sCL);
        nema_cl_rewind(&g_sCL);
        //
        // Bind Framebuffer
        //
        nema_bind_dst_tex(g_sFB[ui8CurrentIndex].bo.base_phys,
                          g_sFB[ui8CurrentIndex].w,
                          g_sFB[ui8CurrentIndex].h,
                          g_sFB[ui8CurrentIndex].format,
                          g_sFB[ui8CurrentIndex].stride);

        //
        // Set Clipping Rectangle
        //
        nema_set_clip(0,
                      0,
                      RESX,
                      RESY / SMALLFB_STRIPES);

        nema_cl_jump(&g_sCLDrawEntireScene);

        //
        // make sure display transfer is completed before submit next cl.
        //
        if ( is_first_frame == false )
        {
            am_devices_display_wait_transfer_done();
        }
        nema_cl_submit(&g_sCL);
#if (FRAME_BUFFERS == 1)
        //
        //wait GPU render completed before transfer frame when using singlebuffer.
        //
        nema_cl_wait(&g_sCL);
#endif
#if (SMALLFB_STRIPES != 1)
        //
        // Reposition the display area if necessary
        //
        am_devices_display_set_region(RESX,
                                      RESY / SMALLFB_STRIPES,
                                      PANEL_OFFSET,
                                      PANEL_OFFSET + RESY / SMALLFB_STRIPES * stripe);
#endif
        //
        // transfer frame to the display
        //
        am_devices_display_transfer_frame(g_sFB[ui8CurrentIndex].w,
                                          g_sFB[ui8CurrentIndex].h,
                                          g_sFB[ui8CurrentIndex].bo.base_phys,
                                          NULL,
                                          NULL);
        is_first_frame = false;
        ui8CurrentIndex = (ui8CurrentIndex + 1) % FRAME_BUFFERS;
#if (FRAME_BUFFERS > 1)
        //
        //wait GPU render completed after transfer frame when using dualbuffers.
        //
        nema_cl_wait(&g_sCL);
#endif

    }
}

//*****************************************************************************
//
//! @brief change transition effects.
//!
//! this function used to change transition effects,when user defined macro
//! SMALLFB_STRIPES isn't equivalent to 1.please note that some transition effects
//! could be not suit for small frame buffer.
//!
//! @return None.
//
//*****************************************************************************
static void
next_effect(void)
{
#if (SMALLFB_STRIPES != 1)
    if ( g_eEffect == NEMA_TRANS_LINEAR_H )
    {
        g_eEffect = NEMA_TRANS_LINEAR_V;
    }
    else
    {
        g_eEffect = NEMA_TRANS_LINEAR_H;
    }
#else
    g_eEffect = (nema_transition_t)(((int32_t)g_eEffect + 1) % NEMA_TRANS_MAX);
#endif
}

void
transition_effects(void)
{
    float step = MIN_STEP;
    float step_step = ANIMATION_STEP_0_1;

    load_objects();

    g_sCLDrawEntireScene  = nema_cl_create();
    g_sCL  = nema_cl_create();

    while (1)
    {
        display(step);

        if (step <= MIN_STEP)
        {
            step = MIN_STEP;
            step_step = ANIMATION_STEP_0_1;
            next_effect();
        }
        else if (step >= MAX_STEP)
        {
            step = MAX_STEP;
            step_step = -ANIMATION_STEP_0_1;
            next_effect();
        }

        step += step_step;

        nema_calculate_fps();
    }
}

// #### INTERNAL BEGIN ####
#ifdef LOAD_FROM_PSRAM

//uint32_t        DMATCBBuffer[2560];
void            *g_pDevHandle;
void            *g_pHandle;

//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
};


am_devices_mspi_psram_config_t MSPI_PSRAM_HexCE1MSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_48MHZ,
    .ui32NBTxnBufLength       = 0,
    .pNBTxnBuf                = NULL,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};


//
// Take over the interrupt handler for whichever MSPI we're using.
//
#define psram_mspi_isr                                                          \
    am_mspi_isr1(MSPI_PSRAM_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi ## n ## _isr

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void psram_mspi_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_pHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_pHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_pHandle, ui32Status);
}

#endif // LOAD_FROM_PSRAM
// #### INTERNAL END ####

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

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

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

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Power up GPU
    //
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

    //
    // Initialize NemaGFX
    //
    nema_init();

// #### INTERNAL BEGIN ####
#ifdef LOAD_FROM_PSRAM
    //
    // Init PSRAM device.
    //
    uint32_t ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_HexCE1MSPIConfig, &g_pDevHandle, &g_pHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_SetPriority(mspi_interrupts[MSPI_PSRAM_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[MSPI_PSRAM_MODULE]);

    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pDevHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == ui32Status)
    {
        am_util_stdio_printf("Enable XIP mode in the MSPI!\n");
    }
    else
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }
#endif // LOAD_FROM_PSRAM
// #### INTERNAL END ####

#ifdef BAREMETAL
    //
    // Show the transition effect.
    //
    transition_effects();
#else //!< BAREMETAL
    //
    // Initialize plotting interface.
    //
    am_util_debug_printf("FreeRTOS Nemagfx Transition Effect Example\n");

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

