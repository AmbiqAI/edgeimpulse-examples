//*****************************************************************************
//
//! @file dsi_rw_command.c
//!
//! @brief DSI Read/Write Command Example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup dsi_rw_command DSI Read/Write Command Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example demonstrates how to read and write RM67162 display registers with DSI.
//!
//! 1-lane DSI includes 4 signals,
//!   * Differential clock lane - positive (CLKP)
//!   * Differential clock lane - negative (CLKN)
//!   * Differential data lane 0 - positive (D0P)
//!   * Differential data lane 0 - negative (D0N).
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

#include "dsi_rw_command.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define FB_RESX 200
#define FB_RESY 200

//*****************************************************************************
//
//! @brief Print test results
//! @param uint8_t ui8DbiWidth, uint32_t ui32FreqTrim, cmd_type_enum eCmdType, bool bHS, bool bItemPassype
//! @return void.
//
//*****************************************************************************
void
result_print(uint8_t ui8DbiWidth, uint32_t ui32FreqTrim, bool bHS, \
             am_hal_dsi_cmd_type_e eCmdType, bool bWr, uint8_t ui8NumPara, uint8_t ui8NumData, bool bItemPass)
{
    char * pcStr1 = "HS";
    char * pcStr2 = "LP";
    char * pcStr3 = "DCS";
    char * pcStr4 = "Generic";
    char * pcStr5 = "long";
    char * pcStr6 = "short";
    char * pcStr7 = "passed";
    char * pcStr8 = "failed";
    char * psSpeedMode;
    char * psCmdType;
    char * psLength;
    char * psResult;
    if (bHS == true)
    {
        //! HS
        psSpeedMode = pcStr1;
    }
    else
    {
        //! LP
        psSpeedMode = pcStr2;
    }
    if (eCmdType == DCS_CMD)
    {
        psCmdType = pcStr3;
    }
    else
    {
        psCmdType = pcStr4;
    }
    if (bWr == true)
    {
        if (((eCmdType == DCS_CMD) && (ui8NumPara > 1))
            || ((eCmdType == GE_CMD) && (ui8NumPara > 2)))
        {
            psLength = pcStr5;
        }
        else
        {
            psLength = pcStr6;
        }
    }
    if (bItemPass == true)
    {
        psResult = pcStr7;
    }
    else
    {
        psResult = pcStr8;
    }
    if (bWr == true)
    {
        am_util_stdio_printf("%d-bit DBI width, Frequence trim value - 0x%02X, %s mode, %s %s write command, %d parameter(s), test is %s.\n", \
                              ui8DbiWidth, ui32FreqTrim, psSpeedMode, psCmdType, psLength, ui8NumPara, psResult);
    }
    else
    {
        am_util_stdio_printf("%d-bit DBI width, Frequence trim value - 0x%02X, %s mode, %s read command, %d parameter(s), %d byte(s) of returned data, test is %s.\n", \
                              ui8DbiWidth, ui32FreqTrim, psSpeedMode, psCmdType, ui8NumPara, ui8NumData, psResult);
    }
}

