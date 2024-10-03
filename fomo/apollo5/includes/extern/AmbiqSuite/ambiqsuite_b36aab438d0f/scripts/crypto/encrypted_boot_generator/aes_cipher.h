//*****************************************************************************
//
//! @file aes_cipher.h
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AES_CIPHER_H
#define AES_CIPHER_H

#include <stdint.h>

//uint32_t w[60] will work for all key lengths
void KeyExpansion(uint8_t *key, uint32_t *w, int keysize);         //encrypt
void KeyExpansionDecrypt(uint8_t *key, uint32_t *w, int keysize);  //decrypt

void aes_encrypt(uint32_t * data,
                 uint32_t * extended_key,
                 int   num_blocks,
                 int   keysize);  //128/192/256

void aes_decrypt(uint32_t * data,
                 uint32_t * extended_key,
                 int   num_blocks,
                 int   keysize);  //128/192/256

#ifdef AES_CBC_MODE
void aes_encrypt_cbc(uint32_t * data,
                     uint32_t * extended_key,
                     int   num_blocks,
                     int   keysize,  //128/192/256
                     uint32_t * iv); //16 bytes on initialize vector

void aes_decrypt_cbc(uint32_t * data,
                     uint32_t * extended_key,
                     int   num_blocks,
                     int   keysize,  //128/192/256
                     uint32_t * iv);
#endif

#endif

