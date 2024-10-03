//*****************************************************************************
//
//! @file sdio_rs9116_wifi_interface_task.h
//!
//! @brief Functions and variables related to the emmc task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef SDIO_RS9116_WIFI_INTERFACE_TASK_H
#define SDIO_RS9116_WIFI_INTERFACE_TASK_H

//*****************************************************************************
//
// sdio rs9116 wifi interface task handle.
//
//*****************************************************************************
extern TaskHandle_t sdio_task_handle;
extern SemaphoreHandle_t g_semSdioWrite;
extern SemaphoreHandle_t g_semSdioRead;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void Sdio_RS116_WiFi_interface_Task(void *pvParameters);


#endif // SDIO_SDIO_RS9116_WIFI_INTERFACE_TASK_H