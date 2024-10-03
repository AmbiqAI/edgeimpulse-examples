
/**
 *  \file device_queue_pl.c
 *
 *  This file contains source code for the platform specific portions of
 *  Device Queue.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* -------------------------------------- Header File Inclusion */
#include "device_queue_pl.h"
#include "smp_pl.h"

/* -------------------------------------- External Global Variables */


/* -------------------------------------- Exported Global Variables */

/* -------------------------------------- Staic Global Variables */

/* -------------------------------------- Functions */

void device_queue_pl_init ( void )
{
}


void device_queue_pl_shutdown ( void )
{
}


void device_queue_free_pl (DEVICE_HANDLE    * hndl)
{
    BT_IGNORE_UNUSED_PARAM(hndl);
}

void device_queue_full_pl (void)
{
#ifdef HOST_PURGE_ON_DB_FULL
    UCHAR index;

    /*
     * NOTE:
     *
     * The default reference handling here tries to purge SMP
     * database to create a free device entity. Platform
     * implementation can modify this as required. If one does
     * not want to make any free space here but act at a later point of
     * time, this function can be left empty. But in that case, all other
     * incoming connections will be disconnected from the stack.
     */

    /* Call to free a device and make space through SMP purge  */
    smp_purge_device_list_pl (&index);
#endif /* HOST_PURGE_ON_DB_FULL */
}


void device_queue_disconnect_on_full_pl
     (
         /* IN */ BT_DEVICE_ADDR    * bd_addr
     )
{
    BT_IGNORE_UNUSED_PARAM(bd_addr);

    /**
     *  Code shall not reach here.
     *  Confirm with the below print in the log.
     */
    DQ_PL_ERR (
    "[DQ PL]: Disconnect on FULL\n");

    /* Do the same handling as DQ full to try create an empty space */
    device_queue_full_pl();

    return;
}

#ifdef STORAGE_CHECK_CONSISTENCY_ON_RESTORE
void device_queue_cleanup_pl
     (
         /* IN */ UINT32 dq_valid_entries
     )
{
    DEVICE_HANDLE index;

    DQ_PL_TRC(
    "[DQ PL] Valid DQ entries bit field 0x%08X.\n", dq_valid_entries);

#if (1 != BT_MAX_DEVICE_QUEUE_SIZE)
    for (index = 0; index < BT_MAX_DEVICE_QUEUE_SIZE; index++)
#else
    index = 0;
#endif /* (1 != BT_MAX_DEVICE_QUEUE_SIZE) */
    {
        /**
         * Check if the associated device queue handle is valid or not.
         * If not valid, free the device queue element.
         */
        if (0 == (dq_valid_entries & (1 << index)))
        {
            device_queue_free(&index);
        }
    }

    return;
}
#endif /* STORAGE_CHECK_CONSISTENCY_ON_RESTORE */

