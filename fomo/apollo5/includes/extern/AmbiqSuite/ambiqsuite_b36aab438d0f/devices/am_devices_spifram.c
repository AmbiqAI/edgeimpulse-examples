//*****************************************************************************
//
//! @file am_devices_spifram.c
//!
//! @brief Generic spifram driver.
//!
//! @addtogroup spifram SPI FRAM Driver
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

#include <string.h>
#include "am_mcu_apollo.h"
#include "am_devices_spifram.h"

//! Maximum supported by IOM device in one transfer.
#define MAX_TRANSFERSIZE (4095)
//! Adjusted 4091 max for IOM 4.
#define MAX_TRANSFERSIZE_IOM4 (4091)
//! Support max 4 queued transactions
#define MAX_QUEUED_WRITES   4

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
am_devices_spifram_t g_sSpiIOMSettings;

//! @name Write buffer when single write must be used. Otherwise use caller's buffer.
//! @{
am_hal_iom_buffer(MAX_TRANSFERSIZE) g_xferBuffer[MAX_QUEUED_WRITES];
uint32_t                            g_WrBufIdx = 0;
am_hal_iom_buffer(4)                g_tempBuffer[MAX_QUEUED_WRITES];
uint32_t                            g_TempBufIdx = 0;

//! @}

//*****************************************************************************
//
// Initialize the spifram driver.
//
//*****************************************************************************
void
am_devices_spifram_init(am_devices_spifram_t *psIOMSettings)
{
    //
    // Initialize the IOM settings from the application.
    //
    g_sSpiIOMSettings.ui32IOMModule = psIOMSettings->ui32IOMModule;
    g_sSpiIOMSettings.ui32ChipSelect = psIOMSettings->ui32ChipSelect;

}

am_hal_iom_buffer(4) g_spiCmdResponse;

//*****************************************************************************
//
// Reads the ID register for the external fram
//
//*****************************************************************************
uint32_t
am_devices_spifram_id(uint8_t *manufacturerID, uint16_t *productID)
{

    //
    // Send a command to read the ID register in the external fram.
    //
    //
    // Check to see if the callback pointer is valid.
    //
    g_spiCmdResponse.words[0] = 0;
    am_hal_iom_spi_read(g_sSpiIOMSettings.ui32IOMModule,
                        g_sSpiIOMSettings.ui32ChipSelect,
                        g_spiCmdResponse.words, 4,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_SPIFRAM_RDID));

    //
    // Return the ID
    //
    /* Shift values to separate manuf and prod IDs */
    //manufacturerID = (a[0]);
    //productID = (a[2] << 8) + a[3];
    *manufacturerID = (g_spiCmdResponse.words[0] & 0x000000FF);
    *productID = ((g_spiCmdResponse.words[0] & 0xFF000000 ) >> 24) +
                 ((g_spiCmdResponse.words[0] & 0x00FF0000 ) >>  8);

    return g_spiCmdResponse.words[0];
}

//*****************************************************************************
//
// Reads the current status of the external fram
//
//*****************************************************************************
uint8_t
am_devices_spifram_status(void)
{

    //
    // Send the command and read the response.
    //
    am_hal_iom_spi_read(g_sSpiIOMSettings.ui32IOMModule,
                        g_sSpiIOMSettings.ui32ChipSelect,
                        g_spiCmdResponse.words, 1,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_SPIFRAM_RDRSR));

    //
    // Return the status read from the external fram.
    //
    return g_spiCmdResponse.bytes[0];
}

