//*****************************************************************************
//
//  custss_api.h
//! @file
//!
//! @brief Customized Serice Server API header file
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef CUSTSS_API_H
#define CUSTSS_API_H

#include "wsf_timer.h"
#include "att_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//*****************************************************************************
//
// function definitions
//
//*****************************************************************************


//*****************************************************************************
//
//! @fn     CustssReadCback
//!
//! @brief  Attribute read callback function.
//!
//! @param  connId       DM connection ID.
//! @param  handler      Attribute handle.
//! @param  operation    Attribute operation.
//! @param  offset       Attribute offset.
//! @param  pAttr        Pointer to attribute
//!
//! @return none
//
//*****************************************************************************
static uint8_t CustssReadCback(dmConnId_t connId, uint16_t handle, \
                                    uint8_t operation, uint16_t offset, attsAttr_t *pAttr);

//*****************************************************************************
//
//! @fn     CustssWriteCback
//!
//! @brief  Attribute write callback function.
//!
//! @param  connId       DM connection ID.
//! @param  handler      Attribute handle.
//! @param  operation    Attribute operation.
//! @param  offset       Attribute offset.
//! @param  len          Datalen written to attribute.
//! @param  pValue       Pointer to data
//! @param  pAttr        Pointer to attribute
//!
//! @return none
//
//*****************************************************************************
static uint8_t CustssWriteCback(dmConnId_t connId, uint16_t handle, \
                                    uint8_t operation, uint16_t offset, uint16_t len, uint8_t *pValue, attsAttr_t *pAttr);

//*****************************************************************************
//
//! @fn     CustssScheduledActionOnTimesUp
//!
//! @brief  Start the scheduled action.
//!
//! @param  pMsg       Pointer to wsf message.
//!
//! @return none
//
//*****************************************************************************
static void CustssScheduledActionOnTimesUp(wsfMsgHdr_t *pMsg);

//*****************************************************************************
//
//! @fn     CustssScheduledActionStop
//!
//! @brief  Stop the scheduled action.
//!
//! @param  connId      DM connection ID.
//! @param  timerEvt    Event to be sent when the timer expires.
//! @param  msec        Time delay in milliseconds.
//!
//! @return none
//
//*****************************************************************************
void CustssScheduledActionStart(dmConnId_t connId, uint8_t timerEvt, wsfTimerTicks_t msec);

//*****************************************************************************
//
//! @fn     CustssScheduledActionStop
//!
//! @brief  Stop the scheduled action.
//!
//! @param  connId          DM connection ID.
//!
//! @return none
//
//*****************************************************************************
void CustssScheduledActionStop(dmConnId_t connId);

//*****************************************************************************
//
//! @fn     CustssInit
//!
//! @brief  Initialize the customized service server.
//!
//! @param  handlerId       Handler of application
//!
//! @return none
//
//*****************************************************************************
void CustssInit(wsfHandlerId_t handlerId);

//*****************************************************************************
//
//! @fn     CustssDeinit
//!
//! @brief  De-initialize the customized service server.
//!
//! @param  none
//!
//! @return none
//
//*****************************************************************************
void CustssDeinit(void);

//*****************************************************************************
//
//! @fn     CustssStart
//!
//! @brief  Start the customized service server.
//!
//! @param  none
//!
//! @return none
//
//*****************************************************************************
void CustssStart(void);

//*****************************************************************************
//
//! @fn     CustssStop
//!
//! @brief  Stop the customized service server.
//!
//! @param  none
//!
//! @return none
//
//*****************************************************************************
void CustssStop(void);

//*****************************************************************************
//
//! @fn     CustssSendNtf
//!
//! @brief  Send an attribute protocol Handle Value Notification.
//!
//! @param  connId       DM connection ID.
//! @param  idx          Index of attrribute.
//! @param  handle       Attribute handle.
//! @param  len          Length of data.
//! @param  pVal        Pointer to data.
//!
//! @return none
//
//*****************************************************************************
void CustssSendNtf(dmConnId_t connId, uint8_t idx, uint16_t handle, uint16_t len, uint8_t *pVal);

//*****************************************************************************
//
//! @fn     CustssSendInd
//!
//! @brief  Send an attribute protocol handle value indication.
//!
//! @param  connId       DM connection ID.
//! @param  idx          Index of attrribute.
//! @param  handle       Attribute handle.
//! @param  len          Length of data.
//! @param  pVal         Pointer to data.
//!
//! @return none
//
//*****************************************************************************
void CustssSendInd(dmConnId_t connId, uint8_t idx, uint16_t handle, uint16_t len, uint8_t *pVal);

//*****************************************************************************
//
//! @fn     CustssProcMsg
//!
//! @brief  Process wsf messages.
//!
//! @param  pMsg        Pointer to wsf message.
//!
//! @return none
//
//*****************************************************************************
void CustssProcMsg(wsfMsgHdr_t *pMsg);

#ifdef __cplusplus
}
#endif

#endif // CUSTSS_API_H
