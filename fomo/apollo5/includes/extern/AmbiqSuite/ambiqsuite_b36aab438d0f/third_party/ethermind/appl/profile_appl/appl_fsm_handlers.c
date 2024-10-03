
/**
 *  \file appl_fsm_handlers.c
 *
 *  This file contains event handlers for application specific FSM.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#include "BT_common.h"
#include "appl.h"
#include "appl_fsm_handlers.h"
#include "appl_fsm.h"

/* #define APPL_GAP_ADV appl_gap_stop_advertising */

API_RESULT dummy_handler
           (
               APPL_EVENT_PARAM * param
           )
{
    APPL_TRC(
    "[APPL]:>>>> dummy_handler\n");

    APPL_TRC(
    "[APPL]:<<<< dummy_handler\n");

    return API_SUCCESS;
}


API_RESULT se_appl_device_init_handler
           (
               APPL_EVENT_PARAM    * param
           )
{
    API_RESULT         retval;

    APPL_TRC (
    "[APPL]: >>> se_appl_device_init_handler\n");

    /**
     *  Device Initialization procedures, intiailizing hardware, sensors
     *  Initialize GATT Data Base.
     *  Initialize transport. Here Bluetooth.
     */

    /* Initializing GATT DB Module */
    BT_gatt_db_init ();

    /** Intialize Profiles and services application(s) */
    APPL_PROFILE_INIT ();

    retval = BT_bluetooth_on
             (
                 appl_hci_event_indication_callback,
                 appl_bluetooth_on_complete,
                 NULL
             );

    if (retval != API_SUCCESS)
    {
        APPL_ERR (
        "*** FAILED to Switch Bluetooth ON\n");
        return API_FAILURE;
    }

    APPL_TRC (
    "[APPL]: <<< se_appl_device_init_handler\n");

    return retval;
}


API_RESULT se_appl_power_on_complete_handler
           (
               APPL_EVENT_PARAM    * param
           )
{
    API_RESULT          retval;
    APPL_EVENT_PARAM    fsm_event_params;

    APPL_TRC (
    "[APPL]: >>> se_appl_power_on_complete_handler\n");

    SET_ALL_APPL_STATE(SL_0_OPERATING);
    /**
     *  Note: ATT Registration has been moved from here as this action is
     *  required on coming back to life after entry to sleep mode. However as
     *  sleep mode state is not a part of Application FSM yet, this is moved
     *  to initialization procedure in appl.c
     */
    /**
     *  Take any other actions like turing on LED to indicate device is ready
     *  for use now etc here.
     *  Request Initializing the transport for measurement transfer.
     *  Can include parameters to provided details like enter General Discoverable
     *  Mode or etc as event parameters.
     */

    APPL_START_IDLE_TIMER ();

    APPL_EVENT_PARAM_INIT (&fsm_event_params);

    retval = fsm_post_event
             (
                 APPL_FSM_ID,
                 ev_appl_transport_connect_req,
                 &fsm_event_params
             );

    APPL_TRC (
    "[APPL]: <<< se_appl_power_on_complete_handler\n");

    return retval;
}


API_RESULT se_appl_transport_connect_req_in_operating_handler
           (
               APPL_EVENT_PARAM    * param
           )
{
    API_RESULT   retval;

    APPL_TRC (
    "[APPL]: >>> se_appl_transport_connect_req_in_operating_handler\n");

    SET_ALL_APPL_STATE(SL_0_TRANSPORT_INITIALIZATION);

    /**
     *  Transport connect request may be in context of preparing for
     *  a preknown device or inviting connection from fresh devices. These details
     *  should be a part of event parameters.
     *  Any user notification (LED) to indicate transport connection in progress should
     *  also be included here.
     *  Start Idle timer to detect no activity and save power. (TODO)
     */
    APPL_PERIPHERAL_EVENT_HANDLER_P1
    (
         appl_strt_nrml_undrct_cntbl_adv,
         retval,
         APPL_ADV_DATA_ID
    );

    APPL_CENTRAL_EVENT_HANDLER_P0
    (
         appl_strt_nrml_psv_scan,
         retval
    );

    APPL_TRC (
    "[APPL]: <<< se_appl_transport_connect_req_in_operating_handler\n");

    return retval;
}


