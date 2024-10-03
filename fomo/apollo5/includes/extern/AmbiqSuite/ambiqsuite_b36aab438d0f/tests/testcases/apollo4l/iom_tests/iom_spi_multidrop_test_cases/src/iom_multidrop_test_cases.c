//*****************************************************************************
//
//! @file iom_psram.c
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

#define ADXL363_MAIN
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "iom_multidrop_test_cases.h"
#include "iom_clock_config.h"
#include "am_devices_spipsram.h"
#include "am_widget_adxl363.h"
#include "am_widget_iom_mulitdrop.h"
#include "unity.h"

//*****************************************************************************
// Customize the following for the test
//*****************************************************************************
// Control the example execution

//*****************************************************************************
//*****************************************************************************

#define NUM_PSRAM_BYTES_20MSEC 80

#if defined APS6404L
#define PSRAM_DEVICE_ID          AM_DEVICES_SPIPSRAM_KGD_PASS
//#define am_iom_test_devices_t    am_devices_spipsram_config_t
#else
#error "Unknown PSRAM Device"
#endif

#define IOM_INTERRUPT1(n)       AM_HAL_INTERRUPT_IOMASTER ## n
#define IOM_INTERRUPT(n)        IOM_INTERRUPT1(n)
#define PSRAM_IOM_IRQn          ((IRQn_Type)(IOMSTR0_IRQn + PSRAM_IOM_MODULE))
#define IOM_TEST_FREQ           AM_HAL_IOM_16MHZ

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
#define ADXL_FIFO_SAMPLE_THRESH 30

//
//! globals used in this module (file)
//
typedef struct {
    //
    //! data read from adxl363
    //
    uint32_t        ui32ReadDataAcc[ADXL_FIFO_SAMPLE_THRESH * 2 + 4] ;
    //
    //! processed accelerometer data
    //
    ADXL363Vector_t outNrmlVectors[ADXL_FIFO_SAMPLE_THRESH] ;  // storage space for processed xyz vectors


    //
    //! keep from setting up or tearing down more than once
    //
    bool isSetup ;

}
iom_multidrop_test_vars_t;

//
//! define operating state for the state machine
//
typedef enum
{
    //
    //!
    //
    eOpCheckInt1,
    //
    //! read accelerometer
    //
    eOpDoAccelRead,
    //
    //! wait for accelerometer read to complete
    //
    eOpWaitAccelRead,
    //
    //! do PSRAM read
    //
    eOpDoPSramRead,
    //
    //! wait for PSRAM read to complete
    //
    eOpWaitPsramRead,
    //
    //! do PSRAM write
    //
    eOPDoPsRamWrite,
    //
    //! wait for PSRAM write to complete
    //
    eOpWaitPsramWrite,
}
psramOpStates_e ;

//
//! allocate globals for accelerometer driver
//
am_devices_adxl363_t      am_dev_adxl363v ;

//
//! allocate globals used in this file
//
iom_multidrop_test_vars_t g_adxl_vars ;

//
//! allocate globals for PSRAM driver
//
iom_psram_vars_t g_psram_test_vars ;

//
//! grab globals used in mulitdrop widget
//! this gives visibility into the ctimer isr
//
extern am_widget_iom_multdrop_vars g_WidgetMultiDropv ;

//*****************************************************************************
//
// Static Function Prototypes
//
//*****************************************************************************
static void timer_init(void) ;
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
static uint32_t iom_adxl363_init(void);

//
//! define the function pointers for the psram test
//
const psram_device_func_t device_funcPointers =
{
#if defined APS6404L
  // Fireball installed SPI PSRAM device
    .devName        = "SPI PSRAM APS6404L",
    .fpDeviceInit          = am_devices_spipsram_init,
    .fpDeviceTerminate     = am_devices_spipsram_term,
    .fpReadDevId           = am_devices_spipsram_read_id,
    .fpNonBlockingWrite    = am_devices_spipsram_nonblocking_write,
    .fpNonBlockingRead     = am_devices_spipsram_nonblocking_read,
    .fpBlockingWrite       = am_devices_spipsram_blocking_write,
    .fpBlockingRead        = am_devices_spipsram_blocking_read,
#else
#error "Unknown PSRAM Device"
#endif
};


