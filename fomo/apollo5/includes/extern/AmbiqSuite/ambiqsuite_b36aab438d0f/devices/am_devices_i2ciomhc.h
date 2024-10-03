//*****************************************************************************
//
//! @file am_devices_i2cioshc.h
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
#ifndef AM_DEVICES_I2CIOMHC_H
#define AM_DEVICES_I2CIOMHC_H

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
//#define AM_DEVICES_I2CIOSHC_LAST_RCV_BYTE   0x4F
//#define AM_DEVICES_I2CIOSHC_FIFO_BASE       0x78
//#define AM_DEVICES_I2CIOSHC_FIFO_END        0x100
#define AM_DEVICES_I2CIOMHC_PAYLOAD_SIZE    (62)

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************

//*****************************************************************************
//
//! Message Header
//
//*****************************************************************************
typedef struct
{
    //
    //! Length of the message including header bytes.
    //
    uint32_t Length;

    //
    //! Channel Number on which this message flows.
    //
    uint8_t ChannelNumber;

    //
    //! Sequence Number for this message.
    //
    uint8_t SequenceNumber;

    //
    //! Pointer to the actual Message
    //
    uint8_t Message[AM_DEVICES_I2CIOMHC_PAYLOAD_SIZE];

} am_devices_i2ciomhc_message_t;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Initialize this device service module.
//
//*****************************************************************************
extern void am_devices_i2ciomhc_init(void);

//*****************************************************************************
//
//! @brief Read and clear the interrupt status in the host side of the I/O slave.
//! This function uses polled I/O master I2C reads and writes.
//
//*****************************************************************************
extern uint8_t am_devices_i2ciomhc_read_int_status(void);

//*****************************************************************************
//
//! @brief  Receive a packet from the slave over I2C.
//!
//! @param Msg
//!
//! This function uses polled I/O master I2C reads and writes.
//
//*****************************************************************************
extern void am_devices_i2ciomhc_rcv_packet(am_devices_i2ciomhc_message_t *Msg);
//*****************************************************************************
//
//! @brief  Transmit a packet to the slave over I2C.
//!
//! @param Msg
//!
//! This function uses polled I/O master I2C reads and writes.\n
//! Assumes 2 Length bytes includes 4 for the header.
//
//*****************************************************************************
extern void am_devices_i2ciomhc_xmit_packet(am_devices_i2ciomhc_message_t *Msg);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_I2CIOMHC_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

