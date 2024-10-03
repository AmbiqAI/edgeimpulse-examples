//*****************************************************************************
//
//! @file svc_amdtps.h
//!
//! @brief AmbiqMicro Data Transfer Protocol service definition
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef SVC_AMDTPS_H
#define SVC_AMDTPS_H

//
// Put additional includes here if necessary.
//

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

/*! Base UUID:  00002760-08C2-11E1-9073-0E8AC72EXXXX */
#define ATT_UUID_AMBIQ_BASE             0x2E, 0xC7, 0x8A, 0x0E, 0x73, 0x90, \
                                            0xE1, 0x11, 0xC2, 0x08, 0x60, 0x27, 0x00, 0x00

/*! Macro for building Ambiq UUIDs */
#define ATT_UUID_AMBIQ_BUILD(part)      UINT16_TO_BYTES(part), ATT_UUID_AMBIQ_BASE

/*! Partial amdtp service UUIDs */
#define ATT_UUID_AMDTP_SERVICE_PART     0x1011

/*! Partial amdtp rx characteristic UUIDs */
#define ATT_UUID_AMDTP_RX_PART          0x0011

/*! Partial amdtp tx characteristic UUIDs */
#define ATT_UUID_AMDTP_TX_PART          0x0012

/*! Partial amdtp ack characteristic UUIDs */
#define ATT_UUID_AMDTP_ACK_PART         0x0013

/* Amdtp services */
#define ATT_UUID_AMDTP_SERVICE          ATT_UUID_AMBIQ_BUILD(ATT_UUID_AMDTP_SERVICE_PART)

/* Amdtp characteristics */
#define ATT_UUID_AMDTP_RX               ATT_UUID_AMBIQ_BUILD(ATT_UUID_AMDTP_RX_PART)
#define ATT_UUID_AMDTP_TX               ATT_UUID_AMBIQ_BUILD(ATT_UUID_AMDTP_TX_PART)
#define ATT_UUID_AMDTP_ACK              ATT_UUID_AMBIQ_BUILD(ATT_UUID_AMDTP_ACK_PART)

// AM DTP Service
#define AMDTPS_START_HDL               0x0800
#define AMDTPS_END_HDL                 (AMDTPS_MAX_HDL - 1)


/* AMDTP Service Handles */
enum
{
  AMDTP_SVC_HDL = AMDTPS_START_HDL,     /* AMDTP service declaration */
  AMDTPS_RX_CH_HDL,                     /* AMDTP write command characteristic */
  AMDTPS_RX_HDL,                        /* AMDTP write command data */
  AMDTPS_TX_CH_HDL,                     /* AMDTP notify characteristic */
  AMDTPS_TX_HDL,                        /* AMDTP notify data */
  AMDTPS_TX_CH_CCC_HDL,                 /* AMDTP notify client characteristic configuration */
  AMDTPS_ACK_CH_HDL,                    /* AMDTP rx ack characteristic */
  AMDTPS_ACK_HDL,                       /* AMDTP rx ack data */
  AMDTPS_ACK_CH_CCC_HDL,                /* AMDTP rx ack client characteristic configuration */
  AMDTPS_MAX_HDL
};


//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
//extern uint32_t g_ui32Stuff;

//*****************************************************************************
//
// Function definitions.
//
//*****************************************************************************
void SvcAmdtpsAddGroup(void);
void SvcAmdtpsRemoveGroup(void);
void SvcAmdtpsCbackRegister(attsReadCback_t readCback, attsWriteCback_t writeCback);

#ifdef __cplusplus
}
#endif

#endif // SVC_AMDTPS_H
