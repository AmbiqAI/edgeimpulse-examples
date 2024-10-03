//*****************************************************************************
//
//! @file am_widget_adxl363.h
//!
//! @brief intermediate adxl363 driver interface
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_WIDGET2_IOM_H
#define AM_WIDGET2_IOM_H

#include "am_devices_adxl363.h"

//
// user INT2 pin as output from this micro, used to start accel conversion
//
#define PULSE_INT2

//
// use PWM to toggle the INT2 pin
//
#define USE_PWM

//
// use GPIO interrupt. The rising edge of INT1, indicating fifo watermark, will
// cause an interrupt
//
#define USE_INTERRUPT
//
// use non blocking transfers to read the acclerometer fifo
//
#define USE_NON_BLOCKING

#define FIFO_SAMPLE_THRESH 30
#define PWM_TIMER_CH 4
#define PWM_OUT_FREQ (100*100) // hz * 100
#define PWM_DUTY_CYCLE 50


//
// status for adxl363 int1 pin state check
//
typedef enum
{
    //
    // default value, this is an error
    //
    eAdxlInt_NA,
    //
    // int1 pin is high, read data
    //
    eAdxlIntStartRead,
    //
    // int1 pin is low, nothing happening, ok to sleep
    //
    eAdxlSleep,
    //
    // interal gpio error on int pin read
    //
    eAdxlError,
}
adxl_int_pin_status_e ;


//*****************************************************************************
//
//! @brief reads data from accelerometer fifo
//! this is called when the int1 accelerometer line goes high
//! @param uint32_t *ui32InBuff buffer where incoming data is stored
//! @param uint32_t number of bytes to read
//! @note not sure how much memory needs to be allocated to read 60 bytes, 64 bytes is not enough??
//! @note must be reading shorts or longs
//! @return generic status
//
//*****************************************************************************
uint32_t iom_widget_adxl363_readData(am_devices_adxl363_t *am_dev_adxl363, uint32_t *ui32InBuff, uint32_t maxIo,
                                     bool useNonBlocking);

//*****************************************************************************
//
//! @brief reads and tests the device ID
//! @param am_devices_adxl363_t *am_dev_adxl363 pointer to structure defined
//! in driver
//! @return generic status
//
//*****************************************************************************
uint32_t iom_widiget_adxl363_checkDevId( am_devices_adxl363_t *am_dev_adxl363v ) ;

//*****************************************************************************
//
//! @brief int pin handling
//!
//! @param amDevAdxl363  pointer to device/driver data struct
//! @return genric system status
//
//*****************************************************************************
uint32_t iom_widget_adxl3563_io_config(am_devices_adxl363_t *amDevAdxl363);

//*****************************************************************************
//
//! @brief shutdown the adxl363 pins and pwm
//
//*****************************************************************************
uint32_t iom_widget_adxl363_stop(void) ;

//*****************************************************************************
//
//! @brief Enable int1 pin interrupt
//!
//! @note Enable external interrupt when int1 from acceleromter goes high
//! signalling data buffer is read to read
//!
//! @return none
//
//*****************************************************************************
void iom_widget_adxl363_enable_fifo_int(bool enableInterrupt) ;

//*****************************************************************************
//
//! @brief a semi-general purpose PWM fucntion
//!
//! This function will start pwm at specified frequency and duty cycle
//! usign the specified timer on the specified output pin
//!
//! @note the output pin must be configured elsewhere
//!
//! @param ui32TimerNumber - timer used for pwm
//! @param ui32PinNumber   - output pin number
//! @param ui32Freq_x100   - freq * 100
//! @param ui32DCPercent   - duty cycle% 0-100
//!
//! @return status         - generic or interface specific status.
//
//*****************************************************************************
uint32_t
am_widget_adxl363_do_pwm(uint32_t ui32TimerNumber,
                         uint32_t ui32PinNumber,
                         uint32_t ui32Freq_x100,
                         uint32_t ui32DCPercent ) ;

//*****************************************************************************
//
//! @brief init adxl363 driver and iom settings
//!
//! @param amDevicesAdxl363
//! @param ui32iomNumber
//! @param ui32chipSelect
//!
//! @return generic status
//
//*****************************************************************************
uint32_t
am_widget_adxl363_cmn_init( am_devices_adxl363_t *amDevicesAdxl363,
                            uint32_t ui32iomNumber,
                            uint32_t ui32chipSelect) ;

//*****************************************************************************
//
//! @brief Set up the IOM (SPI) pins for the adxl363 on
//! the system test board, apollo4.
//!
//! @param ui32IomModuleNumber iom module used
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
uint32_t am_widget_adxl363_pinSetup(uint32_t ui32IomModuleNumber);

//*****************************************************************************
//
//! @brief deinit the driver for adxl363
//!
//! @param amDevicesAdxl363  - pointer to device variables
//! @return  status          - generic or interface specific status.
//
//*****************************************************************************
uint32_t am_widget_adxl363_deinit( am_devices_adxl363_t *amDevicesAdxl363 ) ;

//*****************************************************************************
//
//! @brief set the status of the int1 pin
//!
//! @return generic status
//
//*****************************************************************************
adxl_int_pin_status_e am_widget_adxl363_checkInt1Pin( void ) ;


#endif //AM_WIDGET2_IOM_H
