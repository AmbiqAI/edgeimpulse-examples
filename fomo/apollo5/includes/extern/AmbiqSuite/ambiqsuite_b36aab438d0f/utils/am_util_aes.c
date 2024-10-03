//*****************************************************************************
//
//! @file am_util_aes.c
//!
//! @brief AES Encryption
//!
//! @addtogroup aes AES
//! @ingroup utils
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//aes_encrypt.c for AES written by Kevin Coopman

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "am_util_aes.h"
#include "am_util_aes_tables.h"

#define pre_round_macro_en(in1, in2, in3, in4, out)         \
  out ^=  T1[(in1)&0xff] ^ T2[(in2>>8)&0xff] ^ T3[(in3>>16)&0xff] ^ T4[(in4>>24) & 0xff];

#define loop_round_macro_en(in1, in2, in3, in4, column_out) \
  column_out ^=  T1[(in1>>24)&0xff] ^ T2[(in2>>16)&0xff] ^ T3[(in3>>8)&0xff] ^ T4[in4 & 0xff];

#define final_round_macro_en(in1, in2, in3, in4, out)       \
  out =  s_box[(in1 >> 24) & 0xff] ^ (s_box[(in2 >> 16) & 0xff] << 8) ^ (s_box[(in3 >> 8) & 0xff] << 16) ^ (s_box[in4 & 0xff] << 24);

//*****************************************************************************
//
// Encrypt block(s) of data using AES encryption
//
//*****************************************************************************
void aes_encrypt(uint32_t * data,
                 uint32_t * extended_key_base,
                 int   num_blocks,
                 int   keysize)
{

    int i, jj;
    unsigned int R4, R5, R6, R7;
    unsigned int R9, R10, R11, R12;
    uint32_t * extended_key;
    int loop_count = (( (keysize - 128) >> 6) + 4);

    //
    // instead of always allocating memory for variables, this loop was put in
    // place to go through all the blocks. The pointers at the end of the loop
    // increment to the next block and also the extended key pointer is reset to
    // the beginning
    //
    for ( jj = 0; jj < num_blocks; jj++ )
    {
        extended_key = extended_key_base;

        R4 =  data[0];  //data[3,2,1,0]
        R5 =  data[1];  //data[7,6,5,4]
        R6 =  data[2];  //data[11,10,9,8]
        R7 =  data[3];  //data[15,14,13,12]

        //
        //get extended key
        //
        R9      = *extended_key++;
        R10     = *extended_key++;
        R11     = *extended_key++;
        R12     = *extended_key++;

        //
        //add in rounded key
        //
        R4 ^= R9;
        R5 ^= R10;
        R6 ^= R11;
        R7 ^= R12;

        R9      = *extended_key++;
        R10     = *extended_key++;
        R11     = *extended_key++;
        R12     = *extended_key++;

        //
        //round 1
        //
        pre_round_macro_en(R4, R5, R6, R7, R9);
        pre_round_macro_en(R5, R6, R7, R4, R10);
        pre_round_macro_en(R6, R7, R4, R5, R11);
        pre_round_macro_en(R7, R4, R5, R6, R12);

        for ( i = 0; i < loop_count; i++ )
        {
            R4 = *extended_key++;
            R5 = *extended_key++;
            R6 = *extended_key++;
            R7 = *extended_key++;

            //
            //column0 ------------------------------------------------------
            //
            loop_round_macro_en(R9, R10, R11, R12, R4);

            //
            //column1  ------------------------------------------------------
            //
            loop_round_macro_en(R10, R11, R12, R9, R5);

            //
            //column2  ------------------------------------------------------
            //
            loop_round_macro_en(R11, R12, R9, R10, R6);

            //
            //column3  ------------------------------------------------------
            //
            loop_round_macro_en(R12, R9, R10, R11, R7);

            //**************************************************************//
            //
            //second loop unrolled
            //

            R9  = *extended_key++;
            R10 = *extended_key++;
            R11 = *extended_key++;
            R12 = *extended_key++;

            //
            //column0 ------------------------------------------------------
            //
            loop_round_macro_en(R4, R5, R6, R7, R9);

            //
            //column1 ------------------------------------------------------
            //
            loop_round_macro_en(R5, R6, R7, R4, R10);

            //
            //column2  ------------------------------------------------------
            //
            loop_round_macro_en(R6, R7, R4, R5, R11);

            //
            //column3  ------------------------------------------------------
            //
            loop_round_macro_en(R7, R4, R5, R6, R12);
        }

        //********************************************************************//
        //********************************************************************//
        //
        //final round
        //
        final_round_macro_en(R9, R10, R11, R12, R4);
        final_round_macro_en(R10, R11, R12, R9, R5);
        final_round_macro_en(R11, R12, R9, R10, R6);
        final_round_macro_en(R12, R9, R10, R11, R7);

        //
        //extended key
        //
        R9  = *extended_key++;
        R10 = *extended_key++;
        R11 = *extended_key++;
        R12 = *extended_key++;

        //
        //STEP 9 - add the final round key and store the memory
        //
        R4 ^= R9;
        R5 ^= R10;
        R6 ^= R11;
        R7 ^= R12;

        //
        //store the output block
        //
        data[0] = R4;
        data[1] = R5;
        data[2] = R6;
        data[3] = R7;

        //
        //increment the data pointer to the next block
        //
        data += 4;
    } //for (jj=0; jj < num_blocks; jj++)
}

