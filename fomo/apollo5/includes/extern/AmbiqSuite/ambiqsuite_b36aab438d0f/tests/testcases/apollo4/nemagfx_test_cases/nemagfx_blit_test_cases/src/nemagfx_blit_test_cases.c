//*****************************************************************************
//
//! @file nemagfx_blit_test_cases.c
//!
//! @brief NemaGFX blit test cases.
//! Need to connect RM67162 to DC SPI4 interface.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "nemagfx_test_common.h"
#include "Ambiq200x104_rgba.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define FB_RESX 200
#define FB_RESY 200

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
static img_obj_t fb = {{0}, FB_RESX, FB_RESY, -1, 0, NEMA_TSC6, 0};
img_obj_t AM_logo = {{0}, 64, 64, 200 * 4, 0, NEMA_RGBA8888, 0}; // set to 64*64 or 128*128, repeat blit passed.
nemadc_layer_t dc_layer = {(void *)0, 0, FB_RESX, FB_RESY, -1, 0, 0, FB_RESX, FB_RESY, 0xff, NEMADC_BL_SRC, 0, NEMADC_TSC6, 0, 0, 0, 0, 0,0,0};
AM_SHARED_RW nema_cmdlist_t cl;

//*****************************************************************************
//
//! @brief Load memory objects.
//!
//! @return NULL.
//
//*****************************************************************************
void
load_objects(void)
{
    fb.bo = nema_buffer_create(fb.w * fb.h * 3);
    nema_buffer_map(&fb.bo);

    dc_layer.baseaddr_phys = fb.bo.base_phys;
    dc_layer.baseaddr_virt = fb.bo.base_virt;

    AM_logo.bo = nema_buffer_create(Ambiq200x104_rgba_len);
    nema_memcpy(AM_logo.bo.base_virt, Ambiq200x104, Ambiq200x104_rgba_len);
}

//*****************************************************************************
//
//! @brief Test NemaGFX blit feature.
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
nemagfx_blit_test(void)
{
    bool bTestPass = true; 
    uint16_t ui16PanelResX = 390; // panel'ui32SampleType max resolution
    uint16_t ui16PanelResY = 390; // panel'ui32SampleType max resolution
    uint16_t ui16MinX, ui16MinY;
    uint32_t ui32SampleType, ui32RReg, ui32RData = 0;
    uint32_t ui32FreqDiv;
    uint8_t ui8Byte0, ui8Byte1;
    uint32_t ui32TexColor;
    uint8_t ui8Golden[2];
    //
    // Assign a fixed value to display type.
    //
    g_eDispType = RM67162_SPI4;
    //
    // Set the display region to center
    //
    ui16MinX = (FB_RESX >= ui16PanelResX)? 0 : ((ui16PanelResX - FB_RESX) >> 2)  << 1;
    ui16MinY = (FB_RESY >= ui16PanelResY)? 0 : ((ui16PanelResY - FB_RESY) >> 2)  << 1;

    am_devices_nemadc_rm67162_init(MIPICFG_SPI4, MIPICFG_1RGB888_OPT0, FB_RESX, FB_RESY, ui16MinX, ui16MinY);

    load_objects();

    nemadc_set_layer(0, &dc_layer);

    //Create Command Lists
    cl = nema_cl_create();
    //
    //Bind Command List
    //
    nema_cl_bind(&cl);
    //
    //Bind Framebuffer
    //
    nema_bind_dst_tex(fb.bo.base_phys, fb.w, fb.h, fb.format, fb.stride);
    //
    //Set Clipping Rectangle
    //
    nema_set_clip(0, 0, FB_RESX, FB_RESY);
    //
    // Check Sampling Modes - CLAMP/REPEAT/BORDER/MIRROR
    // MIRROR is not working on the FPGA (probably neither in ASIC)
    // We need to fix this in REV-B chip
    //
    nema_set_blend_blit(NEMA_BL_SRC);
    const uint32_t sampling_type[] =
    {
        NEMA_TEX_CLAMP,
        NEMA_TEX_REPEAT,
        NEMA_TEX_BORDER,
        NEMA_TEX_MIRROR
    };
    ui32TexColor = 0xFFBBCCDD; // A-B-G-R
    nema_set_tex_color(ui32TexColor);

    for(ui32SampleType = 0; ui32SampleType < 3; ui32SampleType++) // Removed MIRROR test 
    {
        nema_bind_src_tex(AM_logo.bo.base_phys, AM_logo.w, AM_logo.h, AM_logo.format, AM_logo.stride, NEMA_FILTER_PS | sampling_type[ui32SampleType]);

        nema_blit_subrect(0, 0, FB_RESX, FB_RESY, -50, -50);

        nema_cl_submit(&cl);
        nema_cl_wait(&cl);
        nema_cl_rewind(&cl);
        nemadc_set_layer(0, &dc_layer);
        nemadc_send_frame_single();
        nemadc_layer_disable(0);

        am_util_delay_ms(500);
        ui32RReg = nemadc_reg_read(NEMADC_REG_CLKCTRL);
#ifdef APOLLO4_FPGA
        //
        // Make sure the frequency after division is lower than 3MHz.
        //
        ui32FreqDiv = ((APOLLO4_FPGA / FORMAT_CLK_DIV / TARGET_FREQ + 1) << DC_DIV2_POS); 
#else
        ui32FreqDiv = (SILICON_FORMAT_DIV << DC_DIV2_POS);
#endif
        nemadc_reg_write(NEMADC_REG_CLKCTRL, ui32RReg | ui32FreqDiv);
        am_util_delay_ms(100); // Added a delay to wait CLK back to stable.
        //
        // read frame from panel
        //
        ui32RData = am_nemadc_read(MIPI_read_memory_start, 3); // first byte is dummy byte.
        ui8Byte0 = ((ui32RData >> 8) & 0x000000FF); 
        ui8Byte1 = (ui32RData & 0x000000FF);
        if (ui32SampleType == 0) // CLAMP
        {
            ui8Golden[0] = Ambiq200x104[0];
            ui8Golden[1] = Ambiq200x104[1];
        }
        else if(ui32SampleType == 1) // REPEAT
        {
            ui8Golden[0] = Ambiq200x104[14 * 200 * 4 + 14 * 4]; // Find the pixel at panel origin. (64-50)*200*4+(64-50)*4
            ui8Golden[1] = Ambiq200x104[14 * 200 * 4 + 14 * 4 + 1];
        }
        else if(ui32SampleType == 2) // BORDER
        {
            ui8Golden[0] = (ui32TexColor & 0x000000FF);
            ui8Golden[1] = ((ui32TexColor >> 8) & 0x000000FF);
        }

        if((ui8Byte1 != ui8Golden[1]) || (ui8Byte0 != ui8Golden[0]))
        {
            bTestPass = false;
            am_util_stdio_printf("NemaGFX blit (sampling type - %d) test failed! Expected 0x%02X 0x%02X, was 0x%02X 0x%02X!\n", ui32SampleType, ui8Golden[0], ui8Golden[1], ui8Byte0, ui8Byte1);
        }
        else
        {
            am_util_stdio_printf("NemaGFX blit (sampling type - %d) test passed! Expected 0x%02X 0x%02X, was 0x%02X 0x%02X!\n", ui32SampleType, ui8Golden[0], ui8Golden[1], ui8Byte0, ui8Byte1);
        }
        nemadc_reg_write(NEMADC_REG_CLKCTRL, ui32RReg); // Disabled format clock division. 
        am_util_delay_ms(100); // Added a delay to wait CLK back to stable.
    }

    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;

} // nemagfx_blit_test()
