//*****************************************************************************
//
//! @file test_display.c
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
// Standard AmbiqSuite includes.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "am_devices_tma525.h"
#include "am_devices_display_generic.h"
//#include "DejaVuSerif8pt1b.h"

//*****************************************************************************
//
// third-party/devices include files.
//
//*****************************************************************************

#include "nema_core.h"
#include "nema_font.h"
#include "nema_event.h"
#include "nema_utils.h"


#ifdef AM_PART_APOLLO4B
AM_SHARED_RW uint32_t axiScratchBuf[20];
#endif

static img_obj_t fb;

#define RESX 454
#define RESY 454

typedef enum
{
    TOUCH_INIT  = 0,
    TOUCH_PRESSED,
    TOUCH_RELEASED,
    TOUCH_PRESSED_AGAIN
} touch_state_t;

static const unsigned long RED       = 0xFF0000FF;
static void touch_handler(void *);

void fill_round_rect_test(uint8_t screen_clear);

#define PATTERN_BUF_SIZE      8

nema_event_t g_event;
bool touch_press   = false;
bool touch_release = false;

static am_devices_tc_tma525_info_t g_sTouchInfo = {0};

static void touch_handler(void *x)
{
    if (am_devices_tma525_get_point(&g_sTouchInfo) == AM_DEVICES_TMA525_STATUS_SUCCESS)
    {
        g_event.mouse_x = g_sTouchInfo.x0;
        g_event.mouse_y = g_sTouchInfo.y0;
        touch_release = g_sTouchInfo.touch_released;

        if (touch_release == true)
        {
            touch_press = false;
        }
        else
        {
            touch_press = true;
            am_util_stdio_printf("x = %d, y = %d\n", g_event.mouse_x, g_event.mouse_y);
        }
    }
}


void fb_release(void)
{
    nema_buffer_destroy(&fb.bo);
}

nema_cmdlist_t cl;
void fill_round_rect_test(uint8_t screen_clear)
{
    char buf[12];
    int x1, y1;
    x1 = RESX / 2 - 40;
    y1 = 50;

    static uint8_t flag = 0;
    if (flag == 0)
    {
        cl = nema_cl_create();
    }

    static int last_x, last_y;

    nema_cl_rewind(&cl);
    nema_cl_bind(&cl);
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, fb.stride);

    nema_set_clip(0, 0, RESX, RESY);
    nema_set_blend_fill(NEMA_BL_SRC);

    if (screen_clear)
    {
        nema_clear(0);
    }

    if (flag == 1)
    {
        nema_draw_line(0, last_y, RESX, last_y, 0);
        nema_draw_line(last_x, 0, last_x, RESY, 0);
        nema_draw_circle(last_x, last_y, 20, 0);
        nema_fill_rect(x1, y1, 100, 32, 0);
        if (screen_clear == 0)
        {
            nema_draw_line(last_x, last_y, g_event.mouse_x, g_event.mouse_y, RED);
        }
    }
    nema_draw_line(0, g_event.mouse_y, RESX, g_event.mouse_y, RED);
    nema_draw_line(g_event.mouse_x, 0, g_event.mouse_x, RESY, RED);
    nema_draw_circle(g_event.mouse_x, g_event.mouse_y, 20, RED);

    last_x = g_event.mouse_x;
    last_y = g_event.mouse_y;

    //am_util_stdio_sprintf(buf, "x%d, y%d", g_event.mouse_x, g_event.mouse_y);
//    nema_bind_font(&DejaVuSerif8pt1b);

    nema_print(buf, x1, y1, 100, 32, RED, NEMA_ALIGNX_LEFT);
    //am_util_stdio_printf(buf);

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

    flag = 1;
}

//*****************************************************************************
//
// Main function
//
//*****************************************************************************

int qspi_display_test(uint32_t mode)
{
    int32_t test_loops = 3;
    //
    // External power on
    //
    am_bsp_external_pwr_on();


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

    am_devices_tma525_init(2, touch_handler, NULL);

    // fb_reload();
    fb.w = RESX;
    fb.h = RESY;
    fb.format = NEMA_RGB24;
    fb.stride = fb.w * nema_format_size(fb.format);
    fb.bo = nema_buffer_create(fb.stride * fb.h);
    if ( fb.bo.base_virt == (void *)NULL )
    {
        //
        // have no enough space.this check is important!
        //
        am_util_stdio_printf("Failed to create FB!\n");
        goto _fail;
    }
    fb.color = 0;

    fb.sampling_mode = 0;

    am_util_debug_printf("FB: V:%p P:0x%08x\n", (void *)fb.bo.base_virt, fb.bo.base_phys);
    g_event.mouse_x = RESX / 2;
    g_event.mouse_y = RESY / 2;
   // test_touch_panel();
    do
    {
        if ( touch_press == true )
        {
            fill_round_rect_test(1);
            test_loops --;
        }
        else
        {
            fill_round_rect_test(0);
        }
    }while(test_loops >= 0);
    //
    // disable the interrupts in the NVIC.
    //
    NVIC_DisableIRQ(g_sInterrupts[TP_GPIO_IDX]);
    return 0;
_fail:
    //
    // disable the interrupts in the NVIC.
    //
    NVIC_DisableIRQ(g_sInterrupts[TP_GPIO_IDX]);
    return -1;
}

