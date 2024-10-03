//*****************************************************************************
//
//! @file nemagfx_performance_test.c
//!
//! @brief NemaGFX Performance Test Example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_performance_test NemaGFX Performance Test Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example put texture or framebuffer at PSRAM/SSRAM and run various baseline
//! test of GPU. The following operations are included:copy, blend, scale,
//! rotate, scale+rotate,perspective,vector graphics ,and so on.
//!
//! HOW TO USE: Compile->Download->Collect SWO output->Copy results to a file
//! ->rename this file as *.csv -> open by Excel -> save it as *.xlsx
//!
//! Note: Make sure the PSRAM is connected before running this test.
//
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "nemagfx_performance_test.h"

#include "nema_hal.h"
#include "nema_math.h"
#include "nema_core.h"
#include "nema_utils.h"
#include "nema_event.h"
#include "nema_programHW.h"
#include "nema_matrix3x3.h"

#if !defined(AM_PART_APOLLO4L)
#include "nema_vg.h"
#include "nema_vg_path.h"
#include "nema_vg_tsvg.h"
#include "nema_vg_font.h"
#include "nema_vg_paint.h"
#include "nema_vg_context.h"
#endif

#if defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L)
#include "am_devices_mspi_psram_aps25616n.h"
#include "am_devices_mspi_psram_aps25616n.c"
#else
#include "am_devices_mspi_psram_aps12808l.h"
#include "am_devices_mspi_psram_aps12808l.c"
#endif

//
// Textures
//
#if defined(AM_PART_APOLLO4L)
#include "oli_100x100_argb8888.h"
#include "oli_100x100_rgba565.h"
#include "oli_100x100_tsc6.h"
#include "oli_360x360_argb8888.h"
#include "oli_360x360_rgba565.h"
#include "oli_360x360_tsc6.h"
#define oli_20x250_argb8888_length              (20*250*4)
#define oli_100x100_rgb24_length                (100*100*3)
#define oli_360x360_rgb24_length                (360*360*3)
#else
#include "img_468x468_tsc6.h"
#include "img_468x468.z_rgba.h"
#endif
//
// fonts
//
#include "simhei34pt4b.h"
#include "simhei30pt8b.h"
#include "simhei30pt4b.h"
#if !defined(AM_PART_APOLLO4L)
//
// vector texture/font
//
#include "ume_ugo5_ttf.c"
#include "ume_ugo5_ttf.h"
#include "tiger.tsvg.h"
#define VECTOR_FONT  ume_ugo5_ttf
#endif

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#if defined(AM_PART_APOLLO4L)
#define FB_RESX 390
#define FB_RESY 390
#else
#define FB_RESX 466
#define FB_RESY 466
#endif
#define TEST_NA 0xFF
#define TEST_ITEM_TOTAL  (sizeof(g_sTestItemList)/sizeof(TestItem_t))
#if defined(AM_PART_APOLLO4L)
//
// apollo4l has no DC module.
//
#define NO_DC               0
//
// defined below two macro to eliminate buiding error.
//
#define NEMADC_ARGB8888     NO_DC
#define NEMADC_RGB565       NO_DC
#define NEMADC_RGB24        NO_DC
//
// color format
//
am_devices_disp_color_e eDispColor = COLOR_FORMAT_RGB565;
#endif

#define MAX_GAUSSIAN_RADIUS     (10)
#define MAX_KERNAL_SIZE         (MAX_GAUSSIAN_RADIUS*2+1)

#define DEBUG_PIN (95)

uintptr_t g_tex_addr = MSPI_XIP_BASE_ADDRESS;

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
    TEST_PERSPECTIVE,
    TEST_GAUSSIAN_BLUR,
#if !defined(AM_PART_APOLLO4L)
    TEST_VG,
    TEST_VG_FONT,
#endif
}
TestType_e;

//
// Test type strings
//
const char* g_pui8TestTypeStr[] =
{
    "Unknown", "Fill", "Copy", "Blend", "Blend_Part", "Blend_Global_Alpha", "Blend_Font", "Scale", "Rotate", "Perspective", "Gaussian_Blur",
#if !defined(AM_PART_APOLLO4L)
    "Vector_Graphics", "Vector_Graphics_Font"
#endif
};

//
// Source/Destination location enumerations
//
typedef enum
{
    LT_NO_USED = 0,
    LT_SSRAM,
    LT_PSRAM,
    LT_EXTENDED_RAM,
}
LocationType_e;

//
// Source/Destination location strings
//
const char* g_pui8LocationStr[] =
{
   "NO_USED","SSRAM", "PSRAM", "EXTENDED_RAM",
};

//
// Source type enumerations
//
typedef enum
{
    ST_NO_USED = 0,
    ST_TEXTURE,
    ST_FONT,
}
SourceType_e;

//
// Test item structure
//
typedef struct
{
    TestType_e              eTestType;                  //!< test type

    LocationType_e          eTEXLocation;               //!< texture location
    LocationType_e          eFBLocation;                //!< frame buffer location
    nema_tex_format_t       sTexFormat;                 //!< texture format
    nema_tex_format_t       sFBFormat;                  //!< framebuffer format
    uint32_t                ui32DCFormat;               //!< dc format

    SourceType_e            eSrcType;                   //!< Source type:font or texture.
    uint8_t                 ui8SamplingMode;            //!< NEMA_FILTER_PS or  NEMA_FILTER_BL
    uint32_t                ui32SrcWidth;               //!< source width
    uint32_t                ui32SrcHight;               //!< source hight
    uint32_t                ui32DesWidth;               //!< destination width when it was blit/blend to framebuffer
    uint32_t                ui32DesHight;               //!< destination hight when it was blit/blend to framebuffer
    uint32_t                ui32Angle;                  //!< it is usually refers to the angle of rotation,specially,represent blur radius for gaussian blur test.
    bool                    enable_tiling;              //!< tiling enable
    bool                    enable_morton;              //!< morton enable
}
TestItem_t;