//*****************************************************************************
//
// Encrypt block(s) of data using AES encryption and cipher block chaining
//
//*****************************************************************************
void aes_encrypt_cbc(uint32_t * data,
                     uint32_t * extended_key_base,
                     int   num_blocks,
                     int   keysize,
                     uint32_t * iv)
{
    int i, jj;
    unsigned int R4, R5, R6, R7;
    unsigned int R9, R10, R11, R12;
    uint32_t * extended_key;
    int loop_count = ( ((keysize - 128) >> 6) + 4 );

    //
    //load the initialization vector up for the first XOR
    //
    R4 = iv[0];
    R5 = iv[1];
    R6 = iv[2];
    R7 = iv[3];

    //
    //instead of always allocation memory for variables, this loop
    // was put in place to go through all the blocks. The pointers at the end of the
    // loop increment to the next block and also the extended key pointer is reset to the
    // beginning
    //
    for (  jj = 0; jj < num_blocks; jj++ )
    {
        extended_key = extended_key_base;

        //
        //CBC chaning and XOR in the previous block
        //
        R4 ^= data[0];
        R5 ^= data[1];
        R6 ^= data[2];
        R7 ^= data[3];

        //
        //get extended key
        //
        R9      = *extended_key++;
        R10     = *extended_key++;
        R11     = *extended_key++;
        R12     = *extended_key++;

        //
        //add in rounded key
        //
        R4 ^= R9;
        R5 ^= R10;
        R6 ^= R11;
        R7 ^= R12;

        R9      = *extended_key++;
        R10     = *extended_key++;
        R11     = *extended_key++;
        R12     = *extended_key++;

        //round 1
        pre_round_macro_en(R4, R5, R6, R7, R9);
        pre_round_macro_en(R5, R6, R7, R4, R10);
        pre_round_macro_en(R6, R7, R4, R5, R11);
        pre_round_macro_en(R7, R4, R5, R6, R12);

        for ( i = 0; i < loop_count; i++ )
        {
            R4 = *extended_key++;
            R5 = *extended_key++;
            R6 = *extended_key++;
            R7 = *extended_key++;

            //
            //column0 ------------------------------------------------------
            //
            loop_round_macro_en(R9, R10, R11, R12, R4);

            //
            //column1  ------------------------------------------------------
            //
            loop_round_macro_en(R10, R11, R12, R9, R5);

            //
            //column2  ------------------------------------------------------
            //
            loop_round_macro_en(R11, R12, R9, R10, R6);

            //
            //column3  ------------------------------------------------------
            //
            loop_round_macro_en(R12, R9, R10, R11, R7);

            //**************************************************************//
            //
            //second loop unrolled
            //

            R9  = *extended_key++;
            R10 = *extended_key++;
            R11 = *extended_key++;
            R12 = *extended_key++;

            //
            //column0 ------------------------------------------------------
            //
            loop_round_macro_en(R4, R5, R6, R7, R9);

            //
            //column1 ------------------------------------------------------
            //
            loop_round_macro_en(R5, R6, R7, R4, R10);

            //
            //column2  ------------------------------------------------------
            //
            loop_round_macro_en(R6, R7, R4, R5, R11);

            //
            //column3  ------------------------------------------------------
            //
            loop_round_macro_en(R7, R4, R5, R6, R12);
        }

        //********************************************************************//
        //********************************************************************//
        //
        //final round
        //
        final_round_macro_en(R9, R10, R11, R12, R4);
        final_round_macro_en(R10, R11, R12, R9, R5);
        final_round_macro_en(R11, R12, R9, R10, R6);
        final_round_macro_en(R12, R9, R10, R11, R7);

        //
        //extended key
        //
        R9  = *extended_key++;
        R10 = *extended_key++;
        R11 = *extended_key++;
        R12 = *extended_key++;

        //
        //STEP 9 - add the final round key and store the memory
        //
        R4 ^= R9;
        R5 ^= R10;
        R6 ^= R11;
        R7 ^= R12;

        //
        //store the output block
        //
        data[0] = R4;
        data[1] = R5;
        data[2] = R6;
        data[3] = R7;

        //
        //increment the data pointer to the next block
        //
        data += 4;
    } //for (jj=0; jj < num_blocks; jj++)

    //
    // Save the last IV in case we need it for a future iteration.
    //
    iv[0] = data[0];
    iv[1] = data[1];
    iv[2] = data[2];
    iv[3] = data[3];
} //function

