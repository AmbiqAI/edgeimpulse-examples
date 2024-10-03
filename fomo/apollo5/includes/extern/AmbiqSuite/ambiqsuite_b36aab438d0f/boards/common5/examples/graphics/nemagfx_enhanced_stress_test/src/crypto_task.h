//*****************************************************************************
//
//! @file crypto_task.h
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef _CRYPTO_TASK_H
#define _CRYPTO_TASK_H

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

extern void crypto_task(void *pvParameters);
extern TaskHandle_t CryptoTaskHandle;

#endif