//*****************************************************************************
//
// Reads the current status of the external fram
//
//*****************************************************************************
void
am_devices_spifram_status_write(uint32_t ui32StatusValue)
{
    //am_hal_iom_buffer(4) cmdBuf;

    g_tempBuffer[g_TempBufIdx].bytes[0] = AM_DEVICES_SPIFRAM_WREN;
    g_tempBuffer[g_TempBufIdx].bytes[1] = 0;
    g_tempBuffer[g_TempBufIdx].bytes[2] = 0;
    g_tempBuffer[g_TempBufIdx].bytes[3] = 0;

    //
    // Send the 1 byte write enable command to the IOM.
    //
    am_hal_iom_spi_write(g_sSpiIOMSettings.ui32IOMModule,
                         g_sSpiIOMSettings.ui32ChipSelect,
                         g_tempBuffer[g_TempBufIdx].words,
                         1, AM_HAL_IOM_RAW);

    //
    // Send the 2 byte Write Status Register command
    //
    g_tempBuffer[g_TempBufIdx].bytes[0] = AM_DEVICES_SPIFRAM_WRSR;
    g_tempBuffer[g_TempBufIdx].bytes[1] = (uint8_t)ui32StatusValue;

    //
    // Send the 2 byte write command to the IOM.
    //
    am_hal_iom_spi_write(g_sSpiIOMSettings.ui32IOMModule,
                         g_sSpiIOMSettings.ui32ChipSelect,
                         g_tempBuffer[g_TempBufIdx].words,
                         2, AM_HAL_IOM_RAW);

    g_tempBuffer[g_TempBufIdx].bytes[0] = AM_DEVICES_SPIFRAM_WRDI;
    g_tempBuffer[g_TempBufIdx].bytes[1] = 0;
    g_tempBuffer[g_TempBufIdx].bytes[2] = 0;
    g_tempBuffer[g_TempBufIdx].bytes[3] = 0;

    //
    // Issue the WRDI command to reset WEL (Write Enable Latch).
    // The WREN command sets WEL.
    //
    am_hal_iom_spi_write(g_sSpiIOMSettings.ui32IOMModule,
                         g_sSpiIOMSettings.ui32ChipSelect,
                         g_tempBuffer[g_TempBufIdx].words,
                         1, AM_HAL_IOM_RAW);
}

//*****************************************************************************
//
// Return max transfersize or bytes Remaining.
//
//*****************************************************************************
uint32_t am_devices_spifram_max_transfersize(
         uint8_t rwcmd, uint32_t iomModule, uint32_t bytesRemaining)
{
    uint32_t max_transfersize =
        (iomModule == 4 ? MAX_TRANSFERSIZE_IOM4 : MAX_TRANSFERSIZE);

    if (rwcmd == AM_DEVICES_SPIFRAM_WRITE)
    {
        max_transfersize -= 3;
    }

    return (bytesRemaining > max_transfersize ? max_transfersize : bytesRemaining);
}

