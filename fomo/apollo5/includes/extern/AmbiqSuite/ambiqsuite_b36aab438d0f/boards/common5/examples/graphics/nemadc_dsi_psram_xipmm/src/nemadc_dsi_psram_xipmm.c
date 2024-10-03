//*****************************************************************************
//
//! @file nemadc_dsi_psram_xipmm.c
//!
//! @brief DSI example.
//!
//! This example demonstrates how to drive a MIPI DSI panel.
//!
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
#include "nema_utils.h"
#include "nemadc_dsi_psram_xipmm.h"
#include "am_devices_dc_dsi_raydium.h"
#include "am_devices_mspi_psram_aps25616n.h"
#include "pinwheel_456x456_rgba.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
//
// It has the restriction that the layer size is a multiple of 4 for Apollo5.
//
#define FB_RESX 456
#define FB_RESY 456

#define MSPI_BUFFER_SIZE        (4*1024)  // 4K example buffer size.

#define MSPI_PSRAM_MODULE              0

#if (MSPI_PSRAM_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS           (MSPI0_APERTURE_START_ADDR)
#elif (MSPI_PSRAM_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS           (MSPI1_APERTURE_START_ADDR)
#elif (MSPI_PSRAM_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS           (MSPI2_APERTURE_START_ADDR)
#endif // #if (MSPI_PSRAM_MODULE == 0)

uint32_t        DMATCBBuffer[2560];
uint8_t         TestBuffer[2048];
uint8_t         DummyBuffer[1024];
uint8_t         g_TXBuffer[MSPI_BUFFER_SIZE];
uint8_t         g_RXBuffer[MSPI_BUFFER_SIZE];
void            *g_pDevHandle;
void            *g_pHandle;

