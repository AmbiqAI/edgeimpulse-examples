//*****************************************************************************
//
//! @file nemadc_psram_test_cases.c
//!
//! @brief NemaDC access psram test cases.
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
#include "pinwheel_400x400_rgba.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define FB_RESX 400
#define FB_RESY 400
#define CRC_REF 0x6D2B93B1

#define MSPI_BUFFER_SIZE        (4*1024)  // 4K example buffer size.

#define MSPI_PSRAM_MODULE              0

#if (MSPI_PSRAM_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS           (MSPI0_APERTURE_START_ADDR)
#elif (MSPI_PSRAM_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS           (MSPI1_APERTURE_START_ADDR)
#elif (MSPI_PSRAM_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS           (MSPI2_APERTURE_START_ADDR)
#endif // #if (MSPI_PSRAM_MODULE == 0)

AM_SHARED_RW uint32_t DMATCBBuffer[2560];
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
nemadc_psram_test(void)
{
    bool bTestPass = true;
    uint32_t ui32Status;
    uint32_t ui32CRC;
    //
    // Assign a fixed value to display type.
    //
    g_sDispCfg.eInterface = DISP_IF_QSPI;
    dc_common_interface(FB_RESX,FB_RESY,MIPI_DCS_RGB888|MIPICFG_PF_OPT0);

    //
    // Init PSRAM device.
    //
#ifdef WINBOND_PSRAM
    ui32Status = am_devices_mspi_psram_w958d6nw_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_Config, &g_pDevHandle, &g_pHandle);
#else
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_Config, &g_pDevHandle, &g_pHandle);
#endif
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_EnableIRQ(mspi_interrupts[MSPI_PSRAM_MODULE]);
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

    nema_memcpy((char*)layer0.baseaddr_virt, pinwheel_400x400_rgba, sizeof(pinwheel_400x400_rgba));
    
#ifndef APS25616N_OCTAL_MODE_EN
    am_util_stdio_printf("\nHEX MODE MSPI\n");
#else
    am_util_stdio_printf("\nOCTAL MODE MSPI\n");
#endif

    nemadc_set_layer(0, &layer0); // This function includes layer enable.

    nemadc_transfer_frame_prepare(g_sDispCfg.eTEType != DISP_TE_DISABLE);
    if(g_sDispCfg.eTEType == DISP_TE_DISABLE)
    {
        //
        //It's necessary to launch frame manually when TE is disabled.
        //
        nemadc_transfer_frame_launch();
    }
    nemadc_wait_vsync();

    ui32CRC = nemadc_reg_read(NEMADC_REG_CRC);
    nemadc_layer_disable(0);
    tsi_free(layer0.baseaddr_virt);
    
    if (ui32CRC != CRC_REF)
    {
        bTestPass = false;
        am_util_stdio_printf("\nExpected DC CRC is 0x%08X, current CRC is 0x%08X.\n", CRC_REF, ui32CRC);
    }

    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;
} // nemadc_psram_test()
