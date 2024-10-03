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

#include "am_devices_mspi_psram_aps25616n.h"
#include "golden_340_a4.h"
#include "golden_340_a8.h"
#include "golden_340_rgba565.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define RESX            340
#define RESY            340
#define LOOP_CNT        100
#define LIMITED_FPS     (40.0f)
#define FRAME_BUFFERS   2
#define DC_IDX          ((ui8GPUBuffersIndex+1)%FRAME_BUFFERS)

#define LOAD_FROM_PSRAM       //undefine this macro will load texture from SSRAM.

#define MSPI_PSRAM_MODULE 0
#if (MSPI_PSRAM_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS 0x14000000
#elif (MSPI_PSRAM_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS 0x18000000
#elif (MSPI_PSRAM_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS 0x1C000000
#endif // MSPI_PSRAM_MODULE == 0

static img_obj_t sGoldenA4 = {{0},  RESX, RESY, -1, 1, NEMA_A4, NEMA_FILTER_PS};
static img_obj_t sGoldenA8 = {{0},  RESX, RESY, -1, 1, NEMA_A8, NEMA_FILTER_PS};
static img_obj_t sGoldenRGB565 = {{0},  RESX, RESY, -1, 1, NEMA_RGB565, NEMA_FILTER_PS};
static img_obj_t sFrameBuffer[FRAME_BUFFERS] =  {{{0},RESX, RESY, RESX * 2, 0, NEMA_RGB565, NEMA_FILTER_PS},\
                                                  {{0},RESX, RESY, RESX * 2, 0, NEMA_RGB565, NEMA_FILTER_PS}};
static nemadc_layer_t sDCLayer = {(void *)0, 0, RESX, RESY, -1, 0, 0, RESX, RESY, 0x80, \
                                 NEMADC_BL_SRC, 0, NEMADC_RGB565, 0, 0, 0, 0, 0,0,0, 0};
static uint8_t ui8GPUBuffersIndex = 0;
static nema_cmdlist_t g_sCL;

#ifdef LOAD_FROM_PSRAM
static uintptr_t g_watch_addr = MSPI_XIP_BASE_ADDRESS;
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
static nema_buffer_t
createBuffer(void *ptr, size_t size)
{
    nema_buffer_t bo;
    bo.base_phys = g_watch_addr;
    bo.base_virt = (void *)(bo.base_phys);
    bo.size = size;
    memcpy(bo.base_virt,ptr,size);
    g_watch_addr += (bo.size + 7) >> 3 << 3;
    return bo;
}
#endif //LOAD_FROM_PSRAM
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
loadObjects()
{
#ifdef LOAD_FROM_PSRAM
    sGoldenRGB565.bo = createBuffer((void*)golden_340_rgba565, sizeof(golden_340_rgba565));
    sGoldenA4.bo = createBuffer((void*)golden_340_a4, sizeof(golden_340_a4));
    sGoldenA8.bo = createBuffer((void*)golden_340_a8, sizeof(golden_340_a8));
#else
    sGoldenRGB565.bo = nema_buffer_create(sizeof(golden_340_rgba565));
    memcpy(sGoldenRGB565.bo.base_virt,(void*)golden_340_rgba565,sizeof(golden_340_rgba565));
    sGoldenA4.bo = nema_buffer_create(sizeof(golden_340_a4));
    memcpy(sGoldenA4.bo.base_virt,(void*)golden_340_a4,sizeof(golden_340_a4));
    sGoldenA8.bo = nema_buffer_create(sizeof(golden_340_a8));
    memcpy(sGoldenA8.bo.base_virt,(void*)golden_340_a8,sizeof(golden_340_a8));
#endif
    for(uint8_t idx = 0;idx < FRAME_BUFFERS;idx++)
    {
        sFrameBuffer[idx].bo = nema_buffer_create(sFrameBuffer[idx].stride * sFrameBuffer[idx].h);
    }
    g_sCL = nema_cl_create();
}
//*****************************************************************************
//
//! @brief draw texture to destination buffers and display the images.
//!
//! @param isDualBuffer - whether or not use dual buffer when drawing.
//!
//! when isDualBuffer is true ,draw texture images with dual buffers.else
//! only simple buffer.
//!
//! @return None.
//
//*****************************************************************************
void
drawImages(bool isDualBuffer)
{
    nema_cl_rewind(&g_sCL);
    nema_cl_bind(&g_sCL);
    nema_bind_dst_tex(sFrameBuffer[ui8GPUBuffersIndex].bo.base_phys, RESX, RESY, sFrameBuffer[ui8GPUBuffersIndex].format, sFrameBuffer[ui8GPUBuffersIndex].stride);
    nema_set_clip(0, 0, RESX, RESY);
    nema_clear(0);
    nema_set_blend_blit(NEMA_BL_SIMPLE);
    nema_bind_src_tex(sGoldenA8.bo.base_phys,
                      sGoldenA8.w,
                      sGoldenA8.h,
                      sGoldenA8.format,
                      sGoldenA8.stride,
                      NEMA_FILTER_PS);
    nema_blit(0, 0);
    nema_bind_src_tex(sGoldenA4.bo.base_phys,
                      sGoldenA4.w,
                      sGoldenA4.h,
                      sGoldenA4.format,
                      sGoldenA4.stride,
                      NEMA_FILTER_PS);
    nema_blit(0, 0);
    nema_bind_src_tex(sGoldenRGB565.bo.base_phys,
                      sGoldenRGB565.w,
                      sGoldenRGB565.h,
                      sGoldenRGB565.format,
                      sGoldenRGB565.stride,
                      NEMA_FILTER_PS);
    nema_blit(0, 0);
    nema_cl_submit(&g_sCL);
    if(isDualBuffer)
    {
        sDCLayer.baseaddr_phys = sFrameBuffer[DC_IDX].bo.base_phys;
        sDCLayer.baseaddr_virt = sFrameBuffer[DC_IDX].bo.base_virt;
        nemadc_set_layer(0, &sDCLayer);

        nemadc_transfer_frame_prepare(false);
        //
        //It's necessary to launch frame manually when TE is disabled.
        //
        nemadc_transfer_frame_launch();
        nemadc_wait_vsync();
        nema_cl_wait(&g_sCL);
        ui8GPUBuffersIndex = DC_IDX;
    }
    else
    {
        nema_cl_wait(&g_sCL);
        sDCLayer.baseaddr_phys = sFrameBuffer[ui8GPUBuffersIndex].bo.base_phys;
        sDCLayer.baseaddr_virt = sFrameBuffer[ui8GPUBuffersIndex].bo.base_virt;
        nemadc_set_layer(0, &sDCLayer);
        nemadc_transfer_frame_prepare(false);
        //
        //It's necessary to launch frame manually when TE is disabled.
        //
        nemadc_transfer_frame_launch();
        nemadc_wait_vsync();
    }
}

