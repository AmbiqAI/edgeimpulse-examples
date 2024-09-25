//*****************************************************************************
//
//! @file mspi_hex_ddr_psram_bandwidth_example.h
//!
//! @brief Headers file for the example of MSPI bandwidth test with DDR HEX SPI PSRAM.
//! @{
//!
//!
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
#ifndef _MSPI_HEX_DDR_PSRAM_BANDWIDTH_EXAMPLE_H
#define _MSPI_HEX_DDR_PSRAM_BANDWIDTH_EXAMPLE_H

#ifdef __cplusplus
extern "C"
{
#endif

//
//! the abstracted marco for the various mspi devices
//
#if defined(APS25616N)
#define MSPI_HEX_DDR_PSRAM_1P8V_DEVICE
#elif defined(APS25616BA)
#define MSPI_HEX_DDR_APM_PSRAM_1P2V_DEVICE
#elif defined(W958D6NW)
#define MSPI_HEX_DDR_WINBOND_PSRAM_DEVICE
#endif

#if defined(MSPI_HEX_DDR_PSRAM_1P8V_DEVICE)
#include "am_devices_mspi_psram_aps25616n.h"
#define AM_ABSTRACT_MSPI_SUCCESS AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS
#define AM_ABSTRACT_MSPI_ERROR   AM_DEVICES_MSPI_PSRAM_STATUS_ERROR
#define am_abstract_mspi_devices_config_t am_devices_mspi_psram_config_t
#define am_abstract_mspi_devices_timing_config_t am_devices_mspi_psram_ddr_timing_config_t
#elif defined(MSPI_HEX_DDR_APM_PSRAM_1P2V_DEVICE)
#include "am_devices_mspi_psram_aps25616ba_1p2v.h"
#define AM_ABSTRACT_MSPI_SUCCESS AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS
#define AM_ABSTRACT_MSPI_ERROR   AM_DEVICES_MSPI_PSRAM_STATUS_ERROR
#define am_abstract_mspi_devices_config_t am_devices_mspi_psram_config_t
#define am_abstract_mspi_devices_timing_config_t am_devices_mspi_psram_ddr_timing_config_t
#elif defined(MSPI_HEX_DDR_WINBOND_PSRAM_DEVICE)
#include "am_devices_mspi_psram_w958d6nw.h"
#define AM_ABSTRACT_MSPI_SUCCESS AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS
#define AM_ABSTRACT_MSPI_ERROR   AM_DEVICES_MSPI_PSRAM_STATUS_ERROR
#define am_abstract_mspi_devices_config_t am_devices_mspi_psram_config_t
#define am_abstract_mspi_devices_timing_config_t am_devices_mspi_psram_ddr_timing_config_t
#endif

typedef enum
{
    MEM_NONE,
    SSRAM,
    MCU_TCM,
    NUM_MEM
} am_memory_type_e;

typedef enum
{
  MSPI_DMA_ACCESS,
  MSPI_XIPMM_ACCESS_32B,
  MSPI_XIPMM_ACCESS_16B,
  MSPI_XIPMM_ACCESS_8B,
  MSPI_MEMCPY_XIPMM_ACCESS,
  MSPI_GPU_RGB24,
  MSPI_GPU_RGBA8888,
  MSPI_GPU_RGB565,
  MSPI_ACCESS_MAX,
}mspi_access_e;

typedef enum
{
    GPU_MSPI_BURST_LENGTH_16 = 4,
    GPU_MSPI_BURST_LENGTH_32 = 5,
    GPU_MSPI_BURST_LENGTH_64 = 6,
    GPU_MSPI_BURST_LENGTH_128 = 7,
} nemagfx_mspi_burst_length_t;

//
//! the universal struct for the various mspi devices struct
//! each internal function pointer members refer to their different drivers
//
typedef struct
{
    uint8_t  devName[30];
    uint32_t (*mspi_init)(uint32_t ui32Module, am_abstract_mspi_devices_config_t *psMSPISettings, void **ppHandle, void **ppMspiHandle);
    uint32_t (*mspi_term)(void *pHandle);

    uint32_t (*mspi_read_id)(void *pHandle);

    uint32_t (*mspi_read)(void *pHandle, uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes,
                           bool bWaitForCompletion);

    uint32_t (*mspi_read_adv)(void *pHandle, uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes,
                           uint32_t ui32PauseCondition,
                           uint32_t ui32StatusSetClr,
                           am_hal_mspi_callback_t pfnCallback,
                           void *pCallbackCtxt);

    uint32_t (*mspi_read_callback)(void *pHandle, uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes);

    uint32_t (*mspi_write)(void *pHandle, uint8_t *ui8TxBuffer,
                           uint32_t ui32WriteAddress,
                           uint32_t ui32NumBytes,
                           bool bWaitForCompletion);

    uint32_t (*mspi_write_adv)(void *pHandle,
                           uint8_t *puiTxBuffer,
                           uint32_t ui32WriteAddress,
                           uint32_t ui32NumBytes,
                           uint32_t ui32PauseCondition,
                           uint32_t ui32StatusSetClr,
                           am_hal_mspi_callback_t pfnCallback,
                           void *pCallbackCtxt);

    uint32_t (*mspi_mass_erase)(void *pHandle);
    uint32_t (*mspi_sector_erase)(void *pHandle, uint32_t ui32SectorAddress);

    uint32_t (*mspi_xip_enable)(void *pHandle);
    uint32_t (*mspi_xip_disable)(void *pHandle);
    uint32_t (*mspi_scrambling_enable)(void *pHandle);
    uint32_t (*mspi_scrambling_disable)(void *pHandle);
    uint32_t (*mspi_init_timing_check)(uint32_t ui32Module,
                                        am_abstract_mspi_devices_config_t *pDevCfg,
                                        am_abstract_mspi_devices_timing_config_t *pDevSdrCfg);
    uint32_t (*mspi_init_timing_apply)(void *pHandle,
                                        am_abstract_mspi_devices_timing_config_t *pDevSdrCfg);
} mspi_device_func_t;

typedef struct
{
  uint8_t *pTxBuffer;
  uint8_t *pRxBuffer;
} buffer_config_t;

typedef struct
{
  const am_hal_mspi_device_e    eDeviceMode;
  const char                    *string;
}mspi_device_mode_str_t;

typedef struct
{
  const am_hal_mspi_clock_e     eFreq;
  const char                    *string;
} mspi_speed_t;

typedef struct
{
  const uint8_t                 ui8Format;
  const char                    *string;
}mspi_gpu_format_str_t;

typedef struct
{
  uint32_t      XIPBase;
  uint32_t      XIPMMBase;
} MSPIBaseAddr_t;

typedef struct
{
  bool                          bMemContinous;
  float                         f32WriteBandwidth;
  float                         f32ReadBandwidth;
}mspi_bandwidth_t;

typedef struct
{
#if defined(AM_PART_APOLLO5B)
  am_hal_mspi_cpu_read_burst_t  sCPURQCFG;
#endif

  uint32_t                      ui32NumBytes;               // Number of bytes to read/write.
  uint32_t                      ui32ByteOffset;             // Byte offset in the RX/TX buffers.
  uint32_t                      ui32SectorAddress;          // Sector address.
  uint32_t                      ui32SectorOffset;           // Offset into Sector to write/read.

  uint8_t *                     pTxBuffer;
  uint8_t *                     pRxBuffer;

  uint8_t                       ui8Format;

  mspi_bandwidth_t              sbandwidth;

} mspi_test_config_t;

#ifdef __cplusplus
}
#endif

#endif // _MSPI_HEX_DDR_PSRAM_BANDWIDTH_EXAMPLE_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