void setUp(void) ;
void tearDown(void) ;




//*****************************************************************************
//
//! setup
//
//*****************************************************************************
void
setUp(void)
{
  
}
//*****************************************************************************
//
//! teardown
//
//*****************************************************************************
void
tearDown(void)
{
  
}
//*****************************************************************************
//
//! global setup
//
//*****************************************************************************
void
globalSetUp(void)
{
    if ( g_adxl_vars.isSetup)
    {
        return ;
    }
    g_adxl_vars.isSetup = true ;

    g_psram_test_vars.ui32BuffSize          = PSRAM_PATTERN_BUF_SIZE;
    g_psram_test_vars.pdeviceFP             = &device_funcPointers;
    g_psram_test_vars.ui32SpiClockFreq      = IOM_TEST_FREQ;
    g_psram_test_vars.ui32IOMNum            = PSRAM_IOM_MODULE;
    g_psram_test_vars.ui32ChipSelectNum     = 0 ;
    g_psram_test_vars.pCallbackFunction     = NULL;
    g_psram_test_vars.ui32BufferSizeBytes   = DMATCBBufferBUFF_SIZE;


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

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("IOM PSRAM Example\n");

    //
    // TimerA0 init.
    //
    timer_init();

    //
    // Enable the timer Interrupt.
    //
    am_hal_timer_interrupt_enable(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE1));

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_SetPriority(TIMER_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(TIMER_IRQn);
    am_hal_interrupt_master_enable();

    //
    // Enable the iom interrupt in the NVIC.
    //
    NVIC_ClearPendingIRQ(PSRAM_IOM_IRQn);
    NVIC_EnableIRQ(PSRAM_IOM_IRQn);


#ifdef ENABLE_PSRAM
    am_wdg_iom_multidrop_init_pattern(g_psram_test_vars.PatternBuf.bytes, PSRAM_PATTERN_BUF_SIZE);
#endif

    //
    // Initialize the ADXL363
    //
    uint32_t ui32Status = iom_adxl363_init() ;
    if ( ui32Status){
        am_util_stdio_printf("Unable to initialize adxl363\n");
        TEST_ASSERT_TRUE(false) ;
        while(1);
    }

    am_util_stdio_printf("adxl363 OK\n");


    //
    // Initialize PSRAM
    //
    ui32Status = am_wdg_iom_multidrop_psram_init(&g_psram_test_vars,
                                                 am_dev_adxl363v.pHandle);
    if (ui32Status)
    {
        am_util_stdio_printf("Unable to initialize PSRAM\n");
        TEST_ASSERT_TRUE(false) ;
        while(1);
    }
    am_util_stdio_printf("PSRAM OK\n");

    //
    // start up sampling on the adxl363
    //
    ui32Status = iom_widget_adxl3563_io_config(&am_dev_adxl363v);
    if ( ui32Status )
    {
        am_util_stdio_printf("failed to start adxl363\n");
        return ;
    }

    //
    // Start timer 0, 1 second interrupt
    //
    am_hal_timer_start(0);


}
//*****************************************************************************
//
//! teardown
//
//*****************************************************************************
void
globalTearDown(void)
{
    if ( !g_adxl_vars.isSetup )
    {
        return ;

    }
    g_adxl_vars.isSetup = false ;
    // Cleanup
    am_util_stdio_printf("\nEnd of PSRAM Example\n");

    //
    // Disable the timer Interrupt.
    //
    am_hal_timer_interrupt_disable(0);

    //
    // disable the interrupts in the NVIC.
    //
    NVIC_DisableIRQ(TIMER_IRQn);
    NVIC_DisableIRQ(PSRAM_IOM_IRQn);
    am_wdg_iom_multidrop_psram_term(&g_psram_test_vars);

    iom_widget_adxl363_stop() ;

    //
    // turn off the loadswitch
    //
    am_hal_gpio_state_write( AM_BSP_GPIO_ACCEL_DEVICE_EN, AM_HAL_GPIO_OUTPUT_CLEAR) ;


    am_widget_adxl363_deinit(&am_dev_adxl363v) ;
}

