//*****************************************************************************
//
//! @file nemagfx_tp_svg_map.c
//!
//! @brief NemaGFX Touch Panel Rotating Zoom Example
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_tp_svg_map NemaGFX Touch Panel Rotating Zoom Example
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
#include "nemagfx_tp_svg_map.h"
#include "map_tsvg.h"

// #define SUPPORT_LOG
#ifdef SUPPORT_LOG
#define APPLICATION_PRINT(...) am_util_stdio_printf(__VA_ARGS__)
#else
#define APPLICATION_PRINT(...)
#endif

#define SSRAM_MAP_SVG                   (0x20280000 + 0)

typedef struct {
    float zoom_factor;
    float rotation_angle;
} compare_info_t;

typedef struct {
    float x;
    float y;
} offset_info_t;

typedef enum
{
    TOUCH_RELEASE    = 0,
    TOUCH_ONE_POINT,
    TOUCH_TWO_POINTS
}touch_status_e;

void            *g_pCHSC5816Handle;
void            *g_pIOMCHSC5816Handle;

AM_SHARED_RW uint32_t DMATCBBuffer[1024];
am_devices_iom_chsc5816_config_t g_sI2cNBConfig =
{
    .ui32ClockFreq          = AM_HAL_IOM_400KHZ,
    .ui32NBTxnBufLength     = sizeof(DMATCBBuffer)/4,
    .pNBTxnBuf              = DMATCBBuffer
};

// Config CQ buffer locate in SSRAM as non-cacheable.
am_hal_mpu_attr_t sMPUAttr =
{
    .ui8AttrIndex = 0,
    .bNormalMem = true,
    .sOuterAttr = {.bNonTransient = false, .bWriteBack = true, .bReadAllocate = false, .bWriteAllocate = false},
    .sInnerAttr = {.bNonTransient = false, .bWriteBack = true, .bReadAllocate = false, .bWriteAllocate = false},
    .eDeviceAttr = 0
};
am_hal_mpu_region_config_t sMPUCfg =
{
    .ui32RegionNumber = 0,
    .ui32BaseAddress = (uint32_t)DMATCBBuffer,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)DMATCBBuffer + sizeof(DMATCBBuffer) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true
};

static nema_matrix3x3_t matrix;
nema_cmdlist_t cl_svg;
static offset_info_t g_sCurrentOffset = {0,0};
static compare_info_t g_sCompareInfo = {1, 0.0};
static am_devices_tc_chsc5816_info_t g_sIntTouchInfo = {0};
static am_devices_tc_chsc5816_info_t g_sTouchInfo = {0};
static am_devices_tc_chsc5816_info_t g_sTPLastTouch = {0};

touch_status_e eTouchStatus = TOUCH_RELEASE;
volatile bool g_TP_Triggered = false;
void *g_pSvgBin = NULL;
static bool g_bXferDone = false;

img_obj_t g_sFrameBuffer[2] = 
{
    {{0},  RESX, RESY, -1, 1, NEMA_RGB24, NEMA_FILTER_BL},
    {{0},  RESX, RESY, -1, 1, NEMA_RGB24, NEMA_FILTER_BL},
};

img_obj_t* g_pFrameBufferGPU = &g_sFrameBuffer[0];
img_obj_t* g_pFrameBufferDC  = &g_sFrameBuffer[1];


static void calculate_frame_from_multi_touch(void);
static void touch_handler(void *);

//
// Take over the interrupt handler for whichever IOM we're using.
//
#define fram_iom_isr                                                          \
    am_iom_isr1(AM_BSP_TP_IOM_MODULE)
#define am_iom_isr1(n)                                                        \
    am_iom_isr(n)
#define am_iom_isr(n)                                                         \
    am_iomaster ## n ## _isr
//*****************************************************************************
//
// IOM ISRs.
//
//*****************************************************************************
void
fram_iom_isr(void)
{
    uint32_t ui32Status;
    if (!am_hal_iom_interrupt_status_get(g_pIOMCHSC5816Handle, true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMCHSC5816Handle, ui32Status);
            am_hal_iom_interrupt_service(g_pIOMCHSC5816Handle, ui32Status);
        }
    }
}

void pfnCHSC5816_GETPOINT_Callback(void *pCallbackCtxt)
{
    g_bXferDone = true;
}

