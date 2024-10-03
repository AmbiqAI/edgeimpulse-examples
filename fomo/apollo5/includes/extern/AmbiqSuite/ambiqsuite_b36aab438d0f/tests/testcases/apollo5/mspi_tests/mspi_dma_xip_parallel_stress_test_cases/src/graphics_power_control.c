//*****************************************************************************
//
//! @file graphics_power_control.c
//!
//! @brief Power ON/OFF GPU,DC,DSI(DPHY).
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "mspi_dma_xip_parallel_stress_test_cases.h"
#if DISPLAY_TASK_ENABLE==1
#include "nema_core.h"
#include "nema_utils.h"
#include "nema_event.h"
#include "nema_hal.h"
#include "nema_regs.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************


#if (DISP_CTRL_IP == DISP_CTRL_IP_DC)
#include "nema_dc_mipi.h"
#include "nema_dc.h"
#include "nema_dc_regs.h"
//
// DSI sends ULPS pattern on or off when DSI enters/exits ULPS mode
//
#define ULPS_PATTERN_ON     true

static uint32_t nema_dc_reg_config = 0;
static uint32_t nema_dc_reg_resxy  = 0;

static uint32_t dsi_reg_function   = 0;
static uint32_t dsi_reg_afetrim1   = 0;

//*****************************************************************************
//
//! @brief Disable DSI to reduce power consumption.
//!
//! Retained two important present registers value for DSI power on.
//!
//! @return true
//
//*****************************************************************************
static bool
dphy_power_down(void)
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_DISPPHY, &status);
    if ( status )
    {
        //
        // Get DSI functional programming register
        //
        dsi_reg_function = DSI->DSIFUNCPRG;

        //
        // Get DSI trim register that included DSI frequency
        //
        dsi_reg_afetrim1 = DSI->AFETRIM1 & 0x0000007F;

        am_hal_dsi_napping(ULPS_PATTERN_ON);
    }
    return true;
}

//*****************************************************************************
//
//! @brief Enable DSI
//!
//! Resume the previous DSI configuration.
//!
//! @return bool.
//
//*****************************************************************************
static bool
dphy_power_up(void)
{
    if (dsi_reg_afetrim1 == 0 && dsi_reg_function == 0)
    {
        //
        // DSI configuration parameters are invalid.
        //
        return false;
    }

    uint32_t ui32FreqTrim = _FLD2VAL(DSI_AFETRIM1_AFETRIM1, dsi_reg_afetrim1);
    uint8_t ui8LanesNum = (uint8_t)_FLD2VAL(DSI_DSIFUNCPRG_DATALANES, dsi_reg_function);
    uint8_t ui8DBIBusWidth = (uint8_t)_FLD2VAL(DSI_DSIFUNCPRG_REGNAME, dsi_reg_function);
    if (4 == ui8DBIBusWidth)
    {
        ui8DBIBusWidth = 16;
    }
    else if (3 == ui8DBIBusWidth)
    {
        ui8DBIBusWidth = 8;
    }
    else if (2 == ui8DBIBusWidth)
    {
        ui8DBIBusWidth = 9;
    }
    else
    {
        return false;
    }

    am_hal_dsi_wakeup(ui8LanesNum, ui8DBIBusWidth, ui32FreqTrim, ULPS_PATTERN_ON);
    return true;
}

//*****************************************************************************
//
//! @brief Disable Display Controller
//!
//! @note Retained two important present registers value for DC power on.
//!
//! @return true.
//
//*****************************************************************************
static bool
dc_power_down(void)
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_DISP, &status);
    if ( status )
    {
        //
        // Get DC present color format
        //
        nema_dc_reg_config = nemadc_reg_read(NEMADC_REG_INTERFACE_CFG);
        //
        // Get DC present resolution
        //
        nema_dc_reg_resxy = nemadc_reg_read(NEMADC_REG_RESXY);
        nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, 0);
        nemadc_MIPI_CFG_out(0);
        nemadc_set_mode(0);
        am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_DISP);
    }
    return true;
}

//*****************************************************************************
//
//! @brief Enable & Initilize Display controller
//!
//! Resume the previous DC configuration
//!
//! @return bool
//
//*****************************************************************************
static bool
dc_power_up(void)
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_DISP, &status);
    if ( !status )
    {
        if (nema_dc_reg_config == 0 && nema_dc_reg_resxy == 0)
        {
            //
            // DC configuration parameters are invalid.
            //
            return false;
        }

        am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);
        if (0 != nemadc_init())
        {
            //
            // Initialize DC failed.
            //
            return false;
        }

        nemadc_initial_config_t sDCConfig = {0};
        
        sDCConfig.ui32PixelFormat = nema_dc_reg_config & 0x1FF;
        uint32_t interface = sDCConfig.ui32PixelFormat >> 6 << 6;
        if ((interface == MIPICFG_PF_DBI8) || 
            (interface == MIPICFG_PF_DBI9) || 
            (interface == MIPICFG_PF_DBI16))
        {
           sDCConfig.eInterface = DISP_INTERFACE_DBIDSI;
        }
        else if(interface == MIPICFG_PF_QSPI)
        {
            sDCConfig.eInterface = DISP_INTERFACE_QSPI;
        }
        else if(interface == MIPICFG_PF_DSPI)
        {
            sDCConfig.eInterface = DISP_INTERFACE_DSPI;
        }
        else if(interface == MIPICFG_PF_SPI)
        {
            sDCConfig.eInterface = DISP_INTERFACE_SPI4;
        }
        else
        {
            //
            // Invalid interface.
            //
            return false;
        }

        sDCConfig.bTEEnable = (nema_dc_reg_config & MIPICFG_DIS_TE) != 0;

        sDCConfig.ui16ResX = (uint16_t)(nema_dc_reg_resxy >> 16);
        sDCConfig.ui16ResY = (uint16_t)(nema_dc_reg_resxy & 0xFFFF);

        nemadc_configure(&sDCConfig);
    }
    return true;
}
#endif

//*****************************************************************************
//
//! @brief this function used to disable GPU power
//!
//! @note after called this function,please don't operation any GPU registers
//! until enable and initial the GPU.
//!
//! @return 1- GPU power had disabled before
//!         0- GPU power have disabled now.
//
//*****************************************************************************
int32_t
gpu_power_down(void)
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_GFX, &status);
    if ( status )
    {
        //
        // Disable clock
        //
        nema_reg_write(NEMA_CGCTRL, 0);
        //
        // Reset GPU status
        //
        nema_reg_write(NEMA_STATUS, 0xFFFFFFFF);
        am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);
    }
    else
    {
        return 1;
    }
    return 0;
}
//*****************************************************************************
//
//! @brief this function used to enable GPU power and initialize nemaGFX
//!
//! @return 0- GPU power have initialize completely.
//!         other- GPU power initialize error.
//
//*****************************************************************************
int32_t
gpu_power_up(void)
{
    int32_t i32Ret = 1;
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_GFX, &status);
    if ( !status )
    {
        am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

    }
    //
    // Initialize NemaGFX
    //
    i32Ret = nema_init();
    
    return i32Ret;
}

#if (DISP_CTRL_IP == DISP_CTRL_IP_DC)
int32_t
display_power_down(void)
{
    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        dphy_power_down();
    }
    else
    {
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_OFF, NULL);
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_DISABLE, NULL);
    }
    dc_power_down();
    return 0;
}

int32_t
display_power_up()
{
    dc_power_up();
    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        dphy_power_up();
    }
    else
    {
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC96, NULL);
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);
    }
    return 0;
}

#endif
#endif