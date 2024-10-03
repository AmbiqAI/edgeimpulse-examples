//*****************************************************************************
//
//! @file mve_task.h
//!
//! @brief Functions and variables related to the hello task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef MVE_TASK_H
#define MVE_TASK_H

#ifndef BAREMETAL
//*****************************************************************************
//
// FreeRTOS include files.
//
//*****************************************************************************
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

#endif /* BAREMETAL */

//*****************************************************************************
//
// MVE task handle.
//
//*****************************************************************************
extern TaskHandle_t mve_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void MveTaskSetup(void);
extern void MveTask(void *pvParameters);

#endif // MVE_TASK_H
