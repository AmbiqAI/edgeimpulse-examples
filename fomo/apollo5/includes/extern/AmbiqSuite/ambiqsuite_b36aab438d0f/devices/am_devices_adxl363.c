//*****************************************************************************
//
//! @file am_devices_adxl363.c
//!
//! @brief Driver to interface with the ADXL363
//!
//! These functions implement the ADXL363 support routines for use on Ambiq
//! Micro MCUs.
//!
//! @addtogroup adxl363 ADXL363 SPI Driver
//! @ingroup devices
//! @{
//
//**************************************************************************

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
// #### INTERNAL BEGIN ####
#if defined(ADJ_CLK)
#include "iom_clock_config.h"
#endif
// #### INTERNAL END ####
//
//! globals used in adxl363 driver
//
typedef struct
{
    //
    //! pointer to IOM config structure for this device
    //
    am_hal_iom_spi_device_t    *g_psIOMSettings;
    //
    //! function pointer for write routine (spi send)
    //
    am_devices_adxl363_write_t g_pfnSpiWrite ;
    //
    //! function pointer for read routine
    //
    am_devices_adxl363_read_t  g_pfnSpiRead ;
    //
    //! iom command sequence buffer
    //
    uint32_t                   gui32NBTxnBuf[256] ;     // array overflow when this was 32
}
am_hal_adxl363_globals_t ;

static am_hal_adxl363_globals_t gadxl363 ;

//
//! enum defines commands for ADXL363
//
typedef enum
{
    eWRITE_REG      = 0x0A,
    eREAD_REG       = 0x0B,
    eREAD_FIFO      = 0x0D,
}
am_devices_adxl363_spi_cmds_e;

//
//! register definitions for adxl363
//
typedef enum
{
    eADXL363_REG_DEVID_AD,
    eADXL363_REG_DEVID_MST,
    eADXL363_REG_DEVID,
    eADXL363_REG_REVID,
    eADXL363_REG_XDATA      = 0x08,
    eADXL363_REG_YDATA,
    eADXL363_REG_ZDATA,
    eADXL363_REG_STATUS,
    eADXL363_REG_FIFO_ENTRIES_L,
    eADXL363_REG_FIFO_ENTRIES_H,
    eADXL363_REG_XDATA_L,
    eADXL363_REG_XDATA_H,
    eADXL363_REG_YDATA_L,
    eADXL363_REG_YDATA_H,
    eADXL363_REG_ZDATA_L,
    eADXL363_REG_ZDATA_H,
    eADXL363_REG_TEMP_L, // 0x14
    eADXL363_REG_TEMP_H,
    eADXL363_REG_ADC_DATA_L,
    eADXL363_REG_ADC_DATA_H,  // 0x17
    eADXL363_REG_SOFT_RESET = 0x1F,
    eADXL363_REG_THRESH_ACT_L,
    eADXL363_REG_THRESH_ACT_H,
    eADXL363_REG_TIME_ACT,
    eADXL363_REG_THRESH_INACT_L,
    eADXL363_REG_THRESH_INACT_H,
    eADXL363_REG_TIME_INACT_L,
    eADXL363_REG_TIME_INACT_H,
    eADXL363_REG_ACT_INACT_CTL,
    eADXL363_REG_FIFO_CTL,
    eADXL363_REG_FIFO_SAMPLES,
    eADXL363_REG_INTMAP1,
    eADXL363_REG_INTMAP2,
    eADXL363_REG_FILTER_CTL,
    eADXL363_REG_POWER_CTL,
    eADXL363_REG_SELF_TEST,

}
am_hal_adxl363_regdefs_e;

//
//! iom module setup struct
//! declare variables
//
static const am_hal_iom_config_t g_amHalIomConfig =
{
    .eInterfaceMode     = AM_HAL_IOM_SPI_MODE,
    .ui32ClockFreq      = 8000000ul,
    .eSpiMode           = AM_HAL_IOM_SPI_MODE_0,
    .pNBTxnBuf          = gadxl363.gui32NBTxnBuf,
    .ui32NBTxnBufLength = sizeof(gadxl363.gui32NBTxnBuf) / 4,
};

