/*******************************************************************************
 * Copyright (c) 2021 Think Silicon S.A.
 *
   Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this header file and/or associated documentation files to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Materials, and to permit persons to whom the Materials are furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * NEMAGUI API.
 *
 *  The software is provided 'as is', without warranty of any kind, express or
 *  implied, including but not limited to the warranties of merchantability,
 *  fitness for a particular purpose and noninfringement. In no event shall
 *  Think Silicon S.A. be liable for any claim, damages or other liability, whether
 *  in an action of contract, tort or otherwise, arising from, out of or in
 *  connection with the software or the use or other dealings in the software.
 ******************************************************************************/

#ifndef NG_UTILS_H__
#define NG_UTILS_H__

#include "nema_core.h"
#include "nema_utils.h"

#define SAFE_CAST(x, srcType, dstType) \
   (((union {srcType a; dstType b;}){.a=x}).b)

#ifndef CLAMP
    #define CLAMP(x, low, high) ( (x) < (low) ? (low) : ( (x) > (high) ? (high) : (x) ) )
#endif

void float2str(char *str, int size, float f, int precision);
void int2str(char *str, int size, int val);
void concatenate_strings(char *str1, char *str2);
int  round_up(int numToRound, int multiple);
void append_trailing_zeros(char *str, int len, int zeros_count);

static nema_buffer_t create_bo_from_pointer(const void *ptr, size_t size) {
    nema_buffer_t bo;

#ifdef UNIFIED_MEMORY
    bo.base_virt = (void *)ptr;
    bo.base_phys = (uintptr_t)ptr;
    bo.size      = size;
#else
    bo = nema_buffer_create(size);
    (void)nema_buffer_map(&bo);
    nema_memcpy(bo.base_virt, ptr, size);
#endif

    return bo;
}

#define NG_LOAD_ARRAY(array) create_bo_from_pointer((void *)(array), sizeof(array))

#endif //NG_UTILS_H_
