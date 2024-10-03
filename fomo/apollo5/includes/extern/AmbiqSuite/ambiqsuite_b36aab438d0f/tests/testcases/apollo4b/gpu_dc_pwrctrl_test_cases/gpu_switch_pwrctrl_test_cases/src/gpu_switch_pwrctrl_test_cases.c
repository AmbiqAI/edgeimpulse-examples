//*****************************************************************************
//
//! @file gpu_pwrctrl_test_cases.c
//!
//! @brief GPU power control test cases.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "gpu_dc_pwrctrl_test_common.h"
#include "stdlib.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************


//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

#define RESX (g_sDispCfg.ui16ResX / 64 * 64)
#define RESY (g_sDispCfg.ui16ResY / 64 * 64)

#define FRAME_BUFFERS  2
static img_obj_t g_sFBs[FRAME_BUFFERS];
static nemadc_layer_t sLayer[FRAME_BUFFERS] = {0};

static void
fb_reload_rgb565(void)
{
    for (int i = 0; i < FRAME_BUFFERS; ++i)
    {
        g_sFBs[i].w = RESX;
        g_sFBs[i].h = RESY;
        g_sFBs[i].format = NEMA_RGB565;
        g_sFBs[i].stride = RESX * 2;
        g_sFBs[i].bo = nema_buffer_create(g_sFBs[i].stride * g_sFBs[i].h);
        (void)nema_buffer_map(&g_sFBs[i].bo);

        sLayer[i].startx = sLayer[i].starty = 0;
        sLayer[i].sizex = sLayer[i].resx = g_sFBs[i].w;
        sLayer[i].sizey = sLayer[i].resy = g_sFBs[i].h;
        sLayer[i].stride = g_sFBs[i].stride;
        sLayer[i].format = NEMADC_RGB565 | (uint32_t)NEMADC_MODULATE_A;

        sLayer[i].blendmode = NEMADC_BL_SIMPLE;
        sLayer[i].alpha     = 0x80;
        sLayer[i].flipx_en      = 0;
        sLayer[i].flipy_en      = 0;
        sLayer[i].baseaddr_phys = g_sFBs[i].bo.base_phys;
        sLayer[i].baseaddr_virt = g_sFBs[i].bo.base_virt;
    }
}

static int i32CurFB = 0;
static int i32LastFB = -1;

static void
swap_fb(void)
{
    i32LastFB = i32CurFB;
    i32CurFB = (i32CurFB + 1) % FRAME_BUFFERS;
}

#define MAX_CIRCLE_NUM               (1)

typedef struct
{
    int i32LastX, i32LastY;
    int i32CenterX, i32CenterY;
    int i32Radius;
    int speed_x, speed_y;
    unsigned char red, green, blue, aplha;
    unsigned char isFilled;
    char direction;
}
Circle_t;

Circle_t g_sCircle[MAX_CIRCLE_NUM];

void
update_circle(void)
{
    uint16_t i;
    int16_t i16CurrentY;
    int16_t i16CurrentX;

    for (i = 0; i < MAX_CIRCLE_NUM; i++)
    {
        i16CurrentY = g_sCircle[i].i32CenterY + g_sCircle[i].speed_y;
        if (i16CurrentY > RESY - g_sCircle[i].i32Radius || i16CurrentY < g_sCircle[i].i32Radius)
        {
            if (g_sCircle[i].speed_y > 0)
            {
                g_sCircle[i].i32CenterY = RESY - g_sCircle[i].i32Radius;
                g_sCircle[i].speed_y = -g_sCircle[i].speed_y;
            }
            else
            {
                g_sCircle[i].i32CenterY = g_sCircle[i].i32Radius;
                g_sCircle[i].speed_y = abs(g_sCircle[i].speed_y);
            }
        }
        else
        {
            g_sCircle[i].i32CenterY = i16CurrentY;
        }
    }

    for (i = 0; i < MAX_CIRCLE_NUM; i++)
    {
        i16CurrentX = g_sCircle[i].i32CenterX + g_sCircle[i].speed_x;
        if (i16CurrentX > RESX - g_sCircle[i].i32Radius || i16CurrentX < g_sCircle[i].i32Radius)
        {
            if (g_sCircle[i].speed_x > 0)
            {
                g_sCircle[i].i32CenterX = RESX - g_sCircle[i].i32Radius;
                g_sCircle[i].speed_x = -g_sCircle[i].speed_x;
            }
            else
            {
                g_sCircle[i].i32CenterX = g_sCircle[i].i32Radius;
                g_sCircle[i].speed_x = abs(g_sCircle[i].speed_x);
            }
        }
        else
        {
            g_sCircle[i].i32CenterX = i16CurrentX;
        }
    }

    for (i = 0; i < MAX_CIRCLE_NUM; i++)
    {
        nema_fill_circle(g_sCircle[i].i32CenterX, g_sCircle[i].i32CenterY, g_sCircle[i].i32Radius,
                         nema_rgba(g_sCircle[i].red, g_sCircle[i].green, g_sCircle[i].blue, g_sCircle[i].aplha));
    }
}