am_devices_mspi_psram_config_t MSPI_PSRAM_Config =
{
#ifndef APS25616N_OCTAL_MODE_EN
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
#else
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,
#endif
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
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
void
psram_mspi_isr(void)
{
    uint32_t ui32Status;
    am_hal_mspi_interrupt_status_get(g_pHandle, &ui32Status, false);
    am_hal_mspi_interrupt_clear(g_pHandle, ui32Status);
    am_hal_mspi_interrupt_service(g_pHandle, ui32Status);
}


int32_t
dsi_psram_xipmm(void)
{
    int32_t i32Ret;
    uint32_t ui32MipiCfg = MIPICFG_8RGB888_OPT0; //!< default config
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_dsi_raydium_config_t sDisplayPanelConfig;
    //
    // Set the display region to center
    //
    sDisplayPanelConfig.ui16ResX = FB_RESX;
    if (FB_RESX < g_sDispCfg.ui16ResX)
    {
        ui16MinX = (g_sDispCfg.ui16ResX - sDisplayPanelConfig.ui16ResX) >> 1;
        ui16MinX = (ui16MinX >> 1) << 1;
    }
    else
    {
        ui16MinX = 0;
    }

    sDisplayPanelConfig.ui16ResY = FB_RESY;
    if (FB_RESY < g_sDispCfg.ui16ResY)
    {
        ui16MinY = (g_sDispCfg.ui16ResY - sDisplayPanelConfig.ui16ResY) >> 1;
        ui16MinY = (ui16MinY >> 1) << 1;
    }
    else
    {
        ui16MinY = 0;
    }

    g_sDispCfg.eTEType = DISP_TE_DISABLE;
    sDCConfig.ui16ResX = sDisplayPanelConfig.ui16ResX;
    sDCConfig.ui16ResY = sDisplayPanelConfig.ui16ResY;
    sDCConfig.bTEEnable = (g_sDispCfg.eTEType == DISP_TE_DC);
    sDisplayPanelConfig.ui16MinX = ui16MinX + g_sDispCfg.ui16Offset;
    sDisplayPanelConfig.ui16MinY = ui16MinY;
    sDisplayPanelConfig.bTEEnable = (g_sDispCfg.eTEType != DISP_TE_DISABLE);
    sDisplayPanelConfig.bFlip = g_sDispCfg.bFlip;
    //
    // Initialize NemaGFX
    //
    i32Ret = nema_sys_init();
    if (i32Ret != 0)
    {
        am_util_stdio_printf("GPU init failed!\n");
        return i32Ret;
    }
    //
    // Initialize Nema|dc
    //
    i32Ret = nemadc_init();
    if (i32Ret != 0)
    {
        am_util_stdio_printf("DC init failed!\n");
        return i32Ret;
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

    am_devices_dc_dsi_raydium_hardware_reset();
    sDCConfig.eInterface = DISP_INTERFACE_DBIDSI;
    sDCConfig.ui32PixelFormat = ui32MipiCfg;
    sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
    nemadc_configure(&sDCConfig);
    am_devices_dc_dsi_raydium_init(&sDisplayPanelConfig);

    //
    // send layer 0 to display via NemaDC
    //
    nemadc_layer_t layer0 = {0};
    layer0.resx          = FB_RESX;
    layer0.resy          = FB_RESY;
    layer0.buscfg        = 0;
    layer0.format        = NEMADC_RGBA8888;
    layer0.blendmode     = NEMADC_BL_SRC;
    layer0.stride        = layer0.resx * 4;
    layer0.startx        = 0;
    layer0.starty        = 0;
    layer0.sizex         = layer0.resx;
    layer0.sizey         = layer0.resy;
    layer0.alpha         = 0xff;
    layer0.flipx_en      = 0;
    layer0.flipy_en      = 0;
    layer0.baseaddr_virt = (void *)MSPI_XIP_BASE_ADDRESS;
    layer0.baseaddr_phys = (unsigned)(layer0.baseaddr_virt);

    nema_memcpy((char*)layer0.baseaddr_virt, pinwheel_456x456_rgba, sizeof(pinwheel_456x456_rgba));

    nemadc_set_layer(0, &layer0); // This function includes layer enable.

    nemadc_transfer_frame_prepare(g_sDispCfg.eTEType != DISP_TE_DISABLE);
    if ( g_sDispCfg.eTEType == DISP_TE_DISABLE )
    {
        //
        //It's necessary to launch frame manually when TE is disabled.
        //
        nemadc_transfer_frame_launch();
    }
    nemadc_wait_vsync();
    nemadc_layer_disable(0);
    tsi_free(layer0.baseaddr_virt);
    return 0;
}


//*****************************************************************************
//
// Main function
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

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();
    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    //
    // Print a banner.
    //
    am_util_stdio_printf("namedc_dsi_psram_xipmm example.\n");
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();
    am_devices_mspi_psram_ddr_timing_config_t MSPIDdrTimingConfig;
    am_util_stdio_printf("Starting MSPI DDR Timing Scan: \n");
    if ( AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == am_devices_mspi_psram_aps25616n_ddr_init_timing_check(MSPI_PSRAM_MODULE, &MSPI_PSRAM_Config, &MSPIDdrTimingConfig) )
    {
        am_util_stdio_printf("==== Scan Result: RXDQSDELAY0 = %d \n", MSPIDdrTimingConfig.ui32Rxdqsdelay);
    }
    else
    {
        am_util_stdio_printf("==== Scan Result: Failed, no valid setting.  \n");
        while(1);
    }
    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_Config, &g_pDevHandle, &g_pHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_EnableIRQ(mspi_interrupts[MSPI_PSRAM_MODULE]);
    //
    //  Set the DDR timing from previous scan.
    //
    am_devices_mspi_psram_aps25616n_apply_ddr_timing(g_pDevHandle, &MSPIDdrTimingConfig);
    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pDevHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }
    am_bsp_disp_pins_enable();

    //
    // to avoid DC transmission image distortion for octal-SPI PSRAM.
    //
    g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X18;
    if (g_sDispCfg.eInterface != DISP_IF_DSI)
    {
        return 0;
    }
    //
    // VDD18 control callback function
    //
    am_hal_dsi_register_external_vdd18_callback(am_bsp_external_vdd18_switch);
    //
    // Enable DSI power and configure DSI clock.
    //
    am_hal_dsi_init();

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);

    dsi_psram_xipmm();

    while (1)
    {
    }

}

