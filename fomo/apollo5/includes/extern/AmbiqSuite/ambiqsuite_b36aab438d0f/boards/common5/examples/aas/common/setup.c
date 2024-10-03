//*****************************************************************************
//
//! @file setup.c
//!
//! @brief Initialize system and all subtasks.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "libmetal/lib/thread.h"

#include "am_util_debug.h"
#include "am_util_stdio.h"

#ifdef AAS_USB_AUDIO_TOOL
#include "usb_task.h"
#endif

#include "audio_task.h"
#include "user_task.h"

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
// Perform initial setup for system and all subtasks.
//
//*****************************************************************************
void aust_task_setup(void)
{
    system_setup();

    audio_task_setup();

#ifdef AAS_USB_AUDIO_TOOL
    usb_task_setup();
#endif

    user_task_setup();
}
