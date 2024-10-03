//*****************************************************************************
//
//! @file am_hal_tpiu.h
//!
//! @brief Support functions for the ARM TPIU module
//!
//! Provides support functions for configuring the ARM TPIU module
//!
//! @addtogroup tpiu4 TPIU - Trace Port Interface Unit
//! @ingroup apollo5a_hal
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
#ifndef AM_HAL_TPIU_H
#define AM_HAL_TPIU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @name TPIU bit rate defines.
//! @{
//
//*****************************************************************************
#define AM_HAL_TPIU_BAUD_57600      (115200 / 2)
#define AM_HAL_TPIU_BAUD_115200     (115200 * 1)
#define AM_HAL_TPIU_BAUD_230400     (115200 * 2)
#define AM_HAL_TPIU_BAUD_460800     (115200 * 4)
#define AM_HAL_TPIU_BAUD_250000     (1000000 / 4)
#define AM_HAL_TPIU_BAUD_500000     (1000000 / 2)
#define AM_HAL_TPIU_BAUD_1M         (1000000 * 1)
#define AM_HAL_TPIU_BAUD_2M         (1000000 * 2)
#define AM_HAL_TPIU_BAUD_DEFAULT    (AM_HAL_TPIU_BAUD_1M)
//! @}

// ##### INTERNAL BEGIN #####
#if 0
//*****************************************************************************
//
//! @name TPIU register defines.
//! @{
//
//*****************************************************************************
#define AM_HAL_TPIU_SSPSR       0xE0040000  //! Supported Parallel Port Sizes
#define AM_HAL_TPIU_CSPSR       0xE0040004  //! Current Parallel Port Size
#define AM_HAL_TPIU_ACPR        0xE0040010  //! Asynchronous Clock Prescaler
#define AM_HAL_TPIU_SPPR        0xE00400F0  //! Selected Pin Protocol
#define AM_HAL_TPIU_TYPE        0xE0040FC8  //! TPIU Type
//! @}

//*****************************************************************************
//
// TPIU ACPR defines.
//
//*****************************************************************************
#define AM_HAL_TPIU_ACPR_SWOSCALER_M    0x0000FFFF  //! SWO baud rate prescalar

//*****************************************************************************
//
//! TPIU_SPPR TXMODE defines.
//
//*****************************************************************************
typedef enum
{
    AM_HAL_TPIU_PIN_PROTOCOL_PARALLEL     = 0,
    AM_HAL_TPIU_PIN_PROTOCOL_MANCHESTER   = 1,
    AM_HAL_TPIU_PIN_PROTOCOL_NRZ          = 2
} am_hal_tpiu_pin_protocol_e;

//*****************************************************************************
//
//! @name TPIU Type defines
//! @{
//
//*****************************************************************************
#define AM_HAL_TPIU_TYPE_NRZVALID       0x00000800
#define AM_HAL_TPIU_TYPE_MANCVALID      0x00000400
#define AM_HAL_TPIU_TYPE_PTINVALID      0x00000200
#define AM_HAL_TPIU_TYPE_FIFOSZ_M       0x000001C0
//! @}

//*****************************************************************************
//
//! @brief Structure used for configuring the TPIU
//
//*****************************************************************************
typedef enum
{
    AM_HAL_TPIU_TRACECLKIN_6MHZ         = MCUCTRL_DBGCTRL_DBGCLKSEL_HFRCDIV2,
    AM_HAL_TPIU_TRACECLKIN_3MHZ         = MCUCTRL_DBGCTRL_DBGCLKSEL_HFRCDIV2,
    AM_HAL_TPIU_TRACECLKIN_750KHZ       = MCUCTRL_DBGCTRL_DBGCLKSEL_HFRCDIV8,
    AM_HAL_TPIU_TRACECLKIN_MAX          = AM_HAL_TPIU_TRACECLKIN_750KHZ
} am_hal_tpiu_traceclkin_e;

