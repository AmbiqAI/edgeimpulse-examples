//*****************************************************************************
//
//! @file ios_test_common.c
//!
//! @brief IOS test cases common definitions and functions.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "ios_test_common.h"

//
// Map IOM Speed to String. Also used for speed iteration.
//
ios_speed_t ios_spi_speeds[] =
{
    // {AM_HAL_IOM_48MHZ,  "48MHZ"},
    {AM_HAL_IOM_24MHZ,  "24MHZ"},
    {AM_HAL_IOM_16MHZ,  "16MHZ"},
    {AM_HAL_IOM_12MHZ,  "12MHZ"},
    {AM_HAL_IOM_8MHZ,   "8MHZ"},
    {AM_HAL_IOM_6MHZ,   "6MHZ"},
    {AM_HAL_IOM_4MHZ,   "4MHZ"},
    {AM_HAL_IOM_3MHZ,   "3MHZ"},
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
    {AM_HAL_IOM_10KHZ,  "10KHZ"}
};

ios_speed_t ios_i2c_speeds[] =
{
    {AM_HAL_IOM_1MHZ,   "1MHZ"},
    {AM_HAL_IOM_400KHZ, "400KHZ"},
    {AM_HAL_IOM_100KHZ, "100KHZ"},
};

AM_SHARED_RW uint32_t        DMATCBBuffer[1024];

//*****************************************************************************
//
// SPI Slave Configuration
//
//*****************************************************************************

am_hal_ios_config_t g_sIOSSpiConfig =
{
    // Configure the IOS in SPI mode.
    .ui32InterfaceSelect = AM_HAL_IOS_USE_SPI,

    // Eliminate the "read-only" section, so an external host can use the
    // entire "direct write" section.
    .ui32ROBase = 0x8,

    // Making the "FIFO" section as big as possible.
    .ui32FIFOBase = 0x10,

// #if (TEST_IOS_MODULE == 0)
//     // We don't need any RAM space, so extend the FIFO all the way to the end
//     // of the LRAM.
//     .ui32RAMBase = 0x100,

//     // FIFO Threshold - set to half the size
//     .ui32FIFOThreshold = 0x80,
// #else
    .ui32RAMBase = 0x40,
    .ui32FIFOThreshold = 0x20,
// #endif
    // FIFO Wraparound Enable
    .ui8WrapEnabled = 1,
};

am_hal_iom_config_t g_sIOMSpiConfig =
{
    .eInterfaceMode     = AM_HAL_IOM_SPI_MODE,
    .ui32ClockFreq      = AM_HAL_IOM_8MHZ,
    .eSpiMode           = AM_HAL_IOM_SPI_MODE_0,
    .pNBTxnBuf          = &DMATCBBuffer[0],
    .ui32NBTxnBufLength = sizeof(DMATCBBuffer) / 4
};

#define     I2C_ADDR            0x10

//*****************************************************************************
//
// I2C Slave Configuration
//
//*****************************************************************************
am_hal_ios_config_t g_sIOSI2cConfig =
{
    // Configure the IOS in I2C mode.
    .ui32InterfaceSelect = AM_HAL_IOS_USE_I2C | AM_HAL_IOS_I2C_ADDRESS(I2C_ADDR << 1),
    // Eliminate the "read-only" section, so an external host can use the
    // entire "direct write" section.
    .ui32ROBase = 0x8,

    // Set the FIFO base to the maximum value, making the "direct write"
    // section as big as possible.
    .ui32FIFOBase = 0x10,

    // We don't need any RAM space, so extend the FIFO all the way to the end
    // of the LRAM.
    // .ui32RAMBase = 0x100,
    // // FIFO Threshold - set to half the size
    // .ui32FIFOThreshold = 0x80,
    .ui32RAMBase = 0x40,
    // FIFO Threshold - set to half the size
    .ui32FIFOThreshold = 0x20,

    // FIFO Wraparound Enable
    .ui8WrapEnabled = 1,
};

