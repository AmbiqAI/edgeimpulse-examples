
/**
 *  \file BT_os.c
 *
 *  This source file is part of EtherMind OS Abstraction module.
 *  In this file the platform dependent library calls are abstracted
 *  and mapped to calls used within the EtherMind Stack.
 *
 *  Version: FreeRTOS
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* -------------------------------------------- Header File Inclusion */
#include "BT_os.h"

#ifdef BT_STATUS
#include "BT_status.h"
#endif /* BT_STATUS */

/* -------------------------------------------- External Global Variables */
extern int appl_init(void);


/* -------------------------------------------- Exported Global Variables */

#ifdef HAVE_FREERTOS_STUB
#define CHIP_FREQ_MAINCK_RC_4MHZ        (4000000UL)
uint32_t SystemCoreClock = CHIP_FREQ_MAINCK_RC_4MHZ;
#endif /* HAVE_FREERTOS_STUB */

/* -------------------------------------------- Static Global Variables */
/* Application signal notification handler */
static void(*bt_process_term_handler)(void);

/* -------------------------------------------- Functions */

void BT_os_init (void)
{
    /**
     *  For FreeRTOS, this function needs to be called from BT_bluetooth_on
     *  and needed threads much have been created when the scheduler is started
     */
#ifdef FREERTOS_ENABLE_TRACE_UTILITY
    vTraceStart ();
#endif /* FREERTOS_ENABLE_TRACE_UTILITY */

    vTaskStartScheduler ();
}

/**
 *  \fn BT_thread_create
 *
 *  \brief To create a task/thread
 *
 *  \par Description:
 *  This function implements the OS Wrapper for the Thread Create call.
 *  It creates a new thread of control  that  executes concurrently  with
 *  the  calling  thread.
 *
 *  \param [out] thread
 *         Caller allocated location to hold the Thread ID
 *         for the newly Created Thread
 *  \param [in] attr
 *         The Thread Attributes, pre-initialized using BT_thread_attr_init()
 *  \param [in] start_routine
 *         The Thread Start Routine for the newly created task/thread.
 *         Upon creation, the new task/thread calls this function,
 *         passing it 'thread_args' as the argument
 *  \param [in] thread_args
 *         This parameter points to a caller allocated "resident" memory
 *         location, containing the arguments to be passed to
 *         the newly created task's/thread's start routine
 *
 *  \return INT32 : 0 on Success. -1 on Failure.
 */
INT32 BT_thread_create
      (
          /* OUT */ BT_thread_type *         thread,
          /* IN */  BT_thread_attr_type *    attr,
          /* IN */  BT_THREAD_START_ROUTINE  start_routine,
          /* IN */  void *                   thread_args
      )
{
    portBASE_TYPE retval;

    (void) attr;
    (void) thread_args;

    retval =
    xTaskCreate
    (
        /* Pointer to the function that implements the task */
        (pdTASK_CODE) start_routine,
        /*
         * TBD: A descriptive name for the task. This is mainly used to facilitate
         * debugging. Maximum length is defined by the configMAX_TASK_NAME_LEN parameter
         */
        /* (const signed portCHAR * const)"EtherMind Task", */
        (const char * const)"EtherMind Task",
        /*
         * TBD: The size in bytes of the memory pointed to by the pcStackBuffer pointer.
         * The minimum allowable size for the stack buffer is 136 bytes.
         */
        (unsigned portLONG)256*2, /* ulStackDepthBytes */
        /*
         * TBD: Task functions take a void * parameter - the value of which is set by
         * pvParameters when the task is created.
         */
        (void *)NULL /* pvParameters */,
        /*
         * TBD: The priority of the task. Can take any value between 0 and
         * (configMAX_PRIORITIES - 1). The lower the numeric value of the
         * assigned priority, the lower the relative priority of the task.
         */
        2  /* uxPriority */,
        /*
         * TBD: Used to pass back a handle by which the created task can be
         * referenced, for example, when changing the priority of the task or
         * subsequently deleting the task.
         */
        thread /* pxCreatedTask */
    );

    return (INT32)retval;
}


