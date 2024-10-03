//*****************************************************************************
//
//! @file nemagfx_test_common.c
//!
//! @brief NemaGFX test cases common source code.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "nemagfx_test_common.h"

//*****************************************************************************
//
// PSRAM setting
//
//*****************************************************************************
#ifdef USE_PSRAM

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
#if defined(APOLLO5_FPGA) || defined(NEMAGFX_COMMON_FORCE_OCTAL_PSRAM)
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,
#else
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
#endif
#ifdef NEMAGFX_COMMON_USE_192M_PSRAM
    // 96MHz MSPI SCLK w/ DDR == 192MHz Mtransfers/s
    .eClockFreq               = AM_HAL_MSPI_CLK_192MHZ,
#elif defined NEMAGFX_COMMON_USE_250M_PSRAM
    // 125MHz MSPI SCLK w/ DDR == 250MHz Mtransfers/s
    .eClockFreq               = AM_HAL_MSPI_CLK_250MHZ,
#else
    // 48MHz MSPI SCLK w/ DDR == 96MHz Mtransfers/s
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
#endif
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
#endif

//*****************************************************************************
//
// Optional Global setup/tear-down.
//
// globalSetUp() will get called before the test group starts, and
// globalTearDown() will get called after it ends. These won't be necessary for
// every test.
//
//*****************************************************************************
void
globalSetUp(void)
{

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Print the banner
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("apollo5 NemaGFX Test Cases\n\n");

#ifdef NEMA_USE_CUSTOM_MALLOC
    tcm_heap_init(LV_MEM_TCM_ADR, LV_MEM_TCM_SIZE, 256);
    ssram_heap_init(LV_MEM_SSRAM_ADR, LV_MEM_SSRAM_SIZE, 16*1024);
    psram_heap_init(LV_MEM_PSRAM_ADR, LV_MEM_PSRAM_SIZE, 64*1024);
#endif

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

    //
    // Initialize NemaGFX
    //
    if(nema_init() != 0)
    {
        am_util_stdio_printf("GPU init failed!\n");
    }

    uint32_t ui32Status;

#ifdef USE_PSRAM
    //
    // Run MSPI DDR timing scan
    //
    am_devices_mspi_psram_ddr_timing_config_t MSPIDdrTimingConfig;
    am_util_debug_printf("Starting MSPI DDR Timing Scan: \n");
    if ( AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == am_devices_mspi_psram_aps25616n_ddr_init_timing_check(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &MSPIDdrTimingConfig) )
    {
        am_util_debug_printf("==== Scan Result: RXDQSDELAY0 = %d \n", MSPIDdrTimingConfig.ui32Rxdqsdelay);
    }
    else
    {
        am_util_debug_printf("==== Scan Result: Failed, no valid setting.  \n");
    }

    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_SetPriority(MspiInterrupts[MSPI_PSRAM_MODULE], PSRAM_ISR_PRIORITY);
    NVIC_EnableIRQ(MspiInterrupts[MSPI_PSRAM_MODULE]);

    //
    // Apply DDR timing setting
    //
    ui32Status = am_devices_mspi_psram_aps25616n_apply_ddr_timing(g_pPsramHandle, &MSPIDdrTimingConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to apply the timming scan parameter!\n");
    }

    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }
#endif

}

void
globalTearDown(void)
{
#ifdef USE_PSRAM

    uint32_t ui32Status;

    //
    // Need to make sure all pending XIPMM transactions are flushed
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    ui32Status = am_devices_mspi_psram_aps25616n_ddr_disable_xip(g_pPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to disable XIP mode in the MSPI!\n");
    }

    ui32Status = am_devices_mspi_psram_aps25616n_ddr_deinit(g_pPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to deinit the MSPI and PSRAM Device correctly!\n");
    }
#endif
}

#ifdef USE_NEMADC

