//*****************************************************************************
//
//! @file ios_test_cases.c
//!
//! @brief Test program for testing IOS Widget
//!
//! This program initiates a SPI/I2C transaction at specified address/size,
//! For access to LRAM locations (address != 0x7F), IOM sends a pattern to IOS,
//! IOS receives it, manipulates the data and send it back to IOM for
//! verification.
//! For access to FIFO (address == 0x7F), a pattern is send from IOS to IOM,
//! which is then verified for accuracy.
//!
//! SWO is configured in 1M baud, 8-n-1 mode.
//!
//! The test utilizes an IOM<->IOS transfer within the same device (basically
//! a loopback configuration). Running the test requires jumpering pins in
//! order to connect the IOM to the IOS.
//!
//! @verbatim
//! The pin jumpers are as follows:
//! (these are BSP-dependent and apply to apollo4p_eb mapping in bsp_pins.src)
//!
//! SPI:
//! # (BSP_PIN)  to #  (BSP_PIN)    :   IOS                IOM0
//!----------------------------------------------------------------------------
//! 0 (IOS_SCK)  to 5  (IOM0_SCK)   :   IOS SPI SCK    to  IOM0 SPI SCK
//! 1 (IOS_MOSI) to 6  (IOM0_MOSI)  :   IOS SPI MOSI   to  IOM0 SPI MOSI
//! 2 (IOS_MISO) to 7  (IOM0_MISO)  :   IOS SPI MISO   to  IOM0 SPI MISO
//! 3 (IOS_CE)   to 12 (IOM0_CS)    :   IOS SPI nCE    to  IOM0 SPI nCE
//! 4 (IOS_INT)  to 53 (n/a*)       :   IOS Interrupt  to  Handshake Interrupt
//!
//! I2C:
//! # (BSP_PIN)  to #  (BSP_PIN)    :   IOS                IOM0
//!----------------------------------------------------------------------------
//! 0 (IOS_SCL)  to 5  (IOM0_SCL)   :   IOS I2C SCL    to  IOM0 I2C SCL
//! 1 (IOS_SDA)  to 6  (IOM0_SDA)   :   IOS I2C SDA    to  IOM0 I2C SDA
//! 4 (IOS_INT)  to 53 (n/a*)       :   IOS Interrupt  to  Handshake Interrupt
//!
//! *GPIO53 is assigned as HANDSHAKE_IOM_PIN in widgets/apollo4p/am_widget_ios.c
//! @endverbatim
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_widget_ios.h"

#define TEST_IOM_MODULE     0
#define TEST_IOS_MODULE     0
#define TEST_SPI
//#define TEST_I2C
//#define TEST_DIRECT_ACCESS
#define TEST_FIFO
//#define TEST_DIRECT_ACCESS_MAX_SIZE
//
// Map IOM Speed to String. Also used for speed iteration.
//
#ifdef TEST_SPI
static const struct
{
    const uint32_t MHz;
    const char *MHzString;
} spiSpeedString[] =
{
//#if !defined(APOLLO4_FPGA)
//    {AM_HAL_IOM_16MHZ,  "16MHZ"},
////    {AM_HAL_IOM_12MHZ,  "12MHZ"},
//#endif
//    {AM_HAL_IOM_8MHZ,   "8MHZ"},
//    {AM_HAL_IOM_6MHZ,   "6MHZ"},
//    {AM_HAL_IOM_4MHZ,   "4MHZ"},
//    {AM_HAL_IOM_3MHZ,   "3MHZ"},
//    {AM_HAL_IOM_2MHZ,   "2MHZ"},
//    {AM_HAL_IOM_1_5MHZ, "1.5MHZ"},
    {AM_HAL_IOM_1MHZ,   "1MHZ"},
//    {AM_HAL_IOM_750KHZ, "750KHZ"},
//    {AM_HAL_IOM_500KHZ, "500KHZ"},
//    {AM_HAL_IOM_400KHZ, "400KHZ"},
//    {AM_HAL_IOM_375KHZ, "375KHZ"},
//    {AM_HAL_IOM_250KHZ, "250KHZ"},
//    {AM_HAL_IOM_100KHZ, "100KHZ"}
//    {AM_HAL_IOM_50KHZ,  "50KHZ"},
//    {AM_HAL_IOM_10KHZ,  "10KHZ"}
};
#define IOM_SPEED_SPI_MAX       0  // 12 MHZ

