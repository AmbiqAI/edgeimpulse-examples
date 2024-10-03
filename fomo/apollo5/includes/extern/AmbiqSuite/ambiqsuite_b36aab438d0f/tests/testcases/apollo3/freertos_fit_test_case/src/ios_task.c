//*****************************************************************************
//
//! @file ios_task.c
//!
//! @brief Test program for testing IOS task
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
//! SPI:
//! Pins    :   IOS                         IOM1
//! 0 to 8  :   IOS SPI SCK             to  IOM1 SPI SCK
//! 1 to 10 :   IOS SPI MOSI            to  IOM1 SPI MOSI
//! 2 to 9  :   IOS SPI MISO            to  IOM1 SPI MISO
//! 3 to 14 :   IOS SPI nCE             to  IOM1 SPI nCE
//! 4 to 38 :   IOS Interrupt           to  Handshake Interrupt
//!
//! I2C:
//! 0 to 8  :   IOS I2C SCL             to  IOM1 I2C SCL
//! 1 to 9  :   IOS I2C SDA             to  IOM1 I2C SDA
//! 4 to 38 :   IOS Interrupt           to  Handshake Interrupt
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

#include "freertos_fit_test_cases.h"
#include "am_widget_ios.h"

#define TEST_IOM_MODULE     1
#define TEST_IOS_MODULE     0
#define TEST_SPI
//#define TEST_I2C
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
//    {AM_HAL_IOM_12MHZ,  "12MHZ"},
//    {AM_HAL_IOM_8MHZ,   "8MHZ"},
//    {AM_HAL_IOM_6MHZ,   "6MHZ"},
//    {AM_HAL_IOM_4MHZ,   "4MHZ"},
//    {AM_HAL_IOM_3MHZ,   "3MHZ"},
    {AM_HAL_IOM_2MHZ,   "2MHZ"},
    {AM_HAL_IOM_1_5MHZ, "1.5MHZ"},
    {AM_HAL_IOM_1MHZ,   "1MHZ"},
    {AM_HAL_IOM_750KHZ, "750KHZ"},
    {AM_HAL_IOM_500KHZ, "500KHZ"},
    {AM_HAL_IOM_400KHZ, "400KHZ"},
    {AM_HAL_IOM_375KHZ, "375KHZ"},
    {AM_HAL_IOM_250KHZ, "250KHZ"},
    {AM_HAL_IOM_100KHZ, "100KHZ"},
    {AM_HAL_IOM_50KHZ,  "50KHZ"},
    {AM_HAL_IOM_10KHZ,  "10KHZ"},
};
#define IOM_SPEED_SPI_MAX       0  // 12 MHZ
#define IOM_SPEED_SPI_MIN       10 // 400 KHz
#endif // 0

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

uint32_t        IOSDMATCBBuffer[1024];

TaskHandle_t Ios_task_handle;

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
    .pNBTxnBuf          = &IOSDMATCBBuffer[0],
    .ui32NBTxnBufLength = sizeof(IOSDMATCBBuffer) / 4
};

#define MAX_SPI_SIZE    1023
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
    .pNBTxnBuf          = &IOSDMATCBBuffer[0],
    .ui32NBTxnBufLength = sizeof(IOSDMATCBBuffer) / 4
};

#define MAX_I2C_SIZE   255
#endif // 0

void          *pWidget;
char          errStr[128];

//*****************************************************************************
//
// Primary test case.
//
//*****************************************************************************
void IosLoopTask(void *pvParameters)
{
    am_widget_iosspi_config_t widgetCfg;
    am_widget_iosspi_test_t   testCfg;
    void *pWidget;
    uint32_t directSize, i, j;
    char errStr[200];
    bool bResult = false;
    const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;

    am_util_stdio_printf("IOS Task\r\n");

    widgetCfg.stimulusCfgIom.iomModule = TEST_IOM_MODULE;
    widgetCfg.stimulusCfgIos.iosModule = TEST_IOS_MODULE;

    // SPI
#ifdef TEST_SPI
    widgetCfg.stimulusCfgIos.iosHalCfg = g_sIOSSpiConfig;
    widgetCfg.stimulusCfgIom.iomHalCfg = g_sIOMSpiConfig;

    directSize = widgetCfg.stimulusCfgIos.iosHalCfg.ui32ROBase;

    while(1)
    {
        int spd = IOM_SPEED_SPI_MAX;
        widgetCfg.stimulusCfgIom.iomHalCfg.ui32ClockFreq = spiSpeedString[spd].MHz;
        am_util_stdio_printf("\nSPI Speed %s ", spiSpeedString[spd].MHzString);
        bResult = am_widget_iosspi_setup(&widgetCfg, &pWidget, errStr);
        if (bResult)
        {
            am_util_stdio_printf("Setup Failed: %s\n", errStr);
            break;
        }
        // Test LRAM R/W
        am_util_stdio_printf("\nAddress Range %d - %d ", 0, directSize - 1);
        for (i = 0; i < directSize; i++)
        {
            for (j = 1; j < (directSize - i + 1); j++)
            {
                testCfg.address = i;
                testCfg.size = j;
                bResult = am_widget_iosspi_test(pWidget, &testCfg, errStr);
                if (bResult)
                {
                    am_util_stdio_printf("\naddress = %d:size = %d FAIL\n", i, j);
                    am_util_stdio_printf("%s\n", errStr);
                }
                if (0 == (j%10))
                {
                  AM_UPDATE_TEST_PROGRESS();
                  am_util_stdio_printf("IOS\n");
                }
                // am_util_stdio_printf("\nIOS Test: address = %d:size = %d Success!\n", i, j);
                vTaskDelay(xDelay);
            }
        }
    }
#endif

#ifdef TEST_I2C
    // I2C
    widgetCfg.stimulusCfgIos.iosHalCfg = g_sIOSI2cConfig;
    widgetCfg.stimulusCfgIom.iomHalCfg = g_sIOMI2cConfig;

    directSize = widgetCfg.stimulusCfgIos.iosHalCfg.ui32ROBase;

    while(1)
    {
        int spd = IOM_SPEED_I2C_MAX;
        widgetCfg.stimulusCfgIom.iomHalCfg.ui32ClockFreq = i2cSpeedString[spd].MHz;
        am_util_stdio_printf("\nI2C Speed %s ", i2cSpeedString[spd].MHzString);
        bResult = am_widget_iosspi_setup(&widgetCfg, &pWidget, errStr);
        if (bResult)
        {
            am_util_stdio_printf("Setup Failed: %s\n", errStr);
            break;
        }
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
                    am_util_stdio_printf("\naddress = %d:size = %d FAIL\n", i, j);
                    am_util_stdio_printf("%s\n", errStr);
                }
                if (0 == (j%10))
                {
                  am_util_stdio_printf("IOS\n");
                }
                // am_util_stdio_printf("\nIOS Test: address = %d:size = %d Success!\n", i, j);
                vTaskDelay(xDelay);
            }
        }
    }
#endif
}

