
/**
 *  \file write_task_pl.h
 *
 *  Platform specific Header File for Write Task module.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_WRITE_TASK_PL_
#define _H_WRITE_TASK_PL_

/* ----------------------------------------------- Header File Inclusion */
#include "BT_common.h"


/* ----------------------------------------------- Global Definitions */


/* ----------------------------------------------- Structures/Data Types */


/* ----------------------------------------------- Function Declarations */
/**
 *  \fn wt_init_pl
 *
 *  \par Description
 *  This function is called by Write Task during its initialization to
 *  handle platform specific operations such as initialization of the
 *  mutex & conditional variables, and, creation of the write task itself.
 *
 *  
 *
 *  \return API_RESULT
 */
API_RESULT wt_init_pl (void);

/**
 *  \fn wt_exit_pl
 *
 *  \par Description
 *  This function is called by the Write Task Start Routine
 *  (write_task_start_routine) to know if the Task needs to break out
 *  of its core loop and return (which effectively means exit of the
 *  task).
 *
 *  
 *
 *  \return UCHAR
 *          0 - No exit, continue to execute.
 *          1 - Break from core loop & return.
 */
#define wt_exit_pl()                    0

/**
 *  \fn wt_lock_pl
 *
 *  \par Description
 *  This function locks Write Task Mutex for synchronization with other
 *  tasks in the system.
 *
 *  
 *
 *  \return void 
 */
#define wt_lock_pl()                    BT_thread_mutex_lock(&wt_mutex)
#define wt_lock_void_pl()               BT_thread_mutex_lock(&wt_mutex)

/**
 *  \fn wt_unlock_pl
 *
 *  \par Description
 *  This function unlocks Write Task Mutex for synchronization with other
 *  tasks in the system.
 *
 *  
 *
 *  \return void 
 */
#define wt_unlock_pl()                  BT_thread_mutex_unlock(&wt_mutex)
#define wt_unlock_void_pl()             BT_thread_mutex_unlock(&wt_mutex)

/**
 *  \fn wt_wait_pl
 *
 *  \par Description
 *  This function implements the CPU relieving wait for Write Task
 *  until it is signalled again for execution & servicing of queues.
 *
 *  \param: None
 *
 *  \return void 
 */
#define wt_wait_pl() \
        BT_thread_cond_wait (&wt_cv, &wt_mutex);

/**
 *  \fn wt_signal_pl
 *
 *  \par Description
 *  This function implements signalling of Write Task to wake it up
 *  from sleep to service its queues.
 *
 *  \param: None
 *
 *  \return void 
 */
#define wt_signal_pl() \
        BT_thread_cond_signal (&wt_cv);

#endif /* _H_WRITE_TASK_PL_ */


