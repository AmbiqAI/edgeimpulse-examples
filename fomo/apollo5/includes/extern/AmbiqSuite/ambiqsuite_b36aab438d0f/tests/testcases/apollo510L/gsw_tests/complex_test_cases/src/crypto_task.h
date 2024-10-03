//*****************************************************************************
//
//! @file crypto_task.h
//!
//! @brief Functions and variables related to the crypto task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef CRYPTO_TASK_H
#define CRYPTO_TASK_H

//*****************************************************************************
//
// Crypto task handles.
//
//*****************************************************************************
extern TaskHandle_t crypto_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void CryptoTaskSetup(void);
extern void CryptoTask(void *pvParameters);

#endif // CRYPTO_TASKS_H
