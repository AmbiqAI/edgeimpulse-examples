//*****************************************************************************
//
//! @file aes_decrypt.c
//aes_decrypt.c for AES written by Kevin Coopman
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdio.h>
#include "aes_cipher.h"
#include "aes_decrypt_tables.h"


void KeyExpansionDecrypt(uint8_t *key, uint32_t *w, int keysize)
{
    int i;
    int Nr;
    uint32_t tmp;
    switch (keysize)
    {
        case 128: Nr = 10;  break;     //4*32 = 128   11*4 = 44 words = 176 bytes for extended key
        case 192: Nr = 12;  break;     //6*32 = 192   13*4 = 52 words = 208 bytes for extended key
        case 256: Nr = 14;  break;     //8*32 = 256   15*4 = 60 words = 240 bytes for extended key
        default: return;
    }

    uint32_t extended_key[60];
    KeyExpansion(key, extended_key, keysize);

    //
    // Copy the key backwards and add invmix columns
    //
    w[4 * (Nr + 1) - 1 ] = extended_key[0];
    w[4 * (Nr + 1) - 2 ] = extended_key[1];
    w[4 * (Nr + 1) - 3 ] = extended_key[2];
    w[4 * (Nr + 1) - 4 ] = extended_key[3];

    for (i = 4; i < 4 * (Nr); i++)
    {
        tmp = extended_key[i];
        w[4 * (Nr + 1) - 1 - i] =
            U0[T5[(tmp >> 24)       ] & 0xff] ^
            U1[T5[(tmp >> 16) & 0xff] & 0xff] ^
            U2[T5[(tmp >> 8)  & 0xff] & 0xff] ^
            U3[T5[(tmp)       & 0xff] & 0xff];
    }

    //
    // add in the last round
    //
    w[3] = extended_key[i    ];
    w[2] = extended_key[i + 1];
    w[1] = extended_key[i + 2];
    w[0] = extended_key[i + 3];
}


#define final_round_macro(in1, in2, in3, in4, out)          \
  out = (S5[(in1) & 0xff]) | (S5[(in2 >> 8) & 0xff] << 8) | (S5[(in3 >> 16) & 0xff] << 16) | (S5[(in4 >> 24) & 0xff] << 24);

//big endian main loop
#define loop_round_marco(in1, in2, in3, in4, column_out)    \
   column_out ^= (U0[(in1) & 0xff]) ^ (U1[(in2 >> 8) & 0xff]) ^ (U2[(in3 >> 16) & 0xff]) ^ (U3[(in4 >> 24) & 0xff]);


//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
#ifndef AES_CBC_MODE
void aes_decrypt(uint32_t *data,
                 uint32_t *extended_key_base,
                 int   num_blocks,
                 int   keysize)
#else
void aes_decrypt_cbc(uint32_t *data,
                     uint32_t *extended_key_base,
                     int   num_blocks,
                     int   keysize,
                     uint32_t *iv)
#endif
{
    int i, jj;
    unsigned int R4, R5, R6, R7;   //buffer_32ptr
    unsigned int R9, R10, R11, R12;
    int loop_count = (( (keysize - 128) >> 6) + 4);
    uint32_t * key;

    //
    //load up the initialization block if we are doing cbc mode
    //
#ifdef AES_CBC_MODE
    uint32_t IV0 = iv[0];
    uint32_t IV1 = iv[1];
    uint32_t IV2 = iv[2];
    uint32_t IV3 = iv[3];
#endif

    for ( jj = 0; jj < num_blocks; jj++ )
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
        // add the round key in
        //
        R9 ^= R4;
        R10 ^= R5;
        R11 ^= R6;
        R12 ^= R7;

        //
        // preround
        //
        R7 = *key++;
        R6 = *key++;
        R5 = *key++;
        R4 = *key++;

        //
        //column0 ------------------------------------------------------
        //
        loop_round_marco(R9, R12, R11, R10, R4);

        //
        //column1  ------------------------------------------------------
        //
        loop_round_marco(R10, R9, R12, R11, R5);

        //
        //column2  ------------------------------------------------------
        //
        loop_round_marco(R11, R10, R9, R12, R6);

        //
        //column3  ------------------------------------------------------
        //
        loop_round_marco(R12, R11, R10, R9, R7);

        //
        //main loop
        //
        for (i = 0; i < loop_count; i++)
        {
            R12 = *key++;
            R11 = *key++;
            R10 = *key++;
            R9  = *key++;

            //
            //column0 ------------------------------------------------------
            //
            loop_round_marco(R4, R7, R6, R5, R9);

            //
            //column1 ------------------------------------------------------
            //
            loop_round_marco(R5, R4, R7, R6, R10);

            //
            //column2  ------------------------------------------------------
            //
            loop_round_marco(R6, R5, R4, R7, R11);

            //
            //column3  ------------------------------------------------------
            //
            loop_round_marco(R7, R6, R5, R4, R12);

            //
            // load extended keys
            //
            R7 = *key++;
            R6 = *key++;
            R5 = *key++;
            R4 = *key++;

            //
            //column0 ------------------------------------------------------
            //
            loop_round_marco(R9, R12, R11, R10, R4);

            //
            //column1  ------------------------------------------------------
            //
            loop_round_marco(R10, R9, R12, R11, R5);

            //
            //column2  ------------------------------------------------------
            //
            loop_round_marco(R11, R10, R9, R12, R6);

            //
            //column3  ------------------------------------------------------
            //
            loop_round_marco(R12, R11, R10, R9, R7);
        }

        //********************************************************************//

        //
        //final round
        //
        final_round_macro(R4, R7, R6, R5, R9);
        final_round_macro(R5, R4, R7, R6, R10);
        final_round_macro(R6, R5, R4, R7, R11);
        final_round_macro(R7, R6, R5, R4, R12);

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

#ifdef AES_CBC_MODE
        R4 ^= IV0;
        R5 ^= IV1;
        R6 ^= IV2;
        R7 ^= IV3;

        IV0 = data[0];
        IV1 = data[1];
        IV2 = data[2];
        IV3 = data[3];
#endif

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

    } //for (jj = 0; jj < num_blocks; jj++)

#ifdef AES_CBC_MODE
    //
    //save the initialization block in case the caller needs it again.
    //
    iv[0] = IV0;
    iv[1] = IV1;
    iv[2] = IV2;
    iv[3] = IV3;
#endif

} //function