//*****************************************************************************
//
//! @brief Take over default IOM channel ISR.
//!
//! @note Initialize Timer0 to interrupt every 20msec
//
//*****************************************************************************
#define TIMER_TICS_SEC 50  // 20 millisecond timer
static void
timer_init(void)
{
    am_hal_timer_config_t       TimerConfig;

    //
    // Set up TIMER0
    //
    am_hal_timer_default_config_set(&TimerConfig);
    TimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    TimerConfig.ui32PatternLimit = 0;
    // number of clock ticks in one second isHFRC_CLOCK/16)
    TimerConfig.ui32Compare1 = (HFRC_CLOCK/16)/TIMER_TICS_SEC;  // Default Clock is HFRC/16 or 6MHz.  50 hz, 20 msec
    
    if (AM_HAL_STATUS_SUCCESS != am_hal_timer_config(0, &TimerConfig))
    {
      am_util_stdio_printf("Failed to configure TIMER0\n");
    }
    am_hal_timer_clear(0);

    //
    // Clear the timer Interrupt
    //
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE1));

} // timer_init()


//*****************************************************************************
//
// manages the psram/adxl363 non blocking test
//
//*****************************************************************************
void
iom_test_100msec_accel_psram(void )
{
  
    bool   testPassed = true ;

    //
    // Loop forever.
    //
    psramOpStates_e eOpState = eOpCheckInt1;

    uint32_t ui32RetVal;
    uint32_t ui32TimerTicks = 0 ;
    uint32_t ui32Timout = 0 ;
    uint32_t ui32NumRead = 0 ;
    uint32_t ui32ErrCount = 0 ;
    bool     sleep_ready = false;
    bool     timer1sec;
    bool keep_testing = true ;

    uint32_t  ui32AdxlFailedSamples = 0 ;
    uint32_t  ui32AdxlTotalSamples  = 0 ;

    //
    // run state machine
    //

    while(keep_testing && ui32ErrCount < 10)
    {
        while ( g_WidgetMultiDropv.ctimer_tick)
        {
            if ( ui32Timout ) ui32Timout-- ;
            g_WidgetMultiDropv.ctimer_tick-- ;
            if (++ui32TimerTicks >= TIMER_TICS_SEC )
            {
                ui32TimerTicks = 0 ;
                timer1sec      = true ;
            }
        }

        switch (eOpState)
        {
            default:
            case eOpCheckInt1:
                //
                // this is more or less the idle state
                //
                if (timer1sec || am_dev_adxl363v.dataReady)
                {
                    timer1sec = false ;
                    adxl_int_pin_status_e rd_pin_state = am_widget_adxl363_checkInt1Pin() ;
                    if( rd_pin_state == eAdxlIntStartRead )
                    {
                        eOpState = eOpDoAccelRead ;
                    }
                    else if( rd_pin_state == eAdxlSleep )
                    {
                        sleep_ready = true ;
                    }
                }

                break ;
            case eOpDoAccelRead:
                //
                // accelerometer fifo level is past watermark, read the fifo via spi non blocking
                //
                am_dev_adxl363v.dataReady = false ;

                ui32RetVal = iom_widget_adxl363_readData( &am_dev_adxl363v,
                                                          g_adxl_vars.ui32ReadDataAcc,
                                                          ADXL_FIFO_SAMPLE_THRESH * 2,
                                                          true);
                if (ui32RetVal)
                {
                    am_util_stdio_printf("error starting adxl read %d\n", ui32RetVal);
                    eOpState = eOpCheckInt1 ;
                    ui32ErrCount++ ;
                }
                else
                {
                    eOpState   = eOpWaitAccelRead;
                    ui32Timout = 200 / TIMER_TICS_SEC ;
                    sleep_ready = true ;
                }

                break ;
            case eOpWaitAccelRead:
                if ( ui32Timout == 0 )
                {
                    //
                    // something went wrong with accelerometer read
                    //
                    am_util_stdio_printf("error: accel wait read timeout\n" );
                    eOpState = eOpCheckInt1 ;
                    ui32ErrCount++ ;
                    break ;
                }

                if ( am_dev_adxl363v.adxlCallback )
                {
                    am_dev_adxl363v.adxlCallback = false;
                    //
                    // analyze returned accel. data
                    //
                    ui32AdxlFailedSamples += am_devices_adxl363_extract_data((uint8_t *) g_adxl_vars.ui32ReadDataAcc,
                                                                             ADXL_FIFO_SAMPLE_THRESH * 2,
                                                                             g_adxl_vars.outNrmlVectors,
                                                                             am_dev_adxl363v.eGForceRange);
                    ui32AdxlTotalSamples += ADXL_FIFO_SAMPLE_THRESH;

                    //
                    // immediately start the PSRAM read
                    //
                    eOpState = eOpDoPSramRead;
                }
                else
                {
                    sleep_ready = true ;
                }
                break ;
            case eOpDoPSramRead:
                //
                // start the nonblocking psram read
                //
                ui32RetVal = am_wdg_iom_multidrop_psram_read(&g_psram_test_vars, 
                                                             g_psram_test_vars.RxBuf.bytes, 
                                                             g_psram_test_vars.ui32BuffSize, 
                                                             true) ;
                if (ui32RetVal)
                {
                    am_util_stdio_printf("error starting PSRAM read %d\n", ui32RetVal);
                    eOpState   = eOpCheckInt1 ;
                }
                else
                {
                    eOpState   = eOpWaitPsramRead;
                    ui32Timout = 200 / TIMER_TICS_SEC ;
                }
                sleep_ready = true ;
                break ;
            case eOpWaitPsramRead:
                if ( ui32Timout == 0 )
                {
                    am_util_stdio_printf("error: psram wait read timeout\n" );
                    eOpState = eOpCheckInt1 ;
                    ui32ErrCount++ ;
                    break ;
                }
                if ( !g_psram_test_vars.readPsRamInProgress ){
                    //
                    // psram read ended
                    //
                    if ( g_psram_test_vars.bReadWriteComplete )
                    {
                        //
                        // test returned data (compare to reference)
                        //
                        g_psram_test_vars.bReadWriteComplete = false ;
                        int32_t i32VerifyStatus = am_wdg_iom_multidrop_verify_data(
                                                                    g_psram_test_vars.PatternBuf.bytes,
                                                                    g_psram_test_vars.RxBuf.bytes,
                                                                    NUM_PSRAM_BYTES_20MSEC ) ;
                        if ( i32VerifyStatus ){
                            testPassed = false ;
                            am_util_stdio_printf("\n%d Reads done\n", ui32NumRead);
                            keep_testing = false ;
                        }
                        else if ( ++ui32NumRead >= NUM_ITERATIONS)
                        {
                            am_util_stdio_printf("\n%d Reads done\n", NUM_ITERATIONS);
                            keep_testing = false ;
                        }
                    }
                    else
                    {
                        //
                        // this should not happen
                        //
                        am_util_stdio_printf( "read psram synch error\n" ) ;
                        ui32ErrCount++ ;
                    }
                    eOpState = eOpCheckInt1 ;
                }
                else
                {
                    sleep_ready = true ;
                }
                break ;

        } // switch

        if ( sleep_ready )
        {
            sleep_ready = false ;
            am_hal_sysctrl_sleep(true);
            g_psram_test_vars.ui32SleepCount++ ;
        }

    } // while

    if ( ui32ErrCount < 10 ){
        am_util_stdio_printf( "accel psram 100msec test passed, total num adxl363 samples %d, num failed samples %d\n",
                              ui32AdxlTotalSamples, ui32AdxlFailedSamples) ;
        
        if ( (ui32AdxlTotalSamples < 60) ||
            (ui32AdxlFailedSamples > ui32AdxlTotalSamples/4) )
        {
            ui32RetVal = AM_DEVICES_ADXL363_ERROR ;
            testPassed = false ;
        }
    }
    else
    {
        testPassed = false ;
      
        am_util_stdio_printf("accel psram 100msec test fail %d\n",ui32ErrCount );
    }
    TEST_ASSERT_TRUE(testPassed) ;
} // accel_psram_100msec_test

