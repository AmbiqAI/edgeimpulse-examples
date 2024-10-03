//*****************************************************************************
//
//! @file am_devices_i2cioshc.c
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

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_util_ring_buffer.h"
#include "am_devices_i2cioshc.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define SIGNAL_GPIO                         4
#define SLAVE_BUFFER_SIZE                   128

// ##### INTERNAL BEGIN #####
//*****************************************************************************
//
//! IO Slave configuration structure.
//
//*****************************************************************************
/*=====> put something like this in the user's program
am_hal_ios_config_t main_sIOSConfig =
{
    //
    //! Select SPI mode 0
    //
    .ui32InterfaceSelect = (  AM_HAL_IOS_USE_I2C
                           // | AM_HAL_IOS_LSB_FIRST
                            | AM_HAL_IOS_I2C_ADDRESS(0x4A)
                            ),

    //
    //! Set up the Direct read section from 0x0 to 0x74, and use the rest for
    //! FIFO space.
    //
    .ui32ROBase = FIFO_BASE,
    .ui32FIFOBase = FIFO_BASE,
    .ui32RAMBase = FIFO_END,

    //
    //! Set the FIFO threshold low, so we wake up less often.
    //
    .ui32FIFOThreshold = 32
};
                   <==== */
// ##### INTERNAL END #####
//*****************************************************************************
//
// Initialize the I/O Slave and get ready to commune with the hosts.
//
//*****************************************************************************
void
am_devices_i2cioshc_init(am_hal_ios_config_t *psIOSConfig)
{
    //
    // Configure the IOS based on the settings structure.
    //
    am_hal_ios_config(psIOSConfig);

    //
    // Clear out any IOS register-access interrupts that may be active, and
    // enable interrupts for the registers we're interested in.
    //
    am_hal_ios_access_int_clear(AM_HAL_IOS_ACCESS_INT_ALL);
    am_hal_ios_access_int_enable(AM_HAL_IOS_ACCESS_INT_4F);

    am_hal_ios_int_clear(AM_HAL_IOS_INT_ALL);
    am_hal_ios_int_enable(AM_HAL_IOS_INT_FSIZE);

    //
    // Set the bit in the NVIC to accept access interrupts from the IO Slave.
    //
#if AM_CMSIS_REGS
#ifdef AM_IRQ_PRIORITY_DEFAULT
    NVIC_SetPriority(IOSLAVEACC_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_SetPriority(IOSLAVE_IRQn,    AM_IRQ_PRIORITY_DEFAULT);
#endif // AM_IRQ_PRIORITY_DEFAULT
    NVIC_EnableIRQ(IOSLAVEACC_IRQn);
    NVIC_EnableIRQ(IOSLAVE_IRQn);
#else // AM_CMSIS_REGS
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_IOSACC);
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_IOSLAVE);
#endif // AM_CMSIS_REGS
}

//*****************************************************************************
//
//  Call this function from their I/O Slave Access ISR
//
//*****************************************************************************
void
am_devices_i2cioshc_acc_service(uint8_t *pui8Destination, uint32_t ui32Status)
{
    uint8_t ui8Size, ui8Index, ui8BufferIndex;

    //
    // Register access of LRAM location 0x4F means we have a new packet from the
    // host. These packets are variable sized and assumed to end at byte 0x4F.
    // This means their starting point for the first byte moves relative to
    // to the fixed location 0x4F. The host always writes the length of the packet
    // into 0x4F as its last write.
    //
    if (ui32Status & AM_HAL_IOS_ACCESS_INT_4F)
    {
        //
        // Read the size of the new packet.
        //
        ui8Size = am_hal_ios_pui8LRAM[0x4F];

        //
        // Find variable length packet starting address.
        //
        ui8Index = 0x4F - ui8Size - 1 + 1;

        //
        // Always copy to the beginning of their buffer.
        //
        ui8BufferIndex = 0;

        //
        // Copy the packet into our SRAM buffer.
        //
        for ( ; ui8Index < 0x4F; ui8Index++)
        {
            //
            // Keep track of our place in the buffer using a global index
            // variable, since we don't know yet if this is a complete packet.
            //
            pui8Destination[ui8BufferIndex++] = am_hal_ios_pui8LRAM[ui8Index];
        }
    }
}

