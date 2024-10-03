//*****************************************************************************
//
//! @file nemagfx_drawing_test_cases.c
//!
//! @brief NemaGFX drawing test cases.
//! Need to connect RM67162 to DC SPI4 interface.
//! Need to run this case 30 times at least for 1 part, and hardware reset or
//! power cycle part before every run.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// This application has a large number of common include files. For
// convenience, we'll collect them all together in a single header and include
// that everywhere.
//
//*****************************************************************************
#include "nemagfx_test_common.h"
#include "utils.h"

extern img_obj_t fb;
TLS_VAR nema_cmdlist_t *cl_cur, cl0, cl1, context_cl;
#define swap_cmd_lists() do {                   \
    cl_cur = (cl_cur == &cl0) ? &cl1 : &cl0;    \
} while(0)

static int premultiply_color;

static int render_frame()
{
    int x0 = 0;
    int y2 = 60;
    int x1 = 0;
    uint32_t col = 0x000000FF;
    int y0 = 0;
    int x2 = 60;
    int y1 = 60;

    if (premultiply_color != 0)
    {
     //   col = nema_premultiply_rgba(col);
    }
    nema_fill_triangle(x0, y0, x1, y1, x2, y2, col);

    int area = (int)(0.5f * nema_abs(x0 * (y1 - y2) +
                               x1 * (y2 - y0) +
                               x2 * (y0 - y1)));

    return area;
}

void bench_fill_tri(int blend)
{
    context_cl = nema_cl_create();
    cl0 = nema_cl_create();
    cl1 = nema_cl_create();
    cl_cur = &cl0;

    nema_cl_bind(&context_cl);
    //Bind Framebuffer
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, (nema_tex_format_t)(fb.format), -1);
    //Set Clipping Rectangle
    nema_set_clip(0, 0, FB_RESX, FB_RESY);

    premultiply_color = blend;

    nema_cl_bind(cl_cur);
    nema_cl_rewind(cl_cur);
    nema_cl_branch(&context_cl);

    render_frame();

    nema_cl_return();
    nema_cl_submit(cl_cur);

    swap_cmd_lists();
    nema_cl_wait(cl_cur);

    nema_cl_bind(cl_cur);
    nema_cl_rewind(cl_cur);
    nema_cl_branch(&context_cl);

    nema_cl_wait(cl_cur);
    swap_cmd_lists();

    nema_cl_wait(cl_cur);

    nema_cl_destroy(&context_cl);
    nema_cl_destroy(&cl0);
    nema_cl_destroy(&cl1);
}
//*****************************************************************************
//
//! @brief Test NemaGFX drawing feature.
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
nemagfx_drawing_test(void)
{
    bool bTestPass = true;
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_xspi_raydium_config_t sDisplayPanelConfig;

    g_sDispCfg.eInterface = DISP_IF_SPI4;
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

    am_devices_dc_xspi_raydium_hardware_reset();
    sDCConfig.eInterface = DISP_INTERFACE_SPI4;
    sDCConfig.ui32PixelFormat = MIPICFG_1RGB888_OPT0;
    sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
    nemadc_configure(&sDCConfig);
    am_devices_dc_xspi_raydium_init(&sDisplayPanelConfig);

    int result = 0;
    suite_init();
    AM_REGVAL(0x40090090) = 0x00000002;

    clear_background(0x0);
    bench_fill_tri(0);
    bTestPass = bench_stop(1, result);
    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;
}
