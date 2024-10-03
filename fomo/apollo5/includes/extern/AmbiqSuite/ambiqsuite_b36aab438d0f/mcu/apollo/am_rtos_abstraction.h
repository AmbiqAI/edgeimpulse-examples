//*****************************************************************************
//
//! @file am_rtos_abstraction.h
//!
//! @brief File to help with some RTOS abstractions.
//!
//! This file helps with some RTOS abstractions.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef RTOS_ABSTRACTIONS_H
#define RTOS_ABSTRACTIONS_H

//*****************************************************************************
//
// Use FreeRTOS as the RTOS
//
//*****************************************************************************

#ifdef  USE_freeRTOS
/**************************************************************//**
 * RTOS abstraction for base type of things
******************************************************************/
#define RTOS_BASE_TYPE          BaseType_t

/**************************************************************//**
 * RTOS abstraction for starting the RTOS scheduler
******************************************************************/
#define RTOS_SCHEDULER_GO()     vTaskStartScheduler()

/**************************************************************//**
 * RTOS abstraction declaring a Thread ID (structure)
******************************************************************/
#define RTOS_THREAD_ID_DECL     TaskHandle_t

/**************************************************************//**
 * RTOS abstraction declaring a Counting Semaphore
******************************************************************/
#define RTOS_SEMA_DECL          SemaphoreHandle_t

/**************************************************************//**
 * RTOS abstraction declaring a MUTEX
******************************************************************/
#define RTOS_MUTEX_DECL         SemaphoreHandle_t

/**************************************************************//**
 * RTOS abstraction defining a MUTEX
******************************************************************/
#define RTOS_MUTEX_DEF(X)  /* do nothing in freeRTOS */

/**************************************************************//**
 * RTOS abstraction for initializing a MUTEX   no param in freeRTOS
******************************************************************/
#define RTOS_MUTEX_INIT(X)      xSemaphoreCreateMutex()

/**************************************************************//**
 * RTOS abstraction for locking a MUTEX
******************************************************************/
#define RTOS_MUTEX_LOCK(X, Y)   xSemaphoreTake((X), (Y))

/**************************************************************//**
 * RTOS abstraction for unlocking a MUTEX
******************************************************************/
#define RTOS_MUTEX_UNLOCK(X)    xSemaphoreGive((X))

/**************************************************************//**
 * RTOS abstraction declaring an Event Control Block
******************************************************************/
#define RTOS_EVENT_DECL         EventGroupHandle_t

/**************************************************************//**
 * RTOS abstraction wait on an event
 * @param X  RTOS event structure
 * @param Y  Bits to wait on
 * @param Z  Ticks to wait
******************************************************************/
#define RTOS_EVENT_WAIT(X, Y, Z) xEventGroupWaitBits((X), (Y), pdTRUE, pdFALSE, (Z))

/**************************************************************//**
 * RTOS abstraction signal a thread
******************************************************************/
#define RTOS_SIGNAL_SET(X, Y)   xEventGroupSetBits((X), (Y))


//*****************************************************************************
//
// Use RTX as the RTOS
//
//*****************************************************************************

#elif defined  USE_RTX
/**************************************************************//**
 * RTOS abstraction for starting the RTOS scheduler
******************************************************************/
#define RTOS_SCHEDULER_GO()     osDelay(osWaitForever)

/**************************************************************//**
 * RTOS abstraction declaring a Thread ID (structure)
******************************************************************/
#define RTOS_THREAD_ID_DECL     osThreadId

/**************************************************************//**
 * RTOS abstraction signal a thread
******************************************************************/
#define RTOS_SIGNAL_SET(X, Y)   osSignalSet((X), (Y))

/**************************************************************//**
 * RTOS abstraction declaring a Counting Semaphore
******************************************************************/
#define RTOS_SEMA_DECL          OS_SEM

/**************************************************************//**
 * RTOS abstraction defining a MUTEX
******************************************************************/
#define RTOS_MUTEX_DEF(X)       osMutexDef(X)

/**************************************************************//**
 * RTOS abstraction declaring a MUTEX
******************************************************************/
#define RTOS_MUTEX_DECL         osMutexId

