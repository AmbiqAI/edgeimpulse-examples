// ****************************************************************************
//
//  beaconscanner_api.h
//! @file
//!
//! @brief Beacon Scanner sample application interface
//!
//! @{
//
// ****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************


#ifndef BEACONSCANNER_API_H
#define BEACONSCANNER_API_H

#include "wsf_os.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/
/*************************************************************************************************/
/*!
 *  \fn     BeaconScannerStart
 *
 *  \brief  Start Beacon Scan application.
 *
 *  \return None.
 */
/*************************************************************************************************/
void BeaconScannerStart(void);

/*************************************************************************************************/
/*!
 *  \fn     BeaconScannerHandlerInit
 *
 *  \brief  Beacon Scan handler init function called during system initialization.
 *
 *  \param  handlerID  WSF handler ID for App.
 *
 *  \return None.
 */
/*************************************************************************************************/
void BeaconScannerHandlerInit(wsfHandlerId_t handlerId);


/*************************************************************************************************/
/*!
 *  \fn     BeaconScannerHandler
 *
 *  \brief  WSF event handler for Beacon Scan application.
 *
 *  \param  event   WSF event mask.
 *  \param  pMsg    WSF message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void BeaconScannerHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg);

#ifdef __cplusplus
};
#endif

#endif /* BEACONSCANNER_API_H */
