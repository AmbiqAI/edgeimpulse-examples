//*****************************************************************************
//
//! @file gpu_pwrctrl_test_cases.c
//!
//! @brief GPU power control test cases.
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
#define MAX_CNT 1000
#define EXPECTED_CHECKSUM (0x00003F39UL)

///*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
static img_obj_t fb = {{0}, FB_RESX, FB_RESY, -1, 0, NEMA_TSC6, 0};
img_obj_t AM_logo = {{0}, 64, 64, 200 * 4, 0, NEMA_RGBA8888, 0}; // set to 64*64 or 128*128, repeat blit passed.

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

    AM_logo.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, Ambiq200x104_rgba_len);
    nema_memcpy(AM_logo.bo.base_virt, Ambiq200x104, Ambiq200x104_rgba_len);
}

bool get_checksum(uint32_t ref_checksum)
{
    bool bPass = true;
    uint32_t * buff;
    uint32_t checksum, i;
    uint32_t size = nema_texture_size(fb.format, 0, fb.w, fb.h);
    buff = (uint32_t *)fb.bo.base_virt;
    checksum = 0;

    //
    // We should invalidate the cache before checking data.
    //
    am_hal_cachectrl_range_t Range;
    Range.ui32Size = size;
    Range.ui32StartAddr = buff;
    am_hal_cachectrl_dcache_invalidate(&Range, false);

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

//*****************************************************************************
//
//! @brief Test NemaGFX blit feature.
//!
//! @return bTestPass.
//
//*****************************************************************************
static void
nemagfx_blit(void)
{
    nema_cmdlist_t cl = nema_cl_create();
    //
    //Bind Command List
    //
    nema_cl_bind(&cl);
    //
    //Bind Framebuffer
    //
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, fb.stride);
    //
    //Set Clipping Rectangle
    //
    nema_set_clip(0, 0, FB_RESX, FB_RESY);

    nema_set_blend_blit(NEMA_BL_SRC);

    nema_bind_src_tex(AM_logo.bo.base_phys, AM_logo.w, AM_logo.h, AM_logo.format, AM_logo.stride, NEMA_FILTER_PS | NEMA_TEX_CLAMP);

    nema_blit_subrect(0, 0, FB_RESX, FB_RESY, -50, -50);

    nema_cl_submit(&cl);
    nema_cl_wait(&cl);
    nema_cl_destroy(&cl);
} // nemagfx_blit_test()

//*****************************************************************************
//
//! @brief GPU blit test case with power control
//!
//! 
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
nemagfx_pwrctrl_test(void)
{
    bool bTestPass = true; 
    uint32_t ui32Cnt = 0, ui32PassCnt = 0;

    load_objects();

    for (ui32Cnt = 0; ui32Cnt < MAX_CNT; ui32Cnt++)
    {
        am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
        //
        // Initialize NemaGFX
        //
        if(nema_init() != 0)
        {
            am_util_stdio_printf("GPU init failed!\n");
        }

        nemagfx_blit();

        bTestPass = get_checksum(EXPECTED_CHECKSUM);

        if(bTestPass)
        {
            am_util_stdio_printf("GPU power control test passed %d time(s), total number of times set is %d.\n", ++ui32PassCnt, MAX_CNT);
        }
        else
        {
            break;
        }

        am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);
    }

    nema_buffer_destroy(&fb.bo);
    nema_buffer_destroy(&AM_logo.bo);

    TEST_ASSERT_TRUE(bTestPass);
    return bTestPass;
}
