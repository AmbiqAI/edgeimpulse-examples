//*****************************************************************************
//
//! @file dtm_api.h
//!
//! @brief Global includes for the dtm_main app.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef DTM_API_H
#define DTM_API_H

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

/*****************************************************************************/
/* ENABLE_DTM_MODE_FEATURE is used to enable or disable the DTM mode in general
 *  BLE example. 1: enable, 0: disable.
 * Add the judgement of this marco in your code when using the variable/function
 * of DTM mode, like:
 *
 * #if defined(ENABLE_DTM_MODE_FEATURE) && (ENABLE_DTM_MODE_FEATURE == 1)
 *
 * **codes**
 *
 * #endif
 */
/*****************************************************************************/
#define ENABLE_DTM_MODE_FEATURE       1

//*****************************************************************************
//
// External variable Declarations
//
//*****************************************************************************
extern bool g_bDtmModeRunning;

//*****************************************************************************
//
// Function Declarations
//
//*****************************************************************************

#ifndef BLE_BRIDGE_SINGLE_MODE
void ui_switch_to_dtm(void);
void ui_exit_from_dtm(void);
void reset_ble_and_enter_dtm(void);
#endif

void dtm_init(void);
void dtm_process(void);
void serial_interface_init(void);
void serial_interface_deinit(void);
void serial_data_read(uint8_t* pui8Data, uint32_t* ui32Length);
void serial_data_write(uint8_t* pui8Data, uint32_t ui32Length);
void serial_irq_enable(void);
void serial_irq_disable(void);
void serial_task(void);

#endif
