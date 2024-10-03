//*****************************************************************************
//
//! @file iom_fram.c
//!
//! @brief Example that demonstrates IOM, connecting to I2C FRAM
//! Purpose: FRAM is initialized with a known pattern data using Blocking IOM Write.
//! This example starts a 1 second timer. At each 1 second period, it initiates
//! reading a fixed size block from the FRAM device using Non-Blocking IOM
//! Read, and comparing against the predefined pattern
//!
//! Printing takes place over the ITM at 1M Baud.
//!
//! Additional Information:
//! Define one of FRAM_DEVICE_ macros to select the FRAM device
//! Recommend to use 1.8V power supply voltage with CYGNUS shield
//!
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

//*****************************************************************************
// Customize the following for the test
//*****************************************************************************
// Control the example execution
#define FRAM_IOM_MODULE         6
#define PATTERN_BUF_SIZE        128
#define NUM_INERATIONS          16
//*****************************************************************************
//*****************************************************************************
#include "am_devices_mb85rc256v.h"
#define FRAM_DEVICE_ID          AM_DEVICES_MB85RC64TA_ID
#define am_iom_test_devices_t   am_devices_mb85rc256v_config_t

#define FRAM_IOM_IRQn           ((IRQn_Type)(IOMSTR0_IRQn + FRAM_IOM_MODULE))

#define FRAM_IOM_FREQ           AM_HAL_IOM_1MHZ

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

    uint32_t (*fram_blocking_read)(void *pHandle, uint8_t *pui8RxBuffer,
                            uint32_t ui32ReadAddress,
                            uint32_t ui32NumBytes);

    uint32_t (*fram_nonblocking_read)(void *pHandle, uint8_t *pui8RxBuffer,
                                                      uint32_t ui32ReadAddress,
                                                      uint32_t ui32NumBytes,
                                                      am_hal_iom_callback_t pfnCallback,
                                                      void *pCallbackCtxt);
} fram_device_func_t;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
volatile bool                           g_bReadFram = false;
volatile bool                           g_bVerifyReadData = false;
static void                                    *g_IomDevHdl;
static void                                    *g_pIOMHandle;
static am_hal_iom_buffer(PATTERN_BUF_SIZE)     gPatternBuf;
static am_hal_iom_buffer(PATTERN_BUF_SIZE)     gRxBuf;
// Buffer for non-blocking transactions
static uint32_t                                DMATCBBuffer[256];
static fram_device_func_t device_func =
{
    .devName = "I2C FRAM MB85RC64TA",
    .fram_init = am_devices_mb85rc256v_init,
    .fram_term = am_devices_mb85rc256v_term,
    .fram_read_id = am_devices_mb85rc256v_read_id,
    .fram_blocking_write = am_devices_mb85rc256v_blocking_write,
    .fram_nonblocking_write = am_devices_mb85rc256v_nonblocking_write,
    .fram_blocking_read = am_devices_mb85rc256v_blocking_read,
    .fram_nonblocking_read = am_devices_mb85rc256v_nonblocking_read,
};

//*****************************************************************************
//
// Timer configuration.
//
//*****************************************************************************
#define LOOP_TIMER              0

am_hal_timer_config_t  g_TimerConfig;

//*****************************************************************************
//
// Function to initialize Timer A0 to interrupt every 1 second.
//
//*****************************************************************************
void
timer_init(void)
{
    //
    // Set up the default configuration.
    //
    am_hal_timer_default_config_set(&g_TimerConfig);

    //
    // Update the clock in the config.
    //
    g_TimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_LFRC_DIV32;
    g_TimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    g_TimerConfig.ui32Compare0 = 1000;
    g_TimerConfig.ui32Compare1 = 0;

    //
    // Configure the TIMER.
    //
    am_hal_timer_config(LOOP_TIMER, &g_TimerConfig);
    //
    // Clear the TIMER.
    //
    am_hal_timer_clear(LOOP_TIMER);
    am_hal_timer_enable(LOOP_TIMER);
}

//*****************************************************************************
//
// Timer Interrupt Service Routine (ISR)
//
//*****************************************************************************
//void am_timer00_isr(void)
//{
//    g_bReadFram = true;
//}
//
//void
//am_ctimer_isr(void)
//{
//    uint32_t ui32Status;
//    //
//    // Clear the interrupt that got us here.
//    //
//    am_hal_timer_interrupt_status_get(false, &ui32Status);
//    am_hal_timer_interrupt_clear(ui32Status);
//}

//
// Take over the interrupt handler for whichever IOM we're using.
//
#define fram_iom_isr                                                          \
    am_iom_isr1(FRAM_IOM_MODULE)
