//*****************************************************************************
//
//! @file nemadc_jdi_test_case.c
//!
//! @brief NemaDC jdi interfaces test cases.
//! This exmaple demonstrates how to update full frame with 2 active layers(layer 0 
//! and layer 1),how to update 8 partial regions with 4 active layers.
//
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_bsp.h"
#include "nema_hal.h"
#include "nema_dc.h"
#include "nema_dc_mipi.h"
#include "am_util_delay.h"
#include "am_util_stdio.h"
#include "nema_dc_regs.h"
#include "tsi_malloc.h"
#include "string.h"
#include "am_devices_dc_jdi_sharp.h"

#include "test_card_rgba2222.h"
#include "logo_rgba2222.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//
// the below two values are dependent on your screen size.
//
#define FB_RESX                 280
#define FB_RESY                 280

#define FB_LOGO_RESX            280
#define FB_LOGO_RESY            88

//
// Define the start point of the Y-axis of the middle logo
//
#define FB_LOGO_ANCHOR          100

#define LAYER_ACTIVE            1
#define LAYER_INACTIVE          0

//
// For Apollo5b silicon ,it could support up to 16 partial regions
//

#define PARTIAL_REGIONS_MAX     16

//
// For this example,it demonstrates Fast Forward feature with 8 partial regions. 
//
#define PARTIAL_REGIONS         8
#define PARTIAL_REGIONS_OFFSET  (FB_LOGO_RESY / PARTIAL_REGIONS) 

//*****************************************************************************
//
//! @brief Test Nemadc jdi interface.
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
nemadc_jdi_test(void)
{
    bool bTestPass = false;
    am_devices_dc_jdi_timer_config_t sTimerConfig = {
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
    sLayer0.resy      = FB_RESY;
    sLayer0.buscfg    = 0;
    sLayer0.format    = NEMADC_RGBA2222;
    sLayer0.blendmode = NEMADC_BL_SRC;
    sLayer0.stride    = sLayer0.resx * 1;
    sLayer0.startx    = 0;
    sLayer0.starty    = 0;
    sLayer0.alpha     = 0xff;
    sLayer0.flipx_en  = 0;
    sLayer0.flipy_en  = 0;
    sLayer0.extra_bits= 0;
    sLayer0.baseaddr_virt = tsi_malloc(sLayer0.resy*sLayer0.stride);
    sLayer0.baseaddr_phys = (unsigned)(sLayer0.baseaddr_virt);
    nema_memcpy(sLayer0.baseaddr_virt,test_card_rgba2222,sLayer0.resy * sLayer0.stride);

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
    sLayer1.extra_bits= 0;
    sLayer1.baseaddr_virt = tsi_malloc(sLayer1.resy*sLayer1.stride);
    sLayer1.baseaddr_phys = (unsigned)(sLayer1.baseaddr_virt);
    nema_memcpy(sLayer1.baseaddr_virt,logo_rgba2222,sLayer1.resy * sLayer1.stride);

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
    sLayer2.extra_bits= 0;
    sLayer2.baseaddr_virt = tsi_malloc(sLayer2.resy*sLayer2.stride);
    sLayer2.baseaddr_phys = (unsigned)(sLayer2.baseaddr_virt);
    nema_memcpy(sLayer2.baseaddr_virt,logo_rgba2222,sLayer2.resy * sLayer2.stride);

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
    sLayer3.extra_bits= 0;
    sLayer3.baseaddr_virt = tsi_malloc(sLayer3.resy*sLayer3.stride);
    sLayer3.baseaddr_phys = (unsigned)(sLayer3.baseaddr_virt);
    nema_memcpy(sLayer3.baseaddr_virt,logo_rgba2222,sLayer3.resy * sLayer3.stride);

    am_devices_dc_jdi_sharp_timer_start();

    nemadc_set_bgcolor(0x0);
    //
    // Full frame update(enable layer 0 and 2)
    //
    nemadc_mip_setup( LAYER_ACTIVE, &sLayer0,
                    LAYER_INACTIVE, &sLayer1,
                    LAYER_ACTIVE, &sLayer2,
                    LAYER_INACTIVE, &sLayer3,
                    0,
                    0,280
                    );
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();

    //
    // Fast Forward(enable 4 layers)
    //
    for(uint8_t i = 0;i < PARTIAL_REGIONS_MAX/PARTIAL_REGIONS;i++)
    {
        //
        // Row Start of a partial region must be bigger than the Row End of the previous region.
        //
        nemadc_mip_setup( LAYER_ACTIVE, &sLayer0,
                        LAYER_ACTIVE, &sLayer1,
                        LAYER_ACTIVE, &sLayer2,
                        LAYER_ACTIVE, &sLayer3,
                        PARTIAL_REGIONS,
                        sLayer1.starty + i * PARTIAL_REGIONS_OFFSET, sLayer1.starty + (i+1) * PARTIAL_REGIONS_OFFSET,
                        sLayer1.starty + (i+2) * PARTIAL_REGIONS_OFFSET, sLayer1.starty + (i+3) * PARTIAL_REGIONS_OFFSET,
                        sLayer1.starty + (i+4) * PARTIAL_REGIONS_OFFSET, sLayer1.starty + (i+5) * PARTIAL_REGIONS_OFFSET,
                        sLayer1.starty + (i+6) * PARTIAL_REGIONS_OFFSET, sLayer1.starty + (i+7) * PARTIAL_REGIONS_OFFSET,
                        sLayer3.starty + i * PARTIAL_REGIONS_OFFSET, sLayer3.starty + (i+1) * PARTIAL_REGIONS_OFFSET,
                        sLayer3.starty + (i+2) * PARTIAL_REGIONS_OFFSET, sLayer3.starty + (i+3) * PARTIAL_REGIONS_OFFSET,
                        sLayer3.starty + (i+4) * PARTIAL_REGIONS_OFFSET, sLayer3.starty + (i+5) * PARTIAL_REGIONS_OFFSET,
                        sLayer3.starty + (i+6) * PARTIAL_REGIONS_OFFSET, sLayer3.starty + (i+7) * PARTIAL_REGIONS_OFFSET
                        );
        nemadc_transfer_frame_launch();
        nemadc_wait_vsync();
        if(sLayer1.format == NEMADC_RGBA2222)
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
    //
    // disable panel
    //
    //am_devices_dc_jdi_sharp_timer_stop();

    tsi_free(sLayer0.baseaddr_virt);
    tsi_free(sLayer1.baseaddr_virt);
    tsi_free(sLayer2.baseaddr_virt);
    tsi_free(sLayer3.baseaddr_virt);

    am_util_stdio_printf("\nThis example is not an automatic test, it need to manually check!!\n\n");
    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;

} // nemadc_jdi_test()