//*****************************************************************************
//
// Handler of touch gpio interrupt
//
//*****************************************************************************
static void touch_handler(void *x)
{
    am_devices_chsc5816_nonblocking_get_point(g_pCHSC5816Handle, &g_sIntTouchInfo, pfnCHSC5816_GETPOINT_Callback, NULL);
}

//*****************************************************************************
//
// Record touch infomation
//
//*****************************************************************************
void touch_process(void)
{
    if (g_sDispCfg.eIC == DISP_IC_CO5300)
    {
        if(!g_sIntTouchInfo.touch_released)
        {
            if (g_sIntTouchInfo.finger_number == 2)
            {
                //Record the last touch point, and get the current touch point.
                memcpy((void*)(&g_sTPLastTouch), (const void*)&g_sTouchInfo, sizeof(g_sTouchInfo));
                memcpy((void*)(&g_sTouchInfo), (const void*)&g_sIntTouchInfo, sizeof(g_sTouchInfo));

                if(eTouchStatus != TOUCH_TWO_POINTS)
                {
                    // If it wasn't dual touch before, initial point.
                    memcpy((void*)(&g_sTPLastTouch), (const void*)&g_sTouchInfo, sizeof(g_sTouchInfo));
                }
                APPLICATION_PRINT("g_sTPLastTouch x0 = %d, y0 = %d, x1 = %d, y1 = %d\n",
                                g_sTPLastTouch.x0, g_sTPLastTouch.y0,
                                g_sTPLastTouch.x1, g_sTPLastTouch.y1);
                APPLICATION_PRINT("g_sTouchInfo x0 = %d, y0 = %d, x1 = %d, y1 = %d\n",
                                g_sTouchInfo.x0, g_sTouchInfo.y0,
                                g_sTouchInfo.x1, g_sTouchInfo.y1);

                eTouchStatus = TOUCH_TWO_POINTS;
            }
            else if (g_sIntTouchInfo.finger_number == 1)
            {
                //Record the last touch point, and get the current touch point.
                memcpy((void*)(&g_sTPLastTouch), (const void*)&g_sTouchInfo, sizeof(g_sTouchInfo));
                memcpy((void*)(&g_sTouchInfo), (const void*)&g_sIntTouchInfo, sizeof(g_sTouchInfo));

                if(eTouchStatus != TOUCH_ONE_POINT)
                {
                    // If it wasn't single touch before, initial point.
                    memcpy((void*)(&g_sTPLastTouch), (const void*)&g_sTouchInfo, sizeof(g_sTouchInfo));
                }
                APPLICATION_PRINT("g_sTPLastTouch x0 = %d, y0 = %d \n",
                g_sTPLastTouch.x0, g_sTPLastTouch.y0);
                APPLICATION_PRINT("g_sTouchInfo x0 = %d, y0 = %d \n",
                g_sTouchInfo.x0, g_sTouchInfo.y0);

                eTouchStatus = TOUCH_ONE_POINT;
            }
        }
        else
        {
            eTouchStatus = TOUCH_RELEASE;
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

//*****************************************************************************
//
// Init Framebuffer
//
//*****************************************************************************
void fb_init(void)
{
    uint32_t size;

    size = nema_texture_size(g_pFrameBufferGPU->format, 0, g_pFrameBufferGPU->w, g_pFrameBufferGPU->h);

    g_sFrameBuffer[0].bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, size);

    if ( g_sFrameBuffer[0].bo.base_virt == NULL )
    {
      am_util_stdio_printf("TSI buffer Malloc failed!\n");
    }

    g_sFrameBuffer[1].bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, size);

    if ( g_sFrameBuffer[1].bo.base_virt == NULL )
    {
      am_util_stdio_printf("TSI buffer Malloc failed!\n");
    }
}

//*****************************************************************************
//
// swap fb pointers
//
//*****************************************************************************
void swap_fb(void)
{
    img_obj_t* temp;
    temp = g_pFrameBufferGPU;
    g_pFrameBufferGPU = g_pFrameBufferDC;
    g_pFrameBufferDC = temp;
}

//*****************************************************************************
//
// Load svg texture
//
//*****************************************************************************
void load_objects(void)
{
    nema_memcpy((void*)SSRAM_MAP_SVG, map_tsvg, map_tsvg_length);
    g_pSvgBin = (void*)SSRAM_MAP_SVG;
}

float calculateAngle(float x1, float y1, float x2, float y2)
{
    float angle = nema_atan2_r((y2 - y1) , (x2 - x1)) * (float)(180.0 / NEMA_PI);
    return angle;
}

//*****************************************************************************
//
// Calculate X,Y offset
//
//*****************************************************************************
void calculate_frame_from_single_touch(void)
{
    g_sCurrentOffset.x = g_sTPLastTouch.x0 - g_sTouchInfo.x0;
    g_sCurrentOffset.y = g_sTPLastTouch.y0 - g_sTouchInfo.y0;

    APPLICATION_PRINT("x_offset = %.02f;   y_offset = %.02f;\n", g_sCurrentOffset.x, g_sCurrentOffset.y);
}


//*****************************************************************************
//
// Calculate zoom factor and rotation angle
//
//*****************************************************************************
void calculate_frame_from_multi_touch(void)
{
    float last_x_size;
    float last_y_size;
    float last_tri_size;

    float current_x_size;
    float current_y_size;
    float current_tri_size;

    last_x_size = abs(g_sTPLastTouch.x1 - g_sTPLastTouch.x0);
    last_y_size = abs(g_sTPLastTouch.y1 - g_sTPLastTouch.y0);
    last_tri_size = nema_sqrt( nema_pow(last_x_size, 2) + nema_pow(last_y_size, 2));

    current_x_size = abs(g_sTouchInfo.x1 - g_sTouchInfo.x0);
    current_y_size = abs(g_sTouchInfo.y1 - g_sTouchInfo.y0);
    current_tri_size = nema_sqrt( nema_pow(current_x_size, 2) + nema_pow(current_y_size, 2));

    g_sCompareInfo.zoom_factor = current_tri_size/last_tri_size;

    float last_angle;
    float current_angle;

    last_angle = calculateAngle(g_sTPLastTouch.x0, g_sTPLastTouch.y0,
                                g_sTPLastTouch.x1, g_sTPLastTouch.y1);
    current_angle = calculateAngle(g_sTouchInfo.x0, g_sTouchInfo.y0,
                                g_sTouchInfo.x1, g_sTouchInfo.y1);
    g_sCompareInfo.rotation_angle = current_angle - last_angle;
    
    APPLICATION_PRINT("zoom_factor = %.02f\n", g_sCompareInfo.zoom_factor);
    APPLICATION_PRINT("last rotation_angle = %.02f, current rotation_angle = %.02f, diff angle = %.02f\n",
                          last_angle, current_angle, g_sCompareInfo.rotation_angle);
}

//*****************************************************************************
//
// Draw vg map.
//
//*****************************************************************************
void map_rotation_zoom(void)
{
    nema_cl_wait(&cl_svg);
    nema_cl_bind_circular(&cl_svg);
    nema_cl_rewind(&cl_svg);

    if(eTouchStatus == TOUCH_TWO_POINTS)
    {
        nema_mat3x3_translate(matrix, -RESX / 2, -RESY / 2);
        nema_mat3x3_rotate(matrix, g_sCompareInfo.rotation_angle);
        nema_mat3x3_scale(matrix, g_sCompareInfo.zoom_factor, g_sCompareInfo.zoom_factor);
        nema_mat3x3_translate(matrix, RESX / 2, RESY / 2);
    }
    else if(eTouchStatus == TOUCH_ONE_POINT)
    {
        nema_mat3x3_translate(matrix, -g_sCurrentOffset.x, -g_sCurrentOffset.y);
    }
    nema_vg_set_global_matrix(matrix);

    nema_set_clip(0, 0, RESX, RESY);
    // Bind Framebuffer
    nema_bind_dst_tex((uintptr_t)g_pFrameBufferGPU->bo.base_virt, RESX, RESY, NEMA_RGB24, -1);
    nema_clear(0xffffffff);

    nema_vg_draw_tsvg(g_pSvgBin);
    nema_cl_submit(&cl_svg);

    //
    // wait transfer done
    //
    am_devices_display_wait_transfer_done();
    swap_fb();

    am_devices_display_transfer_frame(g_pFrameBufferDC->w,
                                g_pFrameBufferDC->h,
                                g_pFrameBufferDC->bo.base_phys,
                                NULL, NULL);
}

void test_touch_panel(void)
{
    float default_scale = 0.f;
    uint32_t svg_w, svg_h;

    // Set an initial scale
    if ( default_scale == 0.f )
    {
        // Get resource width and hight.
        nema_vg_get_tsvg_resolution(g_pSvgBin, &svg_w, &svg_h);

        // Some svg file doesn't provide this information.
        if ( (svg_w == 0U) || (svg_h == 0U) )
        {
            svg_w = 498;
            svg_h = 600;
        }

        float scalex = g_pFrameBufferGPU->w / (float)svg_w;
        float scaley = g_pFrameBufferGPU->h / (float)svg_h;
        default_scale  = nema_min2(scalex, scaley);
    }

    APPLICATION_PRINT("rasterizing svg %f x %f\n", svg_w, svg_h);
    APPLICATION_PRINT("default_scale: %f\n",  default_scale);

    nema_mat3x3_load_identity(matrix);
    nema_mat3x3_translate(matrix, -(svg_w * 0.5f), -(svg_h * 0.5f));
    nema_mat3x3_scale(matrix, default_scale, default_scale);
    nema_mat3x3_translate(matrix, RESX / 2, RESY / 2);

    cl_svg = nema_cl_create_sized(16 * 1024);

    map_rotation_zoom();

    while(1)
    {
        if(g_bXferDone)
        {
            g_bXferDone = false;
            touch_process();
            if(eTouchStatus == TOUCH_TWO_POINTS)
            {
                if (asset_touch_info(&g_sTouchInfo))
                {
                    calculate_frame_from_multi_touch();
                    map_rotation_zoom();
                }
            }
            else if(eTouchStatus == TOUCH_ONE_POINT)
            {
                calculate_frame_from_single_touch();
                map_rotation_zoom();
            }
        }
    }
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
    am_hal_cachectrl_dcache_disable();

    am_hal_gpio_pinconfig(12, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_clear(12);

    // Set up the attributes.
    am_hal_mpu_attr_configure(&sMPUAttr, sizeof(sMPUAttr) / sizeof(am_hal_mpu_attr_t));
    //
    // Clear the MPU regions.
    //
    am_hal_mpu_region_clear();
    //
    // Set up the regions.
    //
    am_hal_mpu_region_configure(&sMPUCfg, sizeof(sMPUCfg) / sizeof(am_hal_mpu_region_config_t));
    //
    // Invalidate and clear DCACHE, this is required by CM55 TRF.
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);
    //
    // MPU enable
    //
    am_hal_mpu_enable(true, true);

    g_sDispCfg.eIC = DISP_IC_CO5300;
    //
    // It has the restriction that the layer size is a multiple of 4 for Apollo5.
    //
    g_sDispCfg.ui16ResX = RESX;
    g_sDispCfg.ui16ResY = RESY;
    g_sDispCfg.ui16Offset = 6;
    //
    // Initialize display
    //
    am_devices_display_init(RESX,
                            RESY,
                            COLOR_FORMAT_RGB888,
                            false);

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

#ifdef CPU_RUN_IN_HP_MODE
    if ( am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("CPU enter HP mode failed!\n");
    }
#endif

#ifdef GPU_RUN_IN_HP_MODE
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);

    //
    //Switch to HP mode.
    //
    am_hal_pwrctrl_gpu_mode_e current_mode;
    am_hal_pwrctrl_gpu_mode_select(AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE);
    am_hal_pwrctrl_gpu_mode_status(&current_mode);
    if ( AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE != current_mode )
    {
        am_util_stdio_printf("gpu switch to HP mode failed!\n");
    }
#endif

    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    //
    // Initialize NemaGFX
    //
    nema_init();

    //initialize nema_vg
    nema_vg_init(g_pFrameBufferGPU->w, g_pFrameBufferGPU->h);

    nema_vg_handle_large_coords(1U, 1U);

    //
    // Initialize Touch
    //
    if (g_sDispCfg.eIC == DISP_IC_CO5300)
    {
        am_devices_chsc5816_init(AM_BSP_TP_IOM_MODULE, &g_sI2cNBConfig, &g_pCHSC5816Handle, &g_pIOMCHSC5816Handle, AM_BSP_GPIO_TOUCH_INT, AM_BSP_GPIO_TOUCH_RST, touch_handler, NULL);
    }
    else
    {
        APPLICATION_PRINT("Error touch IC not supported!\n");
    }

    //
    // Initialize frame buffer
    //
    fb_init();

    //
    // Load texture
    //
    load_objects();

    //
    // touch test
    //
    test_touch_panel();

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

