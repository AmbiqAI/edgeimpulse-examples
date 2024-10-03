//*****************************************************************************
//
//! @file encrypted_boot_generator.c
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
#include <string.h>
#include <ctype.h>

#include "aes_cipher.h"

//*****************************************************************************
//
// Usage information.
//
//*****************************************************************************
const char usage_string[] =
"Usage: encrypted_boot_generator INPUT OUTPUT SYMBOL KEYNUM LOAD_ADDRESS\n"
"Encrypt a binary for use with the secure bootloader.\n"
"\n"
"    INPUT               Input binary filename.\n"
"    OUTPUT              Output binary filename.\n"
"    SYMBOL              Symbol Name for this binary module (all lower case).\n"
"    KEYNUM              Key number (0-7)\n"
"    LOAD_ADDRESS        Load Address fot this binary)\n"
"\n"
"Example:\n"
"    encrypted_boot_generator input encrypted.bin symbol 1 0x70000   Encrypt input.bin using key 1 load at 0x70000.\n"
"                                                              Output to encrypted.bin, encrypted.ota and encrypted.h.\n"
"                                                              #defines and array names will include this symbol in the dot H file.\n"
"\n";



uint32_t extended_key[60]; // 240 bytes
uint8_t key128[16] =
{
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};
uint32_t iv[4] =
{
    0xa6d2ae28, 0x16157e2b, 0x3c4fcf09, 0x8815f7ab
};
uint32_t g_iv[4];

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
uint32_t FilterIV[4] =
{
    0x03BC860B, 0x5EE68104,
    0x029CD729, 0xF00F6306
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

uint32_t g_size;
uint32_t *g_encrypt_buffer;
uint32_t g_crc;
uint32_t g_key_number;
char     g_symbol_upper_case[64];
char     g_symbol_lower_case[64];

uint32_t
compute_crc(void)
{
    uint32_t poly32 = 0x1EDC6F41;
    uint32_t ui32Rem = 0;
    uint32_t ui32B;
    uint8_t *ui8P = (uint8_t *)g_encrypt_buffer;
    int i, j;
    for ( i = 0; i < g_size; i++ )
    {
        ui32B = *ui8P++;
        ui32Rem ^= (ui32B << 24);
        for ( j = 0; j < 8; j++ )
        {
            if ( ui32Rem  & 0x80000000 )
            {
                ui32Rem = (ui32Rem << 1) ^ poly32;
            }
            else
            {
                ui32Rem = (ui32Rem << 1);
            }
        }
    }
    return ui32Rem;
}

void
encrypt_file(const char *filename, const char *output)
{
    FILE *fp;
    FILE *input;
    int size;

    // Open the input file.
    input = fopen(filename, "rb");
    if ( input == NULL )
    {
        exit(1);
    }

    // Figure out how big the input file is.
    fseek(input, 0, SEEK_END);

    size = ftell(input);
    fseek(input, 0, SEEK_SET);
    printf("File Size: %dB\n", size);

    // reopening the file to fix the Windows compilation problem.
    fclose(input);
    fopen(filename, "rb");

    // Make a buffer big enough to hold the entire input file along with enough
    // padding to bring the total length to a multiple of 16 bytes.
    int bufsize = (size % 16) ? ((size - (size % 16)) + 16) : size;
    g_encrypt_buffer = (uint32_t *) calloc(bufsize, sizeof(uint8_t));
    printf("Buffer Size: %dB\n", bufsize);
    g_size = bufsize;

    // Read the file's contents into the buffer we just made.
    fread(g_encrypt_buffer, 1, size, input);

    // We're done with the file, so we can close it now.
    fclose(input);

    // Grab the initial vector while it is still "Initial"
    g_iv[0] = iv[0];
    g_iv[1] = iv[1];
    g_iv[2] = iv[2];
    g_iv[3] = iv[3];

    // Encrypt the buffer.
    KeyExpansion(key128, extended_key, 128);
    aes_encrypt_cbc(g_encrypt_buffer, extended_key,
                    (bufsize / 16), 128, iv);

    // Write the whole buffer to an output file.
    fp = fopen(output, "wb");
    fwrite(g_encrypt_buffer, 1, bufsize, fp);
    fclose(fp);

    {// fixme just doing a little printf debugging
        int i;
        printf("\n dumping a few words from the encryption buffer\n");
        for ( i = 0; i < 32; i++ )
        {
            printf(" 0x%8.8x,", g_encrypt_buffer[i]);
            if ( (i & 7) == 7 )
            {
                printf("\n");
            }
        }
        printf("\n");
    }
    free(g_encrypt_buffer);
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
        // fixme shouldn't there be an "i" in the index?
        // fixme this currently matches the CW secure bootloader code and we have 20,000 parts with that code in it.
        pui8Key[i] = ((uint8_t *)StandardKeys)[ui8FilterIndex * 16];
        pui8Key[i] ^= ((uint8_t *)FilterBlock)[ui8FilterIndex * 16];
#ifdef NOTDEF
        fixme this will show the above bug
        printf("pui8Key[%d] = 0x%2.2x\n", i, pui8Key[i]);
#endif
    }

    //
    // Run a few decrypt operations on the key to scramble it a bit more.
    //
    KeyExpansionDecrypt(FilterKey, extended_key, 128);
    aes_decrypt_cbc((uint32_t *)pui8Key, extended_key, 1, 128, FilterIV);
    aes_decrypt_cbc((uint32_t *)pui8Key, extended_key, 1, 128, FilterIV);
    aes_decrypt_cbc((uint32_t *)pui8Key, extended_key, 1, 128, FilterIV);

    //
    // Write the whole buffer to an output file.
    //
    fp = fopen(filename, "wb");
#ifdef NOTDEF
    fwrite(pui8Key, 1, 16, fp);
#endif
    fclose(fp);

    return;
}

