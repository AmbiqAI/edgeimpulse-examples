//*****************************************************************************
//
//! @file mram_program.c
//!
//! @brief MRAM programming example.
//!
//! Purpose: This example shows how to modify the internal Main MRAM using HAL
//! MRAM helper functions.
//!
//! This example works on the portion of the MRAM at the 1MB boundary.
//!
//! Printing takes place over the ITM at 1M Baud.
//!
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
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

// Define to enable the INFO0 write/read test in Apollo5a **** READ NOTE ****
//
// Note: For Apollo5 RevA INFO0-MRAM is not readable when INFO1 or INFO0 are configured for OTP
// This code should not be enabled unless the part is configured for MRAM for both INFO0 and INFO1
// Apollo5 RevB removes this restriction, with both INFO0-MRAM and INFO0-OTP readable and writable
// at all times.
//#define ENABLE_INFO_TEST_APOLLO5A

// Pick arbitrary address and size at 1M boundary in 1st instance of MRAM to write.
#define ARB_MRAM_ADDRESS    (AM_HAL_MRAM_ADDR + (1024 * 1024))
#define ARB_MRAM_SIZE_WORDS  512

// Offset in INFO0-MRAM to write (picking unused locations at 1K boundary)
#define ARB_INFO_OFFSET 0x400

// Source buffer to write to MRAM from
static uint32_t ui32SourceBuf[ARB_MRAM_SIZE_WORDS];

// Data to write into unused INFO0-MRAM - 5 words
static uint32_t ui32Info0[] = { 0xAAAAAAAA, 0x55555555, 0xA5A5A5A5, 0xDEADBEEF, 0xBEEFDEAD };

// buffer to read back INFO0 into
uint32_t ui32Info0ReadBack[sizeof(ui32Info0) >> 2];


//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32ReturnCode;
    int32_t i32ErrorFlag = 0;
    uint32_t *pui32Src;
    int32_t ix;
    uint32_t *ui32PrgmAddr;
    am_hal_cachectrl_range_t cacheRange;

    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    //
    // Initialize the peripherals for this board.
    //
    am_bsp_low_power_init();

    //
    // Enable printing through the ITM interface.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("MRAM Programming Example\n");

    //
    // Select an arbitrary section address in MRAM.
    //
    ui32PrgmAddr = (uint32_t *)ARB_MRAM_ADDRESS;

    //
    // Fill the section we are going to program programmed.
    //
    am_util_stdio_printf("  ... pre-filling the section to be programmed.\n");
    ui32ReturnCode = am_hal_mram_main_fill(AM_HAL_MRAM_PROGRAM_KEY,
                                              0xA5A5A5A5,
                                              ui32PrgmAddr,
                                              ARB_MRAM_SIZE_WORDS);

    //
    // Check for an error from the HAL.
    //
    if (ui32ReturnCode)
    {
        am_util_stdio_printf("Pre-fill failed at 0x%08x, ui32ReturnCode = 0x%x.\n",
                             ui32PrgmAddr,
                             ui32ReturnCode);
        i32ErrorFlag++;
    }

    //
    // Setup a pattern to write to the MRAM.
    //
    am_util_stdio_printf("  ... programming pattern to MRAM at 1M boundary\n");

    pui32Src = ui32SourceBuf;
    for (ix = 0; ix < ARB_MRAM_SIZE_WORDS; ix++)
    {
        *pui32Src++ = 0x100 + (ix * 4);
    }

    //
    // Program a few words in a section in the main block of instance 1.
    //
    ui32ReturnCode = am_hal_mram_main_program(AM_HAL_MRAM_PROGRAM_KEY,
                                              ui32SourceBuf,
                                              ui32PrgmAddr,
                                              ARB_MRAM_SIZE_WORDS);

    //
    // Check for an error from the HAL.
    //
    if (ui32ReturnCode)
    {
        am_util_stdio_printf("MRAM program failed at 0x%08x, ui32ReturnCode = 0x%x.\n",
                             ui32PrgmAddr,
                             ui32ReturnCode);
        i32ErrorFlag++;
    }

    //
    // MRAM changed so invalidate the cache range we just wrote
    //
    cacheRange.ui32StartAddr = (uint32_t) ui32PrgmAddr;
    cacheRange.ui32Size = ARB_MRAM_SIZE_WORDS * 4;
    am_hal_cachectrl_dcache_invalidate(&cacheRange, false);

    //
    // Check the section just programmed.
    //
    am_util_stdio_printf("  ... verifying the section just programmed.\n");
    for ( ix = 0; ix < ARB_MRAM_SIZE_WORDS; ix++)
    {
        if ( *(uint32_t*)(ui32PrgmAddr + ix) != ui32SourceBuf[ix] )
        {
            am_util_stdio_printf("ERROR: MRAM address 0x%08x did not program properly:\n"
                                 "  Expected value = 0x%08x, programmed value = 0x%08x.\n",
                                 ui32PrgmAddr + ix,
                                 ui32SourceBuf[ix],
                                 *(uint32_t*)(ui32PrgmAddr + (ix * 4)) );
        i32ErrorFlag++;
        }
    }

    //
    // Fill the section just programmed.
    //
    am_util_stdio_printf("  ... filling the section just programmed.\n");
    ui32ReturnCode = am_hal_mram_main_fill(AM_HAL_MRAM_PROGRAM_KEY,
                                              0x00000000,
                                              ui32PrgmAddr,
                                              ARB_MRAM_SIZE_WORDS);

    //
    // Check for an error from the HAL.
    //
    if (ui32ReturnCode)
    {
        am_util_stdio_printf(" MRAM filled section at 0x%08x ui32ReturnCode =  0x%x.\n",
                             ui32PrgmAddr, ui32ReturnCode);
        i32ErrorFlag++;
    }

    //
    // MRAM changed so invalidate the cache range we just wrote
    //
    am_hal_cachectrl_dcache_invalidate(&cacheRange, false);

    //
    // Check that the entire section is filled.
    //
    am_util_stdio_printf("  ... verifying the section just filled.\n");
    for ( ix = 0; ix < ARB_MRAM_SIZE_WORDS; ix++ )
    {
        if ( *(uint32_t*)(ui32PrgmAddr + ix) != 0x00000000 )
        {
            am_util_stdio_printf("ERROR: MRAM address 0x%08x did not ERASE properly:\n"
                                 "  Expected value = 0x00000000, programmed value = 0x%08x.\n",
                                 ui32PrgmAddr + ix,
                                 *(uint32_t*)(ui32PrgmAddr + (ix * 4)) );
            i32ErrorFlag++;
        }
    }

    // **** Read Note before enabling on Apollo5a ****
