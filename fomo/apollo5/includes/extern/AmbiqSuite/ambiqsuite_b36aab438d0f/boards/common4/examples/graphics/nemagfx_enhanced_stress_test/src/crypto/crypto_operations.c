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
#define OTP_OFFSET_START        0x1C00 // Customer Keybank area
#define OTP_SIZE                ((0x1FFC - 0x1C00) / 4) // Full keybank

//******************************************************************************
//
// Runtime crypto globals.
//
//******************************************************************************
#define MBEDTLS_CONFIG_FILE <config-cc312-apollo4-no-os.h>
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
// Global to track if Crypto needs to be quiesced
volatile bool gbQuiesce = false;
volatile bool bCryptoInUse = false;

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

// #### INTERNAL BEGIN ####
#if ((ENABLE_CRYPTO_POWER == 3) || (ENABLE_CRYPTO_POWER == 4))
static void setAIKey(bool bUnlock)
{
    // Implement AIKey logic here for INTERNAL code
    // This needs to be populated !!!!
#error "AIKey Logic needs to be added!!"
}

#define APB_SYNC am_hal_sysctrl_bus_write_flush
#endif

#if (ENABLE_CRYPTO_POWER == 3)
void
brendan_powerup(void)
{
    uint32_t dummy;
    setAIKey(true);
    // Start with clocks to crypto under PWRCTRL state machine control
    // AM_BFW(CLKGEN, MISC, PWRONCLKENCRYPTO, 0);
    dummy = AM_REGVAL(0x40004044);
    dummy &= ~0x800;
    AM_REGVAL(0x40004044) = dummy;
    // AM_REGWR(MCUCTRLPRIV, DEVICEPGCTRL1, 0x09);  // Match register defaults - device PG (power-gated)
    AM_REGVAL(0x400403DC) = 0x09;
    //      APB_SYNC();  // APB sync read from 0x47FF0000
    //      // Per FAL-371 need PWRCTRL crypto clocks - perform arbitrary (ignored) action to trigger clocks.
    //      // May need to differ from current state in order for PWRCTRL state machine clocks to be enabled.
    //      AM_BFWe(PWRCTRL, DEVPWREN, PWRENCRYPTO, EN);

    //MCUCTRLPRIV->DEVICEPGCTRL1_b.DEVICEPGOVER1 = 1;  // Enable override
    AM_REGVAL(0x400403DC) = 0x80000009;
    APB_SYNC();  // APB sync read from 0x47FF0000
    // Per FAL-371 need PWRCTRL crypto clocks - perform arbitrary (ignored) action to trigger clocks.
    // May need to differ from current state in order for PWRCTRL state machine clocks to be enabled.
    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 1;

    //MCUCTRLPRIV->DEVICEPGCTRL1_b.CRYPTOPGOR = 0;  // Power up (!PG)
    AM_REGVAL(0x400403DC) = 0x80000001;
    APB_SYNC();  // APB sync read from 0x47FF0000
    // Per FAL-371 need PWRCTRL crypto clocks - perform arbitrary (ignored) action to trigger clocks.
    // May need to differ from current state in order for PWRCTRL state machine clocks to be enabled.
    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 0;
    while (PWRCTRL->DEVPWRSTATUS_b.PWRSTCRYPTO) ;
    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 1;
    while (!PWRCTRL->DEVPWRSTATUS_b.PWRSTCRYPTO) ;
    // Turn off clocks to crypto
    dummy = AM_REGVAL(0x40004044);
    dummy |= 0x800;
    AM_REGVAL(0x40004044) = dummy;
    APB_SYNC();
    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 0;

    //AM_BFW(MCUCTRLPRIV, DEVICEPGCTRL1, CRYPTORESETOR, 1);  // Deassert reset
    dummy = AM_REGVAL(0x400403DC);
    dummy |= 0x20;
    AM_REGVAL(0x400403DC) = dummy;
    APB_SYNC();  // APB sync read from 0x47FF0000
    // Per FAL-371 need PWRCTRL crypto clocks - perform arbitrary (ignored) action to trigger clocks.
    // May need to differ from current state in order for PWRCTRL state machine clocks to be enabled.
    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 1;

    //AM_BFW(MCUCTRLPRIV, DEVICEPGCTRL1, CRYPTOISOOR,   1);  // Deassert isolation
    dummy = AM_REGVAL(0x400403DC);
    dummy |= 0x10;
    AM_REGVAL(0x400403DC) = dummy;

    APB_SYNC();  // APB sync read from 0x47FF0000
    // Per FAL-371 need PWRCTRL crypto clocks - perform arbitrary (ignored) action to trigger clocks.
    // May need to differ from current state in order for PWRCTRL state machine clocks to be enabled.
    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 0;

    setAIKey(false);
}

