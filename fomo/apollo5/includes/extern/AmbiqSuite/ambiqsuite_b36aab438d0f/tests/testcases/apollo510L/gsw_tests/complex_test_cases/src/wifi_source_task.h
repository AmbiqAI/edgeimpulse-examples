//*****************************************************************************
//
//! @file file_writer_task.h
//!
//! @brief Functions and variables related to the file writer task
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef WIFI_SOURCE_TASK_H
#define WIFI_SOURCE_TASK_H

#include "common_emmc_fatfs.h"
#include "FreeRTOS.h"

// from sdio_test_common.h
#define ALIGN(x) __attribute__((aligned(1<<x)))

//*****************************************************************************
//
// WiFi Source task handle.
//
//*****************************************************************************
extern TaskHandle_t wifi_source_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

extern void WifiSourceTaskSetup(void);
extern void WifiSourceTask(void *pvParameters);

#endif // WIFI_SOURCE_TASK_H
