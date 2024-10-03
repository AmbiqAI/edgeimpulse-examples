//*****************************************************************************
//
//  appl_amota.h
//! @file
//!
//! @brief  Application Header File for AMOTA.
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
 *  \file appl_amota.h
 *
 *  Application Header File for AMOTA.
 */


#ifndef _H_APPL_AMOTA_
#define _H_APPL_AMOTA_

/* ----------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "appl.h"

// enable debug print for AMOTA profile
// #define AMOTA_DEBUG_ON

/* --------------------------------------------- Global Definitions */
#define AMOTA_PACKET_SIZE           (512 + 16)    // Bytes
#define AMOTA_LENGTH_SIZE_IN_PKT    2
#define AMOTA_CMD_SIZE_IN_PKT       1
#define AMOTA_CRC_SIZE_IN_PKT       4
#define AMOTA_HEADER_SIZE_IN_PKT    AMOTA_LENGTH_SIZE_IN_PKT + AMOTA_CMD_SIZE_IN_PKT

#define AMOTA_FW_HEADER_SIZE        44

#define AMOTA_FW_STORAGE_INTERNAL   0
#define AMOTA_FW_STORAGE_EXTERNAL   1

/* ----------------------------------------- Data Types/ Structures */
/*! Configurable parameters */
typedef struct
{
    //! Short description of each member should go here.
    uint32_t reserved;
}
AmotasCfg_t;

/* --------------------------------------------- Functions */
void appl_amotas_init(void);
void appl_manage_trasnfer(GATT_DB_HANDLE handle, UINT16 config);
void appl_amotas_connect(DEVICE_HANDLE  * dq_handle);
void appl_amotas_send_data(UCHAR *data, UINT16 len);
void appl_amotas_disconnect(void);

API_RESULT appl_amotas_write_cback(GATT_DB_HANDLE *handle, ATT_VALUE *value);

void amota_mtu_update(APPL_HANDLE    * appl_handle, UINT16 t_mtu);

/* Profile handling */
#define APPL_PROFILE_INIT(...) appl_amotas_init()
#define APPL_PROFILE_CONNECT(x) appl_amotas_connect(x)
#define APPL_SEND_MEASUREMENT(x) //appl_send_amotas_measurement(x)
#define APPL_PROFILE_DISCONNECT_HANDLER(x) appl_amotas_disconnect()
#define GATT_DB_PROFILE_HANDLER  gatt_db_amotas_handler
#define APPL_PROFILE_HVN_NTF_COMPLETE_HANDLER(x, y, z)
#define APPL_PROFILE_HVN_IND_COMPLETE_HANDLER(x, y, z)
#define APPL_PROFILE_MTU_UPDT_COMPLETE_HANDLER(x, y) amota_mtu_update(x, y)

#define APPL_USE_IDLE_TIMER
#define APPL_IDLE_TIMEOUT           30
#endif /* _H_APPL_AMOTA_ */