const uint8_t sbox[16][16] =
{
    {0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76},
    {0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0},
    {0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15},
    {0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75},
    {0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84},
    {0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF},
    {0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8},
    {0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2},
    {0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73},
    {0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB},
    {0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79},
    {0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08},
    {0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A},
    {0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E},
    {0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF},
    {0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16}
};

//*****************************************************************************
// Rotate word
//*****************************************************************************
#define ROTWORD(x) ( ((x) << 8) | ((x) >> 24) )

//*****************************************************************************
//
// Substitue word into sbox
//
//*****************************************************************************
uint32_t SubWord(int word)
{
   uint32_t result;

   result = (int)sbox[(word >> 4) & 0x0000000F][word & 0x0000000F];
   result += (int)sbox[(word >> 12) & 0x0000000F][(word >> 8) & 0x0000000F] << 8;
   result += (int)sbox[(word >> 20) & 0x0000000F][(word >> 16) & 0x0000000F] << 16;
   result += (int)sbox[(word >> 28) & 0x0000000F][(word >> 24) & 0x0000000F] << 24;

   return (result);
}

//*****************************************************************************
//
// Swap Bytes
//
//*****************************************************************************
uint32_t SwapBytes(int word)
{
   uint32_t result;

   result  = (word & 0x000000FF) << 24;
   result |= (word & 0x0000FF00) << 8;
   result |= (word & 0x00FF0000) >> 8;
   result |= (word & 0xFF000000) >> 24;

   return (result);
}

