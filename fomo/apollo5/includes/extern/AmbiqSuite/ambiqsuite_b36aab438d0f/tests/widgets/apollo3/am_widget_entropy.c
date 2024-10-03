//*****************************************************************************
//
//! @file am_widget_entropy.h
//!
//! @brief Widgets for testing the HW entropy solution for Apollo3
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
// #include "am_bsp.h"
#include "am_util.h"

//******************************************************************************
//
// Settings.
//
//******************************************************************************
#define TOTAL_OUTPUT_BYTES   1024

//******************************************************************************
//
// Prototypes.
//
//******************************************************************************
static void entropy_complete_callback(void *context);

//******************************************************************************
//
// Globals
//
//******************************************************************************
volatile uint32_t am_widget_entropy_data_received = 0;

//******************************************************************************
//
// ISR for CTIMER.
//
//******************************************************************************
void
am_ctimer_isr(void)
{
    uint32_t ui32Status;

    //
    // Read and clear the interrupt status.
    //
    ui32Status = am_hal_ctimer_int_status_get(true);
    am_hal_ctimer_int_clear(ui32Status);

    //
    // Call the registered interrupt handlers for any interrupts we recieved.
    //
    am_hal_ctimer_int_service(ui32Status);
}

//******************************************************************************
//
// ISR for Systick
//
//******************************************************************************
void
SysTick_Handler(void)
{
    am_hal_entropy_systick_handler();
}

//******************************************************************************
//
// Callback function for entropy generator - Called when data has been transferred.
//
//******************************************************************************
static void
entropy_complete_callback(void *context)
{
    uint32_t *flag = context;
    *flag = 1;
}

//******************************************************************************
//
// Gathers a fixed number of random bytes and prints them to SWO for
// analysis.
//
//******************************************************************************
#define LINE_LENGTH 32

static uint8_t entropy_output[LINE_LENGTH];
static uint32_t entropy_length = LINE_LENGTH;

uint32_t
am_widget_entropy_get_values_test(uint32_t numBytes)
{
    // Initialize entropy hardware
    am_hal_entropy_init();
    am_hal_entropy_enable();

    am_util_stdio_printf("\n");
    am_util_stdio_printf("\n");
    am_util_stdio_printf("ENTROPY DATA START (%d bytes):\n", numBytes);

    for (uint32_t i = 0; i < numBytes; i += entropy_length)
    {
        am_widget_entropy_data_received = 0;
        am_hal_entropy_get_values(entropy_output, entropy_length, entropy_complete_callback, (void *) &am_widget_entropy_data_received);

        //
        // Note: we can't sleep here, because it would stop SysTick, which is part of the entropy generation method.
        //
        while (am_widget_entropy_data_received == 0);

        for (uint32_t j = 0; j < entropy_length; j++)
        {
            am_util_stdio_printf("%02X", entropy_output[j]);

            if (j % 32 == 31)
            {
                am_util_stdio_printf("\n");
            }
            else
            {
                am_util_stdio_printf(" ");
            }
        }
    }

    am_hal_entropy_disable();

    return 0;
}
