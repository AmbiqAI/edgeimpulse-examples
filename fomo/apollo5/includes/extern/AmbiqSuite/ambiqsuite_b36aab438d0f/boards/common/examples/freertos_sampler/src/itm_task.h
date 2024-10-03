//*****************************************************************************
//
//! @file itm_task.h
//!
//! @brief FreeRTOS task to manage output thruugh item registers.
//!
//! This task serves an itm register write queue. Queue entries can be
//! arbitrary ITM register writes or print strings. This task includes a
//! state machine to manage the power state of the ITM/TPIU/SWO pieces of this
//! output streaming mechansim.  When the ITM has been idle for 3 ticks then
//! everything is powered down. When a queue element arrives to a powered down
//! ITM flow then everything is powered up and it all starts over.
//!
//! The state machine also manages the output of strings of up to 128
//! characters. These 128 byte strings come attached to a single queue element.
//!
//! The goal is to provide thread safe print output from multiple tasks writing
//! to the single ITM output queue. While managing the ITM/TPIU/SWO power
//! state.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef ITM_TASK_H
#define ITM_TASK_H

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define ITM_TASK_DATA_READY_EVENT            0x1

//*****************************************************************************
//
// Typedefs
//
//*****************************************************************************

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern TaskHandle_t xITMTask;
extern EventGroupHandle_t xITMEvents;



//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern void vITMTask(void *pvParameters);
extern void ITMTaskSetup(void);
extern void ITMWrite2Stimulus(uint32_t stim_reg, uint32_t stim_value);

#ifdef __cplusplus
}
#endif

#endif // ITM_TASK_H


