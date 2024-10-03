//*****************************************************************************
//
//! @file mspi_display.c
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "mspi_mspi_display.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
volatile bool         g_bDisplayDone = false;
volatile bool         g_bTEInt = false;
void                  *g_MSPIDisplayHandle;
void                  *g_DisplayHandle;

//*****************************************************************************
//
// Local Variables
//
//*****************************************************************************
// Buffer for non-blocking transactions for Display MSPI - Needs to be big enough to accomodate
// all the transactions
//static uint32_t        g_MspiDisplayQBuffer[(AM_HAL_MSPI_CQ_ENTRY_SIZE / 4) * (NUM_FRAGMENTS + 1)];
static uint32_t        g_MspiDisplayQBuffer[(AM_HAL_MSPI_CQ_ENTRY_SIZE / 4) * 12];


// Display MSPI configuration
am_devices_mspi_rm69330_config_t QuadDisplayMSPICfg =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4,
    .eClockFreq               = AM_HAL_MSPI_CLK_48MHZ,
    .ui32NBTxnBufLength       = sizeof(g_MspiDisplayQBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = g_MspiDisplayQBuffer,
};

//! MSPI interrupts.
static const IRQn_Type mspi_display_interrupts[] =
{
    MSPI0_IRQn,
#if defined(AM_PART_APOLLO3P)
    MSPI1_IRQn,
    MSPI2_IRQn,
#endif
};

//
// Take over the interrupt handler for whichever MSPI we're using.
//
#define display_mspi_isr                                                          \
    am_mspi_isr1(DISPLAY_MSPI_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi ## n ## _isr

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void display_mspi_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_MSPIDisplayHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_MSPIDisplayHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_MSPIDisplayHandle, ui32Status);
}

//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************
void am_gpio_isr(void)
{
    //
    // Read and clear the GPIO interrupt status.
    //
#if defined(AM_PART_APOLLO3P)
    AM_HAL_GPIO_MASKCREATE(GpioIntStatusMask);

    am_hal_gpio_interrupt_status_get(false, pGpioIntStatusMask);
    am_hal_gpio_interrupt_clear(pGpioIntStatusMask);
    am_hal_gpio_interrupt_service(pGpioIntStatusMask);
#elif defined(AM_PART_APOLLO3)
    uint64_t ui64Status;

    am_hal_gpio_interrupt_status_get(false, &ui64Status);
    am_hal_gpio_interrupt_clear(ui64Status);
    am_hal_gpio_interrupt_service(ui64Status);
#else
#error Unknown device.
#endif
}
// TE ISR handler
static void
teInt_handler(void)
{
    static uint32_t count = 0;

    if (g_bDisplayDone && g_bNewDisplay)
    {
        if (++count == TE_DELAY)
        {
            g_bTEInt = true;
            count = 0;
        }
    }
}

// Initialization
uint32_t
display_init(void)
{
    uint32_t ui32Status;

    NVIC_SetPriority(DISPLAY_MSPI_IRQn, 0x4);

    //
    // Initialize the display specific GPIO signals.
// #### INTERNAL BEGIN ####
    // FIX ME - This needs to be ported to the official Apollo3/3P + Cygnus BSPs when ready.
// #### INTERNAL END ####
    //
    am_hal_gpio_pinconfig(AM_BSP_GPIO_DSPL_RESET, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_output_set(AM_BSP_GPIO_DSPL_RESET);

    am_hal_gpio_pinconfig(AM_BSP_GPIO_DSPL_TE, g_AM_BSP_GPIO_DSPL_TE);

    am_hal_gpio_pinconfig(AM_BSP_GPIO_DSPL0_OLED_EN, g_AM_HAL_GPIO_INPUT);

    am_hal_gpio_pinconfig(AM_BSP_GPIO_DSPL0_OLED_PWER_EN, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_output_clear(AM_BSP_GPIO_DSPL0_OLED_PWER_EN);

    am_hal_gpio_pinconfig(AM_BSP_GPIO_DSPL0_VIO_EN, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_output_set(AM_BSP_GPIO_DSPL0_VIO_EN);

    am_hal_gpio_pinconfig(AM_BSP_GPIO_DSPL0_DSPL_3V3_EN, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_output_set(AM_BSP_GPIO_DSPL0_DSPL_3V3_EN);

// #### INTERNAL BEGIN ####
    //
    // FIXME!!!
    //
// #### INTERNAL END ####
    // Initialize the MSPI Display
    ui32Status = am_devices_mspi_rm69330_init(DISPLAY_MSPI_MODULE, &QuadDisplayMSPICfg, &g_DisplayHandle, &g_MSPIDisplayHandle);
    if (AM_DEVICES_MSPI_RM69330_STATUS_SUCCESS != ui32Status)
    {
        DEBUG_PRINT("Failed to init Display device\n");
    }
    NVIC_EnableIRQ(mspi_display_interrupts[DISPLAY_MSPI_MODULE]);

    AM_HAL_GPIO_MASKCREATE(GpioIntMask);
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask, AM_BSP_GPIO_DSPL_TE));
    am_hal_gpio_interrupt_register(AM_BSP_GPIO_DSPL_TE, teInt_handler);
    am_hal_gpio_interrupt_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask, AM_BSP_GPIO_DSPL_TE));

    NVIC_EnableIRQ(GPIO_IRQn);

    am_devices_mspi_rm69330_display_on(g_DisplayHandle);
    return ui32Status;
}

// Callbacks
static void
display_write_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    DEBUG_GPIO_LOW(DBG1_GPIO);

//    DEBUG_GPIO_HIGH(DBG2_GPIO);
    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        DEBUG_PRINT("\nMSPI Display Write Failed 0x%x\n", transactionStatus);
    }
    else
    {
        DEBUG_PRINT("\nMSPI Display Write Done 0x%x\n", transactionStatus);
        g_bDisplayDone = true;
//    DEBUG_GPIO_LOW(DBG2_GPIO);
    }
}
// Initialization
uint32_t
init_mspi_mspi_xfer(void)
{
    uint32_t ui32Status = 0;

    ui32Status = am_devices_mspi_rm69330_set_transfer_window(g_DisplayHandle, 0, COLUMN_NUM, 0, ROW_NUM);
    if (ui32Status)
    {
        DEBUG_PRINT("\nFailed to set transfer window\n");
        return ui32Status;
    }

    return ui32Status;
}

// Rendering
uint32_t
start_mspi_xfer(uint32_t address, uint32_t ui32NumBytes)
{
    uint32_t      ui32Status = 0;
    am_hal_mspi_callback_t  mspiSinkCb = 0;

    mspiSinkCb = display_write_complete;
    DEBUG_PRINT("\nInitiating MSPI Display Transfer\n");
    DEBUG_GPIO_HIGH(DBG1_GPIO);
    ui32Status = am_devices_mspi_rm69330_nonblocking_write_adv(g_DisplayHandle,
                 (uint8_t *)address,
                 ui32NumBytes,
                 0,
                 0,
                 mspiSinkCb,
                 0);
    if (ui32Status)
    {
        DEBUG_PRINT("\nFailed to queue up MSPI Display Write transaction\n");
    }
    return ui32Status;
}
