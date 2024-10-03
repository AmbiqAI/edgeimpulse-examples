//*****************************************************************************
//
//! @file svc_amvole.h
//!
//! @brief AmbiqMicro Voice Over LE service definition
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef SVC_VOLES_H
#define SVC_VOLES_H

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

/* AMA services */
#define ATT_UUID_VOLES_SERVICE          0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x03, 0xFE, 0x00, 0x00

/* VOLE characteristics */
#define ATT_UUID_VOLES_RX               0x76, 0x30, 0xF8, 0xDD, 0x90, 0xA3, 0x61, 0xAC, 0xA7, 0x43, 0x05, 0x30, 0x77, 0xB1, 0x4E, 0xF0
#define ATT_UUID_VOLES_TX               0x0B, 0x42, 0x82, 0x1F, 0x64, 0x72, 0x2F, 0x8A, 0xB4, 0x4B, 0x79, 0x18, 0x5B, 0xA0, 0xEE, 0x2B

#define VOLES_START_HDL               0x0860
#define VOLES_END_HDL                 (VOLES_MAX_HDL - 1)


/* VOLE Service Handles */
enum
{
  VOLES_SVC_HDL = VOLES_START_HDL,     /* VOLE service declaration */
  VOLES_RX_CH_HDL,                     /* VOLE write command characteristic */
  VOLES_RX_HDL,                        /* VOLE write command data */
  VOLES_TX_CH_HDL,                     /* VOLE notify characteristic */
  VOLES_TX_HDL,                        /* VOLE notify data */
  VOLES_TX_CH_CCC_HDL,                 /* VOLE notify client characteristic configuration */
  VOLES_MAX_HDL
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
void SvcVolesAddGroup(void);
void SvcVolesRemoveGroup(void);
void SvcVolesCbackRegister(attsReadCback_t readCback, attsWriteCback_t writeCback);

#ifdef __cplusplus
}
#endif

#endif // SVC_VOLES_H
