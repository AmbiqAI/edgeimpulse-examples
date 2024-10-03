//*****************************************************************************
//
//! @file nemagfx_rw_psram_test_cases.c
//!
//! @brief NemaGFX reads/writes PSRAM test cases.
//! Need to connect APS25616 PSRAM card to MSPI0.
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
#include "am_devices_mspi_psram_aps25616n.h"
#include "windmill_200x200_rgba8888.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define ENABLE_XIPMM
#define MSPI_INT_TIMEOUT        (100)
#define DDR_PSRAM_TARGET_ADDRESS 0
#define MSPI_BUFFER_SIZE        (4*1024)  // 4K example buffer size.
#define DEFAULT_TIMEOUT         10000
#define MSPI_TEST_MODULE              0

#if (MSPI_TEST_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS 0x14000000
#elif (MSPI_TEST_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS 0x18000000
#elif (MSPI_TEST_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS 0x1C000000
#endif // #if (MSPI_TEST_MODULE == 0)

#define FB_RESX 200
#define FB_RESY 200
#define CRC_REF 0x967814A2
#define CRC_REF_1 0x9DC001AD
img_obj_t g_sFB = {{0}, FB_RESX, FB_RESY, -1, 0, NEMA_RGBA8888, 0};
img_obj_t g_sWindMillRGBA8888PSRAM = {{0}, FB_RESX, FB_RESY, -1, 0, NEMA_RGBA8888, 0};
img_obj_t g_sWindMillRGBA8888SSRAM = {{0}, FB_RESX, FB_RESY, -1, 0, NEMA_RGBA8888, 0};

uint32_t        DMATCBBuffer[2560];
uint8_t         g_TXBuffer[MSPI_BUFFER_SIZE];
uint8_t         g_RXBuffer[MSPI_BUFFER_SIZE];
void            *g_pDevHandle;
void            *g_pHandle;
bool            g_bDMAFinish;


am_devices_mspi_psram_config_t MSPI_PSRAM_OctalCE0MSPIConfig =
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

//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
};

//
// Take over the interrupt handler for whichever MSPI we're using.
//
#define psram_mspi_isr                                                          \
    am_mspi_isr1(MSPI_TEST_MODULE)
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
// MSPI DMA finish callback
//
//*****************************************************************************
void
MspiTransferCallback(void *pCallbackCtxt, uint32_t status)
{
    g_bDMAFinish = true;
}

//*****************************************************************************
//
// Load objects.
//
//*****************************************************************************
void
load_objects(void)
{
    //
    // CPU memory copy from MRAM to PSRAM
    //
    g_sWindMillRGBA8888PSRAM.bo.base_phys = (uintptr_t)MSPI_XIP_BASE_ADDRESS;
    g_sWindMillRGBA8888PSRAM.bo.base_virt = (void*)g_sWindMillRGBA8888PSRAM.bo.base_phys;
    g_sWindMillRGBA8888PSRAM.bo.size = g_sWindMillRGBA8888PSRAM.w * g_sWindMillRGBA8888PSRAM.h * 4;
    memcpy(g_sWindMillRGBA8888PSRAM.bo.base_virt, windmill_200x200_rgba8888, g_sWindMillRGBA8888PSRAM.bo.size);

    g_sWindMillRGBA8888SSRAM.bo = nema_buffer_create(g_sWindMillRGBA8888SSRAM.w * g_sWindMillRGBA8888SSRAM.h * 4);
    (void)nema_buffer_map(&g_sWindMillRGBA8888SSRAM.bo);

    g_sFB.bo.base_phys = (uintptr_t)MSPI_XIP_BASE_ADDRESS + (g_sWindMillRGBA8888PSRAM.bo.size + 0x100)>>3<<3;
    g_sFB.bo.base_virt = (void*)g_sFB.bo.base_phys;
    g_sFB.bo.size = g_sFB.w * g_sFB.h * 4;
}

//*****************************************************************************
//
// Calculate CRC
//
//*****************************************************************************
static inline void
crc32_start(uint32_t init, void* buffer, uint32_t length)
{
    // Program the CRC engine to compute the crc
    //
    SECURITY->RESULT          = init;
    SECURITY->SRCADDR         = (uint32_t)buffer;
    SECURITY->LEN             = length;
    SECURITY->CTRL_b.FUNCTION = SECURITY_CTRL_FUNCTION_CRC32;
    //
    // Start the CRC
    //
    SECURITY->CTRL_b.ENABLE = 1;
}

