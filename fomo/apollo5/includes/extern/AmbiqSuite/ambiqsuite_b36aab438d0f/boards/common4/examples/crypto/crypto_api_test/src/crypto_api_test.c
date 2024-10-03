//*****************************************************************************
//
//! @file crypto_api_test.c
//!
//! @brief A stress test for the hardware crypto accelleration in Apollo4.
//!
//! This example prints a banner message over SWO at 1M baud. To see the
//! output of this program, use J-Link SWO Viewer (or similar viewer appl)
//! and configure SWOClock for 1000.
//! The test runs through a fixed number of cycles of powering crypto on and off,
//! and optionally performing RSA encryption/decryption. The purpose is to ensure
//! that the crypto hardware is functioning normally under heavy load.
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
// Configuration
//
//******************************************************************************
#define NUM_TRIALS             100        // Number of crypto power cycles before the test resets
#define MAX_POI_COUNT          100        // Number of MCU POI operations before the test stops itself.
#define PAUSE_GPIO             1          // GPIO to use for pause function. (helpful for connecting debuggers)
#define SECONDARY_TASK_DELAY   2          // How often to run the secondary task (in us)
#define NO_INIT_COUNTER        0x400201C4 // Used to track how many times the test has done a POI. Resets on powercycle.
                                          // 0x400201C4 is SCRATCH1.

#define RUN_CRYPTO_OPERATIONS  1          // 1 - Run SHA and RSA operation. 0 - Power cycle only.

//******************************************************************************
//
// Runtime crypto includes
//
//******************************************************************************
#include "config-cc312-apollo4-no-os.h"

#include "cc_lib.h"

#include "mbedtls/rsa.h"
#include "mbedtls/sha256.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/memory_buffer_alloc.h"
#include "mbedtls_cc_mng.h"
#include "crypto_test_data.h"
#include "rsa_keys.h"

//******************************************************************************
//
// Runtime crypto globals.
//
//******************************************************************************

//
// Dynamic memory for mbedTLS stack.
//
#define AM_MBEDTLS_HEAP_SIZE (128*1024/4)
uint32_t g_ui32MbedTLSHeap[AM_MBEDTLS_HEAP_SIZE];

//
// Context variables for mbedTLS operations.
//
mbedtls_rsa_context rsa;
CCRndContext_t RndContex;
CCRndWorkBuff_t RndWorkBuff;
mbedtls_ctr_drbg_context RndState;
mbedtls_entropy_context MbedtlsEntropy;

//*****************************************************************************
//
// Globals.
//
//*****************************************************************************
uint32_t g_ui32TimerCount = 0;

//******************************************************************************
//
// Prepare a secondary task for the core to do periodically.
//
//******************************************************************************
void
secondary_task_init(void)
{
    am_hal_timer_config_t sTimerConfig;

    am_hal_timer_default_config_set(&sTimerConfig);
    sTimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    sTimerConfig.ui32PatternLimit = 0;
    sTimerConfig.ui32Compare1 = (6 * SECONDARY_TASK_DELAY);  // Default clock is 6MHz

    if (AM_HAL_STATUS_SUCCESS != am_hal_timer_config(0, &sTimerConfig))
    {
        am_util_stdio_printf("Failed to configure TIMER0\n");
    }

    am_hal_timer_clear(0);
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE1));
    NVIC_SetPriority(TIMER_IRQn, 1);

    g_ui32TimerCount = 0;
}

//******************************************************************************
//
// Enable the secondary task
//
//******************************************************************************
void
secondary_task_enable(void)
{
    am_hal_timer_interrupt_enable(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE1));
    NVIC_SetPriority(TIMER_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(TIMER_IRQn);
    am_hal_timer_start(0);
}

//******************************************************************************
//
// Disable the secondary task
//
//******************************************************************************
void
secondary_task_disable(void)
{
    am_hal_timer_stop(0);
    NVIC_DisableIRQ(TIMER_IRQn);
    am_hal_timer_interrupt_disable(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE1));
}

//******************************************************************************
//
// Final tasks for the secondary task before reset.
//
//******************************************************************************
void
secondary_task_complete(void)
{
    secondary_task_disable();
    am_util_stdio_printf("Secondary task count: %d\n", g_ui32TimerCount);
}

//******************************************************************************
//
// Pause if the selected GPIO is pulled high.
//
//******************************************************************************
void
gpio_pause(void)
{
    am_hal_gpio_pincfg_t sInputConfig = AM_HAL_GPIO_PINCFG_INPUT;
    am_hal_gpio_pinconfig(PAUSE_GPIO, sInputConfig);

    uint32_t ui32PausePin = 0;
    am_hal_gpio_state_read(PAUSE_GPIO, AM_HAL_GPIO_INPUT_READ, &ui32PausePin);

    if (ui32PausePin)
    {
        am_util_stdio_printf("Pausing (GPIO high).\n");
        while (1);
    }
}

//******************************************************************************
//
// Crypto power up
//
//******************************************************************************
void
crypto_power_up(void)
{
    uint32_t ui32Status = am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);

    if (ui32Status)
    {
        am_util_stdio_printf("Crypto power-up failed: 0x%08X", ui32Status);
    }
}

