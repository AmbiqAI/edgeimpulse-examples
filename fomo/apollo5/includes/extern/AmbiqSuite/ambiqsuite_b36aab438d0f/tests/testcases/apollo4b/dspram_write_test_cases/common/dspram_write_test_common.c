//*****************************************************************************
//
//! @file mspi_test_common.c
//!
//! @brief Example test case program.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "dspram_write_test_common.h"

#define DEBUG_PRINT        am_util_stdio_printf

void          *pWidget;
char            errStr[128];
uint32_t        DMATCBBuffer[2560];
uint32_t        *g_pDSPRAMAddr[4];
uint32_t        g_uiDSPRAMData[4];
uint8_t WriteBuffer[6] = { 0xA0, 0xA5, 0xAF, 0xA1, 0xA6, 0xAC};

am_hal_pwrctrl_dsp_memory_config_t DSPRAM_Cfg =
{
    .bEnableICache      = false,
    .bRetainCache       = false,
    .bEnableRAM         = true,
    .bActiveRAM         = false,
    .bRetainRAM         = true,
};

am_widget_mspi_devices_config_t MSPI_Flash_Config =
{
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,
    .eClockFreq = AM_HAL_MSPI_CLK_12MHZ,
    .pNBTxnBuf = DMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(DMATCBBuffer) / sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
};

am_widget_mspi_config_t widgetConfig =
{
    .ui32Module = MSPI_TEST_NUM_INSTANCES ,
    .ePSRAM = AM_WIDGET_MSPI_DDR_PSRAM_APS12808L,
    .ui32DMACtrlBufferSize = (sizeof(DMATCBBuffer) / sizeof(uint32_t)),
    .pDMACtrlBuffer = DMATCBBuffer,
};
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
#if (!defined(AM_PART_APOLLO4) && !defined(AM_PART_APOLLO4B))
    //
    // Set the clock frequency.
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);
#endif

#if !defined(APOLLO4_FPGA)
    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();
#endif

  //
    // Initialize the Extended (DSP0/1) RAM for usage.
    //
    am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP0, &DSPRAM_Cfg);
    am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP1, &DSPRAM_Cfg);

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Print the banner.
    //
    am_bsp_itm_printf_enable();
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo4 DSPRAM Write Test Cases. \n\n");

    if ( !APOLLO4_GE_B2 )
    {
        TEST_FAIL();
        am_util_stdio_printf("\nTEST FAIL: This test must be run on Apollo4 B2 or greater.\n");
    }

    //
    // Variable initialization
    //
    widgetConfig.MSPIConfig = MSPI_Flash_Config;

}

void
globalTearDown(void)
{
}

