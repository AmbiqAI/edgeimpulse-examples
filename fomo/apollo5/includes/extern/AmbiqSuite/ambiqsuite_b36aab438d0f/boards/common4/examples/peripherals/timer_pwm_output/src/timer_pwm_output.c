//*****************************************************************************
//
//! @file timer_pwm_output.c
//!
//! @brief TIMER PWM example.
//!
//! @addtogroup peripheral_examples Peripheral Examples
//
//! @defgroup timer_pwm_output TIMER PWM Example
//! @ingroup peripheral_examples
//! @{
//!
//! Purpose: This example shows one way to vary the brightness of an LED using an additional
//! timer in PWM mode.
//! The timer can be clocked from either the HFRC (default) or the LFRC.
//! The led brightness varies over time, make the LEDs appear to breath.
//!
//! Two timers are used:
//! AM_BSP_PWM_LED_TIMER is the used for the high frequency PWM. This controls
//! pins attached to the LEDs.
//! LED_CYCLE_TIMER is used for the lower frequency pwm duty cycle updates
//!
//!
//! @note This example shows a low power operation using reduced DTCM size
//! The 8K size is very near the limit with most BSPs.
//! The standard stack size had to be slightly  reduced for this example's
//! RAM use to fit in 8K.(with IAR compiler)
//!
//! Printing takes place over the ITM at 1M Baud.
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

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "low_power_config.h"

#ifndef AM_BSP_PWM_LED_TIMER
#define AM_BSP_PWM_LED_TIMER 0
#endif

#ifndef AM_BSP_GPIO_LED0
#define AM_BSP_GPIO_LED0 0x10000
#endif
#ifndef AM_BSP_GPIO_LED1
#define AM_BSP_GPIO_LED1 0x10000
#endif

//*****************************************************************************
//
// Macros
//
//*****************************************************************************
#define PWM_CLK     AM_HAL_TIMER_CLOCK_HFRC_DIV16
#define PWM_FREQ    200
#define FREQ_FRAC_SCALING 128
#define LFRC_TCIKS_SEC 900

//
//! using this timer for the lower freq duty cycle update timer
//
#define LED_CYCLE_TIMER 2

#define TABLE_ENTRIES  64
#define TABLE_SCALE 256  ///< max value any table element can have

//*****************************************************************************
//
//! @brief LED brightness profile
//!
//! @details Created by simple Python script for generating the sine-wave values:
//! compute sine wave table with 64 entries and max value of 256.
//! @code
//! import math
//! iperiod = 64
//! iscale  = 256
//! fScale  = float(iscale)
//! halfpi = math.pi / 2.0
//! period = float(iperiod)
//! ampl = period
//! period = period / 2.0
//! aconst = halfpi / period
//!
//! for ix in range(0, iperiod):
//!     a = float(ix) * aconst
//!     vv = math.sin(a) * fScale
//!     vr = round(vv -1)
//!     vi = int(vr)
//!     if vi < 0: vi = 0
//!     if vi > (iscale-1): vi = iscale -1
//!     print(str(ix) + ": " + str(vi))
//
//*****************************************************************************
static const uint8_t g_pui8Brightness[TABLE_ENTRIES] =
{
    0,  12, 24, 37, 49, 61, 73, 85,
    97, 108, 120, 131, 141, 151, 161, 171,
    180, 189, 197, 205, 212, 219, 225, 230,
    236, 240, 244, 247, 250, 252, 254, 255,
    255, 255, 254, 252, 250, 247, 244, 240,
    236, 230, 225, 219, 212, 205, 197, 189,
    180, 171, 161, 151, 141, 131, 120, 108,
    97, 85, 73, 61, 49, 37, 24, 12
};

am_hal_gpio_pincfg_t g_PWMOutPinCfg =
{
    .GP.cfg_b.uFuncSel             = 6, //CT
    .GP.cfg_b.eGPInput             = AM_HAL_GPIO_PIN_INPUT_NONE,
    .GP.cfg_b.eGPRdZero            = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir              = AM_HAL_GPIO_PIN_INTDIR_NONE,
    .GP.cfg_b.eGPOutCfg            = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
    .GP.cfg_b.eDriveStrength       = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
    .GP.cfg_b.uSlewRate            = 0,
    .GP.cfg_b.ePullup              = AM_HAL_GPIO_PIN_PULLUP_NONE,
    .GP.cfg_b.uNCE                 = 0,
    .GP.cfg_b.eCEpol               = AM_HAL_GPIO_PIN_CEPOL_ACTIVELOW,
    .GP.cfg_b.uRsvd_0              = 0,
    .GP.cfg_b.ePowerSw             = AM_HAL_GPIO_PIN_POWERSW_NONE,
    .GP.cfg_b.eForceInputEn        = AM_HAL_GPIO_PIN_FORCEEN_NONE,
    .GP.cfg_b.eForceOutputEn       = AM_HAL_GPIO_PIN_FORCEEN_NONE,
    .GP.cfg_b.uRsvd_1              = 0,
};

