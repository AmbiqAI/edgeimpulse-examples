
/**
 *  \file appl_fsm.h
 *
 *  This file defines state and events related to APPL FSM.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_FSM_
#define _H_APPL_FSM_


#if 0
char event_option[] =
"    --------- EVENTS ---------\n\
\n\
    0x01 -> ev_appl_device_init_req    \n\
    0x02 -> ev_appl_power_on_complete_ind    \n\
    0x03 -> ev_appl_transport_connect_req    \n\
    0x04 -> ev_appl_transport_connected_ind    \n\
    0x05 -> ev_appl_transport_configuration_req    \n\
    0x06 -> ev_appl_transport_operating_ind    \n\
    0x07 -> ev_appl_transport_suspend_ind    \n\
    0x08 -> ev_appl_transport_configuration_ind    \n\
    0x09 -> ev_appl_transport_disconnection_req    \n\
    0x0A -> ev_appl_transport_disconnected_ind    \n\
    0x0B -> ev_appl_measurement_req    \n\
    0x0C -> ev_appl_measurement_ind    \n\
    0x0D -> ev_appl_data_transfer_req    \n\
    0x0E -> ev_appl_data_transfer_complete_ind    \n\
    0x0F -> ev_appl_low_power_entry_req    \n\
    0x10 -> ev_appl_low_power_exit_req    \n\
    0xFF -> ev_appl_error_ind    \n\
    18 -> Exit    \n\
\n\
Your Option - ";
#endif



/* Event Defines */
typedef enum
{
    ev_appl_device_init_req = 0x01,
    ev_appl_power_on_complete_ind = 0x02,
    ev_appl_transport_connect_req = 0x03,
    ev_appl_transport_connected_ind = 0x04,
    ev_appl_transport_configuration_req = 0x05,
    ev_appl_transport_operating_ind = 0x06,
    ev_appl_transport_suspend_ind = 0x07,
    ev_appl_transport_configuration_ind = 0x08,
    ev_appl_transport_disconnection_req = 0x09,
    ev_appl_transport_disconnected_ind = 0x0A,
    ev_appl_measurement_req = 0x0B,
    ev_appl_measurement_ind = 0x0C,
    ev_appl_data_transfer_req = 0x0D,
    ev_appl_data_transfer_complete_ind = 0x0E,
    ev_appl_low_power_entry_req = 0x0F,
    ev_appl_low_power_exit_req = 0x10,
    ev_appl_error_ind = 0xFF

} EVENTS;

/* Level 0 State Defines */
typedef enum
{
    SL_0_IDLE = 0x0001,
    SL_0_POWERING_ON = 0x0002,
    SL_0_OPERATING = 0x0004,
    SL_0_TRANSPORT_INITIALIZATION = 0x0008,
    SL_0_TRANSPORT_CONNECTED = 0x0010,
    SL_0_TRANSPORT_CONFIGURATION = 0x0020,
    SL_0_TRANSPORT_OPERATING = 0x0040,
    SL_0_TRANSPORT_SUSPEND = 0x0080,
    SL_0_TRANSPORT_DISCONNECTING = 0x0100,
    SL_0_DATA_AVAILABLE = 0x0200,
    SL_0_DATA_TRANSFER = 0x0400,
    SL_0_LOW_POWER = 0x0800,
    SL_0_POWERING_OFF = 0x1000

} STATES_LEVEL_0;

#endif /* _H_APPL_FSM_ */

