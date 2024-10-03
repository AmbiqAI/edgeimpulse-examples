//*****************************************************************************
//
//! @file display_task.c
//!
//! @brief Task to handle DISPLAY operations.
//!
//! AM_DEBUG_PRINTF
//! If enabled, debug messages will be sent over ITM.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifdef LV_AMBIQ_FB_REFRESH_CPU_ONLY

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "lvgl.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#include "nema_graphics.h"
#include "nema_dc.h"
#include "nema_dc_mipi.h"

#include "display_task.h"

//*****************************************************************************
//
// Display setting
//
//*****************************************************************************
//Frame buffer size
#ifndef LV_AMBIQ_FB_RESX
    #define LV_AMBIQ_FB_RESX (390U)
#endif

#ifndef LV_AMBIQ_FB_RESY
    #define LV_AMBIQ_FB_RESY (390U)
#endif

//Use direct mode, see https://docs.lvgl.io/master/porting/display.html#direct-mode.
#ifndef USE_DIRECT_MODE
    #define USE_DIRECT_MODE 1
#endif

//Use full refresh, see https://docs.lvgl.io/master/porting/display.html#full-refresh.
#ifndef USE_FULL_REFRESH
    #define USE_FULL_REFRESH 0
#endif

//Display refresh timeout
#ifndef DISPLAY_REFRESH_TIMEOUT
    #define DISPLAY_REFRESH_TIMEOUT (1000U) //1000 ticks, 1000*1ms= 1s
#endif
//*****************************************************************************
//
// Setting check.
//
//*****************************************************************************
#define FRAME_BUFFER_FORMAT       NEMA_BGRA8888

#define DC_PIXEL_FORMAT FMT_RGB888
#define DC_LAYER_FORMAT NEMADC_BGRA8888

//Offset of the panel
#define PANEL_OFFSET_X (((g_sDispCfg.ui16ResX/4*4)-LV_AMBIQ_FB_RESX)/2)
#define PANEL_OFFSET_Y (((g_sDispCfg.ui16ResY/4*4)-LV_AMBIQ_FB_RESY)/2)


#if !USE_DIRECT_MODE && !USE_FULL_REFRESH
#warning "Display panel must support partial refresh feature, and the panel should \
          have no restriction on the start and end column setting."
#endif

//*****************************************************************************
//
// DISPLAY task handle.
//
//*****************************************************************************
TaskHandle_t DisplayTaskHandle;

//*****************************************************************************
//
// Display refresh command.
//
//*****************************************************************************
typedef struct display_refresh_cmd_t_
{
   uint32_t start_x;
   uint32_t start_y;
   uint32_t width;
   uint32_t hight;
   uint8_t  format;
   void* pFrameBuffer;
   lv_disp_drv_t *disp_drv;
}display_refresh_cmd;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
display_refresh_cmd cmdDisplayRefresh =
{
    .start_x =0,
    .start_y = 0,
    .width = 0,
    .hight = 0,
    .format = 0,
    .pFrameBuffer =NULL,
    .disp_drv = NULL,
};

//*****************************************************************************
//
// Private data
//
//*****************************************************************************
static img_obj_t g_sFrameBuffer =
{
    {0},  LV_AMBIQ_FB_RESX, LV_AMBIQ_FB_RESY, -1, 1, FRAME_BUFFER_FORMAT, NEMA_FILTER_BL,
};

static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;

static uint32_t ui32MipiCfg = MIPICFG_8RGB888_OPT0; // default config

//layer
static nemadc_layer_t g_sLayerOne =
{
    .startx        = 0,
    .sizex         = LV_AMBIQ_FB_RESX,
    .resx          = LV_AMBIQ_FB_RESX,
    .starty        = 0,
    .sizey         = LV_AMBIQ_FB_RESY,
    .resy          = LV_AMBIQ_FB_RESY,
    .stride        = -1,
    .format        = DC_LAYER_FORMAT,
    .blendmode     = NEMADC_BL_SRC,
    .buscfg        = 0,
    .alpha         = 0xff,
    .flipx_en      = 0,
    .flipy_en      = 0,
};

//*****************************************************************************
//
// display flush callback.
//
//*****************************************************************************
void 
display_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, 
                 lv_color_t * color_p)
{
    cmdDisplayRefresh.format = FRAME_BUFFER_FORMAT;
    cmdDisplayRefresh.pFrameBuffer = color_p;
    cmdDisplayRefresh.start_x = area->x1;
    cmdDisplayRefresh.start_y = area->y1;
    cmdDisplayRefresh.width = area->x2 - area->x1  + 1;
    cmdDisplayRefresh.hight = area->y2 - area->y1  + 1;
    cmdDisplayRefresh.disp_drv = disp_drv;

    //Trigger the display refresh task.
#ifdef USE_DEBUG_PIN
    am_hal_gpio_output_set(DEBUG_PIN_2);
    am_hal_gpio_output_clear(DEBUG_PIN_2);
#endif
    xTaskNotifyGive( DisplayTaskHandle);

}


