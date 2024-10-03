//*****************************************************************************
//
//! @file daxi_transition_effects.c
//!
//! @brief Run nemagfx_transition_effects with vary DAXI settings, record the
//! overall run time for each DAXI setting.
//!
//! The frame buffer is in PSRAM and texture is in SSRAM by default. This test
//! also supports cache test, please enable macro "CACHE_TEST" in config-template.ini
//! if you want to run it.
//!
//! We can also run this test in XIP mode by loading bin into PSRAM with the
//! loader example in
//! boards\common4\examples\interfaces\mspi_ddr_psram_loader
//! and following the guideline in this path.
//!
//! The transition effects include
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
#include "am_devices_mspi_psram_aps25616n.h"
#include "daxi_test_common.h"

// #### INTERNAL BEGIN ####
#if (defined(LOAD_FROM_PSRAM) || defined(FB_IN_PSRAM)) && !defined(XIP_EXECUTION)

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

#endif // (defined(LOAD_FROM_PSRAM) || defined(FB_IN_PSRAM)) && !defined(XIP_EXECUTION)

//*****************************************************************************
//
// Optional Global setup.
//
// globalSetUp() will get called before the test group starts, and
//
//*****************************************************************************
void
globalSetUp(void)
{
    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo4 DAXI Test Cases\n\n");

    //
    // Enable global IRQ.
    //
    am_hal_interrupt_master_enable();

}
//*****************************************************************************
//
// Variables Definition.
//
//*****************************************************************************
static img_obj_t            g_sScreen0 = {{0},256, 256, -1, 0, (uint8_t)NEMA_RGB24, 0};
static img_obj_t            g_sScreen1 = {{0},256, 256, -1, 0, (uint8_t)NEMA_RGB24, 0};

static img_obj_t            g_sFB[FRAME_BUFFERS];
static nema_cmdlist_t       g_sCLDrawEntireScene;
static nema_transition_t    g_eEffect = NEMA_TRANS_LINEAR_H;
static nema_cmdlist_t       g_sCL;

static void
load_objects(void)
{
#ifdef FB_IN_PSRAM
    uint32_t ui32CurStartAddr;
    ui32CurStartAddr = MSPI_XIPMM_BASE_ADDRESS + g_i32Ant_bgr24_length + g_i32Beach_bgr24_length;
#endif
    for (int32_t i = 0; i < FRAME_BUFFERS; ++i)
    {
        g_sFB[i].w = RESX;
        g_sFB[i].h = RESY / SMALLFB_STRIPES;
        g_sFB[i].format = NEMA_RGB24;
        g_sFB[i].stride = nema_format_size(g_sFB[i].format) * g_sFB[i].w;

#ifndef FB_IN_PSRAM
#ifdef SMALLFB
        g_sFB[i].bo = nema_buffer_create(g_sFB[i].stride * g_sFB[i].h / SMALLFB_STRIPES);
#else
        g_sFB[i].bo = nema_buffer_create(g_sFB[i].stride * g_sFB[i].h);
#endif

        (void)nema_buffer_map(&g_sFB[i].bo);
#else

#ifdef SMALLFB

        g_sFB[i].bo.base_phys = ui32CurStartAddr + g_sFB[i].stride * g_sFB[i].h / SMALLFB_STRIPES;
        g_sFB[i].bo.base_virt = (void *)g_sFB[i].bo.base_phys;
#else
        g_sFB[i].bo.base_phys = ui32CurStartAddr + g_sFB[i].stride * g_sFB[i].h;
        g_sFB[i].bo.base_virt = (void *)g_sFB[i].bo.base_phys;
#endif
        ui32CurStartAddr = g_sFB[i].bo.base_phys;
#endif
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
    g_sScreen0.bo.base_phys = (uintptr_t)MSPI_XIPMM_BASE_ADDRESS;
    g_sScreen0.bo.base_virt = (void*)g_sScreen0.bo.base_phys;
    g_sScreen1.bo.base_phys = (uintptr_t)(MSPI_XIPMM_BASE_ADDRESS + sizeof(g_ui8Ant_bgr24));
    g_sScreen1.bo.base_virt = (void*)g_sScreen1.bo.base_phys;


    memcpy((void*)g_sScreen0.bo.base_phys, g_ui8Ant_bgr24, sizeof(g_ui8Ant_bgr24));
    memcpy((void*)g_sScreen1.bo.base_phys, g_ui8Beach_bgr24, sizeof(g_ui8Beach_bgr24));

#else
// #### INTERNAL END ####
    //
    // Preload the textures from MRAM to SSRAM
    // Set the textures address in SSRAM for GPU
    //

    g_sScreen0.bo = nema_buffer_create(sizeof(g_ui8Ant_bgr24));
    memcpy((void*)g_sScreen0.bo.base_phys, g_ui8Ant_bgr24, sizeof(g_ui8Ant_bgr24));
    g_sScreen1.bo = nema_buffer_create(sizeof(g_ui8Beach_bgr24));
    memcpy((void*)g_sScreen1.bo.base_phys, g_ui8Beach_bgr24, sizeof(g_ui8Beach_bgr24));

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
    uint8_t ui8CurrentIndex = 0;
    static bool is_first_frame = true;
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

        if(is_first_frame == false)
        {
            //
            // wait transfer done
            //
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
        am_devices_display_set_region(RESX,
                                      RESY / SMALLFB_STRIPES,
                                      PANEL_OFFSET,
                                      PANEL_OFFSET + (RESY / SMALLFB_STRIPES) * stripe);
#endif
        
#ifdef WITH_DISPLAY
        //
        // transfer frame to the display
        //
        am_devices_display_transfer_frame(g_sFB[ui8CurrentIndex].w,
                                          g_sFB[ui8CurrentIndex].h,
                                          g_sFB[ui8CurrentIndex].bo.base_phys,
                                          NULL,
                                          NULL);
        is_first_frame = false;
#endif
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



static int32_t
init(void)
{
    load_objects();

    g_sCLDrawEntireScene  = nema_cl_create();
    g_sCL  = nema_cl_create();
    return 0;
}
static void
loop(void)
{
#define MAX_FRAMES        30
    float step = MIN_STEP;
    float step_step = ANIMATION_STEP_0_1;
    uint32_t i;
    for (i = 0; i < MAX_FRAMES; i++)
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
            // nema_calculate_fps();
    }
}

bool
transition_effects(void)
{
    uint32_t i, j, ui32Status;
    bool bPass = true;
#ifdef DAXI_TEST
    daxi_test_cfg_init();
#endif
#ifdef CACHE_TEST
    cache_test_cfg_init();
    for (j = 0; j < sizeof(g_sCacheCfg) / sizeof(cache_enable_config_t); j++)
    {
        if (g_sCacheCfg[j].bCacheEnable == false)
        {
            ui32Status = am_hal_cachectrl_disable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to disable cache!");
            }
        }
        else
        {
            ui32Status = am_hal_cachectrl_config(&(g_sCacheCfg[j].sCacheCtrl));
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to configure cache!");
            }
            ui32Status = am_hal_cachectrl_enable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to enable cache!");
            }
        }
#endif
#ifdef DAXI_TEST
        for (i = 0; i < sizeof(g_sDaxiCfg) / sizeof(am_hal_daxi_config_t); i++)
        {
            if (g_sDaxiCfg[i].bDaxiPassThrough == true)
            {
                ui32Status = am_hal_daxi_control(AM_HAL_DAXI_CONTROL_DISABLE, NULL);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to disable DAXI!");
                }
            }
            else
            {
                ui32Status = am_hal_daxi_config(&g_sDaxiCfg[i]);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to configure DAXI!");
                }
            }
