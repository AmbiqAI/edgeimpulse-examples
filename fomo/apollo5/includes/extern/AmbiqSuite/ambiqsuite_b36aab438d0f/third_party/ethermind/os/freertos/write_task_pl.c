
/**
 *  \file write_task_pl.c
 *
 *  This file contains source code for the platform specific portions of
 *  Write Task.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* -------------------------------------- Header File Inclusion */
#include "write_task_internal.h"


/* -------------------------------------- External Global Variables */


/* -------------------------------------- Exported Global Variables */
/** Write Task Mutex */
BT_DEFINE_MUTEX (wt_mutex)

/** Write Task Conditional Variable for Waiting/Signalling */
BT_DEFINE_COND (wt_cv)


/* -------------------------------------- Staic Global Variables */
/* Write Task ID */
DECL_STATIC BT_thread_type wt_tid;


/* -------------------------------------- Functions */
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
API_RESULT wt_init_pl (void)
{
    /* Initialize Write Task Mutex */
    BT_MUTEX_INIT(wt_mutex, WT);

    /* Initialize Write Task CV & Mutex */
    BT_COND_INIT(wt_cv, WT);

    WT_TRC(
    "[WT] Creating Write Task ...\n");

    /* Create Write Task */
    if (BT_thread_create (&wt_tid,
                          NULL,
                          write_task_start_routine,
                          NULL) < 0)
    {
        WT_ERR(
        "[WT] FATAL ERROR: FAILED to Create Write Task\n");

        return API_FAILURE;
    }

    return API_SUCCESS;
}


