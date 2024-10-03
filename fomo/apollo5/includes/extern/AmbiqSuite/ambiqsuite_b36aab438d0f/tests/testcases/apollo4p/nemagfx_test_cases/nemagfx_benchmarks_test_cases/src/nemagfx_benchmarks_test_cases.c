//*****************************************************************************
//
//! @file main.c
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

// -----------------------------------------------------------------------------
// Copyright (c) 2019 Think Silicon S.A.
// Think Silicon S.A. Confidential Proprietary
// -----------------------------------------------------------------------------
//     All Rights reserved - Unpublished -rights reserved under
//         the Copyright laws of the European Union
//
//  This file includes the Confidential information of Think Silicon S.A.
//  The receiver of this Confidential Information shall not disclose
//  it to any third party and shall protect its confidentiality by
//  using the same degree of care, but not less than a reasonable
//  degree of care, as the receiver uses to protect receiver's own
//  Confidential Information. The entire notice must be reproduced on all
//  authorised copies and copies may only be made to the extent permitted
//  by a licensing agreement from Think Silicon S.A..
//
//  The software is provided 'as is', without warranty of any kind, express or
//  implied, including but not limited to the warranties of merchantability,
//  fitness for a particular purpose and noninfringement. In no event shall
//  Think Silicon S.A. be liable for any claim, damages or other liability, whether
//  in an action of contract, tort or otherwise, arising from, out of or in
//  connection with the software or the use or other dealings in the software.
//
//
//                    Think Silicon S.A.
//                    http://www.think-silicon.com
//                    Patras Science Park
//                    Rion Achaias 26504
//                    Greece
// -----------------------------------------------------------------------------
#include "nemagfx_test_common.h"
#include "am_bsp.h"
#include "nema_core.h"
#include "nema_utils.h"
#ifndef DONT_USE_NEMADC
#include "nema_dc.h"
#include "nema_dc_mipi.h"
#endif

#include "utils.h"
#include "bench.h"
#include "am_devices_mspi_psram_aps25616n.h"

int bench_fill_quad(int blend);
int bench_fill_rect(int blend);
int bench_fill_tri(int blend);

int bench_draw_circle(int blendmode);
int bench_draw_line(int blendmode);
int bench_draw_rect(int blendmode);
int bench_draw_string(int blendmode);

int bench_blit(int blendmode, int rotation);
int bench_stretch_blit(int blendmode, float scale, int nema_tex_mode);
int bench_stretch_blit_rotate(int blendmode, float scale, int nema_tex_mode);

int bench_textured_tri(int blendmode, int nema_tex_mode);
int bench_textured_quad(int blendmode, int nema_tex_mode);

#ifndef DEFAULT_EXEC_MODE
#define DEFAULT_EXEC_MODE CPU_GPU
#endif

TLS_VAR AM_SHARED_RW nema_cmdlist_t *cl_cur, cl0, cl1, context_cl;
execution_mode exec_mode = DEFAULT_EXEC_MODE;

extern float start_wall, stop_wall;

#define MSPI_PSRAM_MODULE 0

//#ifdef LOAD_FROM_PSRAM

uint32_t        DMATCBBuffer[2560];
void            *g_pDevHandle;
void            *g_pHandle;

//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,

};

am_devices_mspi_psram_config_t MSPI_PSRAM_HexCE0MSPIConfig =
{
#ifdef APS25616N_OCTAL_MODE_EN
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,
#else
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
#endif
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
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
void psram_mspi_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_pHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_pHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_pHandle, ui32Status);
}

//#endif // LOAD_FROM_PSRAM

static int
render_frame()
{
  return 0;
}

static int
test_irq(int mode)
{

    cl0 = nema_cl_create();
    nema_cl_bind(&cl0);

    //just some dummy instructions
    nema_set_blend_fill(NEMA_BL_SRC);


    start_wall = nema_get_time();
    stop_wall  = start_wall;


    int cl_count = 0;

    do
    {
        do
        {
            nema_cl_submit(&cl0);
            if (mode == 0)
            {
                nema_cl_wait(&cl0);
            }
            ++cl_count;
        } while ( cl_count % 1000 != 0 );

        stop_wall = nema_get_time();

    } while ( (stop_wall - start_wall) < TIMEOUT_S );

    nema_cl_destroy(&cl0);

    return cl_count;
}

