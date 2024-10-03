//*****************************************************************************
//
//  voles_api.h
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

#ifndef VOLES_API_H
#define VOLES_API_H

#include "wsf_timer.h"
#include "att_api.h"
#include "vole_common.h"

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
    bool_t        voleToSend;          // VOLE notify ready to be sent on this channel
}
volesConn_t;

/*! Configurable parameters */
typedef struct
{
    //! Short description of each member should go here.
    uint32_t reserved;
}
VolesCfg_t;

//*****************************************************************************
//
// function definitions
//
//*****************************************************************************

void voles_init(wsfHandlerId_t handlerId, eVoleCodecType codec_type);
void voles_proc_msg(wsfMsgHdr_t *pMsg);
void voles_transmit_voice_data(void);

int voles_set_codec_type(eVoleCodecType codec_type);

uint8_t voles_write_cback(dmConnId_t connId, uint16_t handle, uint8_t operation,
                       uint16_t offset, uint16_t len, uint8_t *pValue, attsAttr_t *pAttr);

void voles_start(dmConnId_t connId, uint8_t timerEvt, uint8_t voleCccIdx);

void voles_stop(dmConnId_t connId);


#ifdef __cplusplus
}
#endif

#endif // VOLES_API_H
