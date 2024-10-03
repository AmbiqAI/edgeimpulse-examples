//*****************************************************************************
//
//! @file nemadc_jdi_configurations.c
//!
//! @brief NemaDC JDI interface configurations
//!
//! A pixel has 2-bit static random-access memory (SRAM) to store the image data
//! for each sub-pixel and it is capable of displaying 64 colors.this exmaple
//! demonstrates how to update full frame with 2 active layers(layer 0 and layer 1),
//! how to update 8 partial regions with 4 active layers.
//! It is also a simple example of dynamically controlling the power of DC.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "nema_dc_jdi.h"
#include "nemadc_jdi_configurations.h"
#include "am_devices_dc_jdi_sharp.h"
#include "test_card_rgba2222.h"
#include "logo_rgba2222.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
//
// The below two values are dependent on your screen size. In addition, it has the restriction that the layer size is a multiple of 4 for Apollo5.
//
#define FB_RESX                 280
#define FB_RESY                 280

//
// It has the restriction that the layer size is a multiple of 4 for Apollo5.
//
#define FB_LOGO_RESX            280
#define FB_LOGO_RESY            88

//
// Define the start point of the Y-axis of the middle logo
//
#define FB_LOGO_ANCHOR          100

#define LAYER_ACTIVE            1
#define LAYER_INACTIVE          0

//
// Apollo5b supports up to 16 partial regions
//

#define PARTIAL_REGIONS_MAX     16

//
// For this example, it demonstrates Fast Forward feature with 8 partial regions.
//
#define PARTIAL_REGIONS         8
#define PARTIAL_REGIONS_OFFSET  (FB_LOGO_RESY / PARTIAL_REGIONS)

//*****************************************************************************
//
//! @brief Disable the power & clock for display controller to save power
//!
//! @return true.
//
//*****************************************************************************
static bool
dc_power_down(void)
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_DISP, &status);
    if ( status )
    {
        nemadc_backup_registers();

        am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_DISP);

        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_DISABLE, NULL);
    }
    return true;
}

