//*****************************************************************************
//
//! @file pixart_pah8005.h
//
//*****************************************************************************
#ifndef PIXART_pah8005_H
#define PIXART_pah8005_H

#ifdef __cplusplus
extern "C"
{
#endif

// defines to be used in am_devices_amx8x5_t structure.
#define pah8005_MODE_SPI                  true
#define pah8005_MODE_I2C                  false

// defines to be used in am_devices_amx8x5_t structure.
#define BMI160_MODE_SPI                  true
#define BMI160_MODE_I2C                  false

typedef struct
{
    //
    // SPI or I2C mode.
    //
    bool bMode;

    //
    // Module number to use for IOM access.
    //
    uint32_t ui32IOMModule;

    //
    // Chip Select number to use for IOM access. (unused in I2C mode).
    //
    uint32_t ui32ChipSelect;

    //
    // Address for I2C communication. (unused in SPI mode)
    //
    uint32_t ui32Address;
}
pixart_pah8005_t;

typedef struct
{
    //
    // SPI or I2C mode.
    //
    bool bMode;

    //
    // Module number to use for IOM access.
    //
    uint32_t ui32IOMModule;

    //
    // Chip Select number to use for IOM access. (unused in I2C mode).
    //
    uint32_t ui32ChipSelect;

    //
    // Address for I2C communication. (unused in SPI mode)
    //
    uint32_t ui32Address;
}
bosch_bmi160_t;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern uint8_t writeRegister(uint8_t addr, uint8_t data);
extern uint8_t readRegister(uint8_t addr);
extern uint8_t writeBMI160(uint8_t addr, uint8_t data);
extern uint8_t readBMI160(uint8_t addr, uint8_t *data);
extern bool accelerometer_init(void);

/* ---------------------------------------------------------------
 * Below for pah8005 main code
*///--------------------------------------------------------------
extern uint32_t get_sys_tick(void);

//---------------------------------------pah8005 functions----------------------
extern bool pah8005_init(void);
extern void pah8005_task(void);
extern bool Pixart_HRD(void);
extern void led_ctrl(uint8_t);

#ifdef __cplusplus
}
#endif

#endif // PIXART_pah8005_H
