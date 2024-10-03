//*****************************************************************************
//
//! @file svc_amotas.h
//!
//! @brief AmbiqMicro OTA service definition
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef SVC_AMOTAS_H
#define SVC_AMOTAS_H

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

/*! Partial amota service UUIDs */
#define ATT_UUID_AMOTA_SERVICE_PART     0x1001

/*! Partial amota rx characteristic UUIDs */
#define ATT_UUID_AMOTA_RX_PART          0x0001

/*! Partial amota tx characteristic UUIDs */
#define ATT_UUID_AMOTA_TX_PART          0x0002

/* Amota services */
#define ATT_UUID_AMOTA_SERVICE          ATT_UUID_AMBIQ_BUILD(ATT_UUID_AMOTA_SERVICE_PART)

/* Amota characteristics */
#define ATT_UUID_AMOTA_RX               ATT_UUID_AMBIQ_BUILD(ATT_UUID_AMOTA_RX_PART)
#define ATT_UUID_AMOTA_TX               ATT_UUID_AMBIQ_BUILD(ATT_UUID_AMOTA_TX_PART)

// AM OTA Service
#define AMOTAS_START_HDL               0x0820
#define AMOTAS_END_HDL                 (AMOTAS_MAX_HDL - 1)


/* AMOTA Service Handles */
enum
{
  AMOTA_SVC_HDL = AMOTAS_START_HDL,     /* AMOTA service declaration */
  AMOTAS_RX_CH_HDL,                     /* AMOTA write command characteristic */
  AMOTAS_RX_HDL,                        /* AMOTA write command data */
  AMOTAS_TX_CH_HDL,                     /* AMOTA notify characteristic */
  AMOTAS_TX_HDL,                        /* AMOTA notify data */
  AMOTAS_TX_CH_CCC_HDL,                 /* AMOTA notify client characteristic configuration */
  AMOTAS_MAX_HDL
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
void SvcAmotasAddGroup(void);
void SvcAmotasRemoveGroup(void);
void SvcAmotasCbackRegister(attsReadCback_t readCback, attsWriteCback_t writeCback);

#ifdef __cplusplus
}
#endif

#endif // SVC_AMOTAS_H
