//*****************************************************************************
//
//! @file nemagfx_vg_test.c
//!
//! @brief This example include all the build-in VG demos in the NemaSDK.
//! Modified the macro defines in nemagfx_vg_test.h(line 64-71) to select
//! among the following demos.
//! <masking_example>: this demo shows how to use the masking feature with NemaVG.
//! <paint_example>: this demo shows how to use different paint features.
//! <paint_lut_example>: this demo shows how to use the LUT format texture
//!                      in the VG paint object.
//! <render_vg_font>: this demo shows how to render the TTF font with NemaVG.
//! <shape>: this demo shows various pre-defined shapes with different paint settings.
//! <text_transformation>: this demo shows how to move the text object by setting
//!                        different transform matrixes.
//! <tsvg_benchmark>: run the SVG benchmark by rotating a tiger head image in SVG format.
//! <tsvg_render_example>: render an SVG image that includes both shapes and fonts.
//! Note: Only one of the above macro defines can be enable.
//
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "nemagfx_vg_test.h"
#include "nemagfx_buffer_customize.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
//static uint32_t        ui32DMATCBBuffer[2560];
void            *g_pPsramHandle;
void            *g_pMSPIPsramHandle;

am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = 0,
    .pNBTxnBuf                = NULL,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

//! MSPI interrupts.
static const IRQn_Type MspiInterrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
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

   am_hal_mspi_interrupt_status_get(g_pMSPIPsramHandle, &ui32Status, false);

   am_hal_mspi_interrupt_clear(g_pMSPIPsramHandle, ui32Status);

   am_hal_mspi_interrupt_service(g_pMSPIPsramHandle, ui32Status);

}

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
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

#ifdef DEBUG_PIN
    //debug pin setting
    am_hal_gpio_pinconfig(DEBUG_PIN, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_clear(DEBUG_PIN);
#endif

#ifdef NEMA_WAIT_CL_PIN_1
    am_hal_gpio_pinconfig(NEMA_WAIT_CL_PIN_1, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_clear(NEMA_WAIT_CL_PIN_1);
#endif

#ifdef NEMA_WAIT_CL_PIN_2
    am_hal_gpio_pinconfig(NEMA_WAIT_CL_PIN_2, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_clear(NEMA_WAIT_CL_PIN_2);
#endif

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

    //
    // Enable global IRQ.
    //
    am_hal_interrupt_master_enable();

#ifdef CPU_RUN_IN_HP_MODE
    //
    //CPU switch to HP mode.
    //
    if ( am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("HP_LP:Enter HP mode failed!\n");
    }
#endif

    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_SetPriority(MspiInterrupts[MSPI_PSRAM_MODULE], 5);
    NVIC_EnableIRQ(MspiInterrupts[MSPI_PSRAM_MODULE]);

    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }

    //
    // Initialize plotting interface.
    //
    am_util_stdio_printf("nemagfx_vg_test Example\n");

#ifdef GPU_RUN_IN_HP_MODE
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);

    //
    //GPU switch to HP mode.
    //
    am_hal_pwrctrl_gpu_mode_e current_mode;
    am_hal_pwrctrl_gpu_mode_select(AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE);
    am_hal_pwrctrl_gpu_mode_status(&current_mode);
    if ( AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE != current_mode )
    {
        am_util_stdio_printf("gpu switch to HP mode failed!\n");
    }
#endif

    //
    // Power on GPU
    //
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

    //
    // Customize graphics .
    //
#ifdef NEMA_USE_CUSTOM_MALLOC
    graphic_heap_init();
#endif

    //
    // Run different VG test item, please change the macro define in the nemagfx_vg_test.h
    //
#if defined(RUN_MASKING_EXAMPLE)
    extern int masking_example(void);
    masking_example();
#elif defined(RUN_PAINT_EXAMPLE)
    extern int paint_example(void);
    paint_example();
#elif defined(RUN_PAINT_LUT_EXAMPLE)
    extern int paint_lut_example(void);
     paint_lut_example();
#elif defined(RUN_RENDER_VG_FONT)
    extern int render_vg_font(void);
    render_vg_font();
#elif defined(RUN_SHAPE)
    extern int shapes(void);
    shapes();
#elif defined(RUN_TEXT_TRANSFORMATION)
    extern int text_transformation(void);
    text_transformation();
#elif defined(RUN_TSVG_BENCHMARK)
    extern int tsvg_benchmark(void);
    tsvg_benchmark();
#elif defined(RUN_TSVG_RENDER_EXAMPLE)
    extern int tsvg_render_example(void);
    tsvg_render_example();
#elif defined(RUN_TSVG_MEASURE)
    extern int tsvg_measure(void);
    tsvg_measure();
#elif defined(RUN_RASTER_ARC)
    extern int raster_arc(void);
    raster_arc();
#elif defined(RUN_RENDER_RAW_TTF)
    extern int render_raw_ttf(void);
    render_raw_ttf();
#elif defined(RUN_JOINS_CAPS)
    extern int render_joins_caps(void);
    render_joins_caps();
#else
    #error "Not supported selection!"
#endif

}

