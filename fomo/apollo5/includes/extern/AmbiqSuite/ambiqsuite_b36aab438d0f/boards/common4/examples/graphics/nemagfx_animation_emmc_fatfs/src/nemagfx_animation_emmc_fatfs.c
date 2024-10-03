//*****************************************************************************
//
//! @file nemagfx_animation_emmc_fatfs.c
//!
//! @brief NemaGFX Animation eMMC and FATFS Example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_animation_emmc_fatfs NemaGFX Animation eMMC and FATFS Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: nemagfx_animation_emmc_fatfs is a demo of TSC6 animation images. The
//! program reads the TSC6 image file from EMMC with FatFs file system.
//! It saves frame-buffer space scale of 6:1. it will significantly save RAM use.
//! Note:  the width and height of the frame-buffer should be 4-pixels aligned
//!
//! AM_DEBUG_PRINTF
//! If enabled, debug messages will be sent over ITM.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "nemagfx_animation_emmc_fatfs.h"
#include "main.h"
#include "mmc_apollo4.h"
#include "tusb.h"
#include "am_devices_mspi_psram_aps12808l.h"
#include "watch.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief load texture via USB
//!
//! First step,defined macro ENABLE_USB_TASK ,connected USB cable and load drop.tsc
//! and image.tsc into eMMC.at the same time display nothing.
//! Next step,undefined this macro and rebuild project,the display demostrate loop
//! scrolling animation watchfaces.
//
//*****************************************************************************
//#define ENABLE_USB_TASK

//#define AM_DEBUG_PRINTF

#define IMAGES_DROP_13          13      //0.1487 * 13 = 1.93MB
#define IMAGES_BALL_31          31      //0.1487 * 31 = 4.61MB
#define IMAGES_IMAGE_50         50      //0.1487 * 50 = 7.44MB

#define IMAGES_NAME_DROP    "drop"
#define IMAGES_NAME_BALL    "ball"
#define IMAGES_NAME_IMAGE   "image"

#define THE_SECOND_ANIMATION_OFFSET (IMAGES_IMAGE_50 * IMG_BUF_SIZE_TSC6)
#define LOOP_TIMES          512
//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
FRESULT res;                        /* File function return code */
FATFS eMMCFatFs;                    /* Filesystem object */
FIL TestFile[FRAME_BUFFERS];        /* File object */
char eMMCPath[4];                   /* eMMC logical drive path */
static uint8_t work_buf[FF_MAX_SS]; /* working buffer for format process */
static uint32_t read_cnt;           /* numbers of bytes to read */

#ifdef AM_PART_APOLLO4B
// AXI Scratch buffer
// Need to allocate 20 Words even though we only need 16, to ensure we have 16 Byte alignment
AM_SHARED_RW uint32_t axiScratchBuf[20];
#endif
#ifdef LOAD_IMG_FROM_EMMC_DIRECTLY
uint16_t g_ui16Index[FRAME_BUFFERS] = {0};
uint16_t g_ui16Count[FRAME_BUFFERS] = {0};
#if defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment = 16
    uint8_t g_ui8TSC6img[FRAME_BUFFERS][IMG_BUF_SIZE_TSC6];
#else
    uint8_t g_ui8TSC6img[FRAME_BUFFERS][IMG_BUF_SIZE_TSC6]  __attribute__ ((aligned (16)));
#endif //defined(__IAR_SYSTEMS_ICC__)
#else
uintptr_t pImageStartAddr[FRAME_BUFFERS];
animationImage_t g_sAImages[FRAME_BUFFERS] = {{{0}, NEMADC_TSC6, IMG_BUF_SIZE_TSC6, 0, 0}, \
                                            {{0}, NEMADC_TSC6, IMG_BUF_SIZE_TSC6, 0, 0}};
#endif //LOAD_IMG_FROM_EMMC_DIRECTLY
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

am_devices_mspi_psram_config_t MSPI_PSRAM_OctalCE1MSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,
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
// Initialize eMMC with FafFs filesystem
//
//*****************************************************************************
void
emmc_file_system_init(void)
{
    res = f_mount(&eMMCFatFs, (TCHAR const*)eMMCPath, 1);
    if ( res == FR_OK )
    {
        am_util_stdio_printf("\nFatFs is initialized\n");
    }
    else
    {
        am_util_stdio_printf("Fail to mount the volume\nTry to format the eMMC...\n");
        res = f_mkfs((TCHAR const*)eMMCPath, 0, work_buf, sizeof(work_buf));
        if ( res == FR_OK )
        {
            am_util_stdio_printf("\neMMC is formatted\n");
        }
        else
        {
            am_util_stdio_printf("Fail to format the eMMC\n");
        }
    }
}

