//*****************************************************************************
//
//! @file config_write_psram.h
//!
//! @brief read psram performance .
//
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef CONFIG_READ_PSRAM_H
#define CONFIG_READ_PSRAM_H

#include "nemagfx_performance_test.h"


//*****************************************************************************
//
// Test item list
//
//*****************************************************************************
const TestItem_t g_sTestItemList[] =
{
    //!<TestType,         TEX,      FB,      TexFormat,     FBFormat,     DCFormat,     texture/font, Src Burst size,    Des Burst size,    SamplingMode, SrcWidth,SrcHight,DesWidth,DesHight,32Angle,tiling,morton
    {TEST_UNKNOWN,      LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_NO_USED, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, TEST_NA,        466, 466, 466, 466, 0,     false,  false},

    {TEST_COPY,         LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_COPY,         LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_COPY,         LT_PSRAM, LT_SSRAM, NEMA_RGB565,   NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},

};



#endif
