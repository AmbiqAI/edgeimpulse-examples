//*****************************************************************************
//
//! @file am_device_adxl363_test.h
//!
//! @brief Test code to interface with the ADXL363
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
#ifndef AM_DEVICE_ADXL363_TEST_H
#define AM_DEVICE_ADXL363_TEST_H


#ifdef __cplusplus
extern "C"
{
#endif






void setUp(void) ;
void tearDown(void) ;


//*****************************************************************************
//
//! @brief Start the ADXL363 test.
//!
//! This function calls driver functions that
//! set up the spi and sets the internal registers in the device
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
uint32_t iom_test_adxl363(void ) ;


//*****************************************************************************
//
//! @brief Run the ADXL363 test.
//!
//! This function calls driver functions that
//! set up the spi and accelrometer, and runs a burst mode fifo read test
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
uint32_t iom_adxl363_fifo_read_tst(uint32_t numBlocksToRead ) ;

//*****************************************************************************
//
//! @brief disable devices interrupts previouls enabled
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
uint32_t iom_adxl363_disable_accelerometer(void) ;



#ifdef __cplusplus
}
#endif


#endif //AM_DEVICE_ADXL363_TEST_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
