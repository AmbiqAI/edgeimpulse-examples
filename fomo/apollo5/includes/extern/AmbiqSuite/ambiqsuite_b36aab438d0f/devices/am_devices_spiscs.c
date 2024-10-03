//*****************************************************************************
//
//! @file am_devices_spiscs.c
//!
//! @brief Slave-side functions for the SPISC protocol
//!
//! Slave-side functions for the SPI "switched configuration" protocol.
//!
//! @addtogroup spiscs SPISCS - SPI Switched Configuration Slave Protocol
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
#include "am_devices_spiscs.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define SIGNAL_GPIO                         31
#define FIFO_BASE                           0x78
#define FIFO_END                            0x100
#define FIFO_LENGTH                         (FIFO_END - FIFO_BASE)

#define SLAVE_BUFFER_SIZE                   512

//*****************************************************************************
//
// Buffers for the SPI Slave messages.
//
//*****************************************************************************
uint8_t g_pui8RxBuffer[SLAVE_BUFFER_SIZE];
uint32_t g_ui32BufferIndex = 0;

bool g_bDataReady = false;

//*****************************************************************************
//
// IO Slave configuration structure.
//
//*****************************************************************************
am_hal_ios_config_t g_sIOSConfig =
{
    //
    // Select SPI mode 0
    //
    .ui32InterfaceSelect = (AM_HAL_IOS_USE_SPI | AM_HAL_IOS_SPIMODE_0),

    //
    // Set up the Direct read section from 0x0 to 0x74, and use the rest for
    // FIFO space.
    //
    .ui32ROBase = FIFO_BASE,
    .ui32FIFOBase = FIFO_BASE,
    .ui32RAMBase = FIFO_END,

    //
    // Set the FIFO threshold low, so we wake up less often.
    //
    .ui32FIFOThreshold = 32
};

