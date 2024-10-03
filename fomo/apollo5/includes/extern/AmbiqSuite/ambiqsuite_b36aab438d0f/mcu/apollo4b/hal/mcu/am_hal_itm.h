//*****************************************************************************
//
//! @file am_hal_itm.h
//!
//! @brief Functions for operating the instrumentation trace macrocell
//!
//! @addtogroup itm4_4b ITM - Instrumentation Trace Macrocell
//! @ingroup apollo4b_hal
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

#ifndef AM_HAL_ITM_H
#define AM_HAL_ITM_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Sync Packet Defines
//
//*****************************************************************************
#define AM_HAL_ITM_SYNC_REG             23
#define AM_HAL_ITM_SYNC_VAL             0xF8F8F8F8

//*****************************************************************************
//
// PrintF Setup
//
//*****************************************************************************
#define AM_HAL_ITM_PRINT_NUM_BYTES      1
#define AM_HAL_ITM_PRINT_NUM_REGS       1
extern uint32_t am_hal_itm_print_registers[AM_HAL_ITM_PRINT_NUM_REGS];

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern void am_hal_itm_enable(void);
extern void am_hal_itm_disable(void);
extern void am_hal_itm_not_busy(void);
extern void am_hal_itm_sync_send(void);
extern void am_hal_itm_trace_port_enable(uint8_t ui8portNum);
extern void am_hal_itm_trace_port_disable(uint8_t ui8portNum);
extern bool am_hal_itm_stimulus_not_busy(uint32_t ui32StimReg);
extern void am_hal_itm_stimulus_reg_word_write(uint32_t ui32StimReg,
                                               uint32_t ui32Value);
extern void am_hal_itm_stimulus_reg_short_write(uint32_t ui32StimReg,
                                                uint16_t ui16Value);
extern void am_hal_itm_stimulus_reg_byte_write(uint32_t ui32StimReg,
                                               uint8_t ui8Value);
extern bool am_hal_itm_print_not_busy(void);
extern void am_hal_itm_print(char *pcString);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_ITM_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

