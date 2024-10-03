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

AM_SHARED_RW uint32_t DMATCBBuffer[4096];

uint8_t         TestBuffer[2048];
uint8_t         DummyBuffer[1024];

mspi_speed_t mspi_test_speeds[] =
{
  { AM_HAL_MSPI_CLK_250MHZ,     " 250MHz" },
  { AM_HAL_MSPI_CLK_192MHZ,     " 192MHz" },
  { AM_HAL_MSPI_CLK_125MHZ,     " 125MHz" },
  { AM_HAL_MSPI_CLK_96MHZ,      " 96MHz" },

  { AM_HAL_MSPI_CLK_62P5MHZ,    " 62.5MHz"},
  { AM_HAL_MSPI_CLK_48MHZ,      " 48MHz" },

  { AM_HAL_MSPI_CLK_31P25MHZ,   " 31.25MHz" },
  { AM_HAL_MSPI_CLK_24MHZ,      " 24MHz" },

  { AM_HAL_MSPI_CLK_20P83MHZ,   " 20.83MHz"},
  { AM_HAL_MSPI_CLK_16MHZ,      " 16MHz" },

  { AM_HAL_MSPI_CLK_15P63MHZ,   " 15.63MHz"},
  { AM_HAL_MSPI_CLK_12MHZ,      " 12MHz" },

  { AM_HAL_MSPI_CLK_10P42MHZ,   " 10.42MHz"},
  { AM_HAL_MSPI_CLK_8MHZ,       "  8MHz" },

  { AM_HAL_MSPI_CLK_7P81MHZ,    "  7.81MHz"},
  { AM_HAL_MSPI_CLK_6MHZ,       "  6MHz" },

  { AM_HAL_MSPI_CLK_5P21MHZ,    "  5.21MHz"},
  { AM_HAL_MSPI_CLK_4MHZ,       "  4MHz" },

  { AM_HAL_MSPI_CLK_3P91MHZ,    "  3.91MHz"},
  { AM_HAL_MSPI_CLK_3MHZ,       "  3MHz" },
};

void          *pWidget[AM_REG_MSPI_NUM_MODULES];
char          errStr[128];

#if (defined(MICRON_N25Q256A) || defined(MACRONIX_MX25U12835F) || defined(CYPRESS_S25FS064S) ||\
     defined(APS6404L) || defined(MB85RS64V) || defined(DOSILICON_DS35X1GA) || defined(ISSI_IS25WX064) ||\
     defined(TOSHIBA_TC58CYG0) || defined(WINBOND_W25N02KW))
am_widget_mspi_devices_config_t MSPI_Flash_Config =
{
    .eDeviceConfig = AM_HAL_MSPI_FLASH_SERIAL_CE0,
    .eClockFreq = AM_HAL_MSPI_CLK_16MHZ,
    .pNBTxnBuf = DMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(DMATCBBuffer)/sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
};
#elif defined(ADESTO_ATXP032)
am_widget_mspi_devices_config_t MSPI_Flash_Config =
{
    .eDeviceConfig = AM_HAL_MSPI_FLASH_SERIAL_CE0,
    .eClockFreq = AM_HAL_MSPI_CLK_16MHZ,
    .pNBTxnBuf = DMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(DMATCBBuffer)/sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
};
#elif defined(MACRONIX_MX25UM51245G)
am_widget_mspi_devices_config_t MSPI_Flash_Config =
{
    .ui32DeviceNum = 0,
    .eDeviceConfig = AM_HAL_MSPI_FLASH_SERIAL_CE0,
    .eClockFreq = AM_HAL_MSPI_CLK_24MHZ,
    .bDQSEnable = false,
    .pNBTxnBuf = DMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(DMATCBBuffer) / sizeof(uint32_t)),
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
#elif defined(APS25616N)
am_widget_mspi_devices_config_t MSPI_Flash_Config =
{
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,
    .eClockFreq = AM_HAL_MSPI_CLK_48MHZ,
    .pNBTxnBuf = DMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(DMATCBBuffer)/sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
};
#elif defined(APS25616BA)
am_widget_mspi_devices_config_t MSPI_Flash_Config =
{
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,
    .eClockFreq = AM_HAL_MSPI_CLK_48MHZ,
    .pNBTxnBuf = DMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(DMATCBBuffer)/sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
};
#elif defined(W958D6NW)
am_widget_mspi_devices_config_t MSPI_Flash_Config =
{
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,
    .eClockFreq = AM_HAL_MSPI_CLK_48MHZ,
    .pNBTxnBuf = DMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(DMATCBBuffer)/sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
};
#elif defined(AMBT52)
am_widget_mspi_devices_config_t MSPI_Flash_Config =
{
    .eDeviceConfig = AM_HAL_MSPI_FLASH_QUAD_CE1,
    .eClockFreq = AM_HAL_MSPI_CLK_12MHZ, //TODO/4 div for FPGA
    .pNBTxnBuf = DMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(DMATCBBuffer)/sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
    .ui16ReadInstr = AM_DEVICES_MSPI_AMBT52_READ_UL,
    .ui16WriteInstr = AM_DEVICES_MSPI_AMBT52_WRITE_UL,
    .bChangeInstr = false,
};
#endif


