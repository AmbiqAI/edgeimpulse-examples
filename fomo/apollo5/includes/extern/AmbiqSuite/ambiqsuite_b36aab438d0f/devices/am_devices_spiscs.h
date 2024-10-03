//*****************************************************************************
//
//! @file am_devices_spiscs.h
//!
//! @brief Slave-side functions for the SPISC protocol
//!
//! Slave-side functions for the SPI "switched configuration" protocol.
//!
//! @addtogroup spiscs SPISCS - SPI Switched Configuration Slave Protocol
//! @ingroup devices
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
#ifndef AM_DEVICES_SPISCS_H
#define AM_DEVICES_SPISCS_H

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Set up the necessary pins and configuration registers for 4-wire SPI
//! communication between the IO Master and the IO Slave.
//
//*****************************************************************************
extern void am_devices_spiscs_init(void);

//*****************************************************************************
//
//! @brief
//! @param ui32Status
//
//*****************************************************************************
extern void am_devices_spiscs_fifo_service(uint32_t ui32Status);
//*****************************************************************************
//
//! @brief
//! @param ui32Status
//
//*****************************************************************************

extern void am_devices_spiscs_acc_service(uint32_t ui32Status);
//*****************************************************************************
//
//! @brief
//! @param pui8Data
//! @param ui8Size
//
//*****************************************************************************

extern void am_devices_spiscs_write(uint8_t *pui8Data, uint8_t ui8Size);

//*****************************************************************************
//
//! @brief
//! @param pui8Data
//! @param ui32MaxSize
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_spiscs_read(uint8_t *pui8Data, uint32_t ui32MaxSize);

//*****************************************************************************
//
//! @brief Check to see if we've received any messages.
//!
//! This function is used to check the SPI Slave recieve buffer for messages.
//!
//! @return
//
//*****************************************************************************
extern bool am_devices_spiscs_check(void);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_SPISCS_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