bool
test_blit_balls(void)
{
    nema_cmdlist_t sCLCircles;
    uint16_t i;
    uint16_t cnt=0;
    uint8_t ui8FlagDC = 0;
    srand(1);
    for (i = 0; i < MAX_CIRCLE_NUM; i++)
    {
        g_sCircle[i].i32Radius = rand() % 100+80;
        g_sCircle[i].i32CenterX = rand() % (RESX - g_sCircle[i].i32Radius * 2) + g_sCircle[i].i32Radius;
        g_sCircle[i].i32CenterY = rand() % (RESY - g_sCircle[i].i32Radius * 2) + g_sCircle[i].i32Radius;

        g_sCircle[i].red = rand() % 256;
        g_sCircle[i].green = rand() % 256;
        g_sCircle[i].blue = rand() % 256;
        g_sCircle[i].aplha = rand() % 256;

        g_sCircle[i].speed_x = rand() % 2 + 1;
        g_sCircle[i].speed_y = rand() % 2 + 1;
    }
    sCLCircles = nema_cl_create();
    am_util_stdio_printf("Apollo4b GPU Switch Power\n\n");

    while (1)
    {
        nema_cl_bind(&sCLCircles);
        nema_bind_dst_tex(g_sFBs[i32CurFB].bo.base_phys, g_sFBs[i32CurFB].w, g_sFBs[i32CurFB].h, g_sFBs[i32CurFB].format, g_sFBs[i32CurFB].stride);
        nema_set_clip(0, 0, RESX, RESY);
        nema_clear(nema_rgba(0x00, 0x00, 0x00, 0xff));
        nema_set_blend_fill(NEMA_BL_SIMPLE);
        update_circle();

        if (i32LastFB >= 0)
        {
            if (ui8FlagDC == 1)
            {
                nemadc_wait_vsync();
            }
        }

        nema_cl_submit(&sCLCircles);
        if (i32LastFB >= 0)
        {

            nemadc_set_layer(0, &sLayer[i32LastFB]);

            nemadc_transfer_frame_prepare(false);
            //
            //It's necessary to launch frame manually when TE is disabled.
            //
            nemadc_transfer_frame_launch();

            ui8FlagDC = 1;
        }

        nema_cl_wait(&sCLCircles);
        nema_cl_rewind(&sCLCircles);
        swap_fb();
        if(cnt++ > 1000)
        {
            am_util_stdio_printf("GPU Switch test success!\n\n");
            return true;
        }
        am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);
        //am_util_delay_ms(1);
        am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
        int i32Ret;
        i32Ret = nema_init();
        if (i32Ret != 0)
        {
            am_util_stdio_printf("GPU Switch test error!\n\n");
            return false;
            //return i32Ret;
        }

#ifndef BAREMETAL
        taskYIELD();
//        vTaskDelay(1);
#endif
    }
    //return false;
}

//*****************************************************************************
//
//! @brief GPU blit test case with power control
//!
//!
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
gpu_switch_pwrctrl_test(void)
{
    bool bTestPass = true;
    uint32_t ui32MipiCfg = MIPICFG_16RGB888_OPT0;    //!< default config
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_xspi_raydium_config_t sDisplayPanelConfig;

    g_sDispCfg.eInterface = DISP_IF_DSI;
    // g_sDsiCfg.ui8NumLanes = 1;
    // g_sDsiCfg.eDbiWidth = AM_HAL_DSI_DBI_WIDTH_8;
    // g_sDsiCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X12;
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
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

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
    //
    // If you want to change display configurations in example, please change them here as below.
    //
    // g_sDispCfg.ui8NumLanes = 1;
    // g_sDispCfg.eDbiWidth = AM_HAL_DSI_DBI_WIDTH_8;
    // g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X12;

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
        return false;
    }
    if (g_sDispCfg.eInterface == DISP_IF_DSI)
    {
        uint8_t ui8LanesNum = g_sDispCfg.ui8NumLanes;
        uint8_t ui8DbiWidth = g_sDispCfg.eDbiWidth;
        uint32_t ui32FreqTrim = g_sDispCfg.eDsiFreq;
        pixel_format_t eFormat = FMT_RGB888;
        if (am_hal_dsi_para_config(ui8LanesNum, ui8DbiWidth, ui32FreqTrim, false) != 0)
        {
            return false;
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
                return false;
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

//    nema_event_init(1, 0, 0, RESX, RESY);

    fb_reload_rgb565();
    bTestPass = test_blit_balls();
    TEST_ASSERT_TRUE(bTestPass);
    return bTestPass;
}
