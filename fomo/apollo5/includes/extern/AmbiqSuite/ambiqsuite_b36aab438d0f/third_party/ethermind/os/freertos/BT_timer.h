
/**
 *  \file BT_timer.h
 *
 *  This Header File contains the APIs and the ADTs exported by the
 *  EtherMind Timer Library for Windows (User-mode).
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_BT_TIMER_
#define _H_BT_TIMER_


/* --------------------------------------------------- Header File Inclusion */
#include "BT_common.h"


/* --------------------------------------------------- Global Definitions */

/* Timer Handles must be initialized to this value */
#define BT_TIMER_HANDLE_INIT_VAL    NULL

/* Flag: Timer Entity State */
#define TIMER_ENTITY_FREE               0x00
#define TIMER_ENTITY_IN_USE             0x01
#define TIMER_ENTITY_IN_FREE            0x02

/* Flag: Timer Entity Data to be freed or not */
#define TIMER_ENTITY_HOLD_ALLOC_DATA    0x00
#define TIMER_ENTITY_FREE_ALLOC_DATA    0x01

/* ----------------------------------------------- Structures/Data Types */

/* Timer Entity */
typedef struct timer_entity_struct
{
    /* Callback to call when Timer expires */
    void (* callback) (void *, UINT16);

    /** Timer Callback Parameter */
    UCHAR  *allocated_data;

    /* Next Element in the Timer Q */
    struct timer_entity_struct *next;

    /* Argument to pass to callback */
#ifndef BT_NO_STATIC_DATA_SIZE
    UCHAR static_data[BT_STATIC_DATA_SIZE];
#endif /* BT_NO_STATIC_DATA_SIZE */
    /* BT_DEFINE_STATIC_DATA(static_data) */

    /* Time at which it is supposed to expire */
    UINT32 expire_at;

    /* Timeout Value asked by the User */
    UINT16 timeout;

    /* Length of the data */
    UINT16 data_length;

    /* Is this Entity Allocated ? */
    UCHAR in_use;

} TIMER_ENTITY;

typedef TIMER_ENTITY *  BT_timer_handle;



/* --------------------------------------------------- Function Declarations */

#ifdef __cplusplus
extern "C" {
#endif

void em_timer_init ( void );
void timer_bt_init ( void );
void timer_bt_shutdown ( void );

API_RESULT BT_start_timer
           (
               BT_timer_handle *handle,
               UINT16 timeout,
               void (* callback) (void *, UINT16),
               void *args, UINT16 size_args
           );

API_RESULT BT_restart_timer
           (
               BT_timer_handle handle,
               UINT16 new_timeout
           );

API_RESULT BT_stop_timer ( BT_timer_handle handle );

API_RESULT BT_is_active_timer ( BT_timer_handle handle );

/* Debug Routine - Internal Use Only */
API_RESULT BT_list_timer ( void );

#ifdef __cplusplus
};
#endif

#endif /* _H_BT_TIMER_ */

