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

TLS_VAR img_obj_t fb = {{0}, (FB_RESX / 4) * 4, (FB_RESY / 4) * 4, -1, 0, NEMA_RGB565, 0};


void
clear_background(uint32_t col)
{
    // Create CmdList
    static TLS_VAR nema_cmdlist_t cl;
    cl = nema_cl_create();
    // Bind CmdList
    nema_cl_bind(&cl);
    // Bind Framebuffer
    nema_bind_dst_tex(fb.bo.base_phys,
                      fb.w,
                      fb.h,
                      (nema_tex_format_t)(fb.format),
                      fb.stride);
    nema_set_clip(0, 0, FB_RESX, FB_RESY);
    // Clear color
    nema_clear(col);

    nema_cl_unbind();
    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    nema_cl_destroy(&cl);
}

void
suite_init()
{
    // Load framebuffer
    fb.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, fb.w*fb.h*4);
    nema_buffer_map(&fb.bo);
    //
    // color format
    //
    am_devices_disp_color_e eDispColor = COLOR_FORMAT_RGB565;
    //
    // Initialize display
    //
    am_devices_display_init(FB_RESX,
                            FB_RESY,
                            eDispColor,
                            false);
}

bool
bench_stop(int testno, int pix_count)
{
    bool bPass = true;
    uint32_t * buff;
    uint32_t checksum, i, ref_checksum;
    buff = (uint32_t *)fb.bo.base_virt;
    checksum = 0;
    for(i = 0; i < FB_RESX * FB_RESY * 2 / 4; i++)
    {
        checksum += buff[i];
    }
    checksum = (checksum >> 16) + (checksum & 0xffff);
    checksum += (checksum >> 16);
    checksum = 0xffff - checksum;
    ref_checksum = 0xFFFF4C97;
    if (checksum != ref_checksum)
    {
        am_util_stdio_printf("Expected checksum is 0x%08X, current checksum is 0x%08X.\n", ref_checksum, checksum);
        bPass = false;
    }
    else
    {
        am_util_stdio_printf("Current checksum is equal to expected checksum (0x%08X).\n", ref_checksum);
    }
    nema_buffer_flush(&fb.bo);
    //
    // transfer frame to the display
    //
    am_devices_display_transfer_frame(fb.w,
                                      fb.h,
                                      fb.bo.base_phys,
                                      NULL,
                                      NULL);
    //
    // wait transfer done
    //
    am_devices_display_wait_transfer_done();
    return bPass;
}

