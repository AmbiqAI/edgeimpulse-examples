//*****************************************************************************
//
//  am_hal_tpiu.h
//! @file
//!
//! @brief Definitions and structures for working with the TPIU.
//!
//! @addtogroup tpiu4 Trace Port Interface Unit (TPIU)
//! @ingroup apollo4hal
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
  // TPIU bit rate defines.
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

  // ##### INTERNAL BEGIN #####
#if 0
//*****************************************************************************
//
// TPIU register defines.
//
//*****************************************************************************
#define AM_HAL_TPIU_SSPSR       0xE0040000  //! Supported Parallel Port Sizes
#define AM_HAL_TPIU_CSPSR       0xE0040004  //! Current Parallel Port Size
#define AM_HAL_TPIU_ACPR        0xE0040010  //! Asynchronous Clock Prescaler
#define AM_HAL_TPIU_SPPR        0xE00400F0  //! Selected Pin Protocol
#define AM_HAL_TPIU_TYPE        0xE0040FC8  //! TPIU Type

//*****************************************************************************
//
// TPIU ACPR defines.
//
//*****************************************************************************
#define AM_HAL_TPIU_ACPR_SWOSCALER_M    0x0000FFFF  //! SWO baud rate prescalar

//*****************************************************************************
//
// TPIU_SPPR TXMODE defines.
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
  // TPIU Type defines
  //
  //*****************************************************************************
#define AM_HAL_TPIU_TYPE_NRZVALID       0x00000800
#define AM_HAL_TPIU_TYPE_MANCVALID      0x00000400
#define AM_HAL_TPIU_TYPE_PTINVALID      0x00000200
#define AM_HAL_TPIU_TYPE_FIFOSZ_M       0x000001C0

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
    // If ui32SetItmBaud is non-zero, the ITM frequency is set to the given
    //  frequency, and is based on a divide-by-8 HFRC TPIU clock.
    // If zero, other structure members are used to set the TPIU configuration.
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
extern void am_hal_tpiu_clock_enable(void);
extern void am_hal_tpiu_clock_disable(void);
extern void am_hal_tpiu_port_width_set(uint32_t ui32PortWidth);
extern uint32_t am_hal_tpiu_supported_port_width_get(void);
extern uint32_t am_hal_tpiu_port_width_get(void);
extern void am_hal_tpiu_configure(am_hal_tpiu_config_t *psConfig);
#endif
// ##### INTERNAL END #####
extern void am_hal_tpiu_enable(uint32_t ui32SetItmBaud);
extern void am_hal_tpiu_disable(void);

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
