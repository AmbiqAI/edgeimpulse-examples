//*****************************************************************************
//
//! @file nemagfx_burn_in_mitigation.c
//!
//! @brief NemaGFX example.
//! This example demonstrates how to implement burn-in mitigation on software side.
//! It through GPU to blend a mask(4 byte) with NEMA_TEX_REPEAT feature enabled
//! when defined maro GPU_BLEND_MASK, otherwise it will blends a full mask through
//! DC module.The later method used 2 layers at least.
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

#include "nemagfx_burn_in_mitigation.h"
#include "tsi_malloc.h"
#include "nema_error.h"

//#define GPU_BLEND_MASK  //otherwise DC blend mask

#define RESX                400
#define RESY                RESX
//
// Defined the checkboard size
//
#define FULL_CHECKBOARD_X    RESX/2
#define FULL_CHECKBOARD_Y    RESY

//
// Checkboard type: upper left, upper right, down right and down left.
//
#define CHECKBOARD_TYPE      4
//*****************************************************************************
//
// Variables Definition.
//
//*****************************************************************************
static img_obj_t            g_sFB = {{0}, RESX, RESY, -1, 0, NEMA_RGB24, 0};
static nemadc_layer_t       g_sDCLayer = {(void *)0, 0, RESX, RESY, -1, 0, 0, RESX, RESY, 0xff, NEMADC_BL_SRC, 0, NEMADC_RGB24, 0, 0, 0, 0, 0, 0, 0, 0};

#ifdef GPU_BLEND_MASK
//
// GPU could extend a small buffer to full framebuffer through feature NEMA_TEX_REPEAT.
//
#define CHECKBOARD_SIZE_X     2
#define CHECKBOARD_SIZE_Y     CHECKBOARD_SIZE_X
static img_obj_t            g_sCheckboard = {{0}, CHECKBOARD_SIZE_X, CHECKBOARD_SIZE_Y, -1, 0, NEMA_A8, 0};

#else
typedef struct
{
    uint16_t ui16Pixels[FULL_CHECKBOARD_Y][FULL_CHECKBOARD_X/2];
}
Checkboard_t;

static nemadc_layer_t       g_sCheckboardlayer = {(void *)0, 0, FULL_CHECKBOARD_X, FULL_CHECKBOARD_Y, -1, 0, 0, FULL_CHECKBOARD_X, FULL_CHECKBOARD_Y, 0xff, NEMADC_BL_SIMPLE, 0, NEMADC_RGBA2222, 0, 0, 0, 0, 0, 0, 0, 0};

#endif //GPU_BLEND_MASK

//
// DSI sends ULPS pattern on or off when DSI enters/exits ULPS mode
//
#define ULPS_PATTERN_ON     true
//
// blend modes
//
const uint32_t g_ui32BlendMode[] =
{
    NEMA_BL_SIMPLE,
    NEMA_BL_CLEAR,
    NEMA_BL_SRC,
    NEMA_BL_SRC_OVER,
    NEMA_BL_DST_OVER,
    NEMA_BL_SRC_IN,
    NEMA_BL_DST_IN,
    NEMA_BL_SRC_OUT,
    NEMA_BL_DST_OUT,
    NEMA_BL_SRC_ATOP,
    NEMA_BL_DST_ATOP,
    NEMA_BL_ADD,
    NEMA_BL_XOR
};

//*****************************************************************************
//
//! @brief Initialize framebuffer and layers
//!
//! This function initializes FB size,format and the DC mask's.
//!
//! @return None.
//
//*****************************************************************************
void
load(void)
{
    g_sFB.stride = nema_format_size(g_sFB.format) * g_sFB.w;
    g_sFB.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, g_sFB.stride * g_sFB.h);
    // memset((void*)(g_sFB.bo.base_phys), 0, g_sFB.stride * g_sFB.h);
    g_sDCLayer.stride = nemadc_stride_size(g_sDCLayer.format, g_sDCLayer.resx);
    g_sDCLayer.baseaddr_virt = (void*)g_sFB.bo.base_virt;
    g_sDCLayer.baseaddr_phys = (uintptr_t)g_sFB.bo.base_phys;
