//*****************************************************************************
//
//! @file nemadc_jdi_test_case.c
//!
//! @brief NemaDC jdi interfaces test cases.
//
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_bsp.h"
#include "nema_hal.h"
#include "nema_dc.h"
#include "nema_dc_mipi.h"
#include "am_util_delay.h"
#include "am_util_stdio.h"
#include "nema_dc_regs.h"
#include "tsi_malloc.h"
#include "string.h"
#include "am_devices_dc_jdi_sharp.h"

#include "jdi_rgba4444.h"
#include "jdi_rgba2222.h"
#include "jdi_rgba332.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//#define LOAD_TEXTURE      //if not comment out this macro,this example will load texture as input source.

//
// the below two values are dependent on your screen size.
//
#define FB_RESX             280
#define FB_RESY             280

//
// rgba2222 color format
//
#define FB_RED_HI           0x80
#define FB_RED_LO           0x40
#define FB_GREEN_HI         0x20
#define FB_GREEN_LO         0x10
#define FB_BLUE_HI          0x08
#define FB_BLUE_LO          0x04

//#define FB_COLORS           4
#define FB_BOX_SIZE         25 // its should not greater than FB_RESX/8 = 35.
#define FB_BOX_VERTEX_X     (FB_RESX - 8 * FB_BOX_SIZE)/2
#define FB_BOX_VERTEX_Y     (FB_RESY - 8 * FB_BOX_SIZE)/2

uint8_t jdi_fb_array[FB_RESX][FB_RESY];

//*****************************************************************************
//
// Optional setup/tear-down functions.
//
// These will be called before and after each test function listed below.
//
//*****************************************************************************
void
setUp(void)
{
}

void
tearDown(void)
{
}

//*****************************************************************************
//
// Optional Global setup/tear-down.
//
// globalSetUp() will get called before the test group starts, and
// globalTearDown() will get called after it ends. These won't be necessary for
// every test.
//
//*****************************************************************************
void
globalSetUp(void)
{
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC48, NULL);
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);


    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo5 JDI Test Cases\n\n");

    nema_sys_init();
    //
    // Initialize NemaDC
    //
    if(nemadc_init() != 0)
    {
        am_util_stdio_printf("NemaDC init failed!\n");
    }
}

void
globalTearDown(void)
{
}

