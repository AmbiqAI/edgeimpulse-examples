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
nemadc_layer_t dc_layer = {(void *)0, 0, FB_RESX, FB_RESY, -1, 0, 0, FB_RESX, FB_RESY, 0xff, NEMADC_BL_SRC, 0, NEMADC_TSC6, 0, 0, 0, 0, 0,0,0, 0};
AM_SHARED_RW nema_cmdlist_t cl;

static uint8_t index = 0;

uint32_t g_gpu_checksum_table[3] = { 0x00003F39, 0x000046B2, 0x00008D6D};
uint32_t g_dc_checksum_table[2][3]  = {{ 0x1372c447, 0xf3425cd1, 0x4e9c37c8},{ 0xd01c541e, 0xf8a05cc0, 0x65acee15}};

bool get_checksum(uint32_t ref_checksum)
{
    bool bPass = true;
    uint32_t * buff;
    uint32_t checksum, i;
    buff = (uint32_t *)dc_layer.baseaddr_virt;
    checksum = 0;
    for(i = 0; i < FB_RESX * FB_RESY * 3 / 4 / 4; i++) // TSC6 format
    {
        checksum += buff[i];
    }
    checksum = (checksum >> 16) + (checksum & 0xffff);
    checksum += (checksum >> 16);
    checksum = 0xffff - checksum;
    if (checksum != ref_checksum)
    {
        am_util_stdio_printf("ERROR! Expected FB checksum is 0x%08X, current checksum is 0x%08X.\n",
                            ref_checksum, checksum);
        bPass = false;
    }
    else
    {
        am_util_stdio_printf("Current FB checksum is equal to expected checksum (0x%08X).\n",
                            ref_checksum);
    }

    return bPass;
}


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
    uint16_t ui16MinX, ui16MinY;
    nemadc_initial_config_t sDCConfig;
    am_devices_dc_xspi_raydium_config_t sDisplayPanelConfig;
    uint32_t ui32SampleType;
    uint32_t ui32TexColor;
    //
    // Assign a fixed value to display type.
    //
    g_sDispCfg.eInterface = DISP_IF_SPI4;
    //
    // Set the display region to center
    //
    if (FB_RESX < g_sDispCfg.ui16ResX)
    {
        sDisplayPanelConfig.ui16ResX = FB_RESX;
    }
    else
    {
        sDisplayPanelConfig.ui16ResX = g_sDispCfg.ui16ResX;
    }
    ui16MinX = (g_sDispCfg.ui16ResX - sDisplayPanelConfig.ui16ResX) >> 1;
    ui16MinX = (ui16MinX >> 1) << 1;

    if (FB_RESY < g_sDispCfg.ui16ResY)
    {
        sDisplayPanelConfig.ui16ResY = FB_RESY;
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

    am_devices_dc_xspi_raydium_hardware_reset();
    sDCConfig.eInterface = DISP_INTERFACE_SPI4;
    sDCConfig.ui32PixelFormat = MIPICFG_1RGB888_OPT0;
    sDisplayPanelConfig.ui32PixelFormat = sDCConfig.ui32PixelFormat;
    nemadc_configure(&sDCConfig);
    am_devices_dc_xspi_raydium_init(&sDisplayPanelConfig);

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
    if(0)
    {
        index = 1;
    }
    else
    {
        index = 0;
    }

    for(ui32SampleType = 0; ui32SampleType < 3; ui32SampleType++) // Removed MIRROR test
    {
        am_util_stdio_printf("\nNemaGFX blit (sampling type - %d) test.\n", ui32SampleType);
        nema_bind_src_tex(AM_logo.bo.base_phys, AM_logo.w, AM_logo.h, AM_logo.format, AM_logo.stride, NEMA_FILTER_PS | sampling_type[ui32SampleType]);
        nema_blit_subrect(0, 0, FB_RESX, FB_RESY, -50, -50);
        nema_cl_submit(&cl);
        nema_cl_wait(&cl);
        nema_cl_rewind(&cl);
        if (get_checksum(g_gpu_checksum_table[ui32SampleType]) == false)
        {
            bTestPass = false;
        }
        nemadc_set_layer(0, &dc_layer);
        nemadc_transfer_frame_prepare(false);
        //
        //It's necessary to launch frame manually when TE is disabled.
        //
        nemadc_transfer_frame_launch();
        nemadc_wait_vsync();
        nemadc_layer_disable(0);
#ifdef WITH_DISPLAY
        uint32_t ui32FreqDiv, ui32RReg;
        uint32_t ui32RData = 0;
        uint8_t ui8Byte0, ui8Byte1;
        uint8_t ui8Golden[2];
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
            am_util_stdio_printf("ERROR! Expected pixel data are 0x%02X 0x%02X, current data were 0x%02X 0x%02X.\n", ui8Golden[0], ui8Golden[1], ui8Byte0, ui8Byte1);
        }
        else
        {
            am_util_stdio_printf("Current pixel data are equal to expected data (0x%02X 0x%02X).\n", ui8Golden[0], ui8Golden[1]);
        }
        nemadc_reg_write(NEMADC_REG_CLKCTRL, ui32RReg); // Disabled format clock division.
        am_util_delay_ms(100); // Added a delay to wait CLK back to stable.
#endif
        uint32_t read_crc_dc;
        read_crc_dc = nemadc_get_crc();
        if (read_crc_dc != g_dc_checksum_table[index][ui32SampleType])
        {
            bTestPass = false;
            am_util_stdio_printf("ERROR! Expected DC checksum is 0x%08x, current checksum is 0x%08x\n",
                                g_dc_checksum_table[index][ui32SampleType], read_crc_dc);
        }
        else
        {
            am_util_stdio_printf("Current DC checksum is equal to expected checksum (0x%08X).\n",
                                g_dc_checksum_table[index][ui32SampleType]);
        }
    }

    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;

} // nemagfx_blit_test()
