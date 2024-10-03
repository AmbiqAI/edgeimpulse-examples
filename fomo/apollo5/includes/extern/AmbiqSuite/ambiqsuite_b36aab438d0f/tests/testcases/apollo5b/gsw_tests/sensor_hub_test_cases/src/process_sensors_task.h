//*****************************************************************************
//
//! @file process_sensors_task.h
//!
//! @brief Functions and variables related to the Process Sensors task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef PROCESS_SENSORS_TASK_H
#define PROCESS_SENSORS_TASK_H

//*****************************************************************************
//
// Process Sensors task handle.
//
//*****************************************************************************
extern TaskHandle_t ProcessSensorsTaskHandle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void ProcessSensorsTaskSetup(void);
extern void ProcessSensorsTask(void *pvParameters);

#endif // PROCESS_SENSORS_TASK_H
