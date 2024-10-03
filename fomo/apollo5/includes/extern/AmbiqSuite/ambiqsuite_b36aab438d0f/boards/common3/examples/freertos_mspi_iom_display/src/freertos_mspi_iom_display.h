//*****************************************************************************
//
//! @file freertos_mspi_iom_display.h
//!
//! @brief Global includes for the freertos_mspi_iom_display example.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef FREERTOS_MSPI_IOM_DISPLAY_H
#define FREERTOS_MSPI_IOM_DISPLAY_H

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
#define XIPMM_FREQ_HZ           7000
//#define ENABLE_XIP
//#define XIP_UNCACHED
#define XIP_FREQ_HZ             10000
//*****************************************************************************
// Customize the following for the test
//*****************************************************************************
// Control the example execution
#define DISPLAY_IOM_MODULE      AM_BSP_DISPLAY_IOM
#define IOM_FREQ                AM_HAL_IOM_24MHZ
#define MSPI_FREQ               AM_HAL_MSPI_CLK_48MHZ
#define DISPLAY_SPI_CS          AM_BSP_DISPLAY_SPI_CS

// Whether to set the display render window
// This example always uses full screen, and hence this can be disabled for optimization
//#define CONFIG_DISPLAY_WINDOW

#define MODE_DMA                0
#define MODE_XIP                1
#define MODE_XIPMM              2

// How to read the Src Buffers?
// We could read it into the internal SRAM using the DMA, or use the XIP apperture
// to use it directly
//#define MODE_SRCBUF_READ        MODE_DMA
//#define MODE_SRCBUF_READ        MODE_XIP
//#define SRCBUF_XIP_UNCACHED
//#define MODE_SRCBUF_READ        MODE_XIPMM

// How to write the Dest Buffers?
// We could write it into the internal SRAM and then use the DMA to write to PSRAM, or use the XIPMM apperture
// to use it directly
//#define MODE_DESTBUF_WRITE      MODE_DMA
//#define MODE_DESTBUF_WRITE      MODE_XIPMM

#ifndef MODE_DESTBUF_WRITE
#define MODE_DESTBUF_WRITE        MODE_DMA
#endif

#ifndef MODE_SRCBUF_READ
#define MODE_SRCBUF_READ          MODE_DMA
#endif

// Take minimal CPU time for compositing operations (just one vertical bar scrolling horizontally)
//#define MINIMAL_CPU_COMPOSITIING
// No Compositing
//#define NO_COMPOSITING
// No Rendering
//#define NO_RENDERING

// #define MSPI_PSRAM_SERIAL // Configures PSRAM in serial mode (default is quad)
// Control the size of the block of data to be transferred from PSRAM to Display
#define ROW_NUM                 240
#define COLUMN_NUM              240
#define FRAME_SIZE              ROW_NUM * COLUMN_NUM // Display device is limited to its resolution ratio

#define PSRAM_PAGE_SIZE         1024
// Size of temporary buffer being used
// Ideally this should be as large as possible for efficiency - within the limits of MSPI and IOM max transaction sizes
// The impact on memory could be either ways - as increasing this increases the temp buf size
// however, at the same time - it reduces the memory required for command queue, as there are less
// number of transactions needed to transfer full frame buffer
// Currently limited to 1024 - which is the page size of MSPI PSRAM - as it always operates in wrap mode
// hence we're limited to bursts of size 1K
#define TEMP_BUFFER_SIZE        PSRAM_PAGE_SIZE // ((AM_HAL_IOM_MAX_TXNSIZE_SPI + 256) & 0xFFFFFF00)
// Size of SPI Transaction
#define SPI_TXN_SIZE            TEMP_BUFFER_SIZE
// Total number of SPI_TXN_SIZE fragments needs to transfer one full frame buffer
#define NUM_FRAGMENTS           ((FRAME_SIZE + SPI_TXN_SIZE - 1) / SPI_TXN_SIZE)

//#define ENABLE_LOGGING

#define CPU_SLEEP_GPIO          0       // GPIO used to track CPU in sleep on logic analyzer
#define TEST_GPIO1              27
#define TEST_GPIO2              25
#define TEST_GPIO3              41


// #define MSPI_FLASH_SERIAL // Configures flash in serial mode (default is quad)