mspi_access_str_t mspi_test_access[] =
{
  { AM_WIDGET_MSPI_PIO_ACCESS,          " PIO W/R PSRAM" },
  { AM_WIDGET_MSPI_DMA_ACCESS,          " DMA W/R PSRAM" },
  { AM_WIDGET_MSPI_XIPMM_ACCESS_32B,    " XIPMM Word W/R PSRAM" },
  { AM_WIDGET_MSPI_XIPMM_ACCESS_16B,    " XIPMM Short W/R PSRAM" },
  { AM_WIDGET_MSPI_XIPMM_ACCESS_8B,     " XIPMM Byte W/R PSRAM" },
  { AM_WIDGET_MSPI_MEMCPY_XIPMM_ACCESS, " XIPMM memcpy W/R PSRAM" },
};

mspi_device_str_t mspi_test_devices[] =
{
  { AM_WIDGET_MSPI_FLASH_ATXP032,        " ATXP032" },
  { AM_WIDGET_MSPI_FLASH_IS25WX064,      " IS25WX064" },
  { AM_WIDGET_MSPI_FLASH_MX25UM51245G,   " MX25UM51245G" },
  { AM_WIDGET_MSPI_PSRAM_APS6404L,       " APS6404L" },
  { AM_WIDGET_MSPI_PSRAM_APS6408L,       " APS6408L" },
  { AM_WIDGET_MSPI_DDR_PSRAM_APS12808L,  " APS12808L" },
  { AM_WIDGET_MSPI_DDR_PSRAM_APS25616N,  " APS25616N" },
  { AM_WIDGET_MSPI_DDR_PSRAM_APS25616BA, " APS25616BA" },
  { AM_WIDGET_MSPI_DDR_PSRAM_W958D6NW,   " W958D6NW"},
};

