//*****************************************************************************
//
//! @file appl_ibeacon.h
//!
//! @brief Application Header File for iBeacon example.
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
 *  \file appl_ibeacon.h
 *
 *  Application Header File for iBeacon example.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_HRS_
#define _H_APPL_HRS_

/* ----------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "appl.h"

/* ----------------------------------------- Data Types/ Structures */

/* --------------------------------------------- Functions */
void appl_ibeacon_init(void);

/* Profile handling */
#define APPL_PROFILE_INIT(...) appl_ibeacon_init()
#define APPL_PROFILE_CONNECT(x)
#define APPL_SEND_MEASUREMENT(x)
#define APPL_PROFILE_DISCONNECT_HANDLER(x)
#define GATT_DB_PROFILE_HANDLER
#define APPL_PROFILE_HVN_NTF_COMPLETE_HANDLER(x, y, z)
#define APPL_PROFILE_HVN_IND_COMPLETE_HANDLER(x, y, z)
#define APPL_PROFILE_MTU_UPDT_COMPLETE_HANDLER(x, y)

#define APPL_USE_IDLE_TIMER
#define APPL_IDLE_TIMEOUT           30
#endif /* _H_APPL_HRS_ */