void
brendan_powerdown(void)
{
    uint32_t dummy;
    setAIKey(true);

    CRYPTO->HOSTPOWERDOWN = 1;

    // power off crypto
    //AM_BFW(MCUCTRLPRIV, DEVICEPGCTRL1, CRYPTOISOOR,   0);  // Assert isolation
    dummy = AM_REGVAL(0x400403DC);
    dummy &= ~0x10;
    AM_REGVAL(0x400403DC) = dummy;

    APB_SYNC();  // APB sync read from 0x47FF0000
    // Per FAL-371 need PWRCTRL crypto clocks - perform arbitrary (ignored) action to trigger clocks.
    // May need to differ from current state in order for PWRCTRL state machine clocks to be enabled.
    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 1;

    //AM_BFW(MCUCTRLPRIV, DEVICEPGCTRL1, CRYPTORESETOR, 0);  // Assert reset
    dummy = AM_REGVAL(0x400403DC);
    dummy |= 0x20;
    AM_REGVAL(0x400403DC) = dummy;
    APB_SYNC();  // APB sync read from 0x47FF0000
    // Per FAL-371 need PWRCTRL crypto clocks - perform arbitrary (ignored) action to trigger clocks.
    // May need to differ from current state in order for PWRCTRL state machine clocks to be enabled.
    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 0;

    //AM_BFW(MCUCTRLPRIV, DEVICEPGCTRL1, CRYPTOPGOR,    1);  // Power down (PG)
    dummy = AM_REGVAL(0x400403DC);
    dummy |= 0x8;
    AM_REGVAL(0x400403DC) = dummy;
    APB_SYNC();  // APB sync read from 0x47FF0000
    // Per FAL-371 need PWRCTRL crypto clocks - perform arbitrary (ignored) action to trigger clocks.
    // May need to differ from current state in order for PWRCTRL state machine clocks to be enabled.
    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 1;

    // Allow most recent ignored PWRCTRL PWRENCRYPTO disable to take effect before deasserting override
    APB_SYNC();  // APB sync read from 0x47FF0000
    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 0;
    while (PWRCTRL->DEVPWRSTATUS_b.PWRSTCRYPTO) ;

    //AM_BFW(MCUCTRLPRIV, DEVICEPGCTRL1, DEVICEPGOVER1, 0);  // Enable override
    dummy = AM_REGVAL(0x400403DC);
    dummy &= ~0x80000000;
    AM_REGVAL(0x400403DC) = dummy;

    APB_SYNC();  // APB sync read from 0x47FF0000
    setAIKey(false);
}
#endif

