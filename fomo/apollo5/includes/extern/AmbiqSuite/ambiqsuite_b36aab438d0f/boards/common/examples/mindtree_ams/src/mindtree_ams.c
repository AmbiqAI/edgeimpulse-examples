//*****************************************************************************
//
//! @file mindtree_test.c
//!
//! @brief Test application to see if Mindtree will compile.
//!
//! Detailed description.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "stdio.h"
#include "stdarg.h"
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "FreeRTOS.h"
#include "task.h"

extern int appl_init(void);
extern void appl_profile_operations (void);

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

/* Task to be created. */
void vAMSTask( void * pvParameters )
{
    /* The parameter value is expected to be 1 as 1 is passed in the
    pvParameters value in the call to xTaskCreate() below. */
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
    appl_init();

    /** Wait-Loop - to avoid exiting from the process */
    while (1)
    {
        appl_profile_operations ();
    }
}

/* Function that creates a task. */
void vCreateAMSTask( void )
{
    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;

    /* Create the task, storing the handle. */
    xReturned = xTaskCreate(
                    vAMSTask,           /* Function that implements the task. */
                    "AMSTask",          /* Text name for the task. */
                    512,                /* Stack size in words, not bytes. */
                    ( void * ) 1,       /* Parameter passed into the task. */
                    tskIDLE_PRIORITY,   /* Priority at which the task is created. */
                    &xHandle );         /* Used to pass out the created task's handle. */

    if ( xReturned == pdPASS )
    {
        /* The task was created.  Use the task's handle to delete the task. */
        //vTaskDelete( xHandle );
    }
}

//*****************************************************************************
//
// Main
//
//*****************************************************************************
int
main(void)
{
    //
    // Default setup.
    //
    am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);
    am_hal_cachectrl_enable(&am_hal_cachectrl_defaults);
    am_bsp_low_power_init();

    //
    // Enable ITM for debug.
    //
    am_util_stdio_printf_init((am_util_stdio_print_char_t) am_bsp_itm_string_print);
    am_bsp_pin_enable(ITM_SWO);
    am_hal_itm_enable();
    am_bsp_debug_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_printf("AMS example\r\n");

    vCreateAMSTask();
    vTaskStartScheduler();

    //
    // Never reach here.
    //
    while (1)
    {
    }
}
