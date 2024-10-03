//*****************************************************************************
//
//! @file am_widget_iis.c
//!
//! @brief Test widget for testing IIS channels by data transfer using fram.
//
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <string.h>
#include <ctype.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_widget_iis.h"

//*****************************************************************************
//
// I2S pins
//
//*****************************************************************************
#define I2S_DATA_IN_GPIO_FUNC    AM_HAL_PIN_14_I2S0_SDIN
#define I2S_DATA_IN_GPIO_PIN     14
#define I2S_DATA_OUT_GPIO_FUNC   AM_HAL_PIN_12_I2S0_SDOUT
#define I2S_DATA_OUT_GPIO_PIN    12
#define I2S_CLK_GPIO_FUNC        AM_HAL_PIN_11_I2S0_CLK
#define I2S_CLK_GPIO_PIN         11
#define I2S_WS_GPIO_FUNC         AM_HAL_PIN_13_I2S0_WS
#define I2S_WS_GPIO_PIN          13

#define I2S1_SLAVE_DATA_IN_GPIO_FUNC     AM_HAL_PIN_19_I2S1_SDIN
#define I2S1_SLAVE_DATA_IN_GPIO_PIN      19
#define I2S1_SLAVE_DATA_OUT_GPIO_FUNC    AM_HAL_PIN_17_I2S1_SDOUT
#define I2S1_SLAVE_DATA_OUT_GPIO_PIN     17

#define I2S1_SLAVE_CLK_GPIO_FUNC         AM_HAL_PIN_16_I2S1_CLK
#define I2S1_SLAVE_CLK_GPIO_PIN          16
#define I2S1_SLAVE_WS_GPIO_FUNC          AM_HAL_PIN_18_I2S1_WS
#define I2S1_SLAVE_WS_GPIO_PIN           18

extern void *I2S0Handle;
extern void *I2S1Handle;

uint32_t am_widget_iis_pin_enable(void)
{

    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;
    //
    // Configure the necessary pins.
    //
    am_hal_gpio_pincfg_t sPinCfg =
    {
      .GP.cfg_b.eGPOutCfg = 1,
      .GP.cfg_b.ePullup   = 0
    };

    sPinCfg.GP.cfg_b.uFuncSel = I2S_DATA_OUT_GPIO_FUNC;
    ui32Status = am_hal_gpio_pinconfig(I2S_DATA_OUT_GPIO_PIN, sPinCfg);
    sPinCfg.GP.cfg_b.uFuncSel = I2S_DATA_IN_GPIO_FUNC;
    ui32Status = am_hal_gpio_pinconfig(I2S_DATA_IN_GPIO_PIN, sPinCfg);

    sPinCfg.GP.cfg_b.uFuncSel = I2S_CLK_GPIO_FUNC;
    ui32Status = am_hal_gpio_pinconfig(I2S_CLK_GPIO_PIN, sPinCfg);
    sPinCfg.GP.cfg_b.uFuncSel = I2S_WS_GPIO_FUNC;
    ui32Status = am_hal_gpio_pinconfig(I2S_WS_GPIO_PIN, sPinCfg);

    sPinCfg.GP.cfg_b.uFuncSel = I2S1_SLAVE_DATA_OUT_GPIO_FUNC;
    ui32Status = am_hal_gpio_pinconfig(I2S1_SLAVE_DATA_OUT_GPIO_PIN, sPinCfg);
    sPinCfg.GP.cfg_b.uFuncSel = I2S1_SLAVE_DATA_IN_GPIO_FUNC;
    ui32Status = am_hal_gpio_pinconfig(I2S1_SLAVE_DATA_IN_GPIO_PIN, sPinCfg);

    sPinCfg.GP.cfg_b.uFuncSel = I2S1_SLAVE_CLK_GPIO_FUNC;
    ui32Status = am_hal_gpio_pinconfig(I2S1_SLAVE_CLK_GPIO_PIN, sPinCfg);
    sPinCfg.GP.cfg_b.uFuncSel = I2S1_SLAVE_WS_GPIO_FUNC;
    ui32Status = am_hal_gpio_pinconfig(I2S1_SLAVE_WS_GPIO_PIN, sPinCfg);

    return ui32Status;
}

uint32_t am_widget_iis_build_configuration(uint32_t iis_channel,
                                            i2s_loopback_test_settings_t* test_setting,
                                            am_hal_i2s_config_t* config)
{
    // build iis config according to test setting

    if(test_setting->master_channel == iis_channel)
    {
        config->eMode = AM_HAL_I2S_IO_MODE_MASTER;
        config->eXfer = test_setting->master_role;
    }
    else
    {
        config->eMode = AM_HAL_I2S_IO_MODE_SLAVE;
        // set slave role corresponding to the master role
        if(test_setting->master_role == AM_HAL_I2S_XFER_RXTX)
        {
            config->eXfer = AM_HAL_I2S_XFER_RXTX;
        }
        else if(test_setting->master_role == AM_HAL_I2S_XFER_RX)
        {
            config->eXfer = AM_HAL_I2S_XFER_TX;
        }
        else
        {
            config->eXfer = AM_HAL_I2S_XFER_RX;
        }
    }

    config->eClock = (am_hal_i2s_clksel_e)test_setting->clock_source;
    //
    // io configurations
    //
    config->eIO->eFyncCpol = test_setting->fsync_cpol;
    config->eIO->eRxCpol = test_setting->rx_cpol;
    config->eIO->eTxCpol = test_setting->tx_cpol;

    //
    // data configurations
    //
    config->eData->ePhase       = test_setting->phase;
    config->eData->eDataJust    = test_setting->justify;
    config->eData->eChannelLenPhase1    = test_setting->data_width_1;
    config->eData->eChannelLenPhase2    = test_setting->data_width_2;
//    config->eData->eSampleLenPhase1

    config->eDiv3 = 0;
    config->eASRC = 0;

    return AM_WIDGET_IIS_SUCCESS;
}

void am_widget_iis_handler_deinitialize(void)
{
  am_hal_i2s_deinitialize(I2S0Handle);
  am_hal_i2s_deinitialize(I2S1Handle);
}