API_RESULT se_appl_transport_connect_req_handler
           (
               APPL_EVENT_PARAM    * param
           )
{
#ifdef APPL_GAP_CENTRAL_SUPPORT
    API_RESULT retval;

    APPL_CENTRAL_EVENT_HANDLER_P0
    (
         appl_gap_stop_scanning,
         retval
    );

    APPL_TRC (
    "Initiating Connection ...\n");
    APPL_CENTRAL_EVENT_HANDLER_P1
    (
         appl_init_fast_connection,
         retval,
         (void *) param->data_param
    );

    if (API_SUCCESS != retval)
    {
        APPL_ERR (
        "Failed to create LE connection with %02X:%02X:%02X:%02X:%02X:%02X, "
        "reason 0x%04X\n", ((BT_DEVICE_ADDR *)param->data_param)->addr[0],
        ((BT_DEVICE_ADDR *)param->data_param)->addr[1],
        ((BT_DEVICE_ADDR *)param->data_param)->addr[2],
        ((BT_DEVICE_ADDR *)param->data_param)->addr[3],
        ((BT_DEVICE_ADDR *)param->data_param)->addr[4],
        ((BT_DEVICE_ADDR *)param->data_param)->addr[5],
        retval);
    }

    APPL_TRC (
    "[APPL]: <<< se_appl_transport_connect_req_handler\n");
#endif /* APPL_GAP_CENTRAL_SUPPORT */

    return API_SUCCESS;
}


API_RESULT se_appl_transport_connection_complete_handler
           (
               APPL_EVENT_PARAM    * param
           )
{
    API_RESULT    retval;

    APPL_TRC (
    "[APPL]: >>> se_appl_transport_connection_complete_handler\n");

    SET_APPL_STATE(param->handle,SL_0_TRANSPORT_CONNECTED);

    appl_active_connections++;

    if (ATT_MAX_CONNECTION_INSTANCES == appl_active_connections)
    {
        APPL_PERIPHERAL_EVENT_HANDLER_P0
        (
             appl_gap_stop_advertising,
             retval
        );

        APPL_CENTRAL_EVENT_HANDLER_P0
        (
             appl_gap_stop_scanning,
             retval
        );
    }
    else
    {
        /* Start Advertising again! */
            APPL_PERIPHERAL_EVENT_HANDLER_P1
            (
                 appl_strt_nrml_undrct_cntbl_adv,
                 retval,
                 APPL_ADV_DATA_ID
            );

            APPL_CENTRAL_EVENT_HANDLER_P0
            (
                 appl_strt_nrml_psv_scan,
                 retval
            );
    }

    /**
     *  Health Thermometer Sensor is ready for measurement transfer.
     *  Provide appropriate notification to indicate the user of the same.
     *  If any buffered measurements are present and the Sensor is configured
     *  by the Collector to send measurements, transfer measurements to the
     *  Collector. Caution (may to ensure needed security exists on transport
     *  before intiating measurement transfer.
     *  Turn Off Idle Timer. (TODO)
     */
    APPL_STOP_IDLE_TIMER ();

    APPL_TRC (
    "[APPL]: <<< se_appl_transport_connection_complete_handler\n");

    return retval;
}

API_RESULT se_appl_transport_operating_ind_handler
           (
               APPL_EVENT_PARAM    * param
           )
{
    APPL_TRC (
    "[APPL]: >>> se_appl_transport_operating_ind_handler");

    SET_APPL_STATE(param->handle,SL_0_TRANSPORT_OPERATING);

    APPL_PROFILE_CONNECT (&param->handle);

    appl_update_battery_lvl(&param->handle);

    APPL_TRC (
    "[APPL]: <<< se_appl_transport_operating_ind_handler");

    return API_SUCCESS;
}


