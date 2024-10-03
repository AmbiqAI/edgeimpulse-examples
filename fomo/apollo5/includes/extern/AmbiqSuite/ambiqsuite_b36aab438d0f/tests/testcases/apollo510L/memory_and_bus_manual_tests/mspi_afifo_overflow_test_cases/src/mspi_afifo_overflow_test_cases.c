//*****************************************************************************
//
//! @file mspi_afifo_overflow_test_cases.c
//!
//! @brief MSPI AFIFO may overflow when GPU accessing XIPMM in the following scenarios:
//!          Using src/dst keying
//!          Using a blending mode where specific pixels do not have alpha value, so they are not written
//!          Using tile mode
//!        This testcase is created for CAB-1268.
//!
//! This test case does not support automatic test!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "string.h"
#include "stdlib.h"
#include "nema_hal.h"
#include "nema_core.h"
#include "nema_utils.h"
#include "nema_programHW.h"
#include "am_devices_mspi_psram_aps25616n.h"
#include "interlaced_texture.h"

//*****************************************************************************
//
// Macros for test items.
//
//*****************************************************************************
#define RESX 200
#define RESY 200

#define MSPI_PSRAM_MODULE              0

#if (MSPI_PSRAM_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS MSPI0_APERTURE_START_ADDR
#elif (MSPI_PSRAM_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS MSPI1_APERTURE_START_ADDR
#elif (MSPI_PSRAM_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS MSPI2_APERTURE_START_ADDR
#endif // #if (MSPI_PSRAM_MODULE == 0)

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
// Global Variables
//
//*****************************************************************************
img_obj_t fb = {{0}, RESX, RESY, RESX, 0, NEMA_RGBA2222, 0};
img_obj_t tex = {{0}, 100, 100, 100*4, 0, NEMA_RGBA8888, 0};

void            *g_pPSRAMHandle;
void            *g_pMSPIHandle;

am_devices_mspi_psram_config_t MSPI_PSRAM_Config =
{

    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_6MHZ,
    .ui32NBTxnBufLength       = 0,
    .pNBTxnBuf                = NULL,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
};

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
}

void
tearDown(void)
{
}

//*****************************************************************************
//
// Optional Global setup/tear-down.
//
// globalSetUp() will get called before the test group starts, and
// globalTearDown() will get called after it ends. These won't be necessary for
// every test.
//
//*****************************************************************************
void
globalSetUp(void)
{
    uint32_t ui32Status;

    //
    // Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    //
    // Print the banner.
    //
    am_bsp_itm_printf_enable();
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("\n\nApollo5 mspi afifo overflow test cases\n\n");

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

}

void
globalTearDown(void)
{
}

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void
psram_mspi_isr(void)
{
    uint32_t ui32Status;

    am_hal_mspi_interrupt_status_get(g_pMSPIHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_pMSPIHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_pMSPIHandle, ui32Status);
}

void
load_objects(void)
{
    //
    // Malloc frame buffer in PSRAM
    //
    fb.bo.base_phys = MSPI_XIP_BASE_ADDRESS;
    fb.bo.base_virt = (void*)fb.bo.base_phys;
    fb.bo.size = nema_texture_size(fb.format, 0, fb.w, fb.h);
    tex.bo.base_phys = MSPI_XIP_BASE_ADDRESS + 0xA000;
    tex.bo.base_virt = (void*)tex.bo.base_phys;
    tex.bo.size = interlaced_texture_length;

    nema_memcpy(tex.bo.base_virt, interlaced_texture, interlaced_texture_length);
}

