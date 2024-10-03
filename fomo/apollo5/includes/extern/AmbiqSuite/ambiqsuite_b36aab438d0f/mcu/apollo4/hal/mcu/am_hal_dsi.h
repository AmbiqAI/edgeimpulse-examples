//*****************************************************************************
//
//! @file am_hal_dsi.h
//!
//! @brief Hardware abstraction for the DSI
//!
//! @addtogroup
//! @ingroup
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_HAL_DSI_H
#define AM_HAL_DSI_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// DSI error codes.
//
//*****************************************************************************
/*typedef enum
{
    AM_HAL_DSI_INT_STATUS_RX_START_TRANS_ERROR = AM_HAL_STATUS_MODULE_SPECIFIC_START,
    AM_HAL_DSI_INT_STATUS_RX_START_TRANS_SYNC_ERROR,
    AM_HAL_DSI_INT_STATUS_RX_END_TRANS_SYNC_ERROR,
    AM_HAL_DSI_INT_STATUS_RX_ESCAPE_ENTRY_ERROR,
    AM_HAL_DSI_INT_STATUS_RX_LP_TX_SYNC_ERROR,
    AM_HAL_DSI_INT_STATUS_RX_PERIPH_TIMEOUT_ERROR,
    AM_HAL_DSI_INT_STATUS_RX_FALSE_CONTROL_ERROR,
    AM_HAL_DSI_INT_STATUS_RX_ECC_SINGLE_BIT_ERROR,
    AM_HAL_DSI_INT_STATUS_RX_ECC_MULTI_BIT_ERROR,
    AM_HAL_DSI_INT_STATUS_START_TRANS_SYNC_ERROR,

}
am_hal_dsi_interrupt_status_t;*/

//*****************************************************************************
//
// DSI clock lane frequency.
//
//*****************************************************************************
typedef enum
{
    AM_HAL_DSI_FREQ_TRIM_X1  = 0x40,
    AM_HAL_DSI_FREQ_TRIM_X2  = 0x01,
    AM_HAL_DSI_FREQ_TRIM_X3  = 0x41,
    AM_HAL_DSI_FREQ_TRIM_X4  = 0x02,
    AM_HAL_DSI_FREQ_TRIM_X5  = 0x42,
    AM_HAL_DSI_FREQ_TRIM_X6  = 0x03,
    AM_HAL_DSI_FREQ_TRIM_X7  = 0x43,
    AM_HAL_DSI_FREQ_TRIM_X8  = 0x04,
    AM_HAL_DSI_FREQ_TRIM_X9  = 0x44,
    AM_HAL_DSI_FREQ_TRIM_X10 = 0x05,
    AM_HAL_DSI_FREQ_TRIM_X11 = 0x45,
    AM_HAL_DSI_FREQ_TRIM_X12 = 0x06,
    AM_HAL_DSI_FREQ_TRIM_X13 = 0x46,
    AM_HAL_DSI_FREQ_TRIM_X14 = 0x07,
    AM_HAL_DSI_FREQ_TRIM_X15 = 0x47,
    AM_HAL_DSI_FREQ_TRIM_X16 = 0x08,
    AM_HAL_DSI_FREQ_TRIM_X17 = 0x48,
    AM_HAL_DSI_FREQ_TRIM_X18 = 0x09,
    AM_HAL_DSI_FREQ_TRIM_X19 = 0x49,
    AM_HAL_DSI_FREQ_TRIM_X20 = 0x0A,
    AM_HAL_DSI_FREQ_TRIM_X21 = 0x4A,
    AM_HAL_DSI_FREQ_TRIM_X22 = 0x0B,
    AM_HAL_DSI_FREQ_TRIM_X23 = 0x4B,
    AM_HAL_DSI_FREQ_TRIM_X24 = 0x0C,
    AM_HAL_DSI_FREQ_TRIM_X25 = 0x4C,
    AM_HAL_DSI_FREQ_TRIM_X26 = 0x0D,
    AM_HAL_DSI_FREQ_TRIM_X27 = 0x4D,
    AM_HAL_DSI_FREQ_TRIM_X28 = 0x0E,
    AM_HAL_DSI_FREQ_TRIM_X29 = 0x4E,
    AM_HAL_DSI_FREQ_TRIM_X30 = 0x0F,
    AM_HAL_DSI_FREQ_TRIM_X31 = 0x4F,
} am_hal_dsi_freq_trim_e;


//*****************************************************************************
//
// DBI to DSI: DBI data width.
//
//*****************************************************************************
typedef enum
{
    AM_HAL_DSI_DBI_WIDTH_8  = 8,
    AM_HAL_DSI_DBI_WIDTH_16 = 16,
} am_hal_dsi_dbi_width_e;

//! @}

//*****************************************************************************
//
// External functions.
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief DSI configuration
//!
//! @param ui8LanesNum          - Number of lanes.
//! @param ui8DBIBusWidth       - Width of DBI bus.
//! @param ui32FreqTrim         - DPHY output frequency trim.
//! @param bSendUlpsPattern     - Unused parameter
//!
//! This function should be called after DSI power is enabled.
//!
//! @return AM_HAL_STATUS_SUCCESS
//
//*****************************************************************************
extern uint32_t am_hal_dsi_para_config(uint8_t ui8LanesNum,
                                       uint8_t ui8DBIBusWidth,
                                       uint32_t ui32FreqTrim,
                                       bool bSendUlpsPattern);

//*****************************************************************************
//
//! @brief DSI initialization
//!
//! Configure power and clock of DSI.
//!
//! @return AM_HAL_STATUS_SUCCESS
//
//*****************************************************************************
extern uint32_t
am_hal_dsi_init(void);

//*****************************************************************************
//
//! @brief DSI deinit
//!
//! Turn off power and clock of DSI.
//!
//! @return AM_HAL_STATUS_SUCCESS
//
//*****************************************************************************
extern uint32_t
am_hal_dsi_deinit(void);

//*****************************************************************************
//
//! @brief DSI configuration
//!
//! Configure DSI frequency and timing
//!
//! @return AM_HAL_STATUS_SUCCESS
//
//*****************************************************************************
extern uint32_t
am_hal_dsi_timing(uint32_t ui32FreqTrim);

//*****************************************************************************
//
//! @brief DSI state
//!
//! Enter ULPS mode
//!
//! @return AM_HAL_STATUS_SUCCESS
//
//*****************************************************************************
extern uint32_t
am_hal_dsi_ulps_entry(void);

//*****************************************************************************
//
//! @brief DSI state
//!
//! Exit ULPS mode
//!
//! @return AM_HAL_STATUS_SUCCESS
//
//*****************************************************************************
extern uint32_t
am_hal_dsi_ulps_exit(void);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_DSI_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