/**
 *  \fn BT_thread_attr_init
 *
 *  \brief To initialize a task's/thread's attributes to default settings
 *
 *  \par Description:
 *  This function implements the OS Wrapper for the Thread attribute init
 *  call. It initializes the given thread attribute object with default
 *  values.
 *
 *  \param [out] attr
 *         Thread attribute object to be initialized
 *
 *  \return INT32 : 0 on Success. -1 on Failure.
 */
INT32 BT_thread_attr_init
      (
          /* OUT */ BT_thread_attr_type *    attr
      )
{
    (void) attr;
    return 0;
}


/**
 *  \fn BT_thread_mutex_init
 *
 *  \brief To initialize a Mutex object
 *
 *  \par Description:
 *  This function implements the OS Wrapper for the Thread Mutex Init Call.
 *  It creates or initializes the mutex object and fills it with default
 *  values for the attributes.
 *
 *  \param [out] mutex
 *         The Mutex variable to be Initialized
 *  \param [in] mutex_attr
 *         Attribute of the mutex variable on initialization
 *
 *  \return INT32 : 0 on Success. -1 on Failure.
 */
INT32 BT_thread_mutex_init
      (
          /* OUT */ BT_thread_mutex_type *         mutex,
          /* IN */  BT_thread_mutex_attr_type *    mutex_attr
      )

{
    (void) mutex_attr;

    vSemaphoreCreateBinary((*mutex));
    return 0;
}


/**
 *  \fn BT_thread_mutex_lock
 *
 *  \brief To lock a Mutex object
 *
 *  \par Description:
 *  This function implements the OS Wrapper for the Thread Mutex Lock call.
 *  It locks the given mutex.
 *  If the given mutex is currently unlocked, it becomes locked and owned by
 *  the calling thread, and this routine returns immediately.
 *  If the mutex is already locked by another thread, this routine suspends
 *  the calling thread until the mutex is unlocked.
 *
 *  \param [in,out] mutex
 *         The Mutex variable to be locked
 *
 *  \return INT32 : 0 on Success. -1 on Failure.
 */
INT32 BT_thread_mutex_lock
      (
          /* INOUT */ BT_thread_mutex_type *    mutex
      )
{
    return (INT32) xSemaphoreTake( (*mutex), portMAX_DELAY /* (portTickType)0 */ /* xBlockTime */ );
}


/**
 *  \fn BT_thread_mutex_unlock
 *
 *  \brief To unlock a Mutex object
 *
 *  \par Description:
 *  This function implements the OS Wrapper for the Mutex Unlock call.
 *  It unlocks the given mutex. The mutex is assumed to be locked and
 *  owned by the calling thread.
 *
 *  \param [in,out] mutex
 *         The Mutex variable to be unlocked
 *
 *  \return INT32 : 0 on Success. -1 on Failure.
 */
INT32 BT_thread_mutex_unlock
      (
          /* INOUT */ BT_thread_mutex_type *    mutex
      )
{
    return (INT32) xSemaphoreGive(*mutex);
}


/**
 *  \fn BT_thread_cond_init
 *
 *  \brief To initialize a Conditional Variable object
 *
 *  \par Description:
 *  This function implements the Conditional Variable Initialisation call.
 *  It initializes the given conditional variable.
 *
 *  \param [out] cond
 *         The Conditional Variable
 *  \param [in] cond_attr
 *         Attributes of the conditional variable on initialization
 *
 *  \return INT32 : 0 on Success. -1 on Failure.
 *
 *  \note
 *  A conditional variable is a synchronization primitive that allows
 *  threads to suspend execution and relinquish the processors
 *  until some predicate on shared resources is satisfied.
 */
