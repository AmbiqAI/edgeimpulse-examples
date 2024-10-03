//*****************************************************************************
//
//  am_hal_i2c_bit_bang.h
//! @file
//!
//! @brief I2C bit bang module.
//!
//! These functions implement the I2C bit bang utility
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_HAL_I2C_BIT_BANG_H
#define AM_HAL_I2C_BIT_BANG_H

//*****************************************************************************
//
// Enumerated return constants
//
//*****************************************************************************
typedef enum
{
    AM_HAL_I2C_BIT_BANG_SUCCESS = 0,
    AM_HAL_I2C_BIT_BANG_ADDRESS_NAKED,
    AM_HAL_I2C_BIT_BANG_DATA_NAKED,
    AM_HAL_I2C_BIT_BANG_CLOCK_TIMEOUT,
    AM_HAL_I2C_BIT_BANG_DATA_TIMEOUT,
    AM_HAL_I2C_BIT_BANG_STATUS_MAX,
}am_hal_i2c_bit_bang_enum_e;

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern am_hal_i2c_bit_bang_enum_e am_hal_i2c_bit_bang_init(uint32_t sck_gpio_number,
                                     uint32_t sda_gpio_number);

extern am_hal_i2c_bit_bang_enum_e am_hal_i2c_bit_bang_send(uint8_t address,
                                                    uint32_t number_of_bytes,
                                                    uint8_t *pData,
                                                    uint8_t ui8Offset,
                                                    bool bUseOffset,
                                                    bool bNoStop);

extern am_hal_i2c_bit_bang_enum_e am_hal_i2c_bit_bang_receive(uint8_t address,
                                                    uint32_t number_of_bytes,
                                                    uint8_t *pData,
                                                    uint8_t ui8Offset,
                                                    bool bUseOffset,
                                                    bool bNoStop);
#ifdef __cplusplus
}
#endif

#endif //AM_HAL_I2C_BIT_BANG_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
