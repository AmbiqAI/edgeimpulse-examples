/* TSI 2023.xmo */
/*******************************************************************************
 * Copyright (c) 2023 Think Silicon Single Member PC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this header file and/or associated documentation files to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Materials, and to permit persons to whom the Materials are furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * NEMAGFX API. THE UNMODIFIED, NORMATIVE VERSIONS OF THINK-SILICON NEMAGFX
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT:
 *   https://think-silicon.com/products/software/nemagfx-api
 *
 *  The software is provided 'as is', without warranty of any kind, express or
 *  implied, including but not limited to the warranties of merchantability,
 *  fitness for a particular purpose and noninfringement. In no event shall
 *  Think Silicon Single Member PC be liable for any claim, damages or other
 *  liability, whether in an action of contract, tort or otherwise, arising
 *  from, out of or in connection with the software or the use or other dealings
 *  in the software.
 ******************************************************************************/

#ifndef NEMA_SYS_DEFS_H__
#define NEMA_SYS_DEFS_H__

//Start of User Definitions
//-------------------------

//use multiple memory pools (implemented in nema_hal.c)
//#define NEMA_MULTI_MEM_POOLS

//if NEMA_MULTI_MEM_POOLS is defined, use NEMA_MULTI_MEM_POOLS_CNT pools
//must be equal or less than 4
#ifndef NEMA_MULTI_MEM_POOLS_CNT
#define NEMA_MULTI_MEM_POOLS_CNT	2
#endif

#define NEMA_ENABLE_BREAKPOINTS

#ifndef DONT_USE_NEMADC
#define DONT_USE_NEMADC
#endif

#ifndef NEMA_MEM_POOL_FB
#define NEMA_MEM_POOL_FB     0
#endif

#ifndef NEMA_MEM_POOL_ASSETS
#define NEMA_MEM_POOL_ASSETS 0
#endif

#ifndef NEMA_MEM_POOL_CL
#define NEMA_MEM_POOL_CL 0
#endif

#ifndef IS_BIG_ENDIAN
#define IS_BIG_ENDIAN  (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#endif

// Platform Dependent
//-------------------------
#define NEMA_BASEADDR    0x30000000U // NemaGPU Regs Addr
#define NEMADC_BASEADDR  0x020000000 // NemaDC  Regs Addr

#define VMEM_BASEADDR    0x10000000U // Video (graphics) memory base address (physical)
#define VMEM_SIZE        0x00F00000U // Video (graphics) memory size

#ifdef NEMA_MULTI_MEM_POOLS
#define VMEM1_BASEADDR   0x60000000U
#define VMEM1_SIZE       0x00F00000U
// #define VMEM2_BASEADDR   0x32000000U
// #define VMEM2_SIZE       (15*1024*1024)
// #define VMEM3_BASEADDR   0x33000000U
// #define VMEM3_SIZE       (15*1024*1024)
#endif
//End of User Definitions
//-------------------------
#include <stdint.h>

#define TLS_VAR

#include <sys/types.h>		//NULL
#include <stdbool.h>
#include "assist.h"

#endif
