//*****************************************************************************
//
//! @file display_task.c
//!
//! @brief Task to handle DISPLAY operations.
//!
//! AM_DEBUG_PRINTF
//! If enabled, debug messages will be sent over ITM.
//!
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
// Global includes for this project.
//
//*****************************************************************************
#include "nemagfx_enhanced_stress_test.h"

#include "nema_core.h"
#include "nema_utils.h"
#include "nema_event.h"
#include "nema_dc.h"
#include "nema_dc_mipi.h"

#include "graphics_power_control.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define DC_LAYER_TO_USE (0)

#define DC_PIXEL_FORMAT FMT_RGB888

//Offset of the panel
#define PANEL_OFFSET_X (((g_sDispCfg.ui16ResX/4*4)-FB_RESX)/2)
#define PANEL_OFFSET_Y (((g_sDispCfg.ui16ResY/4*4)-FB_RESY)/2)

//*****************************************************************************
//
// DISPLAY task handle.
//
//*****************************************************************************
TaskHandle_t DisplayTaskHandle;
uint32_t ui32MipiCfg = MIPICFG_16RGB888_OPT0; // default config

//layer
nemadc_layer_t g_sLayer =
{
    .startx        = 0,
    .sizex         = FB_RESX,
    .resx          = FB_RESX,
    .starty        = 0,
    .sizey         = FB_RESY / SMALLFB_STRIPES,
    .resy          = FB_RESY / SMALLFB_STRIPES,
    .stride        = -1,
    .format        = NEMADC_RGBA8888,
    .blendmode     = NEMADC_BL_SRC,
    .buscfg        = 0,
    .alpha         = 0xff,
    .flipx_en      = 0,
    .flipy_en      = 0,
    .extra_bits    = 0
};
//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************


//*****************************************************************************
//
// Task function.
//
//*****************************************************************************
//*****************************************************************************
//
// Set pannel display region
//
//*****************************************************************************
#ifdef SMALLFB
void
display_region_set(int8_t parts)
{
    switch (g_sDispCfg.eInterface)
    {
        case DISP_IF_SPI4:
            am_devices_dc_xspi_raydium_set_region( FB_RESX, FB_RESY / SMALLFB_STRIPES, PANEL_OFFSET_X, PANEL_OFFSET_Y + (FB_RESY / SMALLFB_STRIPES) * parts);
            break;
        case DISP_IF_DSPI:
            am_devices_dc_xspi_raydium_set_region( FB_RESX, FB_RESY / SMALLFB_STRIPES, PANEL_OFFSET_X, PANEL_OFFSET_Y + (FB_RESY / SMALLFB_STRIPES) * parts);
            break;
        case DISP_IF_QSPI:
            am_devices_dc_xspi_raydium_set_region( FB_RESX, FB_RESY / SMALLFB_STRIPES, PANEL_OFFSET_X, PANEL_OFFSET_Y + (FB_RESY / SMALLFB_STRIPES) * parts);
            break;
        case DISP_IF_DSI:
            am_devices_dc_dsi_raydium_set_region(FB_RESX, FB_RESY / SMALLFB_STRIPES, PANEL_OFFSET_X, PANEL_OFFSET_Y + (FB_RESY / SMALLFB_STRIPES) * parts);
            break;
        default:
            break; //NOP
    }
}
#endif

void display_refresh_start(void)
{
    nemadc_transfer_frame_prepare(false);
    //
    //It's necessary to launch frame manually when TE is disabled.
    //
    nemadc_transfer_frame_launch();
}

void display_refresh_end(void)
{

}

