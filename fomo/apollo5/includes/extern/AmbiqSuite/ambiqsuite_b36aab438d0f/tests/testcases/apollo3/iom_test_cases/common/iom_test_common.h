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

//#define IOM_TEST_NO_QUEUE
// Whether to queue more than one non-blocking transactions
#ifndef IOM_TEST_NO_QUEUE
#define IOM_TEST_QUEUE
#endif

// Whether to put IOM in low power mode between the tests
#ifndef IOM_TEST_POWER_SAVE_RESTORE
#define IOM_TEST_POWER_SAVE_RESTORE
#endif

#ifndef IOM_TEST_POWERSAVE_MODE
// Low power mode for IOM
#define IOM_TEST_POWERSAVE_MODE AM_HAL_SYSCTRL_DEEPSLEEP
#endif

//
// How to wait for the non-blocking calls
//
#ifndef IOM_TEST_WAIT_MODE
// Making default setting to not sleep while waiting for response for non-blocking call.
// On the FPGA platform, this would cause debugger to lose control if there was
// some real failure, hence hampering the ability to debug it.
#if APOLLO4_FPGA
#define IOM_TEST_WAIT_MODE      AM_HAL_SYSCTRL_WAKE
#else
#define IOM_TEST_WAIT_MODE      AM_HAL_SYSCTRL_DEEPSLEEP
#endif
#endif

#define IOM_TEST_NUM_INSTANCES 7

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

extern void     *g_pIOMHandle;
extern const am_widget_iom_devices_t      DefaultI2CConfig;
extern const am_widget_iom_devices_t      DefaultSPIConfig;
extern uint32_t        IOMI2CSpeeds[];
extern uint32_t IOMSPISpeeds[];
extern bool g_bMaxSpeed48MHz;
extern void iom_test_globalSetUp(void);
extern void iom_test_globalTearDown(void);
extern const char *getSpeedString(int speedIndex);
extern uint32_t    iom_instances[];

extern AMTEST_IOM_FRAM_STATUS_E isFramExpected(uint32_t ui32Module);

extern uint32_t SetFireball(am_devices_fireball_control_e eFBState);

#endif // IOM_TEST_COMMON_H
