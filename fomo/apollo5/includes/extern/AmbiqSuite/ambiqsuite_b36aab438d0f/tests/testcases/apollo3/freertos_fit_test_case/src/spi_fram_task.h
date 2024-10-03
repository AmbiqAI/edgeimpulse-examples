//*****************************************************************************
//
//! @file spi_fram_task.h
//!
//! @brief Functions and variables related to the spi fram task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef SPI_FRAM_TASK_H
#define SPI_FRAM_TASK_H

//*****************************************************************************
//
// spi fram task handle.
//
//*****************************************************************************
extern TaskHandle_t Spi_fram_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern int SpiFramSetup(void);
extern void SpiFramTask(void *pvParameters);

#endif // SPI_FRAM_TASK_H