//
//! define global variables used in this module
//
typedef struct
{
    //
    //! scaled version of sine wave lookup table
    //! scaled so value can simply be dropped into timer1 compare register
    //
    uint32_t pui8BrightnessAdjusted[TABLE_ENTRIES];

    //
    //! This is the compare0 value,
    //! when the counter hits this value and rests
    //! (the value in compare1), that would 100% duty cycle
    //
    uint64_t ui64EndCounts;

    //
    //! index into sine wave lookup talble
    //
    uint32_t ui32Index;
    am_hal_timer_config_t tPwmTimerConfig;
    am_hal_timer_config_t tInterruptTimerConfig;

    //
    //! set in timer isr, advances duty cycle table
    //
    volatile bool bTimerIntOccured;

}
uart_async_local_vars_t;

//*****************************************************************************
//! Globals used in this module
//*****************************************************************************

static uart_async_local_vars_t g_localv;

//*****************************************************************************
// static function prototypes
//*****************************************************************************

static void compute_brightness_adjustment(
    uint32_t ui32TimerMax,
    const uint8_t *pui8RawTable,
    uint32_t ui32RawTableScaleMax,
    uint32_t *pui32outTable,
    uint32_t ui32TableSize);

static uint32_t init_timer(uint32_t ui32TimerNumber);

static void tmr_pwm_update_dc(void);

static uint32_t pwm_example_setup_pwm(uint32_t ui32PwmFreq_x128);

//*****************************************************************************
//
//! @brief Timer Interrupt Service Routine (ISR) for timer02
//!
//! @note the name of this isr changes depending on which timer is used
//! there are examples of where the name is computed at compile time
//! with preprocessor commands.
//!
//! @details This timer is used to control the PWM duty cycle switching:
//! that is the rate the brightness changes.
//!
//! @note This should run much slower than the PWM frequency.
//
//*****************************************************************************
void
am_timer02_isr(void)
{
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(LED_CYCLE_TIMER, AM_HAL_TIMER_COMPARE1));
    am_hal_timer_clear(LED_CYCLE_TIMER);
    //
    // set flag so background loop can take action
    //
    g_localv.bTimerIntOccured = true;
}

//*****************************************************************************
//
//! @brief  Rescale a precomputed table (sine wave here) and save in ram
//!
//! @note Rescale the table so the computation is completed at startup and doesn't
//! have to be done on every PWM output change.
//! The table is rescaled to reflect the max value of the PWM output
//!
//! @param ui32TimerMax  max PWM timer value (at 100% duty cycle)
//! @param pui8RawTable    pointer to precomputed input table
//! @param ui32RawTableScaleMax max scaling in input table
//! @param pui32outTable    pointer to output table
//! @param ui32TableSize   number of data points in input table
//!
//! @note Letting the value go to zero shuts off the output and disables both output pins
//! (even the compliment). This example does not allow PWM to completely shut off
//
//*****************************************************************************
static void
compute_brightness_adjustment(
    uint32_t ui32TimerMax,
    const uint8_t *pui8RawTable,
    uint32_t ui32RawTableScaleMax,
    uint32_t *pui32outTable,
    uint32_t ui32TableSize)
{
    for (uint32_t i = 0; i < ui32TableSize; i++)
    {
        uint32_t ui32Itval = (uint32_t) (*pui8RawTable++);
        ui32Itval *= ui32TimerMax;
        ui32Itval /= ui32RawTableScaleMax;
        ui32Itval = (ui32Itval >= ui32TimerMax) ? ui32TimerMax - 1 : ui32Itval;
        //
        //! keep pwm from completely turning off(see note in fcn header)
        //
        ui32Itval = ui32Itval == 0 ? 1 : ui32Itval;
        *pui32outTable++ = ui32Itval;
    }
}

