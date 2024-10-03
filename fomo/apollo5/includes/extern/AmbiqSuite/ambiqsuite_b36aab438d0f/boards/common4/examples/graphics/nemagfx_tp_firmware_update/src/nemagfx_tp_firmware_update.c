//*****************************************************************************
//
//! @file nemagfx_tp_firmware_update.c
//!
//! @brief NemaGFX Touch Panel FW Upgrade Example
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_tp_firmware_update NemaGFX Touch Panel FW Upgrade Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example demostrate the touch firmware upgrade or degrade using new
//! family display shield board
//! use macro BAREMETAL for baremetal system, comment out BAREMETAL will run
//! with FreeRTOS support
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

#include "am_bsp.h"
#include "am_util.h"
#include "nemagfx_tp_firmware_update.h"
#include "DejaVuSerif8pt1b.h"

//#define SUPPORT_LOG
#ifdef SUPPORT_LOG
#define APPLICATION_PRINT(...) am_util_stdio_printf(__VA_ARGS__)
#else
#define APPLICATION_PRINT(...)
#endif

#ifdef AM_PART_APOLLO4B
AM_SHARED_RW uint32_t axiScratchBuf[20];
#endif

//#define KLEIN_BLUE (0xFF002FA7)
#define KLEIN_BLUE                  (0xFFFFFFFF)
#define FW_NEW_VERSION              (4)
#define FW_OLD_VERSION              (3)
#define BUTTON_COLOR_NOT_PRESSED    (0x70707070)
#define BUTTON_COLOR_PRESSED        (0xA0A0A0A0)

typedef struct {
    float zoom_factor;
    float rotation_angle;
} compare_info_t;

typedef struct _quad_t {
    float x0, y0;
    float x1, y1;
    float x2, y2;
    float x3, y3;
} quad_t;

static am_devices_tc_chsc5816_info_t g_sIntTouchInfo = {0};
static am_devices_tc_chsc5816_info_t g_sTouchInfo = {0};
static am_devices_tc_chsc5816_info_t g_sTPFirstTouch =
{
    .x0 = 128,
    .y0 = 154,
    .x1 = 311,
    .y1 = 128
};

bool touch_press   = false;
bool touch_release = false;
volatile bool g_TP_Triggered = false;

void fw_update_main_screen(uint8_t fw_ver, uint32_t up_button_color, uint32_t down_button_color);
void fw_update_print(void);
static void touch_handler(void *);

static void touch_handler(void *x)
{
    g_TP_Triggered = true;
}

void touch_process(void)
{
    if (g_sDispCfg.eIC == DISP_IC_CO5300)
    {
        if (am_devices_chsc5816_get_point(&g_sIntTouchInfo) == AM_DEVICES_CHSC5816_STATUS_SUCCESS)
        {
            if (g_sIntTouchInfo.finger_number == 2 || g_sIntTouchInfo.finger_number == 1)
            {
                memcpy((void*)(&g_sTouchInfo), (const void*)&g_sIntTouchInfo, sizeof(g_sTouchInfo));

                touch_press = !g_sTouchInfo.touch_released;

                if (touch_press == true)
                {
                    if (g_sTouchInfo.finger_number == 1)
                    {
                        APPLICATION_PRINT("x0 = %d, y0 = %d\n",
                                              g_sTouchInfo.x0, g_sTouchInfo.y0);

                    }
                    else if (g_sTouchInfo.finger_number == 2)
                    {
                        APPLICATION_PRINT("x0 = %d, y0 = %d, x1 = %d, y1 = %d\n",
                                              g_sTouchInfo.x0, g_sTouchInfo.y0, g_sTouchInfo.x1, g_sTouchInfo.y1);
                    }
                    else
                    {
                        APPLICATION_PRINT("finger_number not supported!\n");
                    }
                }
                else
                {
                    memcpy((void*)(&g_sTPFirstTouch), (const void*)&g_sTouchInfo, sizeof(g_sTouchInfo));
                    APPLICATION_PRINT("released x0 = %d, y0 = %d, x1 = %d, y1 = %d\n",
                                          g_sTPFirstTouch.x0, g_sTPFirstTouch.y0,
                                          g_sTPFirstTouch.x1, g_sTPFirstTouch.y1);
                }
            }
        }
    }
    else
    {
        APPLICATION_PRINT("Error, Display with touch module not supported!\n");
        return;
    }
}

