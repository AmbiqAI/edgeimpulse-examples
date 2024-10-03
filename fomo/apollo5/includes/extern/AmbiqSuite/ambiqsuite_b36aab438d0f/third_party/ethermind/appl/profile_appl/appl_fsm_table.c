
/**
 *  \file appl_fsm_table.c
 *
 *  This file contains APPL FSM - state/event transitions.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl_fsm_handlers.h"
#include "appl_fsm.h"

#ifdef PRINT_STATES
/* Get the State Name based upon the Bit Mask */
void print_state_name(UINT32 state_bitmap, UINT16 level)
{
    APPL_TRC("S[%d] = %x ", level, state_bitmap);

    switch (level)
    {
        case 0: /* State Level 0 */
        {
            if (SL_0_IDLE == (state_bitmap & SL_0_IDLE))
            {
                APPL_TRC("(SL_0_IDLE)");
            }
            if (SL_0_POWERING_ON == (state_bitmap & SL_0_POWERING_ON))
            {
                APPL_TRC("(SL_0_POWERING_ON)");
            }
            if (SL_0_OPERATING == (state_bitmap & SL_0_OPERATING))
            {
                APPL_TRC("(SL_0_OPERATING)");
            }
            if (SL_0_TRANSPORT_INITIALIZATION == (state_bitmap & SL_0_TRANSPORT_INITIALIZATION))
            {
                APPL_TRC("(SL_0_TRANSPORT_INITIALIZATION)");
            }
            if (SL_0_TRANSPORT_CONNECTED == (state_bitmap & SL_0_TRANSPORT_CONNECTED))
            {
                APPL_TRC("(SL_0_TRANSPORT_CONNECTED)");
            }
            if (SL_0_TRANSPORT_CONFIGURATION == (state_bitmap & SL_0_TRANSPORT_CONFIGURATION))
            {
                APPL_TRC("(SL_0_TRANSPORT_CONFIGURATION)");
            }
            if (SL_0_TRANSPORT_OPERATING == (state_bitmap & SL_0_TRANSPORT_OPERATING))
            {
                APPL_TRC("(SL_0_TRANSPORT_OPERATING)");
            }
            if (SL_0_TRANSPORT_SUSPEND == (state_bitmap & SL_0_TRANSPORT_SUSPEND))
            {
                APPL_TRC("(SL_0_TRANSPORT_SUSPEND)");
            }
            if (SL_0_TRANSPORT_DISCONNECTING == (state_bitmap & SL_0_TRANSPORT_DISCONNECTING))
            {
                APPL_TRC("(SL_0_TRANSPORT_DISCONNECTING)");
            }
            if (SL_0_DATA_AVAILABLE == (state_bitmap & SL_0_DATA_AVAILABLE))
            {
                APPL_TRC("(SL_0_DATA_AVAILABLE)");
            }
            if (SL_0_DATA_TRANSFER == (state_bitmap & SL_0_DATA_TRANSFER))
            {
                APPL_TRC("(SL_0_DATA_TRANSFER)");
            }
            if (SL_0_LOW_POWER == (state_bitmap & SL_0_LOW_POWER))
            {
                APPL_TRC("(SL_0_LOW_POWER)");
            }
            if (SL_0_POWERING_OFF == (state_bitmap & SL_0_POWERING_OFF))
            {
                APPL_TRC("(SL_0_POWERING_OFF)");
            }
        }
        break;
    }

    APPL_TRC("\n");
    return;
}
#endif /* PRINT_STATES */

DECL_CONST DECL_CONST_QUALIFIER STATE_EVENT_TABLE appl_state_event_table[] =
{
    {
        /*0*/ ev_appl_device_init_req,
        0,
        0
    },
    {
        /*1*/ ev_appl_power_on_complete_ind,
        1,
        1
    },
    {
        /*2*/ ev_appl_transport_connect_req,
        3,
        2
    },
    {
        /*3*/ ev_appl_transport_connected_ind,
        4,
        4
    },
    {
        /*4*/ ev_appl_transport_operating_ind,
        5,
        5
    },
    {
        /*5*/ ev_appl_transport_suspend_ind,
        6,
        6
    },
    {
        /*6*/ ev_appl_transport_configuration_req,
        7,
        7
    },
    {
        /*7*/ ev_appl_transport_configuration_ind,
        8,
        8
    },
    {
        /*8*/ ev_appl_transport_disconnection_req,
        9,
        9
    },
    {
        /*9*/ ev_appl_transport_disconnected_ind,
        10,
        10
    },
    {
        /*10*/ ev_appl_measurement_req,
        11,
        11
    },
    {
        /*11*/ ev_appl_measurement_ind,
        13,
        12
    },
    {
        /*12*/ ev_appl_data_transfer_req,
        14,
        14
    },
    {
        /*13*/ ev_appl_data_transfer_complete_ind,
        15,
        15
    },
    {
        /*14*/ ev_appl_low_power_entry_req,
        16,
        16
    },
    {
        /*15*/ ev_appl_low_power_exit_req,
        17,
        17
    },
    {
        /*16*/ ev_appl_error_ind,
        18,
        18
    }
};

