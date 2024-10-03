//*****************************************************************************
//
//! @file apollo4_secbl_secure.c
//!
//! @brief security related functions of the secondary bootloader
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

#include <string.h>
#include "apollo4_secbl.h"
#include "config-cc312-apollo4-no-os.h"
#include "mbedtls/aes.h"
#include "mbedtls/rsa.h"
#include "mbedtls/sha512.h"
#include "mbedtls/memory_buffer_alloc.h"

mbedtls_aes_context aes_ctx;
mbedtls_rsa_context rsa;

static unsigned char g_mbed_buf[8192];

void
ssbl_secure_init(void)
{
    // Initialize the MBED SW security engine
    mbedtls_memory_buffer_alloc_init(g_mbed_buf, sizeof(g_mbed_buf));
}

bool ssbl_auth_verify(uint8_t *pAuthKey, uint32_t bufAddr, uint32_t length, uint8_t *pDigest, uint8_t *pSig)
{
    bool bmatch = true;

#if AUTH_ALGO == AUTH_DEFAULT_RSA3072_ALGO
    //
    // Initialize the RSA module, and import our keys.
    //
    mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);
    mbedtls_mpi_read_string(&rsa.N, 16, pAuthKey);
    mbedtls_mpi_read_string(&rsa.E, 16, "10001");

    rsa.len = ( mbedtls_mpi_bitlen( &rsa.N ) + 7 ) >> 3;
    if (mbedtls_rsa_check_pubkey(&rsa))
    {
        am_util_stdio_printf("mbedtls_rsa_check_pubkey failed\n");
        while (1);
    }

    //
    // Run SHA384 on our binary, and then compare against the previously calculated hash.
    //
    mbedtls_sha512((uint8_t*)bufAddr, length, pDigest, true);

    //
    // Attempt to verify the provided signature using our public key.
    //
    uint32_t result = mbedtls_rsa_pkcs1_verify(&rsa,
                                               NULL,
                                               NULL,
                                               MBEDTLS_RSA_PUBLIC,
                                               MBEDTLS_MD_SHA384,
                                               DIGEST_SIZE,
                                               pDigest,
                                               pSig);
    //
    // Print our RSA results.
    //
    if (result == 0)
    {
        am_util_stdio_printf("RSA signature verified.\n", result);
        bmatch = true;
    }
    else
    {
        am_util_stdio_printf("Install Authentication failed for image\nRSA Error: 0x%08X\n", result);
        bmatch = false;
    }
    //
    // Release the RSA context. We're done with RSA operations.
    //
    mbedtls_rsa_free(&rsa);
#else
#error "please add the your specifc authentication algorithm here"
#endif
    return bmatch;
}

void ssbl_crypt_init(uint8_t *pKey)
{
#if CRYPT_ALGO == CRYPT_DEFAULT_AES128
    mbedtls_aes_init(&aes_ctx);
    mbedtls_aes_setkey_dec(&aes_ctx, pKey, AES128_SIZE*8);
#else
#error "please add the your specifc cryto algorithm here"
#endif
}

void ssbl_crypt_decrypt(uint8_t *cipherText, uint8_t *plainText, uint32_t size, uint8_t *pIv)
{
#if CRYPT_ALGO == CRYPT_DEFAULT_AES128
    mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_DECRYPT,
                      size, pIv,
                      cipherText, plainText);
#else
#error "please add the your specifc cryto algorithm here"
#endif
}