//*****************************************************************************
//
//! @brief Read data from display module.
//!
//! @param eCmd - Read command.
//! @param ui32Length - Number of read bytes.
//!
//! @return ui32RData - the data read back.
//
//*****************************************************************************
uint32_t
am_nemadc_read(uint32_t eCmd, uint32_t ui32Length)
{
    uint32_t ui32RData = 0;
    uint32_t ui32DBIConfig;
    uint32_t ui32Cfg;
    switch(ui32Length)
    {
        case 1:
            ui32Cfg = 0;
            break;
        case 2:
            ui32Cfg = NemaDC_rcmd16;
            break;
        case 3:
            ui32Cfg = NemaDC_rcmd24;
            break;
        case 4:
            ui32Cfg = NemaDC_rcmd32;
            break;
        default:
            return 0;
    }

    switch (g_sDispCfg.eInterface)
    {
        case DISP_IF_SPI4:
            //
            // 12MHz FPGA image requires longer delay than 48MHz image, so changed delay from 20us to 100us, added some margin.
            //
            nemadc_MIPI_out(NemaDC_DBI_cmd | NemaDC_DBI_read | ui32Cfg | eCmd);
            while((nemadc_reg_read(NEMADC_REG_STATUS) & 0x1c00U) != 0U);
            am_util_delay_us(100);
            ui32RData = nemadc_reg_read(NEMADC_REG_DBIB_RDAT);
            break;
        case DISP_IF_DSPI:
            ui32DBIConfig = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
            nemadc_MIPI_CFG_out(ui32DBIConfig & (~MIPICFG_DSPI) & (~MIPICFG_QSPI));
            nemadc_MIPI_out(NemaDC_DBI_cmd | NemaDC_DBI_read | ui32Cfg | eCmd);
            while((nemadc_reg_read(NEMADC_REG_STATUS) & 0x1c00U) != 0U);
            am_util_delay_us(100);
            ui32RData = nemadc_reg_read(NEMADC_REG_DBIB_RDAT);
            nemadc_MIPI_CFG_out(ui32DBIConfig);
            break;
        case DISP_IF_QSPI:
            // QSPI only supports 1 byte read.
            ui32DBIConfig = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
            nemadc_MIPI_CFG_out(ui32DBIConfig | MIPICFG_SPI_HOLD);
            nemadc_MIPI_out(NemaDC_DBI_cmd | MIPI_MASK_QSPI | SPI_READ);
            nemadc_MIPI_out(NemaDC_DBI_cmd | MIPI_MASK_QSPI | MIPI_CMD16 | (eCmd));
            nemadc_MIPI_out(NemaDC_DBI_cmd | MIPI_MASK_QSPI | NemaDC_DBI_read);
            while((nemadc_reg_read(NEMADC_REG_STATUS) & 0x1c00U) != 0U);
            nemadc_MIPI_CFG_out(ui32DBIConfig);
            am_util_delay_us(100);
            ui32RData = nemadc_reg_read(NEMADC_REG_DBIB_RDAT);
            break;
        default:
            break;
    }

    return ui32RData;
}

uint32_t
dc_common_interface(uint16_t ui16ResX,uint16_t ui16ResY)
{
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_xspi_raydium_config_t sDisplayPanelConfig;
    uint32_t ui32MipiCfg = MIPICFG_16RGB888_OPT0;    //!< default config
    //
    // Set the display region to center
    //
    if (ui16ResX < g_sDispCfg.ui16ResX)
    {
        sDisplayPanelConfig.ui16ResX = ui16ResX;
    }
    else
    {
        sDisplayPanelConfig.ui16ResX = g_sDispCfg.ui16ResX;
    }
    ui16MinX = (g_sDispCfg.ui16ResX - sDisplayPanelConfig.ui16ResX) >> 1;
    ui16MinX = (ui16MinX >> 1) << 1;

    if (ui16ResY < g_sDispCfg.ui16ResY)
    {
        sDisplayPanelConfig.ui16ResY = ui16ResY;
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
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);

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
#endif
