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

#include "nema_hal.h"
#include "nema_math.h"
#include "nema_core.h"
#include "nema_regs.h"
#include "nema_utils.h"
#include "nema_event.h"
#include "nema_graphics.h"
#include "nema_programHW.h"

#include "display_task_copy.h"
#include "lv_gpu_ambiq_nema.h"
#include "lv_ambiq_nema_hal.h"
#include "sensor_hub_test_cases.h"

#define USE_DEBUG_PIN
#define DISPLAY_ON_OFF
//*****************************************************************************
//
// Display setting
//
//*****************************************************************************
//Frame buffer and display panel color format, 0 enable, 1 disable.
#ifndef LV_AMBIQ_FB_USE_RGB565
    #define LV_AMBIQ_FB_USE_RGB565 0
#endif

#ifndef LV_AMBIQ_FB_USE_RGB888
    #define LV_AMBIQ_FB_USE_RGB888 1
#endif

//Frame buffer size
#ifndef LV_AMBIQ_FB_RESX
    #define LV_AMBIQ_FB_RESX (456U)
#endif

#ifndef LV_AMBIQ_FB_RESY
    #define LV_AMBIQ_FB_RESY (456U)
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

#if LV_AMBIQ_FB_USE_RGB888
    #define DISPLAY_FORMAT            COLOR_FORMAT_RGB888
    #define FRAME_BUFFER_FORMAT       NEMA_RGB24
#elif LV_AMBIQ_FB_USE_RGB565
    #define DISPLAY_FORMAT            COLOR_FORMAT_RGB565
    #define FRAME_BUFFER_FORMAT       NEMA_RGB565
#else
    #error "Display format not supported!"
#endif

#if LV_AMBIQ_FB_USE_RGB888 && LV_AMBIQ_FB_USE_RGB565
#warning "Both RGB88 and RGB565 color formats are enabled, using RGB888."
#endif

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

volatile bool display_watchface = false;
volatile bool displayOn = true;

//*****************************************************************************
//
// Private data
//
//*****************************************************************************
static img_obj_t g_sFrameBuffer[2] =
{
    {{0},  LV_AMBIQ_FB_RESX, LV_AMBIQ_FB_RESY, -1, 1, FRAME_BUFFER_FORMAT, NEMA_FILTER_BL,},
    {{0},  LV_AMBIQ_FB_RESX, LV_AMBIQ_FB_RESY, -1, 1, FRAME_BUFFER_FORMAT, NEMA_FILTER_BL,},
};

static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;

//*****************************************************************************
//
//! @brief Enable display
//!
//! @note the panel exit sleep mode and display image.
//
//*****************************************************************************
int32_t
display_panel_enable(void)
{
#ifdef DISPLAY_ON_OFF
    displayOn = true;
    am_hal_gpio_state_write(GPIO_DISPLAY_ON_OFF, AM_HAL_GPIO_OUTPUT_SET);

    nemadc_mipi_cmd_write(MIPI_exit_sleep_mode, NULL, 0, true, false);
    am_util_delay_ms(130);

    nemadc_mipi_cmd_write(MIPI_set_display_on, NULL, 0, true, false);
    am_util_delay_ms(200);
#endif
    return 0;
}

//*****************************************************************************
//
//! @brief Disable display 
//!
//! @note the panel disbled and enter sleep mode.
//
//*****************************************************************************
int32_t
display_panel_disable(void)
{
#ifdef DISPLAY_ON_OFF
    displayOn = false;
    am_hal_gpio_state_write(GPIO_DISPLAY_ON_OFF, AM_HAL_GPIO_OUTPUT_CLEAR);

    nemadc_mipi_cmd_write(MIPI_set_display_off, NULL, 0, true, false);
    am_util_delay_ms(20);

    nemadc_mipi_cmd_write(MIPI_enter_sleep_mode, NULL, 0, true, false);
    //am_util_delay_ms(120);
#endif
    return 0;
}



