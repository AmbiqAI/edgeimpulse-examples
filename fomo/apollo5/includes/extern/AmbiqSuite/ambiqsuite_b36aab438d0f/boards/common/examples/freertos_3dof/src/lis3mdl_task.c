//*****************************************************************************
//
//! @file lis3mdl_task.c
//!
//! @brief Task to handle radio operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "freertos_3dof.h"

//*****************************************************************************
//
// Forward declarations
//
//*****************************************************************************
void lis3mdl_drdy_handler(void);
void data_handler(void);
void configure_lis3mdl_pins(void);
void LIS3MDLTaskSetup(void);
void LIS3MDLTask(void *pvParameters);

//*****************************************************************************
//
// Task handle
//
//*****************************************************************************
TaskHandle_t lis3mdl_task_handle;

//*****************************************************************************
//
// Event handle
//
//*****************************************************************************
EventGroupHandle_t xLIS3MDLEventHandle;

//*****************************************************************************
//
// Device structure for the LIS3MDL
//
//*****************************************************************************
am_devices_lis3mdl_t g_sMAG =
{
    .ui32IOMModule = 1,
    .ui32ChipSelect = 5,
};

//*****************************************************************************
//
// Data buffer for the most recent magnetometer sample.
// Note: The lis3mdl does not have an internal fifo so setting a sample
// size larger than one has no effect on sampling.
//
//*****************************************************************************
#define LIS3MDL_SAMPLE_SIZE                     1
am_devices_lis3mdl_sample(LIS3MDL_SAMPLE_SIZE)  g_sMagData;

//*****************************************************************************
//
// Interrupt handler for the CTS pin
//
//*****************************************************************************
void
lis3mdl_drdy_handler(void)
{
    BaseType_t xHigherPriorityTaskWoken, xResult;

    //
    // Send an event to the main radio task
    //
    xHigherPriorityTaskWoken = pdFALSE;

    xResult = xEventGroupSetBitsFromISR(xLIS3MDLEventHandle, 1,
                                        &xHigherPriorityTaskWoken);

    //
    // If the radio task is higher-priority than the context we're currently
    // running from, we should yield now and run the radio task.
    //
    if (xResult != pdFAIL)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

//*****************************************************************************
//
// Function to handle incoming data from the magnetometer.
//
//*****************************************************************************
void
data_handler(void)
{
    //
    // Plot the data.
    //
    am_util_plot_int(AM_UTIL_PLOT_0, g_sMagData.samples[0]);
    am_util_plot_int(AM_UTIL_PLOT_1, g_sMagData.samples[1]);
    am_util_plot_int(AM_UTIL_PLOT_2, g_sMagData.samples[2]);
}

//*****************************************************************************
//
// Configure GPIOs for communicating with the ADXL
//
//*****************************************************************************
void
configure_lis3mdl_pins(void)
{
    //
    // Enable the chip-select and data-ready pins for the LIS3MDL
    //
    am_bsp_pin_enable(LIS3MDL_CS);
    am_bsp_pin_enable(LIS3MDL_DRDY);

    //
    // Setup ITM pin for plotting
    //
    am_bsp_pin_enable(ITM_SWO);

    //
    // Enable a GPIO interrupt for positive edges on the DRDY pin.
    //
    am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(AM_BSP_GPIO_LIS3MDL_DRDY));
    am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(AM_BSP_GPIO_LIS3MDL_DRDY));
}

//*****************************************************************************
//
// Perform initial setup for the radio task.
//
//*****************************************************************************
void
LIS3MDLTaskSetup(void)
{
    am_util_debug_printf("LIS3MDL: setup\r\n");

    //
    // Create an event handle for our wake-up events.  Make sure we actually
    // allocated space for the events we need.
    //
    xLIS3MDLEventHandle = xEventGroupCreate();
    while (xLIS3MDLEventHandle == NULL);

    //
    // Configure the GPIOs to work with the LIS3MDL.
    //
    configure_lis3mdl_pins();

    //
    // Enable a GPIO interrupt for positive edges on the DRDY pin, and connect
    // it to our handler.
    //
    am_hal_gpio_int_register(AM_BSP_GPIO_LIS3MDL_DRDY, lis3mdl_drdy_handler);
    am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(AM_BSP_GPIO_LIS3MDL_DRDY));
    am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(AM_BSP_GPIO_LIS3MDL_DRDY));

    //
    // Initialize the LIS3MDL driver.
    //
    am_devices_lis3mdl_config(&g_sMAG);
}

//*****************************************************************************
//
// Periodic tasks for the LIS3MDL
//
//*****************************************************************************
void
LIS3MDLTask(void *pvParameters)
{
    //
    // Check to see if the WSF routines are ready to go to sleep.
    //
    while(1)
    {
        //
        // Wait for an event to be posted to the Radio Event Handle.
        //
        xEventGroupWaitBits(xLIS3MDLEventHandle, 1, pdTRUE,
                            pdFALSE, portMAX_DELAY);

        //
        // Start a SPI read command to retrieve the samples from the
        // magnetometer.
        //
        am_devices_lis3mdl_sample_get(&g_sMAG, g_sMagData.words, data_handler);
    }
}

