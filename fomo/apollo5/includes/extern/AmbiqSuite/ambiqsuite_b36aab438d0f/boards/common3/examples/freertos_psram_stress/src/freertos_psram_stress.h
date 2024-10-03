//*****************************************************************************
//
//! @file freertos_psram_stress.h
//!
//! @brief Global includes for the freertos_psram_stress example.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef FREERTOS_PSRAM_STRESS_H
#define FREERTOS_PSRAM_STRESS_H

//*****************************************************************************
//
// Required built-ins.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

//*****************************************************************************
//
// Standard AmbiqSuite includes.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

//*****************************************************************************
//
// FreeRTOS include files.
//
//*****************************************************************************
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"
#include "timers.h"
//#include "rtos.h"

//*****************************************************************************
//
// Task include files.
//
//*****************************************************************************
#include "compose_task.h"
#include "main_task.h"
#include "render_task.h"

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_devices_mspi_psram_aps6404l.h"
#include "am_util.h"

//#define ENABLE_XIPMM
#if FIREBALL_CARD
//
// The Fireball device card multiplexes various devices including each of an SPI
// and I2C FRAM. The Fireball device driver controls access to these devices.
// If the Fireball card is not used, FRAM devices can be connected directly
// to appropriate GPIO pins.
//
#include "am_devices_fireball.h"
#endif // FIREBALL_CARD

//*****************************************************************************
// Customize the following for the test
//*****************************************************************************
// Control the example execution
#define FRAM_IOM_MODULE         1
// Valid only if running in non-autonomous mode
#define NUM_ITERATIONS          32
#define XIP_ITERATIONS          1

#define MSPI_XIP_BASE_ADDRESS   0x04000000
#define MSPI_XIPMM_BASE_ADDRESS 0x51000000UL
// Define 4 regions in the PSRAM

// PSRAM space is divided into following sections for this experiment
// XIP - This is where we'll load the library code
// XIPMM - this is where we'll allocate R/W variables
// FB_Src - this is where we'll initialize Source Frame Buffer Images - Base1 & Base2
// FB - Active Frame buffers - FB1 & FB2
#define PSRAM_XIP_BASE        MSPI_XIP_BASE_ADDRESS
#define PSRAM_XIP_OFFSET      (PSRAM_XIP_BASE - MSPI_XIP_BASE_ADDRESS)
#define PSRAM_XIP_SIZE        (1024*1024)
#define PSRAM_GLOBALS_BASE    (PSRAM_XIP_SIZE + PSRAM_XIP_BASE)
#define PSRAM_GLOBALS_OFFSET  (PSRAM_GLOBALS_BASE - MSPI_XIP_BASE_ADDRESS)
#define PSRAM_GLOBALS_SIZE    (1024*1024)
#define PSRAM_XIPMM_BASE      (PSRAM_GLOBALS_BASE + MSPI_XIPMM_BASE_ADDRESS - MSPI_XIP_BASE_ADDRESS)
#define PSRAM_XIPMM_OFFSET    (PSRAM_XIPMM_BASE - MSPI_XIP_BASE_ADDRESS)
#define PSRAM_XIPMM_SIZE      (1024*1024)

#define PSRAM_SRCFB_BASE      (PSRAM_GLOBALS_SIZE + PSRAM_GLOBALS_BASE)
#define PSRAM_SRCFB_OFFSET    (PSRAM_SRCFB_BASE - MSPI_XIP_BASE_ADDRESS)
#define PSRAM_SRCFB_SIZE      (1024*1024)
#define PSRAM_ACTFB_BASE      (PSRAM_SRCFB_SIZE + PSRAM_SRCFB_BASE)
#define PSRAM_ACTFB_OFFSET    (PSRAM_ACTFB_BASE - MSPI_XIP_BASE_ADDRESS)
#define PSRAM_ACTFB_SIZE      (1024*1024)

// #define MSPI_FLASH_SERIAL // Configures flash in serial mode (default is quad)

// Frame Buffer Size
// Control the size of the block of data to be transferred from Flash to FRAM
#define FB_SIZE                 57600 // FRAM device is limited to 128K in size
#define FB_FRAGMENT_SIZE        (FB_SIZE/4)

#define XIPMM_BLOCK_SIZE      512
#define NUM_XIPMM_BLOCK       (PSRAM_XIPMM_SIZE/XIPMM_BLOCK_SIZE)