//******************************************************************************
//
// Crypto power down
//
//******************************************************************************
void
crypto_power_down(void)
{
    uint32_t ui32Status = am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);

    if (ui32Status)
    {
        am_util_stdio_printf("Crypto power-down failed: 0x%08X", ui32Status);
    }
}

//******************************************************************************
//
// Initialize the crypto test
//
//******************************************************************************
void
crypto_test_init(void)
{
    RndContex.rndState = &RndState;
    RndContex.entropyCtx = &MbedtlsEntropy;

    crypto_power_up();

    mbedtls_memory_buffer_alloc_init((uint8_t*)g_ui32MbedTLSHeap, AM_MBEDTLS_HEAP_SIZE);
    CC_LibInit(&RndContex, &RndWorkBuff);

    mbedtls_mng_suspend(0, 0);
    crypto_power_down();
}

//******************************************************************************
//
// Crypto operations to be used in the main loop.
//
//******************************************************************************
void
crypto_test_operation(void)
{
    //
    // Initialize the RSA module, and import our keys.
    //
    mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);
    mbedtls_mpi_read_string(&rsa.N, 16, PUBLIC_RSA_N);
    mbedtls_mpi_read_string(&rsa.E, 16, PUBLIC_RSA_E);

    rsa.len = ( mbedtls_mpi_bitlen( &rsa.N ) + 7 ) >> 3;

    if (mbedtls_rsa_check_pubkey(&rsa))
    {
        while (1);
    }

    //
    // Run SHA256 on our binary, and then compare against the previously calculated hash.
    //
    uint8_t digest[TEST_HASH_SIZE];
    mbedtls_sha256(pui8TestBinary, TEST_BIN_SIZE, digest, false);

    uint32_t i;
    uint32_t ui32HashErrors = 0;
    for (i = 0; i < TEST_HASH_SIZE; i++)
    {
        if (digest[i] != pui8TestHash[i])
        {
            ui32HashErrors++;
        }
    }

    //
    // Print our SHA256 results.
    //
    if (ui32HashErrors == 0)
    {
        am_util_stdio_printf("SHA256 values match.\n");
    }
    else
    {
        am_util_stdio_printf("Error, %d mismatches found in SHA256.\n", ui32HashErrors);
        while (1);
    }

    //
    // Attempt to verify the provided signature using our public key.
    //
    uint32_t result = mbedtls_rsa_pkcs1_verify(&rsa,
                                               NULL,
                                               NULL,
                                               MBEDTLS_RSA_PUBLIC,
                                               MBEDTLS_MD_SHA256,
                                               32,
                                               digest,
                                               pui8TestSignature);

    //
    // Release the RSA context. We're done with RSA operations.
    //
    mbedtls_rsa_free(&rsa);

    //
    // Print our RSA results.
    //
    if (result == 0)
    {
        am_util_stdio_printf("RSA signature verified.\n", result);
    }
    else
    {
        am_util_stdio_printf("RSA Error: 0x%08X\n", result);
        while (1);
    }

}

//*****************************************************************************
//
// Main function
//
//*****************************************************************************
int
main(void)
{
    //
    // The "NO_INIT_COUNTER" is pointing to scratch register 1. This will be
    // set to zero automatically on a full power-cycle, but it will keep its
    // value through POI. This will help us keep track of how many times we've
    // completed a full cycle of this test.
    //
    AM_REGVAL(NO_INIT_COUNTER) += 1;

    //
    // Enable printing
    //
    am_bsp_debug_printf_enable();
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Crypto power test POI# %d running from 0x%x\n\n", AM_REGVAL(NO_INIT_COUNTER), AM_REGVAL(0xE000ED08));

    //
    // Standard application initial setup.
    //
    am_bsp_low_power_init();
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

    //
    // Initialize a secondary task to add to the CPU load.
    //
    secondary_task_init();
    secondary_task_enable();

    //
    // Prepare the crypto task.
    //
    crypto_test_init();

    //
    // Power the crypto on and off, and run a few crypto operations.
    //
    for (uint32_t i = 0; i < NUM_TRIALS; i++)
    {
        am_util_stdio_printf("**** Trial %d ****\n", i);

        crypto_power_up();
        mbedtls_mng_resume(0, 0);

#if RUN_CRYPTO_OPERATIONS
        crypto_test_operation();
#endif

        mbedtls_mng_suspend(0, 0);
        crypto_power_down();
    }

    //
    // Wrap up our secondary task (also performs a disable).
    //
    secondary_task_complete();

    //
    // Stop here if the configured GPIO is set.
    //
    gpio_pause();

    //
    // Reset the chip using a POI, but stop every 100th POI.
    //
    if ((AM_REGVAL(NO_INIT_COUNTER) % MAX_POI_COUNT) == 0)
    {
        am_util_stdio_printf("Test Complete\n");
        while (1);
    }
    else
    {
        am_util_stdio_printf("Resetting (POI)\n");
        am_util_delay_ms(5);
        AM_REGVAL(0x40000004) = 0x1B;
    }
}
