//*****************************************************************************
//
//! @file am_devices_i2cfram.c
//!
//! @brief Generic I2C FRAM driver.
//!
//! @addtogroup i2cfram I2C FRAM Driver
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

#include <string.h>
#include "am_mcu_apollo.h"
#include "am_devices_i2cfram.h"

//! Maximum supported by IOM device in one transfer.
#define MAX_I2C_TRANSFERSIZE (255)
//! Support max 4 queued transactions
#define MAX_I2C_QUEUED_WRITES   4

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
am_devices_i2cfram_t g_sI2cIOMSettings;

//! Write buffer when single write must be used. Otherwise use caller's buffer.
am_hal_iom_buffer(MAX_I2C_TRANSFERSIZE) g_i2cXferBuffer[MAX_I2C_QUEUED_WRITES];
uint32_t                            g_i2cWrBufIdx = 0;
am_hal_iom_buffer(4)                g_i2cTempBuffer[MAX_I2C_QUEUED_WRITES];
uint32_t                            g_i2cTempBufIdx = 0;

am_hal_iom_buffer(4)         g_i2cCmdResponse;

//*****************************************************************************
//
// Initialize the i2cfram driver.
//
//*****************************************************************************
void
am_devices_i2cfram_init(am_devices_i2cfram_t *psIOMSettings)
{
    //
    // Initialize the IOM settings from the application.
    //
    g_sI2cIOMSettings.ui32IOMModule = psIOMSettings->ui32IOMModule;
    g_sI2cIOMSettings.ui32BusAddress = psIOMSettings->ui32BusAddress;

}

//*****************************************************************************
//
//  read and return the ID register, manfID, productID from external FRAM
//
//*****************************************************************************
uint32_t
am_devices_i2cfram_id(uint16_t *manufacturerID, uint16_t *productID)
{
    //
    // Set the CMD and copy the data into the same buffer.
    //
    g_i2cTempBuffer[g_i2cTempBufIdx].bytes[0] = g_sI2cIOMSettings.ui32BusAddress << 1;

    //
    // Send a command to read the ID register in the external fram.
    am_hal_iom_i2c_write(g_sI2cIOMSettings.ui32IOMModule,
                        0xF8 >> 1,
                        g_i2cTempBuffer[g_i2cTempBufIdx].words, 1,
                        AM_HAL_IOM_NO_STOP | AM_HAL_IOM_RAW);
    g_i2cCmdResponse.words[0] = 0;
    am_hal_iom_i2c_read(g_sI2cIOMSettings.ui32IOMModule,
                        0xF9 >> 1,
                        g_i2cCmdResponse.words, 3,
                        AM_HAL_IOM_RAW);

    //
    // Return the ID
    //
#if 0
    *manufacturerID = ((g_i2cCmdResponse.words[0] & 0x000000FF) << 4) +
                      ((g_i2cCmdResponse.words[0] & 0x0000F000) >> 12);
    *productID = ((g_i2cCmdResponse.words[0] & 0x00FF0000 ) >> 16) +
                 ((g_i2cCmdResponse.words[0] & 0x00000F00 ) );
#else
    *manufacturerID = ((g_i2cCmdResponse.bytes[0]) << 4) +
                      ((g_i2cCmdResponse.bytes[1] & 0xF0) >> 4);
    *productID = (g_i2cCmdResponse.bytes[2]) +
                 ((g_i2cCmdResponse.bytes[1] & 0xF) << 8);
#endif
    return g_i2cCmdResponse.words[0];
}

//*****************************************************************************
//
// Reads the contents of the external fram into a buffer.
//
//*****************************************************************************
void
am_devices_i2cfram_read(uint8_t *pui8RxBuffer, uint32_t ui32ReadAddress,
                        uint32_t ui32NumBytes)
{
    uint32_t ui32BytesRemaining, ui32CurrentReadAddress;
    uint8_t *pui8Dest;
    //
    // Set the total number of bytes,and the starting transfer destination.
    //
    ui32BytesRemaining = ui32NumBytes;
    pui8Dest = pui8RxBuffer;
    ui32CurrentReadAddress = ui32ReadAddress;

    while ( ui32BytesRemaining > 0)
    {
        //
        // Set the transfer size to either TRANSFERSIZE,
        // or the number of remaining bytes, whichever is smaller.
        //
        uint32_t ui32TransferSize =
            (ui32BytesRemaining > MAX_I2C_TRANSFERSIZE) ? MAX_I2C_TRANSFERSIZE: ui32BytesRemaining;

        //
        // READ is a 2-byte command (16-bit address)
        //
        g_i2cTempBuffer[g_i2cTempBufIdx].bytes[0] = (ui32CurrentReadAddress & 0x0000FF00) >> 8;
        g_i2cTempBuffer[g_i2cTempBufIdx].bytes[1] = (ui32CurrentReadAddress & 0x000000FF);

        //
        // Send Read Command to I2CFRAM, Hold CS Low, then SPI Read to receive data.
        //
        am_hal_iom_spi_write(g_sI2cIOMSettings.ui32IOMModule,
                             g_sI2cIOMSettings.ui32BusAddress,
                             g_i2cTempBuffer[g_i2cTempBufIdx].words, 2,
                             AM_HAL_IOM_NO_STOP | AM_HAL_IOM_RAW);

        am_hal_iom_spi_read(g_sI2cIOMSettings.ui32IOMModule,
                            g_sI2cIOMSettings.ui32BusAddress, (uint32_t *)pui8Dest,
                            ui32TransferSize, AM_HAL_IOM_RAW);

        //
        // Update the number of bytes remaining and the destination.
        //
        ui32BytesRemaining -= ui32TransferSize;
        pui8Dest += ui32TransferSize;
        ui32CurrentReadAddress += ui32TransferSize;
    }
}

