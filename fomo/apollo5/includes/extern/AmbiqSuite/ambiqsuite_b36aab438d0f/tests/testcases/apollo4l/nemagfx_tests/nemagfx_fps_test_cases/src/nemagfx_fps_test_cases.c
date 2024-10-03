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
#include "am_devices_mspi_psram_aps25616n.h"
#include "golden_340_rgba565.h"
#include "golden_340_rgb24.h"
#include "golden_340_rgba332.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define RESX            340
#define RESY            340
#define LOOP_CNT        100
#define LIMITED_FPS     (40.0f)

//#define FORMAT     AM_DEVICES_MSPI_RM69330_COLOR_MODE_3BIT
//#define FORMAT     AM_DEVICES_MSPI_RM69330_COLOR_MODE_8BIT
//#define FORMAT    AM_DEVICES_MSPI_RM69330_COLOR_MODE_16BIT
//#define FORMAT    AM_DEVICES_MSPI_RM69330_COLOR_MODE_18BIT
#define FORMAT    AM_DEVICES_MSPI_RM69330_COLOR_MODE_24BIT

#define LOAD_FROM_PSRAM       //undefine this macro will load texture from SSRAM.
#if defined(LOAD_FROM_PSRAM)
#define DMA_METHOD           //undefine this macro load texture use xipmm.
#endif



#define MSPI_PSRAM_MODULE 0
#if (MSPI_PSRAM_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS 0x14000000
#elif (MSPI_PSRAM_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS 0x18000000
#elif (MSPI_PSRAM_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS 0x1C000000
#endif // MSPI_PSRAM_MODULE == 0