INT32 BT_thread_cond_init
      (
          /* OUT */ BT_thread_cond_type *         cond,
          /* IN */  BT_thread_cond_attr_type *    cond_attr
      )
{
    (void) cond_attr;
    vSemaphoreCreateBinary ((*cond));

    return 0;
}


/**
 *  \fn BT_thread_cond_wait
 *
 *  \brief To wait on a Conditional Variable object
 *
 *  \par Description:
 *  This function implements the OS Wrapper for the Thread Wait on
 *  Conditional Variable call.
 *  It waits for the conditional variable to be signaled.
 *  This routine releases the  mutex and waits for the condition variable
 *  to  be signaled. The execution of the calling thread is suspended
 *  until the conditional variable is signaled.
 *  The mutex must be locked by the calling thread before
 *  calling this function. Before returning to the calling thread,
 *  it will re-acquire the mutex variable.
 *
 *  \param [in,out] cond
 *         The Conditional Variable
 *  \param [in,out] cond_mutex
 *         The Conditional Mutex Variable
 *
 *  \return INT32 : 0 on Success. -1 on Failure.
 */
INT32 BT_thread_cond_wait
      (
          /* INOUT */ BT_thread_cond_type *     cond,
          /* INOUT */ BT_thread_mutex_type *    cond_mutex
      )
{
    INT32 retval;

    BT_thread_mutex_unlock (cond_mutex);
    retval = (INT32) xSemaphoreTake( (*cond), portMAX_DELAY /* 100 */ /* xBlockTime */ );
    BT_thread_mutex_lock (cond_mutex);

    return retval;
}


/**
 *  \fn BT_thread_cond_signal
 *
 *  \brief To signal a Conditional Variable object
 *
 *  \par Description:
 *  This function implements the OS Wrapper for the Thread Signalling on
 *  Conditional Variable Call.
 *  It will signal and restart one of the threads that are waiting on
 *  the given conditional variable.
 *
 *  \param [in,out] cond
 *         The Conditional Variable to signal
 *
 *  \return INT32 : 0 on Success. -1 on Failure.
 */
INT32 BT_thread_cond_signal
      (
          /* INOUT */ BT_thread_cond_type *    cond
      )
{
//    return (INT32) xSemaphoreGive(*cond);
    return (INT32) xSemaphoreGiveFromISR(*cond);
}


/**
 *  \fn BT_alloc_mem
 *
 *  \brief To allocate memory dynamically
 *
 *  \par Description:
 *  This function implements the OS Wrapper for the Memory Allocation call.
 *  It allocates memory of 'size' bytes and returns a pointer to
 *  the allocated memory.
 *
 *  \param [in] size
 *         The number of bytes to allocate
 *
 *  \return void * :  Pointer to the newly Allocated Memory segment.
 *                    NULL, in case of failure
 */
void * BT_alloc_mem ( /* IN */ UINT32 size )
{
    void * buf;

    buf = pvPortMalloc (size);

#ifdef BT_STATUS
    if (NULL == buf)
    {
        /* Memory Allocated Failed: Updated EtherMind Status Flag */
        BT_status_set_bit (STATUS_BIT_MEM_ALLOC_FAILED, STATUS_BIT_SET);
    }
#endif /* BT_STATUS */

    return buf;

}


/**
 *  \fn BT_free_mem
 *
 *  \brief To free dynamically allocated memory
 *
 *  \par Description:
 *  This function implements the OS Wrapper for the Memory Free call.
 *  It frees the memory space specified by the given pointer.
 *
 *  \param [in] ptr
 *         Pointer to the Memory location to be freed
 *
 *  \return None
 */
void BT_free_mem ( /* IN */ void * ptr )
{
    vPortFree (ptr);
    return;
}


/**
 *  \fn BT_sleep
 *
 *  \brief To delay execution for a specified number of seconds
 *
 *  \par Description:
 *  This function implements the OS Wrapper for a Thread or task to sleep
 *  for specified number of seconds.
 *
 *  \param [in] tm
 *         Number of seconds the calling task or thread wants to sleep
 *
 *  \return None
 */
