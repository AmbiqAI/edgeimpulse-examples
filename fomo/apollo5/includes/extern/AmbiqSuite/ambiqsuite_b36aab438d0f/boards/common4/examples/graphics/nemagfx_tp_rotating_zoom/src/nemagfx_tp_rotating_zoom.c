//*****************************************************************************
//
//! @file nemagfx_tp_rotating_zoom.c
//!
//! @brief NemaGFX Touch Panel Rotating Zoom Example
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_tp_rotating_zoom NemaGFX Touch Panel Rotating Zoom Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example demostrate the multi touch function using apollo4 family display
//! shield board.
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
#include "nemagfx_tp_rotating_zoom.h"
#include "scotty_large_256x256_indices.h"
#include "scotty_large_256x256_palette.h"

// #define SUPPORT_LOG
#ifdef SUPPORT_LOG
#define APPLICATION_PRINT(...) am_util_stdio_printf(__VA_ARGS__)
#else
#define APPLICATION_PRINT(...)
#endif

#ifdef AM_PART_APOLLO4B
AM_SHARED_RW uint32_t axiScratchBuf[20];
#endif

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

static compare_info_t g_sCompareInfo = {0.5, 0.0};
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

static void calculate_frame_from_multi_touch(void);
static void rotation_zoom_test(void);
static void touch_handler(void *);

static void touch_handler(void *x)
{
    g_TP_Triggered = true;
}