//*****************************************************************************
//
//! @brief Test Nemadc jdi interface.
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
nemadc_jdi_test(void)
{
    bool bTestPass = false;
    am_devices_dc_jdi_timer_config_t sTimerConfig = {
        .ui32TimerNum = 7,
        .eTimerClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16,
        .ui32Frequency = 60
	};
	nemadc_initial_config_t sDCConfig;
	
    uint8_t (* jdi_fb)[FB_RESY] = NULL; 
    //
    // Assign a fixed value to display type.
    //
    g_sDispCfg.eInterface = DISP_IF_JDI;

    am_devices_dc_jdi_sharp_init(&sTimerConfig, &sDCConfig);

    nemadc_configure(&sDCConfig);

    nemadc_layer_t layer0 = {0};
    layer0.resx      = sDCConfig.ui16ResX;
    layer0.resy      = sDCConfig.ui16ResY * 2;
    layer0.buscfg    = 0;
    layer0.format    = NEMADC_RGBA2222;
    layer0.blendmode = NEMADC_BL_SRC;
    layer0.stride    = layer0.resx * 1;
    layer0.startx    = 0;
    layer0.starty    = 0;
    layer0.alpha     = 0xff;
    layer0.flipx_en  = 0;
    layer0.flipy_en  = 0;
    layer0.baseaddr_virt = tsi_malloc(FB_RESY * layer0.stride);
    layer0.baseaddr_phys = (unsigned)(layer0.baseaddr_virt);

#ifdef LOAD_TEXTURE
    //
    // display image
    //
    nema_memcpy(layer0.baseaddr_virt,jdi_rgba2222,FB_RESY * layer0.stride);
#else
    //
    // display 8 X 8 colors palette
    //
    jdi_fb = layer0.baseaddr_virt;
    //
    // fill palette with colors.
    //
    memset(jdi_fb,0x00,layer0.resy * layer0.stride);

    uint16_t minx,maxx;
    uint16_t miny = FB_BOX_VERTEX_Y,maxy = FB_BOX_VERTEX_Y + 2*FB_BOX_SIZE;

    for(uint16_t y = miny; y < maxy; y++)
    {
        //red color bitfield is 0b00
        minx = FB_BOX_VERTEX_X;
        maxx = FB_BOX_VERTEX_X + 2*FB_BOX_SIZE;
        for(uint16_t x = minx; x < maxx; x++)
        {
            //green color bitfield is 0xb00
            uint8_t bluel = (x - minx) > FB_BOX_SIZE ? FB_BLUE_LO : 0x00;
            uint8_t blueh = (y - miny) > FB_BOX_SIZE ? FB_BLUE_HI : 0x00;
            jdi_fb[x][y] = 0x00000000 | blueh | bluel;
        }
        minx = FB_BOX_VERTEX_X + 2*FB_BOX_SIZE;
        maxx = FB_BOX_VERTEX_X + 4*FB_BOX_SIZE;
        for(uint16_t x = minx; x < maxx; x++)
        {
            //green color bitfield is 0xb01
            uint8_t bluel = (x - minx) > FB_BOX_SIZE ? FB_BLUE_LO : 0x00;
            uint8_t blueh = (y - miny) > FB_BOX_SIZE ? FB_BLUE_HI : 0x00;
            jdi_fb[x][y] = FB_GREEN_LO | blueh | bluel;
        }
        minx = FB_BOX_VERTEX_X + 4*FB_BOX_SIZE;
        maxx = FB_BOX_VERTEX_X + 6*FB_BOX_SIZE;
        for(uint16_t x = minx; x < maxx; x++)
        {
            //green color bitfield is 0xb10
            uint8_t bluel = (x - minx) > FB_BOX_SIZE ? FB_BLUE_LO : 0x00;
            uint8_t blueh = (y - miny) > FB_BOX_SIZE ? FB_BLUE_HI : 0x00;
            jdi_fb[x][y] = FB_GREEN_HI | blueh | bluel;
        }
        minx = FB_BOX_VERTEX_X + 6*FB_BOX_SIZE;
        maxx = FB_BOX_VERTEX_X + 8*FB_BOX_SIZE;
        for(uint16_t x = minx; x < maxx; x++)
        {
            //green color bitfield is 0xb11
            uint8_t bluel = (x - minx) > FB_BOX_SIZE ? FB_BLUE_LO : 0x00;
            uint8_t blueh = (y - miny) > FB_BOX_SIZE ? FB_BLUE_HI : 0x00;
            jdi_fb[x][y] = FB_GREEN_HI | FB_GREEN_LO | blueh | bluel;
        }
    }
    miny = FB_BOX_VERTEX_Y + 2*FB_BOX_SIZE;
    maxy = FB_BOX_VERTEX_Y + 4*FB_BOX_SIZE;
    for(uint16_t y = miny; y < maxy; y++)
    {
        //red color bitfield is 0b01
        minx = FB_BOX_VERTEX_X;
        maxx = FB_BOX_VERTEX_X + 2*FB_BOX_SIZE;
        for(uint16_t x = minx; x < maxx; x++)
        {
            //green color bitfield is 0xb00
            uint8_t bluel = (x - minx) > FB_BOX_SIZE ? FB_BLUE_LO : 0x00;
            uint8_t blueh = (y - miny) > FB_BOX_SIZE ? FB_BLUE_HI : 0x00;
            jdi_fb[x][y] = FB_RED_LO | blueh | bluel;
        }
        minx = FB_BOX_VERTEX_X + 2*FB_BOX_SIZE;
        maxx = FB_BOX_VERTEX_X + 4*FB_BOX_SIZE;
        for(uint16_t x = minx; x < maxx; x++)
        {
            //green color bitfield is 0xb01
            uint8_t bluel = (x - minx) > FB_BOX_SIZE ? FB_BLUE_LO : 0x00;
            uint8_t blueh = (y - miny) > FB_BOX_SIZE ? FB_BLUE_HI : 0x00;
            jdi_fb[x][y] = FB_RED_LO | FB_GREEN_LO | blueh | bluel;
        }
        minx = FB_BOX_VERTEX_X + 4*FB_BOX_SIZE;
        maxx = FB_BOX_VERTEX_X + 6*FB_BOX_SIZE;
        for(uint16_t x = minx; x < maxx; x++)
        {
            //green color bitfield is 0xb10
            uint8_t bluel = (x - minx) > FB_BOX_SIZE ? FB_BLUE_LO : 0x00;
            uint8_t blueh = (y - miny) > FB_BOX_SIZE ? FB_BLUE_HI : 0x00;
            jdi_fb[x][y] = FB_RED_LO | FB_GREEN_HI | blueh | bluel;
        }
        minx = FB_BOX_VERTEX_X + 6*FB_BOX_SIZE;
        maxx = FB_BOX_VERTEX_X + 8*FB_BOX_SIZE;
        for(uint16_t x = minx; x < maxx; x++)
        {
            //green color bitfield is 0xb11
            uint8_t bluel = (x - minx) > FB_BOX_SIZE ? FB_BLUE_LO : 0x00;
            uint8_t blueh = (y - miny) > FB_BOX_SIZE ? FB_BLUE_HI : 0x00;
            jdi_fb[x][y] = FB_RED_LO | FB_GREEN_HI | FB_GREEN_LO | blueh | bluel;
        }
    }

    miny = FB_BOX_VERTEX_Y + 4*FB_BOX_SIZE;
    maxy = FB_BOX_VERTEX_Y + 6*FB_BOX_SIZE;
    for(uint16_t y = miny; y < maxy; y++)
    {
        //red color bitfield is 0b10
        minx = FB_BOX_VERTEX_X;
        maxx = FB_BOX_VERTEX_X + 2*FB_BOX_SIZE;
        for(uint16_t x = minx; x < maxx; x++)
        {
            //green color bitfield is 0xb00
            uint8_t bluel = (x - minx) > FB_BOX_SIZE ? FB_BLUE_LO : 0x00;
            uint8_t blueh = (y - miny) > FB_BOX_SIZE ? FB_BLUE_HI : 0x00;
            jdi_fb[x][y] = FB_RED_HI | blueh | bluel;
        }
        minx = FB_BOX_VERTEX_X + 2*FB_BOX_SIZE;
        maxx = FB_BOX_VERTEX_X + 4*FB_BOX_SIZE;
        for(uint16_t x = minx; x < maxx; x++)
        {
            //green color bitfield is 0xb01
            uint8_t bluel = (x - minx) > FB_BOX_SIZE ? FB_BLUE_LO : 0x00;
            uint8_t blueh = (y - miny) > FB_BOX_SIZE ? FB_BLUE_HI : 0x00;
            jdi_fb[x][y] = FB_RED_HI | FB_GREEN_LO | blueh | bluel;
        }
        minx = FB_BOX_VERTEX_X + 4*FB_BOX_SIZE;
        maxx = FB_BOX_VERTEX_X + 6*FB_BOX_SIZE;
        for(uint16_t x = minx; x < maxx; x++)
        {
            //green color bitfield is 0xb10
            uint8_t bluel = (x - minx) > FB_BOX_SIZE ? FB_BLUE_LO : 0x00;
            uint8_t blueh = (y - miny) > FB_BOX_SIZE ? FB_BLUE_HI : 0x00;
            jdi_fb[x][y] = FB_RED_HI | FB_GREEN_HI | blueh | bluel;
        }
        minx = FB_BOX_VERTEX_X + 6*FB_BOX_SIZE;
        maxx = FB_BOX_VERTEX_X + 8*FB_BOX_SIZE;
        for(uint16_t x = minx; x < maxx; x++)
        {
            //green color bitfield is 0xb11
            uint8_t bluel = (x - minx) > FB_BOX_SIZE ? FB_BLUE_LO : 0x00;
            uint8_t blueh = (y - miny) > FB_BOX_SIZE ? FB_BLUE_HI : 0x00;
            jdi_fb[x][y] = FB_RED_HI | FB_GREEN_HI | FB_GREEN_LO | blueh | bluel;
        }
    }

    miny = FB_BOX_VERTEX_Y + 6*FB_BOX_SIZE;
    maxy = FB_BOX_VERTEX_Y + 8*FB_BOX_SIZE;
    for(uint16_t y = miny; y < maxy; y++)
    {
        //red color bitfield is 0b11
        minx = FB_BOX_VERTEX_X;
        maxx = FB_BOX_VERTEX_X + 2*FB_BOX_SIZE;
        for(uint16_t x = minx; x < maxx; x++)
        {
            //green color bitfield is 0xb00
            uint8_t bluel = (x - minx) > FB_BOX_SIZE ? FB_BLUE_LO : 0x00;
            uint8_t blueh = (y - miny) > FB_BOX_SIZE ? FB_BLUE_HI : 0x00;
            jdi_fb[x][y] = FB_RED_HI | FB_RED_LO | blueh | bluel;
        }
        minx = FB_BOX_VERTEX_X + 2*FB_BOX_SIZE;
        maxx = FB_BOX_VERTEX_X + 4*FB_BOX_SIZE;
        for(uint16_t x = minx; x < maxx; x++)
        {
            //green color bitfield is 0xb01
            uint8_t bluel = (x - minx) > FB_BOX_SIZE ? FB_BLUE_LO : 0x00;
            uint8_t blueh = (y - miny) > FB_BOX_SIZE ? FB_BLUE_HI : 0x00;
            jdi_fb[x][y] = FB_RED_HI | FB_RED_LO | FB_GREEN_LO | blueh | bluel;
        }
        minx = FB_BOX_VERTEX_X + 4*FB_BOX_SIZE;
        maxx = FB_BOX_VERTEX_X + 6*FB_BOX_SIZE;
        for(uint16_t x = minx; x < maxx; x++)
        {
            //green color bitfield is 0xb10
            uint8_t bluel = (x - minx) > FB_BOX_SIZE ? FB_BLUE_LO : 0x00;
            uint8_t blueh = (y - miny) > FB_BOX_SIZE ? FB_BLUE_HI : 0x00;
            jdi_fb[x][y] = FB_RED_HI | FB_RED_LO | FB_GREEN_HI | blueh | bluel;
        }
        minx = FB_BOX_VERTEX_X + 6*FB_BOX_SIZE;
        maxx = FB_BOX_VERTEX_X + 8*FB_BOX_SIZE;
        for(uint16_t x = minx; x < maxx; x++)
        {
            //green color bitfield is 0xb11
            uint8_t bluel = (x - minx) > FB_BOX_SIZE ? FB_BLUE_LO : 0x00;
            uint8_t blueh = (y - miny) > FB_BOX_SIZE ? FB_BLUE_HI : 0x00;
            jdi_fb[x][y] = FB_RED_HI | FB_RED_LO | FB_GREEN_HI | FB_GREEN_LO | blueh | bluel;
        }
    }
#endif

    //
    // Please flush the D-cache if CPU has directly written frame buffers.
    //
    nema_buffer_t bo;
    bo.size = layer0.resy * layer0.stride;
    bo.base_phys = layer0.baseaddr_phys;
    nema_buffer_flush(&bo);

    am_devices_dc_jdi_sharp_timer_start();
    
    nemadc_set_layer(0, &layer0);
    if (layer0.flipy_en)
    {
        nemadc_set_layer_addr(0, nemadc_reg_read(NEMADC_REG_LAYER_BASEADDR(0)) - (layer0.resx * nemadc_stride_size(layer0.format, layer0.resx)));
    }
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();

    am_util_delay_ms(1000);

    //
    // FlipX & FlipY
    //
    layer0.flipx_en  = 1;
    layer0.flipy_en  = 1;
    nemadc_set_layer(0, &layer0);
    if (layer0.flipy_en)
    {
        nemadc_set_layer_addr(0, nemadc_reg_read(NEMADC_REG_LAYER_BASEADDR(0)) - (layer0.resx * nemadc_stride_size(layer0.format, layer0.resx)));
    }
    nemadc_transfer_frame_launch();
    nemadc_wait_vsync();

    nemadc_layer_disable(0);

    //
    // disable panel
    //
    //am_devices_dc_jdi_sharp_timer_stop();

    tsi_free(layer0.baseaddr_virt);
    am_util_stdio_printf("\nThis example is not an automatic test, it need to manually check!!\n\n");
    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;

} // nemadc_jdi_test()

