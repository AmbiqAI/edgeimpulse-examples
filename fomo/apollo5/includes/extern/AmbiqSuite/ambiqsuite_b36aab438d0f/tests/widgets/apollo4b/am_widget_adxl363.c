//*****************************************************************************
//
//! @file am_widget_adxl363.c
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
#include <string.h>
#include <ctype.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_widget_adxl363.h"



extern am_devices_adxl363_t am_dev_adxl363v ;

static void iom_adxl363_int1_handler_cb(void *pArg ) ;

static void iom_widget_adxl363_nonBlock_cb(void *pCallbackCtxt,
                                           uint32_t transactionStatus) ;

//*****************************************************************************
//
//! @brief callback when spi read has finished.
//! @note this is called from an ISR
//!
//! This function is called when the spi read completes
//!
//!
//! @return none
//
//*****************************************************************************
static void
iom_widget_adxl363_nonBlock_cb(void *pCallbackCtxt, uint32_t transactionStatus)
{
    am_devices_adxl363_t *amDevicesAdxl363 = (am_devices_adxl363_t *) pCallbackCtxt;
    // spi read has finished
    if (amDevicesAdxl363)
    {
        amDevicesAdxl363->nonBlockBusy = false;
        amDevicesAdxl363->adxlCallback = true;
    }
}

//*****************************************************************************
//
// reads data from accelerometer fifo
//
//*****************************************************************************
uint32_t
iom_widget_adxl363_readData(am_devices_adxl363_t *am_dev_adxl363,
                            uint32_t *ui32InBuff,
                            uint32_t maxIo,
                            bool useNonBlocking)
{
    am_hal_iom_callback_t callback = NULL ;

    if ( useNonBlocking)
    {
        callback = iom_widget_adxl363_nonBlock_cb;
        am_dev_adxl363->nonBlockBusy = true;
    }

    uint32_t ui32RetVal = am_devices_adxl363_sample_get((const am_devices_adxl363_t *) am_dev_adxl363,
                                                        maxIo,
                                                        ui32InBuff,
                                                        callback,
                                                        am_dev_adxl363);

    if ( ui32RetVal == AM_DEVICES_ADXL363_SUCCESS )
    {
        am_dev_adxl363->dataReady      = false ;
        am_dev_adxl363->readInProgress = true ;
    } else {
        am_dev_adxl363->nonBlockBusy     = false ;
    }

    return ui32RetVal ;

}

//*****************************************************************************
//
// reads and tests the device ID
//
//*****************************************************************************
uint32_t
iom_widiget_adxl363_checkDevId( am_devices_adxl363_t *am_dev_adxl363 )
{
    uint32_t ui32TempBuff[1];

    uint32_t  ui32RetVal = am_devices_adxl363_device_id_get((const am_devices_adxl363_t *)am_dev_adxl363, ui32TempBuff);
    if (ui32RetVal)
    {
        am_util_stdio_printf("ADXL363 error reading device id %d\n", ui32RetVal);
        return ui32RetVal;
    }
    uint8_t *pui8p = (uint8_t *) ui32TempBuff;
    if (pui8p[0] != ADXL363_DEV_ID0 || pui8p[1] != ADXL363_DEV_ID1 )
    {
        am_util_stdio_printf("ADXL363 invalid device id %04x\n", (ui32TempBuff[0] & 0xFFFF));
        return AM_DEVICES_ADXL363_ERROR;
    }

    am_util_stdio_printf("ADXL363 device id read success\n");

    return AM_DEVICES_ADXL363_SUCCESS ;

}