#if defined(AM_PART_APOLLO5B) || defined(ENABLE_INFO_TEST_APOLLO5A)
    //
    // Program a few words in a INFO0
    //
    am_util_stdio_printf("  ... programming INFO0\n");
    ui32ReturnCode = am_hal_info0_program(AM_HAL_INFO_INFOSPACE_MRAM_INFO0,
                                         AM_HAL_MRAM_INFO_KEY,
                                         ui32Info0,
                                         ARB_INFO_OFFSET >> 2,
                                         sizeof(ui32Info0) >> 2);
    //
    // Check for an error from the HAL.
    //
    if (ui32ReturnCode)
    {
        am_util_stdio_printf("MRAM program Info0"
                             "ui32ReturnCode = 0x%x.\n",
                             ui32ReturnCode);
        i32ErrorFlag++;
    }

    //
    // Check INFO0-MRAM just programmed.
    //
    am_util_stdio_printf("  ... verifying the INFO0 locations programmed.\n");
    ui32ReturnCode = am_hal_info0_read(AM_HAL_INFO_INFOSPACE_MRAM_INFO0,
                                      ARB_INFO_OFFSET >> 2,
                                      sizeof(ui32Info0) >> 2,
                                      ui32Info0ReadBack);
    //
    // Check for an error from the HAL.
    //
    if (ui32ReturnCode)
    {
        am_util_stdio_printf("MRAM program Info0 failed: ui32ReturnCode = 0x%x.\n",
                             ui32ReturnCode);
        i32ErrorFlag++;
    }


    for ( ix = 0; ix < sizeof(ui32Info0) >> 2; ix++ )
    {
        if ( ui32Info0ReadBack[ix] != ui32Info0[ix] )
        {
            am_util_stdio_printf("ERROR: INFO0 offset 0x%04x did not program properly:\n"
                                 "  Expected value = 0x%08x, programmed value = 0x%08x.\n",
                                 (ix * 4),
                                 ui32Info0[ix],
                                 *(uint32_t*)(ui32Info0ReadBack + (ix * 4)) );
            i32ErrorFlag++;
        }
    }

#endif

    //
    // Report success or any failures and exit.
    //
    if (i32ErrorFlag)
    {
        am_util_stdio_printf("ERROR: MRAM Program Write example failure %d\n",
                             i32ErrorFlag);
    }
    else
    {
        am_util_stdio_printf("MRAM Program example completed successfully \n");
    }

    //
    // Verify that ITM is done printing
    //
    am_hal_itm_not_busy();

    //
    // Provide return code back to the system.
    //
    return i32ErrorFlag;
}