// Whether to use CQ Loop feature
// The CQ Loop feature allows configuration of the sequence once, which can be repeated, without reconfiguring
// This optimizes the CPU time required for programming the operation
// This is best mode to use if we always use the same frame buffer memory and render it completely
// This mode can not be used if the FB location and size of transfer
// is not fixed (e.g. if application is using more than one scratch FB space, and also if we are doing
// partial updates based on the display area changing)
// !!!!!CAUTION!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// When defined - we always compose on the same Frame Buffer. Since the Rendering is initiated first
// we always render the contents before they get recomposed and hence no artifacts are visible.
// It is however possible (if composing is fast) that this may end up showing tearing on the screen
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//#define SEQLOOP

// CQ Raw feature - enables preconstructed CQ using the raw interface
// This is an optimization of the normal mode (when SEQLOOP can not be used)
// This can be used even if the FB location and size of transfer
// is not fixed (e.g. if application is using more than one scratch FB space, and also if we are doing
// partial updates based on the display area changing
#define CQ_RAW

// Start MSPI-IOM transfer as soon as the new buffer is ready
// NOTE: Only rendering to IOM needs to wait for TE interrupt, not necessarily starting the MSPI Reads
// If not defined, the transfer is initiated only on TE interrupt
// CAUTION:
// With SEQLOOP, we're using same buffer for composition, and hence starting transfer early without rendering
// would increase the probability of Composition stoping on the buffer before rendering reads out
// So - START_MSPI_IOM_XFER_ASAP should not be defined
//#define START_MSPI_IOM_XFER_ASAP

// Serialize Composition and Rendering
// This will serialize composition and Rendering operation to eliminate any tearing as both operate on same buffer
#define SERIALIZE_COMPOSITION_WITH_RENDERING

#ifdef SEQLOOP
// Pause flag used for triggering MSPI sequence
// The sequence finishes one iteration and then waits for this flag to repeat
#define MSPI_PAUSEFLAG            0x4
#define NUM_FB                    1
#else
#define MSPI_PAUSEFLAG            0
#define NUM_FB                    2
#endif

#ifdef START_MSPI_IOM_XFER_ASAP
// Pause flag used to trigger IOM command queue
#define IOM_PAUSEFLAG             0x4
#else
#define IOM_PAUSEFLAG             0
#endif

// Select the Display Device
#define DISPLAY_DEVICE_ST7301     1
//*****************************************************************************
//*****************************************************************************

#if (DISPLAY_DEVICE_ST7301 == 1)
#include "am_devices_st7301.h"
#define DISPLAY_IOM_MODE           AM_HAL_IOM_SPI_MODE
#define am_display_devices_t       am_devices_st7301_config_t
#else
#error "Unknown Display Device"
#endif

// Helper Macros to map the ISR based on the IOM being used
#define IOM_INTERRUPT1(n)       AM_HAL_INTERRUPT_IOMASTER ## n
#define IOM_INTERRUPT(n)        IOM_INTERRUPT1(n)
#define DISPLAY_IOM_IRQn        ((IRQn_Type)(IOMSTR0_IRQn + DISPLAY_IOM_MODULE))

//
// Take over the interrupt handler for whichever IOM we're using.
//
#define display_iom_isr                                                          \
    am_iom_isr1(DISPLAY_IOM_MODULE)
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


//#define DEBUG_GPIO_HIGH(gpio)   am_hal_gpio_fastgpio_set(gpio)
//#define DEBUG_GPIO_LOW(gpio)    am_hal_gpio_fastgpio_clr(gpio)
#define DEBUG_GPIO_HIGH(gpio)   am_hal_gpio_state_write(gpio, AM_HAL_GPIO_OUTPUT_SET)
#define DEBUG_GPIO_LOW(gpio)    am_hal_gpio_state_write(gpio, AM_HAL_GPIO_OUTPUT_CLEAR)

