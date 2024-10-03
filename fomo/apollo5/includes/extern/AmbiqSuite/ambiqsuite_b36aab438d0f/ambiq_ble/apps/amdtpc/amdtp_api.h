// ****************************************************************************
//
//  amdtp_api.h
//! @file
//!
//! @brief Ambiq Micro's demonstration of AMDTP client.
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

#ifndef AMDTP_API_H
#define AMDTP_API_H

#include "wsf_os.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/
/*************************************************************************************************/
/*!
 *  \fn     AmdtpcStart
 *
 *  \brief  Start the application.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AmdtpcStart(void);

/*************************************************************************************************/
/*!
 *  \fn     AmdtpcHandlerInit
 *
 *  \brief  Application handler init function called during system initialization.
 *
 *  \param  handlerID  WSF handler ID for App.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AmdtpcHandlerInit(wsfHandlerId_t handlerId);


/*************************************************************************************************/
/*!
 *  \fn     AmdtpcHandler
 *
 *  \brief  WSF event handler for the application.
 *
 *  \param  event   WSF event mask.
 *  \param  pMsg    WSF message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AmdtpcHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg);

void AmdtpcScanStart(void);
void AmdtpcScanStop(void);
void AmdtpcConnOpen(uint8_t idx);
void AmdtpcSendTestData(void);
void AmdtpcSendTestDataStop(void);
void AmdtpcRequestServerSend(void);
void AmdtpcRequestServerSendStop(void);

#ifdef __cplusplus
};
#endif

#endif /* AMDTP_API_H */
