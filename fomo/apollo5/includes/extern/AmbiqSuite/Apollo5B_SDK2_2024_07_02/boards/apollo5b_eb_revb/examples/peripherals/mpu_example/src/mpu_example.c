//*****************************************************************************
//
//! @file mpu_example.c
//!
//! @brief A simple example on how to configure the ARMv8.1 MPU
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

//
// sMPUCfgTest: Typical region settings
//
am_hal_mpu_region_config_t sMPUCfgTest[6] =
{
    {
        .ui32RegionNumber = 0,
        .ui32BaseAddress = 0x00400000U,
        .eShareable = NON_SHARE,
        .eAccessPermission = RO_NONPRIV,
        .bExecuteNever = false,
        .ui32LimitAddress = 0x005FFFFFU,
        .ui32AttrIndex = 1,
        .bEnable = true
    }, // NVM0

    {
        .ui32RegionNumber = 1,
        .ui32BaseAddress = 0x00600000U,
        .eShareable = NON_SHARE,
        .eAccessPermission = RO_NONPRIV,
        .bExecuteNever = false,
        .ui32LimitAddress = 0x007FFFFFU,
        .ui32AttrIndex = 1,
        .bEnable = true
    }, // NVM1

    {
        .ui32RegionNumber = 2,
        .ui32BaseAddress = 0x20080000U,
        .eShareable = NON_SHARE,
        .eAccessPermission = RW_NONPRIV,
        .bExecuteNever = true,
        .ui32LimitAddress = 0x2017FFFFU,
        .ui32AttrIndex = 2,
        .bEnable = true
    }, // SSRAM0

    {
        .ui32RegionNumber = 3,
        .ui32BaseAddress = 0x20180000U,
        .eShareable = NON_SHARE,
        .eAccessPermission = RW_NONPRIV,
        .bExecuteNever = true,
        .ui32LimitAddress = 0x2027FFFFU,
        .ui32AttrIndex = 2,
        .bEnable = true
    }, // SSRAM1

    {
        .ui32RegionNumber = 4,
        .ui32BaseAddress = 0x20280000U,
        .eShareable = NON_SHARE,
        .eAccessPermission = RW_NONPRIV,
        .bExecuteNever = true,
        .ui32LimitAddress = 0x2037FFFFU,
        .ui32AttrIndex = 2,
        .bEnable = true
    }, // SSRAM2

    {
        .ui32RegionNumber = 5,
        .ui32BaseAddress = 0x60000000U,
        .eShareable = NON_SHARE,
        .eAccessPermission = RW_NONPRIV,
        .bExecuteNever = false,
        .ui32LimitAddress = 0x6FFFFFFFU,
        .ui32AttrIndex = 2,
        .bEnable = true
    } // XIPMM0
};

//
// sMPUAttr: Typical MPU attributes.
//
am_hal_mpu_attr_t sMPUAttr[3] =
{
    {
        .ui8AttrIndex = 0, // Index 0 - Non-Cacheable region.
        .bNormalMem = true,
        .sOuterAttr = {.bNonTransient = true, .bWriteBack = false, .bReadAllocate = false, .bWriteAllocate = false},
        .sInnerAttr = {.bNonTransient = true, .bWriteBack = false, .bReadAllocate = false, .bWriteAllocate = false},
        .eDeviceAttr = 0
    },
    {
        .ui8AttrIndex = 1, // Index 1 - Read-Allocation.  Used for code segments.
        .bNormalMem = true,
        .sOuterAttr = {.bNonTransient = false, .bWriteBack = false, .bReadAllocate = true, .bWriteAllocate = false},
        .sInnerAttr = {.bNonTransient = false, .bWriteBack = false, .bReadAllocate = true, .bWriteAllocate = false},
        .eDeviceAttr = 0
    },
    {
        .ui8AttrIndex = 2, // Index 2 - Read-Allocation and Write-Allocation.  Used for data segments.
        .bNormalMem = true,
        .sOuterAttr = {.bNonTransient = false, .bWriteBack = false, .bReadAllocate = true, .bWriteAllocate = true},
        .sInnerAttr = {.bNonTransient = false, .bWriteBack = false, .bReadAllocate = true, .bWriteAllocate = true},
        .eDeviceAttr = 0
    },
};

int main (void)
{
    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    //
    // Initialize the printf interface for ITM output
    //
    if (am_bsp_debug_printf_enable())
    {
        //
        // Cannot print - so no point proceeding
        //
        while(1);
    }

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("MPU Example\n\n");
    am_bsp_debug_printf_disable();

    //
    // Set up the attributes.
    //
    am_hal_mpu_attr_configure(sMPUAttr, sizeof(sMPUAttr) / sizeof(am_hal_mpu_attr_t));

    //
    // Clear the MPU regions.
    //
    am_hal_mpu_region_clear();

    //
    // Set up the regions.
    //
    am_hal_mpu_region_configure(sMPUCfgTest, sizeof(sMPUCfgTest) / sizeof(am_hal_mpu_region_config_t));

    //
    // MPU enable
    //
    am_hal_mpu_enable(true, true);

    //
    // Insert delay for demo.
    //
    am_util_delay_ms(100);

    //
    // Use cmse_TT instruction to get the MPU region and accessibility information.
    //
    cmse_address_info_t addressInfo = cmse_TT((unsigned char*)0x20080000U);
    am_util_stdio_printf("addressInfo : 0x%08x\n", addressInfo);
    am_util_stdio_printf("mpu_region : %d\n", addressInfo.flags.mpu_region);
    am_util_stdio_printf("mpu_region_valid : %d\n", addressInfo.flags.mpu_region_valid);
    am_util_stdio_printf("readwrite_ok : %d\n", addressInfo.flags.readwrite_ok);
    am_util_stdio_printf("read_ok : %d\n", addressInfo.flags.read_ok);

    //
    // MPU disable
    //
    am_hal_mpu_disable();

    //
    // Loop forever while sleeping.
    //
    while (1)
    {
    }
}