//*****************************************************************************
//
//! @brief blit frame buffer with green color
//!
//! @return None.
//
//*****************************************************************************
static int32_t
blit()
{
    load_objects();
    for (uint32_t j = 0; j < 1000; j++)
    {
        if ((j%100) == 0)
        {
            am_util_stdio_printf("Test loop %d\n", j);
        }
        uint32_t ui32ResultCheck = 0;
        //Create Command Lists
        nema_cmdlist_t cl  = nema_cl_create();

        //Bind Command List
        nema_cl_bind(&cl);

        //Bind Framebuffer
        nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, fb.stride);
        //Set Clipping Rectangle
        nema_set_clip(0, 0, RESX, RESY);
        //Set Blending Mode
        nema_set_blend_fill(NEMA_BL_SRC);
        //Fill Rectangle with Color
        nema_fill_rect(0, 0, RESX, RESY, 0);

        nema_fill_rect(0, 0, 150, 150, 0xffff0000);

        nema_bind_src_tex(tex.bo.base_phys, tex.w, tex.h, tex.format, tex.stride, NEMA_FILTER_BL);
        nema_set_src_color_key(0xff0f8c30);
        nema_set_blend_blit(NEMA_BL_SRC  | NEMA_BLOP_SRC_CKEY);
        nema_blit(0, 0);
        nema_blit(100, 0);

        uint8_t *ui8RamdomRW = MSPI_XIP_BASE_ADDRESS + 0x40000;

        nema_cl_submit(&cl);

        for(uint16_t i = 0;i < 20; i++)
        {
            ui8RamdomRW[i] = i % 0xFF;
            ui8RamdomRW[i+0x10000] = i % 0x7F;
            ui8RamdomRW[i+0x30000] = i % 0x3F;
            ui8RamdomRW[i+0x20000] = i % 0x1F;
            ui8RamdomRW[i+0x30000] = i % 0x5F;
            ui8RamdomRW[i] = i % 0x1F;
            ui8RamdomRW[i+0x10000] = i % 0x2F;
            ui8RamdomRW[i+0x30000] = i % 0x4F;
            ui8RamdomRW[i+0x20000] = i % 0x6F;
            ui8RamdomRW[i+0x30000] = i % 0x9F;
        }

        nema_cl_wait(&cl);

        am_hal_cachectrl_range_t Range;
        Range.ui32Size = fb.w * fb.h;
        Range.ui32StartAddr = fb.bo.base_phys;
        am_hal_cachectrl_dcache_invalidate(&Range, true);

        uint32_t *pointer = fb.bo.base_phys;
        for(uint32_t i = 0;i< RESX * RESY/4; i++)
        {
            ui32ResultCheck += pointer[i];
        }

        if(ui32ResultCheck != 0xDA4CA5EC)
        {
            //
            // Failed.
            //
            return 1;
        }
        memset(fb.bo.base_phys, 0x00, RESX * RESY);
        am_hal_cachectrl_dcache_invalidate(&Range, true);
        nema_cl_destroy(&cl);
    }
    return 0;
}

//*****************************************************************************
//
// mspi_afifo_overflow_test
//
//*****************************************************************************
bool
mspi_afifo_overflow_test(void)
{
    uint32_t ui32Status;

#ifndef APOLLO5_FPGA  
    //
    // Run MSPI DDR timing scan
    //

    am_devices_mspi_psram_ddr_timing_config_t MSPIDdrTimingConfig;
    am_util_debug_printf("Starting MSPI DDR Timing Scan: \n");
    if ( AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == am_devices_mspi_psram_aps25616n_ddr_init_timing_check(MSPI_PSRAM_MODULE, &MSPI_PSRAM_Config, &MSPIDdrTimingConfig) )
    {
        am_util_stdio_printf("==== Scan Result: RXDQSDELAY0 = %d \n", MSPIDdrTimingConfig.ui32Rxdqsdelay);
    }
    else
    {
        am_util_stdio_printf("==== Scan Result: Failed, no valid setting.  \n");
        return false;
    }
#endif

    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_Config, &g_pPSRAMHandle, &g_pMSPIHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
        return false;
    }
    NVIC_SetPriority(mspi_interrupts[MSPI_PSRAM_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[MSPI_PSRAM_MODULE]);

#ifndef APOLLO5_FPGA 
    //
    // Apply DDR timing setting
    //
    ui32Status = am_devices_mspi_psram_aps25616n_apply_ddr_timing(g_pPSRAMHandle, &MSPIDdrTimingConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to apply the timming scan parameter!\n");
        return false;
    }
#endif

    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPSRAMHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == ui32Status)
    {
        am_util_stdio_printf("Enable XIP mode in the MSPI!\n");
    }
    else
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
        return false;
    }


    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Power up GPU
    //
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

    //
    // Initialize NemaGFX
    //
    nema_init();
    am_util_stdio_printf("----------GPU Blit test!----------\n");
    //
    // Show gradient effect
    //
    if(1 == blit())
    {
        am_util_stdio_printf("Failed!\n");
        return false;
    }
    else
    {
        am_util_stdio_printf("Successed!\n");
    }

    return true;
}