//
// Typedef - to encapsulate device driver functions
//
typedef struct
{
    uint8_t  devName[20];
    uint32_t (*display_init)(uint32_t ui32Module, am_display_devices_t *pDevConfig, void **ppHandle, void **ppIomHandle);
    uint32_t (*display_term)(void *pHandle);

    uint32_t (*display_blocking_write)(void *pHandle, uint8_t *ui8TxBuffer, uint32_t ui32NumBytes);

    uint32_t (*display_blocking_cmd_write)(void *pHandle, bool bHiPrio, uint32_t cmd, bool bContinue);

    uint32_t (*display_nonblocking_write)(void *pHandle, uint8_t *ui8TxBuffer,
                                uint32_t ui32NumBytes,
                                bool bContinue,
                                am_hal_iom_callback_t pfnCallback,
                                void *pCallbackCtxt);

    uint32_t (*display_nonblocking_write_adv)(void *pHandle, uint8_t *ui8TxBuffer,
                                uint32_t ui32NumBytes,
                                bool bContinue,
                                uint32_t ui32Instr,
                                uint32_t ui32InstrLen,
                                uint32_t ui32PauseCondition,
                                uint32_t ui32StatusSetClr,
                                am_hal_iom_callback_t pfnCallback,
                                void *pCallbackCtxt);

    uint32_t (*display_blocking_read)(void *pHandle, uint8_t *pui8RxBuffer,
                            uint32_t ui32NumBytes);

    uint32_t (*display_nonblocking_read)(void *pHandle, uint8_t *pui8RxBuffer,
                                                      uint32_t ui32NumBytes,
                                                      am_hal_iom_callback_t pfnCallback,
                                                      void *pCallbackCtxt);
    uint32_t (*display_set_transfer_window)(void *pHandle, bool bHiPrio, uint32_t startRow, uint32_t startCol, uint32_t endRow, uint32_t endCol);
} display_device_func_t;


// MSPI PSRAM regions
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
#define PSRAM_XIPMM_OFFSET    (PSRAM_XIPMM_BASE - MSPI_XIPMM_BASE_ADDRESS)
#define PSRAM_XIPMM_SIZE      (1024*1024)

#define PSRAM_SRCFB_BASE      (PSRAM_GLOBALS_SIZE + PSRAM_GLOBALS_BASE)
#define PSRAM_SRCFB_OFFSET    (PSRAM_SRCFB_BASE - MSPI_XIP_BASE_ADDRESS)
#define PSRAM_SRCFB1_OFFSET   (PSRAM_SRCFB_OFFSET)
#define PSRAM_SRCFB2_OFFSET   (((PSRAM_SRCFB1_OFFSET + FRAME_SIZE + PSRAM_PAGE_SIZE - 1)/PSRAM_PAGE_SIZE)*PSRAM_PAGE_SIZE)
#define PSRAM_SRCFB_SIZE      (1024*1024)
#define PSRAM_ACTFB_BASE      (PSRAM_SRCFB_SIZE + PSRAM_SRCFB_BASE)
#define PSRAM_ACTFB_OFFSET    (PSRAM_ACTFB_BASE - MSPI_XIP_BASE_ADDRESS)
#define PSRAM_ACTFB1_OFFSET   (PSRAM_ACTFB_OFFSET)
#define PSRAM_ACTFB2_OFFSET   (((PSRAM_ACTFB1_OFFSET + FRAME_SIZE + PSRAM_PAGE_SIZE - 1)/PSRAM_PAGE_SIZE)*PSRAM_PAGE_SIZE)
#define PSRAM_ACTFB_SIZE      (1024*1024)

#define XIPMM_BLOCK_SIZE      64
#define NUM_XIPMM_BLOCK       (PSRAM_XIPMM_SIZE/XIPMM_BLOCK_SIZE)


#define RENDER_EVENT_START_NEW_FRAME        0x1
#define COMPOSE_EVENT_START_NEW_FRAME       0x1
#define COMPOSE_EVENT_NEW_FRAG_READY        0x2

typedef uint32_t (*mspi_xip_test_function_t)(uint32_t, uint32_t, uint32_t);

extern void            *g_MSPIDevHdl;
extern void            *g_MSPIHdl;
extern void            *g_IomDevHdl;
extern void            *g_IOMHandle;

extern uint32_t        g_ComposeState;

extern display_device_func_t display_func;

extern EventGroupHandle_t xMainEventHandle;
extern EventGroupHandle_t xComposeEventHandle;
extern EventGroupHandle_t xRenderEventHandle;
extern SemaphoreHandle_t xMSPIMutex;
#endif // FREERTOS_MSPI_IOM_DISPLAY_H
