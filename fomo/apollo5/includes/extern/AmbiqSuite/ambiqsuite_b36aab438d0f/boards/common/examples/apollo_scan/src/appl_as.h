//*****************************************************************************
//
//! @file appl_as.h
//!
//! @brief header file for Mindtree apollo scan application
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef APPL_AS_H
#define APPL_AS_H

#include <stdint.h>
#include <stdbool.h>
#include "BT_common.h"
#include "BT_hci_api.h"
#include "BT_att_api.h"
#include "BT_smp_api.h"
#include "smp_pl.h"
#include "l2cap.h"
#include "gatt_defines.h"
#include "comm.h"

typedef struct advData
{
    BT_DEVICE_ADDR advAddr;
    BT_DEVICE_ADDR rspAddr;
    UINT8 advLen;
    UINT8 rspLen;
    UINT8 advData[31];
    UINT8 rspData[31];
}advData_t;

/* TBD: Re-arrange function definition. Avoid explicit function declaration. */
void appl_poweron_stack(void);
void appl_master_menu_handler(void);
API_RESULT appl_bluetooth_on_complete_callback (void);
API_RESULT appl_register_callback(void);
API_RESULT appl_hci_callback
           (
               UCHAR    event_type,
               UCHAR  * event_data,
               UCHAR    event_datalen
           );
void appl_l2cap_callback
     (
         DEVICE_HANDLE * handle,
         UCHAR           event_type,
         UCHAR         * event_data,
         UINT16          event_datalen
     );
API_RESULT appl_att_callback
           (
               ATT_HANDLE    * handle,
               UCHAR           event_type,
               API_RESULT      event_result,
               UCHAR         * event_data,
               UINT16          event_datalen
           );
API_RESULT appl_smp_callback
           (
               /* IN */ SMP_BD_HANDLE   * bd_handle,
               /* IN */ UCHAR             event_type,
               /* IN */ UINT16            event_result,
               /* IN */ void            * event_data,
               /* IN */ UINT16            event_datalen
           );



API_RESULT appl_discover_service
           (
               /* IN */ ATT_UUID           uuid,
               /* IN */ UCHAR              uuid_frmt
           );
void appl_discover_gap_service(void);

void appl_dump_bytes (UCHAR *buffer, UINT16 length);
char * appl_hci_get_command_name (UINT16 opcode);
void appl_store_peer_dev_addr(BT_DEVICE_ADDR * peer_div_addr);
void appl_create_connection(void);
/* TBD: Re-arrange function definition. Avoid explicit function declaration. */
void appl_poweron_stack(void);
void appl_master_menu_handler(void);
API_RESULT appl_bluetooth_on_complete_callback (void);
API_RESULT appl_register_callback(void);
API_RESULT appl_hci_callback
           (
               UCHAR    event_type,
               UCHAR  * event_data,
               UCHAR    event_datalen
           );
void appl_l2cap_callback
     (
         DEVICE_HANDLE * handle,
         UCHAR           event_type,
         UCHAR         * event_data,
         UINT16          event_datalen
     );
API_RESULT appl_att_callback
           (
               ATT_HANDLE    * handle,
               UCHAR           event_type,
               API_RESULT      event_result,
               UCHAR         * event_data,
               UINT16          event_datalen
           );
API_RESULT appl_smp_callback
           (
               /* IN */ SMP_BD_HANDLE   * bd_handle,
               /* IN */ UCHAR             event_type,
               /* IN */ UINT16            event_result,
               /* IN */ void            * event_data,
               /* IN */ UINT16            event_datalen
           );

API_RESULT appl_discover_all_characteristics
           (
               /* IN */ UINT16            start_handle,
               /* IN */ UINT16            end_handle,
                        UINT16            uuid,
               /* IN */ UCHAR             uuid_frmt
           );

API_RESULT appl_discover_all_service
          (
               /* IN */ UINT16            start_handle,
               /* IN */ UINT16            end_handle,
                        UINT16                    uuid,
               /* IN */ UCHAR             uuid_frmt
          );

API_RESULT appl_discover_service
           (
               /* IN */ ATT_UUID          uuid,
               /* IN */ UCHAR             uuid_frmt
           );

#endif
