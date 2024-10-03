//*****************************************************************************
//
//! @file iom_clock_config.c
//!
//! @brief Allow changes in IOM sclk speeds
//!
//!
//! @addtogroup devices External Device Control Library
//! @addtogroup ADXL363 SPI Device Control for the ADXL363 External Accelerometer
//! @ingroup examples
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
#ifndef IOM_CLOCK_CONFIG_H
#define IOM_CLOCK_CONFIG_H


#ifdef __cplusplus
extern "C"
{
#endif


#include "am_hal_iom.h"
#include "am_mcu_apollo.h"

#define HFRC_CLOCK 96000000

uint32_t am_hal_iom_configure_clk_tst(void *pHandle,
                                      const am_hal_iom_config_t *psConfig ) ;

uint32_t am_hal_iom_set_intial_clk(uint32_t  ui32Module, uint32_t ui32ClockFreq) ;

#ifdef __cplusplus
}
#endif

#endif //IOM_CLOCK_CONFIG_H
