
/**
 *  \file appl_rscs.h
 *
 *  Application Header File for Runner's Speed & Cadence Sensor.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_RSCS_
#define _H_APPL_RSCS_

/* ----------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "appl.h"


typedef struct cntrl_point_param
{
    UCHAR          rsp_code;

    UCHAR          op_code;

    APPL_HANDLE    appl_handle;

}APPL_RCSC_CNTRL_POINT_RSP_PARAM;

/* --------------------------------------------- Global Definitions */

/* --------------------------------------------- Functions */
void appl_rscs_init(void);
void appl_rscs_activate_sdp_record (void);
void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config);
void appl_timer_expiry_handler (void *data, UINT16 datalen);
void appl_rscs_connect(APPL_HANDLE  * appl_handle);
void appl_send_rsc_measurement (APPL_HANDLE   * handle);
void appl_rscs_server_reinitialize (void);
API_RESULT appl_rsc_control_point_handler
           (
                GATT_DB_HANDLE  * handle,
                ATT_VALUE       * value

           );

void appl_rsc_stop_timer (APPL_HANDLE   * appl_handle);
void appl_rsc_cntrl_point_cnfg_handler (GATT_DB_HANDLE  *handle,UINT16  config);
void appl_profile_menu_handler (void);

/* Profile handling */
#define APPL_PROFILE_INIT() appl_rscs_init()
#define APPL_PROFILE_POWER_ON_HANDLER()
#define APPL_PROFILE_CONNECT(x) appl_rscs_connect(x)
#define APPL_SEND_MEASUREMENT(x)appl_send_rsc_measurement(x)
#define APPL_PROFILE_DISCONNECT_HANDLER(x)\
        appl_rsc_stop_timer (x)

#define GATT_DB_PROFILE_HANDLER  gatt_db_rscs_handler
#define APPL_PROFILE_HVN_NTF_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_HVN_IND_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_MTU_UPDT_COMPLETE_HANDLER(x,y)

#define APPL_USE_IDLE_TIMER
#define APPL_IDLE_TIMEOUT           30
#endif /* _H_APPL_RSCS_ */


