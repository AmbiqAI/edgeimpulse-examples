//*****************************************************************************
//
//! @file am_devices_adxl364.c
//!
//! @brief Driver to interface with the ADXL364
//!
//! These functions implement the ADXL364 support routines for use on Ambiq
//! Micro MCUs.
//!
//! @addtogroup adxl364 ADXL364 SPI Driver
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
#include "am_devices_adxl364.h"
#include "am_util_delay.h"

//*****************************************************************************
//
// Globals
//
//*****************************************************************************
//! pointer to settings struct
am_hal_iom_spi_device_t *g_psIOMSettings;

//! zero the write function pointer
static am_devices_adxl364_write_t g_pfnSpiWrite = 0;
//! zero the read function pointer
static am_devices_adxl364_read_t g_pfnSpiRead = 0;

//*****************************************************************************
//
//  Initialize the ADXL364 driver.
//
//*****************************************************************************
void
am_devices_adxl364_driver_init(am_hal_iom_spi_device_t *psIOMSettings,
                               am_devices_adxl364_write_t pfnWriteFunc,
                               am_devices_adxl364_read_t pfnReadFunc)
{
    //
    // Initialize the IOM settings for the ADXL364.
    //
    g_psIOMSettings = psIOMSettings;

    g_pfnSpiWrite = pfnWriteFunc ? pfnWriteFunc :
        (am_devices_adxl364_write_t) am_hal_iom_spi_write;

    g_pfnSpiRead = pfnReadFunc ? pfnReadFunc :
        (am_devices_adxl364_read_t) am_hal_iom_spi_read;
}

//*****************************************************************************
//
//  Initialize the ADXL364.
//
//*****************************************************************************
int
am_devices_adxl364_init(bool bSyncMode, uint32_t ui32ClockFreqHz)
{

    uint32_t pui32Command[8];
    uint8_t *pui8Command;

    pui8Command = (uint8_t *) pui32Command;
    //
    // Use polled IOM send routine to reset the ADXL364.
    //
    pui8Command[2] = 0x52; // R for reset is a required parameter
    pui8Command[1] = 0x2D; // register SOFT_RESET on ADXL364
    pui8Command[0] = 0x0A; // SPI WRITE
    g_pfnSpiWrite(g_psIOMSettings->ui32Module, g_psIOMSettings->ui32ChipSelect,
                  pui32Command, 3, AM_HAL_IOM_RAW);

    //
    // Add some delay.
    //
    //am_util_delay_ms(100,ui32ClockFreqHz);
    am_util_delay_cycles(24000*100);

    //
    // Setup command string to send to adxl364
    //
    pui8Command = (uint8_t *) pui32Command;
    pui8Command[20] = 0x00;     // reg #2B STANDBY
    pui8Command[19] = 0x01;     // reg #2A PWR_CTL
    pui8Command[18] = 0x00;     // reg #29 EXT_ADC
    pui8Command[17] = 0x25;     // reg #28 FILTER_CTL   400Hz sample rate
    pui8Command[16] = 0x00;     // reg #27 INT_LOW
    pui8Command[15] = 0x02;     // reg #26 INTMAP2    Interrupt on FIFO ready
    pui8Command[14] = 0x04;     // reg #25 INTMAP1    Interrupt on water mark
    pui8Command[13] = 0xD2;     // reg #24 FIFO_SAMPLES == 210 (0x0D2) NOTE: AH = 0
    pui8Command[12] = 0x02;     // reg #23 FIFO_CTRL   AH=0  FIFO MODE = stream mode
    pui8Command[11] = 0x00;     // reg #22 TIMER_CTL
    pui8Command[10] = 0x00;     // reg #21 ACT_INACT_CTL
    pui8Command[9]  = 0x00;     // reg #20 ACT_MASK
    pui8Command[8]  = 0x02;     // reg #1F TIME_INACT_H
    pui8Command[7]  = 0x00;     // reg #1E TIME_INACT_L
    pui8Command[6]  = 0x00;     // reg #1D THRESHOLD_INACT_H
    pui8Command[5]  = 0x80;     // reg #1C THRESHOLD_INACT_L
    pui8Command[4]  = 0x20;     // reg #1B TIME_ACT
    pui8Command[3]  = 0x02;     // reg #1A THRESHOLD_ACT_H
    pui8Command[2]  = 0x00;     // reg #19 THRESHOLD_ACT_L
    pui8Command[1]  = 0x19;     // register address of THRESHOLD_ACT_L
    pui8Command[0]  = 0x0A;     // SPI WRITE command for the ADXL364

    //
    // Handle optional synchronous sampling mode
    //
    if (bSyncMode)
    {
        pui8Command[11] = 0x40; // reg #22 TIMER_CTL   synchronous sample mode
        pui8Command[15] = 0x00; // reg #26 INTMAP2    Not available in sync mode
    }

    //
    // Use polled IOM send routine to load the command registers.
    //
    g_pfnSpiWrite(g_psIOMSettings->ui32Module, g_psIOMSettings->ui32ChipSelect,
                  pui32Command, 21, AM_HAL_IOM_RAW);

    return 0;
}

