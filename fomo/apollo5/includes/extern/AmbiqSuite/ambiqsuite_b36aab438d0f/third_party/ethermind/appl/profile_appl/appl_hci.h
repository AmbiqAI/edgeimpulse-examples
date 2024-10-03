
/**
 *  \file appl_hci.h
 *
 *  Header File for HCI Command Line Application
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_HCI_
#define _H_APPL_HCI_

/* ----------------------------------------- Header File Inclusion */
#include "appl.h"
#include "BT_hci_api.h"


/* ----------------------------------------- Global Definitions */


/* ----------------------------------------- Structures/Data Types */
typedef struct
{
    UINT16    opcode;
    char *    command_name;
} APPL_HCI_OPCODE_NAME;

/* The HCI Profile Event Indication Callback Type */
typedef struct
{
    /* Pointer to HCI callback function  */
    API_RESULT (* hci_event_ind_cb) (UINT8, UCHAR *, UINT8);

} APPL_HCI_CB_PTR;

/**
 *  Table to remember device details like the device queue handle,
 *  connection handle, this table is needed to mantain mapping between
 *  these two handles.
 *  NOTE: Device Address is not needed as it is possible to get it easily
 *  using device_queue_search_remote_addr routine.
 */
typedef struct
{
    /* Connection Handle for the Device */
    UINT16                  cxn_handle;

    /* Device Queue Handle for the device */
    DEVICE_HANDLE     dq_handle;

}APPL_HCI_DEVICE_INFO;


/* ----------------------------------------- Macros */
#define appl_hci_print_bd_addr(bd)      appl_dump_bytes((bd), 6);

/*
 *  Maximum number of HCI Profile Event Indication Callbacks that Profiles
 *  and Applications on top of HCI (EtherMind Stack) can register
 *  with HCI Application.
 *
 *  Minimum Value: 1
 *  Maximum Value: 254
 */
#define APPL_MAX_NUM_HCI_CB     2

/**
 *  To Lock APPL HCI Global Mutex.
 *  Since in the sample applications are called from the main task context
 *  this is just a place holder, and implement only if required.
 */
#define APPL_HCI_MUTEX_LOCK()

/**
 *  To Unlock APPL HCI Global Mutex.
 *  Since in the sample applications are called from the main task context
 *  this is just a place holder, and implement only if required.
 */
#define APPL_HCI_MUTEX_UNLOCK()

/* ----------------------------------------- Function Declarations */
/* HCI Application Event Indication Callback */
API_RESULT appl_hci_event_indication_callback
           (
               UCHAR  event_type,
               UCHAR  * event_data,
               UCHAR  event_datalen
           );


/* HCI API Handlers */
void appl_hci_disconnect (void);
void appl_hci_get_connection_details (void);
void appl_handle_connection_complete
     (
         UCHAR   * peer_addr,
         UCHAR   peer_addr_type,
         UINT16  connection_handle
     );
void appl_handle_conn_param_req(UCHAR *event_data);
char * appl_hci_get_command_name (UINT16 opcode);

#endif /* _H_APPL_HCI_ */

