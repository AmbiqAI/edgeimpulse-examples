//*****************************************************************************
//
//! @file am_util_delay.c
//!
//! @brief A few useful delay functions.
//!
//! Functions for fixed delays.
//!
//! @addtogroup delay Delay Functionality
//! @ingroup utils
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

#include  <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_util_delay.h"

//*****************************************************************************
//
// Delays for a desired amount of loops.
//
//*****************************************************************************
void
am_util_delay_cycles(uint32_t ui32Iterations)
{
    //
    // Call the BOOTROM cycle delay function
    //
#if defined(AM_PART_APOLLO4_API) || defined(AM_PART_APOLLO5_API)
    am_hal_delay_us( ui32Iterations);
#else
    am_hal_flash_delay(ui32Iterations);
#endif // AM_PART_APOLLO4_API || AM_PART_APOLLO5_API
}

//*****************************************************************************
//
// Delays for a desired amount of milliseconds.
//
//*****************************************************************************
void
am_util_delay_ms(uint32_t ui32MilliSeconds)
{
#if defined(AM_PART_APOLLO4_API) || defined(AM_PART_APOLLO5_API)
    am_hal_delay_us( ui32MilliSeconds * 1000);
#else // AM_PART_APOLLO4_API || AM_PART_APOLLO5_API
    uint32_t ui32Loops, ui32HFRC;
#if AM_APOLLO3_CLKGEN
    am_hal_clkgen_status_t sClkgenStatus;
    am_hal_clkgen_status_get(&sClkgenStatus);
    ui32HFRC = sClkgenStatus.ui32SysclkFreq;
#else // AM_APOLLO3_CLKGEN
    ui32HFRC = am_hal_clkgen_sysclk_get();
#endif // AM_APOLLO3_CLKGEN
    ui32Loops = ui32MilliSeconds * (ui32HFRC / 3000);

    //
    // Call the BOOTROM cycle delay function
    //
    am_hal_flash_delay(ui32Loops);
#endif // AM_PART_APOLLO4_API || AM_PART_APOLLO5_API
}

//*****************************************************************************
//
// Delays for a desired amount of microseconds.
//
//*****************************************************************************
void
am_util_delay_us(uint32_t ui32MicroSeconds)
{
#if defined(AM_PART_APOLLO4_API) || defined(AM_PART_APOLLO5_API)
    am_hal_delay_us( ui32MicroSeconds );
#else // AM_PART_APOLLO4_API || AM_PART_APOLLO5_API
    uint32_t ui32Loops, ui32HFRC;

#if AM_APOLLO3_CLKGEN
    am_hal_clkgen_status_t sClkgenStatus;
    am_hal_clkgen_status_get(&sClkgenStatus);
    ui32HFRC = sClkgenStatus.ui32SysclkFreq;
#else // AM_APOLLO3_CLKGEN
    ui32HFRC = am_hal_clkgen_sysclk_get();
#endif // AM_APOLLO3_CLKGEN
    ui32Loops = ui32MicroSeconds * (ui32HFRC / 3000000);

    //
    // Call the BOOTROM cycle delay function
    //
    am_hal_flash_delay(ui32Loops);
#endif // AM_PART_APOLLO4_API || AM_PART_APOLLO5_API
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

