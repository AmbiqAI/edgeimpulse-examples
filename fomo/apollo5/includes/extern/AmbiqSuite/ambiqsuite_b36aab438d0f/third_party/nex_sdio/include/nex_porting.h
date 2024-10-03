//____________________________________________________________________
// Copyright Info  : All Rights Reserved - (c) Arasan Chip Systems Inc.,
//
// Name            : nex_porting.h
//
// Creation Date   : 13 May 2013
//
// Description     : Contains OS independent wrapper Datastructures and APIs
//					 for RTOS specific APIs
//
// Changed History :
//<Date>            <Author>        <Version>        <Description>
// 13 May 2013       TJ              0.01             OS independent header File for NEX Bus driver
// 27 Jul 2020       Ambiq           0.9              remove Linux OS specific APIs
//____________________________________________________________________

#ifndef _NEX_PORTING_H
#define _NEX_PORTING_H

#include "nex_common_types.h"
#include "am_util.h"

typedef INT32 TIMER_LIST;
typedef INT32 MUTEXLOCK;
typedef INT32 SEMAPHORE;
typedef INT32 TASKLET;
typedef INT32 COMPLETION;
typedef INT32 WORK_STRUCT;

#define NEX_ENOMEM 12
#define NEX_ENODEV 19

#define NEX_DMA_BIDIRECTIONAL 0
#define NEX_DMA_TODEVICE      1
#define NEX_DMA_FROMDEVICE    2
#define NEX_DMA_NONE          3

#define NEX_NULL ((VOID *)0)

#define NEX_BUG_ON(condition)                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        if (condition)                                                                                                 \
            ;                                                                                                          \
    } while (0)

#define DPRINT(str, func, line)  am_util_stdio_printf("%s %28s %4d | \n", str, func, line)

#define RPRINT(format, ...)  am_util_stdio_printf(format, ##__VA_ARGS__)

#define KPRINT(lvl, str, func, line, format, ...) am_util_stdio_printf("%39s %4d | %s"format, func, line, str, ##__VA_ARGS__)

#define NEX_SIZEOF(x) sizeof(x)

#define NEX_MIN(x, y) (((x) < (y)) ? (x) : (y))

static inline VOID *NEX_ERR_PTR(LONG error)
{
    return (VOID *)error;
}

/*******************************************************************************
Name        : Nex_Memset
Argument    : s - source
              c - value to be initialized
              count - No of values to be initialized
Returns     : success/failure
Description : The API is used to initialize memory with the given value.
********************************************************************************/
static inline VOID *Nex_Memset(VOID *s, INT32 c, INT32 count)
{
    CHAR *xs = s;
    while (count--)
        *xs++ = c;
    return s;
}

/*******************************************************************************
Name        : Nex_Memcpy
Argument    : s - source
              c - value to be initialized
              count - No of values to be initialized
Returns     : success/failure
Description : The API is used to copy one area of memory to another.
********************************************************************************/
static inline VOID *Nex_Memcpy(VOID *dest, const VOID *src, INT32 count)
{
    CHAR *tmp = dest;
    const CHAR *s = src;

    while (count--)
        *tmp++ = *s++;
    return dest;
}

VOID M_NEX_DELAY_MILLISEC(ULONG ms);

VOID M_NEX_DELAY_MIRCOSEC(ULONG us);

/*******************************************************************************
Name        : Nex_RTOS_Init
Argument    : None
Returns     : 0 - SUCCESS, -1 - FAILURE
Description : This is a wrapper function to create TaskLet and WorkQueue Tasks.
********************************************************************************/
INT32 Nex_RTOS_Init(VOID);

/*******************************************************************************
Name        : Nex_Timer_Init
Argument    : *func - Function pointer which is to be called when the timer expires
              data - Data to be passed to driver when Timer expires
Returns     : timer id
Description : This is a wrapper function for  timer initialization and
              returns timer id associated with the timer created.
********************************************************************************/
INT32 Nex_Timer_Init(VOID (*func)(ULONG), ULONG data);


/*******************************************************************************
Name        : M_NEX_MODIFY_TIMER
Argument    : timer_id - timer id of the timer to modify
              expires - time when the timer has to expire
Returns     : success/failure
Description : This wrapper function is used to modify the timer value.
********************************************************************************/
INT32 M_NEX_MODIFY_TIMER(INT32 timer, ULONG expires);

/*******************************************************************************
Name        : M_NEX_DELETE_TIMER
Argument    : timer_id - timer id of the timer to be deleted
Returns     : success/failure
Description : This wrapper function is used to deactivate a timer.
********************************************************************************/
INT32 M_NEX_DELETE_TIMER(INT32 timer);

/*******************************************************************************
Name        : Nex_MSleep
Argument    : msecs - time in microseconds to sleep
Returns     : None
Description : This wrapper function is used to sleep for given milliseconds.
********************************************************************************/
VOID Nex_MSleep(UINT32 msecs);