int32_t
asset_touch_info(am_devices_tc_chsc5816_info_t *ps_touchInfo)
{
    if (ps_touchInfo->finger_number == 2 &&
        ps_touchInfo->x0 && ps_touchInfo->x0 < RESX &&
        ps_touchInfo->x1 && ps_touchInfo->x1 < RESX &&
        ps_touchInfo->y0 && ps_touchInfo->y0 < RESX &&
        ps_touchInfo->y1 && ps_touchInfo->y1 < RESX)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

void test_touch_panel(void)
{
    fw_update_main_screen(2, BUTTON_COLOR_NOT_PRESSED, BUTTON_COLOR_NOT_PRESSED);

    while(1)
    {
        if (g_TP_Triggered == true)
        {
            touch_process();

            if(touch_press == true)
            {
                if (asset_touch_info(&g_sTouchInfo))
                {
                    if (g_sTouchInfo.y0 < 200)
                    {
                        fw_update_main_screen(FW_NEW_VERSION, BUTTON_COLOR_PRESSED, BUTTON_COLOR_NOT_PRESSED);
                        am_devices_chsc5816_fw_update(FW_NEW_VERSION);
                        g_sTouchInfo.fw_ver = FW_NEW_VERSION;
                    }
                    else if (g_sTouchInfo.y0 > 266)
                    {
                        fw_update_main_screen(FW_OLD_VERSION, BUTTON_COLOR_NOT_PRESSED, BUTTON_COLOR_PRESSED);
                        am_devices_chsc5816_fw_update(FW_OLD_VERSION);
                        g_sTouchInfo.fw_ver = FW_OLD_VERSION;
                    }
                    else
                    {
                        // do nothing
                    }

                    fw_update_print();
                    return;
                }
            }

            g_TP_Triggered = false;
        }
    }
}

static img_obj_t fb;
void fb_reload(void)
{
    fb.w = RESX;
    fb.h = RESY;
    fb.format = NEMA_RGB24;
    fb.stride = fb.w * nema_format_size(fb.format);
    fb.bo = nema_buffer_create(fb.stride * fb.h);
    if(fb.bo.base_virt == (void *)NULL)
    {
        //
        // have no enough space.this check is important!
        //
        APPLICATION_PRINT("Failed to create FB!\n");
        while(1);
    }
    memset(fb.bo.base_virt, 0, fb.stride * fb.w);
    fb.color = 0;

    fb.sampling_mode = 0;

    APPLICATION_PRINT("FB: V:%p P:0x%08x\n", (void *)fb.bo.base_virt, fb.bo.base_phys);
}

void fb_release(void)
{
    nema_buffer_destroy(&fb.bo);
}

nema_img_obj_t scotty_large_indexed_8bits = {{0}, 256, 256, -1, 0, NEMA_L8, NEMA_TEX_CLAMP};
nema_img_obj_t scotty_large_palette_8bits = {{0}, 16, 16, -1, 0, NEMA_RGBA8888, 0};;

void
load_objects(void)
{
    DejaVuSerif8pt1b.bo = nema_buffer_create(DejaVuSerif8pt1b.bitmap_size);

    if(DejaVuSerif8pt1b.bo.base_virt == NULL)
    {
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }

    memcpy(DejaVuSerif8pt1b.bo.base_virt, DejaVuSerif8pt1bBitmaps, DejaVuSerif8pt1b.bitmap_size);
}

float
calculateAngle(float x1, float y1, float x2, float y2)
{
    float angle = nema_atan2_r((y2 - y1) , (x2 - x1)) * (float)(180.0 / NEMA_PI);
    return angle;
}

nema_cmdlist_t cl;

void fw_update_main_screen(uint8_t fw_ver, uint32_t up_button_color, uint32_t down_button_color)
{
    char buf[32] = {0};

    static uint8_t flag = 0;
    if (flag == 0)
    {
        cl = nema_cl_create();
        flag = 1;
    }

    nema_cl_rewind(&cl);
    nema_cl_bind(&cl);
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, fb.stride);

    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_fill(NEMA_BL_SRC);

    nema_clear(0);

    nema_fill_rect (0, 0, fb.w, fb.h/2 - 10, up_button_color);
    nema_fill_rect (0, fb.h/2 + 10, fb.w, fb.h/2 - 10, down_button_color);

    nema_bind_font(&DejaVuSerif8pt1b);

    memset(buf, 0, sizeof(buf));
    memcpy(buf, "Touch to burn FW v04\n", strlen("Touch to burn FW v04\n"));
    nema_print(buf, 130, 100, 200, 32, KLEIN_BLUE, NEMA_ALIGNX_LEFT);

    memset(buf, 0, sizeof(buf));
    memcpy(buf, "Touch to burn FW v03\n", strlen("Touch to burn FW v03\n"));
    nema_print(buf, 130, 330, 200, 32, KLEIN_BLUE, NEMA_ALIGNX_LEFT);

    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    am_devices_display_transfer_frame(fb.w,
                                      fb.h,
                                      fb.bo.base_phys,
                                      NULL, NULL);
    //
    // wait transfer done
    //
    am_devices_display_wait_transfer_done();
}

