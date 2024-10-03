//*****************************************************************************
//
//! @file crypto_task.c
//!
//! @brief Task to keep the crypto engine busy
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "complex_test_cases.h"
#include "crypto_task.h"

#include "FreeRTOS.h"

//******************************************************************************
//
// Runtime crypto includes
//
//******************************************************************************
#define MBEDTLS_CONFIG_FILE <config-cc312-apollo4-no-os.h>

#include "cc_lib.h"

#include "mbedtls/rsa.h"
#include "mbedtls/sha256.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/memory_buffer_alloc.h"
#include "mbedtls_cc_mng.h"
#include "crypto_test_data.h"
#include "rsa_keys.h"


//*****************************************************************************
//
// crypto task handle.
//
//*****************************************************************************
TaskHandle_t crypto_task_handle;

//*****************************************************************************
//
// Global variables and templates
//
//*****************************************************************************
volatile bool g_bCryptoTaskComplete = false;

extern volatile bool g_bPlaybackTaskComplete;

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
// Perform initial setup for the crypto task.
//
//*****************************************************************************
void
CryptoTaskSetup(void)
{
    bool bOTPEnabled;
    uint32_t ui32Status;
    RndContex.rndState = &RndState;
    RndContex.entropyCtx = &MbedtlsEntropy;

    am_util_stdio_printf("Crypto task: setup\r\n");
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

    if (ui32Status)
    {
        am_util_stdio_printf("Crypto power-up failed: 0x%08X", ui32Status);
    }
    mbedtls_memory_buffer_alloc_init((uint8_t*)g_ui32MbedTLSHeap, AM_MBEDTLS_HEAP_SIZE);
    CC_LibInit(&RndContex, &RndWorkBuff);
}

//*****************************************************************************
//
// CryptoTask - Put data into a stream_buffer located in PSRAM XIP space
//
//*****************************************************************************
void
CryptoTask(void *pvParameters)
{

    am_util_stdio_printf("\r\nRunning Crypto Task\r\n");

    while (!g_bPlaybackTaskComplete)
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
        if (ui32HashErrors != 0)
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
        if (result != 0)
        {
            am_util_stdio_printf("RSA Error: 0x%08X\n", result);
            while (1);
        }

    }
    am_util_stdio_printf("\r\nSuspending Crypto Task\r\n");
    // Indicate the Source Task loops are complete
    g_bCryptoTaskComplete = true;
    // Suspend and delete the task
    vTaskDelete(NULL);
}