#ifdef LOAD_IMG_FROM_EMMC_DIRECTLY
//*****************************************************************************
//
// open files
//
//*****************************************************************************
FRESULT
open_emmc_files(char *prefix, uint8_t order)
{
    char str[32] = {0};
    if ( 0 == memcmp(prefix, IMAGES_NAME_DROP, sizeof(IMAGES_NAME_DROP)) )
    {
        g_ui16Count[order] = IMAGES_DROP_13;
    }
    else if ( 0 == memcmp(prefix, IMAGES_NAME_BALL, sizeof(IMAGES_NAME_BALL)) )
    {
        g_ui16Count[order] = IMAGES_BALL_31;
    }
    else if ( 0 == memcmp(prefix, IMAGES_NAME_IMAGE, sizeof(IMAGES_NAME_IMAGE)) )
    {
        g_ui16Count[order] = IMAGES_IMAGE_50;
    }
    else
    {
        res = FR_NO_FILE;
        return res;
    }
    am_util_stdio_sprintf(str, "%s.tsc", prefix);
    res = f_open(&TestFile[order], str, FA_READ);
    if ( res != FR_OK )
    {
        am_util_stdio_printf("Fail to open file %s.\n", str);
        return res;
    }
    am_util_stdio_printf("succeed to open file %s.\n", str);
    return res;
}

uint8_t *g_loadOnefromEMMC(uint8_t order)
{
    res = f_lseek(&TestFile[order], g_ui16Index[order] * IMG_BUF_SIZE_TSC6);
    if ( res != FR_OK )
    {
        am_util_stdio_printf("Fail to move file pointer!\n");
        return 0;
    }
    res = f_read(&TestFile[order], g_ui8TSC6img[order], IMG_BUF_SIZE_TSC6, (UINT*)&read_cnt);
    if ( (res != FR_OK) || (read_cnt != IMG_BUF_SIZE_TSC6) )
    {
        am_util_stdio_printf("File Read Error!\n");
        return 0;
    }
    if ( ++g_ui16Index[order] >= g_ui16Count[order] )
    {
        g_ui16Index[order] = 0;
    }
    return g_ui8TSC6img[order];
}
#else
//*****************************************************************************
//
// load image files from EMMC
//
//*****************************************************************************
FRESULT
loadImgfromEMMC(char *str, uint16_t idx, uint8_t order)
{
    res = f_open(&TestFile[0], str, FA_READ);
    if ( res == FR_OK )
    {
         res = f_read(&TestFile[0], g_sAImages[order].img[idx], g_sAImages[order].size, (UINT*)&read_cnt);
         if ( (res != FR_OK) || (read_cnt != g_sAImages[order].size) )
         {
           am_util_stdio_printf("File Read Error!\n");
           return res;
         }
         res = f_close(&TestFile[0]);
         if ( res != FR_OK )
         {
           am_util_stdio_printf("Fail to close file\n");
         }
    }
    else
    {
        am_util_stdio_printf("Fail to open file!\n");
    }
    return res;
}

bool
load_from_emmc2psram(void *prefix, uint8_t order)
{
    char suffix[16] = {0}, str[16] = {0};
    memcpy(suffix, ".tsc", sizeof(".tsc"));
    if ( 0 == memcmp(prefix, IMAGES_NAME_DROP, sizeof(IMAGES_NAME_DROP)) )
    {
        g_sAImages[order].cnt = IMAGES_DROP_13;
        pImageStartAddr[order] = g_pWatchAddress;
    }
    else if ( 0 == memcmp(prefix, IMAGES_NAME_BALL, sizeof(IMAGES_NAME_BALL)) )
    {
        g_sAImages[order].cnt = IMAGES_BALL_31;
        pImageStartAddr[order] = g_pWatchAddress;
    }
    else if ( 0 == memcmp(prefix, IMAGES_NAME_IMAGE, sizeof(IMAGES_NAME_IMAGE)) )
    {
        g_sAImages[order].cnt = IMAGES_IMAGE_50;
        pImageStartAddr[order] = g_pWatchAddress + THE_SECOND_ANIMATION_OFFSET;
    }
    else
    {
        return false;
    }
    for ( uint16_t i = 0; i < g_sAImages[order].cnt; i++ )
    {
        am_util_stdio_sprintf(str, "%s_%d%s", prefix, i, suffix);
        g_sAImages[order].img[i] = (void *)(pImageStartAddr[order] + i * g_sAImages[order].size);
        while (loadImgfromEMMC(str, i, order) != FR_OK)
        {
            am_util_stdio_printf("Fail to read image %s from eMMC\n", str);
        }
    }
    return true;
}
#endif //LOAD_IMG_FROM_EMMC_DIRECTLY
//*****************************************************************************
//
// Enable printing to the console.
//
//*****************************************************************************
void
enable_print_interface(void)
{
    //
    // Initialize a debug printing interface.
    //
    am_bsp_debug_printf_enable();
}

