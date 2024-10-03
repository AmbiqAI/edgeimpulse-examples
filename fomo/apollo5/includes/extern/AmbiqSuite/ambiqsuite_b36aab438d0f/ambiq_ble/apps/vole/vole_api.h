// ****************************************************************************
//
//  vole_api.h
//! @file
//!
//! @brief Ambiq Micro's demonstration of Voice Over LE service.
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

#ifndef VOLE_API_H
#define VOLE_API_H

#include "wsf_os.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
  Macros
**************************************************************************************************/

#ifndef VOLE_CONN_MAX
#define VOLE_CONN_MAX                  1
#endif

/*! WSF message event starting value */
#define VOLE_MSG_START               0xA0

/*! WSF message event enumeration */
enum
{
  AMOTA_TIMER_IND = VOLE_MSG_START,
};

extern dmConnId_t g_AmaConnId;


/**************************************************************************************************
  Function Declarations
**************************************************************************************************/
/*************************************************************************************************/
/*!
 *  \fn     AmvosStart
 *
 *  \brief  Start the application.
 *
 *  \return None.
 */
/*************************************************************************************************/
void VoleStart(void);

/*************************************************************************************************/
/*!
 *  \fn     VoleHandlerInit
 *
 *  \brief  Application handler init function called during system initialization.
 *
 *  \param  handlerID  WSF handler ID for App.
 *
 *  \return None.
 */
/*************************************************************************************************/
void VoleHandlerInit(wsfHandlerId_t handlerId);


/*************************************************************************************************/
/*!
 *  \fn     VoleHandler
 *
 *  \brief  WSF event handler for the application.
 *
 *  \param  event   WSF event mask.
 *  \param  pMsg    WSF message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void VoleHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg);
void VoleBleSend(uint8_t * buf, uint32_t len);

#ifdef __cplusplus
};
#endif

#endif /* VOLE_API_H */