//*****************************************************************************
//
// int pin handling
//
//*****************************************************************************
uint32_t
iom_widget_adxl3563_io_config(am_devices_adxl363_t *amDevAdxl363)
{

    uint32_t  ui32RetVal = 0 ;
#ifdef USE_INTERRUPT
    //
    // use int1 pin as watermark interrupt
    // when pin is high, it is time to read the adxl363's fifo
    //
    iom_widget_adxl363_enable_fifo_int(true);
#else
    ui32RetVal = am_hal_gpio_pinconfig(AM_BSP_GPIO_ACCEL_INT1,  g_AM_BSP_GPIO_ACCEL_INT1_k);
#endif

#ifdef PULSE_INT2
    //
    // want to use adxl363 in synchro mode (where this micro controls the sampling)
    // setup int2 as a pwm output pin
    //
    ui32RetVal += am_hal_gpio_pinconfig(AM_BSP_GPIO_ACCEL_INT2_PWM, g_AM_BSP_GPIO_ACCEL_INT2_PWM);
#ifdef USE_PWM
    //
    // start the pwm output that will control the adxl363 sample rate
    //
    am_widget_adxl363_do_pwm(PWM_TIMER_CH,
                 AM_BSP_GPIO_ACCEL_INT2_PWM,
                 PWM_OUT_FREQ,
                 PWM_DUTY_CYCLE);
#endif // USE_PWM
#endif // PULSE_INT2

    if (ui32RetVal == 0)
    {
        //
        // start the accelerometer
        //
        ui32RetVal = am_devices_adxl363_measurement_mode_set(amDevAdxl363);
    }

    
    return ui32RetVal;

}


//*****************************************************************************
//
// Shutdown the ADXL363 test.
//
//*****************************************************************************
uint32_t
iom_widget_adxl363_stop(void )
{
    uint32_t ui32RetVal = 0;
#ifdef USE_INTERRUPT
    iom_widget_adxl363_enable_fifo_int(false);
#else
    ui32RetVal = am_hal_gpio_pinconfig(AM_BSP_GPIO_ACCEL_INT1,  g_AM_BSP_GPIO_ACCEL_INT1_k);
#endif

#ifdef PULSE_INT2
    //
    // use int2 to provide sample clock to the accelerometer
    //
#ifdef USE_PWM
    //
    // start the pwm output to control the adxl363 sample rate
    //
    am_widget_adxl363_do_pwm(PWM_TIMER_CH,
                             AM_BSP_GPIO_ACCEL_INT2_PWM,
                             0,
                             0);
#endif // USE_PWM
#endif // PULSE_INT2

    return ui32RetVal ;
}

//*****************************************************************************
//
// Enable int1 pin interrupt
//
//*****************************************************************************
void
iom_widget_adxl363_enable_fifo_int(bool enableInterrupt)
{
    uint32_t ui32PinNum = AM_BSP_GPIO_ACCEL_INT1;

    if (enableInterrupt)
    {
        am_hal_gpio_pinconfig(ui32PinNum, g_AM_BSP_GPIO_ACCEL_INT1);


        am_hal_gpio_interrupt_register(AM_HAL_GPIO_INT_CHANNEL_0,
                                       ui32PinNum,
                                       (am_hal_gpio_handler_t) iom_adxl363_int1_handler_cb,
                                       &am_dev_adxl363v);

        uint32_t ui32IntStatus = (1 << ui32PinNum);
        am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);

        // Enable the GPIO/button interrupt.
        am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                                      AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                                      &ui32PinNum);

        NVIC_SetPriority(GPIO0_001F_IRQn, AM_IRQ_PRIORITY_DEFAULT);
        NVIC_EnableIRQ(GPIO0_001F_IRQn);
    }
    else
    {
        NVIC_DisableIRQ(GPIO0_001F_IRQn);

        am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                                      AM_HAL_GPIO_INT_CTRL_INDV_DISABLE,
                                      &ui32PinNum);
    }
}

//*****************************************************************************
//
//! @brief gpio_isr
//!
//! @return none
//
//*****************************************************************************
void
am_gpio0_001f_isr(void)
{
    //
    // Read and clear the GPIO interrupt status.
    //
    uint32_t ui32IntStatus;
    am_hal_gpio_interrupt_irq_status_get(GPIO0_001F_IRQn, true, &ui32IntStatus);
    am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);
    am_hal_gpio_interrupt_service(GPIO0_001F_IRQn, ui32IntStatus);
}



