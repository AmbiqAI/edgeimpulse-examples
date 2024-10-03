//*****************************************************************************
//
//! @file encrypt_binary.c
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// PC utility for encrypting binary files for use with the bootloader.
//
//*****************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "aes_cipher.h"

//*****************************************************************************
//
// Usage information.
//
//*****************************************************************************
const char usage_string[] =
    "Usage: encrypt_binary INPUT OUTPUT KEYNUM\n"
    "Encrypt a binary for use with the secure bootloader.\n"
    "\n"
    "    INPUT               Input binary filename.\n"
    "    OUTPUT              Output binary filename.\n"
    "    KEYNUM              Key number (0-7)\n"
    "\n"
    "Example:\n"
    "    encrypt_binary input.bin encrypted.bin 1    Encrypt input.bin using key 1.\n"
    "                                                Output to encrypted.bin.\n"
    "\n";



uint8_t extended_key[240];
uint8_t key128[16] =
{
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};
uint8_t iv[16] =
{
    0x28, 0xae, 0xd2, 0xa6, 0x2b, 0x7e, 0x15, 0x16, 0x09, 0xcf, 0x4f, 0x3c, 0xab, 0xf7, 0x15, 0x88,
};

//*****************************************************************************
//
// Filter offsets.
//
//*****************************************************************************
#define HILLCREST_KEYNUM                    0

//*****************************************************************************
//
// Standard Keys
//
//*****************************************************************************
uint32_t StandardKeys[] =
{
    0x7508E567,
    0x13204DFA,
    0x3CFC216A,
    0xD9A21619,
    0xC8E8C04E,
    0x31E0A961,
    0x8F582661,
    0xF867FD31,
    0x059702A5,
    0x16956CED,
    0xAEDF6F67,
    0x7EFCF2AF,
    0xD7CAE9D3,
    0x3FA6C9A5,
    0xB8A086D3,
    0x2725E458,
    0xD91D6F87,
    0xACCCE6A2,
    0xFC1DD66C,
    0x4732D8D2,
    0xEE106B16,
    0x524EFECC,
    0xFDE3533B,
    0x7B3C9432,
    0xC8171446,
    0x35BC732A,
    0x198049D1,
    0x46156F3D,
    0xAFC75BFE,
    0xE2AA5788,
    0xE63B97E2,
    0xCA3D18A6
};

//*****************************************************************************
//
// Filter Block
//
//*****************************************************************************
uint8_t FilterIV[16] =
{
    0x0B, 0x86, 0xBC, 0x03, 0x04, 0x81, 0xE6, 0x5E,
    0x29, 0xD7, 0x9C, 0x02, 0x06, 0x63, 0x0F, 0xF0
};

uint8_t FilterKey[16] =
{
    0xAB, 0xB1, 0xE8, 0x3B, 0x18, 0x25, 0xEE, 0xE6,
    0x76, 0xB9, 0xBE, 0xFB, 0xB5, 0x83, 0x75, 0xBD
};

uint32_t FilterBlock[32] =
{
    0x56343327, 0xF27536D1, 0xBA324340, 0xB2BDE3F0,
    0xCF7026A7, 0x7D9728F9, 0x89DFC81D, 0x717E1D1C,
    0x520FC21F, 0x55BA2AF3, 0x3F3ADA3C, 0xB26EC12D,
    0xD0DE995C, 0x0FB9CFD2, 0x2900E87D, 0x20102E46,
    0x6E10DD1A, 0x43B87EE7, 0xBA371D8C, 0x9A59861A,
    0xAC56CD37, 0xED2DA872, 0x2EB2DD9F, 0x8CDE2011,
    0x6CDC1DCD, 0x777B701A, 0x18D7AE93, 0x8FB7CCFA,
    0x333786BB, 0x2B9635E4, 0x8273DCEF, 0x1A72704D
};

void
encrypt_file(const char *filename, const char *output)
{
    FILE *fp;
    FILE *input;
    int size;
    uint32_t *encrypt_buffer;

    // Open the input file.
    input = fopen(filename, "rb");

    // Figure out how big the input file is.
    fseek(input, 0, SEEK_END);
    size = ftell(input);
    fseek(input, 0, SEEK_SET);
    printf("File Size: %dB\n", size);

    // Make a buffer big enough to hold the entire input file along with enough
    // padding to bring the total length to a multiple of 16 bytes.
    int bufsize = (size % 16) ? ((size - (size % 16)) + 16) : size;
    encrypt_buffer = (uint32_t *) calloc(bufsize, sizeof(uint8_t));
    printf("Buffer Size: %dB\n", bufsize);

    // Read the file's contents into the buffer we just made.
    fread(encrypt_buffer, 1, size, input);

    // We're done with the file, so we can close it now.
    fclose(input);

    // Encrypt the buffer.
    KeyExpansion(key128, (uint32_t *)extended_key, 128);
    aes_encrypt_cbc(encrypt_buffer, (uint32_t *)extended_key,
                    (bufsize / 16), 128, (uint32_t *)iv);

    // Write the whole buffer to an output file.
    fp = fopen(output, "wb");
    fwrite(encrypt_buffer, 1, bufsize, fp);
    fclose(fp);

    free(encrypt_buffer);
}

//*****************************************************************************
//
// Pull a "real" key from the table of standard keys and filters.
//
//*****************************************************************************
void
write_key(uint8_t *pui8Key, uint8_t ui8FilterIndex, const char *filename)
{
    FILE *fp;
    uint32_t i;

    //
    // Copy the key from the standard key list, and filter it through the
    // filter block.
    //
    for ( i = 0; i < 16; i++ )
    {
        pui8Key[i] = ((uint8_t *)StandardKeys)[ui8FilterIndex * 16];
        pui8Key[i] ^= ((uint8_t *)FilterBlock)[ui8FilterIndex * 16];
    }

    //
    // Run a few decrypt operations on the key to scramble it a bit more.
    //
    KeyExpansionDecrypt(FilterKey, (uint32_t *)extended_key, 128);
    aes_decrypt_cbc((uint32_t *)pui8Key, (uint32_t *)extended_key, 1, 128, (uint32_t *)FilterIV);
    aes_decrypt_cbc((uint32_t *)pui8Key, (uint32_t *)extended_key, 1, 128, (uint32_t *)FilterIV);
    aes_decrypt_cbc((uint32_t *)pui8Key, (uint32_t *)extended_key, 1, 128, (uint32_t *)FilterIV);

    //
    // Write the whole buffer to an output file.
    //
    fp = fopen(filename, "wb");
    fwrite(pui8Key, 1, 16, fp);
    fclose(fp);

    return;
}

int
main(int argc, char *argv[])
{
    int status;
    int keynum;

    if ( argc != 4 )
    {
        printf("ERROR: Wrong number of arguments.\n\n");
        printf(usage_string);
        status = 1;
        return status;
    }

    keynum = atoi(argv[3]);

    if ( keynum > 7 || keynum < 0 )
    {
        printf("ERROR: Bad key number.\n\n");
        printf(usage_string);
        status = 1;
        return status;
    }

    write_key(key128, keynum, "hillcrest_key.bin");

    encrypt_file(argv[1], argv[2]);

    status = 0;
    return status;
}