//*****************************************************************************
//
// Reads the contents of the external fram into a buffer.
//
//*****************************************************************************
void
am_devices_spifram_read(uint8_t *pui8RxBuffer, uint32_t ui32ReadAddress,
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
        uint32_t ui32TransferSize = am_devices_spifram_max_transfersize(
            AM_DEVICES_SPIFRAM_WRITE, g_sSpiIOMSettings.ui32IOMModule,
            ui32BytesRemaining);

        //
        // READ is a 3-byte command (opcode + 16-bit address)
        //
        g_tempBuffer[g_TempBufIdx].bytes[0] = AM_DEVICES_SPIFRAM_READ;
        g_tempBuffer[g_TempBufIdx].bytes[1] = (ui32CurrentReadAddress & 0x0000FF00) >> 8;
        g_tempBuffer[g_TempBufIdx].bytes[2] = (ui32CurrentReadAddress & 0x000000FF);
        g_tempBuffer[g_TempBufIdx].bytes[3] = 0;

        //
        // Send Read Command to SPIFRAM, Hold CS Low, then SPI Read to recieve data.
        //
        am_hal_iom_spi_write(g_sSpiIOMSettings.ui32IOMModule,
                             g_sSpiIOMSettings.ui32ChipSelect,
                             g_tempBuffer[g_TempBufIdx].words, 3,
                             AM_HAL_IOM_CS_LOW | AM_HAL_IOM_RAW);

        am_hal_iom_spi_read(g_sSpiIOMSettings.ui32IOMModule,
                            g_sSpiIOMSettings.ui32ChipSelect, (uint32_t *)pui8Dest,
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
// Programs the given range of fram addresses.
//
//*****************************************************************************
void
am_devices_spifram_write(uint8_t *pui8TxBuffer, uint32_t ui32WriteAddress,
                         uint32_t ui32NumBytes)
{
    uint32_t ui32DestAddress;
    uint32_t ui32BytesRemaining;
    uint8_t *pui8Source;

    //
    // Prepare the command for write-enable.
    //
    uint32_t am_devices_spifram_wren = AM_DEVICES_SPIFRAM_WREN;
    uint32_t am_devices_spifram_wrdi = AM_DEVICES_SPIFRAM_WRDI;

    //
    // Set the total number of bytes, and the starting transfer destination.
    //
    ui32BytesRemaining = ui32NumBytes;
    pui8Source = pui8TxBuffer;
    ui32DestAddress = ui32WriteAddress;

    while ( ui32BytesRemaining )
    {
        //
        // Send the write-enable command to prepare the external fram for
        // program operations, and wait for the write-enable latch to be set in
        // the status register.
        //
        //
        // Send the 1 byte write command, to the spifram.
        //
        am_hal_iom_spi_write(g_sSpiIOMSettings.ui32IOMModule,
                             g_sSpiIOMSettings.ui32ChipSelect,
                             &am_devices_spifram_wren,
                             1, AM_HAL_IOM_RAW);

        //
        // Set the transfer size to either TRANSFERSIZE,
        // or the number of remaining bytes, whichever is smaller.
        //
        uint32_t ui32TransferSize = am_devices_spifram_max_transfersize(
            AM_DEVICES_SPIFRAM_WRITE, g_sSpiIOMSettings.ui32IOMModule,
            ui32BytesRemaining);

        //
        // Set the CMD and copy the data into the same buffer.
        //
        g_xferBuffer[g_WrBufIdx].bytes[0] = AM_DEVICES_SPIFRAM_WRITE;
        g_xferBuffer[g_WrBufIdx].bytes[1] = (ui32DestAddress & 0x0000FF00) >> 8;
        g_xferBuffer[g_WrBufIdx].bytes[2] = (ui32DestAddress & 0x000000FF);

        // IAR doesn't have bcopy(pui8Source, &g_xferBuffer.bytes[3], ui32TransferSize);
        memcpy(&g_xferBuffer[g_WrBufIdx].bytes[3], pui8Source, ui32TransferSize);

        //
        // Send the 3 byte write command and the data to the IOM.
        //
        am_hal_iom_spi_write(g_sSpiIOMSettings.ui32IOMModule,
                             g_sSpiIOMSettings.ui32ChipSelect,
                             g_xferBuffer[g_WrBufIdx].words,
                             ui32TransferSize + 3, AM_HAL_IOM_RAW);
        //
        // Issue the WRDI command to reset WEL (Write Enable Latch).
        // The WREN command sets WEL.
        //
        am_hal_iom_spi_write(g_sSpiIOMSettings.ui32IOMModule,
                             g_sSpiIOMSettings.ui32ChipSelect,
                             &am_devices_spifram_wrdi,
                             1, AM_HAL_IOM_RAW);

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
am_devices_spifram_read_mode(uint8_t *pui8RxBuffer, uint32_t ui32ReadAddress,
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
    oper.peerInfo.ui32ChipSelect = g_sSpiIOMSettings.ui32ChipSelect;
    oper.pfnCallback = 0;
    oper.proto = AM_HAL_IOM_PROTO_SPI;

    while ( ui32BytesRemaining > 0)
    {
        //
        // Set the transfer size to either TRANSFERSIZE,
        // or the number of remaining bytes, whichever is smaller.
        //
        uint32_t ui32TransferSize = am_devices_spifram_max_transfersize(
            AM_DEVICES_SPIFRAM_WRITE, g_sSpiIOMSettings.ui32IOMModule,
            ui32BytesRemaining);

        //
        // READ is a 3-byte command (opcode + 16-bit address)
        //
        g_tempBuffer[g_TempBufIdx].bytes[0] = AM_DEVICES_SPIFRAM_READ;
        g_tempBuffer[g_TempBufIdx].bytes[1] = (ui32CurrentReadAddress & 0x0000FF00) >> 8;
        g_tempBuffer[g_TempBufIdx].bytes[2] = (ui32CurrentReadAddress & 0x000000FF);
        g_tempBuffer[g_TempBufIdx].bytes[3] = 0;

        oper.oper = AM_HAL_IOM_OPER_WRITE;
        oper.pui32Data = g_tempBuffer[g_TempBufIdx].words;
        oper.ui32NumBytes = 3;
        oper.ui32Options = AM_HAL_IOM_CS_LOW | AM_HAL_IOM_RAW;
        //
        // Send Read Command to SPIFRAM, Hold CS Low, then SPI Read to recieve data.
        //
        am_hal_iom_operate(g_sSpiIOMSettings.ui32IOMModule, &oper);
        g_TempBufIdx = (g_TempBufIdx + 1) % MAX_QUEUED_WRITES;

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
        am_hal_iom_operate(g_sSpiIOMSettings.ui32IOMModule, &oper);
    }
}

//
//! Prepare the command for write-enable.
//
static uint32_t g_am_devices_spifram_wren = AM_DEVICES_SPIFRAM_WREN;
//
//! Prepare the command for write-enable.
//
static uint32_t g_am_devices_spifram_wrdi = AM_DEVICES_SPIFRAM_WRDI;

//*****************************************************************************
//
// Programs the given range of fram addresses.
//
//*****************************************************************************
void
am_devices_spifram_write_mode(uint8_t *pui8TxBuffer, uint32_t ui32WriteAddress,
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
    oper.peerInfo.ui32ChipSelect = g_sSpiIOMSettings.ui32ChipSelect;
    oper.pfnCallback = 0;
    oper.proto = AM_HAL_IOM_PROTO_SPI;

    while ( ui32BytesRemaining )
    {
        //
        // Send the write-enable command to prepare the external fram for
        // program operations, and wait for the write-enable latch to be set in
        // the status register.
        //
        //
        // Send the 1 byte write command, to the spifram.
        //
        oper.oper = AM_HAL_IOM_OPER_WRITE;
        oper.pui32Data = &g_am_devices_spifram_wren;
        oper.ui32NumBytes = 1;
        oper.ui32Options = AM_HAL_IOM_RAW;
        am_hal_iom_operate(g_sSpiIOMSettings.ui32IOMModule, &oper);

        //
        // Set the transfer size to either TRANSFERSIZE,
        // or the number of remaining bytes, whichever is smaller.
        //
        uint32_t ui32TransferSize = am_devices_spifram_max_transfersize(
            AM_DEVICES_SPIFRAM_WRITE, g_sSpiIOMSettings.ui32IOMModule,
            ui32BytesRemaining);

        //
        // Set the CMD and copy the data into the same buffer.
        //
        g_xferBuffer[g_WrBufIdx].bytes[0] = AM_DEVICES_SPIFRAM_WRITE;
        g_xferBuffer[g_WrBufIdx].bytes[1] = (ui32DestAddress & 0x0000FF00) >> 8;
        g_xferBuffer[g_WrBufIdx].bytes[2] = (ui32DestAddress & 0x000000FF);

        // IAR doesn't have bcopy(pui8Source, &g_xferBuffer.bytes[3], ui32TransferSize);
        memcpy(&g_xferBuffer[g_WrBufIdx].bytes[3], pui8Source, ui32TransferSize);

        //
        // Send the 3 byte write command and the data to the IOM.
        //
        oper.oper = AM_HAL_IOM_OPER_WRITE;
        oper.pui32Data = g_xferBuffer[g_WrBufIdx].words;
        oper.ui32NumBytes = ui32TransferSize + 3;
        oper.ui32Options = AM_HAL_IOM_RAW;
        am_hal_iom_operate(g_sSpiIOMSettings.ui32IOMModule, &oper);
        g_WrBufIdx = (g_WrBufIdx + 1) % MAX_QUEUED_WRITES;
        //
        // Issue the WRDI command to reset WEL (Write Enable Latch).
        // The WREN command sets WEL.
        //
        oper.oper = AM_HAL_IOM_OPER_WRITE;
        oper.pui32Data = &g_am_devices_spifram_wrdi;
        oper.ui32NumBytes = 1;
        oper.ui32Options = AM_HAL_IOM_RAW;
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
        am_hal_iom_operate(g_sSpiIOMSettings.ui32IOMModule, &oper);
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