#ifdef GPU_BLEND_MASK
    g_sCheckboard.stride = nema_format_size(g_sCheckboard.format) * g_sCheckboard.w;
#else
    g_sCheckboardlayer.stride = nemadc_stride_size(g_sCheckboardlayer.format, g_sCheckboardlayer.resx);
    g_sCheckboardlayer.baseaddr_virt = tsi_malloc(g_sCheckboardlayer.resy * g_sCheckboardlayer.stride);
    g_sCheckboardlayer.baseaddr_phys = (unsigned)(g_sCheckboardlayer.baseaddr_virt);

    Checkboard_t *pCheckboard;

    pCheckboard = (Checkboard_t*)g_sCheckboardlayer.baseaddr_virt;

    for (uint32_t ui32line = 0; ui32line < FULL_CHECKBOARD_Y; ui32line++)
    {
        if(ui32line % 2 != 0)
        {
            memset(pCheckboard->ui16Pixels[ui32line], 0x00, FULL_CHECKBOARD_X);
        }
        else
        {
            for (uint32_t ui32Idx = 0; ui32Idx < FULL_CHECKBOARD_X/2; ui32Idx++)
            {
                pCheckboard->ui16Pixels[ui32line][ui32Idx] = 0x0300; 
            }
        }
    }
    //
    // Please flush the D-cache if CPU has directly written frame buffers.
    //
    nema_buffer_t bo;
    bo.size = g_sCheckboardlayer.resy * g_sCheckboardlayer.stride;
    bo.base_phys = g_sCheckboardlayer.baseaddr_phys;
    nema_buffer_flush(&bo);
#endif // GPU_BLEND_MASK

}

static uint32_t dsi_reg_function   = 0;
static uint32_t dsi_reg_afetrim1   = 0;

//*****************************************************************************
//
//! @brief Disable DSI to reduce power consumption.
//!
//! Retained two important present registers value for DSI power on.
//!
//! @return true
//
//*****************************************************************************
static bool
dphy_power_down(void)
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_DISPPHY, &status);
    if ( status )
    {
        //
        // Get DSI functional programming register
        //
        dsi_reg_function = DSI->DSIFUNCPRG;

        //
        // Get DSI trim register that included DSI frequency
        //
        dsi_reg_afetrim1 = DSI->AFETRIM1 & 0x0000007F;

        am_hal_dsi_napping(ULPS_PATTERN_ON);
    }
    return true;
}

//*****************************************************************************
//
//! @brief Enable DSI
//!
//! Resume the previous DSI configuration.
//!
//! @return bool.
//
//*****************************************************************************
static bool
dphy_power_up(void)
{
    if (dsi_reg_afetrim1 == 0 && dsi_reg_function == 0)
    {
        //
        // DSI configuration parameters are invalid.
        //
        return false;
    }

    uint32_t ui32FreqTrim = _FLD2VAL(DSI_AFETRIM1_AFETRIM1, dsi_reg_afetrim1);
    uint8_t ui8LanesNum = (uint8_t)_FLD2VAL(DSI_DSIFUNCPRG_DATALANES, dsi_reg_function);
    uint8_t ui8DBIBusWidth = (uint8_t)_FLD2VAL(DSI_DSIFUNCPRG_REGNAME, dsi_reg_function);
    if (4 == ui8DBIBusWidth)
    {
        ui8DBIBusWidth = 16;
    }
    else if (3 == ui8DBIBusWidth)
    {
        ui8DBIBusWidth = 8;
    }
    else if (2 == ui8DBIBusWidth)
    {
        ui8DBIBusWidth = 9;
    }
    else
    {
        return false;
    }

    am_hal_dsi_wakeup(ui8LanesNum, ui8DBIBusWidth, ui32FreqTrim, ULPS_PATTERN_ON);
    return true;
}