static inline void
crc32_wait(void)
{
    while((SECURITY->CTRL & SECURITY_CTRL_ENABLE_Msk) != 0);
}

static inline uint32_t
crc32_get(void)
{
    return SECURITY->RESULT;
}

//*****************************************************************************
//
//! @brief nemagfx_rw_psram_test_cases with both texture and fb in PSRAM
//!
//!
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
nemagfx_rw_psram_test(void)
{
    bool bTestPass = true;
    uint32_t ui32Status;
    uint32_t ui32CRCTexture, ui32CRCMemcpy, ui32CRCDMA, ui32CRCGPUBlit;
    float start_time, end_time;

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

#if defined(BAREMETAL) && defined(APOLLO4_FPGA) && defined(AM_PART_APOLLO4L)
	am_hal_stimer_config(AM_HAL_STIMER_HFRC_6MHZ | AM_HAL_STIMER_CFG_RUN);
#endif
    //
    // Init PSRAM device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_TEST_MODULE, &MSPI_PSRAM_OctalCE0MSPIConfig, &g_pDevHandle, &g_pHandle);
#if defined(APS25616N_OCTAL_MODE_EN) || (DISPLAY_MSPI_INST == 1)
    am_util_stdio_printf("\n octal mode mspi \n");
#else
    am_util_stdio_printf("\n hex mode mspi \n");
#endif
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_EnableIRQ(mspi_interrupts[MSPI_TEST_MODULE]);
    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pDevHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }
    //
    // Load objects
    //
    load_objects();

    nema_cmdlist_t sCL;
    sCL = nema_cl_create();
    nema_cl_bind(&sCL);
    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);
    nema_bind_src_tex(g_sWindMillRGBA8888PSRAM.bo.base_phys,
                      g_sWindMillRGBA8888PSRAM.w,
                      g_sWindMillRGBA8888PSRAM.h,
                      g_sWindMillRGBA8888PSRAM.format,
                      g_sWindMillRGBA8888PSRAM.stride,
                      NEMA_FILTER_PS);
    nema_set_clip(0, 0, FB_RESX, FB_RESY);
    nema_set_blend_blit(NEMA_BL_SRC);
    nema_blit(0, 0);
    //
    // GPU blit from PSRAM to PSRAM
    //
    start_time = nema_get_time();
    nema_cl_submit(&sCL);
    //
    // DMA transfer from PSRAM to SSRAM
    //
    g_bDMAFinish = false;
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_nonblocking_read(g_pDevHandle,
                                                         g_sWindMillRGBA8888SSRAM.bo.base_virt,
                                                         (uint32_t)g_sWindMillRGBA8888PSRAM.bo.base_virt,
                                                         g_sWindMillRGBA8888PSRAM.bo.size,
                                                         MspiTransferCallback,
                                                         NULL);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("ERROR! MSPI non blocking read failed!\n");
        bTestPass = false;
    }
    //
    // wait GPU to finish
    //
    nema_cl_wait(&sCL);
    end_time = nema_get_time();
    am_util_stdio_printf("\nGPU used %f second to blit a 200x200 RGBA8888 image from PSRAM to PSRAM.\n\n", end_time - start_time);
    //
    // wait DMA to finish
    //
    while(g_bDMAFinish == false);

    crc32_start(0xFFFFFFFF, (void *)windmill_200x200_rgba8888, sizeof(windmill_200x200_rgba8888));
    crc32_wait();
    ui32CRCTexture = crc32_get();
    //TEST_ASSERT_EQUAL_HEX32_MESSAGE(CRC_REF, ui32CRCTexture, "Texture CRC checking failed!");
    if (CRC_REF != ui32CRCTexture)
    {
        am_util_stdio_printf("\nTexture(in MRAM) CRC checking failed! Expected 0x%08X, was 0x%08X.\n", CRC_REF, ui32CRCTexture);
        bTestPass = false;
    }
    else
    {
        am_util_stdio_printf("\nTexture(in MRAM) CRC checking passed!\n");
    }

    crc32_start(0xFFFFFFFF, (void *)g_sWindMillRGBA8888PSRAM.bo.base_phys, g_sWindMillRGBA8888PSRAM.bo.size);
    crc32_wait();
    ui32CRCMemcpy = crc32_get();
    //TEST_ASSERT_EQUAL_HEX32_MESSAGE(ui32CRCTexture, ui32CRCMemcpy, "CPU memcpy CRC checking failed!");
    if (ui32CRCTexture != ui32CRCMemcpy)
    {
        am_util_stdio_printf("\nCPU memcpy(from MRAM to PSRAM) CRC checking failed! Expected 0x%08X, was 0x%08X.\n", ui32CRCTexture, ui32CRCMemcpy);
        bTestPass = false;
    }
    else
    {
        am_util_stdio_printf("\nCPU memcpy(from MRAM to PSRAM) CRC checking passed!\n");
    }

    crc32_start(0xFFFFFFFF, (void *)g_sWindMillRGBA8888SSRAM.bo.base_phys, g_sWindMillRGBA8888SSRAM.bo.size);
    crc32_wait();
    ui32CRCDMA = crc32_get();
    //TEST_ASSERT_EQUAL_HEX32_MESSAGE(ui32CRCMemcpy, ui32CRCDMA, "DMA CRC checking failed!");
    if (ui32CRCMemcpy != ui32CRCDMA)
    {
        am_util_stdio_printf("\nDMA(from PSRAM to SSRAM) CRC checking failed! Expected 0x%08X, was 0x%08X.\n", ui32CRCMemcpy, ui32CRCDMA);
        bTestPass = false;
    }
    else
    {
        am_util_stdio_printf("\nDMA(from PSRAM to SSRAM) CRC checking passed!\n");
    }

    crc32_start(0xFFFFFFFF, (void *)g_sFB.bo.base_phys, g_sFB.bo.size);
    crc32_wait();
    ui32CRCGPUBlit = crc32_get();
    //TEST_ASSERT_EQUAL_HEX32_MESSAGE(ui32CRCMemcpy, ui32CRCGPUBlit, "GPU blit CRC checking failed!");
    if (ui32CRCMemcpy != ui32CRCGPUBlit)
    {
        am_util_stdio_printf("\nGPU blit(from PSRAM to PSRAM) CRC checking failed! Expected 0x%08X, was 0x%08X.\n\n", ui32CRCDMA, ui32CRCGPUBlit);
        bTestPass = false;
    }
    else
    {
        am_util_stdio_printf("\nGPU blit(from PSRAM to PSRAM) CRC checking passed!\n\n");
    }

    nema_cl_unbind();
    nema_cl_destroy(&sCL);

    crc32_start(0xFFFFFFFF, (void *)windmill_200x200_rgba8888, sizeof(windmill_200x200_rgba8888) / 5); // 1/5 texture in MRAM
    crc32_wait();
    ui32CRCTexture = crc32_get();
    if (CRC_REF_1 != ui32CRCTexture)
    {
        am_util_stdio_printf("\n1/5 Texture(in MRAM) CRC checking failed! Expected 0x%08X, was 0x%08X.\n", CRC_REF_1, ui32CRCTexture);
        bTestPass = false;
    }
    else
    {
        am_util_stdio_printf("\n1/5 Texture(in MRAM) CRC checking passed!\n");
    }

    crc32_start(0xFFFFFFFF, (void *)g_sWindMillRGBA8888PSRAM.bo.base_phys, g_sWindMillRGBA8888PSRAM.bo.size / 5); // 1/5 texture in PSRAM
    crc32_wait();
    ui32CRCMemcpy = crc32_get();
    if (ui32CRCTexture != ui32CRCMemcpy)
    {
        am_util_stdio_printf("\nCPU memcpy 1/5 texture (from MRAM to PSRAM) CRC checking failed! Expected 0x%08X, was 0x%08X.\n", ui32CRCTexture, ui32CRCMemcpy);
        bTestPass = false;
    }
    else
    {
        am_util_stdio_printf("\nCPU memcpy 1/5 texture (from MRAM to PSRAM) CRC checking passed!\n");
    }

    TEST_ASSERT_TRUE(bTestPass);
    return bTestPass;
} // nemagfx_rw_psram_test
