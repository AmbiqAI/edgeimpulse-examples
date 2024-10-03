//*****************************************************************************
//
//! @file expand_key.c
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

const uint8_t sbox[16][16] =
{
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};

//
// rotate word
//
#define ROTWORD(x) ( ((x) << 8) | ((x) >> 24) )


uint32_t SubWord(int word)
{
    uint32_t result;

    result = (int)sbox[(word >> 4) & 0x0000000F][word & 0x0000000F];
    result += (int)sbox[(word >> 12) & 0x0000000F][(word >> 8) & 0x0000000F] << 8;
    result += (int)sbox[(word >> 20) & 0x0000000F][(word >> 16) & 0x0000000F] << 16;
    result += (int)sbox[(word >> 28) & 0x0000000F][(word >> 24) & 0x0000000F] << 24;
    return(result);
}

uint32_t SwapBytes(int word)
{
    uint32_t result;

    result = (word & 0x000000FF) << 24;
    result |= (word & 0x0000FF00) << 8;
    result |= (word & 0x00FF0000) >> 8;
    result |= (word & 0xFF000000) >> 24;
    return(result);
}

//
// key expansion, pretty much matches pseudo code in aes standard
//
void KeyExpansion(uint8_t *key, uint32_t *w, int keysize)
{
    int Nb = 4, Nr, Nk, idx;
    uint32_t temp;

    uint32_t Rcon[] =
    {
        0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000, 0x20000000,
        0x40000000, 0x80000000, 0x1b000000, 0x36000000, 0x6c000000, 0xd8000000,
        0xab000000, 0x4d000000, 0x9a000000
    };

    switch ( keysize )
    {
        case 128:
            Nr = 10;
            Nk = 4;
            break;     //4*32 = 128   11*4 = 44 words = 176 bytes for extended key
        case 192:
            Nr = 12;
            Nk = 6;
            break;     //6*32 = 192   13*4 = 52 words = 208 bytes for extended key
        case 256:
            Nr = 14;
            Nk = 8;
            break;     //8*32 = 256   15*4 = 60 words = 240 bytes for extended key
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
        w[idx] = w[idx - Nk] ^ temp;
    }

    //
    // swap endian if needed for the first and last blocks
    //
    for ( idx = 0; idx < 4; idx++ )
    {
        w[idx] = SwapBytes(w[idx]);
    }

    for (idx =  Nb * Nr; idx <  Nb * (Nr + 1); idx++)
    {
        w[idx] = SwapBytes(w[idx]);
    }
}

#ifdef TEST_KEY

//
// keys and extended keys
//

//-----------------------------------------------------------------------------------------------------
uint8_t key128[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};

unsigned char extended_key2_appendix_b[176] =
{
    43,   126,    21,    22,    40,   174,   210,   166,   171,   247,    21,   136,     9,   207,    79,    60,
    23,   254,   250,   160,   177,    44,    84,   136,    57,    57,   163,    35,     5,   118,   108,    42,
   242,   149,   194,   242,    67,   185,   150,   122,   122,   128,    53,    89,   127,   246,    89,   115,
   125,    71,   128,    61,    62,   254,    22,    71,    68,   126,    35,    30,    59,   136,   122,   109,
    65,   165,    68,   239,   127,    91,    82,   168,    59,    37,   113,   182,     0,   173,    11,   219,
   248,   198,   209,   212,   135,   157,   131,   124,   188,   184,   242,   202,   188,    21,   249,    17,
   122,   163,   136,   109,   253,    62,    11,    17,    65,   134,   249,   219,   253,   147,     0,   202,
    14,   247,    84,    78,   243,   201,    95,    95,   178,    79,   166,   132,    79,   220,   166,    78,
    33,   115,   210,   234,   210,   186,   141,   181,    96,   245,    43,    49,    47,    41,   141,   127,
   243,   102,   119,   172,    33,   220,   250,    25,    65,    41,   209,    40,   110,     0,    92,    87,
   208,    20,   249,   168,   201,   238,    37,   137,   225,    63,    12,   200,   182,    99,    12,   166
};

//-----------------------------------------------------------------------------------------------------

uint8_t key192[24] =
{
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17
};


