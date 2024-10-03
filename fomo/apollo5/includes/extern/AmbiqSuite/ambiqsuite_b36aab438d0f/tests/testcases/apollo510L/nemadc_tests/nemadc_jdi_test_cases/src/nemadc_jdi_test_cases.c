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
// For this example,it demonstrates Fast Forward feature with 4 partial regions. 
//
#define PARTIAL_REGIONS         4
#define PARTIAL_REGIONS_OFFSET  (FB_LOGO_RESY / 8) 

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

    nemadc_layer_t sLayer0_card = {0};
    sLayer0_card.resx      = FB_RESX;
    sLayer0_card.resy      = FB_RESY;
    sLayer0_card.buscfg    = 0;
    sLayer0_card.format    = NEMADC_RGBA2222;
    sLayer0_card.blendmode = NEMADC_BL_SRC;
    sLayer0_card.stride    = sLayer0_card.resx * 1;
    sLayer0_card.startx    = 0;
    sLayer0_card.starty    = 0;
    sLayer0_card.alpha     = 0xff;
    sLayer0_card.flipx_en  = 0;
    sLayer0_card.flipy_en  = 0;
    sLayer0_card.extra_bits= 0;
    sLayer0_card.baseaddr_virt = tsi_malloc(sLayer0_card.resy*sLayer0_card.stride);
    sLayer0_card.baseaddr_phys = (unsigned)(sLayer0_card.baseaddr_virt);
    nema_memcpy(sLayer0_card.baseaddr_virt,test_card_rgba2222,sLayer0_card.resy * sLayer0_card.stride);

    nemadc_layer_t sLayer0_logo = {0};
    sLayer0_logo.resx      = FB_LOGO_RESX;
    sLayer0_logo.resy      = FB_LOGO_RESY;
    sLayer0_logo.buscfg    = 0;
    sLayer0_logo.format    = NEMADC_RGBA2222;
    sLayer0_logo.blendmode = NEMADC_BL_SRC;
    sLayer0_logo.stride    = sLayer0_logo.resx * 1;
    sLayer0_logo.startx    = 0;
    sLayer0_logo.starty    = FB_LOGO_ANCHOR - FB_LOGO_RESY;
    sLayer0_logo.alpha     = 0xff;
    sLayer0_logo.flipx_en  = 0;
    sLayer0_logo.flipy_en  = 0;
    sLayer0_logo.extra_bits= 0;
    sLayer0_logo.baseaddr_virt = tsi_malloc(sLayer0_logo.resy*sLayer0_logo.stride);
    sLayer0_logo.baseaddr_phys = (unsigned)(sLayer0_logo.baseaddr_virt);
    nema_memcpy(sLayer0_logo.baseaddr_virt,logo_rgba2222,sLayer0_logo.resy * sLayer0_logo.stride);

    nemadc_layer_t sLayer1 = {0};

    nemadc_layer_t sLayer2 = {0};

    nemadc_layer_t sLayer3 = {0};

    am_devices_dc_jdi_sharp_timer_start();

    nemadc_set_bgcolor(0x0);
    //
    // Full frame update(enable layer 0)
    //
    nemadc_mip_setup( LAYER_ACTIVE, &sLayer0_card,
                    LAYER_INACTIVE, &sLayer1,
                    LAYER_INACTIVE, &sLayer2,
                    LAYER_INACTIVE, &sLayer3,
                    0,
                    0,280
                    );
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();

    
    // Fast Forward(enable layer 0)
    
    for(uint8_t i = 0;i < PARTIAL_REGIONS_MAX/PARTIAL_REGIONS/2;i++)
    {
        //
        // Row Start of a partial region must be bigger than the Row End of the previous region.
        //
        nemadc_mip_setup( LAYER_ACTIVE, &sLayer0_logo,
                        LAYER_INACTIVE, &sLayer1,
                        LAYER_INACTIVE, &sLayer2,
                        LAYER_INACTIVE, &sLayer3,
                        4,
                        sLayer0_logo.starty + i * PARTIAL_REGIONS_OFFSET, sLayer0_logo.starty + (i+1) * PARTIAL_REGIONS_OFFSET,
                        sLayer0_logo.starty + (i+2) * PARTIAL_REGIONS_OFFSET, sLayer0_logo.starty + (i+3) * PARTIAL_REGIONS_OFFSET,
                        sLayer0_logo.starty + (i+4) * PARTIAL_REGIONS_OFFSET, sLayer0_logo.starty + (i+5) * PARTIAL_REGIONS_OFFSET,
                        sLayer0_logo.starty + (i+6) * PARTIAL_REGIONS_OFFSET, sLayer0_logo.starty + (i+7) * PARTIAL_REGIONS_OFFSET
                        // sLayer1.starty + (i+8) * PARTIAL_REGIONS_OFFSET, sLayer1.starty + (i+9) * PARTIAL_REGIONS_OFFSET,
                        // sLayer1.starty + (i+10) * PARTIAL_REGIONS_OFFSET, sLayer1.starty + (i+11) * PARTIAL_REGIONS_OFFSET,
                        // sLayer1.starty + (i+12) * PARTIAL_REGIONS_OFFSET, sLayer1.starty + (i+13) * PARTIAL_REGIONS_OFFSET,
                        // sLayer1.starty + (i+14) * PARTIAL_REGIONS_OFFSET, sLayer1.starty + (i+15) * PARTIAL_REGIONS_OFFSET
                        );
        nemadc_transfer_frame_launch();
        nemadc_wait_vsync();
    }
    // disable panel
    // am_devices_dc_jdi_sharp_timer_stop();

    tsi_free(sLayer0_card.baseaddr_virt);
    tsi_free(sLayer0_logo.baseaddr_virt);

    am_util_stdio_printf("\nThis example is not an automatic test, it need to manually check!!\n\n");
    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;

} // nemadc_jdi_test()