void run_bench(int testno)
{
    suite_init();

    int result = 0;

#warning fixme
    // AM_REGVAL(0x40090090) = 0x00000002;

    switch (testno)
    {
        case 37:
            bench_start(testno);
            result = test_irq(0);
            bench_stop(testno, result);
            break;
        case 38:
            bench_start(testno);
            result = test_irq(1);
            bench_stop(testno, result);
            break;

        case 1:
            bench_start(testno);
            result = bench_fill_tri(0);
            bench_stop(testno, result);
            break;

        case 2:
            bench_start(testno);
            result = bench_fill_tri(1);
            bench_stop(testno, result);
            break;

        case 3:
            bench_start(testno);
            result = bench_fill_rect(0);
            bench_stop(testno, result);
            break;

        case 4:
            bench_start(testno);
            result = bench_fill_rect(1);
            bench_stop(testno, result);
            break;

        case 5:
            bench_start(testno);
            result = bench_fill_quad(0);
            bench_stop(testno, result);
            break;

        case 6:
            bench_start(testno);
            result = bench_fill_quad(1);
            bench_stop(testno, result);
            break;

        case 7:

            bench_start(testno);
            //am_util_stdio_printf("skip this.");
            result = bench_draw_string(NEMA_BL_SRC);
            bench_stop(testno, result);
            break;

        case 8:
            bench_start(testno);
            result = bench_draw_line(0);
            bench_stop(testno, result);
            break;

        case 9:
            bench_start(testno);
            result = bench_draw_line(1);
            bench_stop(testno, result);
            break;

        case 10:
            bench_start(testno);
            result = bench_draw_rect(0);
            bench_stop(testno, result);
            break;

        case 11:
            bench_start(testno);
            result = bench_draw_rect(1);
            bench_stop(testno, result);
            break;

        case 12:
            bench_start(testno);
            result = bench_blit(NEMA_BL_SRC, NEMA_ROT_000_CCW);
            bench_stop(testno, result);
            break;

        case 13:
            bench_start(testno);
            result = bench_blit(NEMA_BL_SRC | NEMA_BLOP_MODULATE_RGB, NEMA_ROT_000_CCW);
            bench_stop(testno, result);
            break;

        case 14:
            bench_start(testno);
            result = bench_blit(NEMA_BL_SIMPLE, NEMA_ROT_000_CCW);
            bench_stop(testno, result);
            break;

        case 15:
            bench_start(testno);
            result = bench_blit(NEMA_BL_SIMPLE | NEMA_BLOP_MODULATE_RGB, NEMA_ROT_000_CCW);
            bench_stop(testno, result);
            break;

        case 16:
            bench_start(testno);
            result = bench_blit(NEMA_BL_SRC, NEMA_ROT_090_CW);
            bench_stop(testno, result);
            break;

        case 17:
            bench_start(testno);
            result = bench_blit(NEMA_BL_SRC, NEMA_ROT_180_CW);
            bench_stop(testno, result);
            break;

        case 18:
            bench_start(testno);
            result = bench_blit(NEMA_BL_SRC, NEMA_ROT_270_CW);
            bench_stop(testno, result);
            break;

        case 19:
            bench_start(testno);
            result = bench_blit(NEMA_BL_SRC, NEMA_MIR_VERT);
            bench_stop(testno, result);
            break;

        case 20:
            bench_start(testno);
            result = bench_blit(NEMA_BL_SRC, NEMA_MIR_HOR);
            bench_stop(testno, result);
            break;

        case 21:
            bench_start(testno);
            result = bench_blit(NEMA_BL_SRC | NEMA_BLOP_SRC_CKEY, NEMA_ROT_000_CCW);
            bench_stop(testno, result);
            break;

        case 22:
            bench_start(testno);
            result = bench_blit(NEMA_BL_SRC | NEMA_BLOP_DST_CKEY, NEMA_ROT_000_CCW);
            bench_stop(testno, result);
            break;

        case 23:
            bench_start(testno);
            result = bench_stretch_blit(NEMA_BL_SRC, 1.5, NEMA_FILTER_PS);
            bench_stop(testno, result);
            break;

        case 24:
            bench_start(testno);
            result = bench_stretch_blit(NEMA_BL_SIMPLE, 1.5, NEMA_FILTER_PS);
            bench_stop(testno, result);
            break;

        case 25:
            bench_start(testno);
            result = bench_stretch_blit(NEMA_BL_SRC, 1.5, NEMA_FILTER_BL);
            bench_stop(testno, result);
            break;

        case 26:
            bench_start(testno);
            result = bench_stretch_blit(NEMA_BL_SIMPLE, 1.5, NEMA_FILTER_BL);
            bench_stop(testno, result);
            break;

        case 27:
            bench_start(testno);
            result = bench_stretch_blit_rotate(NEMA_BL_SRC, 0.75, NEMA_FILTER_PS);
            bench_stop(testno, result);
            break;

        case 28:
            bench_start(testno);
            result = bench_stretch_blit_rotate(NEMA_BL_SRC, 0.75, NEMA_FILTER_BL);
            bench_stop(testno, result);
            break;

        case 29:
            bench_start(testno);
            result = bench_textured_tri(NEMA_BL_SRC, NEMA_FILTER_PS);
            bench_stop(testno, result);
            break;

        case 30:
            bench_start(testno);
            result = bench_textured_tri(NEMA_BL_SIMPLE, NEMA_FILTER_PS);
            bench_stop(testno, result);
            break;

        case 31:
            bench_start(testno);
            result = bench_textured_tri(NEMA_BL_SRC, NEMA_FILTER_BL);
            bench_stop(testno, result);
            break;

        case 32:
            bench_start(testno);
            result = bench_textured_tri(NEMA_BL_SIMPLE, NEMA_FILTER_BL);
            bench_stop(testno, result);
            break;

        case 33:
            bench_start(testno);
            result = bench_textured_quad(NEMA_BL_SRC, NEMA_FILTER_PS);
            bench_stop(testno, result);
            break;

        case 34:
            bench_start(testno);
            result = bench_textured_quad(NEMA_BL_SIMPLE, NEMA_FILTER_PS);
            bench_stop(testno, result);
            break;

        case 35:
            bench_start(testno);
            result = bench_textured_quad(NEMA_BL_SRC, NEMA_FILTER_BL);
            bench_stop(testno, result);
            break;

        case 36:
            bench_start(testno);
            result = bench_textured_quad(NEMA_BL_SIMPLE, NEMA_FILTER_BL);
            bench_stop(testno, result);
            break;
        default:
            return;
    }

    suite_terminate();
}

