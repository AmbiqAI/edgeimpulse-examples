//*****************************************************************************
//
//! @file nemadc_spi4_test_cases.c
//!
//! @brief NemaDC spi4 interface test cases.
//! Need to connect Display with DC SPI4 interface, and to connect APS25616 PSRAM
//! card to MSPI0.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "nemadc_test_common.h"
#include "psram.h"
#include "windmill_402x476_rgba.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define FB_RESX 402
#define FB_RESY 476
#define CRC_REF 0x238589D2

#define ENABLE_XIPMM
#define MSPI_INT_TIMEOUT        (100)

#define DDR_PSRAM_TARGET_ADDRESS 0
#define MSPI_BUFFER_SIZE        (4*1024)  // 4K example buffer size.

#define DEFAULT_TIMEOUT         10000

#define MSPI_TEST_MODULE              0

#if (MSPI_TEST_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS 0x14000000
#elif (MSPI_TEST_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS 0x18000000
#elif (MSPI_TEST_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS 0x1C000000
#endif // #if (MSPI_TEST_MODULE == 0)

uint32_t        DMATCBBuffer[2560];
uint8_t         TestBuffer[2048];
uint8_t         DummyBuffer[1024];
uint8_t         g_TXBuffer[MSPI_BUFFER_SIZE];
uint8_t         g_RXBuffer[MSPI_BUFFER_SIZE];
void            *g_pDevHandle;
void            *g_pHandle;

am_devices_mspi_psram_config_t MSPI_PSRAM_OctalCE0MSPIConfig =
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
    am_mspi_isr1(MSPI_TEST_MODULE)
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
//*****************************************************************************
//
//! @brief NemaDC spi4 test case with fb in PSRAM
//!
//!
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
nemadc_spi4_psram_test(void)
{
    bool bTestPass = true;
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_xspi_raydium_config_t sDisplayPanelConfig;
    uint32_t ui32PixelFormat = MIPICFG_1RGB888_OPT0;
    uint32_t ui32Status;
    uint32_t ui32CRC;
    //
    // Assign a fixed value to display type.
    //
    g_sDispCfg.eInterface = DISP_IF_SPI4;
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

    sDisplayPanelConfig.ui16ResY = FB_RESY;
    ui16MinY = 0;

    g_sDispCfg.eTEType = DISP_TE_DISABLE;
    sDCConfig.ui16ResX = sDisplayPanelConfig.ui16ResX;
    sDCConfig.ui16ResY = sDisplayPanelConfig.ui16ResY;
    sDCConfig.bTEEnable = (g_sDispCfg.eTEType == DISP_TE_DC);
    sDisplayPanelConfig.ui16MinX = ui16MinX + g_sDispCfg.ui16Offset;
    sDisplayPanelConfig.ui16MinY = ui16MinY;
    sDisplayPanelConfig.bTEEnable = (g_sDispCfg.eTEType != DISP_TE_DISABLE);
    sDisplayPanelConfig.bFlip = g_sDispCfg.bFlip;

    am_bsp_disp_pins_enable();

    am_devices_dc_xspi_raydium_hardware_reset();
    sDCConfig.eInterface = DISP_INTERFACE_SPI4;
    sDCConfig.ui32PixelFormat = ui32PixelFormat;
    sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
    nemadc_configure(&sDCConfig);
    am_devices_dc_xspi_raydium_init(&sDisplayPanelConfig);

    //
    // Init PSRAM device.
    //
#ifdef WINBOND_PSRAM
    ui32Status = am_devices_mspi_psram_w958d6nw_ddr_init(MSPI_TEST_MODULE, &MSPI_PSRAM_OctalCE0MSPIConfig, &g_pDevHandle, &g_pHandle);
#else
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_TEST_MODULE, &MSPI_PSRAM_OctalCE0MSPIConfig, &g_pDevHandle, &g_pHandle);
#endif
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_EnableIRQ(mspi_interrupts[MSPI_TEST_MODULE]);
    //
    // Enable XIP mode.
    //
#ifdef WINBOND_PSRAM
    ui32Status = am_devices_mspi_psram_w958d6nw_ddr_enable_xip(g_pDevHandle);
#else
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pDevHandle);
#endif
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }

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

    memcpy((char*)layer0.baseaddr_virt, windmill_402x476_rgba, sizeof(windmill_402x476_rgba));
#ifndef APS25616N_OCTAL_MODE_EN
    am_util_stdio_printf("\nHEX MODE MSPI\n");
#else
    am_util_stdio_printf("\nOCTAL MODE MSPI\n");
#endif

    nemadc_set_layer(0, &layer0); // This function includes layer enable.

    nemadc_transfer_frame_prepare(false);
    //
    //It's necessary to launch frame manually when TE is disabled.
    //
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();

    ui32CRC = nemadc_reg_read(NEMADC_REG_CRC);
    nemadc_layer_disable(0);
    tsi_free(layer0.baseaddr_virt);
    if (ui32CRC != CRC_REF)
    {
        bTestPass = false;
    }
    am_util_stdio_printf("\nExpected DC CRC is 0x%08X, current CRC is 0x%08X.\n", CRC_REF, ui32CRC);
    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;
} // nemadc_spi4_psram_test()