#define am_iom_isr1(n)                                                        \
    am_iom_isr(n)
#define am_iom_isr(n)                                                         \
    am_iomaster ## n ## _isr

//*****************************************************************************
//
// IOM ISRs.
//
//*****************************************************************************
//
//! Take over default ISR. (Queue mode service)
//
void fram_iom_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle, true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle, ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle, ui32Status);
        }
    }
}

void
init_pattern(void)
{
    uint32_t i;
    for (i = 0; i < PATTERN_BUF_SIZE; i++)
    {
        gPatternBuf.bytes[i] = i & 0xFF;
    }
}

int
fram_init(void)
{
    uint32_t ui32Status;
    uint32_t ui32DeviceId = 0;

    am_iom_test_devices_t stFramConfig;
    stFramConfig.ui32ClockFreq = FRAM_IOM_FREQ;
    stFramConfig.pNBTxnBuf = DMATCBBuffer;
    stFramConfig.ui32NBTxnBufLength = sizeof(DMATCBBuffer) / 4;

    ui32Status = device_func.fram_init(FRAM_IOM_MODULE, &stFramConfig, &g_IomDevHdl, &g_pIOMHandle);
    if (0 == ui32Status)
    {
        ui32Status = device_func.fram_read_id(g_IomDevHdl, &ui32DeviceId);

        if ((ui32Status  != 0) || (ui32DeviceId != FRAM_DEVICE_ID))
        {
            return -1;
        }
        am_util_debug_printf("%s Found\n", device_func.devName);
        // Set up a pattern data in FRAM memory
        am_util_debug_printf("Setting up data pattern in FRAM using blocking write\n");
        return device_func.fram_blocking_write(g_IomDevHdl, &gPatternBuf.bytes[0], 0, PATTERN_BUF_SIZE);
    }
    else
    {
        return -1;
    }
}

static void read_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("\nFRAM Read Failed 0x%x\n", transactionStatus);
    }
    else
    {
        am_util_stdio_printf(".");
        g_bVerifyReadData = true;
    }
}



void
read_fram(void)
{
    uint32_t ui32Status;
    // Initiate read of a block of data from FRAM
    ui32Status = device_func.fram_nonblocking_read(g_IomDevHdl, &gRxBuf.bytes[0], 0, PATTERN_BUF_SIZE, read_complete, 0);
    if (ui32Status == 0)
    {
        g_bReadFram = false;
    }
}

void
fram_term(void)
{
    device_func.fram_term(g_IomDevHdl);
}

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int iom6_fram_test(uint32_t mode)
{
    int iRet;
    uint32_t numRead = 0;


    am_util_debug_printf("IOM FRAM Example\n");

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_EnableIRQ(FRAM_IOM_IRQn);

    am_hal_interrupt_master_enable();

    //
    // Initialize the FRAM Device
    //
    iRet = fram_init();
    if (iRet)
    {
        am_util_stdio_printf("Unable to initialize FRAM\n");
        goto _fail;
    }

    return 0;
_fail:
    return -1;
#if 0
    am_util_stdio_printf("Periodically Reading data from FRAM using non-blocking read - %d times\n", NUM_INERATIONS);
    //
    // Loop forever.
    //
    while(1)
    {
        //
        // Disable interrupt while we decide whether we're going to sleep.
        //
        uint32_t ui32IntStatus = am_hal_interrupt_master_disable();

        if (!g_bReadFram && !g_bVerifyReadData)
        {
            // Wait for Baud rate detection
            am_hal_sysctrl_sleep(true);
            //
            // Enable interrupts
            //
            am_hal_interrupt_master_set(ui32IntStatus);
        }
        else if (g_bReadFram)
        {
            //
            // Enable interrupts
            //
            am_hal_interrupt_master_set(ui32IntStatus);
            read_fram();
        }
        else if (g_bVerifyReadData)
        {
            //
            // Enable interrupts
            //
            am_hal_interrupt_master_set(ui32IntStatus);
            verify_fram_data();
            if (++numRead >= NUM_INERATIONS)
            {
                am_util_stdio_printf("\n%d Reads done\n", NUM_INERATIONS);
                break;
            }
        }
    }
    // Cleanup
    am_util_stdio_printf("\nEnd of FRAM Example\n");

    //
    // disable the interrupts in the NVIC.
    //
    NVIC_DisableIRQ(TIMER_IRQn);
    NVIC_DisableIRQ(FRAM_IOM_IRQn);
    fram_term();
    return 0;
#endif
}