//*****************************************************************************
//
//! @brief decode a 16bit fifo sample input from the ADXL363
//!
//! @param pui16AxisArray  pointer to incoming data
//! @param ui16FifoSample  number of 16 bit samples to process
//! @param pui32AxisCount  processed output data
//! @return
//
//*****************************************************************************
static uint32_t am_devices_adxl363_extract_axis(int16_t *pui16AxisArray,
                                                uint16_t ui16FifoSample,
                                                uint32_t *pui32AxisCount ) ;

//*****************************************************************************
//
// Initialize the ADXL363 driver
//
//*****************************************************************************
uint32_t
am_devices_adxl363_init(am_devices_adxl363_t *psDevice)
{

    //
    // enable the accel enable line, default is ok, not needed
    //
    //am_bsp_external_accel_device_enable( true ) ;

    //am_util_delay_ms(50);
    uint32_t  ui32iom_module ;
    uint32_t ui32RetVal ;

    ui32iom_module = psDevice->ui32IOMModule ;

    ui32RetVal = am_hal_iom_initialize(ui32iom_module, &(psDevice->pHandle) ) ;
    if ( ui32RetVal != AM_HAL_STATUS_SUCCESS)
    {
        return AM_DEVICES_ADXL363_ERROR;
    }
    ui32RetVal = am_hal_iom_power_ctrl( psDevice->pHandle,
                                        AM_HAL_SYSCTRL_WAKE,
                                        false ) ;
    if ( ui32RetVal != AM_HAL_STATUS_SUCCESS)
    {
        return AM_DEVICES_ADXL363_ERROR;
    }

    ui32RetVal = am_hal_iom_configure( psDevice->pHandle, &g_amHalIomConfig ) ;
    if ( ui32RetVal != AM_HAL_STATUS_SUCCESS)
    {
        return AM_DEVICES_ADXL363_ERROR;
    }

    ui32RetVal = am_hal_iom_enable( psDevice->pHandle ) ;

// #### INTERNAL BEGIN ####
#if defined(ADJ_CLK)
    ui32RetVal += am_hal_iom_set_intial_clk( ui32iom_module, g_amHalIomConfig.ui32ClockFreq ) ;
#endif
// #### INTERNAL END ####
    return ui32RetVal ? AM_DEVICES_ADXL363_ERROR : AM_DEVICES_ADXL363_SUCCESS ;
}