//*****************************************************************************
//
//  Call this function from release the inbound hardware buffer once it is
// completely free and no longer needed.
//
//*****************************************************************************
void
am_devices_i2cioshc_release_inbound_hw_buffer(void)
{
    //
    // Set SWINT1 interrupt back to the host so it knows we are finished
    // with this buffer in the IOS LRAM.
    //
    am_hal_ios_host_int_set(0x02);
}

//*****************************************************************************
//
//  Call this function from their I/O Slave IOS Interrupt ISR
//
//*****************************************************************************
void
am_devices_i2cioshc_ios_int_service(void (*XmitMsgHandler)(void),
                                    uint32_t ui32Status)
{
    //
    // Call the message handler to do any application clean up activities.
    //
    if (XmitMsgHandler)
    {
        XmitMsgHandler();
    }
}

//*****************************************************************************
//
//  Call this function to format an SHTP message.
//
//*****************************************************************************
void
am_devices_i2cioshc_message_format(am_devices_i2cioshc_message_t *psMessage,
                                   uint8_t *pBuffer)
{
    uint8_t *pui8SrcPtr, *pui8DstPtr;
    uint32_t ui32I;

    //
    // Insert Length in to Header.
    //
    pBuffer[0] = (psMessage->Length >> 0);
    pBuffer[1] = (psMessage->Length >> 8);

    if (psMessage->Length > 60)
    {
        while (1);
    }

    //
    // Insert Channel Number in to Header.
    //
    pBuffer[2] = psMessage->ChannelNumber;

    //
    // Insert Sequence Number in to Header.
    //
    pBuffer[3] = psMessage->SequenceNumber;

    //
    // Copy the payload in to the target message string.
    //
    pui8SrcPtr = psMessage->Message;
    pui8DstPtr = &pBuffer[4];
    for (ui32I = 0; ui32I < psMessage->Length; ui32I++)
    {
        *pui8DstPtr ++ = *pui8SrcPtr ++;
    }
}

//*****************************************************************************
//
//  Call this function to extract an SHTP message.
//
//*****************************************************************************
void
am_devices_i2cioshc_message_extract(am_devices_i2cioshc_message_t *psMessage,
                                    uint8_t *pBuffer)
{
    uint8_t *pui8SrcPtr, *pui8DstPtr;
    uint32_t ui32I;

    //
    // Extract Length from header.
    //
    psMessage->Length  = (uint32_t)pBuffer[1] << 8;
    psMessage->Length |= pBuffer[0];

    //
    // Extract Channel Number.
    //
    psMessage->ChannelNumber = pBuffer[2];

    //
    // Extract Sequence Number.
    //
    psMessage->SequenceNumber = pBuffer[3];

    //
    // Copy the payload in to the target message string.
    //
    pui8SrcPtr = &pBuffer[4];
    pui8DstPtr = psMessage->Message;
    for (ui32I = 0; ui32I < (psMessage->Length-4); ui32I++)
    {
        *pui8DstPtr ++ = *pui8SrcPtr ++;
    }
}

//*****************************************************************************
//
//  Call this function to send a message to the host.
//
//*****************************************************************************
void
am_devices_i2cioshc_write(uint8_t *pui8Data, uint8_t ui8Size)
{
//##### INTERNAL BEGIN #####
    //
    // TODO: Initialize the FIFO in preparation for sending the message
    //
//##### INTERNAL END #####
    //
    // "Send" the actual packet.
    //
    am_hal_ios_fifo_write_simple(pui8Data, ui8Size);

    //
    // Set SWINT0 in the host interrupt status register to alert the host.
    //
    am_hal_ios_host_int_set(0x01);
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