//*****************************************************************************
//
// Test item list
//
//*****************************************************************************
const TestItem_t g_sTestItemList[] =
{
    //!<TestType,         TEX,      FB,      TexFormat,     FBFormat,     DCFormat,     texture/font, SamplingMode, SrcWidth,SrcHight,DesWidth,DesHight,32Angle,tiling,morton
#if defined(AM_PART_APOLLO4L)
    {TEST_UNKNOWN,      LT_SSRAM, LT_SSRAM, NEMA_XRGB8888, NEMA_XRGB8888, NEMADC_ARGB8888, ST_NO_USED, TEST_NA,         100, 100, 100, 100, 0, false, false},
    //!< The first test     was used to warm up the GPU, its result will not be print, leavfalse, e it alone.
    {TEST_FILL,         LT_SSRAM, LT_SSRAM, NEMA_XRGB8888, NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,         100, 100, 100, 100, 0, false, false},
    {TEST_FILL,         LT_SSRAM, LT_SSRAM, NEMA_RGB565,   NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_PS,         100, 100, 100, 100, 0, false, false},
    {TEST_COPY,         LT_SSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  100, 100, 100, 100, 0, true,  false},
    {TEST_COPY,         LT_SSRAM, LT_SSRAM, NEMA_RGB565,   NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  100, 100, 100, 100, 0, true,  false},
    {TEST_COPY,         LT_SSRAM, LT_SSRAM, NEMA_TSC6,     NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  100, 100, 100, 100, 0, true,  false},
    {TEST_COPY,         LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_PS,  100, 100, 100, 100, 0, true,  false},
    {TEST_COPY,         LT_PSRAM, LT_SSRAM, NEMA_RGB565,   NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_PS,  100, 100, 100, 100, 0, true,  false},
    {TEST_COPY,         LT_PSRAM, LT_SSRAM, NEMA_TSC6,     NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_PS,  100, 100, 100, 100, 0, true,  false},
    {TEST_BLEND,        LT_SSRAM, LT_SSRAM, NEMA_ARGB8888, NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  100, 100, 100, 100, 0, true,  false},
    {TEST_BLEND,        LT_SSRAM, LT_SSRAM, NEMA_ARGB8888, NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_PS,  100, 100, 100, 100, 0, true,  false},
    {TEST_BLEND_FONT,   LT_SSRAM, LT_SSRAM, NEMA_A4,       NEMA_XRGB8888, NEMADC_ARGB8888, ST_FONT, TEST_NA,         36,  36,  36,  36,  0, false, true},
    {TEST_BLEND_FONT,   LT_SSRAM, LT_SSRAM, NEMA_A4,       NEMA_RGB565,   NEMADC_RGB565,   ST_FONT, TEST_NA,         36,  36,  36,  36,  0, false, true},
    {TEST_SCALE,        LT_SSRAM, LT_SSRAM, NEMA_ARGB8888, NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  100, 100, 115, 115, 0, true,  false},
    {TEST_SCALE,        LT_SSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  100, 100, 115, 115, 0, true,  false},
    {TEST_SCALE,        LT_SSRAM, LT_SSRAM, NEMA_RGB565,   NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  100, 100, 115, 115, 0, true,  false},
    {TEST_SCALE,        LT_SSRAM, LT_SSRAM, NEMA_TSC6,     NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  100, 100, 115, 115, 0, true,  false},
    {TEST_SCALE,        LT_SSRAM, LT_SSRAM, NEMA_ARGB8888, NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_BL,  100, 100, 115, 115, 0, true,  false},
    {TEST_SCALE,        LT_SSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_BL,  100, 100, 115, 115, 0, true,  false},
    {TEST_SCALE,        LT_SSRAM, LT_SSRAM, NEMA_RGB565,   NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_BL,  100, 100, 115, 115, 0, true,  false},
    {TEST_SCALE,        LT_SSRAM, LT_SSRAM, NEMA_TSC6,     NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_BL,  100, 100, 115, 115, 0, true,  false},
    {TEST_ROTATE,       LT_SSRAM, LT_SSRAM, NEMA_ARGB8888, NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  20,  250, 20,  250, 87, true,  false},
    {TEST_ROTATE,       LT_SSRAM, LT_SSRAM, NEMA_ARGB8888, NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  20,  250, 20,  250, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGB565,   NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGB565,   NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_TSC6,     NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_TSC6,     NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_SSRAM, LT_SSRAM, NEMA_ARGB8888, NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_PS,  20,  250, 20,  250, 87, true,  false},
    {TEST_ROTATE,       LT_SSRAM, LT_SSRAM, NEMA_ARGB8888, NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_BL,  20,  250, 20,  250, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_BL,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGB565,   NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGB565,   NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_BL,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_TSC6,     NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_TSC6,     NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_BL,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_SSRAM, LT_SSRAM, NEMA_ARGB8888, NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  20,  250, 360, 360, 1 , true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 1 , true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGB565,   NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 1 , true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_TSC6,     NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 1 , true,  false},
    {TEST_ROTATE,       LT_SSRAM, LT_SSRAM, NEMA_ARGB8888, NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  20,  250, 360, 360, 44, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 44, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGB565,   NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 44, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_TSC6,     NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 44, true,  false},
    /*The following test items put frambuffer at PSRAM*/
    {TEST_FILL,         LT_SSRAM, LT_PSRAM, NEMA_XRGB8888, NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,         100, 100, 100, 100, 0, false, false},
    {TEST_FILL,         LT_SSRAM, LT_PSRAM, NEMA_RGB565,   NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_PS,         100, 100, 100, 100, 0, false, false},
    {TEST_COPY,         LT_SSRAM, LT_PSRAM, NEMA_RGB24,    NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  100, 100, 100, 100, 0, true,  false},
    {TEST_COPY,         LT_SSRAM, LT_PSRAM, NEMA_RGB565,   NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  100, 100, 100, 100, 0, true,  false},
    {TEST_COPY,         LT_SSRAM, LT_PSRAM, NEMA_TSC6,     NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  100, 100, 100, 100, 0, true,  false},
    {TEST_COPY,         LT_PSRAM, LT_PSRAM, NEMA_RGB24,    NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_PS,  100, 100, 100, 100, 0, true,  false},
    {TEST_COPY,         LT_PSRAM, LT_PSRAM, NEMA_RGB565,   NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_PS,  100, 100, 100, 100, 0, true,  false},
    {TEST_COPY,         LT_PSRAM, LT_PSRAM, NEMA_TSC6,     NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_PS,  100, 100, 100, 100, 0, true,  false},
    {TEST_BLEND,        LT_SSRAM, LT_PSRAM, NEMA_ARGB8888, NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  100, 100, 100, 100, 0, true,  false},
    {TEST_BLEND,        LT_SSRAM, LT_PSRAM, NEMA_ARGB8888, NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_PS,  100, 100, 100, 100, 0, true,  false},
    {TEST_BLEND_FONT,   LT_SSRAM, LT_PSRAM, NEMA_A4,       NEMA_XRGB8888, NEMADC_ARGB8888, ST_FONT, TEST_NA,         36,  36,  36,  36,  0, false, true},
    {TEST_BLEND_FONT,   LT_SSRAM, LT_PSRAM, NEMA_A4,       NEMA_RGB565,   NEMADC_RGB565,   ST_FONT, TEST_NA,         36,  36,  36,  36,  0, false, true},
    {TEST_SCALE,        LT_SSRAM, LT_PSRAM, NEMA_ARGB8888, NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  100, 100, 115, 115, 0, true,  false},
    {TEST_SCALE,        LT_SSRAM, LT_PSRAM, NEMA_RGB24,    NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  100, 100, 115, 115, 0, true,  false},
    {TEST_SCALE,        LT_SSRAM, LT_PSRAM, NEMA_RGB565,   NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  100, 100, 115, 115, 0, true,  false},
    {TEST_SCALE,        LT_SSRAM, LT_PSRAM, NEMA_TSC6,     NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  100, 100, 115, 115, 0, true,  false},
    {TEST_SCALE,        LT_SSRAM, LT_PSRAM, NEMA_ARGB8888, NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_BL,  100, 100, 115, 115, 0, true,  false},
    {TEST_SCALE,        LT_SSRAM, LT_PSRAM, NEMA_RGB24,    NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_BL,  100, 100, 115, 115, 0, true,  false},
    {TEST_SCALE,        LT_SSRAM, LT_PSRAM, NEMA_RGB565,   NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_BL,  100, 100, 115, 115, 0, true,  false},
    {TEST_SCALE,        LT_SSRAM, LT_PSRAM, NEMA_TSC6,     NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_BL,  100, 100, 115, 115, 0, true,  false},
    {TEST_ROTATE,       LT_SSRAM, LT_PSRAM, NEMA_ARGB8888, NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  20,  250, 20,  250, 87, true,  false},
    {TEST_ROTATE,       LT_SSRAM, LT_PSRAM, NEMA_ARGB8888, NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  20,  250, 20,  250, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_PSRAM, NEMA_RGB24,    NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_PSRAM, NEMA_RGB24,    NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_PSRAM, NEMA_RGB565,   NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_PSRAM, NEMA_RGB565,   NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_PSRAM, NEMA_TSC6,     NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_PSRAM, NEMA_TSC6,     NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_SSRAM, LT_PSRAM, NEMA_ARGB8888, NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_PS,  20,  250, 20,  250, 87, true,  false},
    {TEST_ROTATE,       LT_SSRAM, LT_PSRAM, NEMA_ARGB8888, NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_BL,  20,  250, 20,  250, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_PSRAM, NEMA_RGB24,    NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_PSRAM, NEMA_RGB24,    NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_BL,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_PSRAM, NEMA_RGB565,   NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_PSRAM, NEMA_RGB565,   NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_BL,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_PSRAM, NEMA_TSC6,     NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_PSRAM, NEMA_TSC6,     NEMA_RGB565,   NEMADC_RGB565,   ST_TEXTURE, NEMA_FILTER_BL,  360, 360, 360, 360, 87, true,  false},
    {TEST_ROTATE,       LT_SSRAM, LT_PSRAM, NEMA_ARGB8888, NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  20,  250, 360, 360, 1, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_PSRAM, NEMA_RGB24,    NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 1, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_PSRAM, NEMA_RGB565,   NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 1, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_PSRAM, NEMA_TSC6,     NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 1, true,  false},
    {TEST_ROTATE,       LT_SSRAM, LT_PSRAM, NEMA_ARGB8888, NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_BL,  20,  250, 360, 360, 44, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_PSRAM, NEMA_RGB24,    NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 44, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_PSRAM, NEMA_RGB565,   NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 44, true,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_PSRAM, NEMA_TSC6,     NEMA_XRGB8888, NEMADC_ARGB8888, ST_TEXTURE, NEMA_FILTER_PS,  360, 360, 360, 360, 44, true,  false},
#else
    {TEST_UNKNOWN,      LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_NO_USED, TEST_NA,        466, 466, 466, 466, 0,     false,  false},
    //!< The first test was used to warm up the GPU, its result will not be print, leave it alone.
    {TEST_FILL,         LT_NO_USED,LT_SSRAM, TEST_NA,      NEMA_RGB24,    NEMADC_RGB24,   ST_NO_USED, TEST_NA,           0,  0, 466, 466, 0,     false,  false},
    {TEST_COPY,         LT_SSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_COPY,         LT_SSRAM, LT_SSRAM, NEMA_RGB565,   NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_BLEND,        LT_SSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_BLEND_FONT,   LT_SSRAM, LT_SSRAM, NEMA_A4,       NEMA_RGB24,    NEMADC_RGB24,   ST_FONT,    TEST_NA,          40,  40,  40,  40, 0,    false,  false},

    {TEST_COPY,         LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_COPY,         LT_PSRAM, LT_SSRAM, NEMA_RGB565,   NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_BLEND,        LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_BLEND_PART,   LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  300, 300, 300, 300, 0,    false,  false},
    {TEST_BLEND_FONT,   LT_PSRAM, LT_SSRAM, NEMA_A8,       NEMA_RGB24,    NEMADC_RGB24,   ST_FONT,    TEST_NA,          36,  36,  36,  36, 0,    false,  false},
    {TEST_BLEND_FONT,   LT_PSRAM, LT_SSRAM, NEMA_A4,       NEMA_RGB24,    NEMADC_RGB24,   ST_FONT,    TEST_NA,          36,  36,  36,  36, 0,    false,  false},
    {TEST_BLEND_GLB_A,  LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    {TEST_SCALE,        LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  466, 466, 326, 326, 0,    false,  false},
    {TEST_SCALE,        LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_BL,  466, 466, 326, 326, 0,    false,  false},
    {TEST_SCALE,        LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_BL,  466, 466, 326, 326, 0,    false,  false},
    {TEST_SCALE,        LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  466, 466, 792, 792, 0,    false,  false},
    {TEST_SCALE,        LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_BL,  466, 466, 792, 792, 0,    false,  false},
    {TEST_SCALE,        LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_BL,  466, 466, 792, 792, 0,    false,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  468, 468, 466, 466, 89,   true,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  468, 468, 466, 466, 89,   true,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_BL,  468, 468, 466, 466, 89,   true,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  468, 468, 466, 466, 90,   true,   true},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  466, 466, 466, 466, 180,  false,  false},
    {TEST_ROTATE,       LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_BL,   20, 252, 466, 466, 89,   true,   true},
    {TEST_GAUSSIAN_BLUR,LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  466, 466, 466, 466, 3,    false,  false},
    {TEST_GAUSSIAN_BLUR,LT_PSRAM, LT_SSRAM, NEMA_RGB24,    NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  466, 466, 466, 466, 10,   false,  false},
    {TEST_PERSPECTIVE,  LT_PSRAM, LT_SSRAM, NEMA_RGBA8888, NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_BL,  468, 468, 466, 466, 180,  true,   true},

    {TEST_COPY,         LT_SSRAM, LT_PSRAM, NEMA_RGB24,    NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  466, 466, 466, 466, 0,    false,  false},
    //Decode
    {TEST_COPY,         LT_SSRAM, LT_PSRAM, NEMA_TSC6,     NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  468, 468, 466, 466, 0,    false,  false},
    {TEST_COPY,         LT_PSRAM, LT_PSRAM, NEMA_TSC6,     NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  468, 468, 466, 466, 0,    false,  false},
    {TEST_VG_FONT,      LT_SSRAM, LT_SSRAM, TEST_NA,       NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  0, 0, 40, 40, 0,          false,  false},
    {TEST_VG_FONT,      LT_PSRAM, LT_SSRAM, TEST_NA,       NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  0, 0, 50, 50, 0,          false,  false},
    {TEST_VG,           LT_SSRAM, LT_SSRAM, TEST_NA,       NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  0, 0, 466, 466, 0,        false,  false},
    {TEST_VG,           LT_PSRAM, LT_SSRAM, TEST_NA,       NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  0, 0, 400, 400, 0,        false,  false},
    {TEST_VG,           LT_PSRAM, LT_SSRAM, TEST_NA,       NEMA_RGB24,    NEMADC_RGB24,   ST_TEXTURE, NEMA_FILTER_PS,  0, 0, 400, 400, 89,       false,  false},
#endif
};

