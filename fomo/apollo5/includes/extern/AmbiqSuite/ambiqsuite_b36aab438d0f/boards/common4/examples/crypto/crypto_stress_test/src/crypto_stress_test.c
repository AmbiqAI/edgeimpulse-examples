//*****************************************************************************
//
//! @file crypto_stress_test.c
//!
//! @brief A test to run several crypto operations to stress the system
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

//******************************************************************************
//
// Additional tasks
//
//******************************************************************************
#include "crc_task.h"
#include "crypto_operations.h"

//******************************************************************************
//
// Test configuration
//
//******************************************************************************
#include "crypto_stress_test_config.h"

//******************************************************************************
//
// Pause the program if our configured GPIO is set.
//
//******************************************************************************
void
gpio_pause(void)
{
    am_hal_gpio_pincfg_t sInputConfig = AM_HAL_GPIO_PINCFG_INPUT;
    sInputConfig.GP.cfg_b.ePullup = AM_HAL_GPIO_PIN_PULLDOWN_50K;
    am_hal_gpio_pinconfig(PAUSE_GPIO, sInputConfig);

    uint32_t ui32PausePin = 0;
    am_hal_gpio_state_read(PAUSE_GPIO, AM_HAL_GPIO_INPUT_READ, &ui32PausePin);

    if (ui32PausePin)
    {
        PRINT("Pausing (GPIO high).\n");
        while (1);
    }
}

//******************************************************************************
//
// POI after a specified number of resets.
//
//******************************************************************************
void
execute_poi(void)
{
#if ENABLE_POI
    //
    // Reset the chip using a POI, but stop every 100th POI.
    //
    if ((AM_REGVAL(NO_INIT_COUNTER) % MAX_POI_COUNT) == 0)
    {
        PRINT("Test Complete\n");
        while (1)
        {
            am_hal_gpio_state_write(LOOP_GPIO, AM_HAL_GPIO_OUTPUT_TOGGLE);
        }
    }
    else
    {
        PRINT("Resetting (POI)\n");
        am_util_delay_ms(5);
        AM_REGVAL(0x40000004) = 0x1B;
    }
#endif
}

//******************************************************************************
//
// Watchdog ISR
//
// This is to help us catch CPU hangs.
//
//******************************************************************************
void
am_watchdog_isr(void)
{
    uint32_t ui32Status;

    //
    // Read and clear the interrupt status.
    //
    am_hal_wdt_interrupt_status_get(AM_HAL_WDT_MCU, &ui32Status, true);
    am_hal_wdt_interrupt_clear(AM_HAL_WDT_MCU, ui32Status);

    SET_STATUS_GPIO(15);

    while (1);

} // am_watchdog_isr()

//******************************************************************************
//
// Watchdog setup
//
//******************************************************************************
void
watchdog_setup(void)
{
#if ENABLE_WATCHDOG
    //
    // Configure the watchdog to interrupt us if we don't see any activity for
    // one second.
    //
    am_hal_wdt_config_t sWatchdogConfig =
    {
        // Set the clock for 16HZ
        .eClockSource = AM_HAL_WDT_16HZ,
        .bInterruptEnable = true,
        .ui32InterruptValue = 16,
        .bResetEnable = true,
        .ui32ResetValue = 32,
        .bAlertOnDSPReset = false,
    };

    //
    // Enable the watchdog interrupt.
    //
    am_hal_wdt_config(AM_HAL_WDT_MCU, &sWatchdogConfig);
    am_hal_wdt_interrupt_clear(AM_HAL_WDT_MCU, AM_HAL_WDT_INTERRUPT_MCU);
    am_hal_wdt_interrupt_enable(AM_HAL_WDT_MCU, AM_HAL_WDT_INTERRUPT_MCU);
    NVIC_SetPriority(WDT_IRQn, 1);
    NVIC_EnableIRQ(WDT_IRQn);

    //
    // Start the watchdog.
    //
    am_hal_wdt_start(AM_HAL_WDT_MCU, false);
#endif
}

//******************************************************************************
//
// Pet the watchdog.
//
//******************************************************************************
void
watchdog_pet(void)
{
#if ENABLE_WATCHDOG
    am_hal_wdt_restart(AM_HAL_WDT_MCU);
#endif
}

//******************************************************************************
//
// TIMER ISR to catch spurious TIM13 interrupts.
//
// While TIMER13 is used in the powercontrol code for crypto, we don't expect
// the interrupt to actually get called in a normal working environment. If the
// CPU somehow ends up in this interrupt handler anyway, it should be flagged
// as a problem.
//
//******************************************************************************
void
am_timer13_isr(void)
{
    while (1)
    {
        AM_REGVAL(0x40010224) = 0x8;
        AM_REGVAL(0x40010234) = 0x8;
    }
}

//******************************************************************************
//
// Reset status checking.
//
//******************************************************************************
void
check_reset_status(void)
{
    am_hal_reset_status_t sStatus;

    am_hal_reset_status_get(&sStatus);

    // If we hit the watchdog reset, stop immediately.
    while (sStatus.bWDTStat);
}