#define CPU_SLEEP_GPIO          29       // GPIO used to track CPU in sleep on logic analyzer
#define DEFAULT_TIMEOUT         10000
//#define ENABLE_LOGGING
#define TEST_GPIO               7
#define TEST_GPIO1              6
#define TEST_GPIO2              28
// Whether to use CQ Loop feature
//#define SEQLOOP
#ifdef SEQLOOP
// Keep running without CPU intervention in infinite loop
// If not defined - after each loop, CPU wakes up to trigger next sequence, and we stop after number of iterations
//#define RUN_AUTONOMOUS
#else
// Enables Verification of the FRAM data after it has been written to
// Can not be enabled if running in SEQLOOP mode
#define VERIFY_DATA
#endif

// Select the FRAM Device
#define FRAM_DEVICE_MB85RS1MT     1     // SPI Fram (Fireball)
//#define FRAM_DEVICE_MB85RC64TA    1     // I2C Fram (Fireball)
//#define FRAM_DEVICE_MB85RS64V     1     // SPI Fram
//#define FRAM_DEVICE_MB85RC256V    1     // I2C Fram
//*****************************************************************************
//*****************************************************************************

#define IOM_TEST_FREQ           AM_HAL_IOM_24MHZ

#if (FRAM_DEVICE_MB85RS1MT == 1)
#include "am_devices_mb85rs1mt.h"
#define FRAM_DEVICE_ID          AM_DEVICES_MB85RS1MT_ID
#define am_iom_test_devices_t   am_devices_mb85rs1mt_config_t
#elif (FRAM_DEVICE_MB85RC256V == 1)
#include "am_devices_mb85rc256v.h"
#define FRAM_DEVICE_ID          AM_DEVICES_MB85RC256V_ID
#define am_iom_test_devices_t   am_devices_mb85rc256v_config_t
#elif (FRAM_DEVICE_MB85RS64V == 1)
#include "am_devices_mb85rs64v.h"
#define FRAM_DEVICE_ID          AM_DEVICES_MB85RS64V_ID
#define am_iom_test_devices_t   am_devices_mb85rs64v_config_t
#elif (FRAM_DEVICE_MB85RC64TA == 1)
#include "am_devices_mb85rc256v.h"
#define FRAM_DEVICE_ID          AM_DEVICES_MB85RC64TA_ID
#define am_iom_test_devices_t   am_devices_mb85rc256v_config_t
#else
#error "Unknown FRAM Device"
#endif

// Size of temporary buffer being used - Size it to max SPI size
#define TEMP_BUFFER_SIZE        ((AM_HAL_IOM_MAX_TXNSIZE_SPI + 256) & 0xFFFFFF00)

// Size of SPI Transaction
#define SPI_TXN_SIZE            3840 // ((AM_HAL_IOM_MAX_TXNSIZE_SPI) & 0xFFFFFF00)

// Helper Macros to map the ISR based on the IOM being used
#define IOM_INTERRUPT1(n)       AM_HAL_INTERRUPT_IOMASTER ## n
#define IOM_INTERRUPT(n)        IOM_INTERRUPT1(n)
#define FRAM_IOM_IRQn           ((IRQn_Type)(IOMSTR0_IRQn + FRAM_IOM_MODULE))

//
// Take over the interrupt handler for whichever IOM we're using.
//
#define fram_iom_isr                                                          \
    am_iom_isr1(FRAM_IOM_MODULE)
#define am_iom_isr1(n)                                                        \
    am_iom_isr(n)
#define am_iom_isr(n)                                                         \
    am_iomaster ## n ## _isr

// Friendlier names for the bit masks
#define AM_REG_IOM_CQFLAGS_CQFLAGS_MSPI1START   (_VAL2FLD(IOM0_CQPAUSEEN_CQPEN, IOM0_CQPAUSEEN_CQPEN_SWFLAGEN1))
#define AM_REG_IOM_CQFLAGS_CQFLAGS_MSPI0START   (_VAL2FLD(IOM0_CQPAUSEEN_CQPEN, IOM0_CQPAUSEEN_CQPEN_SWFLAGEN0))

#define IOM_SIGNAL_MSPI_BUFFER0       (AM_REG_IOM_CQFLAGS_CQFLAGS_MSPI0START << 8)
#define IOM_SIGNAL_MSPI_BUFFER1       (AM_REG_IOM_CQFLAGS_CQFLAGS_MSPI1START << 8)
#define MSPI_SIGNAL_IOM_BUFFER0       (MSPI_CQFLAGS_CQFLAGS_SWFLAG0 << 8)
#define MSPI_SIGNAL_IOM_BUFFER1       (MSPI_CQFLAGS_CQFLAGS_SWFLAG1 << 8)