//*****************************************************************************
//
//! @brief Test DCS/Generic read/write commands
//! @param am_hal_dsi_cmd_type_e eCmdType, bool bHS
//! @return bPass.
//
//*****************************************************************************
bool
test_dsi_cmd(am_hal_dsi_cmd_type_e eCmdType, bool bHS)
{
    bool bPass = true;
    bool bItemPass, bWr;
    uint8_t ui8LanesNum = g_sDispCfg.ui8NumLanes;
    uint8_t ui8DbiWidth = g_sDispCfg.eDbiWidth;
    uint32_t ui32FreqTrim = g_sDispCfg.eDsiFreq;
    uint8_t ui8CmdBuf[4];
    uint8_t ui8NumWrPara = 0, ui8NumRdPara = 0, ui8NumRdData = 0;
    uint32_t ui32DsiRdVal = 0;
    //
    // Initialize DSI
    //
    if (am_hal_dsi_para_config(ui8LanesNum, ui8DbiWidth, ui32FreqTrim, false) != 0)
    {
        bPass = false;
        return bPass;
    }

    //
    // Enable dc clock
    //
    nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, NemaDC_clkctrl_cg_clk_en);
    //
    // Set clock divider. B2 and later versions of Apollo4 support setting DC primary clock divide ratio to 1.
    //
    if (APOLLO4_GE_B2)
    {
        nemadc_clkdiv(1, 1, 4, 0);
    }
    else
    {
        nemadc_clkdiv(2, 1, 4, 0);
    }
    //
    // Enable fast pixel generation slow transfer
    //
    if (APOLLO4_GE_B2)
    {
        nemadc_reg_write(NEMADC_REG_CLKCTRL_CG,
                        (NemaDC_clkctrl_cg_clk_swap |
                         NemaDC_clkctrl_cg_l0_bus_clk |
                         NemaDC_clkctrl_cg_clk_en));
    }
    else
    {
        nemadc_reg_write(NEMADC_REG_CLKCTRL_CG,
                        (NemaDC_clkctrl_cg_clk_swap |
                         NemaDC_clkctrl_cg_clk_en));
    }
    nemadc_clkctrl((uint32_t)TB_LCDPANEL_MIPI_DBIB);
    if (APOLLO4_GE_B2)
    {
        nemadc_MIPI_CFG_out(MIPICFG_DBI_EN          |
                            MIPICFG_RESX            |
                            MIPICFG_EXT_CTRL        |
                            MIPICFG_DIS_TE          |
                            MIPICFG_EN_STALL        |
                            MIPICFG_PIXCLK_OUT_EN);
    }
    else
    {
        nemadc_MIPI_CFG_out(MIPICFG_DBI_EN          |
                            MIPICFG_RESX            |
                            MIPICFG_EXT_CTRL        |
                            MIPICFG_DIS_TE          |
                            MIPICFG_PIXCLK_OUT_EN);

    }
    //
    // hardware reset
    //
    am_devices_dc_dsi_raydium_hardware_reset();

    if (eCmdType == DCS_CMD)
    {
        //
        // DCS short write with no parameter, DCS read with no parameter and 1 byte of returned data.
        //
        am_util_delay_ms(10);
        ui8NumWrPara = 0;
        nemadc_mipi_cmd_write(MIPI_set_display_on, NULL, ui8NumWrPara, true,bHS);

        am_util_delay_us(100);
        ui8NumRdData = 1;
        nemadc_mipi_cmd_read(MIPI_get_power_mode,NULL,0,&ui32DsiRdVal,ui8NumRdData,true,bHS);
        //
        // check display on register
        //
        if ((ui32DsiRdVal & 0x00000004 ) != 0)
        {
            bItemPass = true;
        }
        else
        {
            bItemPass = false;
            bPass = false;
        }
        bWr = true;
        result_print(ui8DbiWidth, ui32FreqTrim, bHS, eCmdType, bWr, ui8NumWrPara, 0, bItemPass);
        //
        // DCS short write with 1 parameter, DCS read with no parameter and 1 byte of returned data.
        //
        am_util_delay_ms(10);
        ui8CmdBuf[0] = 0x05;
        ui8NumWrPara = 1;
        nemadc_mipi_cmd_write(MIPI_set_pixel_format, ui8CmdBuf, ui8NumWrPara, true,bHS);
        ui8NumRdData = 1;
        //
        // DCS read with no parameter and 1 byte of returned data.
        //
        nemadc_mipi_cmd_read(MIPI_get_pixel_format,NULL,0,&ui32DsiRdVal,ui8NumRdData,true,bHS);
        if (ui32DsiRdVal == ui8CmdBuf[0])
        {
            bItemPass = true;
        }
        else
        {
            bItemPass = false;
            bPass = false;
        }
        bWr = true;
        result_print(ui8DbiWidth, ui32FreqTrim, bHS, eCmdType, bWr, ui8NumWrPara, 0, bItemPass);
        bWr = false;
        result_print(ui8DbiWidth, ui32FreqTrim, bHS, eCmdType, bWr, ui8NumRdPara, ui8NumRdData, bItemPass);
        //
        // DCS long write with 2 parameters, DCS read with no parameter and 2 bytes of returned data.
        //
        DSI->MAXRETPACSZE_b.COUNTVAL = 2;
        DSI->MAXRETPACSZE_b.HSLP = 1;
        am_util_delay_ms(10);
        ui8CmdBuf[0] = 0x08;
        ui8CmdBuf[1] = 0x0A;
        ui8NumWrPara = 2;
        nemadc_mipi_cmd_write(MIPI_set_tear_scanline, ui8CmdBuf, ui8NumWrPara, true,bHS);
        ui8NumRdData = 2;
        nemadc_mipi_cmd_read(MIPI_get_scanline,NULL,0,&ui32DsiRdVal,ui8NumRdData,true,bHS);
        if ((((ui32DsiRdVal & 0x0000FF00) >> 8) == ui8CmdBuf[0]) && ((ui32DsiRdVal & 0x000000FF) == ui8CmdBuf[1]))
        {
            bItemPass = true;
        }
        else
        {
            bItemPass = false;
            bPass = false;
        }
        bWr = true;
        result_print(ui8DbiWidth, ui32FreqTrim, bHS, eCmdType, bWr, ui8NumWrPara, 0, bItemPass);
        bWr = false;
        result_print(ui8DbiWidth, ui32FreqTrim, bHS, eCmdType, bWr, ui8NumRdPara, ui8NumRdData, bItemPass);
// #### INTERNAL BEGIN ####
//
// Do not test this case right now.
//
#if 0
        //
        // DCS long write, DCS read with no parameter and 4 bytes of returned data.
        //
        am_util_delay_ms(10);
        DSI->MAXRETPACSZE_b.COUNTVAL = 4;
        DSI->MAXRETPACSZE_b.HSLP = 1;
        ui8CmdBuf[0] = 0x02;
        ui8CmdBuf[1] = 0x04;
        ui8CmdBuf[2] = 0x08;
        ui8CmdBuf[3] = 0x10;
        ui8NumWrPara = 4;
        nemadc_mipi_cmd_write(MIPI_write_memory_start, ui8CmdBuf, ui8NumWrPara, true,bHS);
        ui8NumRdData = 4;
        nemadc_mipi_cmd_read(MIPI_read_memory_start,NULL,0,&ui32DsiRdVal,ui8NumRdData,true,bHS);
        if (((( ui32DsiRdVal & 0xFF000000 ) >> 24 ) == ui8CmdBuf[0] ) && ((( ui32DsiRdVal & 0x00FF0000 ) >> 16 ) == ui8CmdBuf[1] )
            && ((( ui32DsiRdVal & 0x0000FF00 ) >> 8 ) == ui8CmdBuf[2] ) && (( ui32DsiRdVal & 0x000000FF ) == ui8CmdBuf[3] ))
        {
            bItemPass = true;
        }
        else
        {
            bItemPass = false;
            bPass = false;
        }
        bWr = true;
        result_print(ui8DbiWidth, ui32FreqTrim, bHS, eCmdType, bWr, ui8NumWrPara, 0, bItemPass);
        bWr = false;
        result_print(ui8DbiWidth, ui32FreqTrim, bHS, eCmdType, bWr, ui8NumRdPara, ui8NumRdData, bItemPass);
#endif
// #### INTERNAL END ####
    }
    else
    {
        //
        // Generic read with 1 parameter and 1 byte of returned data.
        //
        ui8CmdBuf[0] = 0x54;
        ui8NumRdPara = 1;
        ui8NumRdData = 1;
        //
        // Read default value
        //
        nemadc_mipi_cmd_read(0,ui8CmdBuf,ui8NumRdPara,&ui32DsiRdVal,ui8NumRdData,false,bHS);
        if (ui32DsiRdVal == 0x28)
        {
            bItemPass = true;
        }
        else
        {
            bItemPass = false;
            bPass = false;
        }
        bWr = false;
        result_print(ui8DbiWidth, ui32FreqTrim, bHS, eCmdType, bWr, ui8NumRdPara, ui8NumRdData, bItemPass);
        //
        // Generic short write with 1 paremeter, generic read with 1 parameter and 1 byte of returned data.
        //
        am_util_delay_ms(10);
        ui8CmdBuf[0] = MIPI_set_display_on;
        ui8NumWrPara = 1;
        nemadc_mipi_cmd_write(0,ui8CmdBuf, ui8NumWrPara, false,bHS);
        //
        // get display's power mode
        //
        ui8CmdBuf[0] = MIPI_get_power_mode;
        ui8NumRdPara = 1;
        ui8NumRdData = 1;
        nemadc_mipi_cmd_read(0,ui8CmdBuf,ui8NumRdPara,&ui32DsiRdVal,ui8NumRdData,false,bHS);
        //
        // check display on register
        //
        if ((ui32DsiRdVal & 0x00000004) != 0)
        {
            bItemPass = true;
        }
        else
        {
            bItemPass = false;
            bPass = false;
        }
        bWr = true;
        result_print(ui8DbiWidth, ui32FreqTrim, bHS, eCmdType, bWr, ui8NumWrPara, 0, bItemPass);
        //
        // Generic short write with 2 paremeters, generic read with 1 parameter and 1 byte of returned data.
        //
        am_util_delay_ms(10);
        ui8CmdBuf[0] = 0x58;
        ui8CmdBuf[1] = 0x06;
        ui8NumWrPara = 2;
        nemadc_mipi_cmd_write(0,ui8CmdBuf, ui8NumWrPara,false, bHS);
        ui8CmdBuf[0] = 0x59;
        ui8NumRdPara = 1;
        ui8NumRdData = 1;
        nemadc_mipi_cmd_read(0,ui8CmdBuf,ui8NumRdPara,&ui32DsiRdVal,ui8NumRdData,false,bHS);
        if (ui32DsiRdVal == ui8CmdBuf[1])
        {
            bItemPass = true;
        }
        else
        {
            bItemPass = false;
            bPass = false;
        }
        bWr = true;
        result_print(ui8DbiWidth, ui32FreqTrim, bHS, eCmdType, bWr, ui8NumWrPara, 0, bItemPass);
        //
        // Generic long write with 3 parameters, generic read with no parameter and 2 bytes of returned data.
        //
        am_util_delay_ms(10);
        DSI->MAXRETPACSZE_b.COUNTVAL = 2;
        DSI->MAXRETPACSZE_b.HSLP = 1;
        //
// #### INTERNAL BEGIN ####
        // FIXME
// #### INTERNAL END ####
        //
        ui8CmdBuf[0] = MIPI_set_tear_scanline;
        ui8CmdBuf[1] = 0x08;
        ui8CmdBuf[2] = 0x0A;
        ui8NumWrPara = 3;
        nemadc_mipi_cmd_write(0,ui8CmdBuf, ui8NumWrPara, false,bHS);
        //
// #### INTERNAL BEGIN ####
        // FIXME
// #### INTERNAL END ####
        //
        //
        ui8CmdBuf[0] = MIPI_get_scanline;
        ui8NumRdPara = 1;
        ui8NumRdData = 2;
        nemadc_mipi_cmd_read(0,ui8CmdBuf,ui8NumRdPara,&ui32DsiRdVal,ui8NumRdData,false,bHS);
        if ((((ui32DsiRdVal & 0x0000FF00 ) >> 8) == ui8CmdBuf[1]) /*&& ((ui32DsiRdVal & 0x000000FF ) == ui8CmdBuf[2])*/)
        {
            bItemPass = true;
        }
        else
        {
            bItemPass = false;
            bPass = false;
        }
        bWr = true;
        result_print(ui8DbiWidth, ui32FreqTrim, bHS, eCmdType, bWr, ui8NumWrPara, 0, bItemPass);
        bWr = false;
        result_print(ui8DbiWidth, ui32FreqTrim, bHS, eCmdType, bWr, ui8NumRdPara, ui8NumRdData, bItemPass);
//
// Do not test these 2 cases right now.
//
#if 0
        //
        // Generic short write with 0 paremeters, generic read with 0 parameter.
        //
        ui8NumWrPara = 0;
        nemadc_mipi_cmd_write(0,ui8CmdBuf, ui8NumWrPara, false,bHS);
        ui8NumRdPara = 0;
        ui8NumRdData = 1;
        nemadc_mipi_cmd_read(0,ui8CmdBuf,ui8NumRdPara,&ui32DsiRdVal,ui8NumRdData,false,bHS);
        //
        // Generic read with 2 parameters is not supported by Rev A and Rev B0.
        //
        DSI->MAXRETPACSZE_b.COUNTVAL = 1;
        DSI->MAXRETPACSZE_b.HSLP = 1;
        ui8CmdBuf[0] = MIPI_get_scanline;
        //! 1st byte
        ui8CmdBuf[1] = 0x0;
        ui8NumRdPara = 2;
        ui8NumRdData = 1;
        nemadc_mipi_cmd_read(0,ui8CmdBuf,ui8NumRdPara,&ui32DsiRdVal,ui8NumRdData,false,bHS);
#endif
    }
    return bPass;
}

