//*****************************************************************************
//
//! @file config_font.h
//!
//! @brief TTF font performance.
//
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef CONFIG_FONT_H
#define CONFIG_FONT_H

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
    //!< The first test was used to warm up the GPU, its result will not be print, leave it alone.
    {TEST_VG_FONT,      LT_SSRAM, LT_SSRAM, TEST_NA,       NEMA_RGB24,    NEMADC_RGB24,   ST_VG_FONT, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  0, 0, 40, 40, 0,          false,  false},
    {TEST_VG_FONT,      LT_PSRAM, LT_SSRAM, TEST_NA,       NEMA_RGB24,    NEMADC_RGB24,   ST_VG_FONT, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  0, 0, 50, 50, 0,          false,  false},
    {TEST_VG_FONT,      LT_PSRAM, LT_SSRAM, TEST_NA,       NEMA_RGB24,    NEMADC_RGB24,   ST_VG_FONT, GPU_MSPI_BURST_SIZE_64, GPU_MSPI_BURST_SIZE_64, NEMA_FILTER_PS,  0, 0, 50, 50, 0,          false,  false},
};



#endif