//!< TODO: Do we really need to alloc this buffer at SSRAM?
AM_SHARED_RW uint32_t        g_ui32DMATCBBuffer[2560];
void            *g_pPSRAMHandle;
void            *g_pMSPIHandle;
#ifdef AM_PART_APOLLO4B
// AXI Scratch buffer
// Need to allocate 20 Words even though we only need 16, to ensure we have 16 Byte alignment
AM_SHARED_RW uint32_t axiScratchBuf[20];
#endif

#if defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L)

am_devices_mspi_psram_config_t MSPI_PSRAM_HexCE1MSPIConfig =
{
#if defined(APS25616N_OCTAL_MODE_EN ) || (DISPLAY_MSPI_INST == 1) || !defined(AM_BSP_MSPI_PSRAM_MODULE_HEX_DDR_CE)
    .eDeviceConfig            = AM_BSP_MSPI_PSRAM_MODULE_OCTAL_DDR_CE,
#else
    .eDeviceConfig             = AM_BSP_MSPI_PSRAM_MODULE_HEX_DDR_CE,
#endif
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(g_ui32DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = g_ui32DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};
#else
am_devices_mspi_psram_config_t MSPI_PSRAM_OctalCE1MSPIConfig =
{
    .eDeviceConfig            = AM_BSP_MSPI_PSRAM_MODULE_OCTAL_DDR_CE,
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(g_ui32DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = g_ui32DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};
#endif

//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
};

//
// Take over the interrupt handler for whichever MSPI we're using.
//
#define psram_mspi_isr                                                          \
    am_mspi_isr1(MSPI_PSRAM_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi ## n ## _isr

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void
psram_mspi_isr(void)
{
    uint32_t ui32Status;

    am_hal_mspi_interrupt_status_get(g_pMSPIHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_pMSPIHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_pMSPIHandle, ui32Status);
}

//*****************************************************************************
//
//! @brief Convert source/destination color format to string
//!
//! @return char pointer.
//
//*****************************************************************************
char *
format2string(nema_tex_format_t sFormat)
{
    char* ui8Str;

    switch (sFormat)
    {
        case NEMA_ARGB8888:
            ui8Str = "ARGB8888";
            break;
        case NEMA_RGBA8888:
            ui8Str = "RGBA8888";
            break;
        case NEMA_XRGB8888:
            ui8Str = "XRGB8888";
            break;
        case NEMA_RGB24:
            ui8Str = "RGB888";
            break;
        case NEMA_RGB565:
            ui8Str = "RGB565";
            break;
        case NEMA_TSC6:
            ui8Str = "TSC6";
            break;
        case NEMA_A4:
            ui8Str = "A4";
            break;
       case NEMA_A8:
            ui8Str = "A8";
            break;
        default:
            ui8Str = "unknown";
            break;
    }

    return ui8Str;
}

//*****************************************************************************
//
//! @brief load texture to PSRAM or SSRAM
//!
//! @return uintptr_t pointer.
//
//*****************************************************************************
uintptr_t
load_texture(const TestItem_t* psTestItem)
{
    uintptr_t texture = (uintptr_t)NULL;

    switch(psTestItem->eTEXLocation)
    {
        case LT_NO_USED:
            break;
        case LT_SSRAM:
            texture = SSRAM1_BASE_ADDRESS;
            break;
        case LT_PSRAM:
            texture = g_tex_addr;
            break;
        case LT_EXTENDED_RAM:
            break;
        default:
            break;
    }
    if(texture == (uintptr_t)NULL)
    {
        return texture;
    }
    switch (psTestItem->sTexFormat)
    {
#if defined(AM_PART_APOLLO4L)
        case NEMA_ARGB8888:
            if ((psTestItem->ui32SrcWidth == 100) && (psTestItem->ui32SrcHight == 100))
            {
                memcpy((void*)texture, g_ui8Oli100x100ARGB8888, sizeof(g_ui8Oli100x100ARGB8888));
            }
            else if ((psTestItem->ui32SrcWidth == 360) && (psTestItem->ui32SrcHight == 360))
            {
                memcpy((void*)texture, g_ui8Oli360x360ARGB8888, sizeof(g_ui8Oli360x360ARGB8888));
            }
            else if ((psTestItem->ui32SrcWidth == 20) && (psTestItem->ui32SrcHight == 250))
            {
                memset((void*)texture, 0xFF, oli_20x250_argb8888_length);
            }
            break;
        //!< We don't have a RGB24 texture, so we just create one witch was filled by 0xFF,
        //!< you will see all white when it was displayed on the panel
        case NEMA_RGB24:
            if ((psTestItem->ui32SrcWidth == 100) && (psTestItem->ui32SrcHight == 100))
            {
                memset((void*)texture, 0xFF, oli_100x100_rgb24_length);
            }
            else if ((psTestItem->ui32SrcWidth == 360) && (psTestItem->ui32SrcHight == 360))
            {
                memset((void*)texture, 0xFF, oli_360x360_rgb24_length);
            }
            break;
        case NEMA_RGB565:
            if ((psTestItem->ui32SrcWidth == 100) && (psTestItem->ui32SrcHight == 100))
            {
                memcpy((void*)texture, g_ui8Oli100x100RGBA565, sizeof(g_ui8Oli100x100RGBA565));
            }
            else if ((psTestItem->ui32SrcWidth == 360) && (psTestItem->ui32SrcHight == 360))
            {
                memcpy((void*)texture, g_ui8Oli360x360RGBA565, sizeof(g_ui8Oli360x360RGBA565));
            }
            break;
        case NEMA_TSC6:
            if ((psTestItem->ui32SrcWidth == 100) && (psTestItem->ui32SrcHight == 100))
            {
                memcpy((void*)texture, g_ui8Oli100x100Tsc6, sizeof(g_ui8Oli100x100Tsc6));
            }
            else if ((psTestItem->ui32SrcWidth == 360) && (psTestItem->ui32SrcHight == 360))
            {
                memcpy((void*)texture, g_ui8Oli360x360Tsc6, sizeof(g_ui8Oli360x360Tsc6));
            }
            break;
        default:
            break;
#else
        case NEMA_RGBA8888:
        case NEMA_RGB24:
        case NEMA_RGB565:
            memcpy((void*)texture,img_468x468,sizeof(img_468x468));
            //g_tex_addr = texture+(sizeof(img_468x468)+7)>>3<<3;
            break;
        case NEMA_TSC6:
            memcpy((void*)texture,img_468x468_tsc6,sizeof(img_468x468_tsc6));
            //g_tex_addr = texture+(sizeof(img_468x468_tsc6)+7)>>3<<3;
            break;
        default:
            switch(psTestItem->eTestType)
            {
                case TEST_VG_FONT:
                     memcpy((void*)texture, VECTOR_FONT.data, VECTOR_FONT.data_length*sizeof(float));
                     VECTOR_FONT.data = (void*)texture;
                      //bind font
                      nema_vg_bind_font(&VECTOR_FONT);
                       if(psTestItem->ui32DesHight == 40)
                       {
                           nema_vg_set_font_size(16);
                       }
                       else if(psTestItem->ui32DesHight == 50)
                       {
                           nema_vg_set_font_size(18);
                       }
                       else
                       {
                           am_util_stdio_printf("please adjust vg font size.\n");
                       }
                      //g_tex_addr = texture+(sizeof(img_468x468_tsc6)+7)>>3<<3;
                    break;
                case TEST_VG:
                    //nema_buffer_t tsvg_buffer = nema_buffer_create(tiger_tsvg_length);
                    //texture = (uintptr_t)(tsvg_buffer.base_virt);
                    memcpy((void*)texture, tiger_tsvg, tiger_tsvg_length);
                    break;
                default:
                    break;
            }
#endif
            break;
    }

    return texture;
}

//*****************************************************************************
//
//! @brief load font to PSRAM or SSRAM
//!
//! @return nema_font_t pointer.
//
//*****************************************************************************
nema_font_t*
load_font(const TestItem_t* psTestItem)
{
    nema_font_t* psFont = NULL;
    uintptr_t texture = (uintptr_t)NULL;

    switch(psTestItem->eTEXLocation)
    {
        case LT_NO_USED:
            break;
        case LT_SSRAM:
            texture = SSRAM1_BASE_ADDRESS;
            break;
        case LT_PSRAM:
            texture = g_tex_addr;
            break;
        case LT_EXTENDED_RAM:
            break;
        default:
            break;
    }
    if(texture == (uintptr_t)NULL)
    {
        return psFont;
    }
    switch(psTestItem->sTexFormat)
    {
        case NEMA_A4:
            if(psTestItem->ui32DesHight == 36)
            {
                memcpy((void*)texture ,simhei30pt4b.bitmap,simhei30pt4b.bitmap_size);
                simhei30pt4b.bo.base_phys = texture ;
                simhei30pt4b.bo.base_virt = (void*)simhei30pt4b .bo.base_phys;
                psFont = &simhei30pt4b ;
            }
            else if(psTestItem->ui32DesHight == 40)
            {
                memcpy((void*)texture ,simhei34pt4b.bitmap,simhei34pt4b.bitmap_size);
                simhei34pt4b.bo.base_phys = texture ;
                simhei34pt4b.bo.base_virt = (void*)simhei34pt4b .bo.base_phys;
                psFont = &simhei34pt4b ;
            }
            break;
        case NEMA_A8:
            memcpy((void*)texture ,simhei30pt8b.bitmap,simhei30pt8b.bitmap_size);
            simhei30pt8b.bo.base_phys = texture ;
            simhei30pt8b.bo.base_virt = (void*)simhei30pt8b .bo.base_phys;
            psFont = &simhei30pt8b ;
            //g_tex_addr = texture+(g_sSimhei18pt4b.bitmap_size+7)>>3<<3;
            break;
        default:
            break;
    }

    return psFont;
}

//*****************************************************************************
//
//! @brief Init framebuffer and dc layer
//!
//! @return int32_t.
//
//*****************************************************************************
#if !defined(AM_PART_APOLLO4L)
int32_t
init_framebuffer_layer(const TestItem_t* psTestItem,nemadc_layer_t* psLayer,img_obj_t* psFB)
#else
int32_t
init_framebuffer_layer(const TestItem_t* psTestItem,img_obj_t* psFB)
#endif
{
    psFB->w = FB_RESX;
    psFB->h = FB_RESY;
#if defined(AM_PART_APOLLO4L)
    psFB->format = (eDispColor == COLOR_FORMAT_RGB565 ? NEMA_RGB565 : NEMA_RGB24);
#else
    psFB->format = psTestItem->sFBFormat;
#endif
    if(psFB->format == NEMA_TSC4)
    {
        psFB->stride = psFB->w /2;
    }
    else if(psFB->format == NEMA_TSC6 || psFB->format == NEMA_TSC6A)
    {
        psFB->stride = psFB->w * 6/8;
    }
    else
    {
       psFB->stride = nema_stride_size(psFB->format, 0, psFB->w);
    }

    g_tex_addr = MSPI_XIP_BASE_ADDRESS ;
    //
    // Alloc frame buffer space
    //
    switch (psTestItem->eFBLocation)
    {
        case LT_SSRAM:
            psFB->bo = nema_buffer_create(psFB->stride * psFB->h);
            (void)nema_buffer_map(&psFB->bo);
            break;
        case LT_PSRAM:
            psFB->bo.base_phys = g_tex_addr;
            psFB->bo.base_virt = (void*)psFB->bo.base_phys;
            g_tex_addr = g_tex_addr + (psFB->stride*psFB->h+7)/8*8 + 0x200;

            break;
        case LT_EXTENDED_RAM:
            am_util_stdio_printf("Can't create frame buffer at extended ram, to be implement!\n");
            psFB->bo.base_phys = (uintptr_t)NULL;
            psFB->bo.base_virt = (void*)psFB->bo.base_phys;
            break;
        default:
            am_util_stdio_printf("Unkown frame buffer location!\n");
            psFB->bo.base_phys = (uintptr_t)NULL;
            psFB->bo.base_virt = (void*)psFB->bo.base_phys;
            break;
    }

    if ((void*)psFB->bo.base_phys == NULL)
    {
        am_util_stdio_printf("frame buffer malloc failed!\n");
        return -1;
    }

#if !defined(AM_PART_APOLLO4L)
    psLayer->startx        = 0;
    psLayer->sizex         = psFB->w;
    psLayer->resx          = psFB->w;
    psLayer->starty        = 0;
    psLayer->sizey         = psFB->h;
    psLayer->resy          = psFB->h;
    psLayer->stride        = psFB->stride;
    psLayer->format        = psTestItem->ui32DCFormat;
    psLayer->blendmode     = NEMADC_BL_SRC;
    psLayer->baseaddr_phys = psFB->bo.base_phys;
    psLayer->baseaddr_virt = psFB->bo.base_virt;
    psLayer->buscfg        = 0;
    psLayer->alpha         = 0xff;
    psLayer->flipx_en      = 0;
    psLayer->flipy_en      = 0;
#endif //!defined(AM_PART_APOLLO4L)

    return 0;
}

//*****************************************************************************
//
//! @brief blur algorithm provided by vendor
//!
//! @return None.
//
//*****************************************************************************
static void
blur_vh(int dst_x, int dst_y, int kernel_size, int *kernel, int dilation, int do_vert)
{
    nema_set_blend_blit(NEMA_BL_SRC | NEMA_BLOP_MODULATE_RGB | NEMA_BLOP_MODULATE_A);

    int idx = 0;

    for ( int xx = -kernel_size*dilation; xx <= kernel_size*dilation; xx+=dilation, ++idx )
    {
        uint32_t val = kernel[idx]&0xff;
        uint32_t col = (val) | (val << 8) | (val << 16) | (val << 24);

        int x = do_vert == 0 ? xx : 0 ;
        int y = do_vert == 0 ? 0  : xx;

        nema_set_const_color(col);
        nema_blit_subrect(dst_x, dst_y, FB_RESX, FB_RESY, x, y);

        if (idx == 0)
        {
            nema_set_blend_blit(NEMA_BL_ADD | NEMA_BLOP_MODULATE_RGB | NEMA_BLOP_MODULATE_A);
        }
    }
}

int32_t
nemagfx_performance_test(void)
{
    int32_t i;
    static img_obj_t sFB = {0};
#if !defined(AM_PART_APOLLO4L)
    static nemadc_layer_t sLayer = {0};
    uint32_t svg_width, svg_height;
    float svg_w, svg_h;
    float scalex,scaley;
    NEMA_VG_PAINT_HANDLE paint = NULL;
#endif
    static nema_cmdlist_t sCL;
    const TestItem_t* psTestItem;
    float time_start;
    float time_end;
    double time_used;
    double speed_pps;
    uint32_t ui32StartX = 0;
    uint32_t ui32StartY = 0;
    uintptr_t texture = (uintptr_t)NULL;
    nema_font_t* psFont = NULL;
    float x0,y0,x1,y1,x2,y2,x3,y3;

    char ui8Str[] = "����������";
    uint8_t ui8Alpha = 0x77;
    uint8_t ui8GaussianRadius = 0;
    int kernel[MAX_KERNAL_SIZE];

    uint32_t val,col,val0,col0;
    int diff;

    am_hal_gpio_pinconfig(DEBUG_PIN, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_clear(DEBUG_PIN);
    //
    // Create CL
    //
    sCL = nema_cl_create_sized(16*1024);
    am_util_stdio_printf("Type,Texture_Location,FB_Location,Tex_Format,FB_Format,SamplingMode,Src_Width,Src_Hight,Des_Width,Des_Hight,Des_Angle,time(ms),speed_MP/s\n");
    //
    // run the test
    //
    for (i = 0; i < TEST_ITEM_TOTAL; i++)
    {
        psTestItem = &g_sTestItemList[i];
        //
        // Init framebuffer and layer
        //
#if !defined(AM_PART_APOLLO4L)
        init_framebuffer_layer(psTestItem,&sLayer,&sFB);
        //
        // Set DC layer
        //
        nemadc_set_layer(0, &sLayer);
#else
        init_framebuffer_layer(psTestItem,&sFB);
#endif //!defined(AM_PART_APOLLO4L)
        //
        // rewind and bind the CL
        //
        nema_cl_bind(&sCL);
        nema_cl_rewind(&sCL);

        //
        // bind framebuffer as destination texture
        //
        nema_bind_dst_tex(sFB.bo.base_phys, FB_RESX, FB_RESY, sFB.format, sFB.stride);
        //
        // set clip
        //
        nema_set_clip(0, 0, FB_RESX, FB_RESY);
        //
        // clear the frambuffer
        //
        nema_clear(0);
        //
        // submit the CL
        //
        nema_cl_submit(&sCL);
        //
        // wait for GPU
        //
        nema_cl_wait(&sCL);
        nema_cl_unbind();

        //
        // rewind and bind the CL
        //
#if !defined(AM_PART_APOLLO4L)
        if(psTestItem->eTestType == TEST_VG_FONT || psTestItem->eTestType == TEST_VG)
        {
            //
            // init vector graphics
            //
            nema_vg_init(FB_RESX, FB_RESY);
            nema_cl_bind_circular(&sCL);
            //
            // We should treat font as texture when test vector graphics font.
            //
            if(psTestItem->eSrcType == ST_FONT)
            {
                am_util_stdio_printf("we should treat font as textrue when test vector graphics font.\n");
                 return -1;
            }
        }
        else
#endif //!defined(AM_PART_APOLLO4L)
        {
            nema_cl_bind(&sCL);
        }
        nema_cl_rewind(&sCL);
        //
        // bind framebuffer as destination
        //
        nema_bind_dst_tex(sFB.bo.base_phys, FB_RESX, FB_RESY, sFB.format, sFB.stride);
        //
        // set clip
        //
        nema_set_clip(0, 0, FB_RESX, FB_RESY);
        //
        // bind source texture
        //

        switch (psTestItem->eSrcType)
        {
            case ST_TEXTURE:
                texture = load_texture(psTestItem);
                if (texture == (uintptr_t)NULL)
                {
                   am_util_stdio_printf("Can't find source texture!\n");
                   return -1;
                }
#if !defined(AM_PART_APOLLO4L)
                if(psTestItem->eTestType == TEST_VG_FONT)
                {

                }
                else if(psTestItem->eTestType == TEST_VG)
                {

                }
                else if(psTestItem->eTestType == TEST_GAUSSIAN_BLUR)
#else
                if(psTestItem->eTestType == TEST_GAUSSIAN_BLUR)
#endif //!defined(AM_PART_APOLLO4L)
                {

                }
                else
                {
                    if(psTestItem->ui8SamplingMode != NEMA_FILTER_PS && psTestItem->ui8SamplingMode != NEMA_FILTER_BL)
                    {
                        am_util_stdio_printf("invalid sampling mode!\n");
                        return -1;
                    }
                    nema_tex_mode_t mode = (psTestItem->enable_morton) ? (psTestItem->ui8SamplingMode|NEMA_TEX_MORTON_ORDER) : (psTestItem->ui8SamplingMode);
                    nema_bind_src_tex(texture,
                                      psTestItem->ui32SrcWidth,
                                      psTestItem->ui32SrcHight,
                                      psTestItem->sTexFormat,
                                      -1,
                                      mode);

                    nema_enable_tiling(psTestItem->enable_tiling);
                }

                break;
            case ST_FONT:
                psFont = load_font(psTestItem);

                if (psFont == NULL)
                {
                   am_util_stdio_printf("Cann't find psFont!\n");
                   return -1;
                }

                nema_bind_font(psFont);
                break;
            default:
                break;

        }
        //
        // start point
        //
        if(FB_RESX < psTestItem->ui32DesWidth)
        {
            ui32StartX = 0;
        }
        else
        {
            ui32StartX = (FB_RESX-psTestItem->ui32DesWidth) / 2;
        }
        if(FB_RESY < psTestItem->ui32DesWidth)
        {
            ui32StartY = 0;
        }
        else
        {
            ui32StartY = (FB_RESY-psTestItem->ui32DesHight) / 2;
        }

        time_start = nema_get_time();
        am_hal_gpio_output_set(DEBUG_PIN);
        switch(psTestItem->eTestType)
        {
            case TEST_UNKNOWN:
            case TEST_FILL:
                nema_set_blend_fill(NEMA_BL_SRC);
                nema_fill_rect(ui32StartX,
                               ui32StartY,
                               psTestItem->ui32DesWidth,
                               psTestItem->ui32DesHight,
                               0xFF0A59F7);
                break;

            case TEST_COPY:
                nema_set_blend_blit(NEMA_BL_SRC);
                nema_blit(ui32StartX, ui32StartY);
                break;

            case TEST_BLEND:
                nema_set_blend_blit( NEMA_BL_SIMPLE);
                nema_blit(ui32StartX, ui32StartY);
                break;

            case TEST_BLEND_PART:
                nema_set_blend_blit( NEMA_BL_SIMPLE);
                nema_blit_subrect_fit(49, 49, 300, 300, 50, 50, 300, 300);
                break;

            case TEST_BLEND_GLB_A:
                nema_set_blend_blit(NEMA_BL_SIMPLE | NEMA_BLOP_MODULATE_A);
                nema_set_const_color(ui8Alpha << 24|0x00FFFFFF);
                nema_blit(ui32StartX, ui32StartY);
                break;

            case TEST_BLEND_FONT:
                ui32StartX = 100;
//                nema_set_blend_blit(NEMA_BL_SRC);
//                nema_fill_rect(ui32StartX,
//                                 ui32StartY,
//                                 psTestItem->ui32DesWidth,
//                                 psTestItem->ui32DesHight,
//                                 0x70707070);
                nema_print(ui8Str,
                           ui32StartX,
                           ui32StartY,
                           psTestItem->ui32DesWidth*5,
                           psTestItem->ui32DesHight,
                           0xff0a59f7U,
                           NEMA_ALIGNX_LEFT | NEMA_TEXT_WRAP | NEMA_ALIGNY_TOP);
                break;

            case TEST_SCALE:
                nema_set_blend_blit(NEMA_BL_SIMPLE);
                nema_blit_rect_fit(ui32StartX,
                                   ui32StartY,
                                   psTestItem->ui32DesWidth,
                                   psTestItem->ui32DesHight);
                break;

            case TEST_ROTATE:
                nema_set_blend_blit(NEMA_BL_SIMPLE);
                nema_blit_rotate_pivot( FB_RESX / 2,
                                        FB_RESY / 2,
                                        psTestItem->ui32SrcWidth / 2,
                                        psTestItem->ui32SrcHight / 2,
                                        psTestItem->ui32Angle);
                break;

            case TEST_PERSPECTIVE:
                nema_set_blend_blit(NEMA_BL_SIMPLE);
                x0 = -(psTestItem->ui32SrcWidth*0.5f);
                y0 = -(psTestItem->ui32SrcHight*0.5f);
                x1 = x0+psTestItem->ui32SrcWidth;
                y1 = y0;
                x2 = x0+psTestItem->ui32SrcWidth;
                y2 = y0+psTestItem->ui32SrcHight;
                x3 = x0;
                y3 = y0+psTestItem->ui32SrcHight;

                nema_matrix3x3_t m;
                nema_mat3x3_load_identity(m);
                nema_mat3x3_rotate(m, 180);
                nema_mat3x3_translate(m, FB_RESX/2, FB_RESY/2);
                m[2][0] = 1.0e-3;
                m[2][1] = 2.0e-3;
                m[2][2] = 1.0;
                nema_mat3x3_mul_vec(m, &x0, &y0);
                nema_mat3x3_mul_vec(m, &x1, &y1);
                nema_mat3x3_mul_vec(m, &x2, &y2);
                nema_mat3x3_mul_vec(m, &x3, &y3);
                nema_blit_quad_fit(x0, y0,
                                   x1, y1,
                                   x2, y2,
                                   x3, y3);
                break;
            case TEST_GAUSSIAN_BLUR:
                //
                // borrowed variable definition from angle as gaussian radius.
                //
                ui8GaussianRadius = psTestItem->ui32Angle&0xFF;
                if(ui8GaussianRadius > MAX_GAUSSIAN_RADIUS)
                {
                    am_util_stdio_printf("Gaussian kernal size too small!\n");
                }
                val = 255/(ui8GaussianRadius*2+1);
                col = (val) | (val << 8) | (val << 16) | (val << 24);
                diff = 255-(val*(ui8GaussianRadius*2+1));
                val0 = val+diff;
                col0 = (val0) | (val0 << 8) | (val0 << 16) | (val0 << 24);
                for (int i = 0; i < ui8GaussianRadius*2+1; ++i) {
                    kernel[i] = col;
                }
                kernel[ui8GaussianRadius] = col0;
                nema_set_tex_color(0);

                nema_bind_src_tex(texture,
                                  psTestItem->ui32SrcWidth,
                                  psTestItem->ui32SrcHight,
                                  psTestItem->sTexFormat,
                                  -1,
                                  NEMA_FILTER_PS | NEMA_TEX_BORDER);
                nema_bind_dst_tex(sFB.bo.base_phys,
                                  FB_RESX,
                                  FB_RESY,
                                  sFB.format,
                                  sFB.stride);

                blur_vh(0, 0, ui8GaussianRadius, kernel, 1, 0);

                nema_bind_src_tex(sFB.bo.base_phys,
                                  FB_RESX,
                                  FB_RESY,
                                  sFB.format,
                                  sFB.stride,
                                  NEMA_TEX_BORDER);
                nema_bind_dst_tex(texture,
                                  psTestItem->ui32SrcWidth,
                                  psTestItem->ui32SrcHight,
                                  psTestItem->sTexFormat,
                                  -1);
                nema_set_blend_blit(NEMA_BL_SRC);
                nema_blit(0,0);

                nema_bind_src_tex(texture,
                                  psTestItem->ui32SrcWidth,
                                  psTestItem->ui32SrcHight,
                                  psTestItem->sTexFormat,
                                  -1,
                                  NEMA_FILTER_PS | NEMA_TEX_BORDER);
                nema_bind_dst_tex(sFB.bo.base_phys,
                                  FB_RESX,
                                  FB_RESY,
                                  sFB.format,
                                  sFB.stride);

                blur_vh(0, 0, ui8GaussianRadius, kernel, 1, 1);
                break;

#if !defined(AM_PART_APOLLO4L)
            case TEST_VG_FONT:
                ui32StartX = 100;

//                nema_fill_rect(ui32StartX,
//                               ui32StartY,
//                               psTestItem->ui32DesWidth*5,
//                               psTestItem->ui32DesHight,
//                               0x70707070);
                //blend mode
                nema_set_blend_fill(NEMA_BL_SRC);
                //Create and set paint
                paint =  nema_vg_paint_create();
                nema_vg_paint_set_type(paint, NEMA_VG_PAINT_COLOR);
                nema_vg_paint_set_stroke_width(paint, 0.f);
                //Draw font
                nema_vg_paint_set_paint_color(paint, nema_rgba(0x0a, 0x59, 0xf7, 0xff ));
                nema_vg_print(paint,
                              ui8Str,
                              ui32StartX,
                              ui32StartY,
                              psTestItem->ui32DesWidth*5,
                              psTestItem->ui32DesHight,
                              NEMA_VG_ALIGNX_LEFT | NEMA_VG_TEXT_WRAP | NEMA_VG_ALIGNY_TOP,
                              NULL);
                break;
            case TEST_VG:
                nema_vg_get_tsvg_resolution((void *)texture, &svg_width, &svg_height);

                if (svg_width != 0U && svg_height != 0U)
                {
                    svg_w = (float)svg_width;
                    svg_h = (float)svg_height;
                }
                else
                {
                    svg_w = (float)498; //(float)svg_width;
                    svg_h = (float)600; //(float)svg_height;
                }

                scalex = psTestItem->ui32DesWidth / svg_w;
                scaley = psTestItem->ui32DesHight / svg_h;
                nema_matrix3x3_t matrix;
                nema_mat3x3_load_identity(matrix);
                nema_mat3x3_translate(matrix, -svg_w * 0.5f, -svg_h * 0.5f);
                nema_mat3x3_rotate(matrix, psTestItem->ui32Angle);
                nema_mat3x3_scale(matrix, scalex, scaley);
                nema_mat3x3_translate(matrix, FB_RESX / 2, FB_RESY / 2);

                nema_vg_set_global_matrix(matrix);
                //nema_vg_set_quality(NEMA_VG_QUALITY_BETTER);
                nema_vg_set_quality(NEMA_VG_QUALITY_FASTER);
                //nema_vg_set_quality(NEMA_VG_QUALITY_NON_AA);
                nema_vg_draw_tsvg((void *)texture);
                break;
#endif //!defined(AM_PART_APOLLO4L)
            default:
                break;
        }
        //
        // submit the CL
        //
        nema_cl_submit(&sCL);
        //
        // wait for GPU
        //
        nema_cl_wait(&sCL);
        nema_cl_unbind();

        am_hal_gpio_output_clear(DEBUG_PIN);
        time_end = nema_get_time();
#if !defined(AM_PART_APOLLO4L)
        //
        // flush screen
        //
        nemadc_transfer_frame_prepare(false);
        //
        //It's necessary to launch frame manually when TE is disabled.
        //
        nemadc_transfer_frame_launch();
        nemadc_wait_vsync();
#else
        //
        // transfer frame to the display
        //
        am_devices_display_transfer_frame(sFB.w,
                                          sFB.h,
                                          sFB.bo.base_phys,
                                          NULL, NULL);
        //
        // wait transfer done
        //
        am_devices_display_wait_transfer_done();
#endif
        //
        // calculate result and output
        //
        time_used = (double)(time_end - time_start) * 1000.0;
#if !defined(AM_PART_APOLLO4L)
        if(psTestItem->eTestType == TEST_VG_FONT || psTestItem->eTestType == TEST_BLEND_FONT)
        {
            //
            // Time should be divided by 5 because of draw 5 chinese character.
            //
            time_used /= 5;
        }
#endif
        speed_pps = (psTestItem->ui32DesWidth * psTestItem->ui32DesHight) / time_used / 1000.0;

        if (psTestItem->eTestType != TEST_UNKNOWN)
        {
          am_util_stdio_printf("%s,%s,%s,%s,%s,%s,%d,%d,%d,%d,%d,%.3f,%.3f\n",
                              g_pui8TestTypeStr[psTestItem->eTestType],
                              g_pui8LocationStr[psTestItem->eTEXLocation],
                              g_pui8LocationStr[psTestItem->eFBLocation],
                              format2string(psTestItem->sTexFormat),
#if !defined(AM_PART_APOLLO4L)
                              format2string(psTestItem->sFBFormat),
#else
                              format2string(eDispColor == COLOR_FORMAT_RGB565?NEMA_RGB565:NEMA_RGB24),
#endif //!defined(AM_PART_APOLLO4L)
                              (psTestItem->ui8SamplingMode == NEMA_FILTER_PS ? "Point" : "Bilinear"),
                              psTestItem->ui32SrcWidth,
                              psTestItem->ui32SrcHight,
                              psTestItem->ui32DesWidth,
                              psTestItem->ui32DesHight,
                              psTestItem->ui32Angle,
                              time_used,
                              speed_pps);
        }

        //
        // destroy framebuffer
        //
        nema_buffer_unmap(&sFB.bo);
        nema_buffer_destroy(&sFB.bo);
#if !defined(AM_PART_APOLLO4L)
        if(psTestItem->eTestType == TEST_VG_FONT)
        {
            nema_vg_paint_destroy(paint);
            nema_vg_deinit();
        }
        else if(psTestItem->eTestType == TEST_VG)
        {
            nema_vg_deinit();
        }
#endif //!defined(AM_PART_APOLLO4L)
    }
    return 0;
}

//*****************************************************************************
//
// Main function
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32Status;
#if !defined(AM_PART_APOLLO4L)
    uint32_t ui32MipiCfg = MIPICFG_16RGB888_OPT0;    //!< default config
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_dsi_raydium_config_t sDisplayPanelConfig;
#endif
    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();
    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    //
    // Set up scratch AXI buf (needs 64B - aligned to 16 Bytes)
    //
#ifdef AM_PART_APOLLO4B
    am_hal_daxi_control(AM_HAL_DAXI_CONTROL_AXIMEM, (uint8_t *)((uint32_t)(axiScratchBuf + 3) & ~0xF));
#endif

    //
    // External power on
    //
    am_bsp_external_pwr_on();
    am_util_delay_ms(100);
    //
    // Configure the MSPI and PSRAM Device.
    //
#if defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L)
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_HexCE1MSPIConfig, &g_pPSRAMHandle, &g_pMSPIHandle);
#else
    ui32Status = am_devices_mspi_psram_aps12808l_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_OctalCE1MSPIConfig, &g_pPSRAMHandle, &g_pMSPIHandle);
#endif

    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_SetPriority(mspi_interrupts[MSPI_PSRAM_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[MSPI_PSRAM_MODULE]);

    //
    // Enable XIP mode.
    //
#if defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L)
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPSRAMHandle);
#else
    ui32Status = am_devices_mspi_psram_aps12808l_ddr_enable_xip(g_pPSRAMHandle);
#endif
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == ui32Status)
    {
        am_util_stdio_printf("Enable XIP mode in the MSPI!\n");
    }
    else
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }
    //
    // Initialize the printf interface for ITM output
    //
    am_bsp_debug_printf_enable();