//*****************************************************************************
//
//! @brief GPU memcpy, use the GPU do the memcpy, this much faster than CPU memcpy.
//!
//! @param resx - X resolution.
//! @param resy - Y resolution.
//! @param format - Color format.
//! @param des - Destination buffer.
//! @param src - Source buffer.
//!
//! @note The src and des buffers must have the same resolution and color format.
//!
//! @return 0, success, -1, failed.
//
//*****************************************************************************
int 
GPU_memcpy(uint32_t resx, uint32_t resy, uint8_t format, void* des, void* src)
{
    nema_cmdlist_t cl_memcpy;

    // Create the command list for GPU memcpy.
    cl_memcpy = nema_cl_create_sized(0x100);
    LV_ASSERT_NULL(cl_memcpy.bo.base_virt);
    if(cl_memcpy.bo.base_virt == NULL)
    {
        return -1;
    }

#ifdef NEMA_GFX_POWERSAVE
    //Power up GPU
    NEMA_BUILDCL_START
    lv_ambiq_nema_gpu_power_on();
#endif

    //Rewind and bind the CL
    nema_cl_bind(&cl_memcpy);
    nema_cl_rewind(&cl_memcpy);

    // Bind destination buffer
    nema_bind_dst_tex((uintptr_t)des,
                      resx,
                      resy,
                      format,
                      -1);

    //Set clip
    nema_set_clip(0, 0, resx, resy);

    //Set blend mode
    nema_set_blend_blit(NEMA_BL_SRC);

    //Bind source buffer
    nema_bind_src_tex((uintptr_t)src,
                  resx,
                  resy,
                  format,
                  -1,
                  NEMA_FILTER_PS);

    //Blit
    nema_blit(0, 0);

    //start GPU, submit CL
    nema_cl_submit(&cl_memcpy);

#if defined(NEMA_GFX_POWERSAVE) && defined(NEMA_GFX_POWEROFF_END_CL)
    //Power off GPU in CL end
    NEMA_BUILDCL_END
    lv_ambiq_nema_gpu_check_busy_and_suspend();
#endif

    nema_cl_wait(&cl_memcpy);

#if defined(NEMA_GFX_POWERSAVE) && !defined(NEMA_GFX_POWEROFF_END_CL)
    //Power off GPU in frame end
    NEMA_BUILDCL_END
    lv_ambiq_nema_gpu_check_busy_and_suspend();
#endif

    nema_cl_destroy(&cl_memcpy);

    return 0;
}

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

#if USE_DIRECT_MODE && !USE_FULL_REFRESH
    //Do the frame buffer synchronization.
    lv_color_t *src;
    lv_color_t *des;
    if(color_p == g_sFrameBuffer[0].bo.base_virt)
    {
        src = g_sFrameBuffer[0].bo.base_virt;
        des = g_sFrameBuffer[1].bo.base_virt;
    }
    else if(color_p == g_sFrameBuffer[1].bo.base_virt)
    {
        src = g_sFrameBuffer[1].bo.base_virt;
        des = g_sFrameBuffer[0].bo.base_virt;
    }
    else
    {
        LV_ASSERT(0);
    }

#ifdef USE_DEBUG_PIN
    am_hal_gpio_output_set(DEBUG_PIN_1);
#endif

    GPU_memcpy(LV_AMBIQ_FB_RESX, LV_AMBIQ_FB_RESY, FRAME_BUFFER_FORMAT, des, src);

#ifdef USE_DEBUG_PIN
    am_hal_gpio_output_clear(DEBUG_PIN_1);
#endif

#endif //USE_DIRECT_MODE && !USE_FULL_REFRESH

    //Trigger the display refresh task.
#ifdef USE_DEBUG_PIN
    am_hal_gpio_output_set(DEBUG_PIN_2);
    am_hal_gpio_output_clear(DEBUG_PIN_2);