void BT_sleep( /* IN */ UINT32 tm )
{
	/*
	 * 1s = 1000 ms.
	 * 1ms = portTICK_RATE_MS Ticks
	 */
    /* The number of ticks for which the calling task should be held in the Blocked state. */
	vTaskDelay ((portTickType)((tm * 1000) / (portTICK_RATE_MS)));
}


/**
 *  \fn BT_usleep
 *
 *  \brief To delay execution for a specified number of microseconds
 *
 *  \par Description:
 *  This function implements the OS Wrapper for a Thread or task to sleep
 *  for specified number of micro-seconds.
 *
 *  \param [in] tm
 *         Number of micro-seconds the calling task or thread wants to sleep
 *
 *  \return None
 */
void BT_usleep( /* IN */ UINT32 tm )
{
	/*
	 * 1000 us = 1ms.
	 * 1ms = portTICK_RATE_MS Ticks
	 */
    /* The number of ticks for which the calling task should be held in the Blocked state. */
	vTaskDelay ((portTickType)(tm / (1000 * portTICK_RATE_MS)));
}


/**
 *  \fn BT_process_term_notify
 *
 *  \brief To register handler for process termination
 *
 *  \par Description:
 *  This function implements the OS Wrapper to get process termination
 *  signal from system
 *
 *  \param handler (IN)
 *         Handler to be called for notification
 *
 *  \return None
 */
void BT_process_term_notify (void(*handler)(void))
{
    /* Register the callback */
    bt_process_term_handler = handler;
}


void vAssertCalled( unsigned long ulLine, const char * const pcFileName )
{
    (void)ulLine;
    (void)pcFileName;
}

#if 0

void vApplicationIdleHook( void )
{
}


void vApplicationStackOverflowHook( xTaskHandle xTask, signed char *pcTaskName )
{
    (void)xTask;
    (void)pcTaskName;
}


void vApplicationTickHook( void )
{
}

void vApplicationMallocFailedHook( void )
{
    static volatile uint32_t ulCount = 0;

    /* vApplicationMallocFailedHook() will only be called if
    configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
    function that will get called if a call to pvPortMalloc() fails.
    pvPortMalloc() is called internally by the kernel whenever a task, queue,
    timer or semaphore is created.  It is also called by various parts of the
    demo application.  If heap_1.c or heap_2.c are used, then the size of the
    heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
    FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
    to query the size of free heap space that remains (although it does not
    provide information on how the remaining heap might be fragmented).

    Just count the number of malloc fails as some failures may occur simply
    because the network load is very high, resulting in the consumption of a
    lot of network buffers. */
    ulCount++;
}
#endif

/*-----------------------------------------------------------*/
/* Provided to keep the linker happy. */
#ifdef HAVE_FREERTOS_STUB
void _exit( int status )
{
    ( void ) status;
    vAssertCalled( __LINE__, __FILE__ );
    for( ;; );
}

extern caddr_t _sbrk(int incr)
{
    return (caddr_t) 0;
}

extern int _close(int file)
{
    return -1;
}

extern int _fstat(int file, struct stat *st)
{
    return 0;
}

extern int _isatty(int file)
{
    return 1;
}

extern int _lseek(int file, int ptr, int dir)
{
    return 0;
}
#endif /* HAVE_FREERTOS_STUB */

int _open( int file )
{
    (void) file;
    return 0;
}

int _read( void )
{
    return 0;
}

int _write( int x )
{
    ( void ) x;
    return 0;
}
/*-----------------------------------------------------------*/
#if 0
int main(void)
{
    appl_init();

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
    /* If all is well, the scheduler will now be running, and the following
    line will never be reached.  If the following line does execute, then
    there was insufficient FreeRTOS heap memory available for the idle and/or
    timer tasks    to be created.  See the memory management section on the
    FreeRTOS web site for more details. */
    for( ;; );
}
#endif
