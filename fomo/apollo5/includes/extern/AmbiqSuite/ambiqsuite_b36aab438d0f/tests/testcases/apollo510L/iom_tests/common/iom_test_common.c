//*****************************************************************************
//
//! @file iom_test_common.c
//!
//! @brief IOM test cases common definitions and functions.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "iom_test_common.h"

#if 0
AM_SHARED_RW    uint32_t DMATCBBuffer[1024];    // DMA TCB buffer at SSRAM
#else
uint32_t DMATCBBuffer[1024];    // DMA TCB buffer at TCM
#endif

char            errStr[256];
bool            g_bMaxSpeed48MHz = true;


AMTEST_IOM_FRAM_STATUS_E g_bFramExpected[AM_REG_IOM_NUM_MODULES];

iom_speed_t iom_test_speeds[] =
{
    {AM_HAL_IOM_48MHZ,  "48M HZ"},
    {AM_HAL_IOM_24MHZ,  "24M HZ"},
    {AM_HAL_IOM_16MHZ,  "16M HZ"},
    {AM_HAL_IOM_12MHZ,  "12M HZ"},
    {AM_HAL_IOM_8MHZ,   "8M HZ"},
    {AM_HAL_IOM_6MHZ,   "6M HZ"},
    {AM_HAL_IOM_4MHZ,   "4M HZ"},
    {AM_HAL_IOM_3MHZ,   "3M HZ"},
    {AM_HAL_IOM_2MHZ,   "2M HZ"},
    {AM_HAL_IOM_1_5MHZ, "1.5M HZ"},
    {AM_HAL_IOM_1MHZ,   "1M HZ"},
    {AM_HAL_IOM_750KHZ, "750K HZ"},
    {AM_HAL_IOM_500KHZ, "500K HZ"},
    {AM_HAL_IOM_400KHZ, "400K HZ"},
    {AM_HAL_IOM_375KHZ, "375K HZ"},
    {AM_HAL_IOM_250KHZ, "250K HZ"},
    {AM_HAL_IOM_125KHZ, "125K HZ"},
    {AM_HAL_IOM_100KHZ, "100K HZ"},
    {AM_HAL_IOM_50KHZ,  "50K HZ"},
    {AM_HAL_IOM_10KHZ,  "10K HZ"},
};

uint32_t IOMSPISPeedsDefault[] =
{

//    AM_HAL_IOM_12MHZ,
//    AM_HAL_IOM_16MHZ,
    AM_HAL_IOM_24MHZ,
//    AM_HAL_IOM_48MHZ
};
uint32_t IOMSPISPeedsFull[] =
{
    AM_HAL_IOM_500KHZ,
    AM_HAL_IOM_750KHZ,

    AM_HAL_IOM_1_5MHZ,
    AM_HAL_IOM_2MHZ,
    AM_HAL_IOM_3MHZ,
    AM_HAL_IOM_4MHZ,
    AM_HAL_IOM_6MHZ,
    AM_HAL_IOM_8MHZ,
    AM_HAL_IOM_12MHZ,
    AM_HAL_IOM_16MHZ,
    AM_HAL_IOM_24MHZ,
    AM_HAL_IOM_48MHZ
};

uint32_t IOMI2CSPeedsDefault[] =
{
    AM_HAL_IOM_100KHZ,
    AM_HAL_IOM_400KHZ,
    AM_HAL_IOM_1MHZ,
};

const am_widget_iom_devices_config_t DefaultI2CConfig =
{
    .ui32ClockFreq        = AM_HAL_IOM_100KHZ,
    .pNBTxnBuf            = NULL,
    .ui32NBTxnBufLength   = 0,
};

const am_widget_iom_devices_config_t DefaultSPIConfig =
{
//    .ui32ClockFreq        = AM_HAL_IOM_96MHZ,
//    .ui32ClockFreq        = AM_HAL_IOM_48MHZ,
//    .ui32ClockFreq        = AM_HAL_IOM_24MHZ,
//    .ui32ClockFreq        = AM_HAL_IOM_16MHZ,
//    .ui32ClockFreq        = AM_HAL_IOM_12MHZ,
    .ui32ClockFreq        = AM_HAL_IOM_8MHZ,
//    .ui32ClockFreq        = AM_HAL_IOM_6MHZ,
    .pNBTxnBuf            = NULL,
    .ui32NBTxnBufLength   = 0,
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

void
iom_test_globalSetUp(void)
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

#ifdef MCU_VALIDATION_DEBUG_REG2
    // MCU_VALIDATION_DEBUG_REG2 contains a bitmask indicating which IOMs to expect FRAM on
    // Wait for the input to be non-zero
    while(AM_REGVAL(MCU_VALIDATION_DEBUG_REG2) == 0)
    {
        am_util_delay_ms(10);
    }
    uint32_t input = AM_REGVAL(MCU_VALIDATION_DEBUG_REG2);
    for (uint32_t i = 0; i < AM_REG_IOM_NUM_MODULES; i++)
    {
        g_bFramExpected[i] = ((input >> i) & 0x1) ? AMTEST_IOM_FRAM_STATUS_PRESENT : AMTEST_IOM_FRAM_STATUS_NOT_PRESENT;
    }
        if (input & 0x40)
        {
            g_bMaxSpeed48MHz = true;
        }
        else
        {
            g_bMaxSpeed48MHz = false;
        }
#else
    for (uint32_t i = 0; i < AM_REG_IOM_NUM_MODULES; i++)
    {
        g_bFramExpected[i] = AMTEST_IOM_FRAM_STATUS_NOT_KNOWN;
    }
#endif
}

