//*****************************************************************************
//
//! @file nemadc_test_common.c
//!
//! @brief Nemadc test cases common source code.
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

nemadc_initial_config_t g_sDCConfig;
am_devices_dc_xspi_raydium_config_t g_sDisplayPanelConfig;

//*****************************************************************************
//
// Optional setup/tear-down functions.
//
// These will be called before and after each test function listed below.
//
//*****************************************************************************
void
setUp(void)
{
}

void
tearDown(void)
{
}

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
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();
#if defined(ENABLE_DSI_IF)
    //
    // VDD18 control callback function
    //
    am_hal_dsi_register_external_vdd18_callback(am_bsp_external_vdd18_switch);
    //
    // Enable DSI power and configure DSI clock.
    //
    am_hal_dsi_init();
#else
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC96, NULL);
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);
#endif
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);
    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo5 NemaDC Test Cases\n\n");

    nema_sys_init();
    //
    // Initialize NemaDC
    //
    if (nemadc_init() != 0)
    {
        am_util_stdio_printf("NemaDC init failed!\n");
    }
    //
    // Initialize DSI
    //
#if defined(ENABLE_DSI_IF)
    if ( am_hal_dsi_para_config(g_sDispCfg.ui8NumLanes, g_sDispCfg.eDbiWidth, g_sDispCfg.eDsiFreq , false) != 0 )
    {
        am_util_stdio_printf("DSI init failed!\n");
    }
#endif
}

void
globalTearDown(void)
{
}

//*****************************************************************************
//
//! @brief Read data from display module.
//!
//! @param eCmd       - Read command.
//! @param ui32Length - Number of read bytes.
//!
//! @return ui32RData - the data read back.
//
//*****************************************************************************
uint32_t
am_nemadc_read(uint32_t eCmd, uint32_t ui32Length)
{
    uint32_t ui32RData = 0,ui32Cfg,ui32Mask;
    uint32_t ui32RReg = nemadc_reg_read(NEMADC_REG_CLKCTRL);
    //
    // defined special handle sign for SPI4/DSPI read 2,3 and 4 bytes data from RM69330 panel.
    //
    uint8_t is_special_handle_sign = 0;
#ifdef APOLLO5_FPGA
    //
    // Make sure the frequency after division is lower than 3MHz.
    //
    nemadc_clkdiv(1,(APOLLO5_FPGA + 5) / 6,4,0);
#else
    nemadc_clkdiv(1,16,4,0);
#endif
    switch(ui32Length)
    {
        case 1:
            ui32Cfg = 0;
            ui32Mask = 0xFF;
            break;
        case 2:
            ui32Cfg = NemaDC_rcmd16;
            ui32Mask = 0xFFFF;
            break;
        case 3:
            ui32Cfg = NemaDC_rcmd24;
            ui32Mask = 0xFFFFFF;
            break;
        case 4:
            ui32Cfg = NemaDC_rcmd32;
            ui32Mask = 0xFFFFFFFF;
            break;
        default:
            return 0;
    }
    am_util_delay_us(100); // Added a delay to wait CLK back to stable.
    //
    // disable clock gates
    //
    nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, NemaDC_clkctrl_cg_clk_en);
    uint32_t ui32DBIConfig = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    switch (g_sDispCfg.eInterface)
    {
        case DISP_IF_DSPI:
            nemadc_MIPI_CFG_out(ui32DBIConfig & ~(MIPICFG_DSPI | MIPICFG_QSPI));
        case DISP_IF_SPI4:
            if(ui32Length == 2)
            {
                //
                //Read 2bytes from panel RM69330,the output of the panel also lag a clock cycle.(only read 2bytes through commands such as:0x04,0x45,0xA1,0xA8)
                //
                is_special_handle_sign = 2;
                ui32Cfg = NemaDC_rcmd24;
            }
            else if(ui32Length == 3)
            {
                //
                //Read 3bytes from panel RM69330,the output of the panel also lag a clock cycle.(only read 3bytes through commands such as:0x04,0xA1,0xA8)
                //
                is_special_handle_sign = 3;
                ui32Cfg = NemaDC_rcmd32;
                //
                // read 33 bits of data from panel,ignore the MSB bit.(only keep low 32 bits)
                //
                nemadc_reg_write(NEMADC_REG_FORMAT_CTRL,(32<<16));
            }
            else if(ui32Length == 4)
            {
                //
                //Read 4bytes from panel RM69330,the output of the panel also lag a clock cycle.(only read 4bytes through commands such as:0xA1,0xA8)
                //
                //
                // read 33 bits of data from panel,ignore the MSB bit.(only keep low 32 bits)
                //
                nemadc_reg_write(NEMADC_REG_FORMAT_CTRL,(32<<16));
            }
            nemadc_MIPI_out(NemaDC_DBI_cmd | NemaDC_DBI_read | ui32Cfg | eCmd);
            am_util_delay_us(100);
            break;
        case DISP_IF_QSPI:
            if(ui32Length == 4)
            {
                nemadc_reg_write(NEMADC_REG_FORMAT_CTRL,(31<<16));
            }
            nemadc_MIPI_CFG_out(ui32DBIConfig | MIPICFG_SPI_HOLD);
            nemadc_MIPI_out(NemaDC_DBI_cmd | MIPI_MASK_QSPI | SPI_READ);
            nemadc_MIPI_out(NemaDC_DBI_cmd | MIPI_MASK_QSPI | MIPI_CMD16 | (eCmd));
            nemadc_MIPI_out(NemaDC_DBI_cmd | MIPI_MASK_QSPI | NemaDC_DBI_read|ui32Cfg);
            break;
        default:
            break;
    }

    while ((nemadc_reg_read(NEMADC_REG_STATUS) & 0x1c00U) != 0U);
    nemadc_MIPI_CFG_out(ui32DBIConfig);
    am_util_delay_us(100);
    ui32RData = nemadc_reg_read(NEMADC_REG_DBIB_RDAT);

    if(is_special_handle_sign == 2)
    {
        //
        // removed invalid low 7bits from data.
        //
        ui32RData >>= 7;
    }
    else if(is_special_handle_sign == 3)
    {
        //
        // removed invalid low 8bits from data.
        //
        ui32RData >>= 8;
    }
    //
    // disable clock gates
    //
    //nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, 0);

    //nemadc_reg_write(NEMADC_REG_CLKCTRL, ui32RReg); // Disabled format clock division.
    //am_util_delay_us(100); // Added a delay to wait CLK back to stable.

    return ui32RData & ui32Mask;
}