//*****************************************************************************
//
// manages the psram/adxl363 high speed blocking test
//
//*****************************************************************************
void
iom_test_psram_codec_20msec(void )
{

    //
    // write and read simulated codec every 10msec
    // read adxl when it si ready just after codec read
    // use blocking transfers
    //
    bool   testPassed = true ;
    
    am_util_stdio_printf("Periodically Reading data from PSRAM using non-blocking read - %d times\n", NUM_ITERATIONS);

    //
    // init the state for the state machine below
    //
    psramOpStates_e OpState         = eOpCheckInt1;
    uint32_t ui32RetVal;
    uint32_t ui32ErrCount           = 0 ;
    uint32_t ui32NumGoodPsramRW     = 0 ;
    bool     sleep_ready            = false;
    bool     psramGO                = false ;
    uint32_t ui32AdxlFailedSamples  = 0 ;
    uint32_t ui32AdxlTotalSamples   = 0 ;
    bool     keep_testing           = true ;

    // should disable interrupts

    g_WidgetMultiDropv.ctimer_tickCount = 0 ;

    while(keep_testing  && ui32ErrCount < 10 )
    {

        //
        // monitor the 20 msec timer
        // not part of the state machine
        //
        if ( g_WidgetMultiDropv.ctimer_tick)
        {
            g_WidgetMultiDropv.ctimer_tick = 0  ;
            //
            // 20 millisecond timer interrupt occurred, start the psram write
            //
            psramGO = true ;

            //
            // stop test after five seconds
            //
            if ( g_WidgetMultiDropv.ctimer_tickCount > (5 * TIMER_TICS_SEC))
            {
                keep_testing = false ;
                //
                // exit loop now
                //
                break ;
            }
        }

        switch (OpState)
        {
            default:
            case eOpCheckInt1:
                //
                // this is more or less the idle state
                //
                if ( psramGO )
                {
                    //
                    // every 20 msec, simulate codec: write and read 80 bytes from PSRAM
                    //
                    psramGO = false ;
                    OpState = eOPDoPsRamWrite;
                }
                else
                {
                    sleep_ready = true ;
                }

                break ;
            case eOPDoPsRamWrite:
                //
                // read/write/verify psram data here
                //
                am_wdg_iom_multidrop_psram_write(&g_psram_test_vars,
                            g_psram_test_vars.PatternBuf.bytes,
                            NUM_PSRAM_BYTES_20MSEC,
                            false) ;   // blocking write
                am_wdg_iom_multidrop_psram_read(&g_psram_test_vars,
                           g_psram_test_vars.RxBuf.bytes,
                           NUM_PSRAM_BYTES_20MSEC,
                           false) ;   // blocking read
                {
                    int32_t i32VerifyStatus = am_wdg_iom_multidrop_verify_data
                                                (g_psram_test_vars.PatternBuf.bytes,
                                                g_psram_test_vars.RxBuf.bytes,
                                                NUM_PSRAM_BYTES_20MSEC ) ;
                    if ( i32VerifyStatus ){
                        //
                        // set count to exit loop
                        //
                        ui32ErrCount = 100 ;
                        am_util_stdio_printf("error psram verify read %d\n", ui32NumGoodPsramRW);
                    }
                    else
                    {
                        if ( ++ui32NumGoodPsramRW > NUM_ITERATIONS ) {
                          keep_testing = false ;
                        }
                    }
                }
                {
                    //
                    // check if the accelerometer needs reading
                    // about every 100 milliseconds the accelerometer fifo needs to be read
                    //
                    adxl_int_pin_status_e eAdxlStat = am_widget_adxl363_checkInt1Pin();
                    if (eAdxlStat == eAdxlIntStartRead)
                    {
                        OpState = eOpDoAccelRead;
                    }
                    else
                    {
                        //
                        // return to idle state
                        //
                        OpState = eOpCheckInt1;
                    }
                }
                break ;

            case eOpDoAccelRead:
                am_dev_adxl363v.dataReady = false ;
                //
                // non-blocking read from accelerometer fifo
                //
                ui32RetVal = iom_widget_adxl363_readData(&am_dev_adxl363v,
                                                         g_adxl_vars.ui32ReadDataAcc,
                                                         ADXL_FIFO_SAMPLE_THRESH * 2,
                                                         false);
                if (ui32RetVal)
                {
                    am_util_stdio_printf("error starting adxl read %d\n", ui32RetVal);
                    ui32ErrCount++ ;
                }
                else
                {
                    //
                    // analyze returned accel. data
                    //
                    ui32AdxlFailedSamples += am_devices_adxl363_extract_data( (uint8_t *)g_adxl_vars.ui32ReadDataAcc,
                                                                              ADXL_FIFO_SAMPLE_THRESH * 2,
                                                                              g_adxl_vars.outNrmlVectors,
                                                                              am_dev_adxl363v.eGForceRange) ;
                    ui32AdxlTotalSamples += ADXL_FIFO_SAMPLE_THRESH ;
                }
                OpState = eOpCheckInt1 ;

                break ;

        } // switch


        if ( sleep_ready )
        {
            sleep_ready = false ;
            am_hal_sysctrl_sleep(true);
            g_psram_test_vars.ui32SleepCount++ ;
        }

    } // while

    if ( ui32ErrCount < 10 ){
        am_util_stdio_printf("iom_test_psram_codec_20msec successful, total adxl363 stamples %d, num failed samples %d\n",
                             ui32AdxlTotalSamples, ui32AdxlFailedSamples);
        if ( (ui32AdxlTotalSamples < 60) ||
            (ui32AdxlFailedSamples > ui32AdxlTotalSamples/4) )
        {
            ui32RetVal = AM_DEVICES_ADXL363_ERROR ;
            testPassed = false ;
        }
    }
    else
    {
        am_util_stdio_printf("iom_test_psram_codec_20msec fail%d\n", ui32ErrCount );
        testPassed = false ;
    }
    TEST_ASSERT_TRUE(testPassed) ;
} // iom_test_psram_codec_20msec

//*****************************************************************************
//
// Start the ADXL363 test.
//
//*****************************************************************************
static uint32_t
iom_adxl363_init(void )
{
    uint32_t ui32RetVal ;
    // setup sampling at 100hz and do a read (60 bytes) at 10hz
    uint32_t  ui32ChipSelect = ACCEL_IOM_MODULE == 7 ? 0 : 1 ;

    //
    // have to do additional pin setup for accel on IOM4 CS1
    //
    //
    // power up loadswitch, this is in PIN62, needs to be high
    //
    ui32RetVal = am_hal_gpio_pinconfig(AM_BSP_GPIO_ACCEL_DEVICE_EN, g_AM_BSP_GPIO_ACCEL_DEVICE_EN );
    if (ui32RetVal)
    {
        am_util_stdio_printf("adxl loadswitch pin config error %ld\n", ui32RetVal);
        return ui32RetVal ;
    }
    am_hal_gpio_state_write( AM_BSP_GPIO_ACCEL_DEVICE_EN, AM_HAL_GPIO_OUTPUT_SET) ;

    ui32RetVal = am_hal_gpio_pinconfig(AM_BSP_GPIO_ACCEL_CS,
                                       g_AM_BSP_GPIO_ACCEL_CS4_1);
    if (ui32RetVal)
    {
        am_util_stdio_printf("adxl CS1 pin config error %ld\n", ui32RetVal);
    }
    else
    {

        //
        // this will get spi pins setup for the correct IOM num
        // this will only setup the cs1 pin
        //
        ui32RetVal = am_widget_adxl363_cmn_init(&am_dev_adxl363v,
                                                ACCEL_IOM_MODULE,
                                                ui32ChipSelect);
    }



    return ui32RetVal;
} //iom_adxl363_init

#define psram_iom_isr                                                          \
    am_iom_isr1(PSRAM_IOM_MODULE)
#define am_iom_isr1(n)                                                        \
    am_iom_isr(n)
#define am_iom_isr(n)                                                         \
    am_iomaster ## n ## _isr

//*****************************************************************************
//
// IOM ISRs.
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Take over default IOM channel ISR.
//!
//
//*****************************************************************************
void psram_iom_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_psram_test_vars.pIOMHandle, true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_service(g_psram_test_vars.pIOMHandle, ui32Status);
        }
    }
    am_hal_iom_interrupt_clear(g_psram_test_vars.pIOMHandle, ui32Status);
}

