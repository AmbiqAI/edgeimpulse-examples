//*****************************************************************************
//
// file am_memory_map.h
//
// brief Memory map include file.
//
// This file is generated by "generate_link_script.py".
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_MEMORY_MAP_H
#define AM_MEMORY_MAP_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Memory block locations.
//
//*****************************************************************************
#define AM_MEM_MCU_ITCM                0x00000000
#define AM_MEM_MCU_MRAM                0x00410000
#define AM_MEM_MCU_TCM                 0x20000000
#define AM_MEM_SHARED_SRAM             0x20080000
#define AM_MEM_STACK                   0x2007D000
#define AM_MEM_HEAP                    0x2007C000

//*****************************************************************************
//
// Memory block sizes (in bytes)
//
//*****************************************************************************
#define AM_MEM_MCU_ITCM_SIZE           0x00040000
#define AM_MEM_MCU_MRAM_SIZE           0x003F0000
#define AM_MEM_MCU_TCM_SIZE            0x0007C000
#define AM_MEM_SHARED_SRAM_SIZE        0x00300000
#define AM_MEM_STACK_SIZE              0x00003000
#define AM_MEM_HEAP_SIZE               0x00001000

#ifdef __cplusplus
}
#endif

#endif // AM_MEMORY_MAP_H
