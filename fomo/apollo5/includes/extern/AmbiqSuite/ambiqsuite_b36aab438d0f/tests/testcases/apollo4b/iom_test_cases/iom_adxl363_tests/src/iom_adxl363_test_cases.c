//*****************************************************************************
//
//! @file am_device_adxl363_test.c
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
#include "am_bsp.h"
#include "am_util.h"
#include "am_devices_adxl363.h"
#include "iom_adxl363_test_cases.h"
#include "am_widget_adxl363.h"
#include <stdio.h>

#include "unity.h"

#define ACCEL_IOM_MODULE 7
#define IOM_INTERRUPT1(n)       AM_HAL_INTERRUPT_IOMASTER ## n
#define IOM_INTERRUPT(n)        IOM_INTERRUPT1(n)
#define ACCEL_IOM_IRQn          ((IRQn_Type)(IOMSTR0_IRQn + ACCEL_IOM_MODULE))

//
// define a struct that is used to hold debug variables
//
typedef struct {
   uint32_t ui32Isr7Count ;
   int32_t  i32SwoEnableStatus ;
} debug_t ;

//
// allocate debug variables
//
debug_t g_tdebug ;


//
// declare global variables used in this file
//
typedef struct {
    uint32_t            ui32ReadDataAcc[FIFO_SAMPLE_THRESH * 2 + 4] ;
    ADXL363Vector_t     NormalVects[ FIFO_SAMPLE_THRESH ] ;
    uint32_t            ui32StatusReg[20] ;
    uint32_t            ui32SleepCount ;
    bool                upOnce ;

}  
adxl_test_vars_t;

//
// global variables used in this file
//
adxl_test_vars_t gadxl_vars ;
am_devices_adxl363_t am_dev_adxl363v ;

//*****************************************************************************
//
// prototype for print utility used during test (when swo printing is normally off)
//
//*****************************************************************************
static void deepSleepPrint( bool deepSleepEnabled, char *printString ) ;

//
// set sleep mode
//
const static bool enable_deep_sleep = true ;



//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//
// global setup function
//
//*****************************************************************************
void
globalSetUp(void)
{
    setUp() ;

}
//*****************************************************************************
//
// global tear down function
//
//*****************************************************************************
void
globalTearDown(void)
{
    tearDown() ;
}


//*****************************************************************************
//
// set up function
//
//*****************************************************************************
void
setUp(void)
{
    if ( gadxl_vars.upOnce )
    {
        return ;
    }
    gadxl_vars.upOnce = true ;

    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Enable the ITM print interface.
    //
    am_bsp_itm_printf_enable();

    am_util_stdio_terminal_clear();
    am_util_stdio_printf("IOM ADXL363 Example\n");

    am_hal_interrupt_master_enable();
}

//*****************************************************************************
//
// tear down function
//
//*****************************************************************************

void
tearDown(void)
{
    if ( !gadxl_vars.upOnce )
    {
        return ;
    }
    gadxl_vars.upOnce = false ;

    //
    // turn off accelerometer loadswitch
    //
    am_hal_gpio_state_write( AM_BSP_GPIO_ACCEL_DEVICE_EN, AM_HAL_GPIO_OUTPUT_CLEAR) ;


    am_widget_adxl363_do_pwm( PWM_TIMER_CH,
                              AM_BSP_GPIO_ACCEL_INT2_PWM,
                  0,
                  0) ;


    iom_adxl363_disable_accelerometer() ;

}