//*****************************************************************************
//
//  Programs the given range of fram addresses.
//
//*****************************************************************************
void
am_devices_i2cfram_write(uint8_t *pui8TxBuffer, uint32_t ui32WriteAddress,
                         uint32_t ui32NumBytes)
{
    uint32_t ui32DestAddress;
    uint32_t ui32BytesRemaining;
    uint8_t *pui8Source;

    //
    // Set the total number of bytes, and the starting transfer destination.
    //
    ui32BytesRemaining = ui32NumBytes;
    pui8Source = pui8TxBuffer;
    ui32DestAddress = ui32WriteAddress;

    while ( ui32BytesRemaining )
    {
        //
        // Set the transfer size to either TRANSFERSIZE,
        // or the number of remaining bytes, whichever is smaller.
        //
        uint32_t ui32TransferSize =
            (ui32BytesRemaining > MAX_I2C_TRANSFERSIZE - 2) ? \
                (MAX_I2C_TRANSFERSIZE - 2): \
                ui32BytesRemaining;

        //
        // Set the CMD and copy the data into the same buffer.
        // Command is 2 byte offset, followed by the data
        //
        g_i2cXferBuffer[g_i2cWrBufIdx].bytes[0] = (ui32DestAddress & 0x0000FF00) >> 8;
        g_i2cXferBuffer[g_i2cWrBufIdx].bytes[1] = (ui32DestAddress & 0x000000FF);

        memcpy(&g_i2cXferBuffer[g_i2cWrBufIdx].bytes[2], pui8Source, ui32TransferSize);

        //
        // Send Write Offset followed by data to I2CFRAM.
        //
        am_hal_iom_spi_write(g_sI2cIOMSettings.ui32IOMModule,
                             g_sI2cIOMSettings.ui32BusAddress,
                             g_i2cTempBuffer[g_i2cTempBufIdx].words, ui32TransferSize + 2,
                             AM_HAL_IOM_RAW);
        //
        // Update the number of bytes remaining, as well as the source and
        // destination pointers
        //
        ui32BytesRemaining -= ui32TransferSize;
        pui8Source += ui32TransferSize;
        ui32DestAddress += ui32TransferSize;
    }
    return;
}

//##### INTERNAL BEGIN #####

