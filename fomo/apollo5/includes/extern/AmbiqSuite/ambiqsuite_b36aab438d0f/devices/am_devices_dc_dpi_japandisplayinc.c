//*****************************************************************************
//
//! @file am_devices_dc_dpi_japandisplayinc.c
//!
//! @brief Generic Japan Display Inc. display driver with RGB/DPI interface. This
//! driver supports the display panel model type LPM027M700A.
//!
//! @addtogroup dc_dpi Japan Display Inc. DC DPI Driver
//! @ingroup devices
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

#include "am_devices_dc_dpi_japandisplayinc.h"
#include "am_util_delay.h"
#include "am_bsp.h"

#ifndef SIMULATION
#define DELAY am_util_delay_ms
#else
#define DELAY(...)
#endif

//
// the arguments of model type LPM027M700A
//
#define LPM027M700A_RESX                    400
#define LPM027M700A_RESY                    240
#define LPM027M700A_PCLK_MAX                (1.5)  //1.5MHz

#define LPM027M700A_FRONTPORCH_X            6
#define LPM027M700A_BLANKING_X              2
#define LPM027M700A_BACKPORCH_X             2

#define LPM027M700A_FRONTPORCH_Y            2
#define LPM027M700A_BLANKING_Y              1
#define LPM027M700A_BACKPORCH_Y             1

//
// Modifying color coding could cause the mapping of data bits to change, please update the data bits pin in BSP simultaneously.
//
#define LPM027M700A_INTERFACE_COLOR_CODING  NEMADC_P_RGB3_16B

#define JANPAN_DISPLAY_INC_COMIN_DUTYCYCLE     50

static uint32_t japandisplayinc_timer_number = JANPAN_DISPLAY_INC_COMIN_TIMER;

//*****************************************************************************
//
// Initialize timer that generate COMIN signal
//
//*****************************************************************************
static uint32_t
dc_dpi_pwm_init(uint32_t ui32TimerNum, am_hal_timer_clock_e eTimerClock, uint32_t ui32Frequency)
{
    uint64_t ui64BaseFreq, ui64EndCounts;
    am_hal_timer_config_t TimerConfig ;
    am_hal_timer_default_config_set( &TimerConfig ) ;
    TimerConfig.eFunction = AM_HAL_TIMER_FN_PWM;
    TimerConfig.eInputClock = eTimerClock;
    //
    // Configure the TIMER.
    //
    am_hal_timer_config(ui32TimerNum, &TimerConfig);
    //
    // Config output COMIN pins
    //
    am_hal_timer_output_config(am_bsp_disp_jdi_timer_pins(0), AM_HAL_TIMER_OUTPUT_TMR0_OUT1 + ui32TimerNum * 2);
// #### INTERNAL BEGIN ####
#ifdef APOLLO5_FPGA
    ui64BaseFreq = (APOLLO5_FPGA * 1000000ull) >> (2 * eTimerClock + 2);
#else
// #### INTERNAL END ####

    ui64BaseFreq = 96000000ull >> (2 * eTimerClock + 2);

// #### INTERNAL BEGIN ####
#endif
// #### INTERNAL END ####
    ui64EndCounts = (uint64_t)(ui64BaseFreq / ui32Frequency + 0.5f);
    TimerConfig.ui32Compare0    = (uint32_t) ui64EndCounts;
    am_hal_timer_compare0_set(ui32TimerNum, TimerConfig.ui32Compare0);

    //
    // the duty cycle computation for PWM mode
    //
    TimerConfig.ui32Compare1 = (uint32_t)(ui64EndCounts * (1 - JANPAN_DISPLAY_INC_COMIN_DUTYCYCLE / 100.0f) + 0.5f);
    am_hal_timer_compare1_set(ui32TimerNum, TimerConfig.ui32Compare1);

    japandisplayinc_timer_number = ui32TimerNum;
    return 0;
}

//*****************************************************************************
//
// Start PWM timer for panel model LPM027M700A
//
//*****************************************************************************
void
am_devices_dc_dpi_japandisplayinc_timer_start(void)
{
    //
    // Enable the TIMER.
    //
    am_hal_timer_enable(japandisplayinc_timer_number);
    //
    // Start the TIMER.
    //
    am_hal_timer_start(japandisplayinc_timer_number);
}

//*****************************************************************************
//
// Stop PWM timer for panel model LPM027M700A
//
//*****************************************************************************
void
am_devices_dc_dpi_japandisplayinc_timer_stop(void)
{
    //
    // Stop the TIMER.
    //
    am_hal_timer_clear_stop(japandisplayinc_timer_number);
    //
    // Disable the TIMER.
    //
    am_hal_timer_disable(japandisplayinc_timer_number);
}
//*****************************************************************************
//
// Initialize the Japan Display Inc. model LPM027M700A panel.
//
//*****************************************************************************
uint32_t
am_devices_dc_dpi_japandisplayinc_init(am_devices_dc_dpi_timer_config_t *pTimerConfig,
                                       nemadc_initial_config_t *pDCConfig)
{
    dc_dpi_pwm_init(pTimerConfig->ui32TimerNum, pTimerConfig->eTimerClock, pTimerConfig->ui32Frequency);

    pDCConfig->eInterface               = DISP_INTERFACE_DPI;
    pDCConfig->ui16ResX                 = LPM027M700A_RESX;
    pDCConfig->ui16ResY                 = LPM027M700A_RESY;

    pDCConfig->ui32FrontPorchX          = LPM027M700A_FRONTPORCH_X;
    pDCConfig->ui32BlankingX            = LPM027M700A_BLANKING_X;
    pDCConfig->ui32BackPorchX           = LPM027M700A_BACKPORCH_X;

    pDCConfig->ui32FrontPorchY          = LPM027M700A_FRONTPORCH_Y;
    pDCConfig->ui32BlankingY            = LPM027M700A_BLANKING_Y;
    pDCConfig->ui32BackPorchY           = LPM027M700A_BACKPORCH_Y;

    pDCConfig->ui32PixelFormat          = LPM027M700A_INTERFACE_COLOR_CODING;

    pDCConfig->fCLKMaxFreq              = LPM027M700A_PCLK_MAX;
    return 0;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
