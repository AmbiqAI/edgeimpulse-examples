//*****************************************************************************
//
//! @file spi_fram_task.c
//!
//! @brief Example that demostrates IOM, connecting to a SPI FRAM
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
#if FIREBALL2_CARD
#include "am_devices_mb85rq4ml.h"
#define am_spi_devices_t am_devices_mb85rq4ml_config_t
#else
#include "am_devices_mb85rs1mt.h"
#define am_spi_devices_t am_devices_mb85rs1mt_config_t
#endif

//
// Typedef - to encapsulate device driver functions
//
typedef struct
{
    uint8_t  devName[20];
    uint32_t (*fram_init)(uint32_t ui32Module, am_spi_devices_t *pDevConfig, void **ppHandle, void **ppIomHandle);
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
#if FIREBALL_CARD || FIREBALL2_CARD
    am_devices_fireball_control_e fram_fireball_control;
#endif
} fram_device_func_t;

//*****************************************************************************
//
// Iom Fram task handle.
//
//*****************************************************************************
TaskHandle_t Spi_fram_task_handle;

//*****************************************************************************
// Customize the following for the test
//*****************************************************************************
// Control the example execution
#define SPI_IOM_MODULE              0
#define SPI_PATTERN_BUF_SIZE        128
#define SPI_TEST_FREQ               AM_HAL_IOM_1MHZ
//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
volatile bool                           g_bSpiReadFram = false;
volatile bool                           g_bVerifySpiReadData = false;
void                                    *g_SpiDevHandle;
void                                    *g_pSPIHandle;
am_hal_iom_buffer(SPI_PATTERN_BUF_SIZE)     gSpiPatternBuf;
am_hal_iom_buffer(SPI_PATTERN_BUF_SIZE)     gSpiRxBuf;

// Buffer for non-blocking transactions
uint32_t                                SPIDMATCBBuffer[256];
fram_device_func_t spi_device_func =
{
#if FIREBALL2_CARD
    // Fireball installed SPI FRAM device
    .devName = "SPI FRAM MB85RQ4ML",
    .fram_init = am_devices_mb85rq4ml_init,
    .fram_term = am_devices_mb85rq4ml_term,
    .fram_read_id = am_devices_mb85rq4ml_read_id,
    .fram_blocking_write = am_devices_mb85rq4ml_blocking_write,
    .fram_nonblocking_write = am_devices_mb85rq4ml_nonblocking_write,
    .fram_blocking_read = am_devices_mb85rq4ml_blocking_read,
    .fram_nonblocking_read = am_devices_mb85rq4ml_nonblocking_read,
    .fram_fireball_control = AM_DEVICES_FIREBALL2_STATE_SPI_FRAM_PSRAM_1P8,
#else
    // Fireball installed SPI FRAM device
    .devName = "SPI FRAM MB85RS1MT",
    .fram_init = am_devices_mb85rs1mt_init,
    .fram_term = am_devices_mb85rs1mt_term,
    .fram_read_id = am_devices_mb85rs1mt_read_id,
    .fram_blocking_write = am_devices_mb85rs1mt_blocking_write,
    .fram_nonblocking_write = am_devices_mb85rs1mt_nonblocking_write,
    .fram_blocking_read = am_devices_mb85rs1mt_blocking_read,
    .fram_nonblocking_read = am_devices_mb85rs1mt_nonblocking_read,
#if FIREBALL_CARD
    .fram_fireball_control = AM_DEVICES_FIREBALL_STATE_SPI_FRAM,
#endif
#endif
};

//*****************************************************************************
//
// Timer configuration.
//
//*****************************************************************************
am_hal_ctimer_config_t g_sSpiTimer =
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

void
am_iomaster0_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pSPIHandle, true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pSPIHandle, ui32Status);
            am_hal_iom_interrupt_service(g_pSPIHandle, ui32Status);
        }
    }
}

//*****************************************************************************
//
// Function to initialize Timer A3 to interrupt every 1 second.
//
//*****************************************************************************
void
spi_timer_init(void)
{
    uint32_t ui32Period;

    //
    // Enable the LFRC.
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_LFRC_START, 0);

    //
    // Set up timer A3.
    //
    am_hal_ctimer_clear(3, AM_HAL_CTIMER_TIMERA);
    am_hal_ctimer_config(3, &g_sSpiTimer);

    //
    // Set up timerA3 to 32Hz from LFRC divided to 1 second period.
    //
    ui32Period = 64;
    am_hal_ctimer_period_set(3, AM_HAL_CTIMER_TIMERA, ui32Period,
                             (ui32Period >> 1));

    //
    // Clear the timer Interrupt
    //
    am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA3);
}

void
init_spi_pattern(void)
{
    uint32_t i;
    for (i = 0; i < SPI_PATTERN_BUF_SIZE; i++)
    {
        gSpiPatternBuf.bytes[i] = i & 0xFF;
    }
}