//*****************************************************************************
//
//! @brief Enable display controller
//!
//! @note Enable clock & power for display controller, then restore the configuration
//!
//! @return true if success, otherwise return false.
//
//*****************************************************************************
static bool
dc_power_up(void)
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_DISP, &status);
    if ( !status )
    {
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);

        am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);
        if (0 != nemadc_init())
        {
            //
            // Initialize DC failed.
            //
            return false;
        }
        return nemadc_restore_registers();
    }
    return true;
}
//*****************************************************************************
//
//! @brief Test JDI interface configurations
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_jdi_configurations(void)
{
    am_devices_dc_jdi_timer_config_t sTimerConfig =
    {
        .ui32TimerNum = 7,
        .eTimerClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16,
        .ui32Frequency = 60
    };
    nemadc_initial_config_t sDCConfig;
    //
    // Assign a fixed value to display type.
    //
    g_sDispCfg.eInterface = DISP_IF_JDI;

    am_bsp_disp_pins_enable();
    am_devices_dc_jdi_sharp_init(&sTimerConfig, &sDCConfig);

    nemadc_configure(&sDCConfig);

    nemadc_layer_t sLayer0 = {0};
    sLayer0.resx      = FB_RESX;
    //
    // The implementation of Apollo5A and Apollo5B are different.
    //
#ifdef AM_PART_APOLLO5A
    sLayer0.resy      = FB_RESY * 2;
#else
    sLayer0.resy      = FB_RESY;
#endif
    sLayer0.buscfg    = 0;
    sLayer0.format    = NEMADC_RGBA2222;
    sLayer0.blendmode = NEMADC_BL_SRC;
    sLayer0.stride    = sLayer0.resx * 1;
    sLayer0.startx    = 0;
    sLayer0.starty    = 0;
    sLayer0.alpha     = 0xff;
    sLayer0.flipx_en  = 0;
    sLayer0.flipy_en  = 0;
    sLayer0.extra_bits = 0;
    sLayer0.baseaddr_virt = tsi_malloc(FB_RESY * sLayer0.stride);
    sLayer0.baseaddr_phys = (unsigned)(sLayer0.baseaddr_virt);
    nema_memcpy(sLayer0.baseaddr_virt, test_card_rgba2222, FB_RESY * sLayer0.stride);

    am_devices_dc_jdi_sharp_timer_start();

    //
    // For Apollo5A, one layer with full frame update is supported; For Apollo5B, multiple layers with up to 16 partial updates are supported. 
    //
#ifdef AM_PART_APOLLO5A
    nemadc_set_layer(0, &sLayer0);
// #### INTERNAL BEGIN ####
    //
    // SW workaround for FlipY on Apollo5 RevA.
    //
// #### INTERNAL END ####
    if (sLayer0.flipy_en)
    {
        nemadc_set_layer_addr(0, nemadc_reg_read(NEMADC_REG_LAYER_BASEADDR(0)) - (sLayer0.resx * nemadc_stride_size(sLayer0.format, sLayer0.resx)));
    }
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();

    am_util_delay_ms(1000);

    //
    // Flip X & Y 
    //
    sLayer0.flipx_en  = 1;
    sLayer0.flipy_en  = 1;
    nemadc_set_layer(0, &sLayer0);
// #### INTERNAL BEGIN ####
    //
    // SW workaround for FlipY on Apollo5 RevA.
    //
// #### INTERNAL END ####
    if (sLayer0.flipy_en)
    {
        nemadc_set_layer_addr(0, nemadc_reg_read(NEMADC_REG_LAYER_BASEADDR(0)) - (sLayer0.resx * nemadc_stride_size(sLayer0.format, sLayer0.resx)));
    }
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();
#else

    nemadc_layer_t sLayer1 = {0};
    sLayer1.resx      = FB_LOGO_RESX;
    sLayer1.resy      = FB_LOGO_RESY;
    sLayer1.buscfg    = 0;
    sLayer1.format    = NEMADC_RGBA2222;
    sLayer1.blendmode = NEMADC_BL_SRC;
    sLayer1.stride    = sLayer1.resx * 1;
    sLayer1.startx    = 0;
    sLayer1.starty    = FB_LOGO_ANCHOR - FB_LOGO_RESY;
    sLayer1.alpha     = 0xff;
    sLayer1.flipx_en  = 0;
    sLayer1.flipy_en  = 0;
    sLayer1.extra_bits = 0;
    sLayer1.baseaddr_virt = tsi_malloc(sLayer1.resy * sLayer1.stride);
    sLayer1.baseaddr_phys = (unsigned)(sLayer1.baseaddr_virt);
    nema_memcpy(sLayer1.baseaddr_virt, logo_rgba2222, sLayer1.resy * sLayer1.stride);

    nemadc_layer_t sLayer2 = {0};
    sLayer2.resx      = FB_LOGO_RESX;
    sLayer2.resy      = FB_LOGO_RESY;
    sLayer2.buscfg    = 0;
    sLayer2.format    = NEMADC_RGBA2222;
    sLayer2.blendmode = NEMADC_BL_SRC;
    sLayer2.stride    = sLayer2.resx * 1;
    sLayer2.startx    = 0;
    sLayer2.starty    = FB_LOGO_ANCHOR;
    sLayer2.alpha     = 0xff;
    sLayer2.flipx_en  = 0;
    sLayer2.flipy_en  = 0;
    sLayer2.extra_bits = 0;
    sLayer2.baseaddr_virt = tsi_malloc(sLayer2.resy * sLayer2.stride);
    sLayer2.baseaddr_phys = (unsigned)(sLayer2.baseaddr_virt);
    nema_memcpy(sLayer2.baseaddr_virt, logo_rgba2222, sLayer2.resy * sLayer2.stride);

    nemadc_layer_t sLayer3 = {0};
    sLayer3.resx      = FB_LOGO_RESX;
    sLayer3.resy      = FB_LOGO_RESY;
    sLayer3.buscfg    = 0;
    sLayer3.format    = NEMADC_RGBA2222;
    sLayer3.blendmode = NEMADC_BL_SRC;
    sLayer3.stride    = sLayer3.resx * 1;
    sLayer3.startx    = 0;
    sLayer3.starty    = FB_LOGO_ANCHOR + FB_LOGO_RESY;
    sLayer3.alpha     = 0xff;
    sLayer3.flipx_en  = 0;
    sLayer3.flipy_en  = 0;
    sLayer3.extra_bits = 0;
    sLayer3.baseaddr_virt = tsi_malloc(sLayer3.resy * sLayer3.stride);
    sLayer3.baseaddr_phys = (unsigned)(sLayer3.baseaddr_virt);
    nema_memcpy(sLayer3.baseaddr_virt, logo_rgba2222, sLayer3.resy * sLayer3.stride);

    nemadc_set_bgcolor(0x0);
    //
    // Full frame update the active layers
    //
    nemadc_mip_setup( LAYER_ACTIVE, &sLayer0,
                    LAYER_INACTIVE, &sLayer1,
                    LAYER_ACTIVE, &sLayer2,
                    LAYER_INACTIVE, &sLayer3,
                    0
                    );
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();

    dc_power_down();
    am_util_delay_ms(200);
    dc_power_up();

    //
    // Single partial update of the active layers with FlipX and FlipY enabled.
    //
    sLayer0.flipx_en  = 1;
    sLayer0.flipy_en  = 1;
    sLayer2.flipx_en  = 1;
    sLayer2.flipy_en  = 1;
    nemadc_mip_setup( LAYER_ACTIVE, &sLayer0,
                    LAYER_INACTIVE, &sLayer1,
                    LAYER_ACTIVE, &sLayer2,
                    LAYER_INACTIVE, &sLayer3,
                    1,
                    70, 210
                    );

    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();

    dc_power_down();
    am_util_delay_ms(200);
    dc_power_up();

    //
    // Please don't enable FlipY when transmitting multi-partial updates.
    //
    sLayer0.flipx_en  = 0;
    sLayer0.flipy_en  = 0;
    sLayer1.flipx_en  = 0;
    sLayer1.flipy_en  = 0;
    sLayer2.flipx_en  = 0;
    sLayer2.flipy_en  = 0;
    sLayer3.flipx_en  = 0;
    sLayer3.flipy_en  = 0;
    if ((sLayer0.flipy_en == 0) && (sLayer1.flipy_en == 0) && (sLayer2.flipy_en == 0) && (sLayer3.flipy_en == 0))
    {
        for ( uint8_t i = 0; i < PARTIAL_REGIONS_MAX / PARTIAL_REGIONS; i++ )
        {
            //
            // Row Start of a partial region must be bigger than the Row End of the previous region.
            //
            nemadc_mip_setup( LAYER_ACTIVE, &sLayer0,
                            LAYER_ACTIVE, &sLayer1,
                            LAYER_ACTIVE, &sLayer2,
                            LAYER_ACTIVE, &sLayer3,
                            PARTIAL_REGIONS,
                            sLayer1.starty + i * PARTIAL_REGIONS_OFFSET, sLayer1.starty + (i + 1) * PARTIAL_REGIONS_OFFSET,
                            sLayer1.starty + (i + 2) * PARTIAL_REGIONS_OFFSET, sLayer1.starty + (i + 3) * PARTIAL_REGIONS_OFFSET,
                            sLayer1.starty + (i + 4) * PARTIAL_REGIONS_OFFSET, sLayer1.starty + (i + 5) * PARTIAL_REGIONS_OFFSET,
                            sLayer1.starty + (i + 6) * PARTIAL_REGIONS_OFFSET, sLayer1.starty + (i + 7) * PARTIAL_REGIONS_OFFSET,
                            sLayer3.starty + i * PARTIAL_REGIONS_OFFSET, sLayer3.starty + (i + 1) * PARTIAL_REGIONS_OFFSET,
                            sLayer3.starty + (i + 2) * PARTIAL_REGIONS_OFFSET, sLayer3.starty + (i + 3) * PARTIAL_REGIONS_OFFSET,
                            sLayer3.starty + (i + 4) * PARTIAL_REGIONS_OFFSET, sLayer3.starty + (i + 5) * PARTIAL_REGIONS_OFFSET,
                            sLayer3.starty + (i + 6) * PARTIAL_REGIONS_OFFSET, sLayer3.starty + (i + 7) * PARTIAL_REGIONS_OFFSET
                            );
            nemadc_transfer_frame_launch();
            nemadc_wait_vsync();

            dc_power_down();
            dc_power_up();
            
            if ( sLayer1.format == NEMADC_RGBA2222 )
            {
                sLayer1.format = NEMADC_BGRA2222;
                sLayer3.format = NEMADC_BGRA2222;
            }
            else
            {
                sLayer1.format = NEMADC_RGBA2222;
                sLayer3.format = NEMADC_RGBA2222;
            }
        }
    }

    tsi_free(sLayer1.baseaddr_virt);
    tsi_free(sLayer2.baseaddr_virt);
    tsi_free(sLayer3.baseaddr_virt);
#endif

    //
    // disable panel
    //
    //am_devices_dc_jdi_sharp_timer_stop();

    tsi_free(sLayer0.baseaddr_virt);
}

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

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Print a banner.
    //
    am_util_stdio_printf("nemadc_jdi_configurations example.\n");
    //
    // The user can select any clock source if it satisfies the panel's request after dividing by the clock divider. For Apollo5A, it clock divider is smaller than Apollo5B.
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC48, NULL);
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);

    nema_sys_init();

    //
    // Initialize NemaDC
    //
    if ( nemadc_init() != 0 )
    {
        am_util_stdio_printf("NemaDC init failed!\n");
    }

#ifdef BURST_MODE
    //
    // Initialize for High Performance Mode
    //
    if (am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) == AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("\nOperating in High Performance Mode\n");
    }
    else
    {
        am_util_stdio_printf("\nFailed to Initialize for High Performance Mode operation\n");
    }
#else
    am_util_stdio_printf("\nOperating in Normal Mode\n");
#endif

    nemadc_jdi_configurations();

    while (1)
    {
    }
}