static int init(void)
{
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_xspi_raydium_config_t sDisplayPanelConfig;

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

#if defined(ENABLE_DSI)
    am_util_stdio_printf("Enable DSI Interface\n");
    g_sDispCfg.eInterface = DISP_IF_DSI;
    am_bsp_disp_pins_enable();

    am_devices_dc_dsi_raydium_hardware_reset();
    sDCConfig.eInterface = DISP_INTERFACE_DBIDSI;
    sDCConfig.ui32PixelFormat = MIPICFG_16RGB888_OPT0;
    sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat ;
    nemadc_configure(&sDCConfig);
    am_devices_dc_dsi_raydium_init(((am_devices_dc_dsi_raydium_config_t *) &sDisplayPanelConfig));
#else
    am_util_stdio_printf("Enable SPI4 Interface\n");
    g_sDispCfg.eInterface = DISP_IF_SPI4;
    am_bsp_disp_pins_enable();

    am_devices_dc_xspi_raydium_hardware_reset();
    sDCConfig.eInterface = DISP_INTERFACE_SPI4;
    sDCConfig.ui32PixelFormat = MIPICFG_1RGB888_OPT0;
    sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
    nemadc_configure(&sDCConfig);
    am_devices_dc_xspi_raydium_init(&sDisplayPanelConfig);
#endif //defined(ENABLE_DSI)

    am_hal_timer_config_t sTimerConfig;
    uint32_t ui32Status;
//#ifdef LOAD_FROM_PSRAM
    //
    // Init PSRAM device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_HexCE0MSPIConfig, &g_pDevHandle, &g_pHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    //NVIC_SetPriority(mspi_interrupts[MSPI_PSRAM_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[MSPI_PSRAM_MODULE]);
    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pDevHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }
//#endif // LOAD_FROM_PSRAM

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
    return 0;
}

bool
nemagfx_benchmarks_test()
{
    int testno = 0;
    uint32_t i;
    bool bTestPass = true;
    int test;

    init();
    for (i = 0; i < TEST_NUM; i++)
    {
        sCurrentCfg = sTestCfg[i];
        am_util_stdio_printf("\n----------------------------\n");
        am_util_stdio_printf("the %d loop, total %d loops.\n",i+1,TEST_NUM);
        am_util_stdio_printf("----------------------------\n\n");
        if (sCurrentCfg.eTexLocation == TEX_PSRAM)
        {
            am_util_stdio_printf("Texture location: \tPSRAM\n");
        }
        else if (sCurrentCfg.eTexLocation == TEX_SSRAM)
        {
            am_util_stdio_printf("Texture location: \tSSRAM\n");
        }
        else
        {
            am_util_stdio_printf("Invalid texture location setting!\n");
        }

        if (sCurrentCfg.eFbLocation == FB_PSRAM)
        {
            am_util_stdio_printf("FB location: \tPSRAM\n");
        }
        else if (sCurrentCfg.eFbLocation == FB_SSRAM)
        {
            am_util_stdio_printf("FB location: \tSSRAM\n");
        }
        else
        {
            am_util_stdio_printf("Invalid FB location setting!\n");
        }

        am_util_stdio_printf("WCache: \t\t%s\n", sCurrentCfg.eWcache == WCACHE_ON ? "ON" : "OFF");
        am_util_stdio_printf("Tiling: \t\t%s\n", sCurrentCfg.eTiling == TILING_ON ? "ON" : "OFF");
        if(sCurrentCfg.eTiling == TILING_ON)
        {
            am_util_stdio_printf("Please note that \"Tiling on\" means we only enabled tiling for items #16, #18, #27~#36, because only these items benefit from \"Tiling on\" according to suggestion.\n\n");
        }
        if ( testno != 0 )
        {
            run_bench(testno);
        }
        else
        {
            for (test = 1; test <= TEST_MAX; ++test )
            {
                run_bench(test);
            }
        }
    }

    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;
}
