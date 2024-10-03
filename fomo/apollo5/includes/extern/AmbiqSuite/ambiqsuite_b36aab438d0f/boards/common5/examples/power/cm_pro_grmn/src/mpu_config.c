//*****************************************************************************
//
//! @file mpu_config.c
//!
//! @brief Configure the ARMv8.1 MPU
//!
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2023, Ambiq Micro, Inc.
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
// This is part of revision stable-aef737a494 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "mpu_config.h"

//
// sMPUCfgTest1: Set SSRAM0 and XIPMM0 to "Read-Allocation and Write-Allocation".
//
am_hal_mpu_region_config_t sMPUCfgTest1[2] = { // Read-Allocation and Write-Allocation.
                                                 {.ui32RegionNumber = 0,
                                                  .ui32BaseAddress = 0x20080000U,
                                                  .eShareable = NON_SHARE,
                                                  .eAccessPermission = RW_NONPRIV,
                                                  .bExecuteNever = true,
                                                  .ui32LimitAddress = 0x2017FFFFU,
                                                  .ui32AttrIndex = 2,
                                                  .bEnable = true
                                                 }, // SSRAM0
                 
                                                 {.ui32RegionNumber = 1,
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
// sMPUCfgTest2: Set SSRAM0 and XIPMM0 to "Non-Cacheable".
//
am_hal_mpu_region_config_t sMPUCfgTest2[2] = { // Non-Cacheable.
                                                 {.ui32RegionNumber = 0,
                                                  .ui32BaseAddress = 0x20080000U,
                                                  .eShareable = NON_SHARE,
                                                  .eAccessPermission = RW_NONPRIV,
                                                  .bExecuteNever = true,
                                                  .ui32LimitAddress = 0x2017FFFFU,
                                                  .ui32AttrIndex = 0,
                                                  .bEnable = true
                                                 }, // SSRAM0
                 
                                                 {.ui32RegionNumber = 1,
                                                  .ui32BaseAddress = 0x60000000U,
                                                  .eShareable = NON_SHARE,
                                                  .eAccessPermission = RW_NONPRIV,
                                                  .bExecuteNever = false,
                                                  .ui32LimitAddress = 0x6FFFFFFFU,
                                                  .ui32AttrIndex = 0,
                                                  .bEnable = true
                                                 } // XIPMM0
                                             };

//
// sMPUAttr: Typical MPU attributes.
//
am_hal_mpu_attr_t sMPUAttr[3] = {
                                    {.ui8AttrIndex = 0,
                                     .bNormalMem = true,
                                     .sOuterAttr = {.bNonTransient = true, .bWriteBack = false, .bReadAllocate = false, .bWriteAllocate = false},
                                     .sInnerAttr = {.bNonTransient = true, .bWriteBack = false, .bReadAllocate = false, .bWriteAllocate = false},
                                     .eDeviceAttr = 0
                                    },
                                    {.ui8AttrIndex = 1,
                                     .bNormalMem = true,
                                     .sOuterAttr = {.bNonTransient = false, .bWriteBack = false, .bReadAllocate = true, .bWriteAllocate = false},
                                     .sInnerAttr = {.bNonTransient = false, .bWriteBack = false, .bReadAllocate = true, .bWriteAllocate = false},
                                     .eDeviceAttr = 0
                                    },
                                    {.ui8AttrIndex = 2,
                                     .bNormalMem = true,
                                     .sOuterAttr = {.bNonTransient = false, .bWriteBack = false, .bReadAllocate = true, .bWriteAllocate = true},
                                     .sInnerAttr = {.bNonTransient = false, .bWriteBack = false, .bReadAllocate = true, .bWriteAllocate = true},
                                     .eDeviceAttr = 0
                                    },
                                };

int32_t mpu_config (am_hal_mpu_region_config_t *psMPURegionConfig, uint32_t ui32CfgCnt)
{    
    am_hal_mpu_attr_configure(sMPUAttr, sizeof(sMPUAttr) / sizeof(am_hal_mpu_attr_t));

    am_hal_mpu_region_clear();

    am_hal_mpu_region_configure(psMPURegionConfig, ui32CfgCnt);

    return AM_HAL_STATUS_SUCCESS;
}
