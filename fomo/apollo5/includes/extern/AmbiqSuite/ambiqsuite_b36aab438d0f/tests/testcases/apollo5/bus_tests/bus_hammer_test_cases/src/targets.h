//*****************************************************************************
//
//! @file targets.h
//!
//! @brief Target memory definitions
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef TARGETS_H
#define TARGETS_H

#include <stdint.h>
#include "nema_graphics.h"
#include "nema_cmdlist.h"
#include "patterns.h"
#include "mspi.h"

// Target geometry

#define FB_RESX     64 // corresponds to lines of 256 bytes of RGBA8888
#define FB_RESY     64  // lines transferred per test - 16k
#define TEX_RESX    256 // corresponds to lines of 1024 bytes of RGBA8888
#define TEX_RESY    16  // lines transferred per test - 16k
#define CL_SIZE     16384 // size of command list - 16k
#define DMA_SIZE    16384 // size of DMA buffer - 16k
#define CM55_SIZE   16384 // size of CM55 buffer - 16k

// Target defs

// For SSRAM0, place bank A in the middle of the block since SSRAMPool covers at least that much of the block
// SSRAM1 and 2 align the bank buffers at the beginning of their blocks
#define SSRAM0_BANK_A_START 0x20100000
#define SSRAM0_BANK_B_START 0x20120000
#define SSRAM0_BANK_C_START 0x20140000
#define SSRAM0_BANK_D_START 0x20160000
#define SSRAM1_BANK_A_START 0x20180000
#define SSRAM1_BANK_B_START 0x201A0000
#define SSRAM1_BANK_C_START 0x201C0000
#define SSRAM1_BANK_D_START 0x201E0000
#define SSRAM2_BANK_A_START 0x20280000
#define SSRAM2_BANK_B_START 0x202A0000
#define SSRAM2_BANK_C_START 0x202C0000
#define SSRAM2_BANK_D_START 0x202E0000

#define ROM_START 0x00000000
#define ROM_READ_LENGTH 16384 // how much of the ROM the CPU will read in issue_rom_workload()

typedef enum
{
    SSRAM0_BANK_A = 0,
    SSRAM0_BANK_B,
    SSRAM0_BANK_C,
    SSRAM0_BANK_D,
    SSRAM1_BANK_A,
    SSRAM1_BANK_B,
    SSRAM1_BANK_C,
    SSRAM1_BANK_D,
    SSRAM2_BANK_A,
    SSRAM2_BANK_B,
    SSRAM2_BANK_C,
    SSRAM2_BANK_D,
    PSRAM_MSPI0,
    FLASH_MSPI1,
    MRAM,
    ROM,
    TCM,
    NUM_TARGETS,
    END_TARGETS
} TargetTypes_t;

typedef struct TargetMem_t
{
    uint8_t cl[CL_SIZE];
    uint8_t tex_a[TEX_RESX * TEX_RESY * 4];
    uint8_t tex_b[TEX_RESX * TEX_RESY * 4];
    uint8_t fb[FB_RESX * FB_RESY * 4];
    uint8_t dma[DMA_SIZE];
    uint8_t cm55[CM55_SIZE];
    // structures for GPU and DC buffers
} TargetMem_t;

typedef struct Target_t
{
    img_obj_t gfx_buffer_a;
    img_obj_t gfx_buffer_b;
    img_obj_t fb_buffer;
    TargetMem_t *mem;
    bool readonly; // do not initialize
    nema_cmdlist_t cl;
} Target_t;

typedef struct TargetPermutations_t
{
    TargetTypes_t *cm55_targets;
    TargetTypes_t *disp_targets;
    TargetTypes_t *dma_targets;
    TargetTypes_t *cl_targets;
    TargetTypes_t *tex_a_targets;
    TargetTypes_t *tex_b_targets;
} TargetPermutations_t;

extern Target_t mem_targets[];
extern const TargetMem_t mram_target;
extern TargetMem_t tcm_target;
extern const uint8_t *g_pui8ROM;


#endif //TARGETS_H