//*****************************************************************************
//
// Initialize the ADXL363.
//
//*****************************************************************************
uint32_t
am_devices_adxl363_config(am_devices_adxl363_t *psDevice)
{
    uint32_t ui32status ;

    am_hal_iom_byte_buffer(16) sCommand;

    //
    // zero 16 bytes
    //
    sCommand.llwords[0] = 0 ;
    sCommand.llwords[1] = 0 ;

    //
    // Setup default register state command string to send to ADXL363.
    // want one sample every 10 msec, read 10 samples (60 bytes) every 100 msec
    //
    sCommand.bytes[14] = 0x10;  // reg #2C FILTER_CTL , half bandwidth, odr is 100hz, 2G
    sCommand.bytes[13] = 0x02;  // reg #2B INTMAP2    Interrupt on FIFO ready
    sCommand.bytes[12] = 0x04;  // reg #2A INTMAP1    Interrupt on water mark
    sCommand.bytes[11] = 30 ;   // reg #29 FIFO_SAMPLES ==10 * 3  (60 bytes)NOTE: AH = 0,
    sCommand.bytes[10] = 0x02 ; // reg #28 FIFO_CTRL   AH=0  FIFO MODE = stream mode
    sCommand.bytes[9]  = 0x05;  // reg #27 ACT_INACT_CTL
    sCommand.bytes[8]  = 0x00;  // reg #26 TIME_INACT_H
    sCommand.bytes[7]  = 0x00;  // reg #25 TIME_INACT_L
    sCommand.bytes[6]  = 0x00;  // reg #24 THRESHOLD_INACT_H
    sCommand.bytes[5]  = 0x00;  // reg #23 THRESHOLD_INACT_L
    sCommand.bytes[4]  = 0x00;  // reg #22 TIME_ACT
    sCommand.bytes[3]  = 0x00;  // reg #21 THRESHOLD_ACT_H
    sCommand.bytes[2]  = 0x00;  // reg #20 THRESHOLD_ACT_L
    sCommand.bytes[1]  = eADXL363_REG_THRESH_ACT_L;  // register address of THRESHOLD_ACT_L
    sCommand.bytes[0]  = eWRITE_REG;  // SPI WRITE command for the ADXL363

    //
    // Handle optional RANGE setting
    //
    switch (psDevice->eGForceRange)
    {
        case AM_DEVICES_ADXL363_4G:
            sCommand.bytes[14] |= 0x40;  // reg +/- 4g
        break;

        case AM_DEVICES_ADXL363_8G:
            sCommand.bytes[14] |= 0x80;  // reg +/- 8g
        break;

        default:
        case AM_DEVICES_ADXL363_2G:
        break;

    }

    //
    // Handle optional sample rate selections
    //
    switch (psDevice->ui32SampleRate)
    {
        case AM_DEVICES_ADXL363_400HZ:
            sCommand.bytes[14] |= 0x05; //  400Hz
        break;

        case AM_DEVICES_ADXL363_100HZ:
            sCommand.bytes[14] |= 0x03; //  100Hz
        break;

        default:
        case AM_DEVICES_ADXL363_200HZ:
            sCommand.bytes[14] |= 0x04; //  200Hz
        break;
    }

    //
    // Handle optional half bandwidth reduction mode
    //
    if (psDevice->bHalfBandwidth)
    {
        sCommand.bytes[14] &= ~0x10;
    }

    //
    // Handle optional synchronous sampling mode. (where sample clock is in int2)
    //
    if (psDevice->bSyncMode)
    {
        sCommand.bytes[14] |= 0x08;  // reg #2C FILTER_CTL   synchronous sample mode
        sCommand.bytes[13] = 0x00;  // reg #2B INTMAP2    Not available in sync mode
    }

    //
    // Override the high water mark with the one supplied.
    //
    sCommand.bytes[11]  =  psDevice->ui32Samples & 0x0000000ff;
    if (psDevice->ui32Samples > 0xFF)
    {
        sCommand.bytes[10] |= 0x08 ;
    }
    am_hal_iom_transfer_t amHalIomTransfer ;

    amHalIomTransfer.uPeerInfo.ui32SpiChipSelect = psDevice->ui32ChipSelect  ;
    amHalIomTransfer.ui32InstrLen = 0 ;
    amHalIomTransfer.ui64Instr = 0 ;
    amHalIomTransfer.ui32NumBytes = 15 ;
    amHalIomTransfer.eDirection   = AM_HAL_IOM_TX ;
    amHalIomTransfer.pui32TxBuffer = sCommand.words ;
    amHalIomTransfer.bContinue     = false ;
    amHalIomTransfer.ui8Priority = 0 ;

// #### INTERNAL BEGIN ####
#if defined(ADJ_CLK)
    ui32status = am_hal_iom_configure_clk_tst(psDevice->pHandle, &g_amHalIomConfig);
    if ( ui32status )
    {
        return ui32status;
    }
#endif
// #### INTERNAL END ####
    //
    // send the config
    // Use polled IOM send routine to load the command registers.
    //
    ui32status = am_hal_iom_blocking_transfer(psDevice->pHandle, &amHalIomTransfer);

    return ui32status ? AM_DEVICES_ADXL363_ERROR : AM_DEVICES_ADXL363_SUCCESS ;
}

