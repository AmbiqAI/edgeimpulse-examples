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
#include "lvgl_enhanced_stress_test.h"
#if DISPLAY_TASK_ENABLE == 1
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
bool
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
bool
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
        nemadc_backup_registers();
        am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_DISP);
    }
    return true;
}

//*****************************************************************************
//
//! @brief Enable & Restore Display controller
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
        am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);
        if (0 != nemadc_init())
        {
            //
            // Initialize DC failed.
            //
            return false;
        }
        return nemadc_restore_registers();
    }
    return true;
}

int32_t
display_power_down(void)
{
    dc_power_down();
    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        dphy_power_down();
    }
    else
    {
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_OFF, NULL);
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_DISABLE, NULL);
    }
    return 0;
}

int32_t
display_power_up()
{
    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        dphy_power_up();
    }
    else
    {
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC96, NULL);
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);
    }
    dc_power_up();
    return 0;
}
#endif

//*****************************************************************************
//
//! @brief Power off GPU
//!
//! @note after called this function,please don't operation any GPU registers
//! until enable and initial the GPU.
//!
//! @return 0: success. 
//!         -1: GPU is busy, try it later.
//
//*****************************************************************************
int32_t
gpu_power_down(void)
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_GFX, &status);
    if ( status )
    {
        if(nema_reg_read(NEMA_STATUS) == 0)
        {
            // If GPU is not busy, we can power off the GPU safely.
            am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);
        }
        else
        {
            return -1;
        }
    }

    return 0;
}
//*****************************************************************************
//
//! @brief Power on GPU power and restore nemaGFX context.
//!
//! @return 0: GPU power have initialize completely.
//!         -1: GPU power initialize error.
//
//*****************************************************************************
int32_t
gpu_power_up(void)
{
    int32_t i32Ret = 0;
    bool enabled;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_GFX, &enabled);
    if ( !enabled )
    {
        am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
        //
        // Initialize NemaGFX
        //
        nema_reinit();
        if(nema_get_error() != NEMA_ERR_NO_ERROR)
        {
            am_util_debug_printf("Nemagfx reinit error!\n");
            return -1;
        }

        nema_reset_last_cl_id();
    }
    return i32Ret;
}

#endif
