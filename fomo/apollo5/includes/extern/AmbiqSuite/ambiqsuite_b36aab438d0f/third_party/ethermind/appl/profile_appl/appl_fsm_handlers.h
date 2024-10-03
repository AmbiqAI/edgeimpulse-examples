
/**
 *  \file appl_fsm_handlers.h
 *
 *  This file contains function handler declarations for APPL 20601 FSM.
 *
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_FSM_HANDLERS_
#define _H_APPL_FSM_HANDLERS_

#include "appl_fsm_defines.h"

#ifdef __cplusplus
extern "C" {
#endif



#ifdef APPL_GAP_CENTRAL_SUPPORT
#define APPL_CENTRAL_EVENT_HANDLER_P0(hndlr,rtvl)\
        {\
            (rtvl) = hndlr();\
        }

#define APPL_CENTRAL_EVENT_HANDLER_P1(hndlr,rtvl,p1)\
        {\
            (rtvl) = hndlr((p1));\
        }

#ifdef VAR_ARG_IN_MACRO_NOT_SUPPORTED
#define APPL_PERIPHERAL_EVENT_HANDLER_P0(hndlr,rtvl)\
        {\
            (rtvl) = hndlr();\
        }
#define APPL_PERIPHERAL_EVENT_HANDLER_P1(hndlr,rtvl,p1)\
        {\
            (rtvl) = hndlr((p1));\
        }
#else
#define APPL_PERIPHERAL_EVENT_HANDLER_P0(...)
#define APPL_PERIPHERAL_EVENT_HANDLER_P1(...)
#endif  /*  VAR_ARG_IN_MACRO_NOT_SUPPORTED */

#else /* APPL_GAP_CENTRAL_SUPPORT */

#ifdef APPL_GAP_PERIPHERAL_SUPPORT
#define APPL_PERIPHERAL_EVENT_HANDLER_P0(hndlr,rtvl)\
        {\
            (rtvl) = hndlr();\
        }
#define APPL_PERIPHERAL_EVENT_HANDLER_P1(hndlr,rtvl,p1)\
        {\
            (rtvl) = hndlr((p1));\
        }

#ifdef VAR_ARG_IN_MACRO_NOT_SUPPORTED
#define APPL_CENTRAL_EVENT_HANDLER_P0(hndlr,rtvl)\
        {\
            (rtvl) = hndlr();\
        }

#define APPL_CENTRAL_EVENT_HANDLER_P1(hndlr,rtvl,p1)\
        {\
            (rtvl) = hndlr((p1));\
        }
#else
#define APPL_CENTRAL_EVENT_HANDLER_P0(...)
#define APPL_CENTRAL_EVENT_HANDLER_P1(...)
#endif  /*  VAR_ARG_IN_MACRO_NOT_SUPPORTED */

#endif /* APPL_GAP_PERIPHERAL_SUPPORT */
#endif /* APPL_GAP_CENTRAL_SUPPORT */

API_RESULT dummy_handler
           (
               APPL_EVENT_PARAM * param
           );

API_RESULT se_appl_device_init_handler
           (
               APPL_EVENT_PARAM *    param
           );
API_RESULT se_appl_power_on_complete_handler
           (
               APPL_EVENT_PARAM *    param
           );
API_RESULT se_appl_transport_connect_req_handler
           (
               APPL_EVENT_PARAM *    param
           );
API_RESULT se_appl_transport_connect_req_in_operating_handler
           (
               APPL_EVENT_PARAM *    param
           );
API_RESULT se_appl_transport_connection_complete_handler
           (
               APPL_EVENT_PARAM *    param
           );
API_RESULT se_appl_transport_operating_ind_handler
           (
               APPL_EVENT_PARAM *    param
           );
API_RESULT se_appl_transport_suspend_ind_handler
           (
               APPL_EVENT_PARAM *    param
           );
API_RESULT se_appl_transport_config_req_handler
           (
               APPL_EVENT_PARAM *    param
           );
API_RESULT se_appl_transport_config_complete_handler
           (
               APPL_EVENT_PARAM *    param
           );
API_RESULT se_appl_error_handler
           (
               APPL_EVENT_PARAM *    param
           );
API_RESULT se_appl_transport_disconnection_complete_handler
           (
               APPL_EVENT_PARAM *    param
           );
API_RESULT se_appl_measurement_req_handler
           (
               APPL_EVENT_PARAM *    param
           );
API_RESULT se_appl_measurement_ind_handler
           (
               APPL_EVENT_PARAM *    param
           );
API_RESULT se_appl_measurement_ind_in_non_operating_state
           (
               APPL_EVENT_PARAM *    param
           );
API_RESULT se_appl_data_transfer_req_handler
           (
               APPL_EVENT_PARAM *    param
           );
API_RESULT se_appl_data_transfer_complete_handler
           (
               APPL_EVENT_PARAM *    param
           );
API_RESULT se_appl_low_power_entry_req_handler
           (
               APPL_EVENT_PARAM *    param
           );
API_RESULT se_appl_low_power_exit_req_handler
           (
               APPL_EVENT_PARAM *    param
           );

#ifdef __cplusplus
};
#endif

#endif /* _H_APPL_FSM_HANDLERS_ */