//*****************************************************************************
//
// Reset the ADXL363.
//
//*****************************************************************************
uint32_t
am_devices_adxl363_reset(const am_devices_adxl363_t *psDevice)
{
    uint32_t ui32RetVal ;
    am_hal_iom_byte_buffer (4) sCommand;

    //
    // Use polled IOM send routine to reset the ADXL363.
    //
    sCommand.llwords[0] = 0 ;
    sCommand.bytes[0]   = 'R' ;

    ui32RetVal = am_devices_adxl363_reg_write(psDevice,
                                        sCommand.bytes,
                                        eADXL363_REG_SOFT_RESET,
                                        1) ;

    am_util_delay_ms(50);

    return ui32RetVal ? AM_DEVICES_ADXL363_ERROR : AM_DEVICES_ADXL363_SUCCESS ;
}
//*****************************************************************************
//
// Set measurement mode for the ADXL363.
//
//*****************************************************************************
uint32_t
am_devices_adxl363_measurement_mode_set(const am_devices_adxl363_t *psDevice)
{

    am_hal_iom_byte_buffer(4) sCommand;

    sCommand.llwords[0] = 0;
    sCommand.bytes[0]   = 0x02; // turn on adxl363 device

    return am_devices_adxl363_reg_write(psDevice,
                                        sCommand.bytes,
                                        eADXL363_REG_POWER_CTL,
                                        1);

}

//*****************************************************************************
//
// this function will place the ADXL363 in standby mode
//
//*****************************************************************************
uint32_t
am_devices_adxl363_measurement_mode_standby(const am_devices_adxl363_t *psDevice)
{

    am_hal_iom_byte_buffer(4) sCommand;

    sCommand.llwords[0] = 0;

    return am_devices_adxl363_reg_write(psDevice,
                                        sCommand.bytes,
                                        eADXL363_REG_POWER_CTL,
                                        1);

}

//*****************************************************************************
//
// read adc registers from ADXL363
//
//*****************************************************************************
uint32_t
am_devices_adxl363_read_adc(const am_devices_adxl363_t *psDevice,
                            uint32_t *pui32Return)
{

    return am_devices_adxl363_ctrl_reg_read(psDevice,
                                            (uint8_t *) pui32Return,
                                            eADXL363_REG_ADC_DATA_L,
                                            2);

}

//*****************************************************************************
//
// Get FIFO depth for the ADXL363.
//
//*****************************************************************************
uint32_t
am_devices_adxl363_fifo_depth_get(const am_devices_adxl363_t *psDevice,
                                  uint32_t *pui32Return)
{

    return am_devices_adxl363_ctrl_reg_read( psDevice,
                                             (uint8_t *) pui32Return,
                                             eADXL363_REG_FIFO_ENTRIES_L,
                                             2) ;

}

//*****************************************************************************
//
// Get the state of the control registers from the ADXL363.
//
//*****************************************************************************
uint32_t
am_devices_adxl363_ctrl_reg_state_get(const am_devices_adxl363_t *psDevice,
                                      uint32_t *pui32Return)
{

    return am_devices_adxl363_ctrl_reg_read(psDevice,
                                            (uint8_t *) pui32Return,
                                            eADXL363_REG_THRESH_ACT_L,
                                            2);

}

//*****************************************************************************
//
// Get the device ID.
//
//*****************************************************************************
uint32_t
am_devices_adxl363_device_id_get(const am_devices_adxl363_t *psDevice,
                                 uint32_t *pui32Return)
{

    return am_devices_adxl363_ctrl_reg_read(psDevice,
                                            (uint8_t *) pui32Return,
                                            eADXL363_REG_DEVID_AD,
                                            DEVICE_ID_SIZE);
}

