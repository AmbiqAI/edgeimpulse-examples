//*****************************************************************************
//
//! @file svc_amdtp.c
//!
//! @brief AM data transfer protocol service implementation
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
#include "wsf_types.h"
#include "att_api.h"
#include "wsf_trace.h"
#include "bstream.h"
#include "svc_ch.h"
#include "svc_amdtp.h"
#include "svc_cfg.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
//#define SOME_MACRO              42          //!< This is the answer

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
//uint32_t g_ui32Stuff;

/**************************************************************************************************
 Static Variables
**************************************************************************************************/
/* UUIDs */
static const uint8_t svcRxUuid[] = {ATT_UUID_AMDTP_RX};
static const uint8_t svcTxUuid[] = {ATT_UUID_AMDTP_TX};
static const uint8_t svcAckUuid[] = {ATT_UUID_AMDTP_ACK};

/**************************************************************************************************
 Service variables
**************************************************************************************************/

/* AMDTP service declaration */
static const uint8_t amdtpSvc[] = {ATT_UUID_AMDTP_SERVICE};
static const uint16_t amdtpLenSvc = sizeof(amdtpSvc);

/* AMDTP RX characteristic */
static const uint8_t amdtpRxCh[] = {ATT_PROP_WRITE_NO_RSP, UINT16_TO_BYTES(AMDTPS_RX_HDL), ATT_UUID_AMDTP_RX};
static const uint16_t amdtpLenRxCh = sizeof(amdtpRxCh);

/* AMDTP TX characteristic */
static const uint8_t amdtpTxCh[] = {ATT_PROP_NOTIFY, UINT16_TO_BYTES(AMDTPS_TX_HDL), ATT_UUID_AMDTP_TX};
static const uint16_t amdtpLenTxCh = sizeof(amdtpTxCh);

/* AMDTP RX ack characteristic */
static const uint8_t amdtpAckCh[] = {(ATT_PROP_WRITE_NO_RSP | ATT_PROP_NOTIFY), UINT16_TO_BYTES(AMDTPS_ACK_HDL), ATT_UUID_AMDTP_ACK};
static const uint16_t amdtpLenAckCh = sizeof(amdtpAckCh);

/* AMDTP RX data */
/* Note these are dummy values */
static const uint8_t amdtpRx[] = {0};
static const uint16_t amdtpLenRx = sizeof(amdtpRx);

/* AMDTP TX data */
/* Note these are dummy values */
static const uint8_t amdtpTx[] = {0};
static const uint16_t amdtpLenTx = sizeof(amdtpTx);

/* Proprietary data client characteristic configuration */
static uint8_t amdtpTxChCcc[] = {UINT16_TO_BYTES(0x0000)};
static const uint16_t amdtpLenTxChCcc = sizeof(amdtpTxChCcc);

/* AMDTP RX ack data */
/* Note these are dummy values */
static const uint8_t amdtpAck[] = {0};
static const uint16_t amdtpLenAck = sizeof(amdtpAck);

/* Proprietary data client characteristic configuration */
static uint8_t amdtpAckChCcc[] = {UINT16_TO_BYTES(0x0000)};
static const uint16_t amdtpLenAckChCcc = sizeof(amdtpAckChCcc);


/* Attribute list for AMDTP group */
static const attsAttr_t amdtpList[] =
{
  {
    attPrimSvcUuid,
    (uint8_t *) amdtpSvc,
    (uint16_t *) &amdtpLenSvc,
    sizeof(amdtpSvc),
    0,
    ATTS_PERMIT_READ
  },
  {
    attChUuid,
    (uint8_t *) amdtpRxCh,
    (uint16_t *) &amdtpLenRxCh,
    sizeof(amdtpRxCh),
    0,
    ATTS_PERMIT_READ
  },
  {
    svcRxUuid,
    (uint8_t *) amdtpRx,
    (uint16_t *) &amdtpLenRx,
    ATT_VALUE_MAX_LEN,
    (ATTS_SET_UUID_128 | ATTS_SET_VARIABLE_LEN | ATTS_SET_WRITE_CBACK),
    ATTS_PERMIT_WRITE
  },
  {
    attChUuid,
    (uint8_t *) amdtpTxCh,
    (uint16_t *) &amdtpLenTxCh,
    sizeof(amdtpTxCh),
    0,
    ATTS_PERMIT_READ
  },
  {
    svcTxUuid,
    (uint8_t *) amdtpTx,
    (uint16_t *) &amdtpLenTx,
    sizeof(amdtpTx), //ATT_VALUE_MAX_LEN,
    0,  //(ATTS_SET_UUID_128 | ATTS_SET_VARIABLE_LEN),
    0,  //ATTS_PERMIT_READ
  },
  {
    attCliChCfgUuid,
    (uint8_t *) amdtpTxChCcc,
    (uint16_t *) &amdtpLenTxChCcc,
    sizeof(amdtpTxChCcc),
    ATTS_SET_CCC,
    (ATTS_PERMIT_READ | ATTS_PERMIT_WRITE)
  },
  {
    attChUuid,
    (uint8_t *) amdtpAckCh,
    (uint16_t *) &amdtpLenAckCh,
    sizeof(amdtpAckCh),
    0,
    ATTS_PERMIT_READ
  },
  {
    svcAckUuid,
    (uint8_t *) amdtpAck,
    (uint16_t *) &amdtpLenAck,
    ATT_VALUE_MAX_LEN,
    (ATTS_SET_UUID_128 | ATTS_SET_VARIABLE_LEN | ATTS_SET_WRITE_CBACK),
    ATTS_PERMIT_WRITE,
  },
  {
    attCliChCfgUuid,
    (uint8_t *) amdtpAckChCcc,
    (uint16_t *) &amdtpLenAckChCcc,
    sizeof(amdtpAckChCcc),
    ATTS_SET_CCC,
    (ATTS_PERMIT_READ | ATTS_PERMIT_WRITE)
  }
};

/* AMDTP group structure */
static attsGroup_t svcAmdtpGroup =
{
  NULL,
  (attsAttr_t *) amdtpList,
  NULL,
  NULL,
  AMDTPS_START_HDL,
  AMDTPS_END_HDL
};

/*************************************************************************************************/
/*!
 *  \fn     SvcAmdtpsAddGroup
 *
 *  \brief  Add the services to the attribute server.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcAmdtpsAddGroup(void)
{
  AttsAddGroup(&svcAmdtpGroup);
}

/*************************************************************************************************/
/*!
 *  \fn     SvcAmdtpRemoveGroup
 *
 *  \brief  Remove the services from the attribute server.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcAmdtpsRemoveGroup(void)
{
  AttsRemoveGroup(AMDTPS_START_HDL);
}

/*************************************************************************************************/
/*!
 *  \fn     SvcAmdtpsCbackRegister
 *
 *  \brief  Register callbacks for the service.
 *
 *  \param  readCback   Read callback function.
 *  \param  writeCback  Write callback function.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcAmdtpsCbackRegister(attsReadCback_t readCback, attsWriteCback_t writeCback)
{
  svcAmdtpGroup.readCback = readCback;
  svcAmdtpGroup.writeCback = writeCback;
}
