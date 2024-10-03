//*****************************************************************************
//
//! @file am_bsp_iom.h
//!
//! @brief Brazo board support for SPI/I2C
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_BSP_IOM_H
#define AM_BSP_IOM_H

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern am_hal_iom_config_t g_am_bsp_iom0_spi_config;
extern am_hal_iom_config_t g_am_bsp_iom0_i2c_config;
extern am_hal_iom_config_t g_am_bsp_iom1_config;

extern am_hal_iom_spi_device_t am_bsp_iom_sSpiFlash;
extern am_hal_iom_spi_device_t am_bsp_iom_sADXL362;
// #### INTERNAL BEGIN ####
extern am_hal_iom_spi_device_t am_bsp_iom_sADXL364;
// #### INTERNAL END ####

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern void am_bsp_iom_init(void);

#ifdef __cplusplus
}
#endif

#endif // AM_BSP_IOM_H