//*****************************************************************************
//
// Key expansion for AES encryption
//
//*****************************************************************************
void KeyExpansion(uint8_t *key, uint32_t *w, int keysize)
{
    //
    // key expansion, pretty much matches pseudo code in aes standard
    //

    int Nb = 4, Nr, Nk, idx;
    uint32_t temp;

    uint32_t Rcon[] =
    {
        0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000, 0x20000000,
        0x40000000, 0x80000000, 0x1b000000, 0x36000000, 0x6c000000, 0xd8000000,
        0xab000000, 0x4d000000, 0x9a000000
    };

    switch (keysize)
    {
       case 128:    //4*32 = 128   11*4 = 44 words = 176 bytes for extended key
            Nr = 10;
            Nk = 4;
            break;
       case 192:    //6*32 = 192   13*4 = 52 words = 208 bytes for extended key
            Nr = 12;
            Nk = 6;
            break;
       case 256:    //8*32 = 256   15*4 = 60 words = 240 bytes for extended key
            Nr = 14;
            Nk = 8;
            break;
       default:
            return;
    }

    //
    // copy the Nk words of the key (just a copy no expand)
    //
    for ( idx = 0; idx < Nk; ++idx )
    {
        w[idx] = ((key[4 * idx]) << 24) | ((key[4 * idx + 1]) << 16) |
                 ((key[4 * idx + 2]) << 8) | ((key[4 * idx + 3]));
    }

    //
    // we expand the key here by the number of rounds + 1, where the block size is 4 32-bit values
    //
    for ( idx = Nk; idx < Nb * (Nr + 1); ++idx )
    {
        temp = w[idx - 1];

        if ((idx % Nk) == 0)
        {
            temp = SubWord(ROTWORD(temp)) ^ Rcon[(idx - 1) / Nk];
        }
        else if (Nk > 6 && (idx % Nk) == 4)
        {
            temp = SubWord(temp);
        }

        w[idx] = w[idx-Nk] ^ temp;
    }

    //
    // swap endian if needed for the first and last blocks
    //
    for (idx = 0; idx < 4; idx++ )
    {
        w[idx] = SwapBytes(w[idx]);
    }

    for (idx =  Nb*Nr; idx <  Nb * (Nr + 1); idx++ )
    {
        w[idx] = SwapBytes(w[idx]);
    }
}

//*****************************************************************************
//
// Key expansion for AES decrytion
//
//*****************************************************************************
void KeyExpansionDecrypt(uint8_t *key, uint32_t *w, int keysize)
{
    int i;
    int Nr;
    uint32_t tmp;
    switch (keysize)
    {
        case 128:   //4*32 = 128   11*4 = 44 words = 176 bytes for extended key
            Nr = 10;
            break;
        case 192:   //6*32 = 192   13*4 = 52 words = 208 bytes for extended key
            Nr = 12;
            break;
        case 256:   //8*32 = 256   15*4 = 60 words = 240 bytes for extended key
            Nr = 14;
            break;
        default:
            return;
    }

    uint32_t extended_key[60];
    KeyExpansion(key, extended_key, keysize);

    //
    //copy the key backwards and add invmix columns
    //
    w[4 * (Nr + 1) - 1 ] = extended_key[0];
    w[4 * (Nr + 1) - 2 ] = extended_key[1];
    w[4 * (Nr + 1) - 3 ] = extended_key[2];
    w[4 * (Nr + 1) - 4 ] = extended_key[3];

    for ( i = 4; i < 4 * (Nr); i++ )
    {
        tmp = extended_key[i];
        w[4 *( Nr + 1) - 1 - i] =
            U0[T5[(tmp >> 24)       ] & 0xff] ^
            U1[T5[(tmp >> 16) & 0xff] & 0xff] ^
            U2[T5[(tmp >> 8)  & 0xff] & 0xff] ^
            U3[T5[(tmp)       & 0xff] & 0xff];
    }

    //
    //add in the last round
    //
    w[3] = extended_key[i];
    w[2] = extended_key[i + 1];
    w[1] = extended_key[i + 2];
    w[0] = extended_key[i + 3];
}

//*****************************************************************************
// big endian final loop
//*****************************************************************************
#define final_round_macro_de(in1, in2, in3, in4, out)       \
  out = (S5[(in1)&0xff]) | (S5[(in2 >> 8) & 0xff] << 8) | (S5[(in3 >> 16) & 0xff] << 16) | (S5[(in4 >> 24) & 0xff] << 24);

//*****************************************************************************
// big endian main loop
//*****************************************************************************
#define loop_round_macro_de(in1, in2, in3, in4, column_out)     \
   column_out ^= (U0[(in1) & 0xff]) ^ (U1[(in2 >> 8) & 0xff]) ^ (U2[(in3 >> 16) & 0xff]) ^ (U3[(in4 >> 24) & 0xff]);