//*****************************************************************************
//
//! @brief this will pull the next value from the adjusted pwm table and load the
//! value into compare1 to change the PWM duty cycle
//
//*****************************************************************************
static void
tmr_pwm_update_dc(void)
{
    if (++g_localv.ui32Index >= TABLE_ENTRIES)
    {
        g_localv.ui32Index = 0;
    }

    uint32_t ui32NewComp1Val = g_localv.pui8BrightnessAdjusted[g_localv.ui32Index];
    am_hal_timer_compare1_set(AM_BSP_PWM_LED_TIMER, ui32NewComp1Val);

    //am_util_stdio_printf("b ness: %d\r\n", ui32NewComp1Val);

} // tmr_pwm_update_dc()

//*****************************************************************************
//
//! @brief Start Update Timer
//! This timer is used to update the duty cycle
//!
//! @param ui32TimerNumber the pwm freq * 128, the fractional scaling
//!
//! @return standard hal status
//
//*****************************************************************************
static uint32_t
init_timer(uint32_t ui32TimerNumber)
{
    am_hal_timer_config_t tTimerConfig;

    am_hal_timer_default_config_set(&tTimerConfig);
    tTimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_LFRC;     // ~900Hz

    //
    // Modify the default parameters.
    // Configure the timer to a suitable rate
    // dividy by 4 will scroll throught the sinewave table at 4 hz
    // with 64 entries it will run though the table in 16 seconds
    //
    tTimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    tTimerConfig.ui32Compare1 = LFRC_TCIKS_SEC / 4;

    //
    // Configure the timer
    //
    uint32_t ui32Status = am_hal_timer_config(ui32TimerNumber, &tTimerConfig);
    if (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("Failed to configure TIMER%d.\n", ui32TimerNumber);
        return ui32Status;
    }

    //
    // Clear the timer and its interrupt
    //
    am_hal_timer_clear(ui32TimerNumber);
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(ui32TimerNumber, AM_HAL_TIMER_COMPARE1));
    //
    // Clear the timer Interrupt
    //
    NVIC_SetPriority((IRQn_Type) (ui32TimerNumber + TIMER0_IRQn), AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ((IRQn_Type)(ui32TimerNumber + TIMER0_IRQn));

    //
    // Enable the timer Interrupt.
    //
    am_hal_timer_interrupt_enable(AM_HAL_TIMER_MASK(ui32TimerNumber, AM_HAL_TIMER_COMPARE1));

    am_hal_timer_enable(ui32TimerNumber);

    return AM_HAL_STATUS_SUCCESS;
} // init_timer()