//*****************************************************************************
//
//! @brief initalize Display Controller interfaces
//!
//! @param ui16ResX                 - resolution X
//! @param ui16ResY                 - resolution Y
//! @param ui32ColorFormat          - color format and color format option
//!
//! @return uint32_t
//
//*****************************************************************************
uint32_t
dc_common_interface(uint16_t ui16ResX,uint16_t ui16ResY,
                    uint32_t ui32ColorFormat)
{
    uint16_t ui16MinX = 0, ui16MinY = 0;
    //
    // obtain valid bitfields of color format & color format option
    //
    uint32_t ui32FinalColor = ui32ColorFormat & 0x3F;
    //
    // Set the display region to center
    //
    g_sDisplayPanelConfig.ui16ResX = ui16ResX;
    if (ui16ResX < g_sDispCfg.ui16ResX)
    {
        ui16MinX = (g_sDispCfg.ui16ResX - g_sDisplayPanelConfig.ui16ResX) >> 2 << 1;
    }
    
    g_sDisplayPanelConfig.ui16ResY = ui16ResY;
    if (ui16ResY < g_sDispCfg.ui16ResY)
    {
        ui16MinY = (g_sDispCfg.ui16ResY - g_sDisplayPanelConfig.ui16ResY) >> 2 << 1;
    }

    //
    // Enable tear effect interrupt or not.
    //
    g_sDispCfg.eTEType = DISP_TE_DC;
    g_sDCConfig.ui16ResX = g_sDisplayPanelConfig.ui16ResX;
    g_sDCConfig.ui16ResY = g_sDisplayPanelConfig.ui16ResY;
    g_sDCConfig.bTEEnable = (g_sDispCfg.eTEType == DISP_TE_DC);
    g_sDisplayPanelConfig.ui16MinX = ui16MinX + g_sDispCfg.ui16Offset;
    g_sDisplayPanelConfig.ui16MinY = ui16MinY;
    g_sDisplayPanelConfig.bTEEnable = (g_sDispCfg.eTEType != DISP_TE_DISABLE);
    g_sDisplayPanelConfig.bFlip = g_sDispCfg.bFlip;

    //
    // It is important to select other interfaces before function am_bsp_disp_pins_enable();
    //
    // g_sDispCfg.eInterface = DISP_IF_QSPI;

    //
    // Initialize display pins configurations.
    //
    am_bsp_disp_pins_enable();

    //
    // Initialize DC module and panels
    //
    switch (g_sDispCfg.eInterface)
    {
        case DISP_IF_SPI4:
            am_devices_dc_xspi_raydium_hardware_reset();
            g_sDCConfig.eInterface = DISP_INTERFACE_SPI4;
            g_sDCConfig.ui32PixelFormat = ui32FinalColor | MIPICFG_PF_SPI;
            g_sDisplayPanelConfig.ui32PixelFormat = g_sDCConfig.ui32PixelFormat;
            nemadc_configure(&g_sDCConfig);
            am_devices_dc_xspi_raydium_init(&g_sDisplayPanelConfig);
            break;
        case DISP_IF_DSPI:
            am_devices_dc_xspi_raydium_hardware_reset();
            g_sDCConfig.eInterface = DISP_INTERFACE_DSPI;
            g_sDCConfig.ui32PixelFormat = ui32FinalColor | MIPICFG_PF_DSPI;
            g_sDisplayPanelConfig.ui32PixelFormat = g_sDCConfig.ui32PixelFormat;
            nemadc_configure(&g_sDCConfig);
            am_devices_dc_xspi_raydium_init(&g_sDisplayPanelConfig);
            break;
        case DISP_IF_QSPI:
            am_devices_dc_xspi_raydium_hardware_reset();
            g_sDCConfig.eInterface = DISP_INTERFACE_QSPI;
            g_sDCConfig.ui32PixelFormat = ui32FinalColor | MIPICFG_PF_QSPI;
            g_sDisplayPanelConfig.ui32PixelFormat = g_sDCConfig.ui32PixelFormat;
            nemadc_configure(&g_sDCConfig);
            am_devices_dc_xspi_raydium_init(&g_sDisplayPanelConfig);
            break;
        case DISP_IF_DSI:
            am_devices_dc_dsi_raydium_hardware_reset();
            g_sDCConfig.eInterface = DISP_INTERFACE_DBIDSI;
            g_sDCConfig.ui32PixelFormat = ui32FinalColor | MIPICFG_PF_DBI16;
            g_sDisplayPanelConfig.ui32PixelFormat = g_sDCConfig.ui32PixelFormat;
            nemadc_configure(&g_sDCConfig);
            am_devices_dc_dsi_raydium_init(((am_devices_dc_dsi_raydium_config_t *) &g_sDisplayPanelConfig));
            break;
        case DISP_IF_JDI:
            break;
        default:
            return 2;
            ; //NOP
    }

    return 0;
}
