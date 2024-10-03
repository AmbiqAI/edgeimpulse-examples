//*****************************************************************************
//
//! @file led_task.c
//!
//! @brief Task to handle LED operation.
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
#include "freertos_lowpower.h"

//*****************************************************************************
//
// LED task handle.
//
//*****************************************************************************
TaskHandle_t led_task_handle;
//*****************************************************************************
//
// Handle for LED-related events.
//
//*****************************************************************************
EventGroupHandle_t xLedEventHandle;


//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************
void
am_gpio_isr(void)
{
    uint64_t ui64Status;

    //
    // Read and clear the GPIO interrupt status.
    //
    ui64Status = am_hal_gpio_int_status_get(false);
    am_hal_gpio_int_clear(ui64Status);

    //
    // Call the individual pin interrupt handlers for any pin that triggered an
    // interrupt.
    //
    am_hal_gpio_int_service(ui64Status);
}

//*****************************************************************************
//
// Interrupt handler for the Buttons
//
//*****************************************************************************
void
button_handler(uint32_t buttonId)
{
    BaseType_t xHigherPriorityTaskWoken, xResult;
    //
    // Send an event to the main LED task
    //
    xHigherPriorityTaskWoken = pdFALSE;

    xResult = xEventGroupSetBitsFromISR(xLedEventHandle, (1 << buttonId),
                                        &xHigherPriorityTaskWoken);

    //
    // If the LED task is higher-priority than the context we're currently
    // running from, we should yield now and run the radio task.
    //
    if (xResult != pdFAIL)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void
button0_handler(void)
{
    uint32_t count;
    uint32_t val;

    //
    // Debounce for 20 ms.
    // We're triggered for rising edge - so we expect a consistent HIGH here
    //
    for (count = 0; count < 10; count++)
    {
        val = am_hal_gpio_input_bit_read(AM_BSP_GPIO_BUTTON0);
        if (!val)
        {
            return; // State not high...must be result of debounce
        }
        am_util_delay_ms(2);
    }

    button_handler(0);
}

void
button1_handler(void)
{
    uint32_t count;
    uint32_t val;

    //
    // Debounce for 20 ms.
    // We're triggered for rising edge - so we expect a consistent HIGH here
    //
    for (count = 0; count < 10; count++)
    {
        val = am_hal_gpio_input_bit_read(AM_BSP_GPIO_BUTTON1);
        if (!val)
        {
            return; // State not high...must be result of debounce
        }
        am_util_delay_ms(2);
    }

    button_handler(1);
}

void
button2_handler(void)
{
    uint32_t count;
    uint32_t val;

    //
    // Debounce for 20 ms.
    // We're triggered for rising edge - so we expect a consistent HIGH here
    //
    for (count = 0; count < 10; count++)
    {
        val = am_hal_gpio_input_bit_read(AM_BSP_GPIO_BUTTON2);
        if (!val)
        {
            return; // State not high...must be result of debounce
        }
        am_util_delay_ms(2);
    }

    button_handler(2);
}

//*****************************************************************************
//
// Perform initial setup for the LED task.
//
//*****************************************************************************
void
LedTaskSetup(void)
{
    am_util_debug_printf("LEDTask: setup\r\n");

    //
    // Create an event handle for our wake-up events.
    //
    xLedEventHandle = xEventGroupCreate();

    //
    // Make sure we actually allocated space for the events we need.
    //
    while (xLedEventHandle == NULL);

    // Initialize the LEDs
    am_devices_led_array_init(am_bsp_psLEDs, AM_BSP_NUM_LEDS);
    am_devices_led_off(am_bsp_psLEDs, 0);
    am_devices_led_off(am_bsp_psLEDs, 1);
    am_devices_led_off(am_bsp_psLEDs, 2);
    am_devices_led_off(am_bsp_psLEDs, 3);
    am_devices_led_off(am_bsp_psLEDs, 4);
    am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_GPIO, configMAX_SYSCALL_INTERRUPT_PRIORITY);
    //
    // Register interrupt handler for button presses
    //
    am_hal_gpio_int_register(AM_BSP_GPIO_BUTTON0, button0_handler);
    am_hal_gpio_int_register(AM_BSP_GPIO_BUTTON1, button1_handler);
    am_hal_gpio_int_register(AM_BSP_GPIO_BUTTON2, button2_handler);

    am_hal_gpio_pin_config(AM_BSP_GPIO_BUTTON0, AM_HAL_GPIO_INPUT);
    am_hal_gpio_pin_config(AM_BSP_GPIO_BUTTON1, AM_HAL_GPIO_INPUT);
    am_hal_gpio_pin_config(AM_BSP_GPIO_BUTTON2, AM_HAL_GPIO_INPUT);

    //
    // Configure the GPIO/button interrupt polarity.
    //
    am_hal_gpio_int_polarity_bit_set(AM_BSP_GPIO_BUTTON0, AM_HAL_GPIO_RISING);
    am_hal_gpio_int_polarity_bit_set(AM_BSP_GPIO_BUTTON1, AM_HAL_GPIO_RISING);
    am_hal_gpio_int_polarity_bit_set(AM_BSP_GPIO_BUTTON2, AM_HAL_GPIO_RISING);

    //
    // Clear the GPIO Interrupt (write to clear).
    //
    am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BUTTON0));
    am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BUTTON1));
    am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BUTTON2));

    //
    // Enable the GPIO/button interrupt.
    //
    am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BUTTON0));
    am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BUTTON1));
    am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BUTTON2));
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_GPIO);


    //
    // Enable interrupts to the core.
    //
    am_hal_interrupt_master_enable();

}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
LedTask(void *pvParameters)
{
    uint32_t bitSet;

    while (1)
    {
        //
        // Wait for an event to be posted to the LED Event Handle.
        //
        bitSet = xEventGroupWaitBits(xLedEventHandle, 0x7, pdTRUE,
                            pdFALSE, portMAX_DELAY);
        if (bitSet != 0)
        {
            // Button Press Event received
            // Toggle respective LED(s)
            if (bitSet & (1 << 0))
            {
                am_devices_led_toggle(am_bsp_psLEDs, 0);
            }
            if (bitSet & (1 << 1))
            {
                am_devices_led_toggle(am_bsp_psLEDs, 1);
            }
            if (bitSet & (1 << 2))
            {
                am_devices_led_toggle(am_bsp_psLEDs, 2);
            }
        }
    }
}