//*****************************************************************************
//
//! @brief set up the PWM timer
//!
//! @param ui32PwmFreq_x128 the pwm freq * 128, the fractional scaling
//!
//! @return
//
//*****************************************************************************
static uint32_t
pwm_example_setup_pwm(uint32_t ui32PwmFreq_x128)
{
    //
    // The default config parameters include:
    //  eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16
    //  eFunction = AM_HAL_TIMER_FN_EDGE
    //  Compare0 and Compare1 maxed at 0xFFFFFFFF
    //

    am_hal_timer_default_config_set(&g_localv.tPwmTimerConfig);

    g_localv.tPwmTimerConfig.eFunction = AM_HAL_TIMER_FN_PWM;
    g_localv.tPwmTimerConfig.eInputClock = PWM_CLK;

    //
    // Configure the PWM timer .
    //
    am_hal_timer_config(AM_BSP_PWM_LED_TIMER, &g_localv.tPwmTimerConfig);

    // Config output pins
    // the second output pin is the compliment of the first
    if (AM_BSP_GPIO_LED0 < 0xFFFF)
    {
        am_hal_timer_output_config(AM_BSP_GPIO_LED0, AM_HAL_TIMER_OUTPUT_TMR0_OUT0 + AM_BSP_PWM_LED_TIMER * 2);
        am_hal_gpio_pinconfig(AM_BSP_GPIO_LED0, g_PWMOutPinCfg);
    }
    if (AM_BSP_GPIO_LED1 < 0xFFFF)
    {
        am_hal_timer_output_config(AM_BSP_GPIO_LED1, AM_HAL_TIMER_OUTPUT_TMR0_OUT1 + AM_BSP_PWM_LED_TIMER * 2);
        am_hal_gpio_pinconfig(AM_BSP_GPIO_LED1, g_PWMOutPinCfg);
    }

    //
    // compute timer counter reset value in PWM mode
    //
    if (PWM_CLK > AM_HAL_TIMER_CLOCK_HFRC_DIV4K)
    {
        am_util_stdio_printf("Timer PWM Frequency too fast\r\n");
        return AM_HAL_STATUS_INVALID_OPERATION;
    }

// #### INTERNAL BEGIN ####
#if APOLLO4_FPGA
    ui64Div = 16 << (2 * PWM_CLK);
#else
// #### INTERNAL END ####
    uint64_t ui64Div = 4 << (2 * PWM_CLK);
// #### INTERNAL BEGIN ####
#endif
// #### INTERNAL END ####
    //
    // note: often more precision if multiply happens before divide
    //
    uint64_t ui64BaseFreq_x128 = (96000000ull / ui64Div) * FREQ_FRAC_SCALING;

    //
    // in the divide below the extra scaling factor will divide out
    // round and compute the compare 0 value
    //
    g_localv.ui64EndCounts = ((ui64BaseFreq_x128 + (ui32PwmFreq_x128 / 2)) / ui32PwmFreq_x128);
    g_localv.tPwmTimerConfig.ui32Compare0 = (uint32_t) g_localv.ui64EndCounts;

    am_hal_timer_compare0_set(AM_BSP_PWM_LED_TIMER, g_localv.tPwmTimerConfig.ui32Compare0);

    //
    // update/compute the sinewave table based on the new compare0 value
    // this way, the table entry simply needs to be saved into the timer1 register
    //
    compute_brightness_adjustment(g_localv.ui64EndCounts,
                                g_pui8Brightness,
                                TABLE_SCALE,
                                g_localv.pui8BrightnessAdjusted,
                                sizeof(g_pui8Brightness));

    //
    // Enable the TIMER.
    //
    am_hal_timer_enable(AM_BSP_PWM_LED_TIMER);

    return AM_HAL_STATUS_SUCCESS;

} // pwm_example_setup_pwm

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32CfgStatus;

    //
    // Set the default cache configuration
    //
    ui32CfgStatus = std_cache_setup();

    //
    // Configure the board for low-power operation.
    //
    am_bsp_low_power_init();

#if (AM_BSP_NUM_LEDS > 0)
    //
    // Configure the pins for this board.
    //
    am_devices_led_array_init(am_bsp_psLEDs, AM_BSP_NUM_LEDS);
#endif

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Timer PWM Output example\n");
#if AM_BSP_NUM_LEDS > 0
    am_util_stdio_printf("Timer will output PWM waves to the LEDs on the EVB.\n");
#else
    am_util_stdio_printf("Timer will output PWM waves to pin%d and pin%d\n\n", AM_BSP_GPIO_LED0, AM_BSP_GPIO_LED1);
#endif
    if ( ui32CfgStatus )
    {
        am_util_stdio_printf("Error: Cache config error: this function will hang\n");
        while(true);
    }
    ui32CfgStatus = deepsleep_config();
    if ( ui32CfgStatus )
    {
        am_util_stdio_printf("Error: Deepsleep config error: this function will hang\n");
        while(true);
    }
    ui32CfgStatus = lowpower_memory_config();
    if ( ui32CfgStatus )
    {
        am_util_stdio_printf("Error: Low Power Config Error: this function will hang\n");
        while(true);
    }

    //
    // allow time for prints to complete
    //
    am_util_delay_ms(10);

    //
    // We are done printing. Disable debug printf messages on ITM.
    //
    am_bsp_debug_printf_disable();

    uint32_t ui32PwmFreq_x128 = PWM_FREQ * FREQ_FRAC_SCALING;

    // set up the pwm
    pwm_example_setup_pwm(ui32PwmFreq_x128);

    //
    // Init the interrupt timer
    //
    init_timer(LED_CYCLE_TIMER);

    //
    // enable global interrupts
    //
    am_hal_interrupt_master_enable();

    //
    // Start the timers.
    //
    am_hal_timer_start(AM_BSP_PWM_LED_TIMER);
    am_hal_timer_start(LED_CYCLE_TIMER);


    //
    // Sleep forever while waiting for an interrupt.
    //
    while (1)
    {
        //
        // Go to Deep Sleep.
        //
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
        if (g_localv.bTimerIntOccured)
        {
            g_localv.bTimerIntOccured = false;
            tmr_pwm_update_dc();
        }
    }
} // main()

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

