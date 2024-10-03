//*****************************************************************************
//
//! @file nemagfx_tsc_fb_psram_xipmm.c
//!
//! @brief NemaGFX example.
//! Nemagfx_tsc_fb_psram_xipmm is a demo of TSC frame-buffer compression with
//! XIPMM support. The program uses TSC4-compressed frame-buffer during run-time.
//! It saves frame-buffer space in RAM in a scale of 1:4 also.The demo use example
//! NEMADC_TSC4 frame buffer shows a 400x400 TSC4 image on the screen, it will
//! significantly save RAM use.
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

//*****************************************************************************
//
// This application has a large number of common include files. For
// convenience, we'll collect them all together in a single header and include
// that everywhere.
//
//*****************************************************************************
#include "nemagfx_tsc_fb_psram_xipmm.h"

#include "am_devices_mspi_psram_aps25616n.h"
#include "am_devices_mspi_psram_aps25616n.c"


#include "oli_400x400_tsc4.h"

#define AM_DEBUG_PRINTF

#define MSPI_BUFFER_SIZE        (4*1024)  //!< 4K example buffer size.

#define RESX 388
#define RESY 388

static img_obj_t g_sFB = {{0}, RESX, RESY, -1, 0, NEMA_TSC4, 0};
img_obj_t g_sTSiLogo = {{0}, 400, 400, -1, 0, NEMA_TSC4, 0};

nemadc_layer_t g_sDCLayer = {(void *)0, 0, RESX, RESY, -1, 0, 0, RESX, RESY, 0xff, NEMADC_BL_SRC, 0, NEMADC_TSC4, 0, 0, 0, 0, 0, 0, 0, 0};

//uint32_t        g_ui32DMABuffer[2560];
uint8_t         g_ui8TXBuffer[MSPI_BUFFER_SIZE];
uint8_t         g_ui8RXBuffer[MSPI_BUFFER_SIZE];
void            *g_pDevHandle;
void            *g_pHandle;


am_devices_mspi_psram_config_t MSPI_PSRAM_HexCE1MSPIConfig =
{
#ifdef APOLLO5_FPGA
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,
#else
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
#endif
    .eClockFreq               = AM_HAL_MSPI_CLK_48MHZ,
    .ui32NBTxnBufLength       = 0,
    .pNBTxnBuf                = NULL,
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


#ifdef ENABLE_XIPMM
//*****************************************************************************
//
// XIPMM check
//
//*****************************************************************************
bool bDoScrambling = true;


bool
run_mspi_xipmm(uint32_t block, bool bUseWordAccesses)
{
    uint32_t ix;

    if ( bUseWordAccesses )
    {
        // Use word accesses if scrambled.
        uint32_t *pAddr1 = (uint32_t *)(MSPI_XIPMM_BASE_ADDRESS);
        uint32_t *pAddr2 = (uint32_t *)(MSPI_XIPMM_BASE_ADDRESS  + 512);

        // Initialize a pattern
        for (ix = 0; ix < 512 / 4; ix++)
        {
            *pAddr1++ = ix;
            *pAddr2++ = ix ^ 0xFFFFFFFF;
        }
        pAddr1 = (uint32_t *)(MSPI_XIPMM_BASE_ADDRESS);
        pAddr2 = (uint32_t *)(MSPI_XIPMM_BASE_ADDRESS + 512);

        // Verify the pattern
        for (ix = 0; ix < 512 / 4; ix++)
        {
            if ( (*pAddr1++ != ix) || (*pAddr2++ != (ix ^ 0xFFFFFFFF)) )
            {
                return false;
            }
        }
    }
    else
    {
        // Use byte accesses.
        uint8_t *pAddr1 = (uint8_t *)(MSPI_XIPMM_BASE_ADDRESS);
        uint8_t *pAddr2 = (uint8_t *)(MSPI_XIPMM_BASE_ADDRESS  + 512);

        // Initialize a pattern
        for (ix = 0; ix < 512; ix++)
        {
            *pAddr1++ = (uint8_t)(ix & 0xFF);
            *pAddr2++ = (uint8_t)((ix & 0xFF) ^ 0xFF);
        }
        pAddr1 = (uint8_t *)(MSPI_XIPMM_BASE_ADDRESS);
        pAddr2 = (uint8_t *)(MSPI_XIPMM_BASE_ADDRESS + 512);

        // Verify the pattern
        for (ix = 0; ix < 512; ix++)
        {
            if ( (*pAddr1++ != (uint8_t)(ix & 0xFF)) || (*pAddr2++ != (uint8_t)((ix & 0xFF) ^ 0xFF)) )
            {
                return false;
            }
        }
    }
    return true;
}
#endif

void
load_objects(void)
{
    //!< Load memory objects
    g_sFB.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, g_sFB.stride * g_sFB.h);
    nema_buffer_map(&g_sFB.bo);
    printf("FB: V:%p P:0x%08x\n", (void *)g_sFB.bo.base_virt, g_sFB.bo.base_phys);

    g_sDCLayer.baseaddr_phys = g_sFB.bo.base_phys;
    g_sDCLayer.baseaddr_virt = g_sFB.bo.base_virt;

    printf("FB: V:%p P:0x%08x\n", (void *)g_sFB.bo.base_virt, g_sFB.bo.base_phys);
    g_sTSiLogo.bo.base_phys = MSPI_XIPMM_BASE_ADDRESS; //!< Point to PSRAM Address for image texture
    g_sTSiLogo.bo.base_virt = (void*)(g_sTSiLogo.bo.base_phys);
    nema_memcpy(g_sTSiLogo.bo.base_virt, g_ui8Oli400x400Tsc4, sizeof(g_ui8Oli400x400Tsc4));
}