//*****************************************************************************
//
// Write to ADXL62 control registers
//
//*****************************************************************************
uint32_t
am_devices_adxl363_reg_write(const am_devices_adxl363_t *psDevice,
                             uint8_t *pui8RegValues, uint8_t ui8StartReg,
                             uint32_t ui32Number)
{
    uint32_t ui32status ;
    am_hal_iom_byte_buffer(4) sCommand;

    //
    // We can't accept a register write of more than 32 bytes.
    //
    if (ui32Number > 30 || ui32Number < 1)
    {
        //return AM_HAL_STATUS_INVALID_ARG;
        return AM_DEVICES_ADXL363_ERROR ;
    }

    sCommand.llwords[0] = 0;
    // these have been swapped
    sCommand.bytes[1]   = eWRITE_REG; // SPI WRITE command for the ADXL363
    sCommand.bytes[0]   = ui8StartReg; // register address of THRESHOLD_ACT_L

    am_hal_iom_transfer_t amHalIomTransfer;

    amHalIomTransfer.uPeerInfo.ui32SpiChipSelect = psDevice->ui32ChipSelect;
    amHalIomTransfer.ui32InstrLen                = 2;
    amHalIomTransfer.ui64Instr                   = sCommand.llwords[0];
    amHalIomTransfer.ui32NumBytes                = ui32Number;
    amHalIomTransfer.eDirection                  = AM_HAL_IOM_TX;
    amHalIomTransfer.pui32TxBuffer               = (uint32_t *) pui8RegValues;
    amHalIomTransfer.pui32RxBuffer               = NULL;
    amHalIomTransfer.bContinue                   = false;
    amHalIomTransfer.ui8Priority                 = 0;

// #### INTERNAL BEGIN ####
#if defined(ADJ_CLK)
    ui32status = am_hal_iom_configure_clk_tst (psDevice->pHandle, &g_amHalIomConfig);
    if ( ui32status )
    {
        return ui32status;
    }
#endif
// #### INTERNAL END ####
    ui32status = am_hal_iom_blocking_transfer(psDevice->pHandle, &amHalIomTransfer);

    return ui32status ? AM_DEVICES_ADXL363_ERROR : AM_DEVICES_ADXL363_SUCCESS ;

}

//*****************************************************************************
//
// Read the state of one or more ADXL363 control registers
//
//*****************************************************************************
uint32_t
am_devices_adxl363_ctrl_reg_read(const am_devices_adxl363_t *psDevice,
                                 uint8_t *pui8Return, uint8_t ui8StartReg,
                                 uint32_t ui32Number)
{
    uint32_t ui32status ;
    am_hal_iom_byte_buffer(2) sCommand;

    //
    // Use polled IOM send routine.
    //
    sCommand.llwords[0] = 0;

    // these have been swapped
    sCommand.bytes[0] = ui8StartReg;
    sCommand.bytes[1] = eREAD_REG;

    am_hal_iom_transfer_t amHalIomTransfer;

    amHalIomTransfer.uPeerInfo.ui32SpiChipSelect = psDevice->ui32ChipSelect;
    amHalIomTransfer.ui32InstrLen                = 2;
    amHalIomTransfer.ui64Instr                   = sCommand.llwords[0];
    amHalIomTransfer.ui32NumBytes                = ui32Number;
    amHalIomTransfer.eDirection                  = AM_HAL_IOM_RX;
    amHalIomTransfer.pui32TxBuffer               = NULL;
    amHalIomTransfer.pui32RxBuffer               = (uint32_t *) pui8Return;
    amHalIomTransfer.bContinue                   = false; //
    amHalIomTransfer.ui8Priority                 = 0;

// #### INTERNAL BEGIN ####
    //
    // ensure spi sclk speed is correct for this device
    //
#if defined(ADJ_CLK)
    ui32status = am_hal_iom_configure_clk_tst (psDevice->pHandle, &g_amHalIomConfig );
    if ( ui32status )
    {
        return ui32status;
    }
#endif
// #### INTERNAL END ####
    ui32status = am_hal_iom_blocking_transfer(psDevice->pHandle, &amHalIomTransfer);

    return ui32status ? AM_DEVICES_ADXL363_ERROR : AM_DEVICES_ADXL363_SUCCESS ;
}

