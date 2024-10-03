//*****************************************************************************
//
//! @file crypto_stress_test_config.h
//!
//! @brief Short description
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef CRYPTO_STRESS_TEST_CONFIG_H
#define CRYPTO_STRESS_TEST_CONFIG_H

//******************************************************************************
//
// General Options for the stress test.
//
//******************************************************************************
#define NUM_TRIALS              100        // Number of crypto power cycles before the test fails
#define MAX_POI_COUNT           10         // Number of crypto power cycles before the test fails
#define PAUSE_GPIO              1          // GPIO to use for pause function.
#define NO_INIT_COUNTER         0x400201C4 // Used to track how many times the test has done a POI. Resets on powercycle.

//******************************************************************************
//
// Crypto operation config.
//
// These are things that the main program can do during the call to
// crypto_test_operation().
//
//******************************************************************************
// 0 == No crypto power up/down
// 1 == Basic - just use PWRCTL registers
// 2 == Use HAL
// #### INTERNAL BEGIN ####
// 3 - Brendan's sequence
// 4 - Brendan's sequence # 2
// #### INTERNAL END ####
#define ENABLE_CRYPTO_POWER     2           // Enable crypto power (req. for all below)
#define ENABLE_OTP_READS        1           // Read from OTP
#define OTP_READ_CYCLES         (25)
#define ENABLE_OTP_WRITES       1           // Run signature verification algorithm.
#define OTP_WRITE_CYCLES        (25)
#define ENABLE_CRYPTO_RUNTIME   1           // Initialize the crypto runtime (req. for all below)
#define ENABLE_RSA_SHA          1           // Run signature verification algorithm.
#define ENABLE_CACHE            0           // Enable Cache
#define ENABLE_LOWPOWER_INIT    1
#define ENABLE_POI              1
//******************************************************************************
//
// Options for the timer-driven task.
//
//******************************************************************************
#define ENABLE_TIMER            1          // Enable timer interrupts.
#ifdef AM_PART_APOLLO4P
#define ENABLE_CRC              1          // Run a CRC in the background using timer interrupts.
#else
#define ENABLE_CRC              0          // Run a CRC in the background using timer interrupts.
#endif
#define TIMER_TASK_DELAY        40        // How often to run the secondary task (in us)
#define ENABLE_INFO_ACCESS      1          // Perform Periodic INFO* access

//******************************************************************************
//
// Debug functions.
//
//******************************************************************************
#define ENABLE_WATCHDOG         0
#define HARDFAULT_GPIO          2          // Will need to change hardfault handler if you change the GPIO number.
#define DEFAULT_ISR_GPIO        3
#define LOOP_GPIO               4
#define TIMER_GPIO              27
#define ENABLE_STIMER           1
// 0 == No sleep
// 1 == Normal Sleep
// 2 == Deep Sleep
// 3 == Alternate between Notmal and Deep
#define ENABLE_SLEEP            3
#define ENABLE_PRINT_VERBOSE    0

//******************************************************************************
//
// Helpful debugging macros.
//
//******************************************************************************

// Configure GPIO 32-63 for Status output
#define CONFIG_STATUS_GPIO()                                   \
    do                                                         \
    {                                                          \
        volatile uint32_t *pui32Config = &GPIO->PINCFG64;      \
        GPIO->PADKEY = GPIO_PADKEY_PADKEY_Key;                 \
                                                               \
        for (uint32_t i = 0; i < 5; i++)                       \
        {                                                      \
            pui32Config[i] = am_hal_gpio_pincfg_output.GP.cfg; \
        }                                                      \
                                                               \
        GPIO->PADKEY = 0;                                      \
        GPIO->WT3 = 0;                                         \
                                                               \
    } while(0)
// Set the Status GPIOs
#define SET_STATUS_GPIO(status)                                \
    do                                                         \
    {                                                          \
        GPIO->WT2 = (status);                                  \
    } while(0)

#if ENABLE_PRINT_VERBOSE
#define PRINT_VERBOSE       am_util_stdio_printf
#else
#define PRINT_VERBOSE(...)
#endif
#define PRINT              am_util_stdio_printf

#endif // CRYPTO_STRESS_TEST_CONFIG_H
