//*****************************************************************************
//
//! @file am_util_stxetx.h
//!
//! @brief Support for in channel packetization for UART and I/O Slave.
//!
//! Functions for providing packetization and depacketization for communication
//! over UART or I/O Slave.
//!
//! @addtogroup stxetx STXETX - UART and I/O Slave
//! @ingroup utils
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
#ifndef AM_UTIL_STXETX_H
#define AM_UTIL_STXETX_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @name STXETX byte code assignments.
//! @brief Byte code defines for the three special codes: STX, ETX and DLE.
//!
//! These macros are used in the byte stream.
//! @{
//
//*****************************************************************************
#define STXETX_STX (0x9A)
#define STXETX_ETX (0x9B)
#define STXETX_DLE (0x99)
//! @}

//*****************************************************************************
//
//!  STXETX global data structure definition.
//!  Global data structure definitons for the STXETX UART protocol.
//
//*****************************************************************************
typedef struct
{
    void (*rx_function)(int32_t, uint8_t *);
    bool bwait4stx_early_exit;
} g_am_util_stxetx_t;

extern g_am_util_stxetx_t g_am_util_stxetx;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Initialize the STXETX utility.
//!
//! @param MeasuredLength - length of payload
//! @param pui8Payload - pointer to the Payload.
//!
//! This function initializes the STXETX protocol utilty and registers the
//! function that will be used to crack open received packets.
//!
//! @returns None
//
//*****************************************************************************
extern void am_util_stxetx_init( void (*rx_function)(int32_t MeasuredLength, uint8_t *pui8Payload));

//*****************************************************************************
//
//! @brief Formats and transmits an STXETX packetized version of the payload.
//!
//! @param bFirst - issue the STX as the first byte.
//! @param bLast  - issue the ETX as the Last byte.
//! @param ui32Length - number of payload bytes.
//! @param ppui8Payload - pointer to pointer to buffer to be transmitted.
//!
//! This function will transmit the contents of the payload buffer.
//!
//! NOTE: There may be more bytes in the output buffer than came from the input
//! buffer. Let N = length of pay load buffer, then the output byte string can
//! be up to 2N long in some pathological cases.  If both first and last are
//! true then the output can be up to 2N+2 bytes long.
//!
//! @returns Number of characters written to puiPayload.
//
//*****************************************************************************
extern uint32_t am_util_stxetx_tx(bool bFirst, bool bLast, uint32_t ui32Length,
                                  uint8_t **ppui8Payload);

//*****************************************************************************
//
//! @brief Waits for STX marking start of packet.
//!
//! This function will recieve bytes from the input stream and wait for a valid
//! STX marking the start of packet.
//!
//! The return code signals whether it found an STX or return do to an external,
//! request to exit, e.g. from an ISR.
//!
//! @returns True for valid STX found, i.e. start of packet and false for any
//!          other return cause.
//
//
//*****************************************************************************
extern bool am_util_stxetx_rx_wait4start(void);

//*****************************************************************************
//
//! @brief Receives and extracts an STXETX formated packet.
//!
//! @param ui32MaxPayloadSize - Maximum payload size
//! @param pui8Payload - pointer to buffer to load from the UART.
//!
//! This function will recieve the contents of an STXETX demarcated packet from
//! a buffer. Once the end of transmission byte (ETX) byte is received, it
//! passes the extracted payload to the registered packet cracker function.
//! If the registered function pointer is NULL, then it is up to the caller to
//! do any further cracking of the packet.
//!
//! @returns The length of the received packet.
//
//*****************************************************************************
extern int32_t am_util_stxetx_rx(uint32_t ui32MaxPayloadSize,
                                 uint8_t *pui8Payload);

#ifdef __cplusplus
}
#endif

#endif // AM_UTIL_STXETX_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

