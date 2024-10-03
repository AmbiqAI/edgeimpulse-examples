//*****************************************************************************
//
//! @file tinyusb_cdc_msc_task.h
//!
//! @brief Functions and variables related to the TinyUSB task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef TINYUSB_TASK_H
#define TINYUSB_TASK_H

//*****************************************************************************
//
// TinyUSB task handle.
//
//*****************************************************************************
extern TaskHandle_t TinyUsbTaskHandle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void TinyUsbTask(void *pvParameters);

#endif // TINYUSB_TASK_H