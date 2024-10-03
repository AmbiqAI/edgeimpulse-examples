//*****************************************************************************
//
//! @file crypto_operations.c
//!
//! @brief Cryptographic operations for the stress test.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>

#include "am_mcu_apollo.h"
#include "am_util.h"

#include "crypto_stress_test_config.h"
#include "crypto_operations.h"

//******************************************************************************
//
// Global macros.
//
//******************************************************************************

//******************************************************************************
//
// Runtime crypto globals.
//
//******************************************************************************
#define MBEDTLS_CONFIG_FILE <config-cc312-apollo4-no-os.h>
// #include "config-cc312-apollo4-no-os.h"

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
// Global to track if Crypto needs to be quiesced
volatile bool bCryptoInUse = false;
uint32_t gOtpOffsetStart = 0;
uint32_t gOtpSize = 0;
uint32_t gOtpPrePwrStatus = 0;
uint32_t gNVMPrePwrStatus = 0;
uint32_t gInfoSelectOtp = 0;
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

//******************************************************************************
//
// Crypto power up
//
//******************************************************************************
void
crypto_power_up(void)
{
    uint32_t ui32Status;
    bool bEnabled;
#ifdef AM_PART_APOLLO5A
    //
    // Ensure OTP or NVM0 is on before powering Crypto up.
    //
    gOtpPrePwrStatus = PWRCTRL->MEMPWRSTATUS_b.PWRSTOTP;
    gNVMPrePwrStatus = PWRCTRL->MEMPWRSTATUS_b.PWRSTNVM0;
    gInfoSelectOtp   = MCUCTRL->SHADOWVALID_b.INFOCSELOTP;

    if ((gInfoSelectOtp == MCUCTRL_SHADOWVALID_INFOCSELOTP_VALID) &&(PWRCTRL->MEMPWRSTATUS_b.PWRSTOTP == 0)) 
    {
        ui32Status = am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_OTP);
        if (ui32Status)
        {
            PRINT("OTP power-up failed: 0x%08X", ui32Status);
        }
    }
    else if((gInfoSelectOtp == MCUCTRL_SHADOWVALID_INFOCSELOTP_DEFAULT) && (PWRCTRL->MEMPWRSTATUS_b.PWRSTNVM0 == 0))
    {
        PWRCTRL->MEMPWREN_b.PWRENNVM = 1;
    }
#elif defined AM_PART_APOLLO5B
    //
    // Ensure OTP is on before powering Crypto up.
    //
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_OTP, &bEnabled);
    gOtpPrePwrStatus = bEnabled;
    if(!bEnabled)
    {
        ui32Status = am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_OTP);
        if (ui32Status)
        {
            PRINT("OTP power-up failed: 0x%08X", ui32Status);
        }
    }
#endif

#if (ENABLE_CRYPTO_POWER == 1) // Straight PWRCTL
    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 1;
    while (!PWRCTRL->DEVPWRSTATUS_b.PWRSTCRYPTO) ;
#elif (ENABLE_CRYPTO_POWER == 2) // HAL
    ui32Status = am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);
    if (ui32Status)
    {
        PRINT("Crypto power-up failed: 0x%08X", ui32Status);
    }
#endif
}

//******************************************************************************
//
// Crypto power down
//
//******************************************************************************
void
crypto_power_down(void)
{
    uint32_t ui32Status;
#if (ENABLE_CRYPTO_POWER == 1) // Straight PWRCTL
    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 0;
    while (PWRCTRL->DEVPWRSTATUS_b.PWRSTCRYPTO) ;
#elif (ENABLE_CRYPTO_POWER == 2) // HAL
    ui32Status = am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_CRYPTO_POWERDOWN, 0);
    if (ui32Status)
    {
        PRINT("Crypto power-down failed: 0x%08X", ui32Status);
    }
#endif
#ifdef AM_PART_APOLLO5A
    if((gInfoSelectOtp == MCUCTRL_SHADOWVALID_INFOCSELOTP_VALID) && (gOtpPrePwrStatus == 0))
    {
        //
        // Power down the OTP after powering Crypto down.
        //
        ui32Status = am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_OTP);
        if (ui32Status)
        {
            PRINT("OTP power-down failed: 0x%08X", ui32Status);
        }
    }
    else if((gInfoSelectOtp == MCUCTRL_SHADOWVALID_INFOCSELOTP_DEFAULT) && (gNVMPrePwrStatus == 0))
    {
        PWRCTRL->MEMPWREN_b.PWRENNVM = PWRCTRL_MEMPWREN_PWRENNVM_DIS;
    }
#elif defined AM_PART_APOLLO5B
    if(gOtpPrePwrStatus == 0)
    {
        //
        // Power down the OTP after powering Crypto down.
        //
        ui32Status = am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_OTP);
        if (ui32Status)
        {
            PRINT("OTP power-down failed: 0x%08X", ui32Status);
        }
    }
#endif
}

//******************************************************************************
//
// Initialize the crypto test
//
//******************************************************************************
void
crypto_test_init(void)
{
#if ENABLE_CRYPTO_RUNTIME
    RndContex.rndState = &RndState;
    RndContex.entropyCtx = &MbedtlsEntropy;

    crypto_power_up();
    mbedtls_memory_buffer_alloc_init((uint8_t*)g_ui32MbedTLSHeap, AM_MBEDTLS_HEAP_SIZE);
    CC_LibInit(&RndContex, &RndWorkBuff);

#if (ENABLE_CRYPTO_POWER != 0)
    mbedtls_mng_suspend(0, 0);
#endif
    crypto_power_down();
#endif
}

//******************************************************************************
//
// Crypto operations
//
//******************************************************************************
void
crypto_test_operation(void)
{
    if (gOtpSize == 0)
    {
        // INFOC-OTP
        gOtpOffsetStart = AM_REG_OTP_INFOC_KEYBANK_CUST_QUADRANT0_KEY0_O;   // Customer Keybank area
        gOtpSize = ((AM_REG_OTP_INFOC_KEYBANK_CUST_QUADRANT3_KEY15_O - AM_REG_OTP_INFOC_KEYBANK_CUST_QUADRANT0_KEY0_O) / 4); // Full keybank
#ifdef AM_PART_APOLLO5A
        if (!(MCUCTRL->SHADOWVALID_b.INFOCSELOTP && MCUCTRL->SHADOWVALID_b.OTPREADY))
        {
            // INFOC-MRAM
            gOtpOffsetStart = AM_REG_INFOC_KEYBANK_CUST_QUADRANT0_KEY0_O; // Customer Keybank area
            gOtpSize = ((AM_REG_INFOC_KEYBANK_CUST_QUADRANT3_KEY15_O - AM_REG_INFOC_KEYBANK_CUST_QUADRANT0_KEY0_O) / 4); // Full keybank
        }
#endif
    }
    crypto_power_up();
    SET_STATUS_GPIO(7);
#if ENABLE_CRYPTO_RUNTIME
    mbedtls_mng_resume(0, 0);
#endif

    crypto_rsa_sign_verify();
    otp_read_loop();
    otp_write_loop();

#if ENABLE_CRYPTO_RUNTIME
#if (ENABLE_CRYPTO_POWER != 0)
    mbedtls_mng_suspend(0, 0);
#endif
#endif

    crypto_power_down();
    SET_STATUS_GPIO(14);
}

//******************************************************************************
//
// RSA and SHA operations.
//
//******************************************************************************
void
crypto_rsa_sign_verify(void)
{
#if ENABLE_RSA_SHA
    bCryptoInUse = true;
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
    SET_STATUS_GPIO(8);
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
        PRINT_VERBOSE("SHA256 values match.\n");
    }
    else
    {
        PRINT("Error, %d mismatches found in SHA256.\n", ui32HashErrors);
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

    SET_STATUS_GPIO(9);
    //
    // Release the RSA context. We're done with RSA operations.
    //
    mbedtls_rsa_free(&rsa);

    //
    // Print our RSA results.
    //
    if (result == 0)
    {
        PRINT_VERBOSE("RSA signature verified.\n", result);
        SET_STATUS_GPIO(20);
    }
    else
    {
        PRINT("RSA Error: 0x%08X\n", result);
        while (1);
    }
    bCryptoInUse = false;
#endif
}

//******************************************************************************
//
// OTP read loop
//
//******************************************************************************
void
otp_read_loop(void)
{
#if ENABLE_OTP_READS
    uint32_t i, j;
    uint32_t ui32ReadVal;
    uint32_t otpOffset = gOtpOffsetStart;

    for (i = 0; i < OTP_READ_CYCLES; i++)
    {
        for (j = 0; j < gOtpSize; j++)
        {
            SET_STATUS_GPIO(10);
            am_hal_infoc_read_word((otpOffset + j*4), &ui32ReadVal);
            SET_STATUS_GPIO(11);
        }
    }
#endif
}

//******************************************************************************
//
// OTP write loop
//
//******************************************************************************
void
otp_write_loop(void)
{
#if ENABLE_OTP_WRITES
    uint32_t i, j;
    uint32_t ui32WriteVal = 0;
    uint32_t otpOffset = gOtpOffsetStart;

    for (i = 0; i < OTP_WRITE_CYCLES; i++)
    {
        for (j = 0; j < gOtpSize; j++)
        {
            SET_STATUS_GPIO(12);
            am_hal_infoc_write_word((otpOffset + j*4), ui32WriteVal);
            SET_STATUS_GPIO(13);
            while (CRYPTO->AIBFUSEPROGCOMPLETED_b.AIBFUSEPROGCOMPLETED == 0);
        }
    }
#endif
}
