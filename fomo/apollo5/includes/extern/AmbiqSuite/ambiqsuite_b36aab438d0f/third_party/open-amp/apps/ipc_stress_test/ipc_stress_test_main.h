//*****************************************************************************
//
//! @file ipc_stress_test_main.h
//!
//! @brief Functions and variables related to IPC stress test operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef IPC_STRESS_TEST_MAIN_H
#define IPC_STRESS_TEST_MAIN_H

#include "ipc_service.h"
#include "rpmsg_virtio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

#define ENABLE_GRAPHIC    1
#if (ENABLE_GRAPHIC)
#include "graphic_task.h"
#endif


//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
void IPCSendTask(void *pvParameters);
void IPCPingPongTask(void *pvParameters);

int ipc_stress_test_setup(struct ipc_device *instance);


#endif // IPC_STRESS_TEST_MAIN_H