API_RESULT se_appl_transport_suspend_ind_handler
           (
               APPL_EVENT_PARAM    * param
           )
{
    APPL_TRC (
    "[APPL]: >>> se_appl_transport_suspend_ind_handler");

    SET_APPL_STATE(param->handle,SL_0_TRANSPORT_SUSPEND);

    APPL_PROFILE_DISCONNECT_HANDLER (&param->handle);

    APPL_TRC (
    "[APPL]: <<< se_appl_transport_suspend_ind_handler");

    return API_SUCCESS;
}


API_RESULT se_appl_transport_config_req_handler
           (
               APPL_EVENT_PARAM    * param
           )
{
#ifdef SMP
    SMP_AUTH_INFO * auth;

    APPL_TRC (
    "[APPL]: >>> se_appl_transport_config_req_handler\n");

    if (0x00 == param->direction)
    {
        /* Configuration request for SMP, Initiate Authentication */
        auth = (SMP_AUTH_INFO *)param->data_param;
        appl_smp_security_request(&(APPL_GET_DEVICE_HANDLE(param->handle)), auth);
    }

    /* Update state */
    SET_APPL_STATE(param->handle, SL_0_TRANSPORT_CONFIGURATION);
#endif /* SMP */

    APPL_TRC (
    "[APPL]: <<< se_appl_transport_config_req_handler\n");

    return API_SUCCESS;
}


API_RESULT se_appl_transport_config_complete_handler
           (
               APPL_EVENT_PARAM    * param
           )
{
    APPL_TRC (
    "[APPL]: >>> se_appl_transport_config_complete_handler\n");

    /* Update state */
    SET_APPL_STATE(param->handle,SL_0_TRANSPORT_OPERATING);

    /* SMP Authentication result in direction member of FSM Event data */
    if (0x00 == param->direction)
    {
        /* Authentication Success */
        APPL_PROFILE_CONNECT(&param->handle);
    }

    APPL_TRC (
    "[APPL]: <<< se_appl_transport_config_complete_handler\n");

    return API_SUCCESS;
}


API_RESULT se_appl_error_handler
           (
               APPL_EVENT_PARAM    * param
           )
{
    APPL_TRC (
    "[APPL]: >>> se_appl_error_handler\n");

    APPL_TRC (
    "[APPL]: <<< se_appl_error_handler\n");

    return API_SUCCESS;
}


API_RESULT se_appl_transport_disconnection_complete_handler
           (
               APPL_EVENT_PARAM    * param
           )
{
    API_RESULT    retval;

    APPL_TRC (
    "[APPL]: >>> se_appl_transport_disconnection_complete_handler\n");

    SET_APPL_STATE(param->handle,SL_0_OPERATING);

    appl_active_connections--;
    retval = API_SUCCESS;

    /**
     *  For proximity use case, User needs to be alerted of Link Loss
     *  Restart Idle Timer if no other active link exists
     */
    if (ATT_MAX_CONNECTION_INSTANCES > appl_active_connections)
    {
         APPL_START_IDLE_TIMER ();

         SET_APPL_STATE(param->handle,SL_0_TRANSPORT_INITIALIZATION);

         APPL_PERIPHERAL_EVENT_HANDLER_P1
         (
              appl_strt_nrml_undrct_cntbl_adv,
              retval,
              APPL_ADV_DATA_ID
         );

         APPL_CENTRAL_EVENT_HANDLER_P0
         (
              appl_strt_nrml_psv_scan,
              retval
         );
    }

#ifdef TRANSPORT_CALLBACK
    APPL_TRANSPORT_DISCONNECT_HANDLER (&param->handle);
#endif /* TRANSPORT_CALLBACK */

    APPL_TRC (
    "[APPL]: <<< se_appl_transport_disconnection_complete_handler\n");

    return retval;
}


