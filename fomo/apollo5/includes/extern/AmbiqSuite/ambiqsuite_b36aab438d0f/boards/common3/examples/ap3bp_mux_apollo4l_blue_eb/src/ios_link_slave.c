//*****************************************************************************
//
//! @file ios_link_slave.c
//!
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
#include "am_bsp.h"
#include "am_util.h"
#include "ap3bp_mux.h"
#include "ios_link.h"

#define INFORM_HOST_PIN         3


#define TEST_IOS_XCMP_INT       1
#define AM_HAL_IOS_INT_ERR  (AM_HAL_IOS_INT_FOVFL | AM_HAL_IOS_INT_FUNDFL | AM_HAL_IOS_INT_FRDERR)
#define AM_HAL_IOS_XCMP_INT (AM_HAL_IOS_INT_XCMPWR | AM_HAL_IOS_INT_XCMPWF | AM_HAL_IOS_INT_XCMPRR | AM_HAL_IOS_INT_XCMPRF)
#define AM_IOSTEST_IOSTOHOST_DATAAVAIL_INTMASK  1
//*****************************************************************************
//
// Global message buffer for the IO master.
//
//*****************************************************************************

static void *g_pIOSHandle;
//*****************************************************************************
//
// Message buffers.
//
// data from the IOS interface, which is only 8 bits wide.
//
//*****************************************************************************

#define AM_IOS_TX_BUFSIZE_MAX   1023
uint8_t g_pui8TxFifoBuffer[AM_IOS_TX_BUFSIZE_MAX];

//*****************************************************************************
//
// I2C Slave Configuration
//
//*****************************************************************************
am_hal_ios_config_t g_sIOSI2cConfig =
{
    // Configure the IOS in I2C mode.
    .ui32InterfaceSelect = AM_HAL_IOS_USE_I2C | AM_HAL_IOS_I2C_ADDRESS(I2C_ADDR << 1),

    // Eliminate the "read-only" section, so an external host can use the
    // entire "direct write" section.
    .ui32ROBase = 0x78,

    // Set the FIFO base to the maximum value, making the "direct write"
    // section as big as possible.
    .ui32FIFOBase = 0x80,

    // We don't need any RAM space, so extend the FIFO all the way to the end
    // of the LRAM.
    .ui32RAMBase = 0x100,

    // FIFO Threshold - set to half the size
    .ui32FIFOThreshold = 0x40,

    .pui8SRAMBuffer = g_pui8TxFifoBuffer,
    .ui32SRAMBufferCap = AM_IOS_TX_BUFSIZE_MAX,
};