//******************************************************************************
//
// STIMER initialization
//
//******************************************************************************
void
stimer_init(void)
{
#if ENABLE_STIMER
    //
    // Enable compare A interrupt in STIMER
    //
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREA);
    am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREA);

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_SetPriority(STIMER_CMPR0_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(STIMER_CMPR0_IRQn);

    //
    // Configure the STIMER and run
    //
    am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
    am_hal_stimer_compare_delta_set(0, 375);
    am_hal_stimer_config(AM_HAL_STIMER_HFRC_375KHZ |
                         AM_HAL_STIMER_CFG_COMPARE_A_ENABLE);

    NVIC_SetPriority(STIMER_CMPR0_IRQn, AM_IRQ_PRIORITY_DEFAULT);
#endif // ENABLE_STIMER
}

//******************************************************************************
//
// ISR for STIMER CMPR0
//
//******************************************************************************
void am_stimer_cmpr0_isr(void)
{
#if ENABLE_STIMER
    am_hal_gpio_state_write(TIMER_GPIO, AM_HAL_GPIO_OUTPUT_SET);
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREA);
    am_hal_stimer_compare_delta_set(0, 300);
    am_hal_gpio_state_write(TIMER_GPIO, AM_HAL_GPIO_OUTPUT_CLEAR);
#endif // ENABLE_STIMER
}

//*****************************************************************************
//
// Main
//
//*****************************************************************************
int
main(void)
{
    gpio_pause();

    CONFIG_STATUS_GPIO();

    am_hal_gpio_pinconfig(HARDFAULT_GPIO, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEFAULT_ISR_GPIO, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(LOOP_GPIO, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(TIMER_GPIO, am_hal_gpio_pincfg_output);

    am_hal_gpio_state_write(HARDFAULT_GPIO, AM_HAL_GPIO_OUTPUT_SET);
    am_hal_gpio_state_write(HARDFAULT_GPIO, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(DEFAULT_ISR_GPIO, AM_HAL_GPIO_OUTPUT_SET);
    am_hal_gpio_state_write(DEFAULT_ISR_GPIO, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(LOOP_GPIO, AM_HAL_GPIO_OUTPUT_SET);
    am_hal_gpio_state_write(LOOP_GPIO, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(TIMER_GPIO, AM_HAL_GPIO_OUTPUT_SET);
    am_hal_gpio_state_write(TIMER_GPIO, AM_HAL_GPIO_OUTPUT_CLEAR);

    for (uint32_t i = 0; i < 16; i++)
    {
        SET_STATUS_GPIO(i);
    }

    SET_STATUS_GPIO(0);

    //
    // The "NO_INIT_COUNTER" is pointing to scratch register 1. This will be
    // set to zero automatically on a full power-cycle, but it will keep its
    // value through POI. This will help us keep track of how many times we've
    // completed a full cycle of this test.
    //
    AM_REGVAL(NO_INIT_COUNTER) += 1;

    SET_STATUS_GPIO(1);

#if ENABLE_LOWPOWER_INIT
    //
    // Standard application initial setup.
    //
    am_bsp_low_power_init();

    //
    // Power down Crypto.
    //
#if defined (AM_PART_APOLLO4B)
    am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_CRYPTO_POWERDOWN, 0);
#else
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);
#endif
#endif

    //
    // Enable printing
    //
    am_bsp_debug_printf_enable();
    am_util_stdio_terminal_clear();
    PRINT("Crypto stress test POI# %d\n", (AM_REGVAL(NO_INIT_COUNTER) % MAX_POI_COUNT));
    PRINT_VERBOSE("Running from 0x%x\n\n", AM_REGVAL(0xE000ED08));

    SET_STATUS_GPIO(2);

    check_reset_status();

#if ENABLE_CACHE
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();
#endif

    SET_STATUS_GPIO(3);

    //
    // Initialize periodic tasks
    //
    stimer_init();
    crc_task_init();
    SET_STATUS_GPIO(4);
    crypto_test_init();
    SET_STATUS_GPIO(5);

    crc_task_enable();
    SET_STATUS_GPIO(6);

    //
    // Enable the watchdog before running the main loop
    //
    watchdog_setup();

    am_hal_interrupt_master_enable();

    //AM_REGVAL(0xFFFFFFFF);

    //while (1);
    //
    // Run the specified number of trials.
    //
    for (uint32_t i = 0; i < NUM_TRIALS; i++)
    {
        PRINT_VERBOSE("**** Trial %d ****\n", i);
        am_hal_gpio_state_write(LOOP_GPIO, AM_HAL_GPIO_OUTPUT_TOGGLE);
        crypto_test_operation();

#if (ENABLE_SLEEP == 1)
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_NORMAL);
#elif (ENABLE_SLEEP == 2)
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
#elif (ENABLE_SLEEP == 3)
        am_hal_sysctrl_sleep(i&0x1? AM_HAL_SYSCTRL_SLEEP_NORMAL: AM_HAL_SYSCTRL_SLEEP_DEEP);
#endif // ENABLE_SLEEP

        watchdog_pet();
    }

    //
    // Perform task cleanup.
    //
    crc_task_disable();

    //
    // Execute a POI, but pause if there's a GPIO, or we've reached our quota
    // of POIs.
    //
    execute_poi();

    while (1);
}
