//*****************************************************************************
//
//! @file svc_cust.h
//!
//! @brief Customerized Service Definition
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef SVC_CUST_H
#define SVC_CUST_H

//
// Put additional includes here if necessary.
//
#include "bstream.h"

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define INCLUDE_USER_DESCR

/*! Base UUID:  00002760-08C2-11E1-9073-0E8AC72EXXXX */
#define ATT_UUID_AMBIQ_BASE             0x2E, 0xC7, 0x8A, 0x0E, 0x73, 0x90, \
                                            0xE1, 0x11, 0xC2, 0x08, 0x60, 0x27, 0x00, 0x00

/*! Macro for building Ambiq UUIDs */
#define ATT_UUID_AMBIQ_BUILD(part)      UINT16_TO_BYTES(part), ATT_UUID_AMBIQ_BASE

/* Partial Customized Service UUID */
#define ATT_UUID_CUST_SERVICE_PART          0x2000
#define ATT_UUID_CUST_SERVICE               ATT_UUID_AMBIQ_BUILD(ATT_UUID_CUST_SERVICE_PART)

/*! Partial WRITEONLY characteristic UUID */
#define ATT_UUID_CUSTS_WRITEONLY_PART       0x2010
#define ATT_UUID_CUSTS_WRITEONLY            ATT_UUID_AMBIQ_BUILD(ATT_UUID_CUSTS_WRITEONLY_PART)

/*! Partial READONLY characteristic UUID */
#define ATT_UUID_CUSTS_READONLY_PART        0x2011
#define ATT_UUID_CUSTS_READONLY             ATT_UUID_AMBIQ_BUILD(ATT_UUID_CUSTS_READONLY_PART)

/*! Partial NOTIFYONLY characteristic UUID */
#define ATT_UUID_CUSTS_NOTIFYONLY_PART      0x2012
#define ATT_UUID_CUSTS_NOTIFYONLY           ATT_UUID_AMBIQ_BUILD(ATT_UUID_CUSTS_NOTIFYONLY_PART)

/*! Partial NOTIFYONLY characteristic UUID */
#define ATT_UUID_CUSTS_INDICATEONLY_PART    0x2013
#define ATT_UUID_CUSTS_INDICATEONLY         ATT_UUID_AMBIQ_BUILD(ATT_UUID_CUSTS_INDICATEONLY_PART)

/* Customized Service UUID */

#define CUSTSVC_HANDLE_START                0x01A0
#define CUSTSVC_HANDLE_END                  (CUSTS_HANDLE_LAST - 1)

/* AMDTP Service Handles */
enum
{
    CUSTS_HANDLE_SVC                        = CUSTSVC_HANDLE_START,
    CUSTS_HANDLE_WRITEONLY_CH,
    CUSTS_HANDLE_WRITEONLY,
#ifdef INCLUDE_USER_DESCR
    CUSTS_HANDLE_WRITEONLY_USR_DESCR,
#endif
    CUSTS_HANDLE_READONLY_CH,
    CUSTS_HANDLE_READONLY,
#ifdef INCLUDE_USER_DESCR
    CUSTS_HANDLE_READONLY_USR_DESCR,
#endif
    CUSTS_HANDLE_NOTIFYONLY_CH,
    CUSTS_HANDLE_NOTIFYONLY,
    CUSTS_HANDLE_NOTIFYONLY_CCC,
#ifdef INCLUDE_USER_DESCR
    CUSTS_HANDLE_NOTIFYONLY_USR_DESCR,
#endif
    CUSTS_HANDLE_INDICATEONLY_CH,
    CUSTS_HANDLE_INDICATEONLY,
    CUSTS_HANDLE_INDICATEONLY_CCC,
#ifdef INCLUDE_USER_DESCR
    CUSTS_HANDLE_INDICATEONLY_USR_DESCR,
#endif
    CUSTS_HANDLE_LAST,

};


//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************

//*****************************************************************************
//
// Function definitions.
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Add the customized service to the attribute server.
//!
//! @param none
//!
//! This function adds the customized service to the attribute server.
//!
//! @return none
//
//*****************************************************************************
void SvcCustAddGroup(void);

//*****************************************************************************
//
//! @brief Remove the customized service from the attribute server.
//!
//! @param none
//!
//! This function removes the customized service to the attribute server.
//!
//! @return none
//
//*****************************************************************************
void SvcCustRemoveGroup(void);

//*****************************************************************************
//
//! @brief Register callbacks for the customized service.
//!
//! @param readCback    - attribute read callback function pointer.
//! @param writeCback   - attribute write callback function pointer.
//!
//! This function registers callback functions for the customized servcie..
//!
//! @return none
//
//*****************************************************************************
void SvcCustCbackRegister(attsReadCback_t readCback, attsWriteCback_t writeCback);

#ifdef __cplusplus
}
#endif

#endif // SVC_CUST_H