//*****************************************************************************
//
// Start the ADXL363 test.
//
//*****************************************************************************
uint32_t
iom_test_adxl363(void)
{
    bool                          testPassed = true ;
    uint32_t    ui32IomModuleNumber = ACCEL_IOM_MODULE ;
    //
    // setup sampling at 100hz and do a read (60 bytes) at 10hz
    //
    am_widget_adxl363_pinSetup(ui32IomModuleNumber);

    am_dev_adxl363v.ui32IOMModule  = ui32IomModuleNumber;
    am_dev_adxl363v.ui32ChipSelect = 0;
    am_dev_adxl363v.ui32Samples    = FIFO_SAMPLE_THRESH;
    am_dev_adxl363v.ui32SampleRate = AM_DEVICES_ADXL363_100HZ;
    am_dev_adxl363v.eGForceRange   = AM_DEVICES_ADXL363_2G;
    am_dev_adxl363v.bHalfBandwidth = false;
    am_dev_adxl363v.bSyncMode      = true;  // when true, micro needs to supply conversion clock on int2,  pwm < 400hz, 50%


    //
    // power up loadswitch,
    //
    uint32_t ui32RetVal = am_hal_gpio_pinconfig(AM_BSP_GPIO_ACCEL_DEVICE_EN, g_AM_BSP_GPIO_ACCEL_DEVICE_EN );
    if ( ui32RetVal != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("ADXL363 loadswitch pin init failed %d\n", ui32RetVal);
        testPassed = false ;
        TEST_ASSERT_TRUE(testPassed) ;
        return ui32RetVal;
    }
    am_hal_gpio_state_write( AM_BSP_GPIO_ACCEL_DEVICE_EN, AM_HAL_GPIO_OUTPUT_SET) ;


    ui32RetVal = am_devices_adxl363_init(&am_dev_adxl363v);
    if (ui32RetVal != AM_DEVICES_ADXL363_SUCCESS)
    {
        am_util_stdio_printf("ADXL363 init failed %d\n", ui32RetVal);
        testPassed = false ;
        TEST_ASSERT_TRUE(testPassed) ;
        return ui32RetVal;
    }

    NVIC_ClearPendingIRQ(ACCEL_IOM_IRQn);
    NVIC_EnableIRQ(ACCEL_IOM_IRQn);

    //
    // send the reset code to the device
    //
    ui32RetVal = am_devices_adxl363_reset(&am_dev_adxl363v);
    if (ui32RetVal != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("ADXL363 reset failed %d\n", ui32RetVal);
        testPassed = false ;
        TEST_ASSERT_TRUE(testPassed) ;
        return ui32RetVal;
    }

    //
    // first, read device id
    //
    ui32RetVal = iom_widiget_adxl363_checkDevId( &am_dev_adxl363v ) ;
    if (ui32RetVal)
    {
        am_util_stdio_printf("ADXL363 device id read error %d\n", ui32RetVal);
        testPassed = false ;
        TEST_ASSERT_TRUE(testPassed) ;
        return ui32RetVal;
    }

    //
    // send configuration to device
    //
    ui32RetVal = am_devices_adxl363_config(&am_dev_adxl363v);
    if (ui32RetVal != AM_DEVICES_ADXL363_SUCCESS)
    {
        am_util_stdio_printf("ADXL363 IOM config failed %d\n", ui32RetVal);
        testPassed = false ;
        TEST_ASSERT_TRUE(testPassed) ;
        return ui32RetVal;
    }

    ui32RetVal = iom_adxl363_fifo_read_tst(32);

    if (ui32RetVal)
    {
        testPassed = false ;
    }
    TEST_ASSERT_TRUE(testPassed) ;
    return ui32RetVal;
}