//*****************************************************************************
//
// Configure the SPI slave.
//
//*****************************************************************************
void ios_link_slave_init(void)
{
    // Configure I2C interface
    am_bsp_ios_pins_enable(0, AM_HAL_IOS_USE_I2C);
    //
    // Configure the IOS interface and LRAM structure.
    //
    am_hal_ios_initialize(0, &g_pIOSHandle);
    am_hal_ios_power_ctrl(g_pIOSHandle, AM_HAL_SYSCTRL_WAKE, false);
    am_hal_ios_configure(g_pIOSHandle, &g_sIOSI2cConfig);

    //
    // Clear out any IOS register-access interrupts that may be active, and
    // enable interrupts for the registers we're interested in.
    //
    am_hal_ios_interrupt_clear(g_pIOSHandle, AM_HAL_IOS_INT_ALL);
    am_hal_ios_interrupt_enable(g_pIOSHandle, AM_HAL_IOS_INT_ERR | AM_HAL_IOS_INT_FSIZE);
#ifdef TEST_IOINTCTL
    am_hal_ios_interrupt_enable(g_pIOSHandle, AM_HAL_IOS_INT_IOINTW);
#endif
#ifdef TEST_IOS_XCMP_INT
    am_hal_ios_interrupt_enable(g_pIOSHandle, AM_HAL_IOS_XCMP_INT);
#endif

    //
    // Set the bit in the NVIC to accept access interrupts from the IO Slave.
    //
    NVIC_EnableIRQ(IOSLAVE_IRQn);
    am_hal_gpio_pinconfig(INFORM_HOST_PIN, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_state_write(INFORM_HOST_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
}


// Inform host of new data available to read
void
inform_host(void)
{
    uint32_t ui32Arg = AM_IOSTEST_IOSTOHOST_DATAAVAIL_INTMASK;
    // Update FIFOCTR for host to read
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_FIFO_UPDATE_CTR, NULL);
    // Interrupt the host
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_HOST_INTSET, &ui32Arg);
    //
    am_hal_gpio_state_write(INFORM_HOST_PIN, AM_HAL_GPIO_OUTPUT_SET);
    am_util_delay_ms(10);
    am_hal_gpio_state_write(INFORM_HOST_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
}


uint32_t link_command_process(uint8_t *data)
{
    link_frame_t  *receive_frame ;
    uint32_t ios_mux_value = 0;
    uint32_t ios_mux_mode = 0;
    uint8_t ui8SendBuf[LINK_FRAME_SIZE];
    uint32_t numWritten = 0;

    receive_frame = (link_frame_t*)data;
    if ( receive_frame->frame.head != LINK_FRAME_HEAD )
    {
        return 1;
    }

    switch(receive_frame->frame.cmd)
    {
        case LINK_CMD_VERSION:
            break;
        case LINK_CMD_MUX_MODE:
            ios_mux_mode = receive_frame->frame.data[0];
            mux_mode_set((mux_mode_e)ios_mux_mode);
            if ( ios_mux_mode == MUX_MODE_IOS_LINK )
            {
                mux_mode_value_set(MUX_MODE_IOS_LINK, IOS_LINK_DEFUALT_VALUE);
            }
            break;
        case LINK_CMD_MUX_DATA:
            ios_mux_value = receive_frame->frame.data[0];
            mux_mode_value_set(MUX_MODE_IOS_LINK, ios_mux_value);
            break;
        default:
            break;
    }

    memset(ui8SendBuf, 0, LINK_FRAME_SIZE);

    ui8SendBuf[0] = LINK_FRAME_HEAD;
    ui8SendBuf[1] = receive_frame->frame.cmd;
    ui8SendBuf[2] = LINK_EOK;


    am_hal_ios_fifo_write(g_pIOSHandle, ui8SendBuf, LINK_FRAME_SIZE, &numWritten);
    inform_host();

    return 0;
}
//*****************************************************************************
//
// IO Slave Main ISR.
//
//*****************************************************************************
void
am_ioslave_ios_isr(void)
{
    uint32_t ui32Status;

    //
    // Check to see what caused this interrupt, then clear the bit from the
    // interrupt register.
    //

    am_hal_ios_interrupt_status_get(g_pIOSHandle, false, &ui32Status);

    am_hal_ios_interrupt_clear(g_pIOSHandle, ui32Status);

    if (ui32Status & AM_HAL_IOS_INT_FUNDFL)
    {
        am_util_stdio_printf("Hitting underflow for the requested IOS FIFO transfer\n");
        // We should never hit this case unless the threshold has beeen set
        // incorrect, or we are unable to handle the data rate
        // ERROR!
        am_hal_debug_assert_msg(0,
            "Hitting underflow for the requested IOS FIFO transfer.");
    }

    if (ui32Status & AM_HAL_IOS_INT_ERR)
    {
        // We should never hit this case
        // ERROR!
        am_hal_debug_assert_msg(0,
            "Hitting ERROR case.");
    }

    if (ui32Status & AM_HAL_IOS_INT_FSIZE)
    {
        //
        // Service the I2C slave FIFO if necessary.
        //
        am_hal_ios_interrupt_service(g_pIOSHandle, ui32Status);
    }

    if (ui32Status & AM_HAL_IOS_INT_XCMPWR)
    {
        //
        // Set up a pointer for writing 32-bit aligned packets through the IO slave
        // interface.
        //
      link_command_process((uint8_t *)am_hal_ios_pui8LRAM);

    }
}

//uint32_t
//ios_receive(uint8_t *pui8Pointer)
//{
//    uint32_t ui32Status = INVALID_STATUS;
//    memcpy(g_pui8TempBuf,pui8Pointer,MAX_CMD_SIZE);
//    return ui32Status;
//}




//uint32_t ios_link_slave_write(const  uint32_t *pBuf, uint32_t size)

