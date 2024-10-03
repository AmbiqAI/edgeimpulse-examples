//*****************************************************************************
//
//! @file pixart_pah8002.h
//
//*****************************************************************************
#ifndef PIXART_PAH8002_H
#define PIXART_PAH8002_H

#ifdef __cplusplus
extern "C"
{
#endif

// defines to be used in am_devices_amx8x5_t structure.
#define PAH8002_MODE_SPI                  true
#define PAH8002_MODE_I2C                  false

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
pixart_pah8002_t;

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
extern uint8_t readRegister(uint8_t addr, uint8_t *data);
extern uint8_t writeBMI160(uint8_t addr, uint8_t data);
extern uint8_t readBMI160(uint8_t addr, uint8_t *data);
extern bool accelerometer_init(void);
extern void accelerometer_get_fifo(int16_t **fifo, uint32_t *fifo_size);

extern void
pixart_callbacks_config(am_hal_iom_callback_t pfnYield,
                                am_hal_iom_callback_t pfnCallback);


extern void pah8002_low_power_enable(void);
extern void pah8002_low_power_disable(void);
extern void pah8002_accel_set(int16_t *sensor_data, uint32_t sensor_data_size);

/* ---------------------------------------------------------------
 * Below for PAH8002 main code
*///--------------------------------------------------------------
extern uint32_t get_sys_tick(void);

//---------------------------------------PAH8002 functions----------------------
extern bool pah8002_init(void);
extern void pah8002_log(void);
extern uint8_t pah8002_task(void);
extern void pah8002_intr_isr(void);

#ifdef __cplusplus
}
#endif

#endif // PIXART_PAH8002_H
