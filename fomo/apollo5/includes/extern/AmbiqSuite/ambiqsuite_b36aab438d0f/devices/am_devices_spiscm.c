//*****************************************************************************
//
//! @file am_devices_spiscm.c
//!
//! @brief Master-side functions for working with the SPISC
//!
//! Master-side functions for communicating to devices using the SPI "switched
//! configuration" protocol.
//!
//! @addtogroup spiscm SPISCM - SPI Switched Configuration Master Protocol
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

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_devices_spiscm.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define SIGNAL_GPIO                         31

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
am_hal_iom_spi_device_t *g_psIOSInterface;
static am_devices_spiscm_write_t g_pfnSpiWrite = 0;
static am_devices_spiscm_read_t g_pfnSpiRead = 0;

//*****************************************************************************
//
// Initialize the SPISC Master Driver
//
//*****************************************************************************
void
am_devices_spiscm_init(am_hal_iom_spi_device_t *psSlaveInterface,
                       am_devices_spiscm_write_t pfnWriteFunc,
                       am_devices_spiscm_read_t pfnReadFunc)
{
    g_psIOSInterface = psSlaveInterface;

    g_pfnSpiWrite = pfnWriteFunc ? pfnWriteFunc :
        (am_devices_spiscm_write_t) am_hal_iom_spi_write;

    g_pfnSpiRead = pfnReadFunc ? pfnReadFunc :
        (am_devices_spiscm_read_t) am_hal_iom_spi_read;
}

//*****************************************************************************
//
// Function for sending packets from the IO Master to the IO Slave.
//
//*****************************************************************************
void
am_devices_spiscm_write(uint32_t *pui32Data, uint32_t ui32NumBytes)
{
    am_hal_iom_buffer(2) sPacketHeader;
    uint32_t ui32Status;

    while ( ui32NumBytes )
    {
        //
        // Check the remaining number of bytes.
        //
        if ( ui32NumBytes < 100 )
        {
            //
            // If we can send the whole thing at once, go ahead and do that.
            //
            g_pfnSpiWrite(g_psIOSInterface, pui32Data, ui32NumBytes,
                                 AM_HAL_IOM_OFFSET(0x84));

            //
            // Write the packet header with the number of bytes and with the
            // continue bit set to zero.
            //
            sPacketHeader.bytes[0] = ui32NumBytes;
            sPacketHeader.bytes[1] = 0;

            g_pfnSpiWrite(g_psIOSInterface, sPacketHeader.words, 2,
                                 AM_HAL_IOM_OFFSET(0x80));
            return;
        }
        else
        {
            //
            // The packet is too big for a single transfer, so we'll send part
            // now, and use a continue bit for the rest.
            //
            g_pfnSpiWrite(g_psIOSInterface, pui32Data, 100,
                                 AM_HAL_IOM_OFFSET(0x84));

            //
            // Write the packet header with the number of bytes and with the
            // continue bit set to one.
            //
            sPacketHeader.bytes[0] = 100;
            sPacketHeader.bytes[1] = 1;

            g_pfnSpiWrite(g_psIOSInterface, sPacketHeader.words, 2,
                                 AM_HAL_IOM_OFFSET(0x80));

            //
            // Make sure to keep track of how much we've sent.
            //
            ui32NumBytes -= 100;
            pui32Data += 25;

            //
            // Wait for the IO Slave buffer to free up.
            //
            while (1)
            {
                //
                // Wait for the signal GPIO.
                //
#if AM_APOLLO3_GPIO
                while ( !am_hal_gpio_input_read(SIGNAL_GPIO) );
#else // AM_APOLLO3_GPIO
                while ( !am_hal_gpio_input_bit_read(SIGNAL_GPIO) );
#endif // AM_APOLLO3_GPIO

                //
                // Read the Slave's status area to make sure we're clear to send.
                //
                g_pfnSpiRead(g_psIOSInterface, &ui32Status, 1,
                                    AM_HAL_IOM_OFFSET(0x02));

                //
                // If the bottom status bit is set, we're clear to continue transmission.
                //
                if ( ui32Status & 0x01 )
                {
                    //
                    // Write the status back to acknowledge.
                    //
                    g_pfnSpiWrite(g_psIOSInterface, &ui32Status, 1,
                                        AM_HAL_IOM_OFFSET(0x83));
                    break;
                }
            }
        }
    }
}

//*****************************************************************************
//
//
//
//*****************************************************************************
uint32_t
am_devices_spiscm_read(uint32_t *ui32Data)
{
    uint32_t ui32PacketSize;
    am_hal_iom_buffer(2) sPacketHeader;

    //
    // Clear the status bit.
    //
    sPacketHeader.bytes[0] = 0;
    g_pfnSpiWrite(g_psIOSInterface, sPacketHeader.words, 1,
                         AM_HAL_IOM_OFFSET(0x83));

    //
    // Check the packet size.
    //
    g_pfnSpiRead(g_psIOSInterface, sPacketHeader.words, 2,
                        AM_HAL_IOM_OFFSET(0x7F));

    ui32PacketSize = sPacketHeader.bytes[0];

    //
    // Read the packet.
    //
    g_pfnSpiRead(g_psIOSInterface, ui32Data,
                        ui32PacketSize, AM_HAL_IOM_OFFSET(0x7F));

    return ui32PacketSize;
}

//*****************************************************************************
//
//
//
//*****************************************************************************
bool
am_devices_spiscm_message_check(void)
{
    am_hal_iom_buffer(2) sStatus;

#if AM_APOLLO3_GPIO
    if ( am_hal_gpio_input_read(SIGNAL_GPIO) )
    {
#else // AM_APOLLO3_GPIO
    if ( am_hal_gpio_input_bit_read(SIGNAL_GPIO) )
    {
#endif // AM_APOLLO3_GPIO
        //
        // Check the status bit to see why we were interrupted.
        //
        g_pfnSpiRead(g_psIOSInterface, sStatus.words, 1,
                            AM_HAL_IOM_OFFSET(0x02));

        //
        // If the message bit is set, we have a message.
        //
        if ( sStatus.bytes[0] & 0x2 )
        {
            return true;
        }
    }

    return false;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

