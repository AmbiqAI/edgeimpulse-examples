//*****************************************************************************
//
//! @file am_devices_i2cioshc.h
//!
//! @brief Slave-side functions for an I2C protocol between Apollo and a Host
//!
//! Slave-side functions for an I2C based communications channel between
//! Apollo and a host such as the application processor in an Android tablet
//! or cell phone.
//!
//! @addtogroup i2cioshc I2C IO Slave Driver
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
#ifndef AM_DEVICES_I2CIOSHC_H
#define AM_DEVICES_I2CIOSHC_H

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define AM_DEVICES_I2CIOSHC_LAST_RCV_BYTE   0x4F
#define AM_DEVICES_I2CIOSHC_FIFO_BASE       0x80
#define AM_DEVICES_I2CIOSHC_FIFO_END        0x100
#define AM_DEVICES_I2CIOSHC_PAYLOAD_SIZE    (62)

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
typedef struct am_devices_i2cioshc_message_s
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
    //! Pointer to the actual message.
    //
    uint8_t Message[AM_DEVICES_I2CIOSHC_PAYLOAD_SIZE];

} am_devices_i2cioshc_message_t;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Initialize the I/O Slave and get ready to commune with the hosts.
//!
//! @param psIOSConfig
//
//*****************************************************************************
extern void am_devices_i2cioshc_init(am_hal_ios_config_t *psIOSConfig);

//*****************************************************************************
//
//! @brief Call this function to format an SHTP message.
//!
//! @param psMessage
//! @param pBuffer
//
//*****************************************************************************
extern void am_devices_i2cioshc_message_format(
                   am_devices_i2cioshc_message_t *psMessage,
                   uint8_t *pBuffer);

//*****************************************************************************
//
//! @brief Call this function to extract an SHTP message.
//!
//! @param psMessage
//! @param pBuffer
//
//*****************************************************************************
extern void am_devices_i2cioshc_message_extract(
                   am_devices_i2cioshc_message_t *psMessage,
                   uint8_t *pBuffer);

//*****************************************************************************
//
//! @brief Call this function from their I/O Slave Access ISR
//!
//! @param pui8Destination
//! @param ui32Status
//
//*****************************************************************************
extern void am_devices_i2cioshc_acc_service(uint8_t *pui8Destination,
                                            uint32_t ui32Status);

//*****************************************************************************
//
//! @brief Call this function from release the inbound hardware buffer once it is
//! completely free and no longer needed.
//
//*****************************************************************************
extern void am_devices_i2cioshc_release_inbound_hw_buffer(void);

//*****************************************************************************
//
//! @brief Call this function from their I/O Slave IOS Interrupt ISR
//!
//! @param XmitMsgHandler
//! @param ui32Status
//
//! We really don't need to do anything here. The host should have cleared
//! the interrupt bit (SWINT0) from their side. The FIFO is finished so
//! nothing to do there either.
//
//*****************************************************************************
extern void am_devices_i2cioshc_ios_int_service(void (*XmitMsgHandler)(void),
                                                uint32_t ui32Status);

//*****************************************************************************
//
//! @brief Call this function to send a message to the host.
//!
//! @param pui8Data
//! @param ui8Size
//!
//! @note The message should include the standard 4 byte header which includes
//! a 2 byte length field.
//
//*****************************************************************************
extern void am_devices_i2cioshc_write(uint8_t *pui8Data,
                                      uint8_t ui8Size);

// @todo delete:
//extern void am_devices_i2cioshc_fifo_service(uint32_t ui32Status);
//extern uint32_t am_devices_i2cioshc_read(uint8_t *pui8Data,
//                                        uint32_t ui32MaxSize);
//extern bool am_devices_i2cioshc_check(void);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_I2CIOSHC_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