//*****************************************************************************
//
// Get a sample from the ADXL363
//
//*****************************************************************************
uint32_t
am_devices_adxl363_sample_get(const am_devices_adxl363_t *psDevice,
                              uint32_t ui32NumSamples,
                              uint32_t *pui32ReturnBuffer,
                              am_hal_iom_callback_t pfnCallback,
                              void *callbackCtxt)
{
    uint32_t ui32status ;
    //
    // Start a non-blocking read from the ADXL. Pass the caller's callback
    // straight through to the IOM HAL.
    //
    am_hal_iom_byte_buffer(4) sCommand;
    sCommand.llwords[0] = 0;

    sCommand.bytes[0] = eREAD_FIFO;

    am_hal_iom_transfer_t amHalIomTransfer;

    amHalIomTransfer.uPeerInfo.ui32SpiChipSelect = psDevice->ui32ChipSelect;
    amHalIomTransfer.ui32InstrLen                = 1;
    amHalIomTransfer.ui64Instr                   = sCommand.llwords[0];
    amHalIomTransfer.ui32NumBytes                = ui32NumSamples;
    amHalIomTransfer.eDirection                  = AM_HAL_IOM_RX;
    amHalIomTransfer.pui32TxBuffer               = NULL;
    amHalIomTransfer.pui32RxBuffer               = pui32ReturnBuffer;
    amHalIomTransfer.bContinue                   = false; //
    amHalIomTransfer.ui8Priority                 = 0;
    amHalIomTransfer.ui32StatusSetClr            = 0;
    amHalIomTransfer.ui32PauseCondition          = 0;

// #### INTERNAL BEGIN ####
    //
    // ensure spi sclk speed is correct for this device
    //
#if defined(ADJ_CLK)
    ui32status = am_hal_iom_configure_clk_tst( psDevice->pHandle, &g_amHalIomConfig );
    if ( ui32status )
    {
        return ui32status;
    }
#endif
// #### INTERNAL END ####
    if (pfnCallback)
    {
        ui32status = am_hal_iom_nonblocking_transfer( psDevice->pHandle,
                                                     &amHalIomTransfer,
                                                     pfnCallback,
                                                     callbackCtxt );

    }
    else
    {
        //
        // the caller didn't provide a callback, so do a blocking read
        //
        ui32status = am_hal_iom_blocking_transfer( psDevice->pHandle, &amHalIomTransfer );
        //ui32RetVal = am_devices_adxl363_nonblocking_read(psDevice->pHandle, &amHalIomTransfer);
    }

    return ui32status ? AM_DEVICES_ADXL363_ERROR : AM_DEVICES_ADXL363_SUCCESS ;
}

//*****************************************************************************
//
// decode a 16bit fifo sample input from the ADXL363
//
//*****************************************************************************
static uint32_t
am_devices_adxl363_extract_axis(
    int16_t *pi16AxisArray,
    uint16_t ui16FifoSample,
    uint32_t *pui32AxisCount )
{
    //
    // top two bits of each 16 bit sample
    // hold x = 0, y = 1 z = 2, temperature = 3
    // extract top two bits
    //
    uint16_t ui16Top2Bits = ui16FifoSample >> 14;

    //
    // get rid of top two bits, this is the data
    // it could be negative so needs sign extending
    //
    uint16_t ui16Rem = ui16FifoSample & 0x3FFF;

    //
    // sign extend, if necessary
    //
    if (ui16Rem & 0x2000)
    {
        //
        // is negative make this an official 16-bit negative number,
        // restore top two bits
        //
        ui16Rem |= 0xC000;

    }
    uint32_t ui32Index   = 0;
    uint32_t ui32AddMask = 1;

    //
    // look at the high two bits and extract and index and action mask
    //
    switch (ui16Top2Bits)
    {
        case 0:
            break;
        case 1:
            ui32Index = 1;
            ui32AddMask = 1 << 8;
            break;
        case 2:
            ui32Index = 2;
            ui32AddMask = 1 << 16;
            break;
        default:
            //
            // index is zero here
            // not doing temperature data, so this is an error
            //
            ui32AddMask = 1 << 24;
            break ;
    } // switch

    //
    // save the sign extended value in the correct index location
    //
    pi16AxisArray[ui32Index] = (int16_t) ui16Rem;

    //
    // the ui16FifoSample of this should be 0x0001001001 when all three value processed
    // if one is missed or counted twice, it will show up here
    //
    *pui32AxisCount += ui32AddMask;

    return AM_DEVICES_ADXL363_SUCCESS ;
}

