//*****************************************************************************
//
//! @file aes_driver_cbc.c
//aes_driver_cbc.c written by Kevin Coopman
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
#include <stdint.h>
#include <string.h>
#include "aes_cipher.h"



#define INPUT_SIZE (4*1024)
uint32_t input_buffer_a[INPUT_SIZE];
uint32_t input_buffer_b[INPUT_SIZE];

//***************************************************************//
//***************************************************************//
//***************************************************************//
int main(int argc, char* argv[])
{

    int i, j;
    uint8_t extended_key[240];
    uint8_t key128[16] =
    {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    };
    uint8_t iv[16] =
    {
        0x28, 0xae, 0xd2, 0xa6, 0x2b, 0x7e, 0x15, 0x16,
        0x09, 0xcf, 0x4f, 0x3c, 0xab, 0xf7, 0x15, 0x88,
    };
    FILE *input;
    FILE *output;
    input  = fopen(argv[1], "rb");

    //
    //NOTE that the buffer input size has to be a multiple of 16 bytes
    //if it is not, just pad the end with zeros
    //
    int size = fread(input_buffer_a, INPUT_SIZE, 1, input);

    //
    //let make a copy so we can check the answer against it
    //
    memcpy(input_buffer_b, input_buffer_a, INPUT_SIZE);

    //
    // 128 Case **********************************************************************************
    //create the extended key
    //
    KeyExpansion(key128, (uint32_t *)extended_key, 128);
    aes_encrypt_cbc((uint32_t *)input_buffer_a,
                    (uint32_t *)extended_key,
                    (INPUT_SIZE / 16),
                    128,
                    (uint32_t *)iv);

    KeyExpansionDecrypt(key128, (uint32_t *)extended_key, 128);
    aes_decrypt_cbc((uint32_t *)input_buffer_a,
                    (uint32_t *)extended_key,
                    (INPUT_SIZE / 16),
                    128,
                    (uint32_t *)iv);


    int wrong = memcmp((unsigned char *)input_buffer_a, (unsigned char *)input_buffer_b, INPUT_SIZE);
    if ( wrong == 0 )
    {
        printf("CORRECT ANSWER, encryption/decryption matches the input file\n");
    }
    else
    {
        printf("WRONG ANSWER\n");
    }

    fclose(input);

    return wrong;
}