#define IOM_SPEED_SPI_MIN       0 // 100 KHz
#endif

#ifdef TEST_I2C
static const struct
{
    const uint32_t MHz;
    const char *MHzString;
} i2cSpeedString[] =
{
    {AM_HAL_IOM_1MHZ,   "1MHZ"},
    {AM_HAL_IOM_400KHZ, "400KHZ"},
    {AM_HAL_IOM_100KHZ, "100KHZ"},
};
#define IOM_SPEED_I2C_MAX       0  // 1 MHz
#define IOM_SPEED_I2C_MIN       2  // 100 KHz
#endif

AM_SHARED_RW uint32_t        DMATCBBuffer[1024];

//*****************************************************************************
//
// SPI Slave Configuration
//
//*****************************************************************************
#ifdef TEST_SPI
static am_hal_ios_config_t g_sIOSSpiConfig =
{
    // Configure the IOS in SPI mode.
    .ui32InterfaceSelect = AM_HAL_IOS_USE_SPI,

    // Eliminate the "read-only" section, so an external host can use the
    // entire "direct write" section.
    .ui32ROBase = 0x78,

    // Making the "FIFO" section as big as possible.
    .ui32FIFOBase = 0x80,

    // We don't need any RAM space, so extend the FIFO all the way to the end
    // of the LRAM.
    .ui32RAMBase = 0x100,

    // FIFO Threshold - set to half the size
    .ui32FIFOThreshold = 0x40,
};

static am_hal_iom_config_t g_sIOMSpiConfig =
{
    .eInterfaceMode     = AM_HAL_IOM_SPI_MODE,
    .ui32ClockFreq      = AM_HAL_IOM_8MHZ,
    .eSpiMode           = AM_HAL_IOM_SPI_MODE_0,
    .pNBTxnBuf          = &DMATCBBuffer[0],
    .ui32NBTxnBufLength = sizeof(DMATCBBuffer) / 4
};

#define MAX_SPI_SIZE    128
#endif // 0

//*****************************************************************************
//
// I2C Slave Configuration
//
//*****************************************************************************
#ifdef TEST_I2C
am_hal_ios_config_t g_sIOSI2cConfig =
{
    // Configure the IOS in I2C mode.
    .ui32InterfaceSelect = AM_HAL_IOS_USE_I2C | AM_HAL_IOS_I2C_ADDRESS(0x20),

    // Eliminate the "read-only" section, so an external host can use the
    // entire "direct write" section.
    .ui32ROBase = 0x78,

    // Set the FIFO base to the maximum value, making the "direct write"
    // section as big as possible.
    .ui32FIFOBase = 0x80,

    // We don't need any RAM space, so extend the FIFO all the way to the end
    // of the LRAM.
    .ui32RAMBase = 0x100,
    // FIFO Threshold - set to half the size
    .ui32FIFOThreshold = 0x40,
};

static am_hal_iom_config_t g_sIOMI2cConfig =
{
    .eInterfaceMode     = AM_HAL_IOM_I2C_MODE,
    .ui32ClockFreq      = AM_HAL_IOM_100KHZ,
    .eSpiMode           = AM_HAL_IOM_SPI_MODE_0,
    .pNBTxnBuf          = &DMATCBBuffer[0],
    .ui32NBTxnBufLength = sizeof(DMATCBBuffer) / 4
};

#define MAX_I2C_SIZE   255
#endif // 0

void          *pWidget;
char          errStr[128];

const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_DEBUG =
{
    .GP.cfg_b.uFuncSel            = AM_HAL_PIN_23_GPIO,
};

extern uint32_t missingAccIsr[];

