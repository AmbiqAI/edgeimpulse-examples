
/**
 *  \file BT_storage.h
 *
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_BT_STORAGE_
#define _H_BT_STORAGE_

/* --------------------------------------------- Header File Inclusion */
#include "BT_common.h"

/* --------------------------------------------- Global Definitions */
/* Storage types currently supported */
#define STORAGE_TYPE_PERSISTENT                  0
#ifndef STORAGE_RETENTION_SUPPORT
#define STORAGE_NUM_TYPES                        1
#else /* STORAGE_RETENTION_SUPPORT */
#define STORAGE_TYPE_RETENTION                   1
#define STORAGE_NUM_TYPES                        2
#endif /* STORAGE_RETENTION_SUPPORT */

/* Storage Operations */
#define STORAGE_STORE                            0x00
#define STORAGE_LOAD                             0x01

/* Core module persistent storage IDs */
#define STORAGE_DQ_PID                           0
#define STORAGE_SMP_PID                          1

/* Number of core module persistent storage IDs */
#define STORAGE_NUM_PIDS                         2

/* Core module retention storage IDs */
#define STORAGE_HCI_RID                          0
#define STORAGE_L2CAP_RID                        1
#define STORAGE_DQ_RID                           2
#define STORAGE_SMP_RID                          3
#define STORAGE_ATT_RID                          4

/* Number of core module retention storage IDs */
#define STORAGE_NUM_RIDS                         5

/**
 *  Storage Events
 *  - AUTH_UPDATE
 *  - SHUTDOWN
 *  - OTHERS
 *
 *  Many granular events can be defined in the future, based on the requirement.
 */

/* Storage Events */
/* Authentication Update */
#define STORAGE_EVENT_AUTH_UPDATE                 0x0001

/* Shutdown */
#define STORAGE_EVENT_SHUTDOWN                    0x0002

/* Others */
#define STORAGE_EVENT_OTHERS                      0x0004

/* All events - used as mask to enable all storage events */
#define STORAGE_EVENTS_ALL                        \
            (STORAGE_EVENT_AUTH_UPDATE) |         \
            (STORAGE_EVENT_SHUTDOWN) |            \
            (STORAGE_EVENT_OTHERS)

/* --------------------------------------------- Structures/Data Types */
typedef void (*STORAGE_CB)(UCHAR id, UCHAR type, UCHAR action) DECL_REENTRANT;

/* --------------------------------------------- Macros */

/* --------------------------------------------- Internal Functions */

/* --------------------------------------------- API Declarations */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef STORAGE_HAVE_EVENT_MASK
/**
 *  API to enable storage events
 */
API_RESULT BT_storage_enable_events
           (
               UINT16    storage_events
           );
#endif /* STORAGE_HAVE_EVENT_MASK */

API_RESULT BT_storage_register_db
           (
               /* IN */ UCHAR  id,
               /* IN */ UCHAR  type,
               /* IN */ STORAGE_CB cb
           );

API_RESULT BT_storage_update_db
           (
               /* IN */ UCHAR id,
               /* IN */ UCHAR type,
               /* IN */ STORAGE_CB cb
           );

API_RESULT BT_storage_store_db
           (
               /* IN */ UCHAR type,
               /* IN */ UINT16 event_type
           );

API_RESULT BT_storage_load_db (UCHAR type);

UINT16 BT_storage_write
       (
           UCHAR id,
           UCHAR type,
           void * data_param,
           UINT16 datalen
       );

UINT16 BT_storage_read
       (
           UCHAR id,
           UCHAR type,
           void * data_param,
           UINT16 datalen
       );

#ifdef __cplusplus
};
#endif

#endif /* _H_BT_STORAGE_ */
