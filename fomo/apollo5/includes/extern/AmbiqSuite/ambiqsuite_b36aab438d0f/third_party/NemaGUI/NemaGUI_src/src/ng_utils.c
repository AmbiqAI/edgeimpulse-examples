// -----------------------------------------------------------------------------
// Copyright (c) 2021 Think Silicon S.A.
// Think Silicon S.A. Confidential Proprietary
// -----------------------------------------------------------------------------
//     All Rights reserved - Unpublished -rights reserved under
//         the Copyright laws of the European Union
//
//  This file includes the Confidential information of Think Silicon S.A.
//  The receiver of this Confidential Information shall not disclose
//  it to any third party and shall protect its confidentiality by
//  using the same degree of care, but not less than a reasonable
//  degree of care, as the receiver uses to protect receiver's own
//  Confidential Information. The entire notice must be reproduced on all
//  authorised copies and copies may only be made to the extent permitted
//  by a licensing agreement from Think Silicon S.A..
//
//  The software is provided 'as is', without warranty of any kind, express or
//  implied, including but not limited to the warranties of merchantability,
//  fitness for a particular purpose and noninfringement. In no event shall
//  Think Silicon S.A. be liable for any claim, damages or other liability, whether
//  in an action of contract, tort or otherwise, arising from, out of or in
//  connection with the software or the use or other dealings in the software.
//
//
//                    Think Silicon S.A.
//                    http://www.think-silicon.com
//                    Patras Science Park
//                    Rion Achaias 26504
//                    Greece
// -----------------------------------------------------------------------------

#include "ng_utils.h"
#include "nema_core.h"

//Convert a float to string, with "precision" decimal digits
void float2str(char *str, int size, float f, int precision) {
    //precision must be [0, 4]
    //str size should be enough

    int tmp_prec = precision;
    int multiplier = 1;
    float f_round_bias = f < 0.f ? -0.5f : 0.5f;

    while(tmp_prec > 0) {
        multiplier *= 10;
        --tmp_prec;
    }

    int tmp = f*(float)multiplier+f_round_bias;
    if (tmp < 0) tmp = -tmp;

    str[size-1] = '\0';
    int i = size-2;

    if (precision > 0) {
        do {
            str[i--] = '0' + (tmp%10);
            tmp /= 10;
            --precision;
        } while(precision > 0);
        str[i--] = '.';
    }

    do {
        str[i--] = '0' + (tmp%10);
        tmp /= 10;
    } while (tmp != 0);

    if (i == size-2)
        str[i--] = '0';
    if (f < 0.f)
        str[i--] = '-';

    ++i;
    if (i > 0) {
        int j = 0;
        while(i <= size-1) {
            str[j] = str[i];
            ++j;
            ++i;
        }
    }
}

void int2str(char *str, int size, int val) {
    //str size should be enough
    str[size-1] = '\0';
    int i = size-2;

    do {
        str[i--] = '0' + (val%10);
        val /= 10;
    } while (val != 0);

    ++i;
    if (i > 0) {
        int j = 0;
        while(i <= size-1) {
            str[j] = str[i];
            ++j;
            ++i;
        }
    }
}

//concatenate strings str1 and str2 to str1
void concatenate_strings(char *str1, char *str2) {
    int i,j;
    i = 0;
    while ( str1[i] != '\0' ) {
        i++;
    }

    for ( j=0; str2[j]!='\0'; ++j, ++i ) {
       str1[i] = str2[j];
    }
}

void append_trailing_zeros(char *str, int len, int zeros_count) {

    char text[30];
    int  used_chars = 0;
    
    for ( int i = 0; i < zeros_count; ++i) {
        text[i] = '0';
        used_chars++;
    }

    for ( int i = zeros_count; str[i-zeros_count] != '\0'; ++i ) {
        text[i] = str[i-zeros_count];
        used_chars++;
    }

    for ( int i = 0; i < used_chars; ++i ) {
        str[i] = text[i];
    }
}

// Round up the integer number "numToRound" to the closer multiple of "multiple"
int round_up(int numToRound, int multiple) {
    if ( multiple == 0 ) {
        return numToRound;
    }

    int remainder = numToRound % multiple;
    if ( remainder == 0 ) {
        return numToRound;
    }

    return numToRound + multiple - remainder;
}
#ifndef CREATE_BO_FROM_POINTER
#define CREATE_BO_FROM_POINTER
nema_buffer_t create_bo_from_pointer(const void *ptr, size_t size) {
    nema_buffer_t bo;

#ifdef UNIFIED_MEMORY
    bo.base_virt = (void *)ptr;
    bo.base_phys = (uintptr_t)ptr;
    bo.size      = size;
#else
    #ifdef ASSET_LOCATE_IN_PSRAM
        bo = nema_psram_buffer_create(size);
    #else
        bo = nema_buffer_create(size);
    #endif
    (void)nema_buffer_map(&bo);
    nema_memcpy(bo.base_virt, ptr, size);
    return bo;
#endif
}
#endif