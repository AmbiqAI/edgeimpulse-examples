//*****************************************************************************
//
//! @file rtos.h
//!
//! @brief Essential functions to make the RTOS run
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef RTOS_H
#define RTOS_H

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern SemaphoreHandle_t g_semDisplayStart;
extern SemaphoreHandle_t g_semDisplayEnd;
extern SemaphoreHandle_t g_semEventStart;
extern SemaphoreHandle_t g_semEventEnd;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern void run_tasks(void);

#ifdef __cplusplus
}
#endif

#endif // RTOS_H