/**************************************************************//**
 * RTOS abstraction for initializing a MUTEX
******************************************************************/
#define RTOS_MUTEX_INIT(X)      osMutexCreate((X))

/**************************************************************//**
 * RTOS abstraction for locking a MUTEX
******************************************************************/
#define RTOS_MUTEX_LOCK(X, Y)   osMutexWait((X), (Y))

/**************************************************************//**
 * RTOS abstraction for unlocking a MUTEX
******************************************************************/
#define RTOS_MUTEX_UNLOCK(X)    osMutexRelease((X))

/**************************************************************//**
 * RTOS abstraction declaring an Event Control Block
******************************************************************/
#define RTOS_EVENT_DECL         uint32_t
#else

/**************************************************************//**
 * RTOS abstraction dummy function fixme replace with real RTOS stuff
******************************************************************/
void  rtos_dummy_example(void);

/**************************************************************//**
 * RTOS abstraction dummy function with one integer parameter fixme replace
 * with real RTOS stuff
******************************************************************/
void  rtos_dummy_exampleN(int N);

/**************************************************************//**
 * RTOS abstraction dummy function fixme replace with real RTOS stuff
******************************************************************/
int  rtos_dummy_exampleN_nonvoid(int N);

/**************************************************************//**
 * RTOS abstraction dummy function fixme replace with real RTOS stuff
******************************************************************/
int  rtos_dummy_exampleN_2x_nonvoid(int N, int M);

/**************************************************************//**
 * RTOS abstraction for starting the RTOS scheduler
******************************************************************/
#define RTOS_SCHEDULER_GO()     rtos_dummy_example()

/**************************************************************//**
 * RTOS abstraction for how the I/O Master locks its hardware resource.
******************************************************************/
#define RTOS_LOCK()             rtos_dummy_example()

/**************************************************************//**
 * RTOS abstraction for how the I/O Master locks its hardware resource
 * in a non-blocking way.
******************************************************************/
#define RTOS_NONBLOCK_LOCK()    rtos_dummy_example_nonvoid()

/**************************************************************//**
 * RTOS abstraction for how the I/O Master unlocks its hardware resource.
******************************************************************/
#define RTOS_UNLOCK()           rtos_dummy_example()

/**************************************************************//**
 * RTOS abstraction for how the I/O Master locks its hardware resource.
******************************************************************/
#define RTOS_IOM_LOCK(N)        rtos_dummy_exampleN(N)

/**************************************************************//**
 * RTOS abstraction for how the I/O Master locks its hardware resource
 * in a non-blocking way.
******************************************************************/
#define RTOS_IOM_NONBLOCK_LOCK(N)  rtos_dummy_exampleN_nonvoid(N)

/**************************************************************//**
 * RTOS abstraction for how the I/O Master unlocks its hardware resource.
******************************************************************/
#define RTOS_IOM_UNLOCK(N)      rtos_dummy_exampleN(N)

/**************************************************************//**
 * RTOS abstraction declaring a MUTEX
******************************************************************/
#define RTOS_MUTEX_DECL         int

/**************************************************************//**
 * RTOS abstraction for initializing a MUTEX   no param in freeRTOS
******************************************************************/
#define RTOS_MUTEX_INIT(X)      (0)

/**************************************************************//**
 * RTOS abstraction for locking a MUTEX
******************************************************************/
#define RTOS_MUTEX_LOCK(X, Y)   rtos_dummy_exampleN_2x_nonvoid(X, Y)

/**************************************************************//**
 * RTOS abstraction for unlocking a MUTEX
******************************************************************/
#define RTOS_MUTEX_UNLOCK(X)    rtos_dummy_example()

/**************************************************************//**
 * RTOS abstraction declaring a Thread ID (structure)
******************************************************************/
#define RTOS_THREAD_ID_DECL     int

/**************************************************************//**
 * RTOS abstraction declaring a Counting Semaphore
******************************************************************/
#define RTOS_SEMA_DECL          int

/**************************************************************//**
 * RTOS abstraction declaring an Event Control Block
******************************************************************/
#define RTOS_ECB_DECL           int
#define RTOS_EVENT_DECL         uint32_t

#endif // USE_RTX

#endif // RTOS_ABSTRACTIONS_H
