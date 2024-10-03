//*****************************************************************************
//
//! @file appl_ancs.h
//!
//! @brief Application Header File for ANCS.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************


/**
 *  \file appl_ancs.h
 *
 *  Application Header File for ANCS.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_ANCS_
#define _H_APPL_ANCS_

/* ----------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "appl.h"
#include "appl_ancs_att_server.h"

/* ----------------------------------------- Data Types/ Structures */
typedef struct
{
    UCHAR    index;

    UCHAR    length;

}APPL_HRS_OBS_DATA_INFO;

/* --------------------------------------------- Global Definitions */
/** LSB of error code has to be spec defined */
#define APPL_HR_CNTRL_PNT_NOT_SUPPORTED   (APPL_ERR_ID | 0x80)

/* --------------------------------------------- Functions */
void appl_ancs_init(void);
void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config);
void appl_timer_expiry_handler (void *data, UINT16 datalen);
void appl_ancs_connect(DEVICE_HANDLE  * dq_handle);
void appl_send_ancs_measurement (APPL_HANDLE   * handle);
void appl_ancs_server_reinitialize (void);
void appl_rcv_service_desc (UINT16 config, ATT_UUID uuid, UINT16 value_handle);
void appl_rcv_service_char (ATT_UUID uuid, UINT16 value_handle);
API_RESULT appl_hr_control_point_handler
           (
                GATT_DB_HANDLE  * handle,
                ATT_VALUE       * value
           );
API_RESULT appl_att_callback
           (
               ATT_HANDLE    * handle,
               UCHAR           event_type,
               API_RESULT      event_result,
               UCHAR         * event_data,
               UINT16          event_datalen
           );

void ancs_mtu_update(APPL_HANDLE    * appl_handle, UINT16 t_mtu);

/* Profile handling */
#define APPL_PROFILE_INIT(...) appl_ancs_init()
#define APPL_PROFILE_CONNECT(x) appl_ancs_connect(x)
#define APPL_SEND_MEASUREMENT(x)appl_send_ancs_measurement(x)
#define APPL_PROFILE_DISCONNECT_HANDLER(x) appl_ancs_server_reinitialize()
#define GATT_DB_PROFILE_HANDLER  gatt_db_ancs_handler
#define APPL_PROFILE_HVN_NTF_COMPLETE_HANDLER(x, y, z)
#define APPL_PROFILE_HVN_IND_COMPLETE_HANDLER(x, y, z)
#define APPL_PROFILE_MTU_UPDT_COMPLETE_HANDLER(x, y) ancs_mtu_update(x, y)

#define APPL_USE_IDLE_TIMER
#define APPL_IDLE_TIMEOUT           30
#endif /* _H_APPL_ANCS_ */


