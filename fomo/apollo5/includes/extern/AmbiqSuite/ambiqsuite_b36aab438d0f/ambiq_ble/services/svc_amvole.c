//*****************************************************************************
//
//! @file svc_amvole.c
//!
//! @brief AM Voice Over LE service implementation
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
#include "svc_amvole.h"
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

/**************************************************************************************************
 Static Variables
**************************************************************************************************/
/* UUIDs */
static const uint8_t svcRxUuid[] = {ATT_UUID_VOLES_RX};
static const uint8_t svcTxUuid[] = {ATT_UUID_VOLES_TX};

/**************************************************************************************************
 Service variables
**************************************************************************************************/

/* VOLES service declaration */
static const uint8_t amvoleSvc[] = {ATT_UUID_VOLES_SERVICE};
static const uint16_t amvoleLenSvc = sizeof(amvoleSvc);

/* VOLES RX characteristic */
static const uint8_t amvoleRxCh[] = {ATT_PROP_WRITE, UINT16_TO_BYTES(VOLES_RX_HDL), ATT_UUID_VOLES_RX};
static const uint16_t amvoleLenRxCh = sizeof(amvoleRxCh);

/* VOLES TX characteristic */
static const uint8_t amvoleTxCh[] = {ATT_PROP_NOTIFY | ATT_PROP_READ, UINT16_TO_BYTES(VOLES_TX_HDL), ATT_UUID_VOLES_TX};
static const uint16_t amvoleLenTxCh = sizeof(amvoleTxCh);

/* VOLES RX data */
/* Note these are dummy values */
static const uint8_t amvoleRx[] = {0};
static const uint16_t amvoleLenRx = sizeof(amvoleRx);

/* VOLES TX data */
/* Note these are dummy values */
static const uint8_t amvoleTx[] = {0};
static const uint16_t amvoleLenTx = sizeof(amvoleTx);

/* Proprietary data client characteristic configuration */
static uint8_t amvoleTxChCcc[] = {UINT16_TO_BYTES(0x0000)};
static const uint16_t amvoleLenTxChCcc = sizeof(amvoleTxChCcc);

/* Attribute list for VOLES group */
static const attsAttr_t amvoleList[] =
{
  {
    attPrimSvcUuid,
    (uint8_t *) amvoleSvc,
    (uint16_t *) &amvoleLenSvc,
    sizeof(amvoleSvc),
    0,
    ATTS_PERMIT_READ
  },
  {
    attChUuid,
    (uint8_t *) amvoleRxCh,
    (uint16_t *) &amvoleLenRxCh,
    sizeof(amvoleRxCh),
    0,
    ATTS_PERMIT_READ
  },
  {
    svcRxUuid,
    (uint8_t *) amvoleRx,
    (uint16_t *) &amvoleLenRx,
    ATT_VALUE_MAX_LEN,
    (ATTS_SET_UUID_128 | ATTS_SET_VARIABLE_LEN | ATTS_SET_WRITE_CBACK),
    ATTS_PERMIT_WRITE
    //ATTS_PERMIT_WRITE_ENC
  },
  {
    attChUuid,
    (uint8_t *) amvoleTxCh,
    (uint16_t *) &amvoleLenTxCh,
    sizeof(amvoleTxCh),
    0,
    ATTS_PERMIT_READ
  },
  {
    svcTxUuid,
    (uint8_t *) amvoleTx,
    (uint16_t *) &amvoleLenTx,
    sizeof(amvoleTx), //ATT_VALUE_MAX_LEN,
#if 0
    #if USE_OUTPUT_VOLES_AMA
      (ATTS_SET_UUID_128 | ATTS_SET_VARIABLE_LEN),
      ATTS_PERMIT_READ
    #else
      0,  //(ATTS_SET_UUID_128 | ATTS_SET_VARIABLE_LEN),
      0   //ATTS_PERMIT_READ
    #endif
#endif
  (ATTS_SET_UUID_128),
  ATTS_PERMIT_READ

  },
  {
    attCliChCfgUuid,
    (uint8_t *) amvoleTxChCcc,
    (uint16_t *) &amvoleLenTxChCcc,
    sizeof(amvoleTxChCcc),
    ATTS_SET_CCC,
    (ATTS_PERMIT_READ | ATTS_PERMIT_WRITE)
  }
};

/* VOLES group structure */
static attsGroup_t svcVolesGroup =
{
  NULL,
  (attsAttr_t *) amvoleList,
  NULL,
  NULL,
  VOLES_START_HDL,
  VOLES_END_HDL
};

/*************************************************************************************************/
/*!
 *  \fn     SvcVolesAddGroup
 *
 *  \brief  Add the services to the attribute server.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcVolesAddGroup(void)
{
  AttsAddGroup(&svcVolesGroup);
}

/*************************************************************************************************/
/*!
 *  \fn     SvcVolesRemoveGroup
 *
 *  \brief  Remove the services from the attribute server.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcVolesRemoveGroup(void)
{
  AttsRemoveGroup(VOLES_START_HDL);
}

/*************************************************************************************************/
/*!
 *  \fn     SvcVolesCbackRegister
 *
 *  \brief  Register callbacks for the service.
 *
 *  \param  readCback   Read callback function.
 *  \param  writeCback  Write callback function.
 *
 *  \return None.
 */
/*************************************************************************************************/
void SvcVolesCbackRegister(attsReadCback_t readCback, attsWriteCback_t writeCback)
{
  svcVolesGroup.readCback = readCback;
  svcVolesGroup.writeCback = writeCback;
}