am_hal_iom_config_t g_sIOMI2cConfig =
{
    .eInterfaceMode     = AM_HAL_IOM_I2C_MODE,
    .ui32ClockFreq      = AM_HAL_IOM_100KHZ,
    .eSpiMode           = AM_HAL_IOM_SPI_MODE_0,
    .pNBTxnBuf          = &DMATCBBuffer[0],
    .ui32NBTxnBufLength = sizeof(DMATCBBuffer) / 4
};

#define IOS_WRAPAROUND_MAX_LRAM_SIZE    0xF0


//#define IOS_WRAP_START_ADDRESS          0x0           // have 2nd transaction halt issue.
#define IOS_WRAP_START_ADDRESS          0x08
//#define IOS_WRAP_START_ADDRESS          0x38
//#define IOS_WRAP_START_ADDRESS          0x78


#if (IOS_WRAP_START_ADDRESS == 0x00)
#define IOS_REGACC_THRESHOLD_01		AM_HAL_IOS_ACCESS_INT_08	// 0x8 at wrap mode
#define IOS_REGACC_THRESHOLD_02		AM_HAL_IOS_ACCESS_INT_2F	// 0x7F at wrap mode
#define IOS_WRAP_HALF_ADDRESS           0x7F
#elif (IOS_WRAP_START_ADDRESS == 0x08)
#define IOS_REGACC_THRESHOLD_01		AM_HAL_IOS_ACCESS_INT_0F	// 0xF at wrap mode
#define IOS_REGACC_THRESHOLD_02		AM_HAL_IOS_ACCESS_INT_2F	// 0x7F at wrap mode
#define IOS_WRAP_HALF_ADDRESS           0x7F
#elif (IOS_WRAP_START_ADDRESS == 0x38)
#define IOS_REGACC_THRESHOLD_01		AM_HAL_IOS_ACCESS_INT_1F	// 0x3F at wrap mode
#define IOS_REGACC_THRESHOLD_02		AM_HAL_IOS_ACCESS_INT_37	// 0x9F at wrap mode
#define IOS_WRAP_HALF_ADDRESS           0x9F
#elif (IOS_WRAP_START_ADDRESS == 0x78)
#define IOS_REGACC_THRESHOLD_01		AM_HAL_IOS_ACCESS_INT_33	// 0x8F at wrap mode
#define IOS_REGACC_THRESHOLD_02		AM_HAL_IOS_ACCESS_INT_3F	// 0xBF at wrap mode
#define IOS_WRAP_HALF_ADDRESS           0xBF                            //
#endif
//*****************************************************************************
//
// SPI Slave Configuration
//
//*****************************************************************************
static am_hal_ios_config_t g_sIOSSpi_Wrap_Config =
{
    // Configure the IOS in SPI mode.
    .ui32InterfaceSelect = AM_HAL_IOS_USE_SPI,

    // Eliminate the "read-only" section, so an external host can use the
    // entire "direct write" section.
#if (IOS_WRAP_START_ADDRESS == 0x0)
    .ui32ROBase = IOS_WRAP_START_ADDRESS,
#elif (IOS_WRAP_START_ADDRESS == 0x78)
    .ui32ROBase = IOS_WRAP_START_ADDRESS,
#else
    .ui32ROBase = IOS_WRAP_START_ADDRESS - 0x8,
#endif

    // Making the "FIFO" section as big as possible.
#if (IOS_WRAP_START_ADDRESS == 0x78)
    .ui32FIFOBase = IOS_WRAP_START_ADDRESS + 0x8,
#else
    .ui32FIFOBase = IOS_WRAP_START_ADDRESS,
#endif

    // We don't need any RAM space, so extend the FIFO all the way to the end
    // of the LRAM.
    .ui32RAMBase = (IOS_WRAPAROUND_MAX_LRAM_SIZE + 0x8),         // FIFO MAX?

    // FIFO Threshold - set to half the size
    .ui32FIFOThreshold = 0x20,

    // Direct access wraparound enable.
    .ui8WrapEnabled = 1,
};


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

void ios_test_common_globalSetUp(void)
{
    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

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
}