//*****************************************************************************
//
// Decrypt block(s) of data using AES encryption
//    Must use KeyExpansionDecrypt prior to calling this function
//
//*****************************************************************************
void aes_decrypt(uint32_t * data,
                 uint32_t * extended_key_base,
                 int   num_blocks,
                 int    keysize)
{

    int i, jj;
    unsigned int R4, R5, R6, R7;   //buffer_32ptr
    unsigned int R9, R10, R11, R12;
    int loop_count = (( (keysize - 128) >> 6) + 4);
    uint32_t * key;

    for (  jj = 0; jj < num_blocks; jj++ )
    {

        key = extended_key_base;

        R4 =  data[0];  //data[3,2,1,0]
        R5 =  data[1];  //data[7,6,5,4]
        R6 =  data[2];  //data[11,10,9,8]
        R7 =  data[3];  //data[15,14,13,12]

        R12    = *key++;
        R11    = *key++;
        R10    = *key++;
        R9     = *key++;

        //
        //add the round key in
        //
        R9 ^= R4;
        R10 ^= R5;
        R11 ^= R6;
        R12 ^= R7;

        //
        //preround
        //
        R7 = *key++;
        R6 = *key++;
        R5 = *key++;
        R4 = *key++;

        //
        //column0 ------------------------------------------------------
        //
        loop_round_macro_de(R9, R12, R11, R10, R4);

        //
        //column1  ------------------------------------------------------
        //
        loop_round_macro_de(R10, R9, R12, R11, R5);

        //
        //column2  ------------------------------------------------------
        //
        loop_round_macro_de(R11, R10, R9, R12, R6);

        //
        //column3  ------------------------------------------------------
        //
        loop_round_macro_de(R12, R11, R10, R9, R7);

        //
        //main loop
        //
        for ( i = 0; i < loop_count; i++ )
        {
            R12 = *key++;
            R11 = *key++;
            R10 = *key++;
            R9  = *key++;

            //
            //column0 ------------------------------------------------------
            //
            loop_round_macro_de(R4, R7, R6, R5, R9);

            //
            //column1 ------------------------------------------------------
            //
            loop_round_macro_de(R5, R4, R7, R6, R10);

            //
            //column2  ------------------------------------------------------
            //
            loop_round_macro_de(R6, R5, R4, R7, R11);

            //
            //column3  ------------------------------------------------------
            //
            loop_round_macro_de(R7, R6, R5, R4, R12);

            //
            //load extended keys
            //
            R7 = *key++;
            R6 = *key++;
            R5 = *key++;
            R4 = *key++;

            //
            //column0 ------------------------------------------------------
            //
            loop_round_macro_de(R9, R12, R11, R10, R4);

            //
            //column1  ------------------------------------------------------
            //
            loop_round_macro_de(R10, R9, R12, R11, R5);

            //
            //column2  ------------------------------------------------------
            //
            loop_round_macro_de(R11, R10, R9, R12, R6);

            //
            //column3  ------------------------------------------------------
            //
            loop_round_macro_de(R12, R11, R10, R9, R7);
        }

        //********************************************************************//
        //
        //final round
        //
        final_round_macro_de(R4, R7, R6, R5, R9);
        final_round_macro_de(R5, R4, R7, R6, R10);
        final_round_macro_de(R6, R5, R4, R7, R11);
        final_round_macro_de(R7, R6, R5, R4, R12);

        //
        //get the extended key
        //
        R7 = *key++;
        R6 = *key++;
        R5 = *key++;
        R4 = *key++;

        //
        //add the final round key and store the memory
        //
        R4 ^= R9;
        R5 ^= R10;
        R6 ^= R11;
        R7 ^= R12;

        //
        //store the output block
        //
        data[0] = R4;
        data[1] = R5;
        data[2] = R6;
        data[3] = R7;

        //
        //increment the block pointer
        //
        data += 4;
    } //for (jj=0; jj < num_blocks; jj++)
} //aes_decrypt

