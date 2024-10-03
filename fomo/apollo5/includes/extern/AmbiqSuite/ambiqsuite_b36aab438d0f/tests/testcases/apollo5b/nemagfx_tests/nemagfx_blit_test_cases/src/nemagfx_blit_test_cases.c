//*****************************************************************************
//
//! @file nemagfx_blit_test_cases.c
//!
//! @brief NemaGFX blit test cases.
//! Need to connect RM67162 to DC SPI4 interface.
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
#include "Ambiq200x104_rgba.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define FB_RESX 200
#define FB_RESY 200

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
static img_obj_t fb = {{0}, FB_RESX, FB_RESY, -1, 0, NEMA_TSC6, 0};
img_obj_t AM_logo = {{0}, 64, 64, 200 * 4, 0, NEMA_RGBA8888, 0}; // set to 64*64 or 128*128, repeat blit passed.

//As the dump_fb.py script can not display TSC6 format frame buffer, we add this frame buffer to convert the TSC6 format to RGBA8888
//This frame buffer will only be used by dump_fb.py.
static img_obj_t fb_rgba = {{0}, FB_RESX, FB_RESY, -1, 0, NEMA_RGBA8888, 0};

nema_cmdlist_t cl;

uint32_t g_gpu_checksum_table[4] = { 0x00003F39, 0x000046B2, 0x00008D6D,0xFFFFCC56};

bool get_checksum(uint32_t ref_checksum)
{
    bool bPass = true;
    uint32_t * buff;
    uint32_t checksum, i;
    uint32_t size = nema_texture_size(fb.format, 0, fb.w, fb.h);
    buff = (uint32_t *)fb.bo.base_virt;

    //
    // We should invalidate the cache before checking data.
    //
    am_hal_cachectrl_range_t Range;
    Range.ui32Size = size;
    Range.ui32StartAddr = buff;
    am_hal_cachectrl_dcache_invalidate(&Range, false);
    checksum = 0;
    for(i = 0; i < size / 4; i++) // TSC6 format
    {
        checksum += buff[i];
    }
    checksum = (checksum >> 16) + (checksum & 0xffff);
    checksum += (checksum >> 16);
    checksum = 0xffff - checksum;
    if (checksum != ref_checksum)
    {
        am_util_stdio_printf("ERROR! Expected FB checksum is 0x%08X, current checksum is 0x%08X.\n",
                            ref_checksum, checksum);
        bPass = false;
    }
    else
    {
        am_util_stdio_printf("Current FB checksum is equal to expected checksum (0x%08X).\n",
                            ref_checksum);
    }

    return bPass;
}

void
setUp(void)
{
}

void
tearDown(void)
{
}


//*****************************************************************************
//
//! @brief Load memory objects.
//!
//! @return NULL.
//
//*****************************************************************************
void
load_objects(void)
{
    fb.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, fb.w * fb.h * 3);
    nema_buffer_map(&fb.bo);

    fb_rgba.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, fb.w * fb.h * 4);
    nema_buffer_map(&fb_rgba.bo);

    AM_logo.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, Ambiq200x104_rgba_len);
    nema_memcpy(AM_logo.bo.base_virt, Ambiq200x104, Ambiq200x104_rgba_len);
}

//*****************************************************************************
//
//! @brief Test NemaGFX blit feature.
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
nemagfx_blit_test(void)
{
    bool bTestPass = true;
    uint32_t ui32SampleType;
    uint32_t ui32TexColor;

    load_objects();

    //This cl will be used to convert the tsc6 frame buffer to rgba8888
    nema_cmdlist_t cl_convert_tsc6 = nema_cl_create();

    //Create Command Lists
    cl = nema_cl_create();

    //
    // Check Sampling Modes - CLAMP/REPEAT/BORDER/MIRROR
    // MIRROR is not working on the FPGA (probably neither in ASIC)
    // We need to fix this in REV-B chip
    //
    const uint32_t sampling_type[] =
    {
        NEMA_TEX_CLAMP,
        NEMA_TEX_REPEAT,
        NEMA_TEX_BORDER,
        NEMA_TEX_MIRROR
    };
    ui32TexColor = 0xFFBBCCDD; // A-B-G-R


    for(ui32SampleType = 0; ui32SampleType < 4; ui32SampleType++) // Removed MIRROR test
    {
        am_util_stdio_printf("\nNemaGFX blit (sampling type - %d) test.\n", ui32SampleType);

        nema_cl_bind(&cl);
        nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, fb.stride);
        nema_set_clip(0, 0, FB_RESX, FB_RESY);
        nema_set_blend_blit(NEMA_BL_SRC);
        nema_set_tex_color(ui32TexColor);
        nema_bind_src_tex(AM_logo.bo.base_phys, AM_logo.w, AM_logo.h, AM_logo.format, AM_logo.stride, NEMA_FILTER_PS | sampling_type[ui32SampleType]);
        nema_blit_subrect(0, 0, FB_RESX, FB_RESY, -50, -50);
        nema_cl_submit(&cl);
        nema_cl_wait(&cl);
        nema_cl_rewind(&cl);
        if (get_checksum(g_gpu_checksum_table[ui32SampleType]) == false)
        {
            bTestPass = false;
        }


        nema_cl_bind(&cl_convert_tsc6);
        nema_bind_dst_tex(fb_rgba.bo.base_phys, fb_rgba.w, fb_rgba.h, fb_rgba.format, fb_rgba.stride);
        nema_set_clip(0, 0, FB_RESX, FB_RESY);
        nema_clear(0);
        nema_set_blend_blit(NEMA_BL_SRC);
        nema_bind_src_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, fb.stride, NEMA_FILTER_PS);
        nema_blit(0,0);
        nema_cl_submit(&cl_convert_tsc6);
        nema_cl_wait(&cl_convert_tsc6);
        nema_cl_rewind(&cl_convert_tsc6);
    }


    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;

} // nemagfx_blit_test()