void touch_process(void)
{
    if (g_sDispCfg.eIC == DISP_IC_RM69330 || g_sDispCfg.eIC == DISP_IC_RM67162)
    {
        if (am_devices_tma525_get_point((am_devices_tc_tma525_info_t *)&g_sTouchInfo) == AM_DEVICES_TMA525_STATUS_SUCCESS)
        {
            touch_release = g_sTouchInfo.touch_released;

            touch_press = !touch_release;

            if (touch_press == true)
            {
                APPLICATION_PRINT("x = %d, y = %d\n", g_sTouchInfo.x0, g_sTouchInfo.y0);
            }

        }
    }
    else if (g_sDispCfg.eIC == DISP_IC_CO5300)
    {
        if (am_devices_chsc5816_get_point(&g_sIntTouchInfo) == AM_DEVICES_CHSC5816_STATUS_SUCCESS)
        {
            if (g_sIntTouchInfo.finger_number == 2)
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
    rotation_zoom_test();

    while(1)
    {
        if (g_TP_Triggered == true)
        {
            touch_process();

            if(touch_press == true)
            {
                if (asset_touch_info(&g_sTouchInfo))
                {
                    calculate_frame_from_multi_touch();
                    rotation_zoom_test();
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
    scotty_large_indexed_8bits.bo = nema_buffer_create(sizeof(scotty_large_256x256_indices));
    memcpy(scotty_large_indexed_8bits.bo.base_virt, scotty_large_256x256_indices,
           sizeof(scotty_large_256x256_indices));
    scotty_large_palette_8bits.bo = nema_buffer_create(sizeof(scotty_large_256x256_palette));
    memcpy(scotty_large_palette_8bits.bo.base_virt, scotty_large_256x256_palette,
           sizeof(scotty_large_256x256_palette));
}

float
calculateAngle(float x1, float y1, float x2, float y2)
{
    float angle = nema_atan2_r((y2 - y1) , (x2 - x1)) * (float)(180.0 / NEMA_PI);
    return angle;
}

void
calculate_frame_from_multi_touch(void)
{
    float last_x_size;
    float last_y_size;
    float last_tri_size;

    float current_x_size;
    float current_y_size;
    float current_tri_size;

    last_x_size = abs(g_sTPFirstTouch.x1 - g_sTPFirstTouch.x0);
    last_y_size = abs(g_sTPFirstTouch.y1 - g_sTPFirstTouch.y0);
    last_tri_size = nema_sqrt( nema_pow(last_x_size, 2) + nema_pow(last_y_size, 2));

    current_x_size = abs(g_sTouchInfo.x1 - g_sTouchInfo.x0);
    current_y_size = abs(g_sTouchInfo.y1 - g_sTouchInfo.y0);
    current_tri_size = nema_sqrt( nema_pow(current_x_size, 2) + nema_pow(current_y_size, 2));

    APPLICATION_PRINT("\n");
    APPLICATION_PRINT("last_tri_size = %.02f\n",last_tri_size);
    APPLICATION_PRINT("current_tri_size = %.02f\n",current_tri_size);
    APPLICATION_PRINT("\n");

    g_sCompareInfo.zoom_factor = current_tri_size/last_tri_size/2;
    APPLICATION_PRINT("magnification = %.02f\n", g_sCompareInfo.zoom_factor);

    float last_angle;
    float current_angle;

    last_angle = calculateAngle(g_sTPFirstTouch.x0, g_sTPFirstTouch.y0,
                                g_sTPFirstTouch.x1, g_sTPFirstTouch.y1);
    current_angle = calculateAngle(g_sTouchInfo.x0, g_sTouchInfo.y0,
                                g_sTouchInfo.x1, g_sTouchInfo.y1);
    g_sCompareInfo.rotation_angle = current_angle - last_angle;

    APPLICATION_PRINT("last rotation_angle = %.02f, current rotation_angle = %.02f, diff angle = %.02f\n",
                          last_angle, current_angle, g_sCompareInfo.rotation_angle);

    return;
}

void rotation_zoom_test(void)
{
    static bool bRunMultiTimes = false;

    if (bRunMultiTimes == true)
    {
        calculate_frame_from_multi_touch();
    }
    bRunMultiTimes = true;

    if (!(g_sCompareInfo.zoom_factor > (float)0.1 && g_sCompareInfo.zoom_factor < (float)5))
    {
        return;
    }

    nema_cmdlist_t cl = nema_cl_create();

    nema_cl_rewind(&cl);
    nema_cl_bind(&cl);

    nema_set_clip(0, 0, RESX, RESY);

    nema_clear(0);

    nema_bind_lut_tex( (uintptr_t)scotty_large_indexed_8bits.bo.base_virt, 256, 256, NEMA_L8, -1, 0,
                          (uintptr_t)scotty_large_palette_8bits.bo.base_virt, NEMA_RGBA8888);

    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, fb.stride);

    nema_set_blend_blit(NEMA_BL_SRC | NEMA_BLOP_LUT);

    quad_t q;
    q.x0 = RESX/2 + 256*g_sCompareInfo.zoom_factor/2  * NEMA_PI * nema_cos(135 - g_sCompareInfo.rotation_angle);
    q.y0 = RESY/2 - 256*g_sCompareInfo.zoom_factor/2  * NEMA_PI * nema_sin(135 - g_sCompareInfo.rotation_angle);
    q.x1 = RESX/2 + 256*g_sCompareInfo.zoom_factor/2  * NEMA_PI * nema_cos(45 -g_sCompareInfo.rotation_angle);
    q.y1 = RESY/2 - 256*g_sCompareInfo.zoom_factor/2  * NEMA_PI * nema_sin(45 -g_sCompareInfo.rotation_angle);
    q.x2 = RESX/2 + 256*g_sCompareInfo.zoom_factor/2  * NEMA_PI * nema_cos(315 -g_sCompareInfo.rotation_angle);
    q.y2 = RESY/2 - 256*g_sCompareInfo.zoom_factor/2  * NEMA_PI * nema_sin(315 -g_sCompareInfo.rotation_angle);
    q.x3 = RESX/2 + 256*g_sCompareInfo.zoom_factor/2  * NEMA_PI * nema_cos(225 -g_sCompareInfo.rotation_angle);
    q.y3 = RESY/2 - 256*g_sCompareInfo.zoom_factor/2  * NEMA_PI * nema_sin(225 -g_sCompareInfo.rotation_angle);

    APPLICATION_PRINT("x0 = %.02f, y0 = %.02f\n", q.x0, q.y0);
    APPLICATION_PRINT("x1 = %.02f, y1 = %.02f\n", q.x1, q.y1);
    APPLICATION_PRINT("x2 = %.02f, y2 = %.02f\n", q.x2, q.y2);
    APPLICATION_PRINT("x3 = %.02f, y3 = %.02f\n", q.x3, q.y3);

    nema_blit_quad_fit(q.x0, q.y0, q.x1, q.y1, q.x2, q.y2, q.x3, q.y3);


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

    nema_cl_destroy(&cl);
}

int
touch_test(void)
{
    if (g_sDispCfg.eIC == DISP_IC_RM69330 || g_sDispCfg.eIC == DISP_IC_RM67162)
    {
        am_devices_tma525_init(AM_BSP_TP_IOM_MODULE, touch_handler, NULL);
    }
    else if (g_sDispCfg.eIC == DISP_IC_CO5300)
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