void fw_update_print(void)
{
    char buf[32] = {0};

    static uint8_t flag = 0;
    if (flag == 0)
    {
        cl = nema_cl_create();
        flag = 1;
    }

    nema_cl_rewind(&cl);
    nema_cl_bind(&cl);
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, fb.stride);

    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_fill(NEMA_BL_SRC);

    nema_clear(0);

    nema_fill_rect (0, 0, fb.w, fb.h, 0x70707070);

    nema_bind_font(&DejaVuSerif8pt1b);

    if (g_sTouchInfo.fw_ver == 3)
    {
        memcpy(buf, "Complete, Touch FW ver is v03\n", strlen("Complete, Touch FW ver is v03\n"));
    }
    else if (g_sTouchInfo.fw_ver == 4)
    {
        memcpy(buf, "Complete, Touch FW ver is v04\n", strlen("Complete, Touch FW ver is v04\n"));
    }
    nema_print(buf, 100, fb.h /2 - 16, 400, 32, KLEIN_BLUE, NEMA_ALIGNX_LEFT);

    nema_cl_submit(&cl);
    nema_cl_wait(&cl);

    am_devices_display_transfer_frame(fb.w,
                                      fb.h,
                                      fb.bo.base_phys,
                                      NULL, NULL);
    //
    // wait transfer done
    //
    am_devices_display_wait_transfer_done();
}

int
touch_test(void)
{
    if (g_sDispCfg.eIC == DISP_IC_CO5300)
    {
        am_devices_chsc5816_init(AM_BSP_TP_IOM_MODULE, touch_handler, NULL);
    }
    else
    {
        APPLICATION_PRINT("Error touch IC not supported!\n");
        return -1;
    }

    fb_reload();
    load_objects();
    test_touch_panel();

    am_util_delay_ms(1000);

    return 0;
}

//*****************************************************************************
//
// Main function
//
//*****************************************************************************

int
main(void)
{
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
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();
    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

    //
    // Initialize display
    //
    am_devices_display_init(RESX,
                            RESY,
                            COLOR_FORMAT_RGB888,
                            false);

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Initialize NemaGFX
    //
    nema_init();

    am_hal_timer_config_t sTimerConfig;
    uint32_t ui32Status;
    ui32Status = am_hal_timer_default_config_set(&sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        APPLICATION_PRINT("Failed to initialize a timer configuration structure with default values!\n");
    }
    sTimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    sTimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    ui32Status = am_hal_timer_config(0, &sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        APPLICATION_PRINT("Failed to configure a timer!\n");
    }
    ui32Status = am_hal_timer_start(0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        APPLICATION_PRINT("Failed to start a timer!\n");
    }

    APPLICATION_PRINT("Restart ********************************************************************\n");

#ifdef BAREMETAL
    touch_test();
#else /* BAREMETAL */
    // Run the application.
    run_tasks();
#endif /* BAREMETAL*/

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

