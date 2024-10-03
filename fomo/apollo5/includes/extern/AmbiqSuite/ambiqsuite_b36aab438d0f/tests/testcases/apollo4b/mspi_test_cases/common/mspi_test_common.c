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

#include "mspi_test_common.h"
#include "am_devices_fireball.h"

#define DEBUG_PRINT        am_util_stdio_printf

AM_SHARED_RW uint32_t DMATCBBuffer[4096] AM_BIT_ALIGNED(128);
AM_SHARED_RW uint32_t axiScratchBuf[20];

uint8_t         TestBuffer[2048];
uint8_t         DummyBuffer[1024];

mspi_speed_t mspi_test_speeds[] =
{
  { AM_HAL_MSPI_CLK_96MHZ,      " 96MHz" },
  { AM_HAL_MSPI_CLK_48MHZ,      " 48MHz" },
  { AM_HAL_MSPI_CLK_24MHZ,      " 24MHz" },
  { AM_HAL_MSPI_CLK_16MHZ,      " 16MHz" },
  { AM_HAL_MSPI_CLK_12MHZ,      " 12MHz" },
  { AM_HAL_MSPI_CLK_8MHZ,       "  8MHz" },
  { AM_HAL_MSPI_CLK_6MHZ,       "  6MHz" },
  { AM_HAL_MSPI_CLK_4MHZ,       "  4MHz" },
  { AM_HAL_MSPI_CLK_3MHZ,       "  3MHz" },
};

#if defined(APOLLO4_FPGA)
uint32_t    mspi_instances[1] = {0};
#else
uint32_t    mspi_instances[2] = {0, 2};
#endif

void          *pWidget[AM_REG_MSPI_NUM_MODULES];
char          errStr[128];

#if (defined(MICRON_N25Q256A) || defined(MACRONIX_MX25U12835F) || defined(ADESTO_ATXP032) || defined(CYPRESS_S25FS064S) ||\
     defined(APS6404L) || defined(MB85RS64V))
am_widget_mspi_devices_config_t MSPI_Flash_Config =
{
    .eDeviceConfig = AM_HAL_MSPI_FLASH_SERIAL_CE0,
    .eClockFreq = AM_HAL_MSPI_CLK_16MHZ,
    .pNBTxnBuf = DMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(DMATCBBuffer)/sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
};
#elif defined(APS12808L)
am_widget_mspi_devices_config_t MSPI_Flash_Config =
{
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,
    .eClockFreq = AM_HAL_MSPI_CLK_48MHZ,
    .pNBTxnBuf = DMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(DMATCBBuffer)/sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
};
#elif defined(AMBT53)
am_widget_mspi_devices_config_t MSPI_Flash_Config =
{
    .eDeviceConfig = AM_HAL_MSPI_FLASH_QUAD_CE1,
    .eClockFreq = AMBT53_MSPI_FREQ,
    .pNBTxnBuf = DMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(DMATCBBuffer)/sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
    .ui16ReadInstr = AM_DEVICES_MSPI_AMBT53_READ_UL,
    .ui16WriteInstr = AM_DEVICES_MSPI_AMBT53_WRITE_UL,
    .bChangeInstr = false,
};
#endif

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
  // Set up scratch AXI buf (needs 64B - aligned to 16 Bytes)
  //
  am_hal_daxi_control(AM_HAL_DAXI_CONTROL_AXIMEM, (uint8_t *)((uint32_t)(axiScratchBuf + 3) & ~0xF));
  //
  // Configure the board for low power operation.
  //
  am_bsp_low_power_init();

  //
  // Print the banner.
  //
  am_bsp_itm_printf_enable();
  am_util_stdio_terminal_clear();
  am_util_stdio_printf("Apollo4 MSPI Test Cases\n\n");

#if FIREBALL_CARD
    //
    // Set the MUX for the Flash Device
    //
    uint32_t ui32Ret, ui32ID;
    bool bFireball2 = false;
    am_devices_fireball_control_e fc;

#if 1
    //
    // Get Fireball ID and Rev info.
    //
    ui32Ret = am_devices_fireball_control(AM_DEVICES_FIREBALL_STATE_ID_GET, &ui32ID);
    if ( ui32Ret != 0 )
    {
        DEBUG_PRINT("FAIL: am_devices_fireball_control(%d) returned 0x%X.\n",
                             AM_DEVICES_FIREBALL_STATE_ID_GET, ui32Ret);
        return;
    }
    else if ( ui32ID == FIREBALL_ID )
    {
        DEBUG_PRINT("Fireball found, ID is 0x%X.\n", ui32ID);
    }
    else if ( ui32ID == FIREBALL2_ID )
    {
        bFireball2 = true;
        DEBUG_PRINT("Fireball2 found, ID is 0x%X.\n", ui32ID);
    }
    else
    {
        DEBUG_PRINT("Unknown device returned ID as 0x%X.\n", ui32ID);
    }

    ui32Ret = am_devices_fireball_control(AM_DEVICES_FIREBALL_STATE_VER_GET, &ui32ID);
    if ( ui32Ret != 0 )
    {
        DEBUG_PRINT("FAIL: am_devices_fireball_control(%d) returned 0x%X.\n",
                             AM_DEVICES_FIREBALL_STATE_VER_GET, ui32Ret);
        return;
    }
    else
    {
        DEBUG_PRINT("Fireball Version is 0x%X.\n", ui32ID);
    }
#endif

#if defined (ADESTO_ATXP032)
    bFireball2 = bFireball2; // fix the warning variable set but not used
    fc = AM_DEVICES_FIREBALL_STATE_OCTAL_FLASH_CE0;
#else
    if (bFireball2)
    {
        fc = AM_DEVICES_FIREBALL2_STATE_MSPI_FRAM_PSRAM_FLASH_1P8; //AM_DEVICES_FIREBALL2_STATE_MSPI_PSRAM_FLASH_3P3;
    }
    else
    {
        fc = AM_DEVICES_FIREBALL_STATE_TWIN_QUAD_CE0_CE1;
    }
#endif
    ui32Ret = am_devices_fireball_control(fc, 0);
    if ( ui32Ret != 0)
    {
        DEBUG_PRINT("FAIL: am_devices_fireball_control(%d) returned 0x%X.\n",
                             fc, ui32Ret);
        return;
    }
#endif // FIREBALL_CARD
}

void
globalTearDown(void)
{
}