//*****************************************************************************
//
//! @brief int1 isr callback handler (gpio)
//! @note this is called from an isr
//! @note this is called when the int1 accelerometer line goes high
//!
//! @return none
//
//*****************************************************************************
static void
iom_adxl363_int1_handler_cb(void *pArg )
{
    if( pArg )
    {
        am_devices_adxl363_t *amDevicesAdxl363 = (am_devices_adxl363_t *) pArg ;
        amDevicesAdxl363->dataReady = true ;
    }

}
//*****************************************************************************
//
//! @brief enables or disables PWM on with
//
//*****************************************************************************
uint32_t
am_widget_adxl363_do_pwm(uint32_t ui32TimerNumber,
                         uint32_t ui32PinNumber,
                         uint32_t ui32Freq_x100,
                         uint32_t ui32DCPercent )
{

    // @todo should have some invalid param and overflow checking

    if ( ui32Freq_x100 == 0 ){
        // turn it off
        return am_hal_timer_disable(ui32TimerNumber);
    }

    uint32_t ui32baseOutCfgAddr = (uint32_t) &(TIMER->OUTCFG0) ;


    // 4 pins per address need to increment by 4 since 32 bit variable
    ui32baseOutCfgAddr += ui32PinNumber & ~0x03 ;
    uint32_t    ui32PinOffset = (ui32PinNumber & 0x03) << 3 ;
    uint32_t    ui32Mask      = ~(0xFF << ui32PinOffset) ;

    //
    // always using output0 for right now
    //
    uint32_t    ui32TimerOffset = 0 ;

    //
    // just doing regular timers for right now
    //
    uint32_t    ui32PadConfig = (ui32TimerNumber << 1) + ui32TimerOffset ;

    uint32_t    *ui32Toff = (uint32_t *)  ui32baseOutCfgAddr ;

    *ui32Toff   = (*ui32Toff &  ui32Mask) | ( ui32PadConfig << ui32PinOffset) ;

    am_hal_timer_config_t  am_hal_tcfg ;
    am_hal_timer_default_config_set( &am_hal_tcfg ) ;
    am_hal_tcfg.eFunction    = AM_HAL_TIMER_FN_PWM;

    //
    // using default freq
    // @todo should look up actual base freq
    //
    // compute timer counter reset value in PWM mode
    //
    uint64_t ui64BaseFreq       = (96000000ull/16) * 100 ;     // this will still fit in 32 bits
    uint64_t ui64EndCounts      = ((ui64BaseFreq + ui32Freq_x100/2) / ui32Freq_x100) ;
    am_hal_tcfg.ui32Compare0    = (uint32_t) ui64EndCounts;


    //
    // this is the duty cycle computation for PWM mode
    //
    uint64_t ui64DcTemp         = ((uint64_t) ui64EndCounts * ui32DCPercent + 100/2) / 100 ;
    if ( ui64DcTemp > ui64EndCounts) ui64DcTemp = ui64EndCounts ;
    am_hal_tcfg.ui32Compare1    = (uint32_t) ui64DcTemp;

    uint32_t ui32stat           = am_hal_timer_enable(ui32TimerNumber);
    if (ui32stat == 0)
    {
        ui32stat = am_hal_timer_config(ui32TimerNumber, &am_hal_tcfg);

        if (ui32stat == 0)
        {
            ui32stat = am_hal_timer_start(ui32TimerNumber);
        }
    }

    return ui32stat;

}