//*****************************************************************************
//
//! @brief Disable the power & clock for display controller to save power
//!
//! @return true.
//
//*****************************************************************************
static bool
dc_power_down(void)
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_DISP, &status);
    if ( status )
    {
        nemadc_backup_registers();

        am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_DISP);
    }
    return true;
}

//*****************************************************************************
//
//! @brief Enable display controller
//!
//! @note Enable clock & power for display controller, then restore the configuration
//!
//! @return true if success, otherwise return false.
//
//*****************************************************************************
static bool
dc_power_up(void)
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_DISP, &status);
    if ( !status )
    {
        am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);
        if (0 != nemadc_init())
        {
            //
            // Initialize DC failed.
            //
            return false;
        }
        return nemadc_restore_registers();
    }
    return true;
}
//*****************************************************************************
//
//! @brief Power off GPU
//!
//! @note after called this function,please don't operation any GPU registers
//! until enable and initial the GPU.
//!
//! @return 0: success. 
//!         -1: GPU is busy, try it later.
//
//*****************************************************************************
int32_t
gpu_power_down(void)
{
    bool status;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_GFX, &status);
    if ( status )
    {
        if(nema_reg_read(NEMA_STATUS) == 0)
        {
            // If GPU is not busy, we can power off the GPU safely.
            am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);
        }
        else
        {
            return -1;
        }
    }

    return 0;
}
//*****************************************************************************
//
//! @brief Power on GPU power and restore nemaGFX context.
//!
//! @return 0: GPU power have initialize completely.
//!         -1: GPU power initialize error.
//
//*****************************************************************************
int32_t
gpu_power_up(void)
{
    int32_t i32Ret = 0;
    bool enabled;
    am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_GFX, &enabled);
    if ( !enabled )
    {
        am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
        //
        // Initialize NemaGFX
        //
        nema_reinit();
        if(nema_get_error() != NEMA_ERR_NO_ERROR)
        {
            am_util_debug_printf("Nemagfx reinit error!\n");
            return -1;
        }

        nema_reset_last_cl_id();
    }
    return i32Ret;
}

//*****************************************************************************
//
//! @brief transfer frame data to panel
//!
//! This function block processor until the action is completed.
//!
//! @return None.
//
//*****************************************************************************
void
transfer_frame(void)
{
    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        dphy_power_up();
    }
    else
    {
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC96, NULL);
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);
    }
    dc_power_up();
    nemadc_set_layer(0, &g_sDCLayer);
#ifdef GPU_BLEND_MASK
#else
    static uint16_t ui16Loop = 0;
    switch(ui16Loop++ % CHECKBOARD_TYPE)
    {
        case 0:
            g_sCheckboardlayer.flipx_en = 0;
            g_sCheckboardlayer.flipy_en = 0;
            break;
        case 1:
            g_sCheckboardlayer.flipx_en = 1;
            g_sCheckboardlayer.flipy_en = 0;
            break;
        case 2:
            g_sCheckboardlayer.flipx_en = 1;
            g_sCheckboardlayer.flipy_en = 1;
            break;
        case 3:
            g_sCheckboardlayer.flipx_en = 0;
            g_sCheckboardlayer.flipy_en = 1;
            break;
        default:
            break;
    }
    nemadc_set_layer(1, &g_sCheckboardlayer);
#endif
    nemadc_transfer_frame_prepare(g_sDispCfg.eTEType != DISP_TE_DISABLE);
    if ( g_sDispCfg.eTEType == DISP_TE_DISABLE )
    {
        //
        //It's necessary to launch frame manually when TE is disabled.
        //
        nemadc_transfer_frame_launch();
    }
    nemadc_wait_vsync();

    dc_power_down();
    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        dphy_power_down();
    }
    else
    {
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_OFF, NULL);
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_DISABLE, NULL);
    }
}

