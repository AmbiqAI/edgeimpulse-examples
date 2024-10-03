//*****************************************************************************
//
//! @file i2c_fram_task.c
//!
//! @brief Example that demostrates IOM, connecting to an I2C FRAM
//! FRAM is initialized with a known pattern data using Blocking IOM Write.
//! This example starts a 1 second timer. At each 1 second period, it initiates
//! reading a fixed size block from the FRAM device using Non-Blocking IOM
//! Read, and comparing againts the predefined pattern
//!
//! Define one of FRAM_DEVICE_ macros to select the FRAM device
//!
//! SWO is configured in 1M baud, 8-n-1 mode.
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
#include "am_devices_mb85rc256v.h"

//*****************************************************************************
//
// I2c Fram task handle.
//
//*****************************************************************************
TaskHandle_t I2c_fram_task_handle;

//*****************************************************************************
// Customize the following for the test
//*****************************************************************************
// Control the example execution
#define I2C_IOM_MODULE              2
#define I2C_PATTERN_BUF_SIZE        128
#define I2C_TEST_FREQ               AM_HAL_IOM_1MHZ

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
volatile bool                           g_bI2cReadFram = false;
volatile bool                           g_bVerifyI2cReadData = false;
void                                    *g_I2cDevHandle;
void                                    *g_pI2CHandle;
am_hal_iom_buffer(I2C_PATTERN_BUF_SIZE)     gI2cPatternBuf;
am_hal_iom_buffer(I2C_PATTERN_BUF_SIZE)     gI2cRxBuf;

// Buffer for non-blocking transactions
uint32_t                                I2CDMATCBBuffer[256];

//*****************************************************************************
//
// Timer configuration.
//
//*****************************************************************************
am_hal_ctimer_config_t g_sI2cTimer =
{
    // Don't link timers.
    0,

    // Set up Timer0A.
    (AM_HAL_CTIMER_FN_REPEAT |
     AM_HAL_CTIMER_INT_ENABLE    |
     AM_HAL_CTIMER_LFRC_32HZ),

    // No configuration for Timer0B.
    0,
};

am_hal_iom_config_t     g_sI2cCfg;

void
am_iomaster2_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pI2CHandle, true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pI2CHandle, ui32Status);
            am_hal_iom_interrupt_service(g_pI2CHandle, ui32Status);
        }
    }
}

//*****************************************************************************
//
// Function to initialize Timer A2 to interrupt every 1 second.
//
//*****************************************************************************
void
i2c_timer_init(void)
{
    uint32_t ui32Period;

    //
    // Enable the LFRC.
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_LFRC_START, 0);

    //
    // Set up timer A2.
    //
    am_hal_ctimer_clear(2, AM_HAL_CTIMER_TIMERA);
    am_hal_ctimer_config(2, &g_sI2cTimer);

    //
    // Set up timerA2 to 32Hz from LFRC divided to 1 second period.
    //
    ui32Period = 64;
    am_hal_ctimer_period_set(2, AM_HAL_CTIMER_TIMERA, ui32Period,
                             (ui32Period >> 1));

    //
    // Clear the timer Interrupt
    //
    am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA2);
}

void
init_i2c_pattern(void)
{
    uint32_t i;
    for (i = 0; i < I2C_PATTERN_BUF_SIZE; i++)
    {
        gI2cPatternBuf.bytes[i] = i & 0xFF;
    }
}