bool
nemagfx_fps_test()
{
    bool bTestPass = false;
    float start_time,stop_time,fps;
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_xspi_raydium_config_t sDisplayPanelConfig;
    uint16_t ui16Loop=0;
    uint32_t ui32Status,ui32MipiCfg = MIPICFG_8RGB888_OPT0; //!< default config
    am_hal_timer_config_t sTimerConfig;

#ifdef LOAD_FROM_PSRAM
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
    am_util_stdio_printf("Load Texture from PSRAM!\n");
#else
    am_util_stdio_printf("Load Texture from SSRAM!\n");
#endif

    ui32Status = am_hal_timer_default_config_set(&sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to initialize a timer configuration structure with default values!\n");
    }
    sTimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    sTimerConfig.eFunction = AM_HAL_TIMER_FN_EDGE;
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
    // Set the display region to center
    //
    if (RESX < g_sDispCfg.ui16ResX)
    {
        sDisplayPanelConfig.ui16ResX = RESX;
    }
    else
    {
        sDisplayPanelConfig.ui16ResX = g_sDispCfg.ui16ResX;
    }
    ui16MinX = (g_sDispCfg.ui16ResX - sDisplayPanelConfig.ui16ResX) >> 1;
    ui16MinX = (ui16MinX >> 1) << 1;

    if (RESY < g_sDispCfg.ui16ResY)
    {
        sDisplayPanelConfig.ui16ResY = RESY;
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

    //
    // Initialize the display
    //
    switch (g_sDispCfg.eInterface)
    {
        case DISP_IF_SPI4:
            am_devices_dc_xspi_raydium_hardware_reset();
            sDCConfig.eInterface = DISP_INTERFACE_SPI4;
            sDCConfig.ui32PixelFormat = MIPICFG_1RGB888_OPT0;
            sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
            nemadc_configure(&sDCConfig);
            am_devices_dc_xspi_raydium_init(&sDisplayPanelConfig);
            break;
        case DISP_IF_DSPI:
            am_devices_dc_xspi_raydium_hardware_reset();
            sDCConfig.eInterface = DISP_INTERFACE_DSPI;
            sDCConfig.ui32PixelFormat = MIPICFG_2RGB888_OPT0;
            sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
            nemadc_configure(&sDCConfig);
            am_devices_dc_xspi_raydium_init(&sDisplayPanelConfig);

            break;
        case DISP_IF_QSPI:
            am_devices_dc_xspi_raydium_hardware_reset();
            sDCConfig.eInterface = DISP_INTERFACE_QSPI;
            sDCConfig.ui32PixelFormat = MIPICFG_4RGB888_OPT0;
            sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
            nemadc_configure(&sDCConfig);
            am_devices_dc_xspi_raydium_init(&sDisplayPanelConfig);
            break;
        case DISP_IF_DSI:
            am_devices_dc_dsi_raydium_hardware_reset();
            sDCConfig.eInterface = DISP_INTERFACE_DBIDSI;
            sDCConfig.ui32PixelFormat = ui32MipiCfg;
            sDisplayPanelConfig.ui32PixelFormat = ui32MipiCfg;
            nemadc_configure(&sDCConfig);
            am_devices_dc_dsi_raydium_init(((am_devices_dc_dsi_raydium_config_t *) &sDisplayPanelConfig));
            break;
        default:
            ; //NOP
    }
    loadObjects();
    start_time = nema_get_time();

    while(1)
    {
        drawImages(true);
        if(++ui16Loop == LOOP_CNT)
        {
            stop_time = nema_get_time();
            break;
        }
    }
    fps = LOOP_CNT/(stop_time-start_time);

    if(fps >= LIMITED_FPS)
    {
        bTestPass = true;
    }
    am_util_stdio_printf("\nFPS %.2f is %s than the minimum tolerance %.2f.\n",fps,bTestPass == true ? "larger" :"less",LIMITED_FPS);
    TEST_ASSERT_TRUE(bTestPass);
    return bTestPass;
}