//*****************************************************************************
//
//! @brief demonstrate blend modes
//!
//! This function demonstrates various of blend modes
//!
//! @return None.
//
//*****************************************************************************
void
test_blend_mode(void)
{
#ifdef GPU_BLEND_MASK
    uint8_t ui8Checkboard[][CHECKBOARD_TYPE] = //__attribute__((aligned(16))) = 
    {
        {0xFF,0x00,0x00,0x00},
        {0x00,0xFF,0x00,0x00},
        {0x00,0x00,0xFF,0x00},
        {0x00,0x00,0x00,0xFF},
    };
#endif
    while(1)
    {
        for (uint32_t ui32I = 0; ui32I < 13; ui32I++)
        {
            gpu_power_up();
            nema_cmdlist_t sCL;
            //
            // Create Command Lists
            //
            sCL = nema_cl_create();
            //
            // Bind Command List
            //
            nema_cl_bind(&sCL);
            //
            // Bind Framebuffer
            //
            nema_bind_dst_tex(g_sFB.bo.base_phys, g_sFB.w, g_sFB.h, g_sFB.format, g_sFB.stride);
            //
            // Set Clipping Rectangle
            //
            nema_set_clip(0, 0, RESX, RESY);
            nema_clear(0x00000000);
            nema_set_blend_fill(NEMA_BL_SRC);
            nema_fill_rect(0, 0, RESX, RESY, nema_rgba(0xFF, 0, 0, 0xFF));
            nema_set_blend_fill(g_ui32BlendMode[ui32I]);
            nema_fill_rect(RESX / 4, RESY / 4, RESX / 2 , RESY / 2, nema_rgba(0, 0, 0xFF, 0x80));

            nema_set_blend_blit(NEMA_BL_SIMPLE);
    #ifdef GPU_BLEND_MASK  
            g_sCheckboard.bo.base_phys = ui8Checkboard[ui32I % CHECKBOARD_TYPE];
            g_sCheckboard.bo.base_virt = g_sCheckboard.bo.base_phys;

            nema_bind_src_tex(g_sCheckboard.bo.base_phys,
                            g_sCheckboard.w,
                            g_sCheckboard.h,
                            g_sCheckboard.format,
                            g_sCheckboard.stride,
                            NEMA_FILTER_PS | NEMA_TEX_REPEAT);
            //nema_blit(0, 0);
            //nema_blit_subrect(0, 0, RESX, RESY, 0, 0);
            nema_blit_rect(0, 0, FULL_CHECKBOARD_X, FULL_CHECKBOARD_Y);
    #endif
            nema_cl_submit(&sCL);
            nema_cl_wait(&sCL);
            nema_cl_rewind(&sCL);
            nema_cl_destroy(&sCL);
            gpu_power_down();
            am_util_delay_ms(5);
            transfer_frame();
            am_util_delay_ms(10);
        }
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
    uint32_t ui32MipiCfg = MIPICFG_16RGB888_OPT0;    //!< default config
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_xspi_raydium_config_t sDisplayPanelConfig;

    //
    // External power on
    //
    am_bsp_external_pwr_on();
    am_util_delay_ms(100);

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);
    //am_hal_cachectrl_dcache_disable();

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    //
    // Disable crypto
    //
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);
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
            sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
            nemadc_configure(&sDCConfig);
            am_devices_dc_dsi_raydium_init(&sDisplayPanelConfig);
            break;
        default:
            ; //NOP
    }
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Initialize NemaGFX
    //
    nema_init();

    load();

    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        dphy_power_down();
    }
    else
    {
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_OFF, NULL);
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_DISABLE, NULL);
    }
    dc_power_down();
    gpu_power_down();

    am_util_delay_ms(2000);

    test_blend_mode();

    while (1)
    {
    }
}
