//*****************************************************************************
//
//! @file i2c_fram_task.h
//!
//! @brief Functions and variables related to the i2c fram task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef I2C_FRAM_TASK_H
#define I2C_FRAM_TASK_H

//*****************************************************************************
//
// i2c fram task handle.
//
//*****************************************************************************
extern TaskHandle_t I2c_fram_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern int I2cFramSetup(void);
extern void I2cFramTask(void *pvParameters);

#endif // I2C_FRAM_TASK_H