mspi_device_mode_str_t mspi_test_device_modes[] =
{
  { AM_HAL_MSPI_FLASH_SERIAL_CE0,       " SERIAL CE0" },
  { AM_HAL_MSPI_FLASH_SERIAL_CE1,       " SERIAL CE1" },
  { AM_HAL_MSPI_FLASH_DUAL_CE0,         " DUAL CE0" },
  { AM_HAL_MSPI_FLASH_DUAL_CE1,         " DUAL CE1" },
  { AM_HAL_MSPI_FLASH_QUAD_CE0,         " QUAD CE0" },
  { AM_HAL_MSPI_FLASH_QUAD_CE1,         " QUAD CE1" },
  { AM_HAL_MSPI_FLASH_OCTAL_CE0,        " OCTAL CE0" },
  { AM_HAL_MSPI_FLASH_OCTAL_CE1,        " OCTAL CE1" },
  { AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,    " OCTAL DDR CE0" },
  { AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1,    " OCTAL DDR CE1" },
  { AM_HAL_MSPI_FLASH_HEX_DDR_CE0,      " HEX DDR CE0" },
  { AM_HAL_MSPI_FLASH_HEX_DDR_CE1,      " HEX DDR CE1" },
  { AM_HAL_MSPI_FLASH_DUAL_CE0_1_1_2,   " DUAL_CE0_1_1_2"},
  { AM_HAL_MSPI_FLASH_DUAL_CE1_1_1_2,   " DUAL_CE1_1_1_2"},
  { AM_HAL_MSPI_FLASH_DUAL_CE0_1_2_2,   " DUAL_CE0_1_2_2"},
  { AM_HAL_MSPI_FLASH_DUAL_CE1_1_2_2,   " DUAL_CE1_1_2_2"},
  { AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4,   " QUAD_CE0_1_1_4"},
  { AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4,   " QUAD_CE1_1_1_4"},
  { AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4,   " QUAD_CE0_1_4_4"},
  { AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4,   " QUAD_CE1_1_4_4"},
  { AM_HAL_MSPI_FLASH_SERIAL_CE0_3WIRE, " SERIAL_CE0_3WIRE"},
  { AM_HAL_MSPI_FLASH_SERIAL_CE1_3WIRE, " SERIAL_CE1_3WIRE"},
  { AM_HAL_MSPI_FLASH_OCTAL_CE0_1_1_8,  " OCTAL_CE0_1_1_8"},
  { AM_HAL_MSPI_FLASH_OCTAL_CE1_1_1_8,  " OCTAL_CE1_1_1_8"},
  { AM_HAL_MSPI_FLASH_OCTAL_CE0_1_8_8,  " OCTAL_CE0_1_8_8"},
  { AM_HAL_MSPI_FLASH_OCTAL_CE1_1_8_8,  " OCTAL_CE1_1_8_8"},
} ;

am_hal_mpu_region_config_t sMPUCfg =
{
    .ui32RegionNumber = 6,
    .ui32BaseAddress = (uint32_t)DMATCBBuffer,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)DMATCBBuffer + sizeof(DMATCBBuffer) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
};
am_hal_mpu_attr_t sMPUAttr =
{
    .ui8AttrIndex = 0,
    .bNormalMem = true,
    .sOuterAttr = {
                    .bNonTransient = false,
                    .bWriteBack = true,
                    .bReadAllocate = false,
                    .bWriteAllocate = false
                  },
    .sInnerAttr = {
                    .bNonTransient = false,
                    .bWriteBack = true,
                    .bReadAllocate = false,
                    .bWriteAllocate = false
                  },
    .eDeviceAttr = 0,
} ;

#ifdef COLLECT_BANDWIDTH
volatile uint32_t g_ui32TimerCount = 0;

//*****************************************************************************
//
// Init function for Timer A0.  per DaveC
//
//*****************************************************************************
void
stimer_init(void)
{
    //
    // Enable compare A interrupt in STIMER
    //
    am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREA);

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_SetPriority(STIMER_CMPR0_IRQn, 3);
    NVIC_EnableIRQ(STIMER_CMPR0_IRQn);

    //
    // Configure the STIMER and run
    //
    am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
    am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);
    am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ |
                         AM_HAL_STIMER_CFG_COMPARE_A_ENABLE);

}

//*****************************************************************************
//
// Timer Interrupt Service Routine (ISR)  also per DaveC
//
//*****************************************************************************
void
am_stimer_cmpr0_isr(void)
{
    //
    // Check the timer interrupt status.
    //
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREA);
    am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);
#if defined APOLLO5_FPGA
    am_hal_gpio_output_toggle(15);
#endif
    g_ui32TimerCount++;
}