//*****************************************************************************
//
// Allocate buffer in SSRAM using tsi_malloc
//
//*****************************************************************************
int
SSRAM_buffer_alloc(img_obj_t* img)
{
    uint32_t size;

    size = nema_texture_size(img->format, 0, img->w, img->h);

    img->bo.base_virt = lv_mem_ssram_alloc(size);
    img->bo.base_phys = (uintptr_t)img->bo.base_virt;
    img->bo.size = size;

    if ( img->bo.base_virt == NULL )
    {
      am_util_stdio_printf("TSI buffer Malloc failed!\n");
      return -1;
    }

    memset(img->bo.base_virt, 0, size);

    return 0;
}


//*****************************************************************************
//
// LVGL display driver setup.
//
//*****************************************************************************
int
lv_disp_drv_setup(void)
{
    int ret;

    //Alloc SSRAM memory for frame buffer
    ret = SSRAM_buffer_alloc(&g_sFrameBuffer);
    if ( ret < 0 )
    {
       return ret;
    }

    // Init display buffer
    lv_disp_draw_buf_init(&disp_buf,
                          g_sFrameBuffer.bo.base_virt,
                          NULL,
                          g_sFrameBuffer.w * g_sFrameBuffer.h);

    // Init display driver
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = display_flush_cb;
    disp_drv.hor_res = LV_AMBIQ_FB_RESX;
    disp_drv.ver_res = LV_AMBIQ_FB_RESY;
    disp_drv.full_refresh = USE_FULL_REFRESH;
    disp_drv.direct_mode = USE_DIRECT_MODE;

    // Register the driver and save the created display objects.
    lv_disp_t * disp;
    disp = lv_disp_drv_register(&disp_drv);
    LV_UNUSED(disp);

    return ret;
}

int
display_setup(void)
{
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_xspi_raydium_config_t sDisplayPanelConfig;

    //
    // Set the display region to center
    //
    if (LV_AMBIQ_FB_RESX < g_sDispCfg.ui16ResX)
    {
        sDisplayPanelConfig.ui16ResX = LV_AMBIQ_FB_RESX;
    }
    else
    {
        sDisplayPanelConfig.ui16ResX = g_sDispCfg.ui16ResX;
    }
    ui16MinX = (g_sDispCfg.ui16ResX - sDisplayPanelConfig.ui16ResX) >> 1;
    ui16MinX = (ui16MinX >> 1) << 1;

    if (LV_AMBIQ_FB_RESY < g_sDispCfg.ui16ResY)
    {
        sDisplayPanelConfig.ui16ResY = LV_AMBIQ_FB_RESY;
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
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);

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
            am_devices_dc_dsi_raydium_init(((am_devices_dc_dsi_raydium_config_t *) &sDisplayPanelConfig));
            break;
        default:
            ; //NOP
    }
    return 0;
}

uint32_t
display_set_region(uint16_t ui16ResX,
                   uint16_t ui16ResY,
                   uint16_t ui16MinX,
                   uint16_t ui16MinY)
{
    uint32_t ui32Status = 0;

    //
    // panel offset
    //
    ui16MinX += g_sDispCfg.ui16Offset;

    switch (g_sDispCfg.eInterface) // TODO
    {
        case DISP_IF_SPI4:
        case DISP_IF_DSPI:
        case DISP_IF_QSPI:
            am_devices_dc_xspi_raydium_set_region(ui16ResX,
                                                  ui16ResY,
                                                  ui16MinX,
                                                  ui16MinY);
#ifdef AM_PART_APOLLO5_API
            nemadc_timing(ui16ResX,  1, 1, 1,
                          ui16ResY, 1, 1, 1);
#else     
            nemadc_timing(ui16ResX, 4, 10, 10,
                          ui16ResY, 10, 50, 10);
#endif //(defined AM_PART_APOLLO5_API)
            break;
        case DISP_IF_DSI:
            am_devices_dc_dsi_raydium_set_region(ui16ResX,
                                                 ui16ResY,
                                                 ui16MinX,
                                                 ui16MinY);
#ifdef AM_PART_APOLLO5_API
            nemadc_timing(ui16ResX,  1, 10, 1,
                          ui16ResY, 1, 1, 1);
#else  
            nemadc_timing(ui16ResX, 4, 10, 1,
                          ui16ResY, 1, 1, 1);
#endif //(defined AM_PART_APOLLO5_API)
            break;
        default:
            return AM_DEVICES_DISPLAY_STATUS_INVALID_ARG;
    }

    return ui32Status;
}