//*****************************************************************************
//
// @brief Run the ADXL363 test.
//
//*****************************************************************************
uint32_t
iom_adxl363_fifo_read_tst(uint32_t numBlocksToRead )
{

    //
    // have int 1 set to alert (go high) when fifo hits threshold
    // watch that line and read 60 bytes whenever it goes high
    // this is pinGP8 on the micro
    //

    uint32_t  ui32RetVal = iom_widget_adxl3563_io_config(&am_dev_adxl363v);

    //
    // run the test
    //
    bool     keepLooping             = ui32RetVal == 0;
    uint32_t ui32ConsecErrCount      = 0;
    uint32_t ui32ReadCount           = 0;
    uint32_t ui32NumSamplesProcessed = 0;
    uint32_t ui32NumSamplesError     = 0;

	//
	// allocate some ram on the stack used for a print buffer
	//
    char i8OutBuff[256] ;

	//
	// if deepseeep is enabled, turn of the swo printing before starting the test
	//
    if ( enable_deep_sleep )
    {
       am_bsp_itm_printf_disable();
    }

    while (keepLooping && ui32ConsecErrCount < 5)
    {
        bool sleepReady = false;
        if (am_dev_adxl363v.nonBlockBusy == false)
        {
            //
            // data read is not (no longer) running
            //
            if (am_dev_adxl363v.readInProgress)
            {
                //
                // read was in progress, now not running, so:
                // data read just completed
                //
                am_dev_adxl363v.readInProgress = false;

                //
                // extract the data, and accumulate/count number of invalid samples
                //
                ui32NumSamplesError += am_devices_adxl363_extract_data((uint8_t *) gadxl_vars.ui32ReadDataAcc,
                                                                       FIFO_SAMPLE_THRESH * 2,
                                                                       gadxl_vars.NormalVects,
                                                                       am_dev_adxl363v.eGForceRange ) ;

                //
                // count total number of samples
                //
                ui32NumSamplesProcessed += FIFO_SAMPLE_THRESH;

                if (++ui32ReadCount >= numBlocksToRead)
                {
                    keepLooping = false;
                }
            }

            if (am_dev_adxl363v.dataReady)
            {
                //
                // data is ready in fifo, read it
                // use non blocking read
                //
                ui32RetVal = iom_widget_adxl363_readData(&am_dev_adxl363v,
                                                         gadxl_vars.ui32ReadDataAcc,
                                                         FIFO_SAMPLE_THRESH * 2,
                                                         true);
                if (ui32RetVal != AM_DEVICES_ADXL363_SUCCESS)
                {
                    ui32ConsecErrCount++;
                    sprintf(i8OutBuff, "ERROR: non block read %ld %ld\n", g_tdebug.ui32Isr7Count, ui32ConsecErrCount) ;
                    deepSleepPrint( enable_deep_sleep, i8OutBuff ) ;
                }
                else
                {
                    ui32ConsecErrCount = 0;
                }
            }
            else
            {
                //
                // read the data ready line
                // don't run this block if spi is being read (nonBlockBusy) (then this may be going low anyway)
                // or it is already known that data is ready to be read (dataReady)
                //
                uint32_t ui32ReadState;
                ui32RetVal = am_hal_gpio_state_read(AM_BSP_GPIO_ACCEL_INT1, AM_HAL_GPIO_INPUT_READ, &ui32ReadState);
                if (ui32RetVal)
                {
                    ui32ConsecErrCount++;
                    sprintf(i8OutBuff, "ERROR: state read %ld %ld\n", g_tdebug.ui32Isr7Count, ui32ConsecErrCount ) ;
                    deepSleepPrint( enable_deep_sleep, i8OutBuff ) ;
                }
                else
                {
                    ui32ConsecErrCount = 0;
                    if (ui32ReadState)
                    {
                        //
                        // this int1 line is high, there is data to read
                        //
                        am_dev_adxl363v.dataReady = true;
                    }
                    else
                    {
                        //
                        // nothing going on, so go to sleep
                        //
                        sleepReady = true;
                    }
                }
            }
        }
        else
        {
            //
            // nothing happening, go to sleep
            //
            sleepReady = true;
        }

        if (sleepReady)
        {
            am_hal_sysctrl_sleep(enable_deep_sleep);

            gadxl_vars.ui32SleepCount++;
        }
    } // while

    //
    // test over, get ready to print, enable swo printing
    //
    if ( enable_deep_sleep )
    {
        g_tdebug.i32SwoEnableStatus = am_bsp_itm_printf_enable();
    }


    if (ui32ConsecErrCount >= 5)
    {
        am_util_stdio_printf("ADXL363 IOM fifo read test fail%d\n", ui32RetVal);
        ui32RetVal = AM_DEVICES_ADXL363_ERROR ;
    }
    else
    {
        am_util_stdio_printf("ADXL363 IOM fifo read test success, num samples %ld, num failed %ld\n",
                             ui32NumSamplesProcessed, ui32NumSamplesError);

        //
        // if not enough samples were collected
        // or the number of bad samples exceeded 2%
        //
        if ( (ui32NumSamplesProcessed < 60) ||
            (ui32NumSamplesError > ui32NumSamplesProcessed/50) )
        {
            ui32RetVal = AM_DEVICES_ADXL363_ERROR ;
        }
    }


    return ui32RetVal;
}


//******************************************************************************
//
//! @brief will enable swo printing, if it is off, then print the string
//! @param deepSleepEnabled
//! @param printString
//
//******************************************************************************
static void deepSleepPrint( bool deepSleepEnabled, char *printString ){

    if ( deepSleepEnabled )
    {
        //
        // enable deep sleep
        //
        g_tdebug.i32SwoEnableStatus = am_bsp_itm_printf_enable() ;
    }

    am_util_stdio_printf( printString ) ;

    if ( deepSleepEnabled )
    {
        //
        // re-disable deep sleep
        //
        am_bsp_itm_printf_disable();
    }

}

//*****************************************************************************
//
// shutdown accelerometer and supporting devices
//
//*****************************************************************************
uint32_t
iom_adxl363_disable_accelerometer(void )
{
    uint32_t ui32RetVal = am_devices_adxl363_measurement_mode_standby(&am_dev_adxl363v);
    iom_widget_adxl363_enable_fifo_int(false) ;
    NVIC_DisableIRQ(ACCEL_IOM_IRQn);
    return ui32RetVal ;

}

//*****************************************************************************
//
//! @brief iom isr, called when non blocking transfer completes
//!
//! @return none
//
//*****************************************************************************
void
am_iomaster7_isr( void )
{
   
    uint32_t ui32Status;
    g_tdebug.ui32Isr7Count++ ;

    am_hal_iom_interrupt_status_get( am_dev_adxl363v.pHandle,
                                     true,
                                     &ui32Status) ;

    if (ui32Status)
    {
        am_hal_iom_interrupt_clear(am_dev_adxl363v.pHandle, ui32Status);
        am_hal_iom_interrupt_service(am_dev_adxl363v.pHandle, ui32Status);
    }

}


//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************


