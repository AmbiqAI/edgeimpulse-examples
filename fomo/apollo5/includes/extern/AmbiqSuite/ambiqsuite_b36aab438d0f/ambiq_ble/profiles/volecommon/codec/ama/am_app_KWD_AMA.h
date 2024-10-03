// ****************************************************************************
//
//! @file am_app_KWD_AMA.h
//!
//! @brief header file of AMA protocol handler
//
// ****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_APP_KWD_AMA_H
#define AM_APP_KWD_AMA_H

#include "accessories.pb.h"

#define AMA_TRANSPORT_HEADER_VERSION_MASK       0xF000
#define AMA_TRANSPORT_HEADER_STEAM_ID_MASK      0x0F80
#define AMA_TRANSPORT_HEADER_LENGTH_TYPE_MASK   0x01

typedef struct
{
  #if USE_OUTPUT_AMVOS_AMA
    uint8_t ama_buf[AMA_BUFFER_SIZE + 3];
  #else
    uint8_t* buf;
  #endif
    uint32_t len;
    uint32_t reserved;
} sRadioQueue_t;

typedef enum
{
    VOS_AMA_INIT = 0,           // New state for disconnected time
    VOS_AMA_IDLE,
    VOS_AMA_LISTENING,
    VOS_AMA_PROCESSING,
    VOS_AMA_SPEAKING
} eVosAmaStatus_t;

//*****************************************************************************
// External function declaration
//*****************************************************************************

extern bool am_app_KWD_AMA_start_speech_send(void);
extern int am_app_KWD_AMA_rx_handler(uint8_t *data, uint16_t len);
extern bool am_app_KWD_AMA_keep_alive_send(void);

extern uint32_t g_ui32AmaDialogID;
extern eVosAmaStatus_t g_eAmaStatus;

#endif  // #ifndef AM_APP_KWD_AMA_H
