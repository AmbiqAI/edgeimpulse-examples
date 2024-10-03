//*****************************************************************************
//
//! @file crypto_test.c
//!
//! @brief A simple example to demonstrate using runtime crypto APIs.
//!
//! These come from mbedtls and some user scenarios from FAEs
//!
//! This example requests 20480 StackSize.
//! The stack size is decalred in the ini file and startup file.
//! The user should specify this size in the apollo5a-system-config.yaml file
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
#include "string.h"

#define MBEDTLS_CONFIG_FILE <config-cc312-apollo4-no-os.h>

#include "cc_lib.h"

#include "mbedtls/aes.h"
#include "mbedtls/md5.h"
#include "mbedtls/ccm.h"
#include "mbedtls/rsa.h"
#include "mbedtls/sha256.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/memory_buffer_alloc.h"
#include "mbedtls_cc_mng.h"

#include "crypto_test_data.h"
#include "rsa_keys.h"

#define ENABLE_CRYPTO_ON_OFF

#define CUSTOMER_MD5_TEST
#define CUSTOMER_CCM_TEST
#define CUSTOMER_AES_TEST
#define MD5_SELF_TEST
#define CCM_SELF_TEST
#define SHA256_SELF_TEST
#define AES_SELF_TEST

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
mbedtls_md5_context *ctx;

//
// Context variables for mbedTLS operations.
//
mbedtls_rsa_context rsa;
CCRndContext_t RndContex;
CCRndWorkBuff_t RndWorkBuff;
mbedtls_ctr_drbg_context RndState;
mbedtls_entropy_context MbedtlsEntropy;
unsigned char outout_data[15];

uint8_t input[16];
uint8_t output[16];
unsigned char aes_key[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,\
                           0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56};

mbedtls_aes_context g_sMbedtlsAESCtx;

int
main(void)
{
    int ret;
    bool bOTPEnabled;
    uint32_t ui32Status;

    /* init Rnd context's inner member */
    RndContex.rndState = &RndState;
    RndContex.entropyCtx = &MbedtlsEntropy;

    //
    // Enable printing
    //
    am_bsp_debug_printf_enable();
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Crypto Demo Example!\n\n");

#ifdef ENABLE_CRYPTO_ON_OFF
    //
    // This will also power down crypto
    //
    am_bsp_low_power_init();
#endif

    //
    // One time library initialization - to be done at the beginning of program
    //
    am_util_stdio_printf("Doing one time Init!\n\n");

#ifdef ENABLE_CRYPTO_ON_OFF
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
#endif

    //
    // Initiailize MbedTLS
    //
    mbedtls_memory_buffer_alloc_init((uint8_t*)g_ui32MbedTLSHeap, AM_MBEDTLS_HEAP_SIZE);
    CC_LibInit(&RndContex, &RndWorkBuff);

#if defined(CUSTOMER_MD5_TEST)
    mbedtls_md5_context a;
    const unsigned char b[32] = {0};
    unsigned char c[16];

    mbedtls_md5_init(&a);
    mbedtls_md5_update(&a, b, 32);
    mbedtls_md5_finish(&a, c);
#endif

#if defined(MD5_SELF_TEST)
    ret = mbedtls_md5_self_test(1);
    if ( 0 != ret )
    {
        am_util_stdio_printf("Deepsleep Example - mbedtls_md5_self_test %d\n", ret);
    }
#endif

#if defined(SHA256_SELF_TEST)
    ret = mbedtls_sha256_self_test(1);
    if ( 0 != ret )
    {
        am_util_stdio_printf("Deepsleep Example - mbedtls_sha256_self_test %d\n", ret);
    }
#endif

#if defined(AES_SELF_TEST)
    ret = mbedtls_aes_self_test(1);
    if ( 0 != ret )
    {
        am_util_stdio_printf("Deepsleep Example - mbedtls_aes_self_test %d\n", ret);
    }
#endif

#if defined(CUSTOMER_AES_TEST)

    unsigned char cipherTextOutput[16];

    memset(input, 0x00, sizeof(input));
    input[0] = 'H';
    input[1] = 'e';
    input[2] = 'l';
    input[3] = 'l';
    input[4] = 'o';
    input[5] = ' ';
    input[6] = 'A';
    input[7] = 'm';
    input[8] = 'b';
    input[9] = 'i';
    input[10] = 'q';
    input[11] = '!';

    mbedtls_aes_init( &g_sMbedtlsAESCtx );

    mbedtls_aes_setkey_enc( &g_sMbedtlsAESCtx, (const unsigned char*) aes_key, 128 );
    mbedtls_aes_crypt_ecb( &g_sMbedtlsAESCtx, MBEDTLS_AES_ENCRYPT, (const unsigned char*)input, cipherTextOutput);

    mbedtls_aes_setkey_dec( &g_sMbedtlsAESCtx, (const unsigned char*) aes_key, 128 );
    mbedtls_aes_crypt_ecb( &g_sMbedtlsAESCtx, MBEDTLS_AES_DECRYPT, (const unsigned char*)cipherTextOutput, output);

    mbedtls_aes_free( &g_sMbedtlsAESCtx );

    if (memcmp(input, output, 16) == 0)
    {
        am_util_stdio_printf("Pass - CUSTOMER_AES_TEST\n");
    }
    else
    {
        am_util_stdio_printf("Fail - CUSTOMER_AES_TEST\n");
    }
#endif //defined(CUSTOMER_AES_TEST)

#if defined(CUSTOMER_CCM_TEST)
    uint8_t device_key[16] =
    {
        0x89,
    };

    uint8_t iv_t[12] =
    {
        0x10,
    };

    uint8_t associated_text[9] =
    {
        0x62,
    };

    uint8_t bind_data[26] =
    {
        0x0A,
    };

    uint8_t tag_t[16] =
    {
        0x00,
    };

    uint8_t ccm_ret[32] = {0};
    mbedtls_ccm_context ctx;

    mbedtls_memory_buffer_alloc_init((uint8_t*)g_ui32MbedTLSHeap, AM_MBEDTLS_HEAP_SIZE);

    mbedtls_ccm_init(&ctx);

    ret = mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, device_key, 128);
    if ( 0 != ret )
    {
        am_util_stdio_printf("Fail - mbedtls_ccm_setkey %d\n", ret);
    }

    ret = mbedtls_ccm_encrypt_and_tag(&ctx, sizeof(bind_data), iv_t, sizeof(iv_t), associated_text, sizeof(associated_text), bind_data, ccm_ret, tag_t, sizeof(tag_t));
    if ( 0 != ret )
    {
        am_util_stdio_printf("Deepsleep Example - mbedtls_ccm_encrypt_and_tag %d\n", ret);
    }
#endif //defined(CUSTOMER_CCM_TEST)

#if defined(CCM_SELF_TEST)
    ret = mbedtls_ccm_self_test(1);
    if ( 0 != ret )
    {
        am_util_stdio_printf("Failed - mbedtls_ccm_self_test\n");
    }
#endif

#ifdef ENABLE_CRYPTO_ON_OFF
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
#endif
    //
    // Placeholder for application doing other things unrelated to crypto
    //
    am_hal_delay_us(1000);

    am_util_stdio_printf("Crypto Demo Example Done\n");

    while (1);
}