#endif
//            am_util_stdio_printf("\ntransition_effects with cache config - %d (CPU->CACHECFG is 0x%08X), daxi config - %d (CPU->DAXICFG is 0x%08X)...\n", j, CPU->CACHECFG, i, CPU->DAXICFG);
            float fExecTime;
#ifndef USE_STIMER
            float fStartTime, fStopTime;
            fStartTime = nema_get_time();
#else
            uint32_t ui32StartTime, ui32StopTime;
            ui32StartTime = am_hal_stimer_counter_get();
            am_hal_stimer_config(AM_HAL_STIMER_HFRC_6MHZ | AM_HAL_STIMER_CFG_RUN);
#endif
            g_eEffect = NEMA_TRANS_LINEAR_H;
            init();
            loop();
            nema_buffer_destroy(&(g_sScreen0.bo));
            nema_buffer_destroy(&(g_sScreen1.bo));
            nema_cl_destroy(&g_sCLDrawEntireScene);
            nema_cl_destroy(&g_sCL);
#ifndef USE_STIMER
            fStopTime = nema_get_time();
            fExecTime = fStopTime - fStartTime;
            am_util_stdio_printf("\nCache config: %d, DAXI config:  %d, Execution Time: %.02f\n", j, i, fExecTime);

#else
            ui32StopTime = am_hal_stimer_counter_get();
            fExecTime = ((float)ui32StopTime - (float)ui32StartTime) / 1500000.0f;
            am_util_stdio_printf("\nCache config: %d, DAXI config:  %d, Execution Time: %.10f seconds\n", j, i, fExecTime);
            am_hal_stimer_counter_clear();
#endif
#ifdef APOLLO4_FPGA
            if (fExecTime > 0.7f)
            {
                am_util_stdio_printf("------ Timeout! ------\n");
                bPass = false;;
            }
#endif
            TEST_ASSERT_TRUE(bPass);
#ifdef DAXI_TEST
        }
#endif
#ifdef CACHE_TEST
    }
#endif

    return bPass;
}
//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int32_t
daxi_transiton_effects_test(void)
{
    uint32_t ui32Status;
    bool bTestPass = true;
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

// #### INTERNAL BEGIN ####
#if (defined(LOAD_FROM_PSRAM) || defined(FB_IN_PSRAM)) && !defined(XIP_EXECUTION)
    //
    // Init PSRAM device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_HexCE1MSPIConfig, &g_pDevHandle, &g_pHandle);

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
#endif // (defined(LOAD_FROM_PSRAM) || defined(FB_IN_PSRAM)) && !defined(XIP_EXECUTION)
// #### INTERNAL END ####
#ifndef USE_STIMER
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
#endif

#ifdef BAREMETAL
    bTestPass = transition_effects();
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
    return bTestPass;
}

