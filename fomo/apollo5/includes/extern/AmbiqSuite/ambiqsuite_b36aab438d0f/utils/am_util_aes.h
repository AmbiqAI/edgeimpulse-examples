//********************************************************************************
//
//! @file am_util_aes.h
//!
//! @brief AES Encryption
//!
//! @addtogroup aes AES
//! @ingroup utils
//! @{
//
//********************************************************************************

//********************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//********************************************************************************
#ifndef AM_UTIL_AES_H
#define AM_UTIL_AES_H

#ifdef __cplusplus
extern "C"
{
#endif

//********************************************************************************
//
// External function definitions
//
//********************************************************************************

//********************************************************************************
//
//! @brief Key expansion for AES encryption
//!
//! @param key - pointer to the key (source)
//! @param w - pointer to the extended key (destination)
//! @param keysize - Key size (128/192/256)
//
//********************************************************************************
extern void KeyExpansion(uint8_t *key, uint32_t *w, int keysize);

//********************************************************************************
//
//! @brief Key expansion for AES decryption
//!
//! @param key - pointer to the key (destination)
//! @param w - pointer to the extended key (source)
//! @param keysize - Key size (128/192/256)
//
//********************************************************************************
extern void KeyExpansionDecrypt(uint8_t *key, uint32_t *w, int keysize);

//********************************************************************************
//
//! @brief Encrypt block(s) of data using AES encryption
//!
//! @param data - pointer to the key (destination)
//! @param extended_key - pointer to the extended key (source)
//! @param num_blocks - number of blocks to encrypt
//! @param keysize - Key size (128/192/256)
//
//********************************************************************************
extern void aes_encrypt(uint32_t * data,
                        uint32_t * extended_key,
                        int   num_blocks,
                        int   keysize);  //128/192/256

//********************************************************************************
//
//! @brief Decrypt block(s) of data using AES encryption
//!
//! @param data - pointer to the key (destination)
//! @param extended_key - pointer to the extended key (source)
//! @param num_blocks - number of blocks to decrypt
//! @param keysize - Key size (128/192/256)
//
//********************************************************************************
extern void aes_decrypt(uint32_t * data,
                        uint32_t * extended_key,
                        int   num_blocks,
                        int   keysize);  //128/192/256

//********************************************************************************
//
//! @brief Encrypt block(s) of data using AES encryption and cipher block chaining
//!
//! @param data - pointer to the key (destination)
//! @param extended_key - pointer to the extended key (source)
//! @param num_blocks - number of blocks to encrypt
//! @param keysize - Key size (128/192/256)
//! @param iv - pointer to cipher block
//
//********************************************************************************
extern void aes_encrypt_cbc(uint32_t * data,
                            uint32_t * extended_key,
                            int   num_blocks,
                            int   keysize,  //128/192/256
                            uint32_t * iv); //16 bytes on initialize vector

//********************************************************************************
//
//! @brief Decrypt block(s) of data using AES encryption and cipher block chaining
//!
//! @param data - pointer to the key (destination)
//! @param extended_key - pointer to the extended key (source)
//! @param num_blocks - number of blocks to decrypt
//! @param keysize - Key size (128/192/256)
//! @param iv - pointer to cipher block
//
//********************************************************************************
extern void aes_decrypt_cbc(uint32_t * data,
                            uint32_t * extended_key,
                            int   num_blocks,
                            int   keysize,  //128/192/256
                            uint32_t * iv);

#ifdef __cplusplus
}
#endif

#endif // AM_UTIL_AES_H

//********************************************************************************
//
// End Doxygen group.
//! @}
//
//********************************************************************************

