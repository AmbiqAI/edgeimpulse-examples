
/**
 *  \file appl_fsm_defines.h
 *
 *  This file defines state and events related to APPL FSM.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_FSM_DEFINES_
#define _H_APPL_FSM_DEFINES_

/* --------------------------------------------- Header File Inclusion */
#include "BT_common.h"
#include "fsm_defines.h"
#include "BT_att_api.h"
#include "BT_device_queue.h"

#define APPL_MAX_NUM_EVENTS     17

#define APPL_EVENT_PARAM_INIT(param)\
        APPL_HANDLE_INIT((param)->handle);\
        (param)->data_param = NULL;\
        (param)->data_len = 0;\

extern DECL_CONST STATE_EVENT_TABLE appl_state_event_table[];
extern DECL_CONST EVENT_HANDLER_TABLE appl_event_handler_table[];

/* --------------------------------------------- Structures/Data Types */

typedef UINT16 APPL_STATE_T;

/* This is the table for active on-going connection only */
typedef struct appl_peer_instance_t
{
    UINT16          cxn_handle;

    ATT_HANDLE      att_handle;

    APPL_STATE_T    state;

    UINT16          mtu;

}APPL_PEER_INSTANCE;

typedef UCHAR APPL_HANDLE;

typedef struct appl_event_param_t
{
    /* Event Data  */
    UCHAR                   * data_param;

    /* Event Data Length */
    UINT16                 data_len;

    /* Device Queue Handle */
    APPL_HANDLE            handle;

    /* Indicated whether the event is peer or application initiated */
    UCHAR                  direction;

}APPL_EVENT_PARAM;

/* --------------------------------------------- Functions */

API_RESULT appl_access_state_handler (void * param, STATE_T * state);


#endif /* _H_APPL_FSM_DEFINES_ */

