//*****************************************************************************
//
//! @file ios_test_common.h
//!
//! @brief IOS test cases common definitions.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef IOS_TEST_COMMON_H
#define IOS_TEST_COMMON_H

#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_util.h"

#define AM_WIDGET_DEBUG_ENABLE  1
#if (AM_WIDGET_DEBUG_ENABLE)
    #define AM_WIDFET_DEBUG_PRINTF(...)  am_util_stdio_printf(__VA_ARGS__)
#else
    #define AM_WIDFET_DEBUG_PRINTF(...)
#endif

#if defined(PRINT_OUTPUT_WITH_UART)
#define LOG_PRINTF(expr ...)    am_util_stdio_printf(expr "\n")
#else
#define LOG_PRINTF(expr ...)    am_util_stdio_printf(expr "\r\n")
#endif


// #define TEST_IOM_MODULE     0


#define IOM_SPEED_SPI_MAX       1
#define IOM_SPEED_SPI_MIN       1


#define IOM_SPEED_I2C_MAX       0  // 1 MHz
#define IOM_SPEED_I2C_MIN       2  // 100 KHz

#define APOLLO510L_IOS_INSTANCE_NUMBER     2
typedef struct
{
    uint32_t        ui32InstanceStart;
    uint32_t        ui32InstanceEnd;
    uint32_t        instances[APOLLO510L_IOS_INSTANCE_NUMBER];

    uint32_t        ui32SPISpeedStart;
    uint32_t        ui32SPISpeedEnd;
    uint32_t        ui32I2CSpeedStart;
    uint32_t        ui32I2CSpeedEnd;

}ios_test_cfg_t;

typedef struct
{
    uint32_t        ui32SpeedStart;
    uint32_t        ui32SpeedEnd;
    uint32_t        ui32SizeStart;
    uint32_t        ui32SizedEnd;
}ios_fd_test_cfg_t;

typedef struct
{
    const uint32_t speed;
    const char      *MHzString;
}ios_speed_t;

extern ios_speed_t ios_spi_speeds[];
extern ios_speed_t ios_i2c_speeds[];

extern am_hal_ios_config_t g_sIOSSpiConfig;
extern am_hal_iom_config_t g_sIOMSpiConfig;
extern am_hal_ios_config_t g_sIOSI2cConfig;
extern am_hal_iom_config_t g_sIOMI2cConfig;

void ios_test_common_globalSetUp(void);

#endif  //IOS_TEST_COMMON_H
