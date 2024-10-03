//*****************************************************************************
//
//! @file nemagfx_transition_effects.c
//!
//! @brief NemaGFX Transition Effects Example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_transition_effects NemaGFX Transition Effects Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example use one frame buffer demonstrate two picture trasition
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
#ifdef AM_PART_APOLLO4B
    #include "im00.rgba565.h"
    #include "im01.rgba565.h"
#else
    #include "ant_bgr24.h"
    #include "beach_bgr24.h"
#endif
//*****************************************************************************
//
// Variables Definition.
//
//*****************************************************************************
#ifdef AM_PART_APOLLO4B
static img_obj_t            g_sScreen0 = {{0},256, 256, -1, 0, (uint8_t)NEMA_RGB565, 0};
static img_obj_t            g_sScreen1 = {{0},256, 256, -1, 0, (uint8_t)NEMA_RGB565, 0};
#else
static img_obj_t            g_sScreen0 = {{0},256, 256, -1, 0, (uint8_t)NEMA_RGB24, 0};
static img_obj_t            g_sScreen1 = {{0},256, 256, -1, 0, (uint8_t)NEMA_RGB24, 0};
#endif

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
#if defined (AM_PART_APOLLO4B)
        g_sFB[i].format = NEMA_RGB565;
#else
        g_sFB[i].format = NEMA_RGB24;
#endif
        g_sFB[i].stride = nema_format_size(g_sFB[i].format) * g_sFB[i].w;

        g_sFB[i].bo = nema_buffer_create(g_sFB[i].stride * g_sFB[i].h);

        (void)nema_buffer_map(&g_sFB[i].bo);

    }

// #### INTERNAL BEGIN ####
//#define LOAD_FROM_DSP_SRAM
#if defined(LOAD_FROM_DSP_SRAM)
    //
    // Preload the textures from MRAM to DSP_SRAM
    // Set the textures address in DSP_SRAM for GPU
    //
    g_sScreen0.bo.base_phys = (uintptr_t)0x10160000;
    g_sScreen0.bo.base_virt = (void*)g_sScreen0.bo.base_phys;
    g_sScreen1.bo.base_phys = (uintptr_t)(0x10160000 + sizeof(g_ui8Ant_bgr24));
    g_sScreen1.bo.base_virt = (void*)g_sScreen1.bo.base_phys;

    //g_sScreen0.bo = nema_buffer_create(IM00_RGBA565_LEN);
    memcpy((void*)g_sScreen0.bo.base_phys, g_ui8Ant_bgr24, sizeof(g_ui8Ant_bgr24));
    //g_sScreen1.bo = nema_buffer_create(IM01_RGBA565_LEN);
    memcpy((void*)g_sScreen1.bo.base_phys, g_ui8Beach_bgr24, sizeof(g_ui8Beach_bgr24));
#elif defined(LOAD_FROM_MRAM)
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

    memcpy((void*)g_sScreen0.bo.base_phys, g_ui8Ant_bgr24, sizeof(g_ui8Ant_bgr24));
    memcpy((void*)g_sScreen1.bo.base_phys, g_ui8Beach_bgr24, sizeof(g_ui8Beach_bgr24));

#else
// #### INTERNAL END ####
    //
    // Preload the textures from MRAM to SSRAM
    // Set the textures address in SSRAM for GPU
    //
#ifdef AM_PART_APOLLO4B
    g_sScreen0.bo = nema_buffer_create(sizeof(g_ui8Im00RGBA565));
    memcpy((void*)g_sScreen0.bo.base_phys, g_ui8Im00RGBA565, sizeof(g_ui8Im00RGBA565));
    g_sScreen1.bo = nema_buffer_create(sizeof(g_ui8Im01RGBA565));
    memcpy((void*)g_sScreen1.bo.base_phys, g_ui8Im01RGBA565, sizeof(g_ui8Im01RGBA565));
#else
    g_sScreen0.bo = nema_buffer_create(sizeof(g_ui8Ant_bgr24));
    memcpy((void*)g_sScreen0.bo.base_phys, g_ui8Ant_bgr24, sizeof(g_ui8Ant_bgr24));
    g_sScreen1.bo = nema_buffer_create(sizeof(g_ui8Beach_bgr24));
    memcpy((void*)g_sScreen1.bo.base_phys, g_ui8Beach_bgr24, sizeof(g_ui8Beach_bgr24));
#endif
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
        if(is_first_frame == false)
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
        ui8CurrentIndex = (ui8CurrentIndex +1) % FRAME_BUFFERS;
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
    if(g_eEffect == NEMA_TRANS_LINEAR_H)
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

// #### INTERNAL BEGIN ####
#if defined(BAREMETAL) && defined(APOLLO4_FPGA)
    am_hal_stimer_config(AM_HAL_STIMER_HFRC_6MHZ | AM_HAL_STIMER_CFG_RUN);
#endif
// #### INTERNAL END ####
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

uint32_t        DMATCBBuffer[2560];
void            *g_pDevHandle;
void            *g_pHandle;

//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
};

#ifndef AM_PART_APOLLO4P
am_devices_mspi_psram_config_t MSPI_PSRAM_QuadCE1MSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_QUAD_CE1,
    .eClockFreq               = AM_HAL_MSPI_CLK_48MHZ,
    .ui32NBTxnBufLength       = sizeof(DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};
#else
am_devices_mspi_psram_config_t MSPI_PSRAM_HexCE1MSPIConfig =
{
#ifdef APS25616N_OCTAL_MODE_EN
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1,
#else
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE1,
#endif
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};
#endif

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

// #### INTERNAL BEGIN ####
void
vddc_vddf_trim(void)
{
    am_util_delay_ms(3000);
    //#warning: need AI key  // AI Key
    //#warning: need AI key  // AI Key
    //#warning: need AI key  // AI Key
    //#warning: need AI key  // AI Key
    //mcuctrlpriv.ldoreg1.coreldoactivetrim 0x40040080 bit9~0
    uint32_t ui32Val0 = AM_REGVAL(0x40020080);
    //AM_REGVAL(0x40020080) = ui32Val0 + 24; //22; //!< please increase trim value with a proper value, to increase VDDC to 750mV.
    am_util_delay_ms(100);
    //mcuctrlpriv.ldoreg2.memldoactivetrim 0x40040088 bit5~0
    ui32Val0 = AM_REGVAL(0x40020088);
    AM_REGVAL(0x40020088) = ui32Val0 + 20;  //28; //!< please increase trim value with a proper value, to increase VDDF to 950mV.
    am_util_delay_ms(100);
}

typedef uint64_t  u64           ;
typedef uint32_t  phys_addr_t   ;

static u64 g_ui64Patterns[]  =
{
    //!< The first entry has to be 0 to leave memtest with zeroed memory
    0,
    0xffffffffffffffffULL,
    0x5555555555555555ULL,
    0xaaaaaaaaaaaaaaaaULL,
    0x1111111111111111ULL,
    0x2222222222222222ULL,
    0x4444444444444444ULL,
    0x8888888888888888ULL,
    0x3333333333333333ULL,
    0x6666666666666666ULL,
    0x9999999999999999ULL,
    0xccccccccccccccccULL,
    0x7777777777777777ULL,
    0xbbbbbbbbbbbbbbbbULL,
    0xddddddddddddddddULL,
    0xeeeeeeeeeeeeeeeeULL,
    0x7a6c7258554e494cULL, /* yeah ;-) */
};

static void
reserve_bad_mem(u64 pattern, phys_addr_t start_bad, phys_addr_t end_bad)
{
    while(1);
}

#define ALIGN(x, a)     (((x) + (a) - 1) & ~(a - 1))
static void
memtest(u64 pattern, phys_addr_t start_phys, phys_addr_t size)
{
    u64 *p, *start, *end;
    phys_addr_t start_bad, last_bad;
    phys_addr_t start_phys_aligned;
    const size_t incr = sizeof(pattern);
    start_phys_aligned = ALIGN(start_phys, incr);
    start = (u64*)(start_phys_aligned);
    end = start + (size - (start_phys_aligned - start_phys)) / incr;
    start_bad = 0;
    last_bad = 0;
    for (p = start; p < end; p++)
    {
        *p = pattern;
    }

    for (p = start; p < end; p++, start_phys_aligned += incr)
    {
        if (*p == pattern)
        {
            continue;
        }
        if (start_phys_aligned == last_bad + incr)
        {
            last_bad += incr;
            continue;
        }
        if (start_bad)
        {
            reserve_bad_mem(pattern, start_bad, last_bad + incr);
        }
        start_bad = last_bad = start_phys_aligned;
    }

    if (start_bad)
    {
        reserve_bad_mem(pattern, start_bad, last_bad + incr);
    }
}
// #### INTERNAL END ####

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32Status;
    //
    // External power on
    //
    am_bsp_external_pwr_on();
    am_util_delay_ms(100);
    am_bsp_low_power_init();
// #### INTERNAL BEGIN ####
    //
    // if DSI or MRAM textures are used, elevate VDDF
    //
    //vddc_vddf_trim();
#ifndef AM_PART_APOLLO4L
    //
    // Initialize DSP RAM.
    //
    am_hal_pwrctrl_dsp_memory_config_t sCfg = g_DefaultDSPMemCfg;
    sCfg.bEnableICache = 0;
    sCfg.bRetainCache = 0;
    am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP0, &sCfg);
    am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP1, &sCfg);

    for (int32_t i = 0; i < sizeof(g_ui64Patterns) / sizeof(g_ui64Patterns[0]); i++)
    {
        memtest(g_ui64Patterns[i], 0x10160000, 480 * 1024);
    }
#endif
// #### INTERNAL END ####

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
#ifdef AM_PART_APOLLO4B
    //
    // DSI output frequency
    //
    g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X12;
    am_devices_display_init(RESX,
                            RESY,
                            COLOR_FORMAT_RGB565,
                            false);
#else
    am_devices_display_init(RESX,
                            RESY,
                            COLOR_FORMAT_RGB888,
                            false);
#endif

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Initialize NemaGFX
    //
    nema_init();

// #### INTERNAL BEGIN ####
#ifdef LOAD_FROM_PSRAM
    //
    // Init PSRAM device.
    //
#ifndef AM_PART_APOLLO4P
    ui32Status = am_devices_mspi_psram_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_QuadCE1MSPIConfig, &g_pDevHandle, &g_pHandle);
#else
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_HexCE1MSPIConfig, &g_pDevHandle, &g_pHandle);
#endif
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_SetPriority(mspi_interrupts[MSPI_PSRAM_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[MSPI_PSRAM_MODULE]);

    //
    // Enable XIP mode.
    //
#ifndef AM_PART_APOLLO4P
    ui32Status = am_devices_mspi_psram_enable_xip(g_pDevHandle);
#else
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pDevHandle);
#endif
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
    am_hal_timer_config_t sTimerConfig;
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

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

