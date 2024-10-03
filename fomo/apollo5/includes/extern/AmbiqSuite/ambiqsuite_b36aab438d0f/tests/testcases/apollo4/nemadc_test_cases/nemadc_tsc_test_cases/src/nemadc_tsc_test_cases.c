//*****************************************************************************
//
//! @file nemadc_tsc_test_cases.c
//!
//! @brief NemaDC TSC test cases.
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
#include "sky_40x40_tsc4.h"
#include "sky_40x40_tsc6.h"
#include "sky_40x40_tsc6a.h"
#include "sky_40x40_rgba.h"
#include "nemadc_test_common.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define FB_RESX 40
#define FB_RESY 40

//*****************************************************************************
//
//! @brief Test NemaDC TSC4 decoding.
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
nemadc_tsc4_test(void)
{
    bool bTestPass = true; 
    uint16_t ui16PanelResX = 390; // panel's max resolution
    uint16_t ui16PanelResY = 390; // panel's max resolution
    uint16_t ui16MinX, ui16MinY;
    uint32_t ui32RReg, ui32RData = 0;
    uint32_t ui32FreqDiv;
    uint8_t ui8Byte0, ui8Byte1;
    uint32_t ui32PixelFormat = MIPICFG_1RGB888_OPT0;
    am_util_stdio_printf("\nDC TSC4 decode test...\n");
    //
    // Assign a fixed value to display type.
    //
    g_eDispType = RM67162_SPI4;
    //
    // Set the display region to center of panel
    //
    ui16MinX = (FB_RESX >= ui16PanelResX)? 0 : ((ui16PanelResX - FB_RESX) >> 2)  << 1;
    ui16MinY = (FB_RESY >= ui16PanelResY)? 0 : ((ui16PanelResY - FB_RESY) >> 2)  << 1;

    am_devices_nemadc_rm67162_init(MIPICFG_SPI4, ui32PixelFormat, FB_RESX, FB_RESY, ui16MinX, ui16MinY);
    //
    // send layer 0 to display via NemaDC
    //
    nemadc_layer_t layer0;
    layer0.resx          = FB_RESX;
    layer0.resy          = FB_RESY;
    layer0.buscfg        = 0;
    layer0.format        = NEMADC_TSC4;
    layer0.blendmode     = NEMADC_BL_SRC;
    layer0.stride        = layer0.resx * 2;
    layer0.startx        = 0;
    layer0.starty        = 0;
    layer0.sizex         = layer0.resx;
    layer0.sizey         = layer0.resy;
    layer0.alpha         = 0xff;
    layer0.flipx_en      = 0;
    layer0.flipy_en      = 0;
    layer0.baseaddr_virt = tsi_malloc(layer0.resy * layer0.stride);
    layer0.baseaddr_phys = (unsigned)tsi_virt2phys(layer0.baseaddr_virt);
    memcpy((char*)layer0.baseaddr_virt, sky_40x40_tsc4, sky_40x40_tsc4_length);

    nemadc_set_layer(0, &layer0); // This function includes layer enable.
    nemadc_send_frame_single();
    nemadc_layer_disable(0);
    tsi_free(layer0.baseaddr_virt);
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

    if((((int32_t)ui8Byte1 - (int32_t)sky_40x40_rgba[1]) > 1) || (((int32_t)ui8Byte1 - (int32_t)sky_40x40_rgba[1]) < -1) || (((int32_t)ui8Byte0 - (int32_t)sky_40x40_rgba[0]) > 1) || (((int32_t)ui8Byte0 - (int32_t)sky_40x40_rgba[0]) < -1))
    {
        bTestPass = false;
        am_util_stdio_printf("DC TSC4 decode test failed! Expected 0x%02X 0x%02X, was 0x%02X 0x%02X!\n", sky_40x40_rgba[0], sky_40x40_rgba[1], ui8Byte0, ui8Byte1);
    }
    else
    {
        am_util_stdio_printf("DC TSC4 decode test passed! Expected 0x%02X 0x%02X, was 0x%02X 0x%02X!\n", sky_40x40_rgba[0], sky_40x40_rgba[1], ui8Byte0, ui8Byte1);
    }
    nemadc_reg_write(NEMADC_REG_CLKCTRL, ui32RReg); // Disabled format clock division. 
    am_util_delay_ms(100); // Added a delay to wait CLK back to stable.
    
    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;

} // nemadc_tsc4_test()

//*****************************************************************************
//
//! @brief Test NemaDC TSC6 decoding.
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
nemadc_tsc6_test(void)
{
    bool bTestPass = true; 
    uint16_t ui16PanelResX = 390; // panel's max resolution
    uint16_t ui16PanelResY = 390; // panel's max resolution
    uint16_t ui16MinX, ui16MinY;
    uint32_t ui32RReg, ui32RData = 0;
    uint32_t ui32FreqDiv;
    uint8_t ui8Byte0, ui8Byte1;
    uint32_t ui32PixelFormat = MIPICFG_1RGB888_OPT0;
    am_util_stdio_printf("\nDC TSC6 decode test...\n");
    //
    // Set the display region to center of panel
    //
    ui16MinX = (FB_RESX >= ui16PanelResX)? 0 : ((ui16PanelResX - FB_RESX) >> 2)  << 1;
    ui16MinY = (FB_RESY >= ui16PanelResY)? 0 : ((ui16PanelResY - FB_RESY) >> 2)  << 1;

    am_devices_nemadc_rm67162_init(MIPICFG_SPI4, ui32PixelFormat, FB_RESX, FB_RESY, ui16MinX, ui16MinY);
    //
    // send layer 0 to display via NemaDC
    //
    nemadc_layer_t layer0;
    layer0.resx          = FB_RESX;
    layer0.resy          = FB_RESY;
    layer0.buscfg        = 0;
    layer0.format        = NEMADC_TSC6;
    layer0.blendmode     = NEMADC_BL_SRC;
    layer0.stride        = layer0.resx * 3;
    layer0.startx        = 0;
    layer0.starty        = 0;
    layer0.sizex         = layer0.resx;
    layer0.sizey         = layer0.resy;
    layer0.alpha         = 0xff;
    layer0.flipx_en      = 0;
    layer0.flipy_en      = 0;
    layer0.baseaddr_virt = tsi_malloc(layer0.resy * layer0.stride);
    layer0.baseaddr_phys = (unsigned)tsi_virt2phys(layer0.baseaddr_virt);
    memcpy((char*)layer0.baseaddr_virt, sky_40x40_tsc6, sky_40x40_tsc6_length);

    nemadc_set_layer(0, &layer0); // This function includes layer enable.
    nemadc_send_frame_single();
    nemadc_layer_disable(0);
    tsi_free(layer0.baseaddr_virt);
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

    if((((int32_t)ui8Byte1 - (int32_t)sky_40x40_rgba[1]) > 1) || (((int32_t)ui8Byte1- (int32_t)sky_40x40_rgba[1]) < -1) || (((int32_t)ui8Byte0 - (int32_t)sky_40x40_rgba[0]) > 1) || (((int32_t)ui8Byte0 - (int32_t)sky_40x40_rgba[0]) < -1))
    {
        bTestPass = false;
        am_util_stdio_printf("DC TSC6 decode test failed! Expected 0x%02X 0x%02X, was 0x%02X 0x%02X!\n", sky_40x40_rgba[0], sky_40x40_rgba[1], ui8Byte0, ui8Byte1);
    }
    else
    {
        am_util_stdio_printf("DC TSC6 decode test passed! Expected 0x%02X 0x%02X, was 0x%02X 0x%02X!\n", sky_40x40_rgba[0], sky_40x40_rgba[1], ui8Byte0, ui8Byte1);
    }
    nemadc_reg_write(NEMADC_REG_CLKCTRL, ui32RReg); // Disabled format clock division.
    am_util_delay_ms(100); // Added a delay to wait CLK back to stable.
    
    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;

} // nemadc_tsc6_test()

//*****************************************************************************
//
//! @brief Test NemaDC TSC6a decoding.
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
nemadc_tsc6a_test(void)
{
    bool bTestPass = true; 
    uint16_t ui16PanelResX = 390; // panel's max resolution
    uint16_t ui16PanelResY = 390; // panel's max resolution
    uint16_t ui16MinX, ui16MinY;
    uint32_t ui32RReg, ui32RData = 0;
    uint32_t ui32FreqDiv;
    uint8_t ui8Byte0, ui8Byte1;
    uint32_t ui32PixelFormat = MIPICFG_1RGB888_OPT0;
    am_util_stdio_printf("\nDC TSC6a decode test...\n");
    //
    // Set the display region to center of panel
    //
    ui16MinX = (FB_RESX >= ui16PanelResX)? 0 : ((ui16PanelResX - FB_RESX) >> 2)  << 1;
    ui16MinY = (FB_RESY >= ui16PanelResY)? 0 : ((ui16PanelResY - FB_RESY) >> 2)  << 1;

    am_devices_nemadc_rm67162_init(MIPICFG_SPI4, ui32PixelFormat, FB_RESX, FB_RESY, ui16MinX, ui16MinY);
    //
    // send layer 0 to display via NemaDC
    //
    nemadc_layer_t layer0;
    layer0.resx          = FB_RESX;
    layer0.resy          = FB_RESY;
    layer0.buscfg        = 0;
    layer0.format        = NEMADC_TSC6A;
    layer0.blendmode     = NEMADC_BL_SRC;
    layer0.stride        = layer0.resx * 3;
    layer0.startx        = 0;
    layer0.starty        = 0;
    layer0.sizex         = layer0.resx;
    layer0.sizey         = layer0.resy;
    layer0.alpha         = 0xff;
    layer0.flipx_en      = 0;
    layer0.flipy_en      = 0;
    layer0.baseaddr_virt = tsi_malloc(layer0.resy * layer0.stride);
    layer0.baseaddr_phys = (unsigned)tsi_virt2phys(layer0.baseaddr_virt);
    memcpy((char*)layer0.baseaddr_virt, sky_40x40_tsc6a, sky_40x40_tsc6a_length);

    nemadc_set_layer(0, &layer0); // This function includes layer enable.
    nemadc_send_frame_single();
    nemadc_layer_disable(0);
    tsi_free(layer0.baseaddr_virt);
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

    if((((int32_t)ui8Byte1 - (int32_t)sky_40x40_rgba[1]) > 1) || (((int32_t)ui8Byte1 - (int32_t)sky_40x40_rgba[1]) < -1) || (((int32_t)ui8Byte0 - (int32_t)sky_40x40_rgba[0]) > 1) || (((int32_t)ui8Byte0 - (int32_t)sky_40x40_rgba[0]) < -1))
    {
        bTestPass = false;
        am_util_stdio_printf("DC TSC6a decode test failed! Expected 0x%02X 0x%02X, was 0x%02X 0x%02X!\n", sky_40x40_rgba[0], sky_40x40_rgba[1], ui8Byte0, ui8Byte1);
    }
    else
    {
        am_util_stdio_printf("DC TSC6a decode test passed! Expected 0x%02X 0x%02X, was 0x%02X 0x%02X!\n", sky_40x40_rgba[0], sky_40x40_rgba[1], ui8Byte0, ui8Byte1);
    }
    nemadc_reg_write(NEMADC_REG_CLKCTRL, ui32RReg); // Disabled format clock division.
    am_util_delay_ms(100); // Added a delay to wait CLK back to stable.

    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;

} // nemadc_tsc6a_test()
