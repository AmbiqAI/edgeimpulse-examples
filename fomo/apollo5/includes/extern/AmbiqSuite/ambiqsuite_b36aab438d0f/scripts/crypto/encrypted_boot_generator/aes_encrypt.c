//*****************************************************************************
//
//! @file aes_encrypt.c
//aes_encrypt.c for AES written by Kevin Coopman
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
#include "aes_encrypt_tables.h"

#define pre_round_macro(in1, in2, in3, in4, out)\
  out ^=  T1[(in1) & 0xff] ^ T2[(in2 >> 8) & 0xff] ^ T3[(in3 >> 16) & 0xff] ^ T4[(in4 >> 24) & 0xff];

#define loop_round_marco(in1, in2, in3, in4, column_out)\
  column_out ^=  T1[(in1 >> 24) & 0xff] ^ T2[(in2 >> 16) & 0xff] ^ T3[(in3 >> 8) & 0xff] ^ T4[in4 & 0xff];

#define final_round_macro(in1, in2, in3, in4, out)\
  out =  s_box[(in1 >> 24) & 0xff] ^ (s_box[(in2 >> 16) & 0xff] << 8) ^ (s_box[(in3 >> 8) & 0xff] << 16) ^ (s_box[in4 & 0xff] << 24);

//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
#ifndef AES_CBC_MODE
void aes_encrypt(uint32_t * data,
                 uint32_t * extended_key_base,
                 int   num_blocks,
                 int   keysize)
#else
void aes_encrypt_cbc(uint32_t * data,
                     uint32_t * extended_key_base,
                     int   num_blocks,
                     int   keysize,
                     uint32_t * iv)
#endif
{
    int i, jj;
    unsigned int R4, R5, R6, R7;
    unsigned int R9, R10, R11, R12;
    uint32_t * extended_key;
    int loop_count = ( ((keysize - 128) >> 6) + 4);

    //
    //load the initialization vector up for the first XOR
    //
#ifdef AES_CBC_MODE
    R4 = iv[0];
    R5 = iv[1];
    R6 = iv[2];
    R7 = iv[3];
#endif

    //
    //instead of always allocation memory for variables, this loop
    // was put in place to go through all the blocks. The pointers at the end of the
    // loop increment to the next block and also the extended key pointer is reset to the
    // beginning
    //
    for (  jj = 0; jj < num_blocks; jj++)
    {
        extended_key = extended_key_base;

#ifndef AES_CBC_MODE
        R4 =  data[0];  //data[3,2,1,0]
        R5 =  data[1];  //data[7,6,5,4]
        R6 =  data[2];  //data[11,10,9,8]
        R7 =  data[3];  //data[15,14,13,12]
#else
        //CBC chaning and XOR in the previous block
        R4 ^= data[0];
        R5 ^= data[1];
        R6 ^= data[2];
        R7 ^= data[3];
#endif

        //get extended key
        R9      = *extended_key++;
        R10     = *extended_key++;
        R11     = *extended_key++;
        R12     = *extended_key++;

        //add in rounded key
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
        pre_round_macro(R4, R5, R6, R7, R9);
        pre_round_macro(R5, R6, R7, R4, R10);
        pre_round_macro(R6, R7, R4, R5, R11);
        pre_round_macro(R7, R4, R5, R6, R12);

        for ( i = 0; i < loop_count; i++ )
        {
            R4 = *extended_key++;
            R5 = *extended_key++;
            R6 = *extended_key++;
            R7 = *extended_key++;

            //
            //column0 ------------------------------------------------------
            //
            loop_round_marco(R9, R10, R11, R12, R4);

            //
            //column1  ------------------------------------------------------
            //
            loop_round_marco(R10, R11, R12, R9, R5);

            //
            //column2  ------------------------------------------------------
            //
            loop_round_marco(R11, R12, R9, R10, R6);

            //
            //column3  ------------------------------------------------------
            //
            loop_round_marco(R12, R9, R10, R11, R7);

            //**************************************************************//
            //second loop unrolled

            R9  = *extended_key++;
            R10 = *extended_key++;
            R11 = *extended_key++;
            R12 = *extended_key++;

            //
            //column0 ------------------------------------------------------
            //
            loop_round_marco(R4, R5, R6, R7, R9);

            //
            //column1 ------------------------------------------------------
            //
            loop_round_marco(R5, R6, R7, R4, R10);

            //
            //column2  ------------------------------------------------------
            //
            loop_round_marco(R6, R7, R4, R5, R11);

            //
            //column3  ------------------------------------------------------
            //
            loop_round_marco(R7, R4, R5, R6, R12);
        }

        //********************************************************************//
        //********************************************************************//
        //
        //final round
        //
        final_round_macro(R9, R10, R11, R12, R4);
        final_round_macro(R10, R11, R12, R9, R5);
        final_round_macro(R11, R12, R9, R10, R6);
        final_round_macro(R12, R9, R10, R11, R7);

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

    } // for (jj = 0; jj < num_blocks; jj++)

#ifdef AES_CBC_MODE
    //
    // Save the last IV in case we need it for a future iteration.
    //
    iv[0] = data[0];
    iv[1] = data[1];
    iv[2] = data[2];
    iv[3] = data[3];
#endif

} //function




