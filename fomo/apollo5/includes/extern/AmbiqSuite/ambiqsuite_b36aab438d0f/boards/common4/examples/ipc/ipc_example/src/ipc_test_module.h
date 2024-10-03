//*****************************************************************************
//
//! @file ipc_test_module.h
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

#ifndef IPC_TEST_MODULE_H
#define IPC_TEST_MODULE_H

#include "ipc_service.h"
#include "rpmsg_virtio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

// Generate random number
#define     random_num(max, min)   ((rand() % (max - min + 1)) + min)

// ipc_test_ep_e definition is referred in endpoint's timer id and ipc event
// Thus update on this table should be careful.
typedef enum
{
    INST0_EP0,
    INST0_EP1,
    INST0_EP_NUM
} ipc_test_ep_e;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
uint32_t ipc_send_data_event_handler(uint32_t ept_to_send);

int ipc_test_setup(struct ipc_device *instance);

void ipc_set_send_data_event(void);
#endif // IPC_TEST_MODULE_H
