//*****************************************************************************
//
//! @file svc_throughput.h
//!
//! @brief AmbiqMicro throughput service definition
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef SVC_THROUGHPUT_H
#define SVC_THROUGHPUT_H

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

/*! Partial throughput service UUIDs */
#define ATT_UUID_THROUGHPUT_SERVICE_PART     0x5450

/*! Partial throughput rx characteristic UUIDs */
#define ATT_UUID_THROUGHPUT_RX_PART          0x5401

/*! Partial throughput tx characteristic UUIDs */
#define ATT_UUID_THROUGHPUT_TX_PART          0x5402
/*! Partial throughput connection update characteristic UUIDs */
#define ATT_UUID_THROUGHPUT_CON_UPT_PART     0x5403

/* Throughput services */
#define ATT_UUID_THROUGHPUT_SERVICE          ATT_UUID_AMBIQ_BUILD(ATT_UUID_THROUGHPUT_SERVICE_PART)

/* Throughput characteristics */
#define ATT_UUID_THROUGHPUT_RX               ATT_UUID_AMBIQ_BUILD(ATT_UUID_THROUGHPUT_RX_PART)
#define ATT_UUID_THROUGHPUT_TX               ATT_UUID_AMBIQ_BUILD(ATT_UUID_THROUGHPUT_TX_PART)
#define ATT_UUID_THROUGHPUT_CON_UPT          ATT_UUID_AMBIQ_BUILD(ATT_UUID_THROUGHPUT_CON_UPT_PART)
// throughput Service
#define THROUGHPUT_START_HDL               0x0840
#define THROUGHPUT_END_HDL                 (THROUGHPUT_MAX_HDL - 1)


/* Throughput Service Handles */
enum
{
  THROUGHPUT_SVC_HDL = THROUGHPUT_START_HDL,     /* Throughput service declaration */
  THROUGHPUT_RX_CH_HDL,                     /* Throughput write command characteristic */
  THROUGHPUT_RX_HDL,                        /* Throughput write command data */
  THROUGHPUT_TX_CH_HDL,                     /* Throughput notify characteristic */
  THROUGHPUT_TX_HDL,                        /* Throughput notify data */
  THROUGHPUT_TX_CH_CCC_HDL,                 /* Throughput notify client characteristic configuration */
  THROUGHPUT_CON_UPT_CH_HDL,                 /* Throughput connection update characteristic*/
  THROUGHPUT_CON_UPT_HDL,                    /* Throughput connection update data*/
  THROUGHPUT_CON_UPT_CCC_HDL,                /* Throughput connection update data notify client characteristic configuration */
  THROUGHPUT_MAX_HDL
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
void SvcThroughputAddGroup(void);
void SvcThroughputRemoveGroup(void);
void SvcThroughputCbackRegister(attsReadCback_t readCback, attsWriteCback_t writeCback);

#ifdef __cplusplus
}
#endif

#endif // SVC_THROUGHPUT_H
