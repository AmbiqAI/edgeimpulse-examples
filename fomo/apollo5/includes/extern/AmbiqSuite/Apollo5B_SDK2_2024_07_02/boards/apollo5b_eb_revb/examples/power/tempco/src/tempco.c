//*****************************************************************************
//
//! @file tempco.c
//!
//! @brief A brief demonstration of ADC temperature measurements and temperature
//! compensation.
//!
//! @addtogroup power_examples Power Examples
//!
//! @defgroup tempco Example
//! @ingroup power_examples
//! @{
//!
//! Purpose: This example initializes and invokes the tempco feature.
//!<br>
//!  1) Initialize ADC Temperature Measurement and Timer.<br>
//!  2) Check for temperature, if current temperature is out of threshold,
//!     then call am_hal_pwrctrl_temp_update.<br>
//!  3) Start a timer to age current temperature value. If current temperature is not stale,
//!     do not measure the temperature again.<br>
//!<br>
//! Additional Information:<br>
//! Debug messages will be sent over ITM/SWO at 1M Baud.<br>
//!<br>
//!
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

//*****************************************************************************
//
// MACROs
//
//*****************************************************************************
//
// ADC slot for temperature measurement
//
#define ADC_TEMPERATURE_SLOT              7
//
// Timer instance for temperature value aging
//
#define TEMPERATURE_STALE_TIMER_INST      13
//
// LFRC frequency
//
#define LFRC_FREQ                         900
//
// Interval for temperature monitor.
// This interval should be adjusted to ensure we sample temperature fast enough
// to ensure we can detect 10C temp change between samples.
//
#define MONITOR_INTERVAL                     10
//
// Number of temperature samples for averaging
//
#define TEMP_NUMSAMPLES                   5
//
// Timer ISR
//
#define timer_isr                                                          \
    am_timer_isr1(TEMPERATURE_STALE_TIMER_INST)
#define am_timer_isr1(n)                                                   \
    am_timer_isr(n)
#define am_timer_isr(n)                                                    \
    am_timer ## n ## _isr

//*****************************************************************************
//
// Globals
//
//*****************************************************************************
uint32_t g_ui32TempADCslot;
am_hal_adc_sample_t g_sSamples[TEMP_NUMSAMPLES];
bool g_bTempStale = true;
float g_fTemps = 0.0f;
am_hal_pwrctrl_temp_thresh_t g_sTempThresh = {0.0f, 0.0f};

//*****************************************************************************
//
// Timer globals
//
//*****************************************************************************

//
// Temperature check interval timer configuration.
//
static
am_hal_timer_config_t g_TimerCfg_TemperatureStaleInterval =
{
    .eInputClock        = AM_HAL_TIMER_CLOCK_LFRC,
    .eFunction          = AM_HAL_TIMER_FN_UPCOUNT,
    .bInvertOutput0     = false,
    .bInvertOutput1     = false,
    .eTriggerType       = AM_HAL_TIMER_TRIGGER_DIS,
    .eTriggerSource     = AM_HAL_TIMER_TRIGGER_TMR0_OUT1,
    .bLowJitter         = 0,
    .ui32PatternLimit   = 0,
    .ui32Compare0       = MONITOR_INTERVAL * LFRC_FREQ,    // Set interval. Timer counter resets to 0 on CMP0
    .ui32Compare1       = MONITOR_INTERVAL * LFRC_FREQ + 1
};

//*****************************************************************************
//
// ADC globals
//
//*****************************************************************************

//
// ADC Device Handle.
//
static void *g_ADCHandle;

//
// ADC Configuration
//
static am_hal_adc_config_t g_sADC_Cfg =
{
    .eClock         = AM_HAL_ADC_CLKSEL_HFRC_24MHZ,     // Select the ADC Clock source
    .ePolarity      = AM_HAL_ADC_TRIGPOL_RISING,        // Polarity
    .eTrigger       = AM_HAL_ADC_TRIGSEL_SOFTWARE,      // ADC trigger source
    .eClockMode     = AM_HAL_ADC_CLKMODE_LOW_POWER,     // Clock mode
    .ePowerMode     = AM_HAL_ADC_LPMODE1,               // Power mode for idle state
    .eRepeat        = AM_HAL_ADC_SINGLE_SCAN,
    .eRepeatTrigger = AM_HAL_ADC_RPTTRIGSEL_TMR
};

static am_hal_adc_slot_config_t g_sSlotCfg =
{
    .eMeasToAvg     = AM_HAL_ADC_SLOT_AVG_1,
    .ui32TrkCyc     = 32,
    .ePrecisionMode = AM_HAL_ADC_SLOT_12BIT,
    .eChannel       = AM_HAL_ADC_SLOT_CHSEL_TEMP,
    .bWindowCompare = false,
    .bEnabled       = true
};