//*****************************************************************************
//
// init adxl363 driver and iom settings
//
//*****************************************************************************
uint32_t
am_widget_adxl363_cmn_init( am_devices_adxl363_t *amDevicesAdxl363,
                            uint32_t ui32iomNumber,
                            uint32_t ui32chipSelect)
{

    //
    // setup sampling at 100hz and do a read (60 bytes) at 10hz
    //
    am_widget_adxl363_pinSetup(ui32iomNumber);

    amDevicesAdxl363->ui32IOMModule  = ui32iomNumber;
    amDevicesAdxl363->ui32ChipSelect = ui32chipSelect;
    amDevicesAdxl363->ui32Samples    = FIFO_SAMPLE_THRESH;
    amDevicesAdxl363->ui32SampleRate = AM_DEVICES_ADXL363_100HZ;
    amDevicesAdxl363->eGForceRange   = AM_DEVICES_ADXL363_2G;
    amDevicesAdxl363->bHalfBandwidth = false;
    amDevicesAdxl363->bSyncMode      = true;  // when true, micro needs to supply conversion clock on int2,  pwm < 400hz, 50%

    uint32_t ui32RetVal = am_devices_adxl363_init(amDevicesAdxl363);
    if (ui32RetVal != AM_DEVICES_ADXL363_SUCCESS)
    {
        am_util_stdio_printf("ADXL363 init failed %d\n", ui32RetVal);
        return ui32RetVal;
    }


    if ( 7 == ui32iomNumber )
    {
        IRQn_Type irq = ((IRQn_Type) (IOMSTR0_IRQn + ui32iomNumber));
        NVIC_ClearPendingIRQ(irq);
        NVIC_EnableIRQ(irq);
    }

    //
    // send the reset code to the device
    //
    ui32RetVal = am_devices_adxl363_reset(amDevicesAdxl363);
    if (ui32RetVal != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("ADXL363 reset failed %d\n", ui32RetVal);
        return ui32RetVal;
    }

    //
    // first, read device id
    //
    ui32RetVal = iom_widiget_adxl363_checkDevId(amDevicesAdxl363) ;
    if (ui32RetVal)
    {
        am_util_stdio_printf("ADXL363 device id read error %d\n", ui32RetVal);
        return ui32RetVal;
    }

    //
    // send configuration to device
    //
    ui32RetVal = am_devices_adxl363_config(amDevicesAdxl363);
    if (ui32RetVal != AM_DEVICES_ADXL363_SUCCESS)
    {
        am_util_stdio_printf("ADXL363 IOM config failed %d\n", ui32RetVal);
        return ui32RetVal;
    }

    return 0 ;
}

//*****************************************************************************
//
// Set up the IOM (SPI) pins
//
//*****************************************************************************
uint32_t
am_widget_adxl363_pinSetup(uint32_t ui32IomModuleNumber)
{
    uint32_t ui32RetVal = AM_DEVICES_ADXL363_SUCCESS ;
    am_bsp_iom_pins_enable(ui32IomModuleNumber, AM_HAL_IOM_SPI_MODE);

    return ui32RetVal ;
}

//*****************************************************************************
//
// deinit the iom
//
//*****************************************************************************
uint32_t
am_widget_adxl363_deinit( am_devices_adxl363_t *amDevicesAdxl363 )
{
    return am_hal_iom_uninitialize(amDevicesAdxl363->pHandle);
} // iom_adxl363_deinit


//*****************************************************************************
//
// check int pin
//
//*****************************************************************************
adxl_int_pin_status_e
am_widget_adxl363_checkInt1Pin( void )
{
    uint32_t ui32ReadState ;
    uint32_t ui32RetVal;

    ui32RetVal = am_hal_gpio_state_read(AM_BSP_GPIO_ACCEL_INT1,
                                        AM_HAL_GPIO_INPUT_READ,
                                        &ui32ReadState);
    if (ui32RetVal != 0)
    {
        //
        // read error
        //
        return eAdxlError ;
        //ui32ConsecErrCount++ ;
    }
    if (ui32ReadState)
    {
        //
        // the pin is high, the adxl363 fifo has data
        //
        return eAdxlIntStartRead;
    }

    //
    // the pin is high, the adxl363 fifo is not full past watermark
    //
    return eAdxlSleep ;
} // checkAdxlIntPin
