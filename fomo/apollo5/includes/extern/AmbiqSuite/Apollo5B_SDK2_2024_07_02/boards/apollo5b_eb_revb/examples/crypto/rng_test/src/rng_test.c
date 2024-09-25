//*****************************************************************************
//
//! @file rng_test.c
//!
//! @brief A simple example to demonstrate use of the mbedtls RNG.
//!
//! @addtogroup crypto_examples Crypto Examples
//
//! @defgroup rng_test RNG Example
//! @ingroup crypto_examples
//! @{
//!
//! Purpose: This example demonstrates the use of the RSA CryptoCell Library<br>
//! It initializes the mbedTLS crypto library.<br>
//! It then invokes crypto and uses it to produce a stream of random numbers.<br>
//! It then disables crypto and goes to sleep<br>
//!
//! This example requests 20480 StackSize.<br>
//! The stack size is declared in the ini file and startup file.<br>
//! The user should specify this size in the apollo5a-system-config.yaml file<br>
//!
//! Additional Information:
//! To enable debug printing, add the following project-level macro definitions.<br>
//! When defined, debug messages will be sent over ITM/SWO at 1M Baud.<br>
//! - #define AM_DEBUG_PRINTF<br>
//!
//! Note that when this macro is defined, the device will never achieve deep
//! sleep, only normal sleep, due to the ITM (and thus the HFRC) being enabled.<br>
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

//
//! @note MBEDTLS_CONFIG_FILE must be defined for crypto to run properly
//
#define MBEDTLS_CONFIG_FILE <config-cc312-apollo4-no-os.h>

#include "cc_lib.h"

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/aes.h"
#include "mbedtls/entropy.h"
#include "mbedtls/memory_buffer_alloc.h"
#include "mbedtls_cc_mng.h"

//*****************************************************************************
//
// Globals.
//
//*****************************************************************************

//
// Dynamic memory for mbedTLS stack.
//
#define AM_MBEDTLS_HEAP_SIZE (128*1024/4)
uint32_t g_ui32MbedTLSHeap[AM_MBEDTLS_HEAP_SIZE];

//
// Context variables for mbedTLS operations.
//
CCRndContext_t RndContext;
CCRndWorkBuff_t RndWorkBuff;
mbedtls_ctr_drbg_context RndState;
mbedtls_entropy_context MbedtlsEntropy;

//
// Output buffer for random numbers.
//
#define NUM_RANDOM_BYTES 128
uint8_t pui8RandomBytes[NUM_RANDOM_BYTES];

int
main(void)
{
    uint32_t ui32Error;
    bool bOTPEnabled;
    uint32_t ui32Status;


    /* init Rnd context's inner member */
    RndContext.rndState = &RndState;
    RndContext.entropyCtx = &MbedtlsEntropy;

    //
    // Enable printing
    //
    am_bsp_debug_printf_enable();
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("RNG Demo Example!\n\n");

    //
    // This will also power down crypto
    //
    am_bsp_low_power_init();

    //
    // One time library initialization - to be done at the beginning of program
    //
    am_util_stdio_printf("Powering On Crypto\n");

    //
    // Check and Power on OTP if it is not already on.
    //
    ui32Status = am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_OTP, &bOTPEnabled);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Error during read of OTP power status\n");
    }
    if (!bOTPEnabled)
    {
        ui32Status = am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_OTP);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Error in power up of OTP module\n");
        }
    }

    //
    // Enable the Crypto module 
    //
    ui32Status = am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Error in power up of Crypto module\n");
    }

    //
    // Initiailize MbedTLS
    //
    mbedtls_memory_buffer_alloc_init((uint8_t*)g_ui32MbedTLSHeap, AM_MBEDTLS_HEAP_SIZE);
    CC_LibInit(&RndContext, &RndWorkBuff);

    //
    // Call the mbedTLS API to generate random bytes. If successful, print the
    // bytes to ITM. Otherwise, print the relevant error message.
    //
    ui32Error = mbedtls_ctr_drbg_random(RndContext.rndState, pui8RandomBytes, NUM_RANDOM_BYTES);
    if (ui32Error)
    {
        am_util_stdio_printf("mbedTLS error: 0x%08X\n", ui32Error);
    }
    else
    {
        am_util_stdio_printf("Retrieved random bytes:\n");
        for (uint32_t i = 0; i < NUM_RANDOM_BYTES; i++)
        {
            am_util_stdio_printf("0x%02X\n", pui8RandomBytes[i]);
        }
    }

    am_util_stdio_printf("Example Done.\n");

    //
    // We can power off crypto now
    //
    mbedtls_mng_suspend(0, 0);
    ui32Status = am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Error in power down of Crypto module\n");
    }
    //
    // Power down OTP as well.
    //
    ui32Status = am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_OTP);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Error in power down of OTP module\n");
    }
    am_util_stdio_printf("Crypto powered down!\n");
    am_bsp_debug_printf_disable();

    //
    // Loop forever while sleeping.
    //
    while (1)
    {
        //
        // Go to Deep Sleep.
        //
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

