// ****************************************************************************
//
//  amdtps_2p5ms_api.h
//! @file
//!
//! @brief Ambiq Micro's demonstration of AMDTP service.
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

#ifndef AMDTP_API_H
#define AMDTP_API_H

#include "wsf_os.h"
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
  Macros
**************************************************************************************************/

#ifndef AMDTP_CONN_MAX
#define AMDTP_CONN_MAX                  1
#endif

#define DATA_LEN_MAX            (512)

typedef struct
{
    wsfTimer_t connUpdateTimer;      // Timer for connection update testing
    wsfTimer_t dataTxTimer;          // Timer for sending notification data
    wsfTimer_t phyUpdateTimer;       // Timer for updating PHY
    wsfTimer_t readEvtCntTimer;      // Read event connection counter timer during connection
    uint16_t *pConnTestInt;          // Pointer to testing connection interval array
    uint8_t connLatency;             // Connection interval latency
    uint8_t connUpdateCnt;           // The counter used for connection update testing, will be increased by 1
                                     // each time when connection update timer expires
    uint16_t txCounter;              // The sent out packet counter
    uint16_t txLen;                  // Transmit data length.
    uint8_t txOffset;                // Offset of the transmit data content.
    uint8_t txData[DATA_LEN_MAX];    // The buffer for the transmit data
    uint8_t *pUpdatePhy;             // Pointer to PHY update array
    uint8_t phyUpdateCnt;            // The counter used for PHY update testing, will be increased by 1
                                     // each time when PHY update timer expires
    uint16_t lastConEvtCnt;          // The connection event counter for last time
    bool firstTimeRdEvtCnt;          // The first time to read event counter
} amdtpsConnEnv_t;


/**************************************************************************************************
  Function Declarations
**************************************************************************************************/
/*************************************************************************************************/
/*!
 *  \fn     AmdtpStart
 *
 *  \brief  Start the application.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AmdtpStart(void);

/*************************************************************************************************/
/*!
 *  \fn     AmdtpHandlerInit
 *
 *  \brief  Application handler init function called during system initialization.
 *
 *  \param  handlerID  WSF handler ID for App.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AmdtpHandlerInit(wsfHandlerId_t handlerId);


/*************************************************************************************************/
/*!
 *  \fn     AmdtpHandler
 *
 *  \brief  WSF event handler for the application.
 *
 *  \param  event   WSF event mask.
 *  \param  pMsg    WSF message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void AmdtpHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg);

#ifdef __cplusplus
};
#endif

#endif /* AMDTP_API_H */