//*****************************************************************************
//
// Reads the contents of the external fram into a buffer.
//
//*****************************************************************************
void
am_devices_i2cfram_read_mode(uint8_t *pui8RxBuffer, uint32_t ui32ReadAddress,
                             uint32_t ui32NumBytes, AM_HAL_IOM_MODE_E mode,
                             am_hal_iom_callback_t pfnCallback)
{
    uint32_t ui32BytesRemaining, ui32CurrentReadAddress;
    uint8_t *pui8Dest;
    am_hal_iom_oper_t oper;
    //
    // Set the total number of bytes,and the starting transfer destination.
    //
    ui32BytesRemaining = ui32NumBytes;
    pui8Dest = pui8RxBuffer;
    ui32CurrentReadAddress = ui32ReadAddress;

    oper.mode = mode;
    oper.peerInfo.ui32BusAddress = g_sI2cIOMSettings.ui32BusAddress;
    oper.pfnCallback = 0;
    oper.proto = AM_HAL_IOM_PROTO_I2C;

    while ( ui32BytesRemaining > 0)
    {
        //
        // Set the transfer size to either TRANSFERSIZE,
        // or the number of remaining bytes, whichever is smaller.
        //
        uint32_t ui32TransferSize =
            (ui32BytesRemaining > MAX_I2C_TRANSFERSIZE) ? MAX_I2C_TRANSFERSIZE: ui32BytesRemaining;

        //
        // READ is a 2-byte command (16-bit address)
        //
        g_i2cTempBuffer[g_i2cTempBufIdx].bytes[0] = (ui32CurrentReadAddress & 0x0000FF00) >> 8;
        g_i2cTempBuffer[g_i2cTempBufIdx].bytes[1] = (ui32CurrentReadAddress & 0x000000FF);

        oper.oper = AM_HAL_IOM_OPER_WRITE;
        oper.pui32Data = g_i2cTempBuffer[g_i2cTempBufIdx].words;
        oper.ui32NumBytes = 2;
        oper.ui32Options = AM_HAL_IOM_NO_STOP | AM_HAL_IOM_RAW;
        //
        // Send Read Command to SPIFRAM, Hold CS Low, then SPI Read to recieve data.
        //
        am_hal_iom_operate(g_sI2cIOMSettings.ui32IOMModule, &oper);
        g_i2cTempBufIdx = (g_i2cTempBufIdx + 1) % MAX_I2C_QUEUED_WRITES;

        oper.oper = AM_HAL_IOM_OPER_READ;
        oper.pui32Data = (uint32_t *)pui8Dest;
        oper.ui32NumBytes = ui32TransferSize;
        oper.ui32Options = AM_HAL_IOM_RAW;

        //
        // Update the number of bytes remaining and the destination.
        //
        ui32BytesRemaining -= ui32TransferSize;
        pui8Dest += ui32TransferSize;
        ui32CurrentReadAddress += ui32TransferSize;
        if (ui32BytesRemaining == 0)
        {
            oper.pfnCallback = pfnCallback;
        }
        am_hal_iom_operate(g_sI2cIOMSettings.ui32IOMModule, &oper);
    }
}

//*****************************************************************************
//
// Programs the given range of fram addresses.
//
//*****************************************************************************
void
am_devices_i2cfram_write_mode(uint8_t *pui8TxBuffer, uint32_t ui32WriteAddress,
                              uint32_t ui32NumBytes, AM_HAL_IOM_MODE_E mode,
                              am_hal_iom_callback_t pfnCallback)
{
    uint32_t ui32DestAddress;
    uint32_t ui32BytesRemaining;
    uint8_t *pui8Source;
    am_hal_iom_oper_t oper;

    //
    // Set the total number of bytes, and the starting transfer destination.
    //
    ui32BytesRemaining = ui32NumBytes;
    pui8Source = pui8TxBuffer;
    ui32DestAddress = ui32WriteAddress;

    oper.mode = mode;
    oper.peerInfo.ui32BusAddress = g_sI2cIOMSettings.ui32BusAddress;
    oper.pfnCallback = 0;
    oper.proto = AM_HAL_IOM_PROTO_I2C;

    while ( ui32BytesRemaining )
    {
        //
        // Send the write-enable command to prepare the external fram for
        // program operations, and wait for the write-enable latch to be set in
        // the status register.
        //
        //
        //
        // Set the transfer size to either TRANSFERSIZE,
        // or the number of remaining bytes, whichever is smaller.
        //
        uint32_t ui32TransferSize =
            (ui32BytesRemaining > MAX_I2C_TRANSFERSIZE - 2) ? \
                (MAX_I2C_TRANSFERSIZE - 2): \
                ui32BytesRemaining;

        //
        // Set the CMD and copy the data into the same buffer.
        // Command is 2 byte offset, followed by the data
        //
        g_i2cXferBuffer[g_i2cWrBufIdx].bytes[0] = (ui32DestAddress & 0x0000FF00) >> 8;
        g_i2cXferBuffer[g_i2cWrBufIdx].bytes[1] = (ui32DestAddress & 0x000000FF);

        // IAR doesn't have bcopy(pui8Source, &g_i2cXferBuffer.bytes[3], ui32TransferSize);
        memcpy(&g_i2cXferBuffer[g_i2cWrBufIdx].bytes[2], pui8Source, ui32TransferSize);

        //
        // Send the 3 byte write command and the data to the IOM.
        //
        oper.oper = AM_HAL_IOM_OPER_WRITE;
        oper.pui32Data = g_i2cXferBuffer[g_i2cWrBufIdx].words;
        oper.ui32NumBytes = ui32TransferSize + 2;
        oper.ui32Options = AM_HAL_IOM_RAW;
        g_i2cWrBufIdx = (g_i2cWrBufIdx + 1) % MAX_I2C_QUEUED_WRITES;
        //
        // Update the number of bytes remaining, as well as the source and
        // destination pointers
        //
        ui32BytesRemaining -= ui32TransferSize;
        pui8Source += ui32TransferSize;
        ui32DestAddress += ui32TransferSize;

        if (ui32BytesRemaining == 0)
        {
            oper.pfnCallback = pfnCallback;
        }
        am_hal_iom_operate(g_sI2cIOMSettings.ui32IOMModule, &oper);
    }

    return;
}

//##### INTERNAL END #####

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