//*****************************************************************************
//
// Decrypt block(s) of data using AES encryption and cipher block chaining
//    Must use KeyExpansionDecrypt prior to calling this function
//
//*****************************************************************************
void aes_decrypt_cbc(uint32_t * data,
                     uint32_t * extended_key_base,
                     int   num_blocks,
                     int    keysize,
                     uint32_t * iv)
{

    int i, jj;
    unsigned int R4, R5, R6, R7;   //buffer_32ptr
    unsigned int R9, R10, R11, R12;
    int loop_count = (( (keysize - 128) >> 6) + 4);
    uint32_t * key;

    //
    //load up the initialization block if we are doing cbc mode
    //
    uint32_t IV0 = iv[0];
    uint32_t IV1 = iv[1];
    uint32_t IV2 = iv[2];
    uint32_t IV3 = iv[3];

    for (  jj = 0; jj < num_blocks; jj++ )
    {
        key = extended_key_base;

        R4 =  data[0];  //data[3,2,1,0]
        R5 =  data[1];  //data[7,6,5,4]
        R6 =  data[2];  //data[11,10,9,8]
        R7 =  data[3];  //data[15,14,13,12]

        R12    = *key++;
        R11    = *key++;
        R10    = *key++;
        R9     = *key++;

        //
        //add the round key in
        //
        R9 ^= R4;
        R10 ^= R5;
        R11 ^= R6;
        R12 ^= R7;

        //
        //preround
        //
        R7 = *key++;
        R6 = *key++;
        R5 = *key++;
        R4 = *key++;

        //
        //column0 ------------------------------------------------------
        //
        loop_round_macro_de(R9, R12, R11, R10, R4);

        //
        //column1  ------------------------------------------------------
        //
        loop_round_macro_de(R10, R9, R12, R11, R5);

        //
        //column2  ------------------------------------------------------
        //
        loop_round_macro_de(R11, R10, R9, R12, R6);

        //
        //column3  ------------------------------------------------------
        //
        loop_round_macro_de(R12, R11, R10, R9, R7);

        //
        //main loop
        //
        for ( i = 0; i < loop_count; i++ )
        {
            R12 = *key++;
            R11 = *key++;
            R10 = *key++;
            R9  = *key++;

            //
            //column0 ------------------------------------------------------
            //
            loop_round_macro_de(R4, R7, R6, R5, R9);

            //
            //column1 ------------------------------------------------------
            //
            loop_round_macro_de(R5, R4, R7, R6, R10);

            //
            //column2  ------------------------------------------------------
            //
            loop_round_macro_de(R6, R5, R4, R7, R11);

            //
            //column3  ------------------------------------------------------
            //
            loop_round_macro_de(R7, R6, R5, R4, R12);

            //
            //load extended keys
            //
            R7 = *key++;
            R6 = *key++;
            R5 = *key++;
            R4 = *key++;

            //
            //column0 ------------------------------------------------------
            //
            loop_round_macro_de(R9, R12, R11, R10, R4);

            //
            //column1  ------------------------------------------------------
            //
            loop_round_macro_de(R10, R9, R12, R11, R5);

            //
            //column2  ------------------------------------------------------
            //
            loop_round_macro_de(R11, R10, R9, R12, R6);

            //
            //column3  ------------------------------------------------------
            //
            loop_round_macro_de(R12, R11, R10, R9, R7);
        }

        //********************************************************************//
        //
        //final round
        //
        final_round_macro_de(R4, R7, R6, R5, R9);
        final_round_macro_de(R5, R4, R7, R6, R10);
        final_round_macro_de(R6, R5, R4, R7, R11);
        final_round_macro_de(R7, R6, R5, R4, R12);

        //
        //get the extended key
        //
        R7 = *key++;
        R6 = *key++;
        R5 = *key++;
        R4 = *key++;

        //
        //add the final round key and store the memory
        //
        R4 ^= R9;
        R5 ^= R10;
        R6 ^= R11;
        R7 ^= R12;

        R4 ^= IV0;
        R5 ^= IV1;
        R6 ^= IV2;
        R7 ^= IV3;

        IV0 = data[0];
        IV1 = data[1];
        IV2 = data[2];
        IV3 = data[3];

        //
        //store the output block
        //
        data[0] = R4;
        data[1] = R5;
        data[2] = R6;
        data[3] = R7;

        //
        //increment the block pointer
        //
        data += 4;
    } //for (jj=0; jj < num_blocks; jj++)

    //
    //save the initialization block in case the caller needs it again.
    //
    iv[0] = IV0;
    iv[1] = IV1;
    iv[2] = IV2;
    iv[3] = IV3;
} //aes_decrypt_cbc

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

