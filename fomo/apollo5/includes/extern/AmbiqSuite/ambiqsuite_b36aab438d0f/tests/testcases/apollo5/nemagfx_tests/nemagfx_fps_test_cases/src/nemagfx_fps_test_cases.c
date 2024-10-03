//*****************************************************************************
//
//! @file nemagfx_fps_test_cases.c
//!
//! @brief NemaGFX example.
//! this example demonstrate the Nema GPU and CPU performance use Nema GPU's
//! basic characteristics, we should care about the FPS after each individual
//! test.
//! need a timer to get the accurate time past.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "nemagfx_test_common.h"
#include "am_bsp.h"
#include "nema_core.h"
#include "nema_utils.h"

#include "am_devices_mspi_psram_aps25616n.h"
#include "golden_340_a4.h"
#include "golden_340_a8.h"
#include "golden_340_rgba565.h"

#include "crc32.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define RESX            340
#define RESY            340
#define LOOP_CNT        100
#define EXPECTED_CRC    0xE3ACF6A1UL

#ifdef APOLLO5_FPGA
    #define LIMITED_FPS_PSRAM           (5.0f)
    #define LIMITED_FPS                 (20.0f)
#else
    #define LIMITED_FPS_PSRAM           (40.0f)
    #define LIMITED_FPS                 (50.0f)
#endif

typedef enum
{
    TEX_PSRAM = 0,
    TEX_SSRAM,
    TEX_END,
}texture_location_t;


static img_obj_t sGoldenA4 = {{0},  RESX, RESY, -1, 1, NEMA_A4, NEMA_FILTER_PS};
static img_obj_t sGoldenA8 = {{0},  RESX, RESY, -1, 1, NEMA_A8, NEMA_FILTER_PS};
static img_obj_t sGoldenRGB565 = {{0},  RESX, RESY, -1, 1, NEMA_RGB565, NEMA_FILTER_PS};
static img_obj_t sGoldenA4_psram = {{0},  RESX, RESY, -1, 1, NEMA_A4, NEMA_FILTER_PS};
static img_obj_t sGoldenA8_psram = {{0},  RESX, RESY, -1, 1, NEMA_A8, NEMA_FILTER_PS};
static img_obj_t sGoldenRGB565_psram = {{0},  RESX, RESY, -1, 1, NEMA_RGB565, NEMA_FILTER_PS};
static img_obj_t sFrameBuffer =  {{0},RESX, RESY, RESX * 2, 0, NEMA_RGB565, NEMA_FILTER_PS};
static nema_cmdlist_t g_sCL;

//*****************************************************************************
//
//! @brief allocate PSRAM memory for source data.
//!
//! @param ptr - texture original address.
//! @param size - texture size.
//!
//! allocate PSRAM memory for source data and copy data to target address.
//! move the address pointer to next available position.
//!
//! @return struct of nema_buffer_t.
//
//*****************************************************************************
#ifdef USE_PSRAM
static nema_buffer_t
createBuffer(void *ptr, size_t size)
{
    static uintptr_t g_watch_addr = MSPI_XIP_BASE_ADDRESS;
    nema_buffer_t bo;
    bo.base_phys = g_watch_addr;
    bo.base_virt = (void *)(bo.base_phys);
    bo.size = size;
    nema_memcpy(bo.base_virt,ptr,size);
    g_watch_addr += (bo.size + 7) >> 3 << 3;
    return bo;
}
#endif

//*****************************************************************************
//
//! @brief initialize texture objects.
//!
//! initialize texture and frame buffer objects,create command list.
//!
//! @return None.
//
//*****************************************************************************
void
load_objects()
{
#ifdef USE_PSRAM
    sGoldenRGB565_psram.bo = createBuffer((void*)golden_340_rgba565, sizeof(golden_340_rgba565));
    //nema_memcpy(sGoldenRGB565_psram.bo.base_virt,(void*)golden_340_rgba565,sizeof(golden_340_rgba565));
    sGoldenA4_psram.bo = createBuffer((void*)golden_340_a4, sizeof(golden_340_a4));
    //nema_memcpy(sGoldenA4_psram.bo.base_virt,(void*)golden_340_a4,sizeof(golden_340_a4));
    sGoldenA8_psram.bo = createBuffer((void*)golden_340_a8, sizeof(golden_340_a8));
    //nema_memcpy(sGoldenA8_psram.bo.base_virt,(void*)golden_340_a8,sizeof(golden_340_a8));
#endif

    sGoldenRGB565.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, sizeof(golden_340_rgba565));
    nema_memcpy(sGoldenRGB565.bo.base_virt,(void*)golden_340_rgba565,sizeof(golden_340_rgba565));
    sGoldenA4.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, sizeof(golden_340_a4));
    nema_memcpy(sGoldenA4.bo.base_virt,(void*)golden_340_a4,sizeof(golden_340_a4));
    sGoldenA8.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, sizeof(golden_340_a8));
    nema_memcpy(sGoldenA8.bo.base_virt,(void*)golden_340_a8,sizeof(golden_340_a8));

    sFrameBuffer.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, sFrameBuffer.stride * sFrameBuffer.h);
    g_sCL = nema_cl_create();
}

void
destroy_objects()
{
    nema_buffer_destroy(&sGoldenRGB565.bo);
    nema_buffer_destroy(&sGoldenA4.bo);
    nema_buffer_destroy(&sGoldenA8.bo);

    nema_buffer_destroy(&sFrameBuffer.bo);
    nema_cl_destroy(&g_sCL);
}