//*****************************************************************************
//
//! @brief Test DSI interface.
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
dsi_rw_command(void)
{
    bool bTestPass = true;
    bool bRet = true;
    //
    // Initialize NemaDC
    //
    if (nemadc_init() != 0)
    {
        am_util_stdio_printf("DC init failed!\n");
    }

    bRet = test_dsi_cmd(DCS_CMD, HS_MODE);
    if (bRet == false)
    {
        bTestPass = false;
    }
    am_util_delay_ms(50);
    bRet = test_dsi_cmd(DCS_CMD, LP_MODE);
    if (bRet == false)
    {
        bTestPass = false;
    }
    am_util_delay_ms(50);
    bRet = test_dsi_cmd(GE_CMD, HS_MODE);
    if (bRet == false)
    {
        bTestPass = false;
    }
    am_util_delay_ms(50);
    bRet = test_dsi_cmd(GE_CMD, LP_MODE);
    if (bRet == false)
    {
        bTestPass = false;
    }
    am_util_delay_ms(50);
    if (bTestPass == true)
    {
        am_util_stdio_printf("\nAll tests passed!\n");
    }
    else
    {
        am_util_stdio_printf("\nAt least 1 test failed!\n");
    }

    return bTestPass;

} // dsi_rw_command()

//*****************************************************************************
//
// Main function
//
//*****************************************************************************
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
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();
    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

    //
    // This example is just for DSI test. If you want to change display configurations in example, please change them here as below.
    //
    // g_sDsiCfg.ui8NumLanes = 1;
    // g_sDsiCfg.eDbiWidth = AM_HAL_DSI_DBI_WIDTH_8;
    // g_sDsiCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X12;
    if (g_sDispCfg.eInterface != DISP_IF_DSI)
    {
        return 0;   //!< If the interface in BSP isn't set to DSI, then return.
        // g_eDispType = RM67162_DSI;   //!< This line forces set display type to a DSI panel if the interface in BSP isn't set to DSI.
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
    am_util_stdio_printf("\nDSI read/write commands tests.\n");

    am_bsp_disp_pins_enable();
    //
    // VDD18 control callback function
    //
    am_hal_dsi_register_external_vdd18_callback(am_bsp_external_vdd18_switch);
    //
    // Enable DSI power and configure DSI clock.
    //
    am_hal_dsi_init();

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);

    dsi_rw_command();

    while (1)
    {
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