//*****************************************************************************
//
//  Reset the ADXL364
//
// Sends a reset command to the ADXL364 over SPI.
//
//*****************************************************************************
void
am_devices_adxl364_reset(void)
{
    int i;
    uint32_t pui32Command[1];
    uint8_t *pui8Command;

    pui8Command = (uint8_t *) pui32Command;

    // use polled IOM send routine to reset the ADXL364
    pui8Command[2] = 0x52; // R for reset is a required parameter
    pui8Command[1] = 0x2D; // register SOFT_RESET on ADXL364
    pui8Command[0] = 0x0A; // SPI WRITE
    g_pfnSpiWrite(g_psIOMSettings->ui32Module, g_psIOMSettings->ui32ChipSelect,
                  pui32Command, 3, AM_HAL_IOM_RAW);

    // add some delay
    for (i = 0; i < 10000; i++)
    {
        *((volatile uint32_t  *) 0x04ffff080 ) = i;
    }
}

//*****************************************************************************
//
//  Set measurement mode for the ADXL364
//
//*****************************************************************************
int
am_devices_adxl364_measurement_mode_set(void)
{
#ifdef NOTDEF
// #### INTERNAL BEGIN ####
    //fixme 364 does not need to set measurement mode separately
// #### INTERNAL END ####
    uint32_t pui32Command[1];
    uint8_t *pui8Command;

    pui8Command = (uint8_t *) pui32Command;

    // use polled IOM send routine
    pui8Command[2] = 0x01; // reg #2D POWER_CTL  set measurement mode
    pui8Command[1] = 0x2A; // register address of POWER_CTL
    pui8Command[0] = 0x0A; // SPI WRITE command for the ADXL364

    g_pfnSpiWrite(g_psIOMSettings->ui32Module, g_psIOMSettings->ui32ChipSelect,
                  pui32Command, 3, AM_HAL_IOM_RAW);
#endif

    return 0;
}

//*****************************************************************************
//
//  Get FIFO depth for the ADXL364
//
//*****************************************************************************
int
am_devices_adxl364_fifo_depth_get(uint32_t * p)
{
    uint32_t pui32Command[1];
    uint8_t *pui8Command;

    pui8Command = (uint8_t *) pui32Command;

    // use polled IOM send routine
    pui8Command[1] = 0x0A; // register FIFO ENTRIES LOW
    pui8Command[0] = 0x0B; // READ

    g_pfnSpiWrite(g_psIOMSettings->ui32Module, g_psIOMSettings->ui32ChipSelect,
                  pui32Command, 2, AM_HAL_IOM_CS_LOW | AM_HAL_IOM_RAW);

    g_pfnSpiRead(g_psIOMSettings->ui32Module, g_psIOMSettings->ui32ChipSelect,
                 p, 2, AM_HAL_IOM_RAW);

    *p &= 0x0000ffff;

    return 0;
}

//*****************************************************************************
//
//  Get a sample from the ADXL364
//
//*****************************************************************************
int
am_devices_adxl364_sample_get(int Number, uint32_t *p)
{
    uint32_t pui32Command[1];
    uint8_t *pui8Command;

    pui8Command = (uint8_t *) pui32Command;

    //
    // use polled IOM send routine
    //
    pui8Command[0] = 0x0D; // READ 2 BYTES FROM THE FIFO

    g_pfnSpiWrite(g_psIOMSettings->ui32Module, g_psIOMSettings->ui32ChipSelect,
                  pui32Command, 1, AM_HAL_IOM_CS_LOW | AM_HAL_IOM_RAW);

    g_pfnSpiRead(g_psIOMSettings->ui32Module, g_psIOMSettings->ui32ChipSelect,
                 p, Number << 1, AM_HAL_IOM_RAW);

    return 0;
}

//*****************************************************************************
//
//  Triggers an IOM command to read a set of samples from the ADXL
//
//*****************************************************************************
int
am_devices_adxl364_sample_get_nonblocking(int Number)
{
    //
    // Initiate a 'READ' command to get the ADXL data.
    //
    am_hal_iom_spi_cmd_run(AM_HAL_IOM_READ, g_psIOMSettings->ui32Module,
                           g_psIOMSettings->ui32ChipSelect, Number << 1,
                           AM_HAL_IOM_OFFSET(0x0D));
    return 0;
}

//*****************************************************************************
//
//  Get the state of the control registers from  the ADXL364
//
//*****************************************************************************
int
am_devices_adxl364_ctrl_reg_state_get(uint32_t * p)
{
    uint32_t pui32Command[1];
    uint8_t *pui8Command;

    pui8Command = (uint8_t *) pui32Command;

    // use polled IOM send routine
    pui8Command[1] = 0x19;
    pui8Command[0] = 0x0B;

    g_pfnSpiWrite(g_psIOMSettings->ui32Module, g_psIOMSettings->ui32ChipSelect,
                  pui32Command, 2, AM_HAL_IOM_CS_LOW | AM_HAL_IOM_RAW);

    g_pfnSpiRead(g_psIOMSettings->ui32Module, g_psIOMSettings->ui32ChipSelect,
                 p, 21, AM_HAL_IOM_RAW);

    return 0;
}

//*****************************************************************************
//
//  Get the device ID
//
//*****************************************************************************
void
am_devices_adxl364_device_id_get(uint32_t * p)
{
    uint32_t pui32Command[1];
    uint8_t *pui8Command;

    pui8Command = (uint8_t *) pui32Command;

    // use polled IOM send routine
    pui8Command[1] = 0x00; // begining of Device ID
    pui8Command[0] = 0x0B;

    g_pfnSpiWrite(g_psIOMSettings->ui32Module, g_psIOMSettings->ui32ChipSelect,
                  pui32Command, 2, AM_HAL_IOM_CS_LOW | AM_HAL_IOM_RAW);

    g_pfnSpiRead(g_psIOMSettings->ui32Module, g_psIOMSettings->ui32ChipSelect,
                 p, 4, AM_HAL_IOM_RAW);
}
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

