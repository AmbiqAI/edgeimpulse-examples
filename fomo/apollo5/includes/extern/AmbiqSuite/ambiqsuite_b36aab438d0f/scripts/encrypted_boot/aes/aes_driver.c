//*****************************************************************************
//
//! @file aes_driver.c
//aes_driver.c written by Kevin Coopman
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
#include "aes_cipher.h"

#define NUM_BLOCKS 4


unsigned char block_AES128[NUM_BLOCKS * 16] =
{
    0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
    0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34
};

unsigned char answer_AES128[NUM_BLOCKS * 16] =
{
    0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb,
    0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32
};

unsigned char block_AES192[NUM_BLOCKS * 16] =
{
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};

unsigned char answer_AES192[NUM_BLOCKS * 16] =
{
    0xdd, 0xa9, 0x7c, 0xa4, 0x86, 0x4c, 0xdf, 0xe0,
    0x6e, 0xaf, 0x70, 0xa0, 0xec, 0x0d, 0x71, 0x91
};

unsigned char block_AES256[] =
{
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};

unsigned char block_AES256_decrypt[] =
{
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};


unsigned char answer_AES256[] =
{
    0x8e, 0xa2, 0xb7, 0xca, 0x51, 0x67, 0x45, 0xbf,
    0xea, 0xfc, 0x49, 0x90, 0x4b, 0x49, 0x60, 0x89
};


//***************************************************************//
//***************************************************************//
//***************************************************************//
int main()
{
    int i, j;
    int fail = 0;
    uint8_t extended_key[240];

    uint8_t key128[16] =
    {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    };
    uint8_t key192[24] =
    {
        0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17\
    };

    uint8_t key256[32] =
    {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };

    //
    //page 37 example out of the standard for AES
    // uint8_t key128_check[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    // uint8_t key128_block[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
    //


    //
    // 128 Case **********************************************************************************
    //create the extended key
    //
    KeyExpansion(key128, (uint32_t *)extended_key, 128);
    aes_encrypt((uint32_t *)block_AES128,
                (uint32_t *)extended_key,
                1,          //test 1 block = 16 bytes
                128);

    KeyExpansionDecrypt(key128, (uint32_t *)extended_key, 128);
    aes_decrypt((uint32_t *)block_AES128,
                (uint32_t *)extended_key,
                1,          //test 1 block = 16 bytes
                128);

    KeyExpansion(key128, (uint32_t *)extended_key, 128);
    aes_encrypt((uint32_t *)block_AES128,
                (uint32_t *)extended_key,
                1,          //test 1 block = 16 bytes
                128);


    //
    //check to make sure the answer is correct
    //
#ifndef ARM
    fail = 0;
#endif
    for ( i = 0; i < 16; i++ )
    {
        if ( block_AES128[i] != answer_AES128[i] )
        {
            fail = 1;
        }
    }

#ifndef ARM
    //
    //report the answer
    //
    if ( fail != 1 )
    {
        printf("AES 128 PASS\n");
    }
    else
    {
        printf("AES 128 FAIL\n");
    }
#endif

    //
    // 196 Case **********************************************************************************
    //create the extended key
    //
    KeyExpansion(key192, (uint32_t *)extended_key, 192);
    aes_encrypt((uint32_t *)block_AES192,
                (uint32_t *)extended_key,
                1,
                192);

    KeyExpansionDecrypt(key192, (uint32_t *)extended_key, 192);
    aes_decrypt((uint32_t *)block_AES192,
                (uint32_t *)extended_key,
                1,
                192);

    KeyExpansion(key192, (uint32_t *)extended_key, 192);
    aes_encrypt((uint32_t *)block_AES192,
                (uint32_t *)extended_key,
                1,
                192);

    //
    //check to make sure the answer is correct
    //
#ifndef ARM
    fail = 0;
#endif
    for ( i = 0; i < 16; i++ )
    {
        if ( block_AES192[i] != answer_AES192[i] )
        {
            fail = 1;
        }
    }
#ifndef ARM
    //
    //report the answer
    //
    if ( fail != 1 )
    {
      printf("AES 192 PASS\n");
    }
    else
    {
      printf("AES 192 FAIL\n");
    }
#endif

    //
    // 256 Case **********************************************************************************
    //create the extended key
    //
    KeyExpansion(key256, (uint32_t *)extended_key, 256);
    aes_encrypt((uint32_t *)block_AES256,
                (uint32_t *)extended_key,
                4,          //test 4 blocks = 4*16 = 64 bytes, a block is 16 bytes in AES
                256);

    KeyExpansionDecrypt(key256, (uint32_t *)extended_key, 256);
    aes_decrypt((uint32_t *)block_AES256,
                (uint32_t *)extended_key,
                4,          //test 4 blocks = 4*16 = 64 bytes, a block is 16 bytes in AES
                256);

    KeyExpansion(key256, (uint32_t *)extended_key, 256);
    aes_encrypt((uint32_t *)block_AES256,
                (uint32_t *)extended_key,
                4,          //test 4 blocks = 4*16 = 64 bytes, a block is 16 bytes in AES
                256);

#ifndef ARM
    fail = 0;
#endif

    //
    //check to make sure the answer is correct
    //
    for ( j = 0; j < NUM_BLOCKS; j++ )
    {
        for ( i = 0; i < 16; i++ )
        {
            if ( block_AES256[i + j * 16] != answer_AES256[i] )
            {
                fail = 1;
            }
        }
    }

#ifndef ARM
    //
    //report the answer
    //
    if (fail != 1)
    {
        printf("AES 256 PASS\n");
    }
    else
    {
        printf("AES 256 FAIL\n");
    }
#endif

    return fail;
}