//*****************************************************************************
//
// decode an array of 16bit fifo sample inputs from the ADXL363
//
//*****************************************************************************
uint32_t groupCount = 0 ;

//
// scaling
// compute g * 1000
// with 2g need to divide by 1024
// with 4g need to divide by 512
// with 8g need to divide by 256
//
const uint32_t g_gforceShiftTable[3] = {
    10, // div by 1024 AM_DEVICES_ADXL363_2G
    9,  // div by 512  AM_DEVICES_ADXL363_4G
    8,  // div by 256  AM_DEVICES_ADXL363_8G
};

uint32_t
am_devices_adxl363_extract_data(uint8_t *pui8InputBuff,
                                uint32_t ui32InputBytes,
                                ADXL363Vector_t *outNrmlVectors,
                                eADXL363Range eGforceRange)
{
    //
    // count number of times this function was called
    //
    groupCount++ ;

    uint32_t ui32MaxPossiblevectors = ui32InputBytes / 6;
    uint16_t *pui16Ivect    = (uint16_t *) pui8InputBuff;
    uint32_t ui32ErrorCount = 0 ;

    if ( eGforceRange > AM_DEVICES_ADXL363_8G)
    {
        return AM_DEVICES_ADXL363_ERROR ;
    }
    uint32_t  ui32shift     = g_gforceShiftTable[eGforceRange] ;

    for ( uint32_t i = 0; i < ui32MaxPossiblevectors; i++ )
    {
        //
        // extract xyz params from data
        // this come from the device in groups of 3: x,y,z data in that order
        //
        uint32_t ui32AxisCount = 0;
        int16_t  i16Axis[4];

        am_devices_adxl363_extract_axis(i16Axis, *pui16Ivect++, &ui32AxisCount);
        am_devices_adxl363_extract_axis(i16Axis, *pui16Ivect++, &ui32AxisCount);
        am_devices_adxl363_extract_axis(i16Axis, *pui16Ivect++, &ui32AxisCount);

        //
        // each axis should appear once and only once
        // each byte contains number of times an axis was decoded
        // temperature, z, y, x  (not using temperature)
        //
        bool error = ui32AxisCount != 0x00010101;

        //
        // look for invalid values
        // if one magnitude is out of whack, it will show up here
        //
        if (!error)
        {
            uint32_t xx = (uint32_t) ((int32_t) i16Axis[0] * (int32_t) i16Axis[0]);
            uint32_t yy = (uint32_t) ((int32_t) i16Axis[1] * (int32_t) i16Axis[1]);
            uint32_t zz = (uint32_t) ((int32_t) i16Axis[2] * (int32_t) i16Axis[2]);
            error = ((xx > (2048 * 2048)) || (yy > (2048 * 2048)) || (zz > (2048 * 2048)));

        }

        //
        // save the sample data status
        //
        outNrmlVectors->ui32ErrorCode = ui32AxisCount;

        if (error)
        {
            am_util_stdio_printf( "adxl error %ld %ld %08lx\n:", groupCount, i, ui32AxisCount ) ;
            ui32ErrorCount++;
            outNrmlVectors->i32vectorsGx1000[0] = -1;
            outNrmlVectors->i32vectorsGx1000[1] = -1;
            outNrmlVectors->i32vectorsGx1000[2] = -1;
        }
        else
        {
            // output is g * 1000
            // with 2g need to divide by 1024 to get to 1 g
            // with 4g need to divide by 512
            // with 8g need to divide by 256
            // no rounding on LSbit

            outNrmlVectors->i32vectorsGx1000[0] = ((int32_t) i16Axis[0] * 1000l) >> ui32shift;
            outNrmlVectors->i32vectorsGx1000[1] = ((int32_t) i16Axis[1] * 1000l) >> ui32shift;
            outNrmlVectors->i32vectorsGx1000[2] = ((int32_t) i16Axis[2] * 1000l) >> ui32shift;

        }

        outNrmlVectors++;

    }

    //
    // return error count so calling application can count failed samples
    //
    return ui32ErrorCount ;

}
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

