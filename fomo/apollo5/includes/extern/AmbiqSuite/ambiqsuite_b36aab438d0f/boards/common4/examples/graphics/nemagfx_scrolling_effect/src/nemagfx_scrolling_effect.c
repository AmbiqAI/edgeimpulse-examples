//*****************************************************************************
//
//! @file nemagfx_scrolling_effect.c
//!
//! @brief NemaGFX Scrolling Effect Example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_scrolling_effect NemaGFX Scrolling Effect Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example demonstates how to implement scrolling effect,it isn't different
//! with nemadc_scolling_effect obviously.the former depend on GPU,the latter depend
//! DC.
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

//*****************************************************************************
//
// This application has a large number of common include files. For
// convenience, we'll collect them all together in a single header and include
// that everywhere.
//
//*****************************************************************************
#include "nemagfx_scrolling_effect.h"
#include "am_devices_mspi_psram_aps25616n.h"
#include "girl_rgba.h"
#include "boy_rgba.h"
#include "nema_utils.h"

#define AM_DEBUG_PRINTF
#define RESX                454
#define RESY                454
#define MSPI_PSRAM_MODULE   0
#define REPEAT_COUNT        1

#if (MSPI_PSRAM_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS 0x14000000
#elif (MSPI_PSRAM_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS 0x18000000
#elif (MSPI_PSRAM_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS 0x1C000000
#endif // #if (MSPI_PSRAM_MODULE == 0)

#define TEX_CNT             2
static uint8_t              ui8Idx = 0;
static img_obj_t            g_sTex[TEX_CNT] = {{{0},  RESX, RESY, -1, 1, NEMA_RGBA8888, NEMA_FILTER_PS},\
                                              {{0},  RESX, RESY, -1, 1, NEMA_RGBA8888, NEMA_FILTER_PS}};
static img_obj_t            g_sFB = {0};
AM_SHARED_RW nema_cmdlist_t g_sCL;

typedef enum
{
    HORIZONTAL,
    VERTICAL
}scrolling_direction_e;

static uintptr_t            g_watch_addr = MSPI_XIP_BASE_ADDRESS;
uint32_t                    g_ui32DMABuffer[2560];
void                        *g_pDevHandle;
void                        *g_pHandle;

am_devices_mspi_psram_config_t MSPI_PSRAM_HexCE1MSPIConfig =
{
#if defined(APS25616N_OCTAL_MODE_EN) || (DISPLAY_MSPI_INST == 1)
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1,
#else
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE1,
#endif
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(g_ui32DMABuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = g_ui32DMABuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

//!< MSPI interrupts.
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
void
psram_mspi_isr(void)
{
    uint32_t ui32Status;

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
create_Buffer(void *ptr, size_t size)
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
//! @brief initialize framebuffer and layer,load texture to PSRAM
//!
//! This function inilialize GPU used framebuffer,DC's layer and load texture to
//! external memories.
//!
//! @return none.
//
//*****************************************************************************
void
load_objects(void)
{
    g_sTex[0].bo = create_Buffer((void*)girl_rgba, sizeof(girl_rgba));
    g_sTex[1].bo = create_Buffer((void*)boy_rgba, sizeof(boy_rgba));

    g_sFB.w = RESX;
    g_sFB.h = RESY;
    g_sFB.color = 0;
    g_sFB.sampling_mode = 0;
    g_sFB.format = NEMA_RGB24;
    g_sFB.stride = nema_format_size(g_sFB.format) * g_sFB.w;
    g_sFB.bo = nema_buffer_create(g_sFB.stride * g_sFB.h);
}
//*****************************************************************************
//
//! @brief implement GPU's scrolling effect.
//!
//! This function could achieve scrolling effect through GPU.
//!
//! @return uint32_t.
//
//*****************************************************************************
void
scrolling_effect(void)
{
    int32_t i32Offset = 0;
    uint8_t ui8Step = 2;
    uint8_t ui8Repeat = 0;
    bool up_or_left_ward = true;
    bool is_first_frame = true;
    scrolling_direction_e scrolling_direction = HORIZONTAL;
    g_sCL = nema_cl_create();
    nema_cl_bind(&g_sCL);

    //
    // horizontial or vertical scrolling effect
    //
    while(1)
    {
        nema_bind_dst_tex(g_sFB.bo.base_phys,
                          g_sFB.w,
                          g_sFB.h,
                          (nema_tex_format_t)(g_sFB.format),
                          g_sFB.stride);

        nema_set_clip(0, 0, RESX, RESY);
        nema_set_blend_blit(NEMA_BL_SRC);

        nema_bind_src_tex(g_sTex[ui8Idx].bo.base_phys,
                          g_sTex[ui8Idx].w,
                          g_sTex[ui8Idx].h,
                          (nema_tex_format_t)(g_sTex[ui8Idx].format),
                          -1,
                          NEMA_FILTER_BL);
        if(scrolling_direction == HORIZONTAL)
        {
            nema_blit(-i32Offset, 0);
        }
        else
        {
            nema_blit(0, -i32Offset);
        }

        nema_bind_src_tex(g_sTex[(ui8Idx+1)%TEX_CNT].bo.base_phys,
                          g_sTex[(ui8Idx+1)%TEX_CNT].w,
                          g_sTex[(ui8Idx+1)%TEX_CNT].h,
                          (nema_tex_format_t)(g_sTex[(ui8Idx+1)%TEX_CNT].format),
                          -1,
                          NEMA_FILTER_BL);

        if(scrolling_direction == HORIZONTAL)
        {
            nema_blit(RESX-i32Offset, 0);
        }
        else
        {
            nema_blit(0, RESY-i32Offset);
        }

        if(is_first_frame == false)
        {
            //
            // wait transfer done
            //
            am_devices_display_wait_transfer_done();
        }

        nema_cl_submit(&g_sCL);

        if(scrolling_direction == HORIZONTAL)
        {
            if(i32Offset >= RESX)
            {
                up_or_left_ward = false;
            }
        }
        else
        {
            if(i32Offset >= RESY)
            {
                up_or_left_ward = false;
            }
        }

        if(i32Offset <= 0)
        {
            up_or_left_ward = true;
            ui8Repeat++;
        }

        up_or_left_ward ? (i32Offset += ui8Step):(i32Offset -= ui8Step);

        nema_calculate_fps();

        nema_cl_wait(&g_sCL);

        //
        // transfer frame to the display
        //
        am_devices_display_transfer_frame(g_sFB.w,
                                          g_sFB.h,
                                          g_sFB.bo.base_phys,
                                          NULL, NULL);
        is_first_frame = false;

        if(ui8Repeat >= REPEAT_COUNT)
        {
            ui8Repeat = 0;
            if(scrolling_direction == HORIZONTAL)
            {
                scrolling_direction = VERTICAL;
            }
            else
            {
                scrolling_direction = HORIZONTAL;
            }
        }

        nema_cl_rewind(&g_sCL);
    }
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
    //
    // External power on
    //
    am_bsp_external_pwr_on();
    am_util_delay_ms(100);
    am_bsp_low_power_init();

    //
    // Initialize the printf interface for ITM output
    //
    am_bsp_itm_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();

    am_util_stdio_printf("NemaGFX scrolling effect.\n\n");

    //
    // color format
    //
    am_devices_disp_color_e eDispColor = COLOR_FORMAT_RGB888;
    //
    // Initialize display
    //
    am_devices_display_init(RESX,
                            RESY,
                            eDispColor,
                            false);

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    //
    // Initialize NemaGFX
    //
    nema_init();
    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_HexCE1MSPIConfig, &g_pDevHandle, &g_pHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_SetPriority(mspi_interrupts[MSPI_PSRAM_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[MSPI_PSRAM_MODULE]);

    am_hal_interrupt_master_enable();
    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pDevHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == ui32Status)
    {
        am_util_stdio_printf("Enable XIP mode in the MSPI!\n");
    }
    else
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }

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

    load_objects();

    scrolling_effect();

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

