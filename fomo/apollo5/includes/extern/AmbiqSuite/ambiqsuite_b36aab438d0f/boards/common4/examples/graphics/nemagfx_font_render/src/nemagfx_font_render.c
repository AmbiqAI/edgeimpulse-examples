//*****************************************************************************
//
//! @file nemagfx_font_render.c
//!
//! @brief NemaGFX Font Render Example.
//!
//! @addtogroup graphics_examples Graphics Examples
//!
//! @defgroup nemagfx_font_render NemaGFX Font Render Example
//! @ingroup graphics_examples
//! @{
//!
//! Purpose: This example demonstrate 8 types of font,from alphabet to symbol,then to chinese.
//! with different alignment.
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

#include "nemagfx_font_render.h"

#include "simhei16pt1b.h"
#include "simhei16pt2b.h"
#include "simhei16pt4b.h"
#include "simhei16pt8b.h"

//#define AM_DEBUG_PRINTF
//*****************************************************************************
//
// Variables Definition.
//
//*****************************************************************************
#if defined (AM_PART_APOLLO4B)
    static img_obj_t g_sFB = {{0}, RESX, RESY, RESX*4, 0, NEMA_RGBA8888, 0};
#else
    static img_obj_t g_sFB = {{0}, RESX, RESY, RESX*3, 0, NEMA_RGB24, 0};
#endif
//*****************************************************************************
//
//! @brief initialize frame buffer.
//!
//! This function create and assign 8 types of font memories.it will loop forever
//! when creating memories failure.
//!
//! @return None.
//
//*****************************************************************************
static void
load_objects(void)
{
    //
    // Load memory objects
    //
    g_sFB.bo = nema_buffer_create(g_sFB.stride * (int32_t)g_sFB.h);
    (void)nema_buffer_map(&g_sFB.bo);

#if USE_1BIT_FONT != 0
    g_sDejaVuSerif8pt1b.bo = nema_buffer_create(g_sDejaVuSerif8pt1b.bitmap_size);
    if(g_sDejaVuSerif8pt1b.bo.base_virt == NULL)
    {
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }
    (void)nema_buffer_map(&g_sDejaVuSerif8pt1b.bo);
    (void)nema_memcpy(g_sDejaVuSerif8pt1b.bo.base_virt, g_sDejaVuSerif8pt1b.bitmap, (size_t)g_sDejaVuSerif8pt1b.bitmap_size);
#endif

#if USE_2BIT_FONT != 0
    g_sDejaVuSerif8pt2b.bo = nema_buffer_create(g_sDejaVuSerif8pt2b.bitmap_size);
    if(g_sDejaVuSerif8pt2b.bo.base_virt == NULL)
    {
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }
    (void)nema_buffer_map(&g_sDejaVuSerif8pt2b.bo);
    (void)nema_memcpy(g_sDejaVuSerif8pt2b.bo.base_virt, g_sDejaVuSerif8pt2b.bitmap, (size_t)g_sDejaVuSerif8pt2b.bitmap_size);
#endif

#if USE_4BIT_FONT != 0
    g_sDejaVuSerif8pt4b.bo = nema_buffer_create(g_sDejaVuSerif8pt4b.bitmap_size);
    if(g_sDejaVuSerif8pt4b.bo.base_virt == NULL)
    {
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }
    (void)nema_buffer_map(&g_sDejaVuSerif8pt4b.bo);
    (void)nema_memcpy(g_sDejaVuSerif8pt4b.bo.base_virt, g_sDejaVuSerif8pt4b.bitmap, (size_t)g_sDejaVuSerif8pt4b.bitmap_size);
#endif

#if USE_8BIT_FONT != 0
    g_sDejaVuSerif8pt8b.bo = nema_buffer_create(g_sDejaVuSerif8pt8b.bitmap_size);
    if(g_sDejaVuSerif8pt8b.bo.base_virt == NULL)
    {
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }
    (void)nema_buffer_map(&g_sDejaVuSerif8pt8b.bo);
    (void)nema_memcpy(g_sDejaVuSerif8pt8b.bo.base_virt, g_sDejaVuSerif8pt8b.bitmap, (size_t)g_sDejaVuSerif8pt8b.bitmap_size);
#endif
    simhei16pt1b.bo = nema_buffer_create(simhei16pt1b.bitmap_size);
    if(simhei16pt1b.bo.base_virt == NULL)
    {
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }
    (void)nema_memcpy(simhei16pt1b.bo.base_virt, simhei16pt1b.bitmap, (size_t)simhei16pt1b.bitmap_size);

    simhei16pt2b.bo = nema_buffer_create(simhei16pt2b.bitmap_size);
    if(simhei16pt2b.bo.base_virt == NULL)
    {
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }
    (void)nema_memcpy(simhei16pt2b.bo.base_virt, simhei16pt2b.bitmap, (size_t)simhei16pt2b.bitmap_size);

    simhei16pt4b.bo = nema_buffer_create(simhei16pt4b.bitmap_size);
    if(simhei16pt4b.bo.base_virt == NULL)
    {
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }
    (void)nema_memcpy(simhei16pt4b.bo.base_virt, simhei16pt4b.bitmap, (size_t)simhei16pt4b.bitmap_size);

    simhei16pt8b.bo = nema_buffer_create(simhei16pt8b.bitmap_size);
    if(simhei16pt8b.bo.base_virt == NULL)
    {
        am_util_stdio_printf("Failed to create FB!\n");
        while(1);
    }
    (void)nema_memcpy(simhei16pt8b.bo.base_virt, simhei16pt8b.bitmap, (size_t)simhei16pt8b.bitmap_size);
}