//*****************************************************************************
//
// Task function.
//
//*****************************************************************************
void
DisplayTask(void *pvParameters)
{
    int ret;
    display_refresh_cmd cmdDisplayRefreshRecord = cmdDisplayRefresh;

    am_util_stdio_printf("Display task start!\n");

#ifdef USE_DEBUG_PIN
    am_hal_gpio_pinconfig(DEBUG_PIN_1, am_hal_gpio_pincfg_output); //Toggle when the GPU finished his work
    am_hal_gpio_pinconfig(DEBUG_PIN_2, am_hal_gpio_pincfg_output); //Keep high when the display task is active
    am_hal_gpio_pinconfig(DEBUG_PIN_3, am_hal_gpio_pincfg_output); //Keep high when the display data transfer is active
    am_hal_gpio_pinconfig(DEBUG_PIN_4, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_5, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_6, am_hal_gpio_pincfg_output);
#endif

    //Init display system, including DC, DSI(or SPI), panel
    ret = display_setup();
    if (ret != 0)
    {
        am_util_stdio_printf("display init failed!\n");

        //suspend and delete this task.
        vTaskDelete(NULL);
    }

    //
    // Set up LVGL display driver.
    //
    ret = lv_disp_drv_setup();
    if (ret != 0)
    {
        am_util_stdio_printf("LVGL display driver setup failed!\n");

        //suspend and delete this task.
        vTaskDelete(NULL);
    }

    //Set the data location to be send by DC.
    g_sLayerOne.baseaddr_phys = g_sFrameBuffer.bo.base_phys;
    g_sLayerOne.baseaddr_virt = g_sFrameBuffer.bo.base_virt;
    nemadc_set_layer(0, &g_sLayerOne);

    while(1)
    {
        //Wait start.
        uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

#ifdef USE_DEBUG_PIN
        am_hal_gpio_output_set(DEBUG_PIN_3);
#endif

        if( ulNotificationValue != 1 )
        {
            LV_LOG_ERROR("Display task wait notify timeout!\n");
        }

#if (!USE_DIRECT_MODE) && (!USE_FULL_REFRESH)
        //Check whether the display region is updated
        if((cmdDisplayRefresh.width != cmdDisplayRefreshRecord.width) || 
           (cmdDisplayRefresh.hight != cmdDisplayRefreshRecord.hight) ||
           (cmdDisplayRefresh.start_x != cmdDisplayRefreshRecord.start_x) || 
           (cmdDisplayRefresh.start_y != cmdDisplayRefreshRecord.start_y))
        {
            //Set display region
            display_set_region(cmdDisplayRefresh.width,
                               cmdDisplayRefresh.hight,
                               cmdDisplayRefresh.start_x,
                               cmdDisplayRefresh.start_y);

            //Updated display region record.
            cmdDisplayRefreshRecord = cmdDisplayRefresh;
        }
#else
        LV_UNUSED(cmdDisplayRefreshRecord);
#endif

        // Flush the frame buffer region.
        // This is necessary, because the frame buffer is written by CPU and read by DC.
        am_hal_cachectrl_range_t sRange;
        sRange.ui32StartAddr = (uint32_t )cmdDisplayRefresh.pFrameBuffer;
        sRange.ui32Size = nema_texture_size(cmdDisplayRefresh.format, 0, 
                                            cmdDisplayRefresh.width, 
                                            cmdDisplayRefresh.hight);
        am_hal_cachectrl_dcache_clean(&sRange);

        //Start DC
        nemadc_transfer_frame_prepare(false);
        //
        //It's necessary to launch frame manually when TE is disabled.
        //
        nemadc_transfer_frame_launch();

#ifdef USE_DEBUG_PIN
        am_hal_gpio_output_set(DEBUG_PIN_4);
#endif

        //Wait DC complete interrupt.
        nemadc_wait_vsync();

#ifdef USE_DEBUG_PIN
        am_hal_gpio_output_clear(DEBUG_PIN_4);
#endif

        // Inform LVGL library that the framebuffer is avaliable*/
        lv_disp_flush_ready(cmdDisplayRefresh.disp_drv);

#ifdef USE_DEBUG_PIN
        am_hal_gpio_output_clear(DEBUG_PIN_3);
#endif

    }

}
#endif