int
SpiFramSetup(void)
{
    uint32_t ui32Status = 1;
    uint32_t ui32DeviceId = 0;

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

    if ( spi_device_func.fram_fireball_control != 0 )
    {
        ui32Ret = am_devices_fireball_control(spi_device_func.fram_fireball_control, 0);
        if ( ui32Ret != 0 )
        {
            am_util_stdio_printf("FAIL: am_devices_fireball_control(%d) returned 0x%X.\n",
                                 spi_device_func.fram_fireball_control, ui32Ret);
            return -1;
        }
    }
#endif // FIREBALL_CARD

    am_spi_devices_t stFramConfig;
    stFramConfig.ui32ClockFreq = SPI_TEST_FREQ;
    stFramConfig.pNBTxnBuf = SPIDMATCBBuffer;
    stFramConfig.ui32NBTxnBufLength = sizeof(SPIDMATCBBuffer) / 4;

    ui32Status = spi_device_func.fram_init(SPI_IOM_MODULE, &stFramConfig, &g_SpiDevHandle, &g_pSPIHandle);
    if (0 == ui32Status)
    {
        ui32Status = spi_device_func.fram_read_id(g_SpiDevHandle, &ui32DeviceId);
#if FIREBALL2_CARD
        if ((ui32Status  != 0) || (ui32DeviceId != AM_DEVICES_MB85RQ4ML_ID))
#else
        if ((ui32Status  != 0) || (ui32DeviceId != AM_DEVICES_MB85RS1MT_ID))
#endif
        {
            return -1;
        }
        am_util_stdio_printf("%s Found\n", spi_device_func.devName);
        init_spi_pattern();
        // Set up a pattern data in FRAM memory
        am_util_stdio_printf("Setting up data pattern in FRAM using blocking write\n");
        return spi_device_func.fram_blocking_write(g_SpiDevHandle, &gSpiPatternBuf.bytes[0], 0, SPI_PATTERN_BUF_SIZE);
    }
    else
    {
        return -1;
    }
}

void
spi_read_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("\nFRAM Read Failed 0x%x\n", transactionStatus);
    }
    else
    {
        //am_util_stdio_printf("\nSPI Read Complete\n");
        g_bVerifySpiReadData = true;
    }
}

void
initiate_spi_fram_read(void)
{
    g_bSpiReadFram = true;
}

int
verify_spi_fram_data(void)
{
    uint32_t i;
    g_bVerifySpiReadData = false;
    // Verify Read FRAM data
    for (i = 0; i < SPI_PATTERN_BUF_SIZE; i++)
    {
        if (gSpiPatternBuf.bytes[i] != gSpiRxBuf.bytes[i])
        {
            am_util_stdio_printf("SPI Receive Data Compare failed at offset %d - Expected = 0x%x, Received = 0x%x\n",
                i, gSpiPatternBuf.bytes[i], gSpiRxBuf.bytes[i]);
            return -1;
        }
    }
    return 0;
}

void
spi_read_fram(void)
{
    uint32_t ui32Status;
    // Initiate read of a block of data from FRAM
    ui32Status = spi_device_func.fram_nonblocking_read(g_SpiDevHandle, &gSpiRxBuf.bytes[0], 0, SPI_PATTERN_BUF_SIZE, spi_read_complete, 0);
    if (ui32Status == 0)
    {
        g_bSpiReadFram = false;
    }
}

void
spi_fram_term(void)
{
    spi_device_func.fram_term(g_SpiDevHandle);
}

void spi_fram_task_init(void)
{
    //
    // TimerA3 init.
    //
    spi_timer_init();

    //
    // Enable the timer Interrupt.
    //
    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA3);

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_EnableIRQ(CTIMER_IRQn);

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_EnableIRQ((IRQn_Type)(IOMSTR0_IRQn));

    am_hal_interrupt_master_enable();

    // Set up the periodic FRAM Read
    am_hal_ctimer_int_register(AM_HAL_CTIMER_INT_TIMERA3, initiate_spi_fram_read);

    //
    // Start timer A3
    //
    am_hal_ctimer_start(3, AM_HAL_CTIMER_TIMERA);
}

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
void SpiFramTask(void *pvParameters)
{
    const TickType_t xDelay = 50 / portTICK_PERIOD_MS;

    uint32_t    ui32Status;
    
    am_util_stdio_printf("SPI FRAM Task\r\n");

    spi_fram_task_init();

    //
    // Loop forever.
    //
    while(1)
    {
        if (!g_bSpiReadFram && !g_bVerifySpiReadData)
        {
          // Wait for Baud rate detection
          vTaskDelay(xDelay);
        }
        else if (g_bSpiReadFram)
        {
            spi_read_fram();
        }
        else if (g_bVerifySpiReadData)
        {
            ui32Status = verify_spi_fram_data();
            if (0 == ui32Status)
            {
              am_util_stdio_printf("SPI\n");
            }
            //am_util_stdio_printf("\nSpi test restart\n");
        }
        vTaskDelay(xDelay);
    }
}