#endif
    if(!displayOn)
    {
        display_panel_enable();
    }
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

    img->bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, size);

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
    ret = SSRAM_buffer_alloc(&g_sFrameBuffer[0]);
    if ( ret < 0 )
    {
       return ret;
    }

    ret = SSRAM_buffer_alloc(&g_sFrameBuffer[1]);
    if ( ret < 0 )
    {
       return ret;
    }

    // Init display buffer
    lv_disp_draw_buf_init(&disp_buf, 
                          g_sFrameBuffer[0].bo.base_virt, 
                          g_sFrameBuffer[1].bo.base_virt,
                          g_sFrameBuffer[0].w * g_sFrameBuffer[0].h);

    // Init display driver
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = display_flush_cb;
    disp_drv.gpu_wait_cb = lv_gpu_ambiq_nema_wait;
    disp_drv.hor_res = LV_AMBIQ_FB_RESX;
    disp_drv.ver_res = LV_AMBIQ_FB_RESY;
    disp_drv.full_refresh = USE_FULL_REFRESH;
    disp_drv.direct_mode = USE_DIRECT_MODE;
    disp_drv.user_data = (void*)&g_sFrameBuffer[0];

    // Register the driver and save the created display objects.
    lv_disp_t * disp;
    disp = lv_disp_drv_register(&disp_drv);
    LV_UNUSED(disp);

    return ret;
}

//*****************************************************************************
//
// GUI Timer and Callback
//
//*****************************************************************************
TimerHandle_t gui_timer = NULL;
volatile bool gui_timer_expired = false;

#if ( BTS_TASK_ENABLE == 1 )
void
gui_timer_callback(TimerHandle_t xTimer)
{
 
    am_util_stdio_printf("GUI timer expired\r\n");
    am_util_stdio_printf("Watch Face OFF\r\n");

    gui_timer_expired = true;
        
}
#endif

//*****************************************************************************
//
// Task function.
//
//*****************************************************************************
extern SemaphoreHandle_t g_semBtsEvent;

void
DisplayTask(void *pvParameters)
{
    int ret;
    uint32_t ui32Status = 0;
    display_refresh_cmd cmdDisplayRefreshRecord = cmdDisplayRefresh;

    am_util_stdio_printf("Display task start!\n");

#ifdef USE_DEBUG_PIN
    am_hal_gpio_pinconfig(DEBUG_PIN_1, am_hal_gpio_pincfg_output); //keep high when the GPU memcpy is working
    am_hal_gpio_pinconfig(DEBUG_PIN_2, am_hal_gpio_pincfg_output); //Toggle when the GPU finished his work
    am_hal_gpio_pinconfig(DEBUG_PIN_3, am_hal_gpio_pincfg_output); //Keep high when the display task is active
    am_hal_gpio_pinconfig(DEBUG_PIN_4, am_hal_gpio_pincfg_output); //Keep high when the display data transfer is active
    am_hal_gpio_pinconfig(DEBUG_PIN_5, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_6, am_hal_gpio_pincfg_output);
#endif

#if ( BTS_TASK_ENABLE == 1 )
    // Create a one-shot timer that expires after 5 seconds
    gui_timer = xTimerCreate("GUI Timer", 5000 / portTICK_PERIOD_MS, false, (void *)0, gui_timer_callback);
    
    if (gui_timer == NULL)
    {
        am_util_stdio_printf("Could not create GUI Timer\r\n");
    }
#endif

    //
    // Init display hardware, including display interface and panel.
    //
#ifdef APOLLO5_FPGA
    ret = am_devices_display_init(LV_AMBIQ_FB_RESX,
                                  LV_AMBIQ_FB_RESY,
                                  DISPLAY_FORMAT,
                                  false);
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC192, NULL);
#else
    ret = am_devices_display_init(LV_AMBIQ_FB_RESX,
                                  LV_AMBIQ_FB_RESY,
                                  DISPLAY_FORMAT,
                                  false);
