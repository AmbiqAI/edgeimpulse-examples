//*****************************************************************************
//
//! @file utils.c
//!
//! @brief NemaGFX example.
//!
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
#include "utils.h"
#include "dest_data.h"

TLS_VAR img_obj_t fb = {{0}, (FB_RESX / 4) * 4, (FB_RESY / 4) * 4, -1, 0, NEMA_RGB565, 0};

void clear_background(uint32_t col)
{
    // Create CmdList
    static TLS_VAR nema_cmdlist_t cl;
    cl = nema_cl_create();
    // Bind CmdList
    nema_cl_bind(&cl);
    // Bind Framebuffer
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, (nema_tex_format_t)(fb.format), fb.stride);
    nema_set_clip(0, 0, FB_RESX, FB_RESY);
    // Clear color
    nema_clear(col);

    nema_cl_unbind();
    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    nema_cl_destroy(&cl);
}

void suite_init()
{
    // Load framebuffer
    fb.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, fb.w*fb.h*4);
    nema_buffer_map(&fb.bo);
}

bool bench_stop(int testno, int pix_count)
{
    bool bPass = true;
    uint32_t * buff;
    uint32_t * ref_data = (uint32_t * )dest_data;
    uint32_t i;
    uint32_t size = nema_texture_size(fb.format, 0, fb.w, fb.h);
    buff = (uint32_t *)fb.bo.base_virt;    
    //
    // We should invalidate the cache before checking data.
    //
    am_hal_cachectrl_range_t Range;
    Range.ui32Size = size;
    Range.ui32StartAddr = buff;
    am_hal_cachectrl_dcache_invalidate(&Range, false);

    for(i = 0; i < size / 4; i++)
    {
        if (buff[i] != ref_data[i])
        {
            am_util_stdio_printf("ERROR! Expected FB data is 0x%08X, current data is 0x%08X. offset is 0x%08X\n", ref_data[i], buff[i], i);
            bPass = false;
        }
    }

    nema_buffer_flush(&fb.bo);

    return bPass;
}