unsigned char  extended_key_appendix_c2[208]  =
{
    0x0,  0x1,  0x2,  0x3,  0x4,  0x5,  0x6,  0x7,  0x8,  0x9,  0xa,  0xb,  0xc,  0xd,  0xe,  0xf,
    0x13, 0x12, 0x11, 0x10, 0x17, 0x16, 0x15, 0x14, 0xf9, 0xf2, 0x46, 0x58, 0xfe, 0xf4, 0x43, 0x5c,
    0xf5, 0xfe, 0x4a, 0x54, 0xfa, 0xf0, 0x47, 0x58, 0xe9, 0xe2, 0x56, 0x48, 0xfe, 0xf4, 0x43, 0x5c,
    0xb3, 0x49, 0xf9, 0x40, 0x4d, 0xbd, 0xba, 0x1c, 0xb8, 0x43, 0xf0, 0x48, 0x42, 0xb3, 0xb7, 0x10,
    0xab, 0x51, 0xe1, 0x58, 0x55, 0xa5, 0xa2, 0x4,  0x41, 0xb5, 0xff, 0x7e, 0xc,  0x8,  0x45, 0x62,
    0xb4, 0x4b, 0xb5, 0x2a, 0xf6, 0xf8, 0x2,  0x3a, 0x5d, 0xa9, 0xe3, 0x62, 0x8,  0xc,  0x41, 0x66,
    0x72, 0x85, 0x1,  0xf5, 0x7e, 0x8d, 0x44, 0x97, 0xca, 0xc6, 0xf1, 0xbd, 0x3c, 0x3e, 0xf3, 0x87,
    0x61, 0x97, 0x10, 0xe5, 0x69, 0x9b, 0x51, 0x83, 0x9e, 0x7c, 0x15, 0x34, 0xe0, 0xf1, 0x51, 0xa3,
    0x2a, 0x37, 0xa0, 0x1e, 0x16, 0x9,  0x53, 0x99, 0x77, 0x9e, 0x43, 0x7c, 0x1e, 0x5,  0x12, 0xff,
    0x88, 0xe,  0x7e, 0xdd, 0x68, 0xff, 0x2f, 0x7e, 0x42, 0xc8, 0x8f, 0x60, 0x54, 0xc1, 0xdc, 0xf9,
    0x23, 0x5f, 0x9f, 0x85, 0x3d, 0x5a, 0x8d, 0x7a, 0x52, 0x29, 0xc0, 0xc0, 0x3a, 0xd6, 0xef, 0xbe,
    0x78, 0x1e, 0x60, 0xde, 0x2c, 0xdf, 0xbc, 0x27, 0xf,  0x80, 0x23, 0xa2, 0x32, 0xda, 0xae, 0xd8,
    0xa4, 0x97, 0xa,  0x33, 0x1a, 0x78, 0xdc, 0x9,  0xc4, 0x18, 0xc2, 0x71, 0xe3, 0xa4, 0x1d, 0x5d
};


//
//-----------------------------------------------------------------------------------------------------
//
uint8_t key256[32] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

uint8_t  extended_key_appendix_c3[240] =
{
      0,     1,     2,     3,     4,     5,     6,     7,     8,     9,    10,    11,
     12,    13,    14,    15,    19,    18,    17,    16,    23,    22,    21,    20,
     27,    26,    25,    24,    31,    30,    29,    28,   159,   194,   115,   165,
    152,   196,   118,   161,   147,   206,   127,   169,   156,   192,   114,   165,
    205,   168,    81,    22,   218,   190,    68,     2,   193,   164,    93,    26,
    222,   186,    64,     6,   240,   223,   135,   174,   104,    27,   241,    15,
    251,   213,   142,   166,   103,    21,   252,     3,    72,   241,   225,   109,
    146,    79,   165,   111,    83,   235,   248,   117,   141,    81,   184,   115,
    127,   130,    86,   198,    23,   153,   167,   201,   236,    76,    41,   111,
    139,    89,   213,   108,   117,    58,   226,    61,   231,   117,    71,    82,
    180,   158,   191,    39,    57,   207,     7,    84,    95,   144,   220,    11,
     72,     9,   123,   194,   164,    69,    82,   173,    47,    28,   135,   193,
     96,   166,   245,    69,   135,   211,   178,    23,    51,    77,    13,    48,
     10,   130,    10,   100,    28,   247,   207,   124,    84,   254,   180,   190,
    240,   187,   230,    19,   223,   167,    97,   210,   254,   250,    26,   240,
    121,    41,   168,   231,    74,   100,   165,   215,    64,   230,   175,   179,
    113,   254,    65,    37,    37,     0,   245,   155,   213,   187,    19,   136,
     10,    28,   114,    90,   153,   102,    90,    78,   224,    79,   242,   169,
    170,    43,    87,   126,   234,   205,   248,   205,    36,   252,   121,   204,
    191,     9,   121,   233,    55,    26,   194,    60,   109,   104,   222,    54
};

//
//-----------------------------------------------------------------------------------------------------
//

int main()
{
    uint32_t extended_key[60];
    uint8_t * ptr = (uint8_t *) extended_key;
    int i;
    int FAIL = 0;

    //
    //check for 128-bit key ---------------------------------------------------------------
    //
    FAIL = 0;
    KeyExpansion(key128, extended_key, 128);

    for ( i = 0; i < 176; i++ )
    {
        if ( ptr[i] != extended_key2_appendix_b[i] )
        {
            FAIL = 1;
        }
    }

    if (!FAIL)
    {
        printf("PASS 128-bit extended key\n");
    }
    else
    {
        printf("FAIL 128-bit extended key\n");
    }

    //
    //check for 128-bit key ---------------------------------------------------------------
    //
    FAIL = 0;
    KeyExpansion(key192, extended_key, 192);

    for ( i = 0; i < 208; i++ )
    {
        if ( ptr[i] != extended_key_appendix_c2[i] )
        {
            FAIL = 1;
            printf("%i, %i %i \n", i , ptr[i], extended_key_appendix_c2[i]);
        }
    }

    if (!FAIL)
    {
        printf("PASS 196-bit extended key\n");
    }
    else
    {
        printf("FAIL 196-bit extended key\n");
    }

    //
    //check for 256-bit key ---------------------------------------------------------------
    //
    FAIL = 0;
    KeyExpansion(key256, extended_key, 256);

    for ( i = 0; i < 240; i++ )
    {
        if ( ptr[i] != extended_key_appendix_c3[i] )
        {
            FAIL = 1;
        }
    }

    if ( !FAIL )
    {
        printf("PASS 256-bit extended key\n");
    }
    else
    {
        printf("FAIL 256-bit extended key\n");
    }

    return 0;
}

#endif