//*****************************************************************************
//
// Enable printing to the console.
//
//*****************************************************************************
void
disable_print_interface(void)
{
    //
    // Initialize a debug printing interface.
    //
    am_bsp_debug_printf_disable();
    am_hal_itm_disable();
}

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32Status;
    int16_t i16Offset, i16Loop;
    am_hal_timer_config_t sTimerConfig;
    //
    // Set up scratch AXI buf (needs 64B - aligned to 16 Bytes)
    //
#ifdef AM_PART_APOLLO4B
    am_hal_daxi_control(AM_HAL_DAXI_CONTROL_AXIMEM, (uint8_t *)((uint32_t)(axiScratchBuf + 3) & ~0xF));
#endif
    //
    // External power on
    //
    am_bsp_external_pwr_on();
    am_util_delay_ms(100);
    am_bsp_low_power_init();
    //
    // Configure SDIO PINs.
    //
    am_bsp_sdio_pins_enable(AM_HAL_HOST_BUS_WIDTH_8);
#ifdef FMC_BOARD_EMMC_TEST
    //
    // EMMC daughter board level shift control
    //
    am_hal_gpio_pinconfig(76, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_clear(76);
#endif
    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();
    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

    //
    // Initialize debug printf
    //
    am_bsp_debug_printf_enable();

    am_devices_display_user_setting_t para = {0};
    para.ui16ResX = RESOLUTION_X;
    para.ui16ResY = RESOLUTION_Y;
    para.eColorMode = COLOR_FORMAT_RGB888;
    am_bsp_display_init(&para);

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Initialize NemaGFX
    //
    nema_init();

    am_util_stdio_printf("NemaGFX_SW TSC with EMMC FatFs Example\n");
    //
    // Enable printing to the console.
    //
#ifdef AM_DEBUG_PRINTF
    enable_print_interface();
#endif
    //
    // Init PSRAM device.
    //
    ui32Status = am_devices_mspi_psram_aps12808l_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_OctalCE1MSPIConfig, &g_pDevHandle, &g_pHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    //NVIC_SetPriority(mspi_interrupts[MSPI_PSRAM_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[MSPI_PSRAM_MODULE]);
    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps12808l_ddr_enable_xip(g_pDevHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }
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
    //
    // load watch hands and mask texture to PSRAM
    //
    g_load_objects();
    emmc_file_system_init();
#ifdef LOAD_IMG_FROM_EMMC_DIRECTLY
    open_emmc_files(IMAGES_NAME_IMAGE, 0);
    open_emmc_files(IMAGES_NAME_DROP, 1);
    am_util_stdio_printf("Load Animation Images from EMMC to TCM then to Display(directly)!\n");
#else
    load_from_emmc2psram(IMAGES_NAME_IMAGE, 0);
    load_from_emmc2psram(IMAGES_NAME_DROP, 1);
    am_util_stdio_printf("load Animation Images from EMMC to PSRAM then to Display(indirectly)!\n");
#endif
#ifdef ENABLE_USB_TASK
    tusb_init();
#endif
    while (1)
    {
#ifdef ENABLE_USB_TASK
        tud_task(); // tinyusb device task
#else
        i16Loop = LOOP_TIMES;
        while(i16Loop--)
        {
            g_draw_watch_with_animation(true, 0, 0, STYLE_ONE, STYLE_TWO);
        }
        i16Loop = 2;
        while(i16Loop--)
        {
            for ( i16Offset = 4; i16Offset < RESOLUTION_X; i16Offset += 4 )
            {
                g_draw_watch_with_animation(false, 0, i16Offset, STYLE_ONE, STYLE_TWO);
            }
            for ( i16Offset = RESOLUTION_X - 4; i16Offset > 0; i16Offset -= 4 )
            {
                g_draw_watch_with_animation(false, 0, i16Offset, STYLE_ONE, STYLE_TWO);
            }
        }
        for ( i16Offset = 4; i16Offset < RESOLUTION_X; i16Offset += 4 )
        {
            g_draw_watch_with_animation(false, 0, i16Offset, STYLE_ONE, STYLE_TWO);
        }
        i16Loop = LOOP_TIMES;
        while(i16Loop--)
        {
            g_draw_watch_with_animation(true, 1, 0, STYLE_TWO, STYLE_ONE);
        }
        i16Loop = 2;
        while(i16Loop--)
        {
            for ( i16Offset = 4; i16Offset < RESOLUTION_X; i16Offset += 4 )
            {
                g_draw_watch_with_animation(false, 1, i16Offset, STYLE_TWO, STYLE_ONE);
            }
            for ( i16Offset = RESOLUTION_X - 4; i16Offset > 0; i16Offset -= 4 )
            {
                g_draw_watch_with_animation(false, 1, i16Offset, STYLE_TWO, STYLE_ONE);
            }
        }
        for ( i16Offset = 4; i16Offset < RESOLUTION_X; i16Offset += 4 )
        {
            g_draw_watch_with_animation(false, 1, i16Offset, STYLE_TWO, STYLE_ONE);
        }
#endif
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

