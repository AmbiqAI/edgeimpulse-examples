//*****************************************************************************
//
//! @file am_hal_dsi.c
//!
//! @brief Hardware abstraction for the DSI
//!
//! @addtogroup
//! @ingroup
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"

//*****************************************************************************
//
// Configure DSI frequency and timing
//
//*****************************************************************************
uint32_t
am_hal_dsi_timing(uint32_t ui32FreqTrim)
{
    //
    // DPHY CLK/DATA timing parameters
    //
    if ((ui32FreqTrim & 0x0F) >= 0x0A)
    {
        DSI->DPHYPARAM_b.HSPREP = 0x05;
        DSI->DPHYPARAM_b.HSZERO = 0x0A;
        DSI->DPHYPARAM_b.HSTRAIL = 0x05;
        DSI->DPHYPARAM_b.HSEXIT = 0x0C;
        DSI->CLKLANETIMPARM_b.HSPREP = 0x02;
        DSI->CLKLANETIMPARM_b.HSZERO = 0x0C;
        DSI->CLKLANETIMPARM_b.HSTRAIL = 0x03;
        DSI->CLKLANETIMPARM_b.HSEXIT = 0x04;
    }
    else if ((ui32FreqTrim & 0x0F) >= 0x07)
    {
        DSI->DPHYPARAM_b.HSPREP = 0x04;
        DSI->DPHYPARAM_b.HSZERO = 0x0A;
        DSI->DPHYPARAM_b.HSTRAIL = 0x05;
        DSI->DPHYPARAM_b.HSEXIT = 0x0C;
        DSI->CLKLANETIMPARM_b.HSPREP = 0x02;
        DSI->CLKLANETIMPARM_b.HSZERO = 0x0C;
        DSI->CLKLANETIMPARM_b.HSTRAIL = 0x03;
        DSI->CLKLANETIMPARM_b.HSEXIT = 0x04;
    }
    else if ((ui32FreqTrim & 0x0F) >= 0x05)
    {
        DSI->DPHYPARAM_b.HSPREP = 0x03;
        DSI->DPHYPARAM_b.HSZERO = 0x0A;
        DSI->DPHYPARAM_b.HSTRAIL = 0x05;
        DSI->DPHYPARAM_b.HSEXIT = 0x0C;
        DSI->CLKLANETIMPARM_b.HSPREP = 0x02;
        DSI->CLKLANETIMPARM_b.HSZERO = 0x0C;
        DSI->CLKLANETIMPARM_b.HSTRAIL = 0x03;
        DSI->CLKLANETIMPARM_b.HSEXIT = 0x04;
    }
    else if ((ui32FreqTrim & 0x0F) >= 0x04)
    {
        DSI->DPHYPARAM_b.HSPREP = 0x02;
        DSI->DPHYPARAM_b.HSZERO = 0x0A;
        DSI->DPHYPARAM_b.HSTRAIL = 0x05;
        DSI->DPHYPARAM_b.HSEXIT = 0x0C;
        DSI->CLKLANETIMPARM_b.HSPREP = 0x02;
        DSI->CLKLANETIMPARM_b.HSZERO = 0x0C;
        DSI->CLKLANETIMPARM_b.HSTRAIL = 0x03;
        DSI->CLKLANETIMPARM_b.HSEXIT = 0x04;
    }
    else
    {
        DSI->DPHYPARAM_b.HSPREP = 0x01;
        DSI->DPHYPARAM_b.HSZERO = 0x0A;
        DSI->DPHYPARAM_b.HSTRAIL = 0x05;
        DSI->DPHYPARAM_b.HSEXIT = 0x0C;
        DSI->CLKLANETIMPARM_b.HSPREP = 0x02;
        DSI->CLKLANETIMPARM_b.HSZERO = 0x0C;
        DSI->CLKLANETIMPARM_b.HSTRAIL = 0x03;
        DSI->CLKLANETIMPARM_b.HSEXIT = 0x04;
    }
    //
    // PLL settings
    //
    DSI->AFETRIM1 &= _VAL2FLD(DSI_AFETRIM1_AFETRIM1, ~0x0000007F);
    DSI->AFETRIM1 |= _VAL2FLD(DSI_AFETRIM1_AFETRIM1, ui32FreqTrim);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Initialize the DSI
//
//*****************************************************************************
uint32_t
am_hal_dsi_para_config(uint8_t ui8LanesNum, uint8_t ui8DBIBusWidth, uint32_t ui32FreqTrim, bool bSendUlpsPattern)
{

    //
    // ui32DSIFuncPrg_REG (DATA_WIDTH, RESERVED, FMT_VIDEO, CH_NO_CM, CH_NO_VM, DATA_LANE_CNT)
    //                   [15:13]     [12:10]   [9:7]      [6:5]     [4:3]     [2:0]
    //
    uint32_t ui32DSIFuncPrg = 0;
    //
    // check number of lanes parameters
    //
    switch (ui8LanesNum)
    {
        case 1:
          ui32DSIFuncPrg |= _VAL2FLD(DSI_DSIFUNCPRG_DATALANES, 0x1);
          break;

        case 2:
          ui32DSIFuncPrg |= _VAL2FLD(DSI_DSIFUNCPRG_DATALANES, 0x2);
          break;

        default:
          return AM_HAL_STATUS_OUT_OF_RANGE;
    }
    //
    // check DBI bus width parameter
    //
    switch (ui8DBIBusWidth)
    {
        case 8:
          ui32DSIFuncPrg |= _VAL2FLD(DSI_DSIFUNCPRG_REGNAME, 3);
          break;

        case 9:
          ui32DSIFuncPrg |= _VAL2FLD(DSI_DSIFUNCPRG_REGNAME, 2);
          break;

        case 16:
          ui32DSIFuncPrg |= _VAL2FLD(DSI_DSIFUNCPRG_REGNAME, 4); // opt1
          break;

        default:
          return AM_HAL_STATUS_OUT_OF_RANGE;
    }

    DSI->RSTENBDFE = _VAL2FLD(DSI_RSTENBDFE_ENABLE, 0);
    //
    // write into DSI functional programming register
    //
    DSI->DSIFUNCPRG = ui32DSIFuncPrg;
    //
    // write into HIGH SPEED RECEIVE TIMEOUT REGISTER
    //
    DSI->HSTXTIMEOUT = _VAL2FLD(DSI_HSTXTIMEOUT_MAXDURTOCNT, 0x00FFFFFF);
    //
    //write into LOW POWER RECEIVE TIMEOUT REGISTER
    //
    DSI->LPRXTO = _VAL2FLD(DSI_LPRXTO_TOCHKRVS, 0xFF);
    //
    // write into TURN AROUND TIMEOUT REGISTER
    //
    DSI->TURNARNDTO = _VAL2FLD(DSI_TURNARNDTO_TIMOUT, 0x1F);
    //
    // write into DEVICE RESET TIMER REGISTER
    //
    DSI->DEVICERESETTIMER = _VAL2FLD(DSI_DEVICERESETTIMER_TIMOUT, 0xFF);
    //
    // write into HIGH TO LOW SWITCH COUNT REGISTER
    //
    DSI->DATALANEHILOSWCNT = _VAL2FLD(DSI_DATALANEHILOSWCNT_DATALHLSWCNT, 0xFFFF);
    DSI->INITCNT = _VAL2FLD(DSI_INITCNT_MSTR, 0x7d0);
    DSI->LPBYTECLK = _VAL2FLD(DSI_LPBYTECLK_VALBYTECLK, 0x3);
    DSI->CLKEOT = _VAL2FLD(DSI_CLKEOT_CLOCK, 1);
    am_hal_dsi_timing(ui32FreqTrim);
    DSI->AFETRIM2 = _VAL2FLD(DSI_AFETRIM2_AFETRIM2, 0x10000000);
    DSI->AFETRIM2 |= _VAL2FLD(DSI_AFETRIM2_AFETRIM2, 0x00480000); // trim_2<22> and trim_2<19> need to be set for DSI TX in 1-lane configuration.
    //
    // enable DPHY
    //
    DSI->RSTENBDFE = _VAL2FLD(DSI_RSTENBDFE_ENABLE, 1);
    DSI->DEVICEREADY = _VAL2FLD(DSI_DEVICEREADY_READY, 1);
    //
    // Wait for DPHY init
    //
    am_hal_delay_us(200);
    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Initialize power and clock of DSI
//
//*****************************************************************************
uint32_t
am_hal_dsi_init(void)
{
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISPPHY);

    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_DPHYPLL, NULL);
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_PLLCLKSEL_HFRC12, NULL);
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_PLLCLK_ENABLE, NULL);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Turn off power and clock of DSI
//
//*****************************************************************************
uint32_t
am_hal_dsi_deinit(void)
{
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_DISPPHY);

    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_OFF, NULL);
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_DISABLE, NULL);
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_PLLCLKSEL_OFF, NULL);
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_PLLCLK_DISABLE, NULL);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Enter ULPS mode
//
//*****************************************************************************
uint32_t
am_hal_dsi_ulps_entry(void)
{
    DSI->AFETRIM3 |= _VAL2FLD(DSI_AFETRIM3_AFETRIM3, 0x00038000); //  trim_3<15>, trim_3<16>, and trim_3<17> need to be set
    DSI->AFETRIM2 |= _VAL2FLD(DSI_AFETRIM2_AFETRIM2, 0x0000001C); //  trim_2<2> & trim_2<3> need to be set
    DSI->AFETRIM1 |= _VAL2FLD(DSI_AFETRIM1_AFETRIM1, 0x00000200); //  trim_1<9> needs to be set

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Exit ULPS mode
//
//*****************************************************************************
uint32_t
am_hal_dsi_ulps_exit(void)
{
    DSI->AFETRIM3 &= _VAL2FLD(DSI_AFETRIM3_AFETRIM3, ~0x00038000); //  trim_3<15>, trim_3<16>, and trim_3<17> need to be cleared
    DSI->AFETRIM2 &= _VAL2FLD(DSI_AFETRIM2_AFETRIM2, ~0x0000001C); //  trim_2<2> & trim_2<3> need to be cleared
    DSI->AFETRIM1 &= _VAL2FLD(DSI_AFETRIM1_AFETRIM1, ~0x00000200); //  trim_1<9> needs to be cleared

    return AM_HAL_STATUS_SUCCESS;

}
//*****************************************************************************
//
// End Doxygen group.
//
//*****************************************************************************