API_RESULT se_appl_measurement_req_handler
           (
               APPL_EVENT_PARAM    * param
           )
{
    APPL_TRC (
    "[APPL]: >>> se_appl_measurement_req_handler");

    APPL_TRC (
    "[APPL]: <<< se_appl_measurement_req_handler");

    return API_SUCCESS;
}


API_RESULT se_appl_measurement_ind_handler
           (
               APPL_EVENT_PARAM    * param
           )
{
    APPL_TRC (
    "[APPL]: >>> se_appl_measurement_ind_handler\n");

    /**
     *  Measurement from sensor is available
     *     Either Tranfer or
     *     Store
     *  based on specification and use case.
     */
    APPL_SEND_MEASUREMENT (&param->handle);

    APPL_TRC (
    "[APPL]: <<< se_appl_measurement_ind_handler\n");

    return API_SUCCESS;
}


API_RESULT se_appl_measurement_ind_in_non_operating_state
           (
               APPL_EVENT_PARAM    * param
           )
{
    APPL_TRC (
    "[APPL]: >>> se_appl_measurement_ind_in_non_operating_state\n");

    APPL_TRC (
    "[APPL]: <<< se_appl_measurement_ind_in_non_operating_state\n");

    return API_SUCCESS;
}



API_RESULT se_appl_data_transfer_req_handler
           (
               APPL_EVENT_PARAM    * param
           )
{
    APPL_TRC (
    "[APPL]: >>> se_appl_data_transfer_req_handler\n");

    APPL_TRC (
    "[APPL]: <<< se_appl_data_transfer_req_handler\n");

    return API_SUCCESS;
}


API_RESULT se_appl_data_transfer_complete_handler
           (
               APPL_EVENT_PARAM    * param
           )
{
    APPL_TRC (
    "[APPL]: >>> se_appl_data_transfer_complete_handler\n");

    APPL_TRC (
    "[APPL]: <<< se_appl_data_transfer_complete_handler\n");

    return API_SUCCESS;
}


API_RESULT se_appl_low_power_entry_req_handler
           (
               APPL_EVENT_PARAM    * param
           )
{
    API_RESULT    retval;

    APPL_TRC (
    "[APPL]: >>> se_appl_low_power_entry_req_handler\n");

    SET_ALL_APPL_STATE(SL_0_LOW_POWER);

    /**
     *  Application could chose to advertise/scan less aggressively to save
     *  power or could enter deep sleep mode and exit the mode on user action
     */
    APPL_PERIPHERAL_EVENT_HANDLER_P1
    (
         appl_strt_lp_undrct_cntbl_adv,
         retval,
         APPL_ADV_DATA_ID
    );

    APPL_CENTRAL_EVENT_HANDLER_P0
    (
         appl_strt_lp_psv_scan,
         retval
    );

    APPL_TRC (
    "[APPL]: <<< se_appl_low_power_entry_req_handler\n");

    return retval;
}


API_RESULT se_appl_low_power_exit_req_handler
           (
               APPL_EVENT_PARAM    * param
           )
{
    API_RESULT    retval;

    APPL_TRC (
    "[APPL]: >>> se_appl_low_power_exit_req_handler\n");

    SET_ALL_APPL_STATE(SL_0_OPERATING);

    APPL_PERIPHERAL_EVENT_HANDLER_P1
    (
         appl_strt_nrml_undrct_cntbl_adv,
         retval,
         APPL_ADV_DATA_ID
    );

    APPL_CENTRAL_EVENT_HANDLER_P0
    (
         appl_strt_nrml_psv_scan,
         retval
    );

    APPL_TRC (
    "[APPL]: <<< se_appl_low_power_exit_req_handler\n");

    return retval;
}