static img_obj_t sGolden = {{0},  RESX, RESY, -1, 1, NEMA_RGB565, NEMA_FILTER_PS};
static img_obj_t sFrameBuffer =  {{0},RESX, RESY, -1, 0, NEMA_RGB565, NEMA_FILTER_PS};

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
#if defined(APS25616N_OCTAL_MODE_EN) || (DISPLAY_MSPI_INST == 1)
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
//*****************************************************************************
//
// MSPI DMA finish callback
//
//*****************************************************************************
void
MspiTransferCallback(void *pCallbackCtxt, uint32_t status)
{
    if ( status != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("DMA failed! status=0x%8X\n", status);
    }
    else
    {
        am_util_stdio_printf("DMA END!\n");
    }
}
//*****************************************************************************
//
// MSPI DMA read
//
//*****************************************************************************
void
DMA_load_texture(uint8_t *pui8RxBuffer,uint32_t ui32ReadAddress,uint32_t ui32NumBytes)
{
    am_util_stdio_printf("DMA START!\n");
    uint32_t ui32Ret = am_devices_mspi_psram_aps25616n_ddr_nonblocking_read(g_pDevHandle,\
                                                                            pui8RxBuffer,\
                                                                            ui32ReadAddress,\
                                                                            ui32NumBytes,\
                                                                            MspiTransferCallback,\
                                                                            NULL);
    if ( ui32Ret != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("DMA START failed!\n");
    }
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

#if (FORMAT == AM_DEVICES_MSPI_RM69330_COLOR_MODE_16BIT)
    sGolden.format = NEMA_RGB565;
    sFrameBuffer.format = NEMA_RGB565;
    sGolden.bo = createBuffer((void*)golden_340_rgba565, sizeof(golden_340_rgba565));
#elif (FORMAT == AM_DEVICES_MSPI_RM69330_COLOR_MODE_8BIT)
    sGolden.format = NEMA_RGB332;
    sFrameBuffer.format = NEMA_RGB332;
    sGolden.bo = createBuffer((void*)golden_340_rgba332, sizeof(golden_340_rgba332));
#elif (FORMAT == AM_DEVICES_MSPI_RM69330_COLOR_MODE_24BIT)
    sGolden.format = NEMA_RGB24;
    sFrameBuffer.format = NEMA_RGB24;
    sGolden.bo = createBuffer((void*)golden_340_rgb24, sizeof(golden_340_rgb24));
#elif (FORMAT == AM_DEVICES_MSPI_RM69330_COLOR_MODE_3BIT)
    sGolden.format = NEMA_RGB332;
    sFrameBuffer.format = NEMA_RGB332;
    sGolden.bo = createBuffer((void*)golden_340_rgba332, sizeof(golden_340_rgba332));
#else
#endif
#else //LOAD_FROM_PSRAM

#if (FORMAT == AM_DEVICES_MSPI_RM69330_COLOR_MODE_16BIT)
    sGolden.format = NEMA_RGB565;
    sFrameBuffer.format = NEMA_RGB565;
    sGolden.bo = nema_buffer_create(sizeof(golden_340_rgba565));
    memcpy(sGolden.bo.base_virt,(void*)golden_340_rgba565,sizeof(golden_340_rgba565));
#elif (FORMAT == AM_DEVICES_MSPI_RM69330_COLOR_MODE_8BIT)
    sGolden.format = NEMA_RGB332;
    sFrameBuffer.format = NEMA_RGB332;
    sGolden.bo = nema_buffer_create(sizeof(golden_340_rgba332));
    memcpy(sGolden.bo.base_virt,(void*)golden_340_rgba332,sizeof(golden_340_rgba332));
#elif (FORMAT == AM_DEVICES_MSPI_RM69330_COLOR_MODE_24BIT)
    sGolden.format = NEMA_RGB24;
    sFrameBuffer.format = NEMA_RGB24;
    sGolden.bo = nema_buffer_create(sizeof(golden_340_rgb24));
    memcpy(sGolden.bo.base_virt,(void*)golden_340_rgb24,sizeof(golden_340_rgb24));
#elif (FORMAT == AM_DEVICES_MSPI_RM69330_COLOR_MODE_3BIT)
    sGolden.format = NEMA_RGB332;
    sFrameBuffer.format = NEMA_RGB332;
    sGolden.bo = nema_buffer_create(sizeof(golden_340_rgba332));
    memcpy(sGolden.bo.base_virt,(void*)golden_340_rgba332,sizeof(golden_340_rgba332));
#else
#endif

#endif //LOAD_FROM_PSRAM

#if (FORMAT == AM_DEVICES_MSPI_RM69330_COLOR_MODE_16BIT)
#if !defined(AM_PART_APOLLO4L)
    uint8_t temp[2];
    for(uint32_t i=0;i< sGolden.w * sGolden.h * 2;i += 2)
    {
        temp[0] = *(uint8_t *)(sGolden.bo.base_phys + i);
        temp[1] = *(uint8_t *)(sGolden.bo.base_phys + i+1);
        *(uint8_t *)(sGolden.bo.base_phys + i)= temp[1];
        *(uint8_t *)(sGolden.bo.base_phys + i+1) = temp[0];
    }
#endif //AM_PART_APOLLO4L
#elif (FORMAT == AM_DEVICES_MSPI_RM69330_COLOR_MODE_24BIT)

#elif (FORMAT == AM_DEVICES_MSPI_RM69330_COLOR_MODE_3BIT)
    for(uint32_t i=0;i< sGolden.w * sGolden.h;i++)
    {
        *(uint8_t *)(sGolden.bo.base_phys + i)= 0x11;
    }
#else
#endif
//    nema_buffer_t buffer;
//    buffer = nema_buffer_create(0x50000);
    sFrameBuffer.stride = sFrameBuffer.w * nema_format_size(sFrameBuffer.format);
    sFrameBuffer.bo = nema_buffer_create(sFrameBuffer.stride * sFrameBuffer.h);
#if defined(DMA_METHOD)
    DMA_load_texture(sFrameBuffer.bo.base_virt,sGolden.bo.base_phys,sFrameBuffer.stride * sFrameBuffer.h);
    am_util_delay_ms(1000);
#else
    memcpy(sFrameBuffer.bo.base_virt,sGolden.bo.base_virt,sFrameBuffer.stride * sFrameBuffer.h);
#endif
    g_sCL = nema_cl_create();
}
//*****************************************************************************
//
//! @brief draw texture to destination buffers and display the images.
//!
//! @param None.
//!
//! draw images then transfer via mspi2 QSPI interface to display.
//!
//! @return None.
//
//*****************************************************************************
void
drawImages(void)
{
    //
    // transfer frame to the display
    //
    am_devices_display_transfer_frame(sFrameBuffer.w,
                                      sFrameBuffer.h,
                                      sFrameBuffer.bo.base_phys,
                                      NULL,
                                      NULL);

    nema_cl_rewind(&g_sCL);
    nema_cl_bind(&g_sCL);
    nema_bind_dst_tex(sFrameBuffer.bo.base_phys,
                      RESX,
                      RESY,
                      sFrameBuffer.format,
                      sFrameBuffer.stride);
    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_blit(NEMA_BL_SIMPLE);
    nema_bind_src_tex(sGolden.bo.base_phys,
                      sGolden.w,
                      sGolden.h,
                      sGolden.format,
                      sGolden.stride,
                      NEMA_FILTER_PS);
    nema_blit(0, 0);
    //
    // wait transfer done
    //
    am_devices_display_wait_transfer_done();
    nema_cl_submit(&g_sCL);
    nema_cl_wait(&g_sCL);

}