void
iom_test_globalTearDown(void)
{
}

//
//! Convert IOM speed setting to string.
//! @note Caller doesn't need to process an error (Unknown).
//
const char *getSpeedString(int speedIndex)
{
    for ( int i = 0; i < SPEED_STRING_SIZE; i++ )
    {
        if (speedIndex == iom_test_speeds[i].speed)
        {
            return iom_test_speeds[i].MHzString;
        }
    }

    //return NULL;
    return "Unknown";
}

AMTEST_IOM_FRAM_STATUS_E isFramExpected(uint32_t ui32Module)
{
    return g_bFramExpected[ui32Module];
}


// ****************************************************************************
//
// Setup Fireball for testing.
//
// ui32FBState must be appropriate for the Fireball or Fireball2 connected.
//
// ****************************************************************************
#ifdef FIREBALL_CARD
//
// The Fireball device card multiplexes various devices including each of an SPI
// and I2C FRAM. The Fireball device driver controls access to these devices.
// If the Fireball card is not used, FRAM devices can be connected directly
// to appropriate GPIO pins.
//
#include "am_devices_fireball.h"
#endif // FIREBALL_CARD

uint32_t
SetFireball(am_devices_fireball_control_e eFBState)
{
#if FIREBALL_CARD

    uint32_t ui32FireballID, ui32FireballVer, ui32RetVal;

    am_devices_fireball_control(AM_DEVICES_FIREBALL_STATE_ID_GET,  &ui32FireballID);
    am_devices_fireball_control(AM_DEVICES_FIREBALL_STATE_VER_GET, &ui32FireballVer);

    if(ui32FireballID == FIREBALL_ID || ui32FireballID == FIREBALL2_ID || ui32FireballID == FIREBALL3_ID)
    {
        am_util_stdio_printf("\nFireball found, version=%d.\n"
                             "Setting Fireball state %d, ", ui32FireballVer, eFBState);
        //
        // Set the Fireball mux.
        //
        ui32RetVal = am_devices_fireball_control(eFBState, 0);
        if ( ui32RetVal )
        {
            am_util_stdio_printf(".. Failed, ret=%d.\n", ui32RetVal);
            return ui32FireballID;
        }
        else
        {
            am_util_stdio_printf(".. success.\n", ui32RetVal);
            return 0x0;
        }
    }
    else
    {
        am_util_stdio_printf("\nWarning: No Fireball found! (ID=0x%08X, Ver=0x%08X)\n", ui32FireballID, ui32FireballVer);
        return 0xFF;
    }
#else
    return 0x0;
#endif
}

uint32_t init_fireball(uint32_t ui32IOMNum)
{
#ifdef FIREBALL_CARD
    //
    // Set the MUX for the Flash Device
    //
    uint32_t ui32Ret[6], ui32FBgen, ui32FBid, ui32FBver;

    //
    // Get Fireball ID and Rev info.
    //
    ui32Ret[0] = am_devices_fireball_control(AM_DEVICES_FIREBALL_STATE_FBGEN_GET, &ui32FBgen);
    ui32Ret[1] = am_devices_fireball_control(AM_DEVICES_FIREBALL_STATE_ID_GET, &ui32FBid);
    ui32Ret[2] = am_devices_fireball_control(AM_DEVICES_FIREBALL_STATE_VER_GET, &ui32FBver);
    if ( ui32Ret[0]  ||  ui32Ret[1]  ||  ui32Ret[2] )
    {
        am_util_stdio_printf("FAIL: am_devices_fireball_control(%X) returned %d.\n"
                             "      am_devices_fireball_control(%X) returned %d.\n",
                             "      am_devices_fireball_control(%X) returned %d.\n",
                             AM_DEVICES_FIREBALL_STATE_FBGEN_GET, ui32Ret[0],
                             AM_DEVICES_FIREBALL_STATE_ID_GET,    ui32Ret[1],
                             AM_DEVICES_FIREBALL_STATE_VER_GET,   ui32Ret[2]);
        return -1;
    }

    if ( ui32FBgen < 10 )   // The value 10 is arbitrary
    {
        am_util_stdio_printf("Fireball%d (fwver=%d) found, ID is 0x%X.\n",
                             ui32FBgen, ui32FBver, ui32FBid);
        if ( ui32FBgen < 3 )
        {
            am_util_stdio_printf("ERROR!  This example requires Fireball3.\n");
            am_util_stdio_printf("        Exiting example.\n");
            return -1;
        }
    }
    else
    {
        am_util_stdio_printf("Unknown Fireball device returned ID as 0x%X.\n", ui32FBid);
        return -1;
    }

    am_util_stdio_printf("Setting the FB3 mux to the IOM I2C FRAM...\n\n");

    //
    // Set the Fireball3 MUX to the MSPI0 flash.
    //
    ui32Ret[0] = am_devices_fireball_control((am_devices_fireball_control_e)(AM_DEVICES_FIREBALL3_STATE_I2C_IOM0 + ui32IOMNum), 0);
    if ( ui32Ret[0] )
    {
        am_util_stdio_printf("FAIL: am_devices_fireball_control(%d) returned 0x%X.\n",
                             (am_devices_fireball_control_e)(AM_DEVICES_FIREBALL3_STATE_I2C_IOM0 + ui32IOMNum),
                             ui32Ret[0]);
        return -1;
    }
#endif // FIREBALL_CARD

    return 0;
}