#if (ENABLE_CRYPTO_POWER == 4)
void
brendan_powerup2(void)
{
    uint32_t dummy;
    setAIKey(true);
    // AM_REGWR(MCUCTRLPRIV, DEVICEPGCTRL1, 0x09);  // Match register defaults - device PG (power-gated)
    AM_REGVAL(0x400403DC) = 0x09;

    // AM_BFW(MCUCTRLPRIV, DEVICEPGCTRL1, DEVICEPGOVER1, 1);  // Enable override
    AM_REGVAL(0x400403DC) = 0x80000009;
    APB_SYNC();  // APB sync read from 0x47FF0000
    // Per FAL-371 need PWRCTRL crypto clocks - perform arbitrary (ignored) action to trigger clocks.
    // May need to differ from current state in order for PWRCTRL state machine clocks to be enabled.
    // AM_BFWe(PWRCTRL, DEVPWREN, PWRENCRYPTO, EN);

    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 1;

    //MCUCTRLPRIV->DEVICEPGCTRL1_b.CRYPTOPGOR = 0;  // Power up (!PG)
    AM_REGVAL(0x400403DC) = 0x80000001;
    APB_SYNC();  // APB sync read from 0x47FF0000
    // Per FAL-371 need PWRCTRL crypto clocks - perform arbitrary (ignored) action to trigger clocks.
    // May need to differ from current state in order for PWRCTRL state machine clocks to be enabled.
    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 0;

    //AM_BFW(MCUCTRLPRIV, DEVICEPGCTRL1, CRYPTORESETOR, 1);  // Deassert reset
    dummy = AM_REGVAL(0x400403DC);
    dummy |= 0x20;
    AM_REGVAL(0x400403DC) = dummy;

    APB_SYNC();  // APB sync read from 0x47FF0000
    // Per FAL-371 need PWRCTRL crypto clocks - perform arbitrary (ignored) action to trigger clocks.
    // May need to differ from current state in order for PWRCTRL state machine clocks to be enabled.
    // AM_BFWe(PWRCTRL, DEVPWREN, PWRENCRYPTO, EN);
    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 1;

    // Do not turn off isolation!
    //      AM_BFW(MCUCTRLPRIV, DEVICEPGCTRL1, CRYPTOISOOR,   1);  // Deassert isolation
    //      APB_SYNC();  // APB sync read from 0x47FF0000
    //      // Per FAL-371 need PWRCTRL crypto clocks - perform arbitrary (ignored) action to trigger clocks.
    //      // May need to differ from current state in order for PWRCTRL state machine clocks to be enabled.
    //      AM_BFWe(PWRCTRL, DEVPWREN, PWRENCRYPTO, DIS);
    setAIKey(false);
}

void
brendan_powerdown2(void)
{
    uint32_t dummy;
    setAIKey(true);

    //  Can't talk to crypto if it's isolated      AM_REGWR(CRYPTO, HOSTPOWERDOWN, 1);

    // power off crypto

    // Should not have disabled isolation earlier!
    //      AM_BFW(MCUCTRLPRIV, DEVICEPGCTRL1, CRYPTOISOOR,   0);  // Assert isolation
    //      APB_SYNC();  // APB sync read from 0x47FF0000
    //      // Per FAL-371 need PWRCTRL crypto clocks - perform arbitrary (ignored) action to trigger clocks.
    //      // May need to differ from current state in order for PWRCTRL state machine clocks to be enabled.
    //      AM_BFWe(PWRCTRL, DEVPWREN, PWRENCRYPTO, EN);

    // AM_BFW(MCUCTRLPRIV, DEVICEPGCTRL1, CRYPTORESETOR, 0);  // Assert reset
    dummy = AM_REGVAL(0x400403DC);
    dummy &= ~0x20;
    AM_REGVAL(0x400403DC) = dummy;
    APB_SYNC();  // APB sync read from 0x47FF0000
    // Per FAL-371 need PWRCTRL crypto clocks - perform arbitrary (ignored) action to trigger clocks.
    // May need to differ from current state in order for PWRCTRL state machine clocks to be enabled.
    //AM_BFWe(PWRCTRL, DEVPWREN, PWRENCRYPTO, DIS);
    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 0;

    // AM_BFW(MCUCTRLPRIV, DEVICEPGCTRL1, CRYPTOPGOR,    1);  // Power down (PG)
    dummy = AM_REGVAL(0x400403DC);
    dummy |= 0x8;
    AM_REGVAL(0x400403DC) = dummy;
    APB_SYNC();  // APB sync read from 0x47FF0000
    // Per FAL-371 need PWRCTRL crypto clocks - perform arbitrary (ignored) action to trigger clocks.
    // May need to differ from current state in order for PWRCTRL state machine clocks to be enabled.
    // AM_BFWe(PWRCTRL, DEVPWREN, PWRENCRYPTO, EN);
    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 1;

    // Allow most recent ignored PWRCTRL PWRENCRYPTO disable to take effect before deasserting override
    APB_SYNC();  // APB sync read from 0x47FF0000
    // AM_BFWe(PWRCTRL, DEVPWREN, PWRENCRYPTO, DIS);
    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 0;
    // while (AM_BFR(PWRCTRL, DEVPWRSTATUS, PWRSTCRYPTO)==1);
    while (PWRCTRL->DEVPWRSTATUS_b.PWRSTCRYPTO) ;

    //AM_BFW(MCUCTRLPRIV, DEVICEPGCTRL1, DEVICEPGOVER1, 0);  // Enable override
    dummy = AM_REGVAL(0x400403DC);
    dummy &= ~0x80000000;
    AM_REGVAL(0x400403DC) = dummy;
    APB_SYNC();  // APB sync read from 0x47FF0000
    // Should be no need to sequence pwrctrl state machine

    setAIKey(false);
}
#endif
// #### INTERNAL END ####

