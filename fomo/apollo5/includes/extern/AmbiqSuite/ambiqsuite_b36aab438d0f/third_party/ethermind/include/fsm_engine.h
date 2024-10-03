
/**
 *  \file fsm_engine.h
 *
 *  This file defines interface offered by the FSM module.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_FSM_ENGINE_
#define _H_FSM_ENGINE_

/* --------------------------------------------- Header File Inclusion */
#include "BT_common.h"
#include "fsm_defines.h"

#define FSM_MAX_MODULE  3

#define ATT_FSM_ID      0x00

#ifdef SMP
#define SMP_FSM_ID      0x01

#ifdef APPL_FSM_SUPPORT
#define APPL_FSM_ID     0x02
#endif /* APPL_FSM_SUPPORT */
#else /* SMP */

#ifdef APPL_FSM_SUPPORT
#define APPL_FSM_ID     0x01
#endif /* APPL_FSM_SUPPORT */
#endif /* SMP */

#ifndef FSM_NO_DEBUG
#ifdef VAR_ARG_IN_MACRO_NOT_SUPPORTED
#define FSM_ERR
#else
#define FSM_ERR(...)       BT_debug_error(BT_MODULE_ID_FSM,__VA_ARGS__)
#endif /* VAR_ARG_IN_MACRO_NOT_SUPPORTED */
#else  /* FSM_NO_DEBUG */
#define FSM_ERR            BT_debug_null
#endif /* FSM_NO_DEBUG */

#ifdef FSM_DEBUG

#define FSM_TRC(...)       BT_debug_trace(BT_MODULE_ID_FSM,__VA_ARGS__)
#define FSM_INF(...)       BT_debug_info(BT_MODULE_ID_FSM,__VA_ARGS__)

#else /* FSM_DEBUG */
#ifdef VAR_ARG_IN_MACRO_NOT_SUPPORTED
#define FSM_TRC
#define FSM_INF
#else
#define FSM_TRC            BT_debug_null
#define FSM_INF            BT_debug_null
#endif /* VAR_ARG_IN_MACRO_NOT_SUPPORTED */

#endif /* FSM_DEBUG */

/* --------------------------------------------- Functions */
#ifdef __cplusplus
extern "C"{
#endif

API_RESULT fsm_register_module (UCHAR fsm_id, FSM_MODULE_TABLE_T * module_fsm);

API_RESULT fsm_post_event
           (
               UCHAR     fsm_id,
               EVENT_T   fsm_event,
               void      * param
           );

#ifdef __cplusplus
};
#endif

#endif /* _H_FSM_ENGINE_ */

