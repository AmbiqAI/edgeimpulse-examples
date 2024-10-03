//*****************************************************************************
//
//! @file codec_main.c
//!
//! @brief Task to handle audio codec example operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_devices_ambt53_mspi.h"
#include "am_util_debug.h"
#include "am_util_stdio.h"

#include "ipc_task.h"
#include "audio_task.h"

/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */


//*****************************************************************************
//
// Local variable.
//
//*****************************************************************************


//*****************************************************************************
//
// System initialize
//
//*****************************************************************************
static void __stack_overflow_handler(const char *thread_name)
{
    am_util_debug_printf("__stack_overflow_handler %s\n\r", thread_name);
}

static void system_setup(void)
{
    metal_set_stack_overflow_handler(__stack_overflow_handler);
}

//*****************************************************************************
//
// Perform initial setup for audio codec task.
//
//*****************************************************************************
void codec_task_setup(void)
{
    system_setup();

    ipc_task_setup();

    audio_task_setup();
}

//*****************************************************************************
//
// ambt53 MSPI Interrupt handler.
//
//*****************************************************************************
void
ambt53_mspi_isr(void)
{
    uint32_t ui32Status;

    am_hal_mspi_interrupt_status_get(pvRpmsgMspiHandle, &ui32Status, false);
    am_hal_mspi_interrupt_clear(pvRpmsgMspiHandle, ui32Status);
    am_hal_mspi_interrupt_service(pvRpmsgMspiHandle, ui32Status);
}
