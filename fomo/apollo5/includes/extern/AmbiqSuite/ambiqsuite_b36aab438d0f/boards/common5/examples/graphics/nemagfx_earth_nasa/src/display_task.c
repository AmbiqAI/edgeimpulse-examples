//*****************************************************************************
//
//! @file display_task.c
//!
//! @brief Task to handle display operations.
//!
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
// Includes.
//
//*****************************************************************************
#include "nemagfx_earth_nasa.h"
#include "rtos.h"
#include "display_task.h"
#include "render_task.h"

//*****************************************************************************
//
// Color foramt.
//
//*****************************************************************************
#ifdef GPU_FORMAT_RGB888
#define DISPLAY_COLOR_FORMAT (COLOR_FORMAT_RGB888)
#else
#define DISPLAY_COLOR_FORMAT (COLOR_FORMAT_RGB565)
#endif


//*****************************************************************************
//
// Display task handle.
//
//*****************************************************************************
TaskHandle_t DisplayTaskHandle;



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

//*****************************************************************************
//
//! @brief Disable both DC and DSI
//!
//! @return 0
//
//*****************************************************************************
static int32_t
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

//*****************************************************************************
//
//! @brief Enable DC and DSI
//!
//! @return 0.
//
//*****************************************************************************
static int32_t
display_power_up(void)
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
// Display task handle.
//
//*****************************************************************************
TaskHandle_t DisplayTaskHandle;

SemaphoreHandle_t g_semDCStart = NULL;
SemaphoreHandle_t g_semDCEnd = NULL;

//*****************************************************************************
//
// Display task.
//
//*****************************************************************************
void
DisplayTask(void *pvParameters)
{
#ifndef GPU_FORMAT_RGB888
    //
    // Decrease DSI frequency to limit it within the panel's frequency range
    //
    g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X13;
#endif
    g_sDispCfg.eIC = DISP_IC_CO5300;
    //
    // It has the restriction that the layer size is a multiple of 4 for Apollo5.
    //
    g_sDispCfg.ui16ResX = RESX;
    g_sDispCfg.ui16ResY = RESY;
    g_sDispCfg.ui16Offset = 6;
    //
    // Initialize Display controller and the Panel.
    //
    am_devices_display_init(RESX,
                            RESY,
                            DISPLAY_COLOR_FORMAT,
                            false);

    while (1)
    {
        //
        // Release the semphone to indicate the display is ready.
        //                       
        xSemaphoreGive(g_semDisplayEnd);
#if (DISP_CTRL_IP == DISP_CTRL_IP_DC)
        //
        // Power down DC/DSI
        //
        display_power_down();
#endif
        xSemaphoreTake(g_semDisplayStart, portMAX_DELAY);

#if (DISP_CTRL_IP == DISP_CTRL_IP_DC)
        //
        // Power up DC/DSI
        //
        display_power_up();
#endif
        //
        // Transfer frame to the penel
        //
        am_devices_display_transfer_frame(g_sFrameBuffer[0].w,
                                          g_sFrameBuffer[0].h,
                                          g_sFrameBuffer[0].bo.base_phys,
                                          NULL,
                                          NULL);
        //
        // wait transfer done
        //
        am_devices_display_wait_transfer_done();

    }
}
