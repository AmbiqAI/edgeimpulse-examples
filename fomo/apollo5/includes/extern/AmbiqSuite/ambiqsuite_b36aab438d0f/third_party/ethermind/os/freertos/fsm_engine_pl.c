
/**
 *  \file fsm_engine_pl.c
 *
 *  This file implementats the Routines to Invoke FSM Event Handlers.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* ----------------------------------------- Header File Inclusion */
#include "fsm_defines.h"
#include "fsm_engine.h"
#include "att_fsm_defines.h"
#include "smp_fsm_engine.h"

#ifdef APPL_FSM_SUPPORT
#include "appl_fsm_defines.h"
#endif /* APPL_FSM_SUPPORT */

/* ----------------------------------------- External Global Variables */

/* ----------------------------------------- Exported Global Variables */

/* ----------------------------------------- Static Global Variables */

DECL_CONST FSM_MODULE_TABLE_T  fsm_module_table[FSM_MAX_MODULE] =
{
    /* ATT Module Details */
    {
        att_access_state_handler,

        NULL,

        att_event_handler_table,

        att_state_event_table,

        ATT_MAX_NUM_EVENTS
    },

#ifdef SMP
    /* SMP Module Details */
    {
        smp_access_state_handler,

        NULL,

        smp_event_handler_table,

        smp_state_event_table,

        SMP_MAX_NUM_EVENTS
    },
#endif /* SMP */

#ifdef APPL_FSM_SUPPORT
    /* APPL Module Details */
    {
        appl_access_state_handler,

        NULL,

        appl_event_handler_table,

        appl_state_event_table,

        APPL_MAX_NUM_EVENTS
    }
#endif /* APPL_FSM_SUPPORT */
};