//*****************************************************************************
//
//! @brief Four quarters display different fonts for every frame.
//!
//! This function display one type of font on one quaters,the whole screen could
//! demonstrate four types for every frame
//!
//! @return Zero.
//
//*****************************************************************************
int32_t
font_render()
{
    load_objects();

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
    nema_set_blend_fill(NEMA_BL_SRC);
    //
    // Fill Rectangle with Color
    //
    nema_fill_rect(0, 0, RESX, RESY, 0x10101010);

    char str[] = "0 bpp - Hello World!!!\n|-------------------------------|\nThe quick brown fox jumps     over the lazy dog!\n\
ThisIsAVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryVeryLongString\nH NemaGFX υποστηρίζει ΚΑΙ Unicode χαρακτήρες!!!";

    int32_t w, h;

#if   USE_1BIT_FONT != 0
    nema_bind_font(&g_sDejaVuSerif8pt1b);
#elif USE_2BIT_FONT != 0
    nema_bind_font(&g_sDejaVuSerif8pt2b);
#elif USE_4BIT_FONT != 0
    nema_bind_font(&g_sDejaVuSerif8pt4b);
#elif USE_8BIT_FONT != 0
    nema_bind_font(&g_sDejaVuSerif8pt8b);
#endif

    const int32_t spacing = 5;

    (void)nema_string_get_bbox(str, (int *)(&w), (int *)(&h), RESX / 2 - spacing * 3, 1);
    h += spacing;
    w += spacing;

    int32_t x, y;

    const int32_t xs[4] = {spacing, w + 2 * spacing, spacing, w + 2 * spacing};
    const int32_t ys[4] = {spacing, spacing, h + 2 * spacing, h + 2 * spacing};
    int32_t idx = 0;

    //
    // first a quarter of frame
    //
#if USE_1BIT_FONT != 0
    x = xs[idx];
    y = ys[idx];
    ++idx;

    str[0] = '1';

    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(x, y, w, h, 0x70707070U);
    nema_bind_font(&g_sDejaVuSerif8pt1b);
    nema_print(str, x, y, w, h, 0xff00ff80U, NEMA_ALIGNX_LEFT | NEMA_TEXT_WRAP | NEMA_ALIGNY_TOP);

    nema_cl_submit(&sCL);
    (void)nema_cl_wait(&sCL);

    nema_cl_rewind(&sCL);
#endif
    //
    // second a quarter of frame
    //
#if USE_2BIT_FONT != 0
    x = xs[idx];
    y = ys[idx];
    ++idx;

    str[0] = '2';

    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(x, y, w, h, 0x70707070U);
    nema_bind_font(&g_sDejaVuSerif8pt2b);
    nema_print(str, x, y, w, h, 0xff00ffffU, NEMA_ALIGNX_RIGHT | NEMA_TEXT_WRAP | NEMA_ALIGNY_BOTTOM);

    nema_cl_submit(&sCL);
    (void)nema_cl_wait(&sCL);

    nema_cl_rewind(&sCL);
#endif
    //
    // third a quarter of frame
    //
#if USE_4BIT_FONT != 0
    x = xs[idx];
    y = ys[idx];
    ++idx;

    str[0] = '4';

    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(x, y, w, h, 0x70707070U);
    nema_bind_font(&g_sDejaVuSerif8pt4b);
    nema_print(str, x, y, w, h, 0xff0080ffU, NEMA_ALIGNX_CENTER | NEMA_TEXT_WRAP | NEMA_ALIGNY_CENTER);

    nema_cl_submit(&sCL);
    (void)nema_cl_wait(&sCL);

    nema_cl_rewind(&sCL);
#endif
    //
    // fourth a quarter of frame
    //
#if USE_8BIT_FONT != 0

    x = xs[idx];
    y = ys[idx];
    ++idx;

    str[0] = '8';

    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(x, y, w, h, 0x70707070U);
    nema_bind_font(&g_sDejaVuSerif8pt8b);
    nema_print(str, x, y, w, h, 0x808ff08fU, NEMA_ALIGNX_JUSTIFY | NEMA_TEXT_WRAP | NEMA_ALIGNY_JUSTIFY);

    nema_cl_submit(&sCL);
    (void)nema_cl_wait(&sCL);

#endif
    //
    // transfer frame to the display
    //
    am_devices_display_transfer_frame(g_sFB.w,
                                      g_sFB.h,
                                      g_sFB.bo.base_phys,
                                      NULL, NULL);
    //
    // wait transfer done
    //
    am_devices_display_wait_transfer_done();

    am_util_delay_ms(5000);
    //
    // start for chinese font display
    //
    nema_clear(0x00000000);

    char str_chinese[] = "这是汉字库图形的例程\
                          这是汉字库图形的例程\
                          这是汉字库图形的例程\
                          这是汉字库图形的例程\
                          这是汉字库图形的例程\
                          这是汉字库图形的例程\
                          这是汉字库图形的例程\
                          这是汉字库图形的例程\n";
    idx = 0;

    //
    // First a Quarter of frame
    //
    x = xs[idx];
    y = ys[idx];
    ++idx;

    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(x, y, w, h, 0x70707070U);
    nema_bind_font(&simhei16pt1b);
    nema_print(str_chinese, x, y, w, h, 0xff00ff80U, NEMA_ALIGNX_RIGHT | NEMA_TEXT_WRAP | NEMA_ALIGNY_BOTTOM);

    nema_cl_submit(&sCL);
    (void)nema_cl_wait(&sCL);

    nema_cl_rewind(&sCL);

    //
    // Second a Quarter of frame
    //
    x = xs[idx];
    y = ys[idx];
    ++idx;

    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(x, y, w, h, 0x70707070U);
    nema_bind_font(&simhei16pt2b);
    nema_print(str_chinese, x, y, w, h, 0xff00ffffU, NEMA_ALIGNX_RIGHT | NEMA_TEXT_WRAP | NEMA_ALIGNY_BOTTOM);

    nema_cl_submit(&sCL);
    (void)nema_cl_wait(&sCL);

    nema_cl_rewind(&sCL);

    //
    // Third a Quarter of frame
    //
    x = xs[idx];
    y = ys[idx];
    ++idx;

    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(x, y, w, h, 0x70707070U);
    nema_bind_font(&simhei16pt4b);
    nema_print(str_chinese, x, y, w, h, 0xff0080ffU, NEMA_ALIGNX_RIGHT  | NEMA_TEXT_WRAP | NEMA_ALIGNY_BOTTOM);

    nema_cl_submit(&sCL);
    (void)nema_cl_wait(&sCL);

    nema_cl_rewind(&sCL);

    //
    // Fourth a Quarter of frame
    //
    x = xs[idx];
    y = ys[idx];
    ++idx;

    nema_set_blend_fill(NEMA_BL_SRC);
    nema_fill_rect(x, y, w, h, 0x70707070U);
    nema_bind_font(&simhei16pt8b);
    nema_print(str_chinese, x, y, w, h, 0x808ff08fU, NEMA_ALIGNX_RIGHT  | NEMA_TEXT_WRAP | NEMA_ALIGNY_BOTTOM);

    nema_cl_submit(&sCL);
    (void)nema_cl_wait(&sCL);

    nema_cl_rewind(&sCL);

    //
    // transfer frame to the display
    //
    am_devices_display_transfer_frame(g_sFB.w,
                                      g_sFB.h,
                                      g_sFB.bo.base_phys,
                                      NULL, NULL);
    //
    // wait transfer done
    //
    am_devices_display_wait_transfer_done();

    nema_cl_destroy(&sCL);
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
#ifdef AM_DEBUG_PRINTF
    am_bsp_debug_printf_enable();
#endif

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

    am_util_stdio_printf("nemafgx_font_render Example\n");

    font_render();

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