//*****************************************************************************
//
// Set up the necessary pins and configuration registers for 4-wire SPI
// communication between the IO Master and the IO Slave.
//
//*****************************************************************************
void
am_devices_spiscs_init(void)
{
    //
    // Configure the IOS based on the settings structure.
    //
    am_hal_ios_config(&g_sIOSConfig);

    //
    // Clear out any IOS register-access interrupts that may be active, and
    // enable interrupts for the registers we're interested in.
    //
    am_hal_ios_access_int_clear(AM_HAL_IOS_ACCESS_INT_ALL);
    am_hal_ios_access_int_enable(AM_HAL_IOS_ACCESS_INT_01);
    am_hal_ios_access_int_enable(AM_HAL_IOS_ACCESS_INT_03);

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
//
//*****************************************************************************
void
am_devices_spiscs_acc_service(uint32_t ui32Status)
{
    uint8_t ui8Size, ui8Index, ui8Continue;

    //
    // Register access of LRAM location 0x1 means we have a new packet from the
    // host.
    //
    if ( ui32Status & AM_HAL_IOS_ACCESS_INT_01 )
    {
        //
        // Read the size of the new packet.
        //
        ui8Size = am_hal_ios_pui8LRAM[0];

        //
        // Write the packet into our SRAM buffer.
        //
        for ( ui8Index = 0; ui8Index < ui8Size; ui8Index++ )
        {
            //
            // Keep track of our place in the buffer using a global index
            // variable, since we don't know yet if this is a complete packet.
            //
            g_pui8RxBuffer[g_ui32BufferIndex] = am_hal_ios_pui8LRAM[ui8Index + 4];

            g_ui32BufferIndex++;

            if ( g_ui32BufferIndex >= SLAVE_BUFFER_SIZE )
            {
                //
                // The message was too big for our buffer. We shouldn't ever
                // get here, but if we do, it probably means that the slave
                // buffer isn't big enough.
                //
                while (1);
            }
        }

        //
        // Check the continue bit. If this is zero, then the packet is
        // complete, and we can call the message handler.
        //
        ui8Continue = am_hal_ios_pui8LRAM[1];
        if ( ui8Continue == 0 )
        {
            //
            // Signal to the upper layers that data is ready.
            //
            g_bDataReady = true;
        }
        else
        {
            //
            // If the continue bit is a one, the master will be waiting on our
            // interrupt line. Set the "buffer free" status bit, and trigger
            // the interrupt line.
            //
            am_hal_ios_pui8LRAM[2] = 0x1;
#if AM_APOLLO3_GPIO
            am_hal_gpio_state_write(SIGNAL_GPIO, AM_HAL_GPIO_OUTPUT_SET);
#else // AM_APOLLO3_GPIO
            am_hal_gpio_out_bit_set(SIGNAL_GPIO);
#endif // AM_APOLLO3_GPIO
        }
    }

    //
    // Register access of LRAM location 0x3 is just a status check.
    //
    if ( ui32Status & AM_HAL_IOS_ACCESS_INT_03 )
    {
        //
        // Clear our interrupt GPIO and remove the status bits.
        //
#if AM_APOLLO3_GPIO
        am_hal_gpio_state_write(SIGNAL_GPIO, AM_HAL_GPIO_OUTPUT_CLEAR);
#else // AM_APOLLO3_GPIO
        am_hal_gpio_out_bit_clear(SIGNAL_GPIO);
#endif // AM_APOLLO3_GPIO
        am_hal_ios_pui8LRAM[2] = 0;
    }

}

//*****************************************************************************
//
//
//*****************************************************************************
void
am_devices_spiscs_write(uint8_t *pui8Data, uint8_t ui8Size)
{
    uint8_t pui8PacketHeader[2];

    pui8PacketHeader[0] = ui8Size;
    pui8PacketHeader[1] = 0;

    //
    // "Send" a packet header containing the packet size by writing it to the FIFO.
    //
    am_hal_ios_fifo_write(pui8PacketHeader, 2);

    //
    // "Send" the actual packet.
    //
    am_hal_ios_fifo_write(pui8Data, ui8Size);

    //
    // Set the status bit and trigger the GPIO to alert the master that we have a message.
    //
    am_hal_ios_pui8LRAM[2] = 0x2;
#if AM_APOLLO3_GPIO
    am_hal_gpio_state_write(SIGNAL_GPIO, AM_HAL_GPIO_OUTPUT_SET);
#else // AM_APOLLO3_GPIO
    am_hal_gpio_out_bit_set(SIGNAL_GPIO);
#endif // AM_APOLLO3_GPIO
}

//*****************************************************************************
//
//
//*****************************************************************************
uint32_t
am_devices_spiscs_read(uint8_t *pui8Data, uint32_t ui32MaxSize)
{
    uint32_t ui32Index;

    //
    // Check the buffer size to make sure that we don't overflow anything.
    //
    if ( g_ui32BufferIndex > ui32MaxSize )
    {
        //
        // Buffer wasn't big enough.
        //
        while (1);
    }

    //
    // If there's no data to grab, just return.
    //
    if ( !g_bDataReady )
    {
        return 0;
    }

    //
    // Copy the read data into the caller's buffer.
    //
    for ( ui32Index = 0; ui32Index < g_ui32BufferIndex; ui32Index++ )
    {
        pui8Data[ui32Index] = g_pui8RxBuffer[ui32Index];
    }

    //
    // Remember how long this packet was.
    //
    ui32Index = g_ui32BufferIndex;

    //
    // Reset the global buffer index, and alert the SPI master that we've cleared our buffers.
    //
    g_ui32BufferIndex = 0;
    g_bDataReady = 0;
    am_hal_ios_pui8LRAM[2] |= 0x1;
#if AM_APOLLO3_GPIO
    am_hal_gpio_state_write(SIGNAL_GPIO, AM_HAL_GPIO_OUTPUT_SET);
#else // AM_APOLLO3_GPIO
    am_hal_gpio_out_bit_set(SIGNAL_GPIO);
#endif // AM_APOLLO3_GPIO

    //
    // Return the packet length.
    //
    return ui32Index;
}

//*****************************************************************************
//
// Check to see if we've received any messages.
//
//*****************************************************************************
bool
am_devices_spiscs_check(void)
{
    return g_bDataReady;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

