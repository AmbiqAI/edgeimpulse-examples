//*****************************************************************************
//
//! @file am_devices_i2ciomhc.c
//!
//! @brief Master-side functions for an I2C protocol between Apollo and a Host
//!
//! Master-side functions for an I2C based communications channel between
//! Apollo and a host such as the application processor in an Android tablet
//! or cell phone. This device service routine mimics a host processor.
//!
//! @addtogroup i2ciomhc I2C IO Master Driver
//! @ingroup devices
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

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_devices_i2ciomhc.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define  SLAVE_I2C_ADDRESS (0x4A)

//*****************************************************************************
//
//  Initialize this device service module.
//
//*****************************************************************************
void am_devices_i2ciomhc_init(void)
{
    uint32_t ui32IntStatusClear = 0x000000FF;
    uint32_t ui32IntEnable      = 0x00000003;

    //
    // Clear the interrupt status bits in the host side of the I/O Slave.
    //
    am_hal_iom_i2c_write(0,                             // IOM 0
                         SLAVE_I2C_ADDRESS,             // I2C Bus Address
                         &ui32IntStatusClear, 1,        // data, length
                         AM_HAL_IOM_OFFSET(0x7A));      // offset reg in slave

    //
    // Set the interrupt enable bits in the host side of the I/O Slave.
    //
    am_hal_iom_i2c_write(0,                             // IOM 0
                         SLAVE_I2C_ADDRESS,             // I2C Bus Address
                         &ui32IntEnable, 1,             // data, length
                         AM_HAL_IOM_OFFSET(0x78));      // offset reg in slave
}

//*****************************************************************************
//
//  Read and clear the interrupt status in the host side of the I/O slave.
//  This function uses polled I/O master I2C reads and writes.
//
//*****************************************************************************
uint8_t am_devices_i2ciomhc_read_int_status(void)
{
    uint32_t ui32IntStatus;
    uint32_t ui32IntStatusClear;

    //
    // Read one byte of status
    //
    am_hal_iom_i2c_read(0,                              // IOM 0
                        SLAVE_I2C_ADDRESS,              // I2C Bus Address
                        &ui32IntStatus, 1,              // data, length
                        AM_HAL_IOM_OFFSET(0x79));       // offset reg in slave

    //
    // Set up to clear the ones we just learned about
    //
    ui32IntStatusClear = ui32IntStatus;

    //
    // Clear the interrupt status bit in the host side of the I/O Slave.
    //
    am_hal_iom_i2c_write(0,                             // IOM 0
                         SLAVE_I2C_ADDRESS,             // I2C Bus Address
                         &ui32IntStatusClear, 1,        // data, length
                         AM_HAL_IOM_OFFSET(0x7A));      // offset reg in slave

    return ui32IntStatus & 0x000000FF;
}

//*****************************************************************************
//
// Receive a packet from the slave over I2C.
// This function uses polled I/O master I2C reads and writes.
//
//*****************************************************************************
void am_devices_i2ciomhc_rcv_packet(am_devices_i2ciomhc_message_t *Msg)
{
    uint32_t ui32Header;
    uint32_t ui32Payload[32];
    uint32_t ui32Length;
    uint32_t ui32I;
    uint8_t *ui8PtrSrc = (uint8_t *)ui32Payload;
    uint8_t *ui8PtrDst = Msg->Message;

    //
    // Read 4 byte header from the FIFO.
    //
    am_hal_iom_i2c_read(0,                              // IOM 0
                        SLAVE_I2C_ADDRESS,              // I2C Bus Address
                        &ui32Header, 4,                 // data, length
                        AM_HAL_IOM_OFFSET(0x7F));       // offset reg in slave

    //
    // Extract the header info
    //
    ui32Length = ((ui32Header >> 0) & 0x000000FF)
               | ((ui32Header >> 8) & 0x000000FF);
    Msg->Length = ui32Length;
    Msg->ChannelNumber  = ((ui32Header >> 16) & 0x000000FF);
    Msg->SequenceNumber = ((ui32Header >> 24) & 0x000000FF);

    //
    // Now read the payload from the FIFO.
    //
    am_hal_iom_i2c_read(0,                              // IOM 0
                        SLAVE_I2C_ADDRESS,              // I2C Bus Address
                        ui32Payload, ui32Length - 4,    // data, length
                        AM_HAL_IOM_OFFSET(0x7F));       // offset reg in slave

    //
    // Copy the payload into the Message structure.
    //
    for (ui32I = 0; ui32I < (ui32Length-4); ui32I++)
    {
        *ui8PtrDst++ = *ui8PtrSrc++;
    }

}

//*****************************************************************************
//
//  Transmit a packet to the slave over I2C.
// This function uses polled I/O master I2C reads and writes.\n
//
//*****************************************************************************
void am_devices_i2ciomhc_xmit_packet(am_devices_i2ciomhc_message_t *Msg)
{
    uint32_t ui32Message[32];
    uint8_t *ui8PtrMessage = (uint8_t *)ui32Message;
    uint8_t *ui8PtrSrc = Msg->Message;
    uint32_t ui32I;
    uint32_t ui32Offset;

    //
    // format the message string for transmission
    //
    *ui8PtrMessage++ = (Msg->Length >> 0) & 0x000000FF;
    *ui8PtrMessage++ = (Msg->Length >> 8) & 0x000000FF;
    *ui8PtrMessage++ =  Msg->ChannelNumber;
    *ui8PtrMessage++ =  Msg->SequenceNumber;

    //
    // Copy the payload into the Message string.
    //
    for (ui32I = 0; ui32I < (Msg->Length-4); ui32I++)
    {
        *ui8PtrMessage++ = *ui8PtrSrc++;
    }

    //
    // This is the tricky part and it won't work if the (hdr+payload+1) > 64 bytes
    // on the Apollo I/O master. On an Android I2C master it could go up to 78 bytes.
    // This byte must be written at register 0x4F in the Slave's I2C register space.
    //
    *ui8PtrMessage++ = Msg->Length & 0x000000FF;
    ui32Offset       = 0x4F - Msg->Length - 1 + 1; // +1 to reach 4F

    //
    // Ok send this puppy to the slave
    //
    am_hal_iom_i2c_write(0,                               // IOM 0
                         SLAVE_I2C_ADDRESS,               // I2C Bus Address
                         ui32Message,                     // data,
                         Msg->Length + 1,                 // length
                         AM_HAL_IOM_OFFSET(ui32Offset));  // offset reg in slave
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