//*****************************************************************************
//
//! @brief Structure used for configuring the TPIU
//
//*****************************************************************************
typedef struct
{
    //
    //! If ui32SetItmBaud is non-zero, the ITM frequency is set to the given
    //!  frequency, and is based on a divide-by-8 HFRC TPIU clock.
    //! If zero, other structure members are used to set the TPIU configuration.
    //
    uint32_t ui32SetItmBaud;

    //
    //! Protocol to use for the TPIU
    //!
    //! Valid values for ui32PinProtocol are:
    //!
    //!     AM_HAL_TPIU_SPPR_PARALLEL
    //!     AM_HAL_TPIU_SPPR_MANCHESTER
    //!     AM_HAL_TPIU_SPPR_NRZ
    //
    am_hal_tpiu_pin_protocol_e ePinProtocol;
} am_hal_tpiu_config_t;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Enable the clock to the TPIU module.
//!
//! This function enables the clock to the TPIU module.
//
//*****************************************************************************
extern void am_hal_tpiu_clock_enable(void);

//*****************************************************************************
//
//! @brief Disable the clock to the TPIU module.
//!
//! This function disables the clock to the TPIU module.
//
//*****************************************************************************
extern void am_hal_tpiu_clock_disable(void);

//*****************************************************************************
//
//! @brief Set the output port width of the TPIU
//!
//! @param ui32PortWidth - The desired port width (in bits)
//!
//! This function uses the TPIU_CSPSR register to set the desired output port
//! width of the TPIU.
//
//*****************************************************************************
extern void am_hal_tpiu_port_width_set(uint32_t ui32PortWidth);

//*****************************************************************************
//
//! @brief Read the supported_output port width of the TPIU
//!
//! This function uses the \e TPIU_SSPSR register to set the supported output
//! port widths of the TPIU.
//!
//! @return Current width of the TPIU output port
//
//*****************************************************************************
extern uint32_t am_hal_tpiu_supported_port_width_get(void);

//*****************************************************************************
//
//! @brief Read the output port width of the TPIU
//!
//! This function uses the \e TPIU_CSPSR register to set the desired output
//! port width of the TPIU.
//!
//! @return Current width of the TPIU output port
//
//*****************************************************************************
extern uint32_t am_hal_tpiu_port_width_get(void);

//*****************************************************************************
//
//! @brief Configure the TPIU based on the values in the configuration struct.
//!
//! @param psConfig - pointer to an am_hal_tpiu_config_t structure containing
//! the desired configuration information.
//!
//! This function reads the provided configuration structure, and sets the
//! relevant TPIU registers to achieve the desired configuration.
//
//*****************************************************************************
extern void am_hal_tpiu_configure(am_hal_tpiu_config_t *psConfig);
#endif
// ##### INTERNAL END #####

//*****************************************************************************
//
//! @brief Enables the TPIU
//!
//! This function enables the ARM TPIU by setting the TPIU registers and then
//! enabling the TPIU clock source in MCU control register.
//!
//! @param ui32SetItmBaud - structure for configuration.
//!     But for simplicity, ui32SetItmBaud can be set to one of the
//!      following, in which case all other structure members are ignored.
//!      In this case, the given BAUD rate is based on a div-by-8 HFRC clock.
//!         AM_HAL_TPIU_BAUD_57600
//!         AM_HAL_TPIU_BAUD_115200
//!         AM_HAL_TPIU_BAUD_230400
//!         AM_HAL_TPIU_BAUD_460800
//!         AM_HAL_TPIU_BAUD_500000
//!         AM_HAL_TPIU_BAUD_1M
//
//*****************************************************************************
extern uint32_t am_hal_tpiu_enable(uint32_t ui32SetItmBaud);

//*****************************************************************************
//
//! @brief Disables the TPIU
//!
//! This function disables the ARM TPIU by disabling the TPIU clock source
//! in MCU control register.
//
//*****************************************************************************
extern uint32_t am_hal_tpiu_disable(void);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_TPIU_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