int
I2cFramSetup(void)
{
    uint32_t ui32Status = 1;
    uint32_t ui32DeviceId = 0;
    uint32_t ui32State = 0;

#if FIREBALL_CARD
    ui32State = AM_DEVICES_FIREBALL_STATE_I2C_IOM2;
#elif FIREBALL2_CARD
    ui32State = AM_DEVICES_FIREBALL2_STATE_I2C_IOM2;
#endif

#if FIREBALL_CARD || FIREBALL2_CARD
    uint32_t ui32Ret, ui32ID;

#if 1
    //
    // Get Fireball ID and Rev info.
    //
    ui32Ret = am_devices_fireball_control(AM_DEVICES_FIREBALL_STATE_ID_GET, &ui32ID);
    if ( ui32Ret != 0 )
    {
        am_util_stdio_printf("FAIL: am_devices_fireball_control(%d) returned 0x%X.\n",
                             AM_DEVICES_FIREBALL_STATE_ID_GET, ui32Ret);
        return -1;
    }
    else if (( ui32ID == FIREBALL_ID ) || ( ui32ID == FIREBALL2_ID ))
    {
        am_util_stdio_printf("Fireball found, ID is 0x%X.\n", ui32ID);
    }
    else
    {
        am_util_stdio_printf("Unknown device returned ID as 0x%X.\n", ui32ID);
    }

    ui32Ret = am_devices_fireball_control(AM_DEVICES_FIREBALL_STATE_VER_GET, &ui32ID);
    if ( ui32Ret != 0 )
    {
        am_util_stdio_printf("FAIL: am_devices_fireball_control(%d) returned 0x%X.\n",
                             AM_DEVICES_FIREBALL_STATE_VER_GET, ui32Ret);
        return -1;
    }
    else
    {
        am_util_stdio_printf("Fireball Version is 0x%X.\n", ui32ID);
    }
#endif

    if ( ui32State != 0 )
    {
        ui32Ret = am_devices_fireball_control(ui32State, 0);
        if ( ui32Ret != 0 )
        {
            am_util_stdio_printf("FAIL: am_devices_fireball_control(%d) returned 0x%X.\n",
                                 ui32State, ui32Ret);
            return -1;
        }
    }
#endif // FIREBALL_CARD

    am_devices_mb85rc256v_config_t stFramConfig;
    stFramConfig.ui32ClockFreq = I2C_TEST_FREQ;
    stFramConfig.pNBTxnBuf = I2CDMATCBBuffer;
    stFramConfig.ui32NBTxnBufLength = sizeof(I2CDMATCBBuffer) / 4;

    ui32Status = am_devices_mb85rc256v_init(I2C_IOM_MODULE, &stFramConfig, &g_I2cDevHandle, &g_pI2CHandle);
    if (0 == ui32Status)
    {
        ui32Status = am_devices_mb85rc256v_read_id(g_I2cDevHandle, &ui32DeviceId);

        if ((ui32Status  != 0) || (ui32DeviceId != AM_DEVICES_MB85RC64TA_ID))
        {
            return -1;
        }
        am_util_stdio_printf("I2C Device Found\n");
        init_i2c_pattern();
        // Set up a pattern data in FRAM memory
        am_util_stdio_printf("Setting up data pattern in FRAM using blocking write\n");
        return am_devices_mb85rc256v_blocking_write(g_I2cDevHandle, &gI2cPatternBuf.bytes[0], 0, I2C_PATTERN_BUF_SIZE);
    }
    else
    {
        return -1;
    }
}

void
i2c_read_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("\nFRAM Read Failed 0x%x\n", transactionStatus);
    }
    else
    {
        //am_util_stdio_printf("\nI2C Read Complete\n");
        g_bVerifyI2cReadData = true;
    }
}

void
initiate_i2c_fram_read(void)
{
    g_bI2cReadFram = true;
}

int
verify_i2c_fram_data(void)
{
    uint32_t i;
    g_bVerifyI2cReadData = false;
    // Verify Read FRAM data
    for (i = 0; i < I2C_PATTERN_BUF_SIZE; i++)
    {
        if (gI2cPatternBuf.bytes[i] != gI2cRxBuf.bytes[i])
        {
            am_util_stdio_printf("I2C Receive Data Compare failed at offset %d - Expected = 0x%x, Received = 0x%x\n",
                i, gI2cPatternBuf.bytes[i], gI2cRxBuf.bytes[i]);
            return -1;
        }
    }
    return 0;
}

void
i2c_read_fram(void)
{
    uint32_t ui32Status;
    // Initiate read of a block of data from FRAM
    ui32Status = am_devices_mb85rc256v_nonblocking_read(g_I2cDevHandle, &gI2cRxBuf.bytes[0], 0, I2C_PATTERN_BUF_SIZE, i2c_read_complete, 0);
    if (ui32Status == 0)
    {
        g_bI2cReadFram = false;
    }
}

void
i2c_fram_term(void)
{
    am_devices_mb85rc256v_term(g_I2cDevHandle);
}

void i2c_fram_task_init(void)
{
    //
    // TimerA2 init.
    //
    i2c_timer_init();

    //
    // Enable the timer Interrupt.
    //
    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA2);

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_EnableIRQ(CTIMER_IRQn);

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_EnableIRQ((IRQn_Type)(IOMSTR2_IRQn));

    am_hal_interrupt_master_enable();

    // Set up the periodic FRAM Read
    am_hal_ctimer_int_register(AM_HAL_CTIMER_INT_TIMERA2, initiate_i2c_fram_read);

    //
    // Start timer A2
    //
    am_hal_ctimer_start(2, AM_HAL_CTIMER_TIMERA);
}

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
void I2cFramTask(void *pvParameters)
{
    const TickType_t xDelay = 50 / portTICK_PERIOD_MS;

    am_util_stdio_printf("I2C FRAM Task\r\n");

    i2c_fram_task_init();

    //
    // Loop forever.
    //
    while(1)
    {
        if (!g_bI2cReadFram && !g_bVerifyI2cReadData)
        {
            // Wait for Baud rate detection
            am_hal_sysctrl_sleep(true);
        }
        else if (g_bI2cReadFram)
        {
            i2c_read_fram();
        }
        else if (g_bVerifyI2cReadData)
        {
            if (0 == verify_i2c_fram_data())
            {
              AM_UPDATE_TEST_PROGRESS();
              am_util_stdio_printf("I2C\n");
            }
        }
        vTaskDelay(xDelay);
    }
}

