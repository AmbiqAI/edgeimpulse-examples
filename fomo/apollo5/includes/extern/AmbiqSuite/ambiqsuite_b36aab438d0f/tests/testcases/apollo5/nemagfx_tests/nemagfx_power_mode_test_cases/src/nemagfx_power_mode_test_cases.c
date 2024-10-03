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
#include "boy_tsc6.h"
#include "crc32.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define FB_RESX 200
#define FB_RESY 200
#define MAX_CNT 1000
#define EXPECTED_CRC (0x2D3203C0UL)

///*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
static img_obj_t fb = {{0}, FB_RESX, FB_RESY, -1, 0, NEMA_RGB24, 0};
img_obj_t boy_tsc6_img = {{0}, 100, 100, -1, 0, NEMA_TSC6, 0};

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
    fb.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, fb.h * nema_stride_size(fb.format, NEMA_TEX_CLAMP, fb.w));
    nema_buffer_map(&fb.bo);

    boy_tsc6_img.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS,boy_tsc6_length);
    nema_memcpy(boy_tsc6_img.bo.base_virt, boy_tsc6, boy_tsc6_length);
}

//*****************************************************************************
//
//! @brief Test NemaGFX blit feature.
//!
//! @return bTestPass.
//
//*****************************************************************************
void
nemagfx_blit(void)
{
    nema_cmdlist_t cl = nema_cl_create();

    nema_cl_bind(&cl);

    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, fb.stride);

    nema_set_clip(0, 0, FB_RESX, FB_RESY);

    nema_clear(0x0);

    nema_set_blend_blit(NEMA_BL_SRC);

    nema_bind_src_tex(boy_tsc6_img.bo.base_phys, boy_tsc6_img.w, boy_tsc6_img.h, boy_tsc6_img.format, boy_tsc6_img.stride, NEMA_FILTER_PS | NEMA_TEX_CLAMP);

    nema_blit_rect_fit(0, 0, FB_RESX, FB_RESY);

    nema_cl_submit(&cl);
    nema_cl_wait(&cl);
    nema_cl_destroy(&cl);
}

//*****************************************************************************
//
//! @brief GPU blit test case with power mode switch
//!
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
nemagfx_power_mode_test(void)
{
    bool bTestPass = true;
    uint32_t ui32PassCnt = 0;
    float time_lp = 0;
    float time_hp = 0;

    //
    //Set up frame buffer and texture
    //
    load_objects();

    for (uint32_t ui32Cnt = 0; ui32Cnt < MAX_CNT; ui32Cnt++)
    {

        //
        //Power down
        //
        am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);

        //
        //Switch to next power mode.
        //
        am_hal_pwrctrl_gpu_mode_e current_mode;
        am_hal_pwrctrl_gpu_mode_e eGPUMode = (ui32Cnt % 2) ? AM_HAL_PWRCTRL_GPU_MODE_LOW_POWER: AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE;
        am_hal_pwrctrl_gpu_mode_select(eGPUMode);
        am_hal_pwrctrl_gpu_mode_status(&current_mode);
        if(eGPUMode != current_mode)
        {
            am_util_stdio_printf("gpu mode selection failed!\n");
            bTestPass = false;
            break;
        }

        //
        //Power up
        //
        am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
        if(nema_init() != 0)
        {
            am_util_stdio_printf("GPU init failed!\n");
        }

        //
        //Run the blit operation
        //
        float time_start = nema_get_time();
        nemagfx_blit();
        float time_elapsed = nema_get_time() - time_start;
        //
        // We should invalidate the cache before checking data.
        //
        am_hal_cachectrl_range_t Range;
        Range.ui32Size = fb.h * nema_stride_size(fb.format, NEMA_TEX_CLAMP, fb.w);
        Range.ui32StartAddr = fb.bo.base_phys;
        am_hal_cachectrl_dcache_invalidate(&Range, false);
        // Check content
        uint32_t crc32_value = crc32(fb.bo.base_virt, fb.h * nema_stride_size(fb.format, NEMA_TEX_CLAMP, fb.w));
        if (crc32_value != EXPECTED_CRC)
        {
            am_util_stdio_printf("Framebuffer content check failed! Expected:%08X, Got:%08X\n", EXPECTED_CRC, crc32_value);
            bTestPass = false;
            break;
        }

        //Check time
        if(current_mode == AM_HAL_PWRCTRL_GPU_MODE_LOW_POWER)
        {
            time_lp = time_elapsed;
        }
        else
        {
            time_hp = time_elapsed;
        }
        if((ui32Cnt >= 2) && (time_lp < time_hp))
        {
            am_util_stdio_printf("HP mode cost more time than LP! HP:%.4f, Got:%.4f\n", time_hp, time_lp);
            bTestPass = false;
            break;
        }

        //Print
        const char* mode = (current_mode == AM_HAL_PWRCTRL_GPU_MODE_LOW_POWER) ? "low power mode" : "high performance mode";
        am_util_stdio_printf("Current mode: %s, Time used: %.4f seconds.\n", mode, time_elapsed);
        am_util_stdio_printf("GPU power control test passed %d time(s), total number of times set is %d.\n", ++ui32PassCnt, MAX_CNT);

    }

    TEST_ASSERT_TRUE(bTestPass);
    return bTestPass;
}
