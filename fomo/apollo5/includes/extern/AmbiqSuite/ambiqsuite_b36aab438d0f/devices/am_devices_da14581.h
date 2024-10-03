//*****************************************************************************
//
//! @file am_devices_da14581.h
//!
//! @brief Support functions for the Dialog Semiconductor DA14581 BTLE radio.
//!
//! @addtogroup da14581 DA14581 BTLE Radio Device Driver
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
#ifndef AM_DEVICES_DA14581_H
#define AM_DEVICES_DA14581_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @name Defines
//! @{
//
//*****************************************************************************
#define AM_DEVICES_DA14581_UART_MODE         (0)
#define AM_DEVICES_DA14581_SPI_MODE          (1)

#define AM_DEVICES_DA14581_SPI_XPORT_CTS     (0x06)
#define AM_DEVICES_DA14581_SPI_XPORT_NOT_CTS (0x07)

//! @}

//*****************************************************************************
//
//! DA14581 device structure
//
//*****************************************************************************
typedef struct
{
    //
    //! MODE UART vs IOM SPI
    //
    uint32_t ui32Mode;

    //
    //! IOM Module #
    //
    uint32_t ui32IOMModule;

    //
    //! IOM Chip Select NOTE: this driver uses GPIO for chip selects
    //
    uint32_t ui32IOMChipSelect;

    //
    //! GPIO # for DA14581 DREADY signal
    //
    uint32_t ui32DREADY;
}
am_devices_da14581_t;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
// #### INTERNAL BEGIN ####
//*****************************************************************************
//
//! @brief Handles Initialization of the DA14581 after SPI HCI Download
//!
//! @param psDevice - Pointer to device information structure for the DA14581
//!
//! This funciton looks for the DREADY line to be high and returns false if
//! not. It sends 1 byte read command over the IOM to read the protocol control
//! byte from the DA14581. If this is not 0x06 then false is returned.
//!
//! @return true if successful.
//
//*****************************************************************************
extern bool am_devices_da14581_init(am_devices_da14581_t *psDevice);
// #### INTERNAL END ####
//*****************************************************************************
//
//! @brief Sets the MAC address to send to the dialog radio on the next boot up.
//!
//! @param pui8MacAddress
//
//*****************************************************************************
extern void am_devices_da14581_mac_set(const uint8_t *pui8MacAddress);
//*****************************************************************************
//
//! @brief Runs a UART based boot sequence for a Dialog radio device.
//!
//! @param pui8BinData    - Pointer to an array of bytes containing the
//!                         firmware for the DA14581
//! @param ui32NumBytes   - Length of the DA14581 firmware image.
//! @param ui32UartModule - Uart module number
//!
//! This function allows the Ambiq device to program a "blank" DA14581 device
//! on startup. It will handle all of the necessary UART negotiation for the
//! Dialog boot procedure, and will verify that the CRC value for the
//! downloaded firmware image is correct.
//!
//! @return true if successful.
//
//*****************************************************************************
extern bool am_devices_da14581_uart_boot(const uint8_t *pui8BinData,
                                         uint32_t ui32NumBytes,
                                         uint32_t ui32UartModule);
// #### INTERNAL BEGIN ####
//*****************************************************************************
//
//! @brief Sends Command Packets to the DA14581 on the SPI Xport
//!
//! @param psDevice - Pointer to device information structure for the DA14581
//! @param options
//! @param type
//! @param pData    - Pointer to data buffer. Note this is uint8_t aligned.
//! @param len      - Length of raw transfer
//!
//! This function sends a buffer of bytes to the DA14581
//!
//! @note This uses polled IOM transfers
//
//*****************************************************************************
extern void am_devices_da14581_spi_send(am_devices_da14581_t *psDevice,
                                        uint32_t options,
                                        uint8_t type, uint8_t *pData,
                                        uint16_t len);

//*****************************************************************************
//
//! @brief Recieve Bytes from the DA14581 on the SPI Xport
//!
//! @param psDevice - Pointer to device information structure for the DA14581
//! @param options  - Options
//! @param pData    - Pointer to data buffer. Note this is uint8_t aligned.
//! @param len      - Length of raw transfer
//!
//! This function receives a buffer of bytes from the DA14581
//!
//! @note This uses polled IOM transfers
//
//*****************************************************************************
extern void am_devices_da14581_spi_receive_bytes(
                                        am_devices_da14581_t *psDevice,
                                        uint32_t options,
                                        uint8_t *pData, uint16_t len);
// #### INTERNAL END ####
#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_DA14581_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

