//*****************************************************************************
//
//! @file timer_task.c
//!
//! @brief Simple task with a while() loop that waits for a semaphore from the 
//!     timer ISR.  After TIMER_TASK_COUNT iterations (set in
//!     src/freertos_sample_test_cases.h), the task breaks out of the while(1)
//!     loop, sets bTimerTaskComplete to "true", and suspends/deletes itself.
//!     The bTimerTaskComplete variable is monitored by the ApplicationIdleTask.
//!     
//!     Timer clock source is LFRC_DIV2, which is a frequency of 1000Hz/2 = 500Hz.
//!     The timer is configured in continuous upcount mode with a compare value 
//!     of 1000, thus the Timer ISR should fire every 2 seconds. In other words,
//!     timer_isr_freq = (1/timer_freq)*cycles = (1/500)*1000 = 2 seconds
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "freertos_sample_test_cases.h"
#include "timer_task.h"

//*****************************************************************************
//
// Timer task handle.
//
//*****************************************************************************
TaskHandle_t timer_task_handle;

//*****************************************************************************
//
// Global variables and templates 
//
//*****************************************************************************
volatile bool g_bTimerTaskComplete = false;
SemaphoreHandle_t g_xTimerIsrSemaphore = NULL;

//*****************************************************************************
//
// Interrupt handler for the CTIMER module.
//
//*****************************************************************************
void
am_ctimer_isr(void)
{
    uint32_t ui32Status;

    am_hal_gpio_output_toggle(GPIO_TIMER_ISR);

    am_hal_timer_interrupt_status_get(false, &ui32Status);
    am_hal_timer_interrupt_clear(ui32Status);

    if (g_xTimerIsrSemaphore != NULL)
    {   
        // Unblock the timer task by releasing the semaphore
        xSemaphoreGiveFromISR(g_xTimerIsrSemaphore, NULL); 
    }
}

//*****************************************************************************
//
// Perform initial setup for the timer task.
//
//*****************************************************************************
void
TimerTaskSetup(void)
{
    am_util_stdio_printf("Timer task: setup\r\n");
}

//*****************************************************************************
//
// TimerTask - will delete itself after TIMER_TASK_COUNT loops complete 
//
//*****************************************************************************

void
TimerTask(void *pvParameters)
{
    uint32_t i = 0;
    g_xTimerIsrSemaphore = xSemaphoreCreateBinary();

    //
    // Initialize timer
    //
    am_hal_timer_config_t       TimerConfig;
    am_hal_timer_default_config_set(&TimerConfig);
    TimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_LFRC;
    TimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    TimerConfig.ui32PatternLimit = 0;
    TimerConfig.ui32Compare0 = 1000;

    //
    // Configure the TIMER.
    //
    am_hal_timer_config(0, &TimerConfig);

    //
    // Clear the TIMER.
    //
    am_hal_timer_clear_stop(0);

    //
    // Enabled the Interrupt.
    //
    am_hal_timer_interrupt_enable(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE0));
    NVIC_SetPriority(TIMER_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(TIMER_IRQn);
    am_hal_interrupt_master_enable();

    //
    // Enable the TIMER.
    //
    am_hal_timer_enable(0);

    //
    // Start the TIMER.
    //
    am_hal_timer_start(0);
    
    am_util_stdio_printf("\r\nRunning TimerTask\r\n");

    while (i < TIMER_TASK_COUNT)
    {
        // Wait for Semaphore from ISR
        xSemaphoreTake(g_xTimerIsrSemaphore, portMAX_DELAY);

        // Print out number of iterations
        am_util_stdio_printf("\r\nTimerTask Iteration: %d\r\n", i++);

    }
    // Indicate the HelloTask loops are complete
    g_bTimerTaskComplete = true;
    // Suspend and delete the task
    vTaskDelete(NULL);
}
