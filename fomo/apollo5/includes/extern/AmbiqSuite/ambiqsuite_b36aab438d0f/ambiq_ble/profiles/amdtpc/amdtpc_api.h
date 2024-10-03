// ****************************************************************************
//
//  amdtpc_api.h
//! @file
//!
//! @brief Ambiq Micro AMDTP client.
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

#ifndef AMDTPC_API_H
#define AMDTPC_API_H

#include "att_api.h"
#include "amdtp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
  Macros
**************************************************************************************************/

enum
{
    AMDTP_RX_HDL_IDX,             /*! Rx data */
    AMDTP_TX_DATA_HDL_IDX,        /*! Tx data */
    AMDTP_TX_DATA_CCC_HDL_IDX,    /*! Tx data client characteristic configuration descriptor */
    AMDTP_ACK_HDL_IDX,            /*! Ack */
    AMDTP_ACK_CCC_HDL_IDX,        /*! Ack client characteristic configuration descriptor */
    AMDTP_HDL_LIST_LEN            /*! Handle list length */
};

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \fn     AmdtpcDiscover
 *
 *  \brief  Perform service and characteristic discovery for AMDTP service.
 *          Parameter pHdlList must point to an array of length AMDTP_HDL_LIST_LEN.
 *          If discovery is successful the handles of discovered characteristics and
 *          descriptors will be set in pHdlList.
 *
 *  \param  connId    Connection identifier.
 *  \param  pHdlList  Characteristic handle list.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AmdtpcDiscover(dmConnId_t connId, uint16_t *pHdlList);

void
amdtpc_init(wsfHandlerId_t handlerId, amdtpRecvCback_t recvCback, amdtpTransCback_t transCback);

void
amdtpc_start(uint16_t rxHdl, uint16_t ackHdl, uint16_t txHdl, uint8_t timerEvt);

void
amdtpc_proc_msg(wsfMsgHdr_t *pMsg);

eAmdtpStatus_t
AmdtpcSendPacket(eAmdtpPktType_t type, bool_t encrypted, bool_t enableACK, uint8_t *buf, uint16_t len);

#ifdef __cplusplus
};
#endif

#endif /* AMDTPC_API_H */
