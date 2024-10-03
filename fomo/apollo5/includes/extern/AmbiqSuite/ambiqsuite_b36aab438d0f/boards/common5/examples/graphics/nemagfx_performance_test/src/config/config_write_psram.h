//*****************************************************************************
//
//! @file config_write_psram.h
//!
//! @brief write psram performance .
//
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef CONFIG_WRITE_PSRAM_H
#define CONFIG_WRITE_PSRAM_H

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

    {TEST_COPY,         LT_SSRAM, LT_PSRAM, NEMA_RGBA8888,    NEMA_RGBA8888,    NEMADC_RGBA8888,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_128, GPU_MSPI_BURST_SIZE_128, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_COPY,         LT_SSRAM, LT_PSRAM, NEMA_RGB24,       NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_128, GPU_MSPI_BURST_SIZE_128, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_COPY,         LT_SSRAM, LT_PSRAM, NEMA_RGB565,    NEMA_RGB565,    NEMADC_RGB565,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_128, GPU_MSPI_BURST_SIZE_128, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},

    {TEST_COPY,         LT_SSRAM, LT_SSRAM, NEMA_RGBA8888,    NEMA_RGBA8888,    NEMADC_RGBA8888,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_128, GPU_MSPI_BURST_SIZE_128, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_COPY,         LT_SSRAM, LT_SSRAM, NEMA_RGB24,       NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_128, GPU_MSPI_BURST_SIZE_128, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_COPY,         LT_SSRAM, LT_SSRAM, NEMA_RGB565,    NEMA_RGB565,    NEMADC_RGB565,   ST_TEXTURE, GPU_MSPI_BURST_SIZE_128, GPU_MSPI_BURST_SIZE_128, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},

    {TEST_FILL,         LT_NO_USED, LT_PSRAM, TEST_NA,      NEMA_RGBA8888,    NEMADC_RGBA8888,   ST_NO_USED, GPU_MSPI_BURST_SIZE_128, GPU_MSPI_BURST_SIZE_128, TEST_NA,           0,  0, 466, 466, 0,     false,  false},
    {TEST_FILL,         LT_NO_USED, LT_PSRAM, TEST_NA,      NEMA_RGB24,    NEMADC_RGB24,   ST_NO_USED, GPU_MSPI_BURST_SIZE_128, GPU_MSPI_BURST_SIZE_128, TEST_NA,           0,  0, 466, 466, 0,     false,  false},
    {TEST_FILL,         LT_NO_USED, LT_PSRAM, TEST_NA,      NEMA_RGB565,    NEMADC_RGB565,   ST_NO_USED, GPU_MSPI_BURST_SIZE_128, GPU_MSPI_BURST_SIZE_128, TEST_NA,           0,  0, 466, 466, 0,     false,  false},

    {TEST_FILL,         LT_NO_USED, LT_SSRAM, TEST_NA,      NEMA_RGBA8888,    NEMADC_RGBA8888,   ST_NO_USED, GPU_MSPI_BURST_SIZE_128, GPU_MSPI_BURST_SIZE_128, TEST_NA,           0,  0, 466, 466, 0,     false,  false},
    {TEST_FILL,         LT_NO_USED, LT_SSRAM, TEST_NA,      NEMA_RGB24,    NEMADC_RGB24,   ST_NO_USED, GPU_MSPI_BURST_SIZE_128, GPU_MSPI_BURST_SIZE_128, TEST_NA,           0,  0, 466, 466, 0,     false,  false},
    {TEST_FILL,         LT_NO_USED, LT_SSRAM, TEST_NA,      NEMA_RGB565,    NEMADC_RGB565,   ST_NO_USED, GPU_MSPI_BURST_SIZE_128, GPU_MSPI_BURST_SIZE_128, TEST_NA,           0,  0, 466, 466, 0,     false,  false},
};



#endif