//*****************************************************************************
//
// Optional setup/tear-down functions.
//
// These will be called before and after each test function listed below.
//
//*****************************************************************************
void
setUp(void)
{
    load_objects();
}

void
tearDown(void)
{
    destroy_objects();
}

//*****************************************************************************
//
//! @brief draw texture to destination buffers and display the images.
//!
//! @param void.
//!
//! when isDualBuffer is true ,draw texture images with dual buffers.else
//! only simple buffer.
//!
//! @return None.
//
//*****************************************************************************
void
drawImages( img_obj_t* ptr_tex_a4,
            img_obj_t* ptr_tex_a8,
            img_obj_t* ptr_tex_rgb565)
{
    nema_cl_rewind(&g_sCL);
    nema_cl_bind(&g_sCL);
    nema_bind_dst_tex(sFrameBuffer.bo.base_phys, RESX, RESY, sFrameBuffer.format, sFrameBuffer.stride);
    nema_set_clip(0, 0, RESX, RESY);
    nema_clear(0);
    nema_set_blend_blit(NEMA_BL_SIMPLE);
    nema_bind_src_tex(ptr_tex_a8->bo.base_phys,
                      ptr_tex_a8->w,
                      ptr_tex_a8->h,
                      ptr_tex_a8->format,
                      ptr_tex_a8->stride,
                      NEMA_FILTER_PS);
    nema_blit(0, 0);
    nema_bind_src_tex(ptr_tex_a4->bo.base_phys,
                      ptr_tex_a4->w,
                      ptr_tex_a4->h,
                      ptr_tex_a4->format,
                      ptr_tex_a4->stride,
                      NEMA_FILTER_PS);
    nema_blit(0, 0);
    nema_bind_src_tex(ptr_tex_rgb565->bo.base_phys,
                      ptr_tex_rgb565->w,
                      ptr_tex_rgb565->h,
                      ptr_tex_rgb565->format,
                      ptr_tex_rgb565->stride,
                      NEMA_FILTER_PS);
    nema_blit(0, 0);
    nema_cl_submit(&g_sCL);
    nema_cl_wait(&g_sCL);
}

bool
nemagfx_fps_test()
{
    bool bTestPass = true;
    float start_time,stop_time;
    img_obj_t* ptr_tex_a4;
    img_obj_t* ptr_tex_a8;
    img_obj_t* ptr_tex_rgb565;
    float fps_limit;

    texture_location_t tex_location = TEX_PSRAM;
    while(tex_location < TEX_END)
    {
#ifndef USE_PSRAM
        //If PSRAM is not enabled, we will bypass the TEX_PSRAM loop.
        if(tex_location == TEX_PSRAM)
        {
            tex_location ++;
            continue;
        }
#endif

        am_util_stdio_printf("\n\n\n\nLoad Texture from %s!\n", (tex_location == TEX_PSRAM) ? "PSRAM":"SSRAM");

        switch(tex_location)
        {
            case TEX_PSRAM:
                ptr_tex_a8 = &sGoldenA8_psram;
                ptr_tex_a4 = &sGoldenA4_psram;
                ptr_tex_rgb565 = &sGoldenRGB565_psram;
                fps_limit = LIMITED_FPS_PSRAM;
            break;
            case TEX_SSRAM:
                ptr_tex_a8 = &sGoldenA8;
                ptr_tex_a4 = &sGoldenA4;
                ptr_tex_rgb565 = &sGoldenRGB565;
                fps_limit = LIMITED_FPS;
            break;
            default:
                ptr_tex_a8 = &sGoldenA8;
                ptr_tex_a4 = &sGoldenA4;
                ptr_tex_rgb565 = &sGoldenRGB565;
                fps_limit = LIMITED_FPS;
            break;
        }

        start_time = nema_get_time();

        for(uint32_t i=0; i<LOOP_CNT; i++)
        {
            drawImages(ptr_tex_a4, ptr_tex_a8, ptr_tex_rgb565);
        }

        stop_time = nema_get_time();
        //
        // We should invalidate the cache before checking data.
        //
        am_hal_cachectrl_range_t Range;
        Range.ui32Size = sFrameBuffer.stride * sFrameBuffer.h;
        Range.ui32StartAddr = sFrameBuffer.bo.base_phys;
        am_hal_cachectrl_dcache_invalidate(&Range, false);
        //Run the content check
        uint32_t crc_value = crc32(sFrameBuffer.bo.base_virt, sFrameBuffer.stride * sFrameBuffer.h);
        if(crc_value != EXPECTED_CRC)
        {
            am_util_stdio_printf("CRC check failed!Expected: %08X, Got:%08X\n", EXPECTED_CRC, crc_value);
            bTestPass = false;
        }

        TEST_ASSERT_TRUE(bTestPass);

        if(!bTestPass)
        {
            break;
        }

        //Run the fps check
        float fps = LOOP_CNT/(stop_time-start_time);

        if(fps < fps_limit)
        {
            bTestPass = false;
        }

        am_util_stdio_printf("FPS %.2f is %s than the minimum tolerance %.2f.\n",fps,bTestPass == true ? "larger" :"less",fps_limit);
        TEST_ASSERT_TRUE(bTestPass);

        if(!bTestPass)
        {
            break;
        }

        //Run the next test loop
        tex_location ++;
    }
    return bTestPass;
}