#define IOM_WAIT_FOR_MSPI_BUFFER0     (_VAL2FLD(IOM0_CQPAUSEEN_CQPEN, IOM0_CQPAUSEEN_CQPEN_MSPI0XNOREN))
#define IOM_WAIT_FOR_MSPI_BUFFER1     (_VAL2FLD(IOM0_CQPAUSEEN_CQPEN, IOM0_CQPAUSEEN_CQPEN_MSPI1XNOREN))
#define MSPI_WAIT_FOR_IOM_BUFFER0     (_VAL2FLD(MSPI_CQFLAGS_CQFLAGS, MSPI_CQFLAGS_CQFLAGS_IOM0READY))
#define MSPI_WAIT_FOR_IOM_BUFFER1     (_VAL2FLD(MSPI_CQFLAGS_CQFLAGS, MSPI_CQFLAGS_CQFLAGS_IOM1READY))


#ifdef ENABLE_LOGGING
#define DEBUG_PRINT am_util_stdio_printf
#else
#define DEBUG_PRINT(...)
#endif
#define DEBUG_PRINT_SUCCESS(...)

#define TEST_GPIO_HIGH(pin)   am_hal_gpio_fastgpio_set(pin)
#define TEST_GPIO_LOW(pin)    am_hal_gpio_fastgpio_clr(pin)
//
// Typedef - to encapsulate device driver functions
//
typedef struct
{
    uint8_t  devName[20];
    uint32_t (*fram_init)(uint32_t ui32Module, am_iom_test_devices_t *pDevConfig, void **ppHandle, void **ppIomHandle);
    uint32_t (*fram_term)(void *pHandle);

    uint32_t (*fram_read_id)(void *pHandle, uint32_t *pDeviceID);

    uint32_t (*fram_blocking_write)(void *pHandle, uint8_t *ui8TxBuffer,
                             uint32_t ui32WriteAddress,
                             uint32_t ui32NumBytes);

    uint32_t (*fram_nonblocking_write)(void *pHandle, uint8_t *ui8TxBuffer,
                                uint32_t ui32WriteAddress,
                                uint32_t ui32NumBytes,
                                am_hal_iom_callback_t pfnCallback,
                                void *pCallbackCtxt);

    uint32_t (*fram_nonblocking_write_adv)(void *pHandle, uint8_t *ui8TxBuffer,
                                uint32_t ui32WriteAddress,
                                uint32_t ui32NumBytes,
                                uint32_t ui32PauseCondition,
                                uint32_t ui32StatusSetClr,
                                am_hal_iom_callback_t pfnCallback,
                                void *pCallbackCtxt);

    uint32_t (*fram_blocking_read)(void *pHandle, uint8_t *pui8RxBuffer,
                            uint32_t ui32ReadAddress,
                            uint32_t ui32NumBytes);

    uint32_t (*fram_nonblocking_read)(void *pHandle, uint8_t *pui8RxBuffer,
                                                      uint32_t ui32ReadAddress,
                                                      uint32_t ui32NumBytes,
                                                      am_hal_iom_callback_t pfnCallback,
                                                      void *pCallbackCtxt);
    uint32_t (*fram_command_send)(void *pHandle, uint32_t ui32Cmd);
#if FIREBALL_CARD
    am_devices_fireball_control_e fram_fireball_control;
#else
    uint32_t                      fram_fireball_control;
#endif
} fram_device_func_t;


#define RENDER_EVENT_START_NEW_FRAME        0x1
#define COMPOSE_EVENT_START_NEW_FRAME       0x1
#define COMPOSE_EVENT_NEW_FRAG_READY        0x2

typedef uint32_t (*mspi_xip_test_function_t)(uint32_t, uint32_t, uint32_t);

extern void            *g_MSPIDevHdl;
extern void            *g_MSPIHdl;
extern void            *g_IomDevHdl;
extern void            *g_IOMHandle;

extern uint32_t        g_numIter;
extern uint32_t        g_ComposeState;

extern fram_device_func_t fram_func;

extern EventGroupHandle_t xMainEventHandle;
extern EventGroupHandle_t xComposeEventHandle;
extern EventGroupHandle_t xRenderEventHandle;
#endif // FREERTOS_PSRAM_STRESS_H