#endif

void
ctimer_init(void)
{
    am_hal_timer_config_t       TimerConfig;

    //
    // Set up the default configuration.
    //
    am_hal_timer_default_config_set(&TimerConfig);

    //
    // Update the clock in the config.
    //
    TimerConfig.eInputClock             = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    TimerConfig.eFunction               = AM_HAL_TIMER_FN_UPCOUNT;

    //
    // Configure the TIMER.
    //
    if(AM_HAL_STATUS_SUCCESS != am_hal_timer_config(TIMER_NUM, &TimerConfig))
    {
        am_util_stdio_printf("Fail to config timer%d\n", TIMER_NUM);
    }

    //
    // Clear the TIMER.
    //
    if(AM_HAL_STATUS_SUCCESS != am_hal_timer_clear_stop(TIMER_NUM))
    {
        am_util_stdio_printf("Fail to clear&stop timer%d\n", TIMER_NUM);
    }

    //
    // Enable the TIMER.
    //
    if(AM_HAL_STATUS_SUCCESS != am_hal_timer_enable(TIMER_NUM))
    {
        am_util_stdio_printf("Fail to enable timer%d\n", TIMER_NUM);
    }

    //
    // Start the TIMER.
    //
    if(AM_HAL_STATUS_SUCCESS !=  am_hal_timer_start(TIMER_NUM))
    {
        am_util_stdio_printf("Fail to start timer%d\n", TIMER_NUM);
    }
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
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);
    //
    // Set up the attributes.
    //
    am_hal_mpu_attr_configure(&sMPUAttr, 1);
    //
    // Clear the MPU regions.
    //
    am_hal_mpu_region_clear();
    //
    // Set up the regions.
    //
    am_hal_mpu_region_configure(&sMPUCfg, 1);
    //
    // Invalidate and clear DCACHE, this is required by CM55 TRF.
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    //
    // MPU enable
    //
    am_hal_mpu_enable(true, true);

    ctimer_init();

#ifdef CPU_HP
    if ( am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) != AM_HAL_STATUS_SUCCESS )
    {
#ifndef APOLLO5_FPGA
        while(1);
#endif
    }
#endif

    //
    // Print the banner.
    //
    am_bsp_itm_printf_enable();
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo5B MSPI Test Cases\n\n");

    am_util_id_t sIdDevice;
    am_util_id_device(&sIdDevice);
    am_util_stdio_printf("Vendor Name: %s\n", sIdDevice.pui8VendorName);
    am_util_stdio_printf("Device type: %s\n", sIdDevice.pui8DeviceName);
    am_util_stdio_printf("Device Info:\n"
                            "\tPart number: 0x%08X\n"
                            "\tChip ID:    0x%08X,0x%08X\n"
                            "\tRevision:    0x%08X (Rev%c%c)\n",
                            sIdDevice.sMcuCtrlDevice.ui32ChipPN,
                            sIdDevice.sMcuCtrlDevice.ui32ChipID0,
                            sIdDevice.sMcuCtrlDevice.ui32ChipID1,
                            sIdDevice.sMcuCtrlDevice.ui32ChipRev,
                            sIdDevice.ui8ChipRevMaj, sIdDevice.ui8ChipRevMin );
    am_util_stdio_printf("App Compiler:    %s\n", COMPILER_VERSION);
    am_util_stdio_printf("HAL Compiler:    %s\n", g_ui8HALcompiler);
    am_util_stdio_printf("HAL SDK version: %d.%d.%d\n",
                         g_ui32HALversion.s.Major,
                         g_ui32HALversion.s.Minor,
                         g_ui32HALversion.s.Revision);
    am_util_stdio_printf("HAL compiled with %s-style registers\n",
                         g_ui32HALversion.s.bAMREGS ? "AM_REG" : "CMSIS");

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
#if defined (COLLECT_BANDWIDTH)
    am_widget_mspi_global_teardown();
#endif
}