//*****************************************************************************
//
// ADC and Timer initialization
//
// The ADC/Timer must be initialized, powered, configured, then it must be
// enabled via a call to am_hal_adc_enable()/am_hal_timer_start().
//
//*****************************************************************************
uint32_t
adc_and_timer_init(void)
{
    uint32_t ui32Retval;
    uint32_t ui32TimerNum = TEMPERATURE_STALE_TIMER_INST;

    //
    // Set the slot to use for temperature
    //
    g_ui32TempADCslot = ADC_TEMPERATURE_SLOT;

    //
    // Initialize the ADC and get the handle.
    //
    if ( am_hal_adc_initialize(0, &g_ADCHandle) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("adc_and_timer_init() Error - reservation of the ADC instance failed.\n");
        return 1;
    }

    //
    // Power on the ADC.
    //
    if (am_hal_adc_power_control(g_ADCHandle, AM_HAL_SYSCTRL_WAKE, false) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("adc_and_timer_init() Error - ADC power on failed.\n");
        return 2;
    }

    //
    // Configure the ADC.
    //
    if ( am_hal_adc_configure(g_ADCHandle, (am_hal_adc_config_t*)&g_sADC_Cfg) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("adc_and_timer_init() Error - configuring ADC failed.\n");
        return 3;
    }

    //
    // Configure and init the timer
    //
    ui32Retval = am_hal_timer_config(ui32TimerNum, (am_hal_timer_config_t*)&g_TimerCfg_TemperatureStaleInterval);
    if ( ui32Retval != AM_HAL_STATUS_SUCCESS )
    {
       return ui32Retval;
    }

    //
    // Enable the timer interrupt
    //
    am_hal_timer_interrupt_enable(TIMER_INTEN_TMR00INT_Msk << (ui32TimerNum * 2));

    NVIC_SetPriority((IRQn_Type)((uint32_t)TIMER0_IRQn + ui32TimerNum), AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ((IRQn_Type)((uint32_t)TIMER0_IRQn + ui32TimerNum));

    //
    // Start the timer
    //
    am_hal_timer_start(ui32TimerNum);

    //
    // At this point the ADC is expected to be initialized, powered, and configured.
    // Configure the temperature slot.
    //
    ui32Retval = am_hal_adc_configure_slot(g_ADCHandle, g_ui32TempADCslot, &g_sSlotCfg);
    if ( ui32Retval != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("ERROR am_hal_adc_configure_slot() returned %d.\n", ui32Retval);
        return 4;
    }

    //
    // Enable the ADC.
    //
    am_hal_adc_enable(g_ADCHandle);

    return 0;

} // adc_and_timer_init()

// ****************************************************************************
//
// Function to trigger the ADC and wait for a value in the FIFO.
//
// ****************************************************************************
static void
adc_trigger_wait(void)
{
    //
    // Trigger and wait for something to show up in the FIFO.
    //
    uint32_t ui32Cnt0;

    ui32Cnt0 = _FLD2VAL(ADC_FIFO_COUNT, ADC->FIFO);
    while ( _FLD2VAL(ADC_FIFO_COUNT, ADC->FIFO) == ui32Cnt0 )
    {
        am_hal_adc_sw_trigger(g_ADCHandle);
        am_hal_delay_us(1);
    }
} // adc_trigger_wait()

// ****************************************************************************
//
// Get samples from temperature sensor.
//
// ****************************************************************************
static uint32_t
adc_temperature_samples_get(uint32_t ui32NumSamples, am_hal_adc_sample_t sSamples[])
{
    uint32_t ux, ui32OneSample;

    ui32OneSample = 1;

    sSamples[0].ui32Sample = sSamples[1].ui32Sample = sSamples[2].ui32Sample = sSamples[3].ui32Sample = sSamples[4].ui32Sample = 0;
    ux = 0;
    while ( ux < ui32NumSamples )
    {
        adc_trigger_wait();

        am_hal_adc_samples_read(g_ADCHandle, true, NULL, &ui32OneSample, &sSamples[ux]);

        if ( sSamples[ux].ui32Slot == g_ui32TempADCslot )
        {
            //
            // This is a temperature sample. Get the next sample.
            //
            ux++;
        }
        else
        {
            return 1;
        }
    }

    return 0;

} // adc_temperature_samples_get()

//*****************************************************************************
//
// Timer ISR
//
//*****************************************************************************
void
timer_isr(void)
{
    //
    // Clear the timer interrupt.
    //
    am_hal_timer_interrupt_clear((TIMER_INTCLR_TMR00INT_Msk << (TEMPERATURE_STALE_TIMER_INST * 2)) | (TIMER_INTCLR_TMR01INT_Msk << (TEMPERATURE_STALE_TIMER_INST * 2)));
    g_bTempStale = true;
    am_hal_sysctrl_sysbus_write_flush();


} // timer_isr()

//*****************************************************************************
//
// Temperature sensing
//
//*****************************************************************************
uint32_t
temp_sensing(void)
{
    float fADCTempVolts = 0.0f;
    float fVT[3] = {0.0f, 0.0f, 0.0f};
    uint16_t ui16TEMP_code = 0;
    uint32_t ux = 0, ui32Retval = 0;
    float fSum = 0.0f;

    if (g_bTempStale)
    {
        //
        // Trigger temperature measurement
        //
        // Power up, configure, and enable the ADC.
        //
        // If the ADC is used for uses in addtion to just temperature measurement,
        // the application would likely provide other steps here such as to check
        // to see if it is already powered up and enabled before going through
        // these steps again.
        // Also in this case, the ADC might not need to be powered down here.
        //
        am_hal_adc_power_control(g_ADCHandle, AM_HAL_SYSCTRL_WAKE, true);

        //
        // Enable the ADC.
        //
        am_hal_adc_enable(g_ADCHandle);

        //
        // Get temperature samples
        //
        adc_temperature_samples_get(TEMP_NUMSAMPLES, g_sSamples);

        //
        // The measured temperature can be considered reliable.
        // Get an average of the temperature.
        //
        while ( ux < TEMP_NUMSAMPLES )
        {
            //
            // Convert and scale the temperature sample into its corresponding voltage.
            //
            ui16TEMP_code = AM_HAL_ADC_FIFO_SAMPLE(g_sSamples[ux].ui32Sample);
            fSum += (float)ui16TEMP_code * AM_HAL_ADC_VREF / 4096.0f; // 12-bit sample
            ux++;
        }

        fADCTempVolts = fSum / ((float)TEMP_NUMSAMPLES);

        //
        // Now call the HAL routine to convert volts to degrees Celsius.
        //
        fVT[0] = fADCTempVolts;
        fVT[1] = 0.0f;
        fVT[2] = -123.456;
        if ( am_hal_adc_control(g_ADCHandle, AM_HAL_ADC_REQ_TEMP_CELSIUS_GET, fVT) == AM_HAL_STATUS_SUCCESS )
        {
            g_fTemps = fVT[1];  // Get the temperature
        }
        else
        {
            return 1;
        }

        //
        // Disable and power down the ADC.
        //
        am_hal_adc_power_control(g_ADCHandle, AM_HAL_SYSCTRL_DEEPSLEEP, true);
        if ((g_fTemps >= g_sTempThresh.fHighThresh) || (g_fTemps < g_sTempThresh.fLowThresh))
        {
            ui32Retval = am_hal_pwrctrl_temp_update(g_fTemps, &g_sTempThresh);
            if ( ui32Retval != AM_HAL_STATUS_SUCCESS )
            {
                return 2;
            }
        }

        g_bTempStale = false;
    }

    return 0;
}

//*****************************************************************************
//
// Main
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32Ret;

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Initialize the printf interface for ITM output
    //
    if (am_bsp_debug_printf_enable())
    {
        // Cannot print - so no point proceeding
        while(1);
    }

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();

    am_util_stdio_printf("tempco example!\n\n");

    am_util_stdio_printf("Begin tempco initialization.\n");
    am_util_stdio_printf("App Compiler:    %s\n", COMPILER_VERSION);

    //
    // Turn off OTP for deepsleep
    //
    ui32Ret = am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_OTP);
    if ( ui32Ret )
    {
        am_util_stdio_printf("Failed to power down OTP.\n");
    }

    //
    // Initialize for ADC temps measurement and timer
    //
    ui32Ret = adc_and_timer_init();
    if ( ui32Ret )
    {
        am_util_stdio_printf("adc_and_timer_init failed, exiting the example.\n");
        while (1);
    }

    //
    // Disable and power down the ADC before deepsleep.
    //
    am_hal_adc_power_control(g_ADCHandle, AM_HAL_SYSCTRL_DEEPSLEEP, true);

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    am_util_stdio_printf("tempco setup completed.\n");
    am_util_stdio_printf("\n");
    am_bsp_debug_printf_disable();

    while (1)
    {
        ui32Ret = temp_sensing();
        //
        // Go to deepsleep.
        //
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
        am_bsp_debug_printf_enable();

        if ( ui32Ret )
        {
            am_util_stdio_printf("Temp sensing failed!\n");
        }
        else
        {
            am_util_stdio_printf("Current temperature is %.02f C \n", g_fTemps);
        }

        am_bsp_debug_printf_disable();
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