//*****************************************************************************
//
// Set up GP, DC and panel.
//
//*****************************************************************************
int
display_setup(void)
{
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_xspi_raydium_config_t sDisplayPanelConfig;

    //
    // Set the display region to center
    //
    if (FB_RESX < g_sDispCfg.ui16ResX)
    {
        sDisplayPanelConfig.ui16ResX = FB_RESX;
    }
    else
    {
        sDisplayPanelConfig.ui16ResX = g_sDispCfg.ui16ResX;
    }
    ui16MinX = (g_sDispCfg.ui16ResX - sDisplayPanelConfig.ui16ResX) >> 1;
    ui16MinX = (ui16MinX >> 1) << 1;

    if (FB_RESY < g_sDispCfg.ui16ResY)
    {
        sDisplayPanelConfig.ui16ResY = FB_RESY;
    }
    else
    {
        sDisplayPanelConfig.ui16ResY = g_sDispCfg.ui16ResY;
    }
    ui16MinY = (g_sDispCfg.ui16ResY - sDisplayPanelConfig.ui16ResY) >> 1;
    ui16MinY = (ui16MinY >> 1) << 1;

    g_sDispCfg.eTEType = DISP_TE_DISABLE;
    sDCConfig.ui16ResX = sDisplayPanelConfig.ui16ResX;
    sDCConfig.ui16ResY = sDisplayPanelConfig.ui16ResY;
    sDCConfig.bTEEnable = (g_sDispCfg.eTEType == DISP_TE_DC);
    sDisplayPanelConfig.ui16MinX = ui16MinX + g_sDispCfg.ui16Offset;
    sDisplayPanelConfig.ui16MinY = ui16MinY;
    sDisplayPanelConfig.bTEEnable = (g_sDispCfg.eTEType != DISP_TE_DISABLE);
    sDisplayPanelConfig.bFlip = g_sDispCfg.bFlip;

    am_bsp_disp_pins_enable();
    //
    // If you want to change display configurations in example, please change them here as below.
    //
    // g_sDispCfg.ui8NumLanes = 1;
    // g_sDispCfg.eDbiWidth = AM_HAL_DSI_DBI_WIDTH_8;
    // g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X12;

    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        //
        // VDD18 control callback function
        //
        am_hal_dsi_register_external_vdd18_callback(am_bsp_external_vdd18_switch);
        //
        // Enable DSI power and configure DSI clock.
        //
        am_hal_dsi_init();
    }
    else
    {
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC96, NULL);
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);
    }
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);

    nema_sys_init();
    //
    //Initialize NemaDC
    //
    if (nemadc_init() != 0)
    {
        return -2;
    }
    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        uint8_t ui8LanesNum = g_sDispCfg.ui8NumLanes;
        uint8_t ui8DbiWidth = g_sDispCfg.eDbiWidth;
        uint32_t ui32FreqTrim = g_sDispCfg.eDsiFreq;
        pixel_format_t eFormat = FMT_RGB888;
        if (am_hal_dsi_para_config(ui8LanesNum, ui8DbiWidth, ui32FreqTrim, false) != 0)
        {
            return -3;
        }
        switch (eFormat)
        {
            case FMT_RGB888:
                if (ui8DbiWidth == 16)
                {
                    ui32MipiCfg = MIPICFG_16RGB888_OPT0;
                }
                if (ui8DbiWidth == 8)
                {
                    ui32MipiCfg = MIPICFG_8RGB888_OPT0;
                }
                break;

            case FMT_RGB565:
                if (ui8DbiWidth == 16)
                {
                    ui32MipiCfg = MIPICFG_16RGB565_OPT0;
                }
                if (ui8DbiWidth == 8)
                {
                    ui32MipiCfg = MIPICFG_8RGB565_OPT0;
                }
                break;

            default:
                //
                // invalid color component index
                //
                return -3;
        }
    }

    //
    // Initialize the display
    //
    switch (g_sDispCfg.eInterface)
    {
        case DISP_IF_SPI4:
            am_devices_dc_xspi_raydium_hardware_reset();
            sDCConfig.eInterface = DISP_INTERFACE_SPI4;
            sDCConfig.ui32PixelFormat = MIPICFG_1RGB888_OPT0;
            sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
            nemadc_configure(&sDCConfig);
            am_devices_dc_xspi_raydium_init(&sDisplayPanelConfig);
            break;
        case DISP_IF_DSPI:
            am_devices_dc_xspi_raydium_hardware_reset();
            sDCConfig.eInterface = DISP_INTERFACE_DSPI;
            sDCConfig.ui32PixelFormat = MIPICFG_2RGB888_OPT0;
            sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
            nemadc_configure(&sDCConfig);
            am_devices_dc_xspi_raydium_init(&sDisplayPanelConfig);

            break;
        case DISP_IF_QSPI:
            am_devices_dc_xspi_raydium_hardware_reset();
            sDCConfig.eInterface = DISP_INTERFACE_QSPI;
            sDCConfig.ui32PixelFormat = MIPICFG_4RGB888_OPT0;
            sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
            nemadc_configure(&sDCConfig);
            am_devices_dc_xspi_raydium_init(&sDisplayPanelConfig);
            break;
        case DISP_IF_DSI:
            am_devices_dc_dsi_raydium_hardware_reset();
            sDCConfig.eInterface = DISP_INTERFACE_DBIDSI;
            sDCConfig.ui32PixelFormat = ui32MipiCfg;
            sDisplayPanelConfig.ui32PixelFormat = ui32MipiCfg;
            nemadc_configure(&sDCConfig);
            am_devices_dc_dsi_raydium_init(((am_devices_dc_dsi_raydium_config_t *) &sDisplayPanelConfig));
            break;
        default:
            ; //NOP
    }
    return 0;
}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************

void
DisplayTask(void *pvParameters)
{
    int ret;

    am_util_stdio_printf("Display task start!\n");
    //Init display system, including DC, DSI(or SPI), panel
    ret = display_setup();
    TEST_ASSERT_TRUE(ret == 0);
    if (ret != 0)
    {
        am_util_stdio_printf("display init failed!\n");

        //suspend and delete this task.
        vTaskDelete(NULL);
    }


    while(1)
    {
        //Wait start.
        xSemaphoreTake( g_semDCStart, portMAX_DELAY);

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_clear(25);
        am_hal_gpio_output_clear(26);
#endif

        //This API will directly return if display system is still power on.
        display_power_up(ui32MipiCfg, FB_RESX, FB_RESY / SMALLFB_STRIPES);

        //Set panel refresh region
        #ifdef SMALLFB
        display_region_set(g_intDisplayPart);
        #endif

        //Set the data location to be send by DC.
        g_sLayer.baseaddr_phys = g_pFrameBufferDC->bo.base_phys;
        g_sLayer.baseaddr_virt = g_pFrameBufferDC->bo.base_virt;
        nemadc_set_layer(DC_LAYER_TO_USE, &g_sLayer);

        //Start DC
        display_refresh_start();

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_set(23);
#endif

        //Wait DC complete interrupt.
        nemadc_wait_vsync();
        //Do follow-up operations required by hardware.
        display_refresh_end();

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_clear(23);
#endif

        xSemaphoreGive(g_semDCEnd);

        //Power Down
        display_power_down();

#ifdef USE_DEBUG_PIN_IN_GUI_TASK
        am_hal_gpio_output_set(25);
#endif

    }

}
