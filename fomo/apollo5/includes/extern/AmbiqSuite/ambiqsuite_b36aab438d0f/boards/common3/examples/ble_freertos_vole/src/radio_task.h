//*****************************************************************************
//
//! @file radio_task.h
//!
//! @brief Functions and variables related to the radio task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef RADIO_TASK_H
#define RADIO_TASK_H
#include "vole_board_config.h"
/*
typedef struct{
  #if USE_OUTPUT_AMVOS_AMA
    uint8_t ama_buf[BLE_MSBC_DATA_BUFFER_SIZE + 3];
  #else
    uint8_t* buf;
  #endif
    uint32_t len;
    uint32_t reserved;
}sRadioQueue_t;
*/
//*****************************************************************************
//
// Radio task handle.
//
//*****************************************************************************
extern TaskHandle_t radio_task_handle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void RadioTaskSetup(void);
extern void RadioTask(void *pvParameters);

#endif // RADIO_TASK_H