#endif
    if (ret != 0)
    {
#if (DISP_CTRL_IP == DISP_CTRL_IP_DC)
        if(ret == AM_DEVICES_DISPLAY_STATUS_OUT_OF_RANGE)
        {
            //Check the dsi frequency does exceed the limit of the screen or not, if it is exceeded, set it according to the screen maximum
            #if (DISPLAY_FORMAT == COLOR_FORMAT_RGB565)
                if ((g_sDispCfg.eDsiFreq & 0x0f) > (AM_HAL_DSI_FREQ_TRIM_X13 & 0x0f))
                {
                    g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X13;
                    am_util_stdio_printf("Warning: The dsi frequency exceeds screen limit, reset to AM_HAL_DSI_FREQ_TRIM_X13\n");
                }
            #elif (DISPLAY_FORMAT == COLOR_FORMAT_RGB888)
                if ((g_sDispCfg.eDsiFreq & 0x0f) >= (AM_HAL_DSI_FREQ_TRIM_X20 & 0x0f))
                {
                    g_sDispCfg.eDsiFreq = AM_HAL_DSI_FREQ_TRIM_X20;
                    am_util_stdio_printf("Warning: The dsi frequency exceeds screen limit, reset to AM_HAL_DSI_FREQ_TRIM_X20\n");
                }
            #endif

            //reinit
            ret = am_devices_display_init(LV_AMBIQ_FB_RESX,
                                  LV_AMBIQ_FB_RESY,
                                  DISPLAY_FORMAT,
                                  true);
            if (ret != 0)
            {
                am_util_stdio_printf("display init failed!\n");
                //suspend and delete this task.
                vTaskDelete(NULL);
            }
        }
        else
#endif
        {
            am_util_stdio_printf("display init failed!\n");
            //suspend and delete this task.
            vTaskDelete(NULL);
        }
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

    display_watchface = true;

#if ( BTS_TASK_ENABLE == 1 )
    vTaskDelay(1000); // give enough time for semaphore task creation
    TEST_ASSERT_TRUE(g_semBtsEvent != NULL);
    if ( g_semBtsEvent == NULL )
    {
        am_util_stdio_printf("g_semBtsEvent not available, deleting GUI task!\n");
        vTaskDelete(NULL);
    }

    while(1)
    {
        display_panel_disable();

        // wait for semaphore from bring to see task
        xSemaphoreTake(g_semBtsEvent, portMAX_DELAY);


        display_watchface = true;

        am_util_stdio_printf("Starting GUI Timer...\r\n");
        xTimerReset(gui_timer, portMAX_DELAY);

#endif

        while(display_watchface)
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
                am_devices_display_set_region(cmdDisplayRefresh.width,
                                            cmdDisplayRefresh.hight,
                                            cmdDisplayRefresh.start_x,
                                            cmdDisplayRefresh.start_y);

                //Updated display region record.
                cmdDisplayRefreshRecord = cmdDisplayRefresh;
            }
#else
            LV_UNUSED(cmdDisplayRefreshRecord);
#endif

            am_devices_display_transfer_frame(cmdDisplayRefresh.width,
                                              cmdDisplayRefresh.hight,
                                              (uint32_t)cmdDisplayRefresh.pFrameBuffer,
                                              NULL, NULL);


#ifdef USE_DEBUG_PIN
            am_hal_gpio_output_set(DEBUG_PIN_4);
#endif
    
            //Wait transfer done.
            uint32_t transfer_ret = am_devices_display_wait_transfer_done();
            if(AM_DEVICES_DISPLAY_STATUS_ERROR == transfer_ret)
            {
                LV_LOG_ERROR("Display refresh timeout!\n");
            }



#ifdef USE_DEBUG_PIN
            am_hal_gpio_output_clear(DEBUG_PIN_4);
#endif

            // Inform LVGL library that the framebuffer is avaliable*/
            lv_disp_flush_ready(cmdDisplayRefresh.disp_drv);

#ifdef USE_DEBUG_PIN
            am_hal_gpio_output_clear(DEBUG_PIN_3);
#endif
            if(gui_timer_expired)
            {
                display_watchface = false;
                gui_timer_expired = false;
            }

        }
#if ( BTS_TASK_ENABLE == 1 )
    }
#endif

}
