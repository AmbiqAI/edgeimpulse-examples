//*****************************************************************************
//
//! @file gps_task.h
//!
//! @brief Functions and variables related to the SPIFRAM task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef GPS_TASK_H
#define GPS_TASK_H

//*****************************************************************************
//
// SPIFRAM task handle.
//
//*****************************************************************************
extern TaskHandle_t GPSTaskHandle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void GPSTaskSetup(void);
extern void GPSTask(void *pvParameters);

#endif // GPS_TASK_H