//*****************************************************************************
//
//*****************************************************************************
void
write_ota_file(const char *filename, uint32_t load_address, uint32_t key_number)
{
    FILE *fp;
    uint32_t ui32Header[9];

    // Write the whole buffer to an output file.
    fp = fopen(filename, "wb");

    // write the OTA header
    ui32Header[0] = 1;  // mark it as an encrypted binary
    ui32Header[1] = load_address;
    ui32Header[2] = g_size;
    ui32Header[3] = g_crc;
    ui32Header[4] = key_number;
    ui32Header[5] = g_iv[0];
    ui32Header[6] = g_iv[1];
    ui32Header[7] = g_iv[2];
    ui32Header[8] = g_iv[3];
    fwrite(ui32Header, 1, sizeof(ui32Header), fp);

    // now add the encrypted bytes
    fwrite(g_encrypt_buffer, 1, g_size, fp);

    // we're done here
    fclose(fp);
}

//*****************************************************************************
//
//*****************************************************************************
void
write_header_file(const char *filename, uint32_t load_address, uint32_t key_number)
{

    int  i;

    FILE *ofp;
    uint8_t *pui8P;

    // Write the whole buffer to an output file.
    ofp = fopen(filename, "w");

    fprintf(ofp, "//*****************************************************************************\n");
    fprintf(ofp, "//\n");
    fprintf(ofp, "//! @file %s\n", filename);
    fprintf(ofp, "//! @brief Encrypted binary image from encrypt_boot_generator.\n");
    fprintf(ofp, "//\n");
    fprintf(ofp, "//*****************************************************************************\n");

    fprintf(ofp, "\n#ifndef %s_H\n", g_symbol_upper_case);
    fprintf(ofp, "#define %s_H\n", g_symbol_upper_case);

    fprintf(ofp, "//*****************************************************************************\n");
    fprintf(ofp, "//\n");
    fprintf(ofp, "// Image Characteristics\n");
    fprintf(ofp, "//\n");
    fprintf(ofp, "//*****************************************************************************\n");
    fprintf(ofp, "#define %s_SIZE              %d\n", g_symbol_upper_case, g_size);
    fprintf(ofp, "#define %s_LINK_ADDRESS      0x%8.8x\n", g_symbol_upper_case, load_address);
    fprintf(ofp, "#define %s_CRC               0x%8.8X\n", g_symbol_upper_case, g_crc);
    fprintf(ofp, "#define %s_KEY_NUMBER        %d\n", g_symbol_upper_case, key_number);


    pui8P = (uint8_t *) &g_iv[0];
    fprintf(ofp, "//*****************************************************************************\n");
    fprintf(ofp, "//\n");
    fprintf(ofp, "// Encryption Information\n");
    fprintf(ofp, "//\n");
    fprintf(ofp, "//*****************************************************************************\n");
    fprintf(ofp, "const uint8_t g_pui8_%s_IV[16]= \n{\n    ", g_symbol_lower_case);
    for ( i = 0; i < 16; i++ )
    {
        fprintf(ofp, " 0x%2.2X", *pui8P++);
        if ( ((i&0x7) == 7) && (i != 15) )
        {
            fprintf(ofp, ",\n    ");
        }
        else if ( ((i&0x7) == 7) && (i == (15)) )
        {
            fprintf(ofp, "\n};\n");
        }
        else
        {
            fprintf(ofp, ",");
        }
    }



    pui8P = (uint8_t *)g_encrypt_buffer;
    fprintf(ofp, "//*****************************************************************************\n");
    fprintf(ofp, "//\n");
    fprintf(ofp, "// Boot Image\n");
    fprintf(ofp, "//\n");
    fprintf(ofp, "//*****************************************************************************\n");
    fprintf(ofp, "const uint8_t g_pui8_%s[%d]= \n", g_symbol_lower_case, g_size);
    fprintf(ofp, "{\n    ");
    for (i = 0; i < g_size; i++ )
    {
        fprintf(ofp, " 0x%2.2X", *pui8P++);
        if ( ((i & 0x7) == 7)  &&  (i != (g_size - 1)) )
        {
            fprintf(ofp, ",\n    ");
        }
        else if ( (i == (g_size - 1)) )
        {
            fprintf(ofp, "\n};\n");
        }
        else
        {
            fprintf(ofp, ",");
        }
    }

    fprintf(ofp, "\n\n#endif // %s_H\n", g_symbol_upper_case);

}



