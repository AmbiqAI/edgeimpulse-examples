//*****************************************************************************
//
//! @file am_bsp_iom.c
//!
//! @brief Brazo board support for SPI/I2C
//!
//! Structures and functions to support communication with various SPI and I2C
//! devices on Brazo.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_bsp_iom.h"

//*****************************************************************************
//
// Global IOM configuration settings.
//
//*****************************************************************************
am_hal_iom_config_t g_am_bsp_iom0_config =
{
    AM_HAL_IOM_SPIMODE,         // Interface mode
    AM_HAL_IOM_2MHZ,            // Interface speed
    0,                          // Clock phase
    0,                          // Clock polarity
    4,                          // FIFO write threshold (in bytes)
    59                          // FIFO read threshold (in bytes)
};

am_hal_iom_config_t g_am_bsp_iom1_config =
{
    AM_HAL_IOM_SPIMODE,         // Interface mode
    AM_HAL_IOM_250KHZ,          // Interface speed
    0,                          // Clock phase
    0,                          // Clock polarity
    4,                          // FIFO write threshold (in bytes)
    12                          // FIFO read threshold (in bytes)
};

//*****************************************************************************
//
// Global IOM device structures.
//
//*****************************************************************************
am_hal_iom_spi_device_t am_bsp_iom_sSpiFlash =
{
    1,                          // Module number
    6,                          // Chip Select
    0,                          // Additional options.
};

am_hal_iom_spi_device_t am_bsp_iom_sADXL362 =
{
    1,                          // Module number
    1,                          // Chip Select
    0,                          // Additional options.
};
// #### INTERNAL BEGIN ####
am_hal_iom_spi_device_t am_bsp_iom_sADXL364 =
{
    1,                          // Module number
    1,                          // Chip Select
    0,                          // Additional options.
};
// #### INTERNAL END ####

//*****************************************************************************
//
// Structures that map GPIO pin numbers to their associated IOM CE pin numbers.
//
// The index of each item in the array is its chip-enable number, while the
// value of the entry is the GPIO number. For example, if IOM0CE0 should appear
// physically on GPIO 42, then am_bsp_iom_pui8IOMCE0Pins[0] should be set to
// 42.
//
//*****************************************************************************
uint8_t am_bsp_iom_pui8IOM0CEPins[] =
{
    42,
    43,
    44,
    45,
    46,
    47,
    48,
    49,
};

uint8_t am_bsp_iom_pui8IOM1CEPins[] =
{
    35,
    24,                         // ADXL
    37,
    // #### INTERNAL BEGIN ####
    //dbaker fixme 38,
    // #### INTERNAL END ####
    15,
    27,
    28,
    29,                         // SPI Flash
    30,
};

//*****************************************************************************
//
//! @brief Performs basic configuration of the IO master module
//!
//! This function configures the IO master module based on the structures
//! defined in IOM portion of the BSP.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_iom_init(void)
{
    //
    // Configure each IO master module using the global BSP structures.
    //
    am_hal_iom_config(0, &g_am_bsp_iom0_config);

// #### INTERNAL BEGIN ####
#if 0
not needed for Apollo A.1
    //
    // Tell the bit-bang IOM SPI layer which pins it should use for each
    // chip-select signal
    //
    am_hal_iom_bb_ce_select(am_bsp_iom_pui8IOM0CEPins,
                            am_bsp_iom_pui8IOM1CEPins);

    //
    // Enable the IO master modules
    //
    am_hal_iom_bb_enable(0);
    am_hal_iom_bb_enable(1);
#endif
// #### INTERNAL END ####
}
