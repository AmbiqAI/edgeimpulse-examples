//*****************************************************************************
//!
//! @file apollo4_secbl_secure.h
//!
//! @brief the header file of secondary bootloader secure functions
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef APOLLO4_SECBL_SECURE_H
#define APOLLO4_SECBL_SECURE_H

#include "apollo4_secbl.h"

#define SHA384_SIZE 48
#define AES128_SIZE 16

#if AUTH_ALGO == AUTH_DEFAULT_RSA3072_ALGO
#define DIGEST_SIZE SHA384_SIZE
#else
#error "please specify the authentication digest length"
#endif

#if CRYPT_ALGO == CRYPT_DEFAULT_AES128
#define CRYPT_SIZE  AES128_SIZE
#define IMG_CRYPT_IV(pComHdr) (void *)&pComHdr->enc_info.aesCtr128
#else
#error "please specify the crypt key length & crypt initial vector"
#endif

typedef struct
{
    uint32_t                clearSize; // clear text size
    bool                    bDecrypt;  // if the image is encrypted
    uint8_t                 key[CRYPT_SIZE]; // encryption key size
    uint8_t                 iv[CRYPT_SIZE]; // initial vector of the encryption algorithm
} ssbl_decrypt_info_t;

//*****************************************************************************
//
//! @brief verify the authentication
//!
//! @param pAuthKey - the authentication key
//! @param bufAddr - buffer address of the data which needs to be authenticated
//! @param length - length of the data which needs to be authenticated
//! @param pDigest - the final authentication digital signature
//! @param pSig - the signature
//!
//! @return None
//
//*****************************************************************************
bool ssbl_auth_verify(uint8_t *pAuthKey, uint32_t bufAddr, uint32_t length, uint8_t *pDigest, uint8_t *pSig);

//*****************************************************************************
//
//! @brief Initialize the MBED SW security engine
//!        Note: default use MBED TLS security engine
//!
//! @param None
//!
//! @return None
//
//*****************************************************************************
void ssbl_secure_init(void);

//*****************************************************************************
//
//! @brief Initialize the MBED AES128 crypt algorithm
//!        Note: default use MBED TLS AES128 algorithm
//!
//! @param pKey - the AES128 cryption key
//!
//! @return None
//
//*****************************************************************************
void ssbl_crypt_init(uint8_t *pKey);

//*****************************************************************************
//
//! @brief decrypt the information in the 'cipherText' and put it in the 'plainText'
//!
//! @param ciperText - the crypted data
//! @param plainText - the decrypted data
//! @param size - size of the data
//! @param pIV - initial vector for decryption
//!
//! @return None
//
//*****************************************************************************
void ssbl_crypt_decrypt(uint8_t *cipherText, uint8_t *plainText, uint32_t size, uint8_t *pIv);

#endif // APOLLO4_SECBL_SECURE_H