//*****************************************************************************
//
// Primary test case.
//
//*****************************************************************************
int iom_ios_loopback_test(uint32_t mode)
{
    am_widget_iosspi_config_t widgetCfg;
    am_widget_iosspi_test_t   testCfg;
    void *pWidget;
    uint32_t i;
//    uint32_t directSize, i, j;
    char errStr[200];
    bool bResult = false;
//    uint32_t totalTest = 0;
//    uint32_t numFail = 0;

    widgetCfg.stimulusCfgIom.iomModule = TEST_IOM_MODULE;
    widgetCfg.stimulusCfgIos.iosModule = TEST_IOS_MODULE;

    // SPI
#ifdef TEST_SPI
    widgetCfg.stimulusCfgIos.iosHalCfg = g_sIOSSpiConfig;
    widgetCfg.stimulusCfgIom.iomHalCfg = g_sIOMSpiConfig;

//    directSize = widgetCfg.stimulusCfgIos.iosHalCfg.ui32ROBase;

#ifdef TEST_DIRECT_ACCESS_MAX_SIZE
    //
    // Loop through all possible speeds.
    //
    for (int spd = IOM_SPEED_SPI_MAX; spd <= IOM_SPEED_SPI_MIN; spd++)
    {
        widgetCfg.stimulusCfgIom.iomHalCfg.ui32ClockFreq = spiSpeedString[spd].MHz;
        am_util_stdio_printf("\nSPI Speed %s ", spiSpeedString[spd].MHzString);
        bResult = am_widget_iosspi_setup(&widgetCfg, &pWidget, errStr);
        if (bResult)
        {
            am_util_stdio_printf("Setup Failed: %s\n", errStr);
            break;
        }
        // Test LRAM R/W
        for (i = 0; i < 10000; i++)
        {
            testCfg.address = 0;
            testCfg.size = 0x50;
            bResult = am_widget_iosspi_test(pWidget, &testCfg, errStr);
            if (bResult)
            {
//              am_util_stdio_printf("\naddress = %d:size = %d FAIL", i, j);
//              am_util_stdio_printf("%s\n", errStr);
                numFail++;
//              break;
            }
            totalTest++;
        }
        am_util_stdio_printf("\nTotal Test = %d:Num Fail = %d\n", totalTest, numFail);
        if (numFail)
        {
            am_util_stdio_printf("\nMissing Bits Counts\n");
            for (i = 0; i < 32; i++)
            {
                am_util_stdio_printf("%2d: %d\n", 31-i , missingAccIsr[i]);
                missingAccIsr[i] = 0;
            }
        }
    }
#endif
    //
    // Loop through all possible speeds.
    //
//    for (int spd = IOM_SPEED_SPI_MAX; spd <= IOM_SPEED_SPI_MIN; spd++)
    int spd = 0;
    {
        widgetCfg.stimulusCfgIom.iomHalCfg.ui32ClockFreq = spiSpeedString[spd].MHz;
        am_util_debug_printf("\nSPI Speed %s ", spiSpeedString[spd].MHzString);
        bResult = am_widget_iosspi_setup(&widgetCfg, &pWidget, errStr);
        if (bResult)
        {
            am_util_stdio_printf("Setup Failed: %s\n", errStr);
            goto _fail;
        }
#ifdef TEST_DIRECT_ACCESS
        // Test LRAM R/W
        am_util_stdio_printf("\nAddress Range %d - %d ", 0, directSize - 1);
//        for (i = 0; i < directSize - 1; i++) // Not going all the way to directSize as there seems to be some issue with hardware
        for (i = 0; i < directSize; i++) // Not going all the way to directSize as there seems to be some issue with hardware
        {
            for (j = 1; j < (directSize - i + 1); j++)
            {
                testCfg.address = i;
                testCfg.size = j;
                bResult = am_widget_iosspi_test(pWidget, &testCfg, errStr);
                if (bResult)
                {
                    am_util_stdio_printf("\naddress = %d:size = %d FAIL", i, j);
                    am_util_stdio_printf("%s\n", errStr);
                    numFail++;
    //                break;
                }
                totalTest++;
            }
            AM_UPDATE_TEST_PROGRESS();
            am_util_stdio_printf(".");
        }
        am_util_stdio_printf("\nTotal Test = %d:Num Fail = %d\n", totalTest, numFail);
        if (numFail)
        {
            am_util_stdio_printf("\nMissing Bits Counts\n");
            for (i = 0; i < 32; i++)
            {
                am_util_stdio_printf("%2d: %d\n", 31-i , missingAccIsr[i]);
                missingAccIsr[i] = 0;
            }
        }
#endif
#ifdef TEST_FIFO
        // Test Fifo
        am_util_debug_printf("\nTest FIFO Size Range %d - %d \n", 1, MAX_SPI_SIZE);
        for (i = 0x80; i <= MAX_SPI_SIZE; i++)
        {
            bool bResult = false;
            testCfg.address = 0x7F;
            testCfg.size = i;
            bResult = am_widget_iosspi_test(pWidget, &testCfg, errStr);
            if (bResult)
            {
                am_util_stdio_printf("\naddress = %d:size = %d FAIL\n", 0x7F, i);
                am_util_stdio_printf("%s\n", errStr);
                break;
            }
            AM_UPDATE_TEST_PROGRESS();
            am_util_debug_printf(".");
        }
        if (i < MAX_SPI_SIZE)
        {
          goto _fail;
        }
        return 0;
    _fail:
        return -1;
#endif
    }
#endif

#ifdef TEST_I2C
    // I2C
    widgetCfg.stimulusCfgIos.iosHalCfg = g_sIOSI2cConfig;
    widgetCfg.stimulusCfgIom.iomHalCfg = g_sIOMI2cConfig;

    directSize = widgetCfg.stimulusCfgIos.iosHalCfg.ui32ROBase;

    //
    // Loop through all possible speeds.
    //
    for (int spd = IOM_SPEED_I2C_MAX; spd <= IOM_SPEED_I2C_MIN; spd++)
    {
        widgetCfg.stimulusCfgIom.iomHalCfg.ui32ClockFreq = i2cSpeedString[spd].MHz;
        am_util_stdio_printf("\nI2C Speed %s ", i2cSpeedString[spd].MHzString);
        bResult = am_widget_iosspi_setup(&widgetCfg, &pWidget, errStr);
        if (bResult)
        {
            am_util_stdio_printf("Setup Failed: %s\n", errStr);
            break;
        }
#ifdef TEST_DIRECT_ACCESS
        // Test LRAM R/W
        am_util_stdio_printf("Address Range %d - %d\n", 0, directSize - 1);
        for (i = 0; i < directSize; i++)
        {
            for (j = 1; j < (directSize - i + 1); j++)
            {
                testCfg.address = i;
                testCfg.size = j;
                bResult = am_widget_iosspi_test(pWidget, &testCfg, errStr);
                if (bResult)
                {
                    am_util_stdio_printf("\naddress = %d:size = %d FAIL ", i, j);
                    am_util_stdio_printf("%s\n", errStr);
                    numFail++;
    //                break;
                }
                totalTest++;
            }
            AM_UPDATE_TEST_PROGRESS();
            am_util_stdio_printf(".");
        }
        am_util_stdio_printf("\nTotal Test = %d:Num Fail = %d\n", totalTest, numFail);
        if (numFail)
        {
            am_util_stdio_printf("\nMissing Bits Counts\n");
            for (i = 0; i < 32; i++)
            {
                am_util_stdio_printf("%2d: %d\n", 31-i , missingAccIsr[i]);
                missingAccIsr[i] = 0;
            }
        }
#endif
#ifdef TEST_FIFO
        // Test Fifo
        am_util_stdio_printf("\nTest FIFO Size Range %d - %d \n", 1, MAX_I2C_SIZE);
        for (i = 1; i <= MAX_I2C_SIZE; i++)
        {
            bool bResult = false;
            testCfg.address = 0x7F;
            testCfg.size = i;
            bResult = am_widget_iosspi_test(pWidget, &testCfg, errStr);
            if (bResult)
            {
                am_util_stdio_printf("\naddress = %d:size = %d FAIL\n", 0x7F, i);
                am_util_stdio_printf("%s\n", errStr);
    //            break;
            }
            AM_UPDATE_TEST_PROGRESS();
            am_util_stdio_printf(".");
        }
#endif
    }
#endif
}