nema_cmdlist_t g_sCL;

int32_t
tsc4_image_show()
{
    load_objects();

    g_sCL = nema_cl_create();

    nema_cl_bind(&g_sCL);

    nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, (nema_tex_format_t)(g_sFB.format), g_sFB.stride);
    nema_bind_src_tex(g_sTSiLogo.bo.base_phys, g_sTSiLogo.w, g_sTSiLogo.h, (nema_tex_format_t)(g_sTSiLogo.format), -1, NEMA_FILTER_BL);

    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_blit(NEMA_BL_SRC);

    nema_blit(0, 0);
    nema_set_blend_fill(NEMA_BL_SRC);

    nema_cl_unbind();
    nema_cl_submit(&g_sCL);
    nemadc_set_layer(0, &g_sDCLayer);

    nemadc_transfer_frame_prepare(false);
    //
    //It's necessary to launch frame manually when TE is disabled.
    //
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();

    nema_cl_wait(&g_sCL);

    nema_cl_destroy(&g_sCL);

    return 0;
}

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{
    //
    // panel's maxmum resolution.
    //
    uint32_t ui32MipiCfg = MIPICFG_16RGB888_OPT0;    //!< default config
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_xspi_raydium_config_t sDisplayPanelConfig;
    uint32_t ui32Status;
    //
    // External power on
    //
    am_bsp_external_pwr_on();
    am_util_delay_ms(100);
    am_bsp_low_power_init();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("GPU PSRAM XIPMM Example\n\n");

    am_bsp_disp_pins_enable();
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

    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        //
        // VDD18 control callback function
        //
        am_hal_dsi_register_external_vdd18_callback(am_bsp_external_vdd18_switch);
        //
        // Enable DSI power and configure DSI clock.
        //
        am_hal_dsi_init();
    }
    else
    {
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC96, NULL);
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);
    }
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);

    nema_sys_init();
    //
    //Initialize NemaDC
    //
    if (nemadc_init() != 0)
    {
        return -2;
    }
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
            am_devices_dc_dsi_raydium_init(&sDisplayPanelConfig);
            break;
        default:
            ; //NOP
    }
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
    // Write image texture to PSRAM with PSRAM driver.
    //

    am_util_stdio_printf("Writing %d Bytes to PSRAM Address 0x%x\n", sizeof(g_ui8Oli400x400Tsc4), 0x0);
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_write(g_pDevHandle, (uint8_t *)g_ui8Oli400x400Tsc4, 0x0, sizeof(g_ui8Oli400x400Tsc4), true);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to write image to PSRAM Device!\n");
    }


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

#ifdef ENABLE_XIPMM //!< run_mspi_xipmm() is only for XIPMM test, can be removed.
    //
    // If scrambling on, force word accesses in XIPMM.
    //
    if ( run_mspi_xipmm(0, bDoScrambling) )
    {
        am_util_stdio_printf("XIPMM aperature is working!\n");
    }
    else
    {
        am_util_stdio_printf("XIPMM aperature is NOT working!\n");
    }
    //
    // Rewrite image texture to PSRAM with XIPMM
    //
    nema_memcpy((uint32_t *)MSPI_XIPMM_BASE_ADDRESS, g_ui8Oli400x400Tsc4, sizeof(g_ui8Oli400x400Tsc4));
#endif

#ifdef BAREMETAL
    tsc4_image_show();
#else // BAREMETAL
    //
    // Initialize plotting interface.
    //
//    am_util_debug_printf("FreeRTOS NemaGFX_SW Transition Effect Example\n");

    //
    // Run the application.
    //
    run_tasks();
#endif // BAREMETAL

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }

}