#if !defined(AM_PART_APOLLO4L)
    //
    // Set the display region to center
    //
    if (FB_RESX < g_sDispCfg.ui16ResX)
    {
        sDisplayPanelConfig.ui16ResX = FB_RESX;
    }
    else
    {
        sDisplayPanelConfig.ui16ResX = g_sDispCfg.ui16ResX;
    }
    ui16MinX = (g_sDispCfg.ui16ResX - sDisplayPanelConfig.ui16ResX) >> 1;
    ui16MinX = (ui16MinX >> 1) << 1;

    if (FB_RESY < g_sDispCfg.ui16ResY)
    {
        sDisplayPanelConfig.ui16ResY = FB_RESY;
    }
    else
    {
        sDisplayPanelConfig.ui16ResY = g_sDispCfg.ui16ResY;
    }
    ui16MinY = (g_sDispCfg.ui16ResY - sDisplayPanelConfig.ui16ResY) >> 1;
    ui16MinY = (ui16MinY >> 1) << 1;

    g_sDispCfg.eTEType = DISP_TE_DISABLE;
    sDCConfig.ui16ResX = sDisplayPanelConfig.ui16ResX;
    sDCConfig.ui16ResY = sDisplayPanelConfig.ui16ResY;
    sDCConfig.bTEEnable = (g_sDispCfg.eTEType == DISP_TE_DC);
    sDisplayPanelConfig.ui16MinX = ui16MinX + g_sDispCfg.ui16Offset;
    sDisplayPanelConfig.ui16MinY = ui16MinY;
    sDisplayPanelConfig.bTEEnable = (g_sDispCfg.eTEType != DISP_TE_DISABLE);
    sDisplayPanelConfig.bFlip = g_sDispCfg.bFlip;

    am_bsp_disp_pins_enable();

    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        //
        // VDD18 control callback function
        //
        am_hal_dsi_register_external_vdd18_callback(am_bsp_external_vdd18_switch);
        //
        // Enable DSI power and configure DSI clock.
        //
        am_hal_dsi_init();
    }
    else
    {
        return -1;
    }
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);

    nema_sys_init();
    //
    //Initialize NemaDC
    //
    if (nemadc_init() != 0)
    {
        return -2;
    }
    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        uint8_t ui8LanesNum = g_sDispCfg.ui8NumLanes;
        uint8_t ui8DbiWidth = g_sDispCfg.eDbiWidth;
        uint32_t ui32FreqTrim = g_sDispCfg.eDsiFreq;
        pixel_format_t eFormat = FMT_RGB888;
        if (am_hal_dsi_para_config(ui8LanesNum, ui8DbiWidth, ui32FreqTrim, false) != 0)
        {
            return -3;
        }
        switch (eFormat)
        {
            case FMT_RGB888:
                if (ui8DbiWidth == 16)
                {
                    ui32MipiCfg = MIPICFG_16RGB888_OPT0;
                }
                if (ui8DbiWidth == 8)
                {
                    ui32MipiCfg = MIPICFG_8RGB888_OPT0;
                }
                break;

            case FMT_RGB565:
                if (ui8DbiWidth == 16)
                {
                    ui32MipiCfg = MIPICFG_16RGB565_OPT0;
                }
                if (ui8DbiWidth == 8)
                {
                    ui32MipiCfg = MIPICFG_8RGB565_OPT0;
                }
                break;

            default:
                //
                // invalid color component index
                //
                return -3;
        }
    }

    am_devices_dc_dsi_raydium_hardware_reset();
    sDCConfig.eInterface = DISP_INTERFACE_DBIDSI;
    sDCConfig.ui32PixelFormat = ui32MipiCfg;
    sDisplayPanelConfig.ui32PixelFormat = ui32MipiCfg;
    nemadc_configure(&sDCConfig);
    am_devices_dc_dsi_raydium_init(&sDisplayPanelConfig);
#else
    //
    // Initialize display
    //
    am_devices_display_init(FB_RESX,
                            FB_RESY,
                            eDispColor,
                            false);
    //
    // Initialize GPU
    //
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
#endif
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Initialize NemaGFX
    //
    nema_init();
    //
    // Initialize timer for time measurement
    //
    am_hal_timer_config_t sTimerConfig;
    ui32Status = am_hal_timer_default_config_set(&sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to initialize a timer configuration structure with default values!\n");
    }
    sTimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    sTimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    ui32Status = am_hal_timer_config(0, &sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure a timer!\n");
    }
    ui32Status = am_hal_timer_start(0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to start a timer!\n");
    }

    //
    // Run the performace test
    //
    nemagfx_performance_test();

    am_util_stdio_printf("Performance test completes!\n");

    while (1)
    {
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