bool
nemagfx_fps_test()
{
    bool bTestPass = false;
    float start_time,stop_time,fps;
    uint16_t ui16Loop=0;
    uint32_t ui32Status;

#ifdef LOAD_FROM_PSRAM
//    am_devices_mspi_psram_ddr_timing_config_t MSPIDdrTimingConfig;
//    am_util_stdio_printf("Starting MSPI DDR Timing Scan: \n");
//    if ( AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == am_devices_mspi_psram_aps25616n_ddr_init_timing_check(MSPI_PSRAM_MODULE, &MSPI_PSRAM_HexCE0MSPIConfig, &MSPIDdrTimingConfig) )
//    {
//        am_util_stdio_printf("==== Scan Result: RXDQSDELAY0 = %d \n", MSPIDdrTimingConfig.ui32Rxdqsdelay);
//    }
//    else
//    {
//        am_util_stdio_printf("==== Scan Result: Failed, no valid setting. \n");
//    }
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
//    ui32Status = am_devices_mspi_psram_aps25616n_apply_ddr_timing(g_pDevHandle, &MSPIDdrTimingConfig);
//    if (AM_HAL_STATUS_SUCCESS != ui32Status)
//    {
//        am_util_stdio_printf("Failed to apply the timming scan parameter!\n");
//    }
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

#if (FORMAT == AM_DEVICES_MSPI_RM69330_COLOR_MODE_16BIT)
    am_devices_disp_color_e eDispColor = COLOR_FORMAT_RGB565;
#elif (FORMAT == AM_DEVICES_MSPI_RM69330_COLOR_MODE_24BIT)
    am_devices_disp_color_e eDispColor = COLOR_FORMAT_RGB888;
#endif
    //
    // Initialize display
    //
    am_devices_display_init(RESX,
                            RESY,
                            eDispColor,
                            false);

    loadObjects();

#if defined(BAREMETAL) && defined(APOLLO4_FPGA) && defined(AM_PART_APOLLO4L)
	am_hal_stimer_config(AM_HAL_STIMER_HFRC_6MHZ | AM_HAL_STIMER_CFG_RUN);
#endif
    start_time = nema_get_time();
    //start_mspi_xfer(sFrameBuffer.bo.base_phys);
    while(1)
    {
        drawImages();
        //start_mspi_xfer(sFrameBuffer.bo.base_phys);
        //start_mspi_xfer(sGolden.bo.base_phys);
        if(++ui16Loop == LOOP_CNT)
        {
            stop_time = nema_get_time();
            //am_util_stdio_printf("time:%.2f\n",stop_time - start_time);
            break;
        }
    }
    fps = LOOP_CNT/(stop_time-start_time);

#if defined(APOLLO4_FPGA)
    bTestPass = true;
    am_util_stdio_printf("\nFPS is %.2f\n",fps);
#else
    if(fps >= LIMITED_FPS)
    {
        bTestPass = true;
    }
    am_util_stdio_printf("\nFPS %.2f is %s than the minimum tolerance %.2f.\n",fps,bTestPass == true ? "larger" :"less",LIMITED_FPS);
#endif
    TEST_ASSERT_TRUE(bTestPass);
    return bTestPass;
}