DECL_CONST DECL_CONST_QUALIFIER EVENT_HANDLER_TABLE appl_event_handler_table[] =
{
    {
        /*0*/ SL_0_IDLE,
        (SE_HNDLR_T )se_appl_device_init_handler
    },
    {
        /*1*/ SL_0_IDLE,
        (SE_HNDLR_T )se_appl_power_on_complete_handler
    },
    {
        /*2*/ SL_0_OPERATING | SL_0_LOW_POWER,
        (SE_HNDLR_T )se_appl_transport_connect_req_in_operating_handler
    },
    {
        /*3*/ SL_0_TRANSPORT_INITIALIZATION,
        (SE_HNDLR_T )se_appl_transport_connect_req_handler
    },
    {
        /*4*/ SL_0_TRANSPORT_INITIALIZATION | SL_0_LOW_POWER,
        (SE_HNDLR_T )se_appl_transport_connection_complete_handler
    },
    {
        /*5*/ SL_0_TRANSPORT_CONNECTED,
        (SE_HNDLR_T )se_appl_transport_operating_ind_handler
    },
    {
        /*6*/ SL_0_TRANSPORT_OPERATING,
        (SE_HNDLR_T )se_appl_transport_suspend_ind_handler
    },
    {
        /*7*/ SL_0_TRANSPORT_CONNECTED | SL_0_TRANSPORT_OPERATING | SL_0_TRANSPORT_SUSPEND,
        (SE_HNDLR_T )se_appl_transport_config_req_handler
    },
    {
        /*8*/ SL_0_TRANSPORT_CONFIGURATION,
        (SE_HNDLR_T )se_appl_transport_config_complete_handler
    },
    {
        /*9*/ SL_0_TRANSPORT_CONNECTED | SL_0_TRANSPORT_CONFIGURATION | SL_0_TRANSPORT_SUSPEND,
        (SE_HNDLR_T )se_appl_error_handler
    },
    {
        /*10*/ SL_0_TRANSPORT_CONNECTED | SL_0_TRANSPORT_CONFIGURATION | SL_0_TRANSPORT_OPERATING | SL_0_TRANSPORT_SUSPEND | SL_0_TRANSPORT_DISCONNECTING | SL_0_OPERATING | SL_0_DATA_AVAILABLE | SL_0_DATA_TRANSFER | SL_0_LOW_POWER | SL_0_POWERING_OFF,
        (SE_HNDLR_T )se_appl_transport_disconnection_complete_handler
    },
    {
        /*11*/ SL_0_OPERATING | SL_0_TRANSPORT_CONNECTED | SL_0_TRANSPORT_OPERATING,
        (SE_HNDLR_T )se_appl_measurement_req_handler
    },
    {
        /*12*/ SL_0_TRANSPORT_OPERATING,
        (SE_HNDLR_T )se_appl_measurement_ind_handler
    },
    {
        /*13*/ SL_0_OPERATING | SL_0_TRANSPORT_CONNECTED | SL_0_TRANSPORT_CONFIGURATION | SL_0_TRANSPORT_DISCONNECTING | SL_0_LOW_POWER | SL_0_POWERING_OFF,
        (SE_HNDLR_T )se_appl_measurement_ind_in_non_operating_state
    },
    {
        /*14*/ SL_0_TRANSPORT_OPERATING,
        (SE_HNDLR_T )se_appl_data_transfer_req_handler
    },
    {
        /*15*/ SL_0_DATA_TRANSFER,
        (SE_HNDLR_T )se_appl_data_transfer_complete_handler
    },
    {
        /*16*/ SL_0_OPERATING | SL_0_TRANSPORT_INITIALIZATION,
        (SE_HNDLR_T )se_appl_low_power_entry_req_handler
    },
    {
        /*17*/ SL_0_LOW_POWER,
        (SE_HNDLR_T )se_appl_low_power_exit_req_handler
    },
    {
        /*18*/ SL_0_OPERATING | SL_0_TRANSPORT_INITIALIZATION | SL_0_TRANSPORT_CONNECTED | SL_0_TRANSPORT_CONFIGURATION | SL_0_TRANSPORT_OPERATING | SL_0_TRANSPORT_SUSPEND | SL_0_TRANSPORT_DISCONNECTING | SL_0_DATA_AVAILABLE | SL_0_DATA_TRANSFER | SL_0_LOW_POWER | SL_0_POWERING_OFF,
        (SE_HNDLR_T )se_appl_error_handler
    }
};

