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
        am_hal_gpio_state_read(AM_BSP_GPIO_BUTTON0,  AM_HAL_GPIO_INPUT_READ, &val);
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
        am_hal_gpio_state_read(AM_BSP_GPIO_BUTTON1,  AM_HAL_GPIO_INPUT_READ, &val);
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
        am_hal_gpio_state_read(AM_BSP_GPIO_BUTTON2,  AM_HAL_GPIO_INPUT_READ, &val);
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
    NVIC_SetPriority(GPIO_IRQn, NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY);
    //
    // Register interrupt handler for button presses
    //
    am_hal_gpio_interrupt_register(AM_BSP_GPIO_BUTTON0, button0_handler);
    am_hal_gpio_interrupt_register(AM_BSP_GPIO_BUTTON1, button1_handler);
    am_hal_gpio_interrupt_register(AM_BSP_GPIO_BUTTON2, button2_handler);

    am_hal_gpio_pinconfig(AM_BSP_GPIO_BUTTON0, g_AM_BSP_GPIO_BUTTON0);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_BUTTON1, g_AM_BSP_GPIO_BUTTON1);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_BUTTON2, g_AM_BSP_GPIO_BUTTON2);

    //
    // Clear the GPIO Interrupt (write to clear).
    //
    AM_HAL_GPIO_MASKCREATE(GpioIntMask0);
    AM_HAL_GPIO_MASKCREATE(GpioIntMask1);
    AM_HAL_GPIO_MASKCREATE(GpioIntMask2);
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask0, AM_BSP_GPIO_BUTTON0));
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask1, AM_BSP_GPIO_BUTTON1));
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask2, AM_BSP_GPIO_BUTTON2));

    //
    // Enable the GPIO/button interrupt.
    //
    am_hal_gpio_interrupt_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask0, AM_BSP_GPIO_BUTTON0));
    am_hal_gpio_interrupt_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask1, AM_BSP_GPIO_BUTTON1));
    am_hal_gpio_interrupt_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask2, AM_BSP_GPIO_BUTTON2));
    NVIC_EnableIRQ(GPIO_IRQn);

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