//*****************************************************************************
//*****************************************************************************
int
main(int argc, char *argv[])
{
    int status;
    int keynum;
    uint32_t load_address;
    char full_file_name[1024];
    int i;

    if ( argc != 6 )
    {
        printf("ERROR: Wrong number of arguments.\n\n");
        printf(usage_string);
        status = 1;
        return status;
    }

    strcpy(g_symbol_lower_case, argv[3]);
    strcpy(g_symbol_upper_case, argv[3]);
    for ( i = 0; i < strlen(g_symbol_upper_case); i++ )
    {
        g_symbol_upper_case[i] = toupper(g_symbol_upper_case[i]);
    }
    printf("lower_case <%s> upper_case<%s>\n", g_symbol_lower_case, g_symbol_upper_case);

    keynum = atoi(argv[4]);

    if ( keynum > 7 || keynum < 0 )
    {
        printf("ERROR: Bad key number.\n\n");
        printf(usage_string);
        status = 1;
        return status;
    }

    load_address = strtoul(argv[5], NULL, 16);
    if ( load_address > 0x7FFFF )
    {
        printf("ERROR: Bad load address.\n\n");
        printf(usage_string);
        status = 1;
        return status;
    }

#if 1   // Debug
    printf("load_address = 0x%8.8x\n", load_address);
#emdof

    write_key(key128, keynum, "current_key.bin");

    strcpy(full_file_name, argv[2]);
    strcat(full_file_name, ".bin");
    encrypt_file(argv[1], full_file_name);

    g_crc = compute_crc();

#if 1   // Debug
    printf("CRC = 0x%8.8x\n", g_crc);
#endif

    strcpy(full_file_name, argv[2]);
    strcat(full_file_name, ".ota");
    write_ota_file(full_file_name, load_address, keynum);

    strcpy(full_file_name, argv[2]);
    strcat(full_file_name, ".h");
    write_header_file(full_file_name, load_address, keynum);


    status = 0;
    return status;
}
