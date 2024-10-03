
/**
 *  \file storage_pl.c
 *
 *  This file contains the implementation for all platform specific
 *  extensions for persistant storage module.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "BT_storage.h"
#include "storage_pl.h"

#ifdef BT_STORAGE

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */
/* Storage File Handle array */
DECL_STATIC FILE * fp[STORAGE_NUM_TYPES];

/* Storage File Name array */
DECL_STATIC UCHAR * fn[STORAGE_NUM_TYPES] =
{
    (UCHAR *)"btps.db",
#ifdef STORAGE_RETENTION_SUPPORT
    (UCHAR *)"btrn.db",
#endif /* STORAGE_RETENTION_SUPPORT */
};

#if (STORAGE_SKEY_SIZE != 0)
/* Storage Signature Key array */
DECL_STATIC UCHAR * ssign[STORAGE_NUM_TYPES];
#endif /* (STORAGE_SKEY_SIZE != 0) */

/* --------------------------------------------- Functions */

void storage_bt_init_pl (void)
{
    UCHAR i;

    for (i = 0; i < STORAGE_NUM_TYPES; i++)
    {
        fp[i] = NULL;
    }
}

void storage_bt_shutdown_pl (void)
{
    UCHAR i;

    for (i = 0; i < STORAGE_NUM_TYPES; i++)
    {
        if (NULL != fp[i])
        {
            fclose (fp[i]);
            fp[i] = NULL;
        }
    }
}

API_RESULT storage_open_pl (UCHAR type, UCHAR mode)
{
    CHAR * rb = "rb";
    CHAR * wb = "wb";
    CHAR * rw;

    /* Set the file access mode */
    rw = (STORAGE_OPEN_MODE_WRITE == mode)? wb: rb;

    fp[type] = fopen ((CHAR *)fn[type], rw);
    if (NULL == fp[type])
    {
        return API_FAILURE;
    }

    return API_SUCCESS;
}

API_RESULT storage_close_pl (UCHAR type, UCHAR mode)
{
    BT_IGNORE_UNUSED_PARAM(mode);

    if (NULL != fp[type])
    {
        fclose (fp[type]);
        fp[type] = NULL;
    }

    return API_SUCCESS;
}

INT16 storage_write_pl (UCHAR type, void * buffer, UINT16 size)
{
    INT16 nbytes;

    nbytes = 0;

    if (NULL != fp[type])
    {
        nbytes = (INT16)(fwrite (buffer, size, sizeof (UCHAR), fp[type]));
    }

    return nbytes;
}

INT16 storage_read_pl (UCHAR type, void * buffer, UINT16 size)
{
    INT16 nbytes;

    nbytes = 0;

    if (NULL != fp[type])
    {
        nbytes = (INT16)(fread (buffer, size, sizeof (UCHAR), fp[type]));
    }

    return nbytes;
}

INT16 storage_write_signature_pl (UCHAR type)
{
#if (STORAGE_SKEY_SIZE != 0)
    INT16 nbytes;

    nbytes = 0;

    if (NULL != fp[type])
    {
        nbytes = fwrite (ssign[type], STORAGE_SKEY_SIZE, sizeof (UCHAR), fp[type]);
    }

    return nbytes;
#else /* (STORAGE_SKEY_SIZE != 0) */
    BT_IGNORE_UNUSED_PARAM(type);

    return 0;
#endif /* (STORAGE_SKEY_SIZE != 0) */
}

INT16 storage_read_signature_pl (UCHAR type)
{
#if (STORAGE_SKEY_SIZE != 0)
    UCHAR sign[STORAGE_SKEY_SIZE];
    INT16 nbytes;

    nbytes = 0;

    if (NULL ! fp[type])
    {
        nbytes = fread (ssign, STORAGE_SKEY_SIZE, sizeof (UCHAR), fp[type]);

        if (BT_mem_cmp (ssign, sign, STORAGE_SKEY_SIZE))
        {
            return -1;
        }
    }

    return nbytes;
#else /* (STORAGE_SKEY_SIZE != 0) */
    BT_IGNORE_UNUSED_PARAM(type);

    return 0;
#endif /* (STORAGE_SKEY_SIZE != 0) */
}

#ifdef STORAGE_CHECK_CONSISTENCY_ON_RESTORE
API_RESULT storage_check_consistency_pl
           (
               /* IN */ UCHAR type
           )
{
    UINT32 smp_dq_entries;

    /**
     * Check device queue has some stray elements (due to inconsistent power cycle of the system etc.),
     * which are not present in the security databases.
     */

    /* Get the device queue valid entries bit fields from SMP */
    smp_get_valid_dq_entries_pl(&smp_dq_entries);

    /* TBD: Check there is no overlap */

    /* Pass on the OR-ed bit fields to device queue for cleanup */
    device_queue_cleanup_pl(smp_dq_entries);

    return API_SUCCESS;
}
#endif /* STORAGE_CHECK_CONSISTENCY_ON_RESTORE */

#endif /* BT_STORAGE */
