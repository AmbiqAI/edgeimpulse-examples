//*****************************************************************************
//
//! @file iom_test_common.h
//!
//! @brief IOM test cases common definitions.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef IOM_TEST_COMMON_H
#define IOM_TEST_COMMON_H

#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_widget_iom.h"
#include "am_util.h"
#include "am_devices_fireball.h"

#define AM_WIDGET_DEBUG_ENABLE  1

#if (AM_WIDGET_DEBUG_ENABLE)
    #define AM_WIDFET_DEBUG_PRINTF(...)  am_util_stdio_printf(__VA_ARGS__)
#else
    #define AM_WIDFET_DEBUG_PRINTF(...)
#endif

#if defined(PRINT_OUTPUT_WITH_UART)
#define LOG_PRINTF(expr ...)            am_util_stdio_printf(expr "\n")
#else
#define LOG_PRINTF(expr ...)            am_util_stdio_printf(expr "\r\n")
#endif


//#define IOM_TEST_NO_QUEUE
// Whether to queue more than one non-blocking transactions
#ifndef IOM_TEST_NO_QUEUE
#define IOM_TEST_QUEUE
#endif

//
// How to wait for the non-blocking calls
//
#ifndef IOM_TEST_WAIT_MODE
// Making default setting to not sleep while waiting for response for non-blocking call.
// On the FPGA platform, this would cause debugger to lose control if there was
// some real failure, hence hampering the ability to debug it.
#if APOLLO5_FPGA
#define IOM_TEST_WAIT_MODE      AM_HAL_SYSCTRL_WAKE
#else
#define IOM_TEST_WAIT_MODE      AM_HAL_SYSCTRL_DEEPSLEEP
#endif
#endif

#if defined(IOM_I2C_MULTIDROP_TEST)
#define IOM_TEST_NUM_INSTANCES 1
#else
#define IOM_TEST_NUM_INSTANCES 1
#endif

#define IOM_INT_TIMEOUT        (100)

#if APS6404L
#define SPEED_STRING_SIZE 4
#else
#define SPEED_STRING_SIZE 19
#endif

typedef enum
{
    AMTEST_IOM_FRAM_STATUS_NOT_KNOWN = 0,
    AMTEST_IOM_FRAM_STATUS_PRESENT,
    AMTEST_IOM_FRAM_STATUS_NOT_PRESENT,
} AMTEST_IOM_FRAM_STATUS_E;

#define APOLLO510L_IOM_INSTANCE_NUMBER     6
typedef struct
{
    uint32_t        ui32InstanceStart;
    uint32_t        ui32InstanceEnd;
    uint32_t        instances[APOLLO510L_IOM_INSTANCE_NUMBER];
    uint32_t        ui32SpeedStart;
    uint32_t        ui32SpeedEnd;
}iom_test_cfg_t;

typedef struct
{
    uint32_t        ui32Instance;
    uint32_t        ui32SpeedStart;
    uint32_t        ui32SpeedEnd;
    uint32_t        ui32SizeStart;
    uint32_t        ui32SizedEnd;
}iom_fd_test_cfg_t;

typedef struct
{
    const uint32_t speed;
    const char *MHzString;
}iom_speed_t;

extern uint32_t DMATCBBuffer[1024];

extern iom_speed_t iom_test_speeds[];
extern uint32_t IOMSPISPeedsDefault[];
extern uint32_t IOMI2CSPeedsDefault[];
extern const am_widget_iom_devices_config_t      DefaultI2CConfig;
extern const am_widget_iom_devices_config_t      DefaultSPIConfig;
extern void     *g_pIOMHandle;
extern char     errStr[256];


extern bool g_bMaxSpeed48MHz;
extern void iom_test_globalSetUp(void);
extern void iom_test_globalTearDown(void);
extern const char *getSpeedString(int speedIndex);
extern uint32_t    iom_instances[];

extern AMTEST_IOM_FRAM_STATUS_E isFramExpected(uint32_t ui32Module);

extern uint32_t SetFireball(am_devices_fireball_control_e eFBState);
extern uint32_t init_fireball(uint32_t ui32IOMNum);

#endif // IOM_TEST_COMMON_H