//******************************************************************************
//
// Crypto power up
//
//******************************************************************************
void
crypto_power_up(void)
{
#if (ENABLE_CRYPTO_POWER == 1) // Straight PWRCTL
    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 1;
    while (!PWRCTRL->DEVPWRSTATUS_b.PWRSTCRYPTO) ;
#elif (ENABLE_CRYPTO_POWER == 2) // HAL
    uint32_t ui32Status = am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);

    if (ui32Status)
    {
        PRINT("Crypto power-up failed: 0x%08X", ui32Status);
    }
// #### INTERNAL BEGIN ####
#elif (ENABLE_CRYPTO_POWER == 3) // BRENDAN_SEQUENCE
    brendan_powerup();
#elif (ENABLE_CRYPTO_POWER == 4) // BRENDAN_SEQUENCE # 2
    brendan_powerup2();
// #### INTERNAL END ####
#endif
    gbQuiesce = false;
}

//******************************************************************************
//
// Crypto power down
//
//******************************************************************************
void
crypto_power_down(void)
{
#if ((ENABLE_CRYPTO_POWER == 1) || (ENABLE_CRYPTO_POWER == 2))
    if (gbQuiesce == false)
    {
        while (CRYPTO->HOSTCCISIDLE_b.HOSTCCISIDLE == 0);
        //
        // Power down the crypto block.
        //
        CRYPTO->HOSTPOWERDOWN_b.HOSTPOWERDOWN = 1;
        gbQuiesce = true;
    }
#endif
#if (ENABLE_CRYPTO_POWER == 1) // Straight PWRCTL
    PWRCTRL->DEVPWREN_b.PWRENCRYPTO = 0;
    while (PWRCTRL->DEVPWRSTATUS_b.PWRSTCRYPTO) ;
#elif (ENABLE_CRYPTO_POWER == 2) // HAL
    uint32_t ui32Status = am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);

    if (ui32Status)
    {
        PRINT("Crypto power-down failed: 0x%08X", ui32Status);
    }
// #### INTERNAL BEGIN ####
#elif (ENABLE_CRYPTO_POWER == 3) // BRENDAN_SEQUENCE
    brendan_powerdown();
#elif (ENABLE_CRYPTO_POWER == 4) // BRENDAN_SEQUENCE # 2
    brendan_powerdown2();
// #### INTERNAL END ####
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
    gbQuiesce = true;
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
    gbQuiesce = true;
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
    uint32_t otpOffset = OTP_OFFSET_START;

    for (i = 0; i < OTP_READ_CYCLES; i++)
    {
        for (j = 0; j < OTP_SIZE; j++)
        {
            SET_STATUS_GPIO(10);
            am_hal_otp_read_word((otpOffset + j*4), &ui32ReadVal);
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
    uint32_t otpOffset = OTP_OFFSET_START;

    for (i = 0; i < OTP_WRITE_CYCLES; i++)
    {
        for (j = 0; j < OTP_SIZE; j++)
        {
            SET_STATUS_GPIO(12);
            am_hal_otp_write_word((otpOffset + j*4), ui32WriteVal);
            SET_STATUS_GPIO(13);
            while (CRYPTO->AIBFUSEPROGCOMPLETED_b.AIBFUSEPROGCOMPLETED == 0);
        }
    }
#endif
}
