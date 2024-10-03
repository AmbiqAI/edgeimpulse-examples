//*****************************************************************************
//
//! @file iom_psram_accel_test.h
//!
//! @brief Example that demostrates IOM, connecting to a SPI PSRAM
//! PSRAM is initialized with a known pattern data using Blocking IOM Write.
//! This example starts a 1 second timer. At each 1 second period, it initiates
//! reading a fixed size block from the PSRAM device using Non-Blocking IOM
//! Read, and comparing againts the predefined pattern
//!
//! Define one of PSRAM_DEVICE_ macros to select the PSRAM device
//!
//! SWO is configured in 1M baud, 8-n-1 mode.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef IOM_PSRAM_ACCEL_TEST_H
#define IOM_PSRAM_ACCEL_TEST_H

#define ACCEL_IOM_MODULE        4
#define PSRAM_IOM_MODULE        4

#define ENABLE_PSRAM

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

//*****************************************************************************
//
// ! @brief manages the psram/adxl363 high speed blocking test
//
//*****************************************************************************

void iom_test_psram_codec_20msec(void ) ;
//*****************************************************************************
//
//! @brief manages the psram/adxl363 non blocking test
//
//*****************************************************************************
void iom_test_100msec_accel_psram(void ) ;

#ifdef __cplusplus
}
#endif

#endif //SUITEWORKING_IOM_PSRAM_ACCEL_TEST_H