/*******************************************************************************
Name        : Nex_Alloc
Argument    : size - the size of the memory to allocate
Returns     : the pointer to the allocated memory
Description : This wrapper function is used to deallocate the memory.
********************************************************************************/
VOID *Nex_Alloc(UINT32 size);

/*******************************************************************************
Name        : Nex_Free
Argument    : *p - pointer to the memory to free
Returns     : None
Description : This wrapper function is used to deallocate the memory.
********************************************************************************/
VOID Nex_Free(const VOID *p);

/*******************************************************************************
Name        : M_NEX_READ_BYTE
Argument    : addr - register address
Returns     : value that is read
Description : This wrapper function is used to read a byte from the Host Controller
              register space.
********************************************************************************/
static inline UCHAR M_NEX_READ_BYTE(UINT32 addr)
{
	return *((volatile UCHAR *)(addr));
}


/*******************************************************************************
Name        : M_NEX_READ_HALF_WORD
Argument    : addr - register address
Returns     : value that is read
Description : This wrapper function is used to read a 2bytes from the Host Controller
              register space.
********************************************************************************/
static inline UINT16 M_NEX_READ_HALF_WORD(UINT32 addr)
{
	return *((volatile UINT16 *)(addr));
}

/*******************************************************************************
Name        : M_NEX_READ_WORD
Argument    : addr - register address
Returns     : value that is read
Description : This wrapper function is used to read a word from the Host Controller
          register space.
********************************************************************************/
static inline UINT32 M_NEX_READ_WORD(UINT32 addr)
{
	return *((volatile UINT32 *)(addr));
}

/*******************************************************************************
Name	    : M_NEX_WRITE_BYTE
Argument    : value - value to be written
              addr - register address
Returns     : None
Description : This wrapper function is used to write a byte to the Host Controller
              register space.
********************************************************************************/
static inline VOID M_NEX_WRITE_BYTE(UCHAR value, UINT32 addr)
{
	*((volatile UCHAR *)(addr)) = value;
}

/*******************************************************************************
Name        : M_NEX_WRITE_HALF_WORD
Argument    : value - value to be written
              addr - register address
Returns     : None
Description : This wrapper function is used to write 2 bytes to the Host Controller
              register space.
********************************************************************************/
static inline VOID M_NEX_WRITE_HALF_WORD(UINT16 value, UINT32 addr)
{
	*((volatile UINT16 *)(addr)) = value;
}

/*******************************************************************************
Name        : M_NEX_WRITE_WORD
Argument    : value - value to be written
              addr - register address
Returns     : None
Description : This wrapper function is used to write a word to the Host Controller
              register space.
********************************************************************************/
static inline VOID M_NEX_WRITE_WORD(UINT32 value, UINT32 addr)
{
	*((volatile UINT32 *)(addr)) = value;
}

/*******************************************************************************
Name        : Nex_Mutex_Init
Argument    : None
Returns     : id that represents the spinlock
Description : This wrapper function is used to initialize the spinlock and
          returns an id associated with the spinlock.
********************************************************************************/
INT32 Nex_Mutex_Init(VOID);

/*******************************************************************************
Name        : Nex_Mutex_Lock
Argument    : mutex_id - id of the MUTEX
Returns     : None
Description : This is a wrapper function to get the mutex.
********************************************************************************/
VOID Nex_Mutex_Lock(INT32 mutex_id);

/*******************************************************************************
Name        : Nex_Mutex_Unlock
Argument    : mutex_id - id of the MUTEX
Returns     : None
Description : This is a wrapper function for releasing the mutex.
********************************************************************************/
VOID Nex_Mutex_Unlock(INT32 mutex_id);

/*******************************************************************************
Name        : Nex_Critical_Enter
Argument    : None
Returns     : None
Description : This wrapper function is used to disable interrupts when entering
              the critical section.
********************************************************************************/
VOID Nex_Critical_Enter(VOID);

/*******************************************************************************
Name        : Nex_Critical_Exit
Argument    : None
Returns     : None
Description : This wrapper function is used to exit from the critical section.
********************************************************************************/
VOID Nex_Critical_Exit(VOID);


/*******************************************************************************
Name        : Nex_Init_Semaphore
Argument    : count - usage count
Returns     : id of semaphore structure
Description : This is a wrapper function for initializing the semaphores.
*******************************************************************************/
INT32 Nex_Init_Semaphore(INT32 count);

/*******************************************************************************
Name        : Nex_Acquire_Semaphore
Argument    : lock : id of semaphore structure
Returns     : success/failure
Description : This Wrapper function is used to lock semaphore operation
          which can be interrupted, when interrupt arrives.
********************************************************************************/
INT32 Nex_Acquire_Semaphore(INT32 lock);

/*******************************************************************************
Name        : Nex_Release_Semaphore
Argument    : lock : id of semaphore structure
Returns     : None
Description : This is a wrapper function to release the locked semaphore.
********************************************************************************/
VOID Nex_Release_Semaphore(INT32 lock);

