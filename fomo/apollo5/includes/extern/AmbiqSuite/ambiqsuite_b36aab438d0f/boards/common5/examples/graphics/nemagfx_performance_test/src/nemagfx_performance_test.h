//*****************************************************************************
//
//! @file nemagfx_performance_test.h
//!
//! @brief Global includes for the nemagfx_performance_test app.
//
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef NEMAGFX_PERFORMANCE_TEST_H
#define NEMAGFX_PERFORMANCE_TEST_H

//*****************************************************************************
//
// Required built-ins.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

//*****************************************************************************
//
// Standard AmbiqSuite includes.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "nema_core.h"
#include "nema_utils.h"
#include "nema_event.h"
#include "nema_dc.h"

//*****************************************************************************
//
// NemaSDK header files.
//
//*****************************************************************************
#include "nema_hal.h"
#include "nema_math.h"
#include "nema_core.h"
#include "nema_utils.h"
#include "nema_event.h"
#include "nema_programHW.h"
#include "nema_matrix3x3.h"
#include "nema_regs.h"
#include "nema_font.h"

#include "nema_vg.h"
#include "nema_vg_path.h"
#include "nema_vg_tsvg.h"
#include "nema_vg_font.h"
#include "nema_vg_paint.h"
#include "nema_vg_context.h"

//*****************************************************************************
//
// PSRAM setting.
//
//*****************************************************************************

#define MSPI_PSRAM_MODULE              0

#if (MSPI_PSRAM_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS MSPI0_APERTURE_START_ADDR
#elif (MSPI_PSRAM_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS MSPI1_APERTURE_START_ADDR
#elif (MSPI_PSRAM_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS MSPI2_APERTURE_START_ADDR
#endif // #if (MSPI_PSRAM_MODULE == 0)

//*****************************************************************************
//
// Test item config.
//
//*****************************************************************************

#define TEST_NA 0xFF
//
// Test type enumerations
//
typedef enum
{
    TEST_UNKNOWN = 0,
    TEST_FILL,
    TEST_COPY,
    TEST_BLEND,
    TEST_BLEND_PART,
    TEST_BLEND_GLB_A,
    TEST_BLEND_FONT,
    TEST_SCALE,
    TEST_ROTATE,
    TEST_SCALE_ROTATE,
    TEST_PERSPECTIVE,
    TEST_GAUSSIAN_BLUR,
    TEST_VG,
    TEST_VG_FONT,
}
TestType_e;

//
// Source/Destination location enumerations
//
typedef enum
{
    LT_NO_USED = 0, // No font or image is used
    LT_SSRAM, // load texture to SSRAM or create frame buffer in SSRAM
    LT_PSRAM, // load texture to PSRAM or create frame buffer in PSRAM
}
LocationType_e;

//
// Source type enumerations
//
typedef enum
{
    ST_NO_USED = 0, // No font or image is used.
    ST_TEXTURE, // Use image texture
    ST_FONT, // Use font
    ST_VG_FONT, // Use image in SVG format
    ST_VG_SVG, //Use TTF font
}
SourceType_e;

typedef enum
{
    GPU_MSPI_BURST_SIZE_16 = 4,
    GPU_MSPI_BURST_SIZE_32 = 5,
    GPU_MSPI_BURST_SIZE_64 = 6,
    GPU_MSPI_BURST_SIZE_128 = 7,
}GpuMspiBurstSize_e;

//
// Test item structure
//
typedef struct
{
    TestType_e               eTestType;                 //!< test type

    LocationType_e          eTEXLocation;               //!< texture location
    LocationType_e          eFBLocation;                //!< frame buffer location
    nema_tex_format_t       sTexFormat;                 //!< texture format, use TEST_NA if no texture is used.
    nema_tex_format_t       sFBFormat;                  //!< framebuffer format
    uint32_t                ui32DCFormat;               //!< dc format

    SourceType_e            eSrcType;                   //!< Source type.
    GpuMspiBurstSize_e      eSrcBurstSize;              //!< MSPI read burst size of source texture.
    GpuMspiBurstSize_e      eDesBurstSize;              //!< MSPI read burst size of destination texture.
    uint8_t                 ui8SamplingMode;            //!< NEMA_FILTER_PS or  NEMA_FILTER_BL, use TEST_NA if no image texture is used.
    uint32_t                ui32SrcWidth;               //!< source width
    uint32_t                ui32SrcHight;               //!< source hight
    uint32_t                ui32DesWidth;               //!< destination width when it was blit/blend to framebuffer
    uint32_t                ui32DesHight;               //!< destination hight when it was blit/blend to framebuffer
    uint32_t                ui32Angle;                  //!< refers to the angle of rotation operation.
    bool                    enable_tiling;              //!< tiling enable, this will affect the scaling and rotation operation performance
    bool                    enable_morton;              //!< morton enable, this will affect the scaling and rotation operation performance
}
TestItem_t;


#endif // NEMAGFX_PERFORMANCE_TEST_H
