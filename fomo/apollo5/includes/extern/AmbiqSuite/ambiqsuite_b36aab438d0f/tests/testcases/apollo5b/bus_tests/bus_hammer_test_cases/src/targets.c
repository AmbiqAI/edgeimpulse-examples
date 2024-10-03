//*****************************************************************************
//
//! @file targets.c
//!
//! @brief Target memory definitions
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "targets.h"
#include "patterns.h"

const uint8_t* g_pui8ROM = ROM_START;

// Prepopulated read-only target that will be located in MRAM when the executable
// is flashed to the DUT
const TargetMem_t mram_target __attribute__ ((aligned(65536))) =
{
    {0},
    PATTERN_RGBA8888,
    PATTERN_RGBA8888,
    PATTERN_RGBA8888,
    PATTERN_MEM,
    PATTERN_MEM
};

TargetMem_t tcm_target __attribute__ ((aligned(65536)));

// Definition of memories that can be used as targets in the bus hammer test
Target_t mem_targets[NUM_TARGETS] =
{
    [SSRAM0_BANK_A] = {
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM0_BANK_A_START))->tex_a), (uintptr_t)(((TargetMem_t*)(SSRAM0_BANK_A_START))->tex_a)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM0_BANK_A_START))->tex_b), (uintptr_t)(((TargetMem_t*)(SSRAM0_BANK_A_START))->tex_b)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{FB_RESX * FB_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM0_BANK_A_START))->fb), (uintptr_t)(((TargetMem_t*)(SSRAM0_BANK_A_START))->fb)},
            FB_RESX, FB_RESY, FB_RESX * 4, 0, NEMA_RGBA8888, 0},
        (TargetMem_t*)(SSRAM0_BANK_A_START),
        false
    },
    [SSRAM0_BANK_B] = {
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM0_BANK_B_START))->tex_a), (uintptr_t)(((TargetMem_t*)(SSRAM0_BANK_B_START))->tex_a)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM0_BANK_B_START))->tex_b), (uintptr_t)(((TargetMem_t*)(SSRAM0_BANK_B_START))->tex_b)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{FB_RESX * FB_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM0_BANK_B_START))->fb), (uintptr_t)(((TargetMem_t*)(SSRAM0_BANK_B_START))->fb)},
            FB_RESX, FB_RESY, FB_RESX * 4, 0, NEMA_RGBA8888, 0},
        (TargetMem_t*)(SSRAM0_BANK_B_START),
        false
    },
    [SSRAM0_BANK_C] = {
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM0_BANK_C_START))->tex_a), (uintptr_t)(((TargetMem_t*)(SSRAM0_BANK_C_START))->tex_a)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM0_BANK_C_START))->tex_b), (uintptr_t)(((TargetMem_t*)(SSRAM0_BANK_C_START))->tex_b)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{FB_RESX * FB_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM0_BANK_C_START))->fb), (uintptr_t)(((TargetMem_t*)(SSRAM0_BANK_C_START))->fb)},
            FB_RESX, FB_RESY, FB_RESX * 4, 0, NEMA_RGBA8888, 0},
        (TargetMem_t*)(SSRAM0_BANK_C_START),
        false
    },
    [SSRAM0_BANK_D] = {
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM0_BANK_D_START))->tex_a), (uintptr_t)(((TargetMem_t*)(SSRAM0_BANK_D_START))->tex_a)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM0_BANK_D_START))->tex_b), (uintptr_t)(((TargetMem_t*)(SSRAM0_BANK_D_START))->tex_b)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{FB_RESX * FB_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM0_BANK_D_START))->fb), (uintptr_t)(((TargetMem_t*)(SSRAM0_BANK_D_START))->fb)},
            FB_RESX, FB_RESY, FB_RESX * 4, 0, NEMA_RGBA8888, 0},
        (TargetMem_t*)(SSRAM0_BANK_D_START),
        false
    },
    [SSRAM1_BANK_A] = {
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM1_BANK_A_START))->tex_a), (uintptr_t)(((TargetMem_t*)(SSRAM1_BANK_A_START))->tex_a)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM1_BANK_A_START))->tex_b), (uintptr_t)(((TargetMem_t*)(SSRAM1_BANK_A_START))->tex_b)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{FB_RESX * FB_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM1_BANK_A_START))->fb), (uintptr_t)(((TargetMem_t*)(SSRAM1_BANK_A_START))->fb)},
            FB_RESX, FB_RESY, FB_RESX * 4, 0, NEMA_RGBA8888, 0},
        (TargetMem_t*)(SSRAM1_BANK_A_START),
        false
    },
    [SSRAM1_BANK_B] = {
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM1_BANK_B_START))->tex_a), (uintptr_t)(((TargetMem_t*)(SSRAM1_BANK_B_START))->tex_a)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM1_BANK_B_START))->tex_b), (uintptr_t)(((TargetMem_t*)(SSRAM1_BANK_B_START))->tex_b)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{FB_RESX * FB_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM1_BANK_B_START))->fb), (uintptr_t)(((TargetMem_t*)(SSRAM1_BANK_B_START))->fb)},
            FB_RESX, FB_RESY, FB_RESX * 4, 0, NEMA_RGBA8888, 0},
        (TargetMem_t*)(SSRAM1_BANK_B_START),
        false
    },
    [SSRAM1_BANK_C] = {
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM1_BANK_C_START))->tex_a), (uintptr_t)(((TargetMem_t*)(SSRAM1_BANK_C_START))->tex_a)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM1_BANK_C_START))->tex_b), (uintptr_t)(((TargetMem_t*)(SSRAM1_BANK_C_START))->tex_b)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{FB_RESX * FB_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM1_BANK_C_START))->fb), (uintptr_t)(((TargetMem_t*)(SSRAM1_BANK_C_START))->fb)},
            FB_RESX, FB_RESY, FB_RESX * 4, 0, NEMA_RGBA8888, 0},
        (TargetMem_t*)(SSRAM1_BANK_C_START),
        false
    },
    [SSRAM1_BANK_D] = {
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM1_BANK_D_START))->tex_a), (uintptr_t)(((TargetMem_t*)(SSRAM1_BANK_D_START))->tex_a)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM1_BANK_D_START))->tex_b), (uintptr_t)(((TargetMem_t*)(SSRAM1_BANK_D_START))->tex_b)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{FB_RESX * FB_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM1_BANK_D_START))->fb), (uintptr_t)(((TargetMem_t*)(SSRAM1_BANK_D_START))->fb)},
            FB_RESX, FB_RESY, FB_RESX * 4, 0, NEMA_RGBA8888, 0},
        (TargetMem_t*)(SSRAM1_BANK_D_START),
        false
    },
    [SSRAM2_BANK_A] = {
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM2_BANK_A_START))->tex_a), (uintptr_t)(((TargetMem_t*)(SSRAM2_BANK_A_START))->tex_a)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM2_BANK_A_START))->tex_b), (uintptr_t)(((TargetMem_t*)(SSRAM2_BANK_A_START))->tex_b)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{FB_RESX * FB_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM2_BANK_A_START))->fb), (uintptr_t)(((TargetMem_t*)(SSRAM2_BANK_A_START))->fb)},
            FB_RESX, FB_RESY, FB_RESX * 4, 0, NEMA_RGBA8888, 0},
        (TargetMem_t*)(SSRAM2_BANK_A_START),
        false
    },
    [SSRAM2_BANK_B] = {
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM2_BANK_B_START))->tex_a), (uintptr_t)(((TargetMem_t*)(SSRAM2_BANK_B_START))->tex_a)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM2_BANK_B_START))->tex_b), (uintptr_t)(((TargetMem_t*)(SSRAM2_BANK_B_START))->tex_b)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{FB_RESX * FB_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM2_BANK_B_START))->fb), (uintptr_t)(((TargetMem_t*)(SSRAM2_BANK_B_START))->fb)},
            FB_RESX, FB_RESY, FB_RESX * 4, 0, NEMA_RGBA8888, 0},
        (TargetMem_t*)(SSRAM2_BANK_B_START),
        false
    },
    [SSRAM2_BANK_C] = {
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM2_BANK_C_START))->tex_a), (uintptr_t)(((TargetMem_t*)(SSRAM2_BANK_C_START))->tex_a)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM2_BANK_C_START))->tex_b), (uintptr_t)(((TargetMem_t*)(SSRAM2_BANK_C_START))->tex_b)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{FB_RESX * FB_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM2_BANK_C_START))->fb), (uintptr_t)(((TargetMem_t*)(SSRAM2_BANK_C_START))->fb)},
            FB_RESX, FB_RESY, FB_RESX * 4, 0, NEMA_RGBA8888, 0},
        (TargetMem_t*)(SSRAM2_BANK_C_START),
        false
    },
    [SSRAM2_BANK_D] = {
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM2_BANK_D_START))->tex_a), (uintptr_t)(((TargetMem_t*)(SSRAM2_BANK_D_START))->tex_a)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM2_BANK_D_START))->tex_b), (uintptr_t)(((TargetMem_t*)(SSRAM2_BANK_D_START))->tex_b)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{FB_RESX * FB_RESY * 4, 0, (void*)(((TargetMem_t*)(SSRAM2_BANK_D_START))->fb), (uintptr_t)(((TargetMem_t*)(SSRAM2_BANK_D_START))->fb)},
            FB_RESX, FB_RESY, FB_RESX * 4, 0, NEMA_RGBA8888, 0},
        (TargetMem_t*)(SSRAM2_BANK_D_START),
        false
    },
    [PSRAM_MSPI0] = {
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(MSPI0_START))->tex_a), (uintptr_t)(((TargetMem_t*)(MSPI0_START))->tex_a)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(MSPI0_START))->tex_b), (uintptr_t)(((TargetMem_t*)(MSPI0_START))->tex_b)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{FB_RESX * FB_RESY * 4, 0, (void*)(((TargetMem_t*)(MSPI0_START))->fb), (uintptr_t)(((TargetMem_t*)(MSPI0_START))->fb)},
            FB_RESX, FB_RESY, FB_RESX * 4, 0, NEMA_RGBA8888, 0},
        (TargetMem_t*)(MSPI0_START),
        false
    },
    [FLASH_MSPI1] = {
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(MSPI1_START))->tex_a), (uintptr_t)(((TargetMem_t*)(MSPI1_START))->tex_a)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(((TargetMem_t*)(MSPI1_START))->tex_b), (uintptr_t)(((TargetMem_t*)(MSPI1_START))->tex_b)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{FB_RESX * FB_RESY * 4, 0, (void*)(((TargetMem_t*)(MSPI1_START))->fb), (uintptr_t)(((TargetMem_t*)(MSPI1_START))->fb)},
            FB_RESX, FB_RESY, FB_RESX * 4, 0, NEMA_RGBA8888, 0},
        (TargetMem_t*)(MSPI1_START),
        true
    },
    [MRAM] = {
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(mram_target.tex_a), (uintptr_t)(mram_target.tex_a)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(mram_target.tex_b), (uintptr_t)(mram_target.tex_b)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{FB_RESX * FB_RESY * 4, 0, (void*)(mram_target.fb), (uintptr_t)(mram_target.fb)},
            FB_RESX, FB_RESY, FB_RESX * 4, 0, NEMA_RGBA8888, 0},
        (TargetMem_t*)&mram_target,
        true
    },
    [ROM] = {
        // all ROM targets except CM55 are invalid, but fill the struct out anyway
        {{TEX_RESX * TEX_RESY * 4, 0, NULL, 0},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{TEX_RESX * TEX_RESY * 4, 0, NULL, 0},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{FB_RESX * FB_RESY * 4, 0, NULL, 0},
            FB_RESX, FB_RESY, FB_RESX * 4, 0, NEMA_RGBA8888, 0},
        (TargetMem_t*)(ROM_START),
        true
    },
    [TCM] = {
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(tcm_target.tex_a), (uintptr_t)(tcm_target.tex_a)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{TEX_RESX * TEX_RESY * 4, 0, (void*)(tcm_target.tex_b), (uintptr_t)(tcm_target.tex_b)},
            TEX_RESX, TEX_RESY, TEX_RESX * 4, 0, NEMA_RGBA8888, 0},
        {{FB_RESX * FB_RESY * 4, 0, (void*)(tcm_target.fb), (uintptr_t)(tcm_target.fb)},
            FB_RESX, FB_RESY, FB_RESX * 4, 0, NEMA_RGBA8888, 0},
        &tcm_target,
        false
    }
};