/*******************************************************************************
Name        : Nex_Task_Init
Argument    : *func - function pointer which is called when the tasklet is scheduled
              data - Data to be passed to driver when tasklet is scheduled
Returns     : id of tasklet structure created
Description : This is a wrapper function for tasklet initialization.
********************************************************************************/
INT32 Nex_Task_Init(VOID (*func)(ULONG), ULONG data);

/*******************************************************************************
Name        : Nex_Task_Schedule
Argument    : task_id - id of tasklet structure used
Returns     : None
Description : This is a wrapper function used to schedule the tasklet.
********************************************************************************/
VOID Nex_Task_Schedule(INT32 task_id, UCHAR in_isr);

/*******************************************************************************
Name        : Nex_Task_Kill
Argument    : task - id of tasklet structure used
Returns     : None
Description :  This is a wrapper function used to kill tasklet.
********************************************************************************/
VOID Nex_Task_Kill(INT32 task_id);

/*******************************************************************************
Name         : Nex_Init_Work
Argument     : *func - function pointer which is called when the workqueue is
                        scheduled
               data - Data to be passed to driver when the workqueue is scheduled
Returns      : id representing the workqueue initialized
Description  : This wrapper function is used to initialize workqueue.
           It stores the driver's workqueue function pointer in the local
           ufs_wq structure and calls it when the workqueue is scheduled.
********************************************************************************/
INT32 Nex_Init_Work(VOID (*func)(ULONG), ULONG data);

/*******************************************************************************
Name         : Nex_Schedule_Work
Argument     : wq_id:  id of workqueue structure used
Returns      : None
Description  : This is a wrapper function for workqueue scheduling call.
********************************************************************************/
VOID Nex_Schedule_Work(INT32 wq_id);

/*******************************************************************************
Name        : Nex_Create_Thread
Argument    : *function - pointer to function to be called
              *arg - void pointer
Returns     : success/failure
Description : This is a wrapper function for creating kernel thread .
********************************************************************************/
INT32 Nex_Create_Thread(INT32 (*function)(VOID *), VOID *arg);

/*******************************************************************************
Name        : Nex_KillProc
Argument    : thread_id - thread id
Returns     : success/failure
Description : This wrapper function is used to kill a thread.
********************************************************************************/
INT32 Nex_Kill_Thread(INT32 thread_id);

/*******************************************************************************
Name        : M_NEX_INIT_COMPLETION
Argument    : None
Returns     : id representing the completion structure
Description : This is a wrapper function which initializes completion structure
          and returns an id associated with it.
********************************************************************************/
INT32 M_NEX_INIT_COMPLETION(VOID);

/*******************************************************************************
Name        : Nex_Block_till_complete
Argument    : comp_id - pointer to completion structure used
Returns     : None
Description : This is a wrapper function for blocking the call.
********************************************************************************/
VOID Nex_Block_till_complete(INT32 comp_id);

/*******************************************************************************
Name        : M_NEX_COMPLETE
Argument    : comp_id - pointer to completion structure used
Returns     : None
Description : This is a wrapper function to call complete.
********************************************************************************/
VOID M_NEX_COMPLETE(INT32 comp_id);

/******************************************************************************
Name        : Nex_Request_irq
Argument    : irq : Interrupt line to allocate
Returns     : None
Description : This wrapper function is used to get an irq number
              and appoint handler to the specific irq number.
********************************************************************************/
VOID Nex_Request_Irq(UINT32 irq);

/*******************************************************************************
Name        : Nex_Free_Bus_irq
Argument    : irq - Interrupt line to free
Returns     : None
Description : This wrapper function is used to free the IRQ number which got allocated.
********************************************************************************/
VOID Nex_Free_Irq(UINT32 irq);

/* SDHC Host controller information structure */
struct sdhc_host_info {
	UINT32 regBaseAddr[MAX_SDHC][MAX_SLOTS]; // This will hold the base address of SDHC controller   
	UINT32 irq[MAX_SDHC];
	INT32  slots[MAX_SDHC];
	INT32  sdhc_cnt;
	INT32  dma_level;
	INT32  debug_level;
	INT32  io_volt; //0 - 3.3V, 1 - 1.8V
};

enum {
	PIO_MODE  = 0,
	SDMA_MODE = 1,
	ADMA_MODE = 2,
};

struct sdhc_host_info *GetSdhcHostInfo(void);

INT32 Nex_Hw_Init(VOID);

INT32 Nex_Hw_DeInit(VOID);


/*******************************************************************************
Name        : Nex_DeInitOsdepVariables
Argument    : None
Returns     : None
Description : This is a wrapper function for deinitializing all Linux dependent
          variables used in driver.
********************************************************************************/
VOID Nex_DeInitOsdepVariables(VOID);

#endif
