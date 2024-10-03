
/**
 *  \file ht_read_task_pl.c
 *
 *  This file contains source codes that platform specific for the
 *  HCI Transport Read Task/Thread.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* Module Header File Inclusion */
#include "hci_transport_internal.h"


/* -------------------------------------- External Global Variables */


/* -------------------------------------- Exported Global Variables */


#ifndef BT_TRANSPORT_SINGLE_TASK
/* -------------------------------------- Static Global Variables */
/* Thread ID of the Read Task */
DECL_STATIC BT_thread_type ht_tid;


/* ------------------------------------- Functions */

/*
 *  Function Name:
 *    ht_read_task_create_pl
 *
 *  Description:
 *    This function is used to create the Read Task/Thread.
 *    If the Read Task/Thread is created by other means, return from here.
 *    This routine is called from the hci_transport_task_init() function.
 *
 *  Input Parameters:
 *      None.
 *
 *  Output Parameters:
 *      None.
 */
void ht_read_task_create_pl ( void )
{
    if ( BT_thread_create ( &ht_tid,
                            NULL,
                            ht_read_task_start_routine,
                            NULL ) < 0 )
    {
        HT_ERR(
        "[HT]: FAILED to Create Read Task.\n");
    }
}


/*
 *  Function Name:
 *    ht_read_task_shutdown_pl
 *
 *  Description:
 *    This function is used to shutdown the task created previously through
 *    ht_read_task_create_pl().
 *
 *    This routine is called from ht_read_task_start_routine() when the state
 *    of Read Task is HT_SHUTDOWN.
 *
 *    It may not be necessary to shutdown the task explicitly when
 *    1. Killing the process, removes the threads/tasks created
 *    2. Stack is never shut down.
 *
 *  Input Parameters:
 *      None.
 *
 *  Output Parameters:
 *      None.
 */
UCHAR ht_read_task_shutdown_pl ( void )
{
    return 0;
}


/*
 *  Function Name:
 *    ht_read_task_signal_pending_pl
 *
 *  Description:
 *    This function is used to find out if any signal is pending for the
 *    process/task. This is currently valid for Linux Driver mode only.
 *    If not implemented, return 0 always.
 *
 *  Input Parameters:
 *      Void.
 *
 *  Output Parameters:
 *      1 - signal pending
 *      0 - No signal pending.
 *
 *  -------------------------------------------------------------------------
 *  IMPORTANT:
 *    This function is not required for Non kernel implementations.
 *    Uncomment the function if required. This function is #define'd to 0
 *    in hci_transport_internal.h. To use the function, remove the #define
 *    and define the function here.
 *  -------------------------------------------------------------------------
 */
#if 0
UCHAR ht_read_task_signal_pending_pl ( void )
{
    return 0;
}
#endif

#endif /* BT_TRANSPORT_SINGLE_TASK */

