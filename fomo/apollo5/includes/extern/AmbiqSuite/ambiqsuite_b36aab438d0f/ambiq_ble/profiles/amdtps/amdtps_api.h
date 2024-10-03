//*****************************************************************************
//
//  amdtps_api.h
//! @file
//!
//! @brief Brief description of the header. No need to get fancy here.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AMDTPS_API_H
#define AMDTPS_API_H

#include "wsf_timer.h"
#include "att_api.h"
#include "amdtp_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************


//
// Connection control block
//
typedef struct
{
    dmConnId_t    connId;               // Connection ID
    bool_t        amdtpToSend;          // AMDTP notify ready to be sent on this channel
}
amdtpsConn_t;

/*! Configurable parameters */
typedef struct
{
    //! Short description of each member should go here.
    uint32_t reserved;
}
AmdtpsCfg_t;

//*****************************************************************************
//
// function definitions
//
//*****************************************************************************

void amdtps_init(wsfHandlerId_t handlerId, AmdtpsCfg_t *pCfg, amdtpRecvCback_t recvCback, amdtpTransCback_t transCback);

void amdtps_proc_msg(wsfMsgHdr_t *pMsg);

uint8_t amdtps_write_cback(dmConnId_t connId, uint16_t handle, uint8_t operation,
                       uint16_t offset, uint16_t len, uint8_t *pValue, attsAttr_t *pAttr);

void amdtps_start(dmConnId_t connId, uint8_t timerEvt, uint8_t amdtpCccIdx);

void amdtps_stop(dmConnId_t connId);

eAmdtpStatus_t
AmdtpsSendPacket(eAmdtpPktType_t type, bool_t encrypted, bool_t enableACK, uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif // AMDTPS_API_H
