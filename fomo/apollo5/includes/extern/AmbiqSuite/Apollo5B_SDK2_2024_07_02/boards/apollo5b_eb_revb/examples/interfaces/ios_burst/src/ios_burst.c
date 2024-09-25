//*****************************************************************************
//
//! @file ios_burst.c
//!
//! @brief Example used for demonstrating the ios burst mode.
//!
//! Purpose: This slave component runs on one EB and is used in conjunction with
//! the companion host example, which runs on a second board.
//!
//! In this pair of examples, IOM and IOS are switched as below for each mode:
//! Host burst write : IOM burst write - IOS LRAM burst read
//! (Host writes unlimited size of data to slave)
//! Host read : IOM FIFO read - IOS FIFO write
//! (Host reads data from slave max. 1023 bytes)
//!
//! Printing takes place over the ITM at 1M Baud.
//!
//! Additional Information:
//! In order to run this example, a host device (e.g. a second board) must be set
//! up to run the host example, ios_burst_host.  The two boards can be connected
//! using fly leads as follow.
//!
//! @verbatim
//! Pin connections for the I/O Master board to the I/O Slave board.
//! SPI:
//!     HOST (ios_burst_host - Apollo5 EB)    SLAVE (ios_burst - Apollo5 EB)
//!     --------------------                    ----------------
//!     GPIO[8]  IOM1 SPI SCK                   GPIO[0]  IOS SPI SCK
//!     GPIO[9]  IOM1 SPI MOSI                  GPIO[1]  IOS SPI MOSI
//!     GPIO[10] IOM1 SPI MISO                  GPIO[2]  IOS SPI MISO
//!     GPIO[92] IOM1 SPI nCE                   GPIO[3]  IOS SPI nCE
//!     GPIO[4]  REQ_ACK (slave to host)        GPIO[4]  IOS SPI INT
//!     GND                                     GND
//! @endverbatim
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "ios_burst.h"

static void *g_pIOSHandle;

AM_IOS_STATE_E g_iosState = IOS_STATE_IDLE;

volatile bool bIosRxComplete = false;
volatile bool bIosRegAcc01 = false;
volatile bool bIosRegAccThre1 = false;
volatile bool bIosRegAccThre2 = false;

volatile bool g_bSensor0Data = false;
volatile uint32_t g_sendIdx = 0;

uint16_t g_ui16RecvBufIdx = 0;
uint16_t g_ui16LRamReadPtr = 0;
uint16_t g_ui16IosRxLength = 0;

uint16_t g_ui16TestCnt = 0;
uint16_t g_ui16TestPassCnt = 0;

//*****************************************************************************
//
// Message buffers.
//
// data from the IOS interface, which is only 8 bits wide.
//
//*****************************************************************************
uint8_t g_pIosRecvBuf[AM_IOM_TX_UNIT_SIZE];

#define AM_TEST_REF_BUF_SIZE    (SENSOR0_DATA_SIZE * 2)
uint8_t g_pui8TestBuf[AM_TEST_REF_BUF_SIZE];

#define AM_IOS_TX_BUFSIZE_MAX   1023
uint8_t g_pui8TxFifoBuffer[AM_IOS_TX_BUFSIZE_MAX];

static const struct
{
    const uint32_t MHz;
    const char *MHzString;
} spiSpeedString[] =
{
    {AM_HAL_IOM_10KHZ,  "10KHZ"},
    {AM_HAL_IOM_50KHZ,  "50KHZ"},
    {AM_HAL_IOM_100KHZ, "100KHZ"},
    {AM_HAL_IOM_250KHZ, "250KHZ"},
    {AM_HAL_IOM_375KHZ, "375KHZ"},
    {AM_HAL_IOM_400KHZ, "400KHZ"},
    {AM_HAL_IOM_500KHZ, "500KHZ"},
    {AM_HAL_IOM_750KHZ, "750KHZ"},
    {AM_HAL_IOM_1MHZ,   "1MHZ"},
    {AM_HAL_IOM_1_5MHZ, "1.5MHZ"},
    {AM_HAL_IOM_2MHZ,   "2MHZ"},
    {AM_HAL_IOM_3MHZ,   "3MHZ"},
    {AM_HAL_IOM_4MHZ,   "4MHZ"},
    {AM_HAL_IOM_6MHZ,   "6MHZ"},
    {AM_HAL_IOM_8MHZ,   "8MHZ"},
    {AM_HAL_IOM_12MHZ,  "12MHZ"},
    {AM_HAL_IOM_16MHZ,  "16Mhz"},
    {AM_HAL_IOM_24MHZ,  "24Mhz"},
};

#define IOM_SPEED_SPI           11  // 3 MHz

//*****************************************************************************
//
// SPI Slave Configuration
//
//*****************************************************************************
static am_hal_ios_config_t g_sIosConfig =
{
    // Configure the IOS in SPI mode.
    .ui32InterfaceSelect = AM_HAL_IOS_USE_SPI,

    // Eliminate the "read-only" section, so an external host can use the
    // entire "direct write" section.
#if (IOS_WRAP_START_ADDRESS == 0x0)
    .ui32ROBase = IOS_WRAP_START_ADDRESS,
#elif (IOS_WRAP_START_ADDRESS == 0x78)
    .ui32ROBase = IOS_WRAP_START_ADDRESS,
#else
    .ui32ROBase = IOS_WRAP_START_ADDRESS - 0x8,
#endif

    // Making the "FIFO" section as big as possible.
#if (IOS_WRAP_START_ADDRESS == 0x78)
    .ui32FIFOBase = IOS_WRAP_START_ADDRESS + 0x8,
#else
    .ui32FIFOBase = IOS_WRAP_START_ADDRESS,
#endif

    // We don't need any RAM space, so extend the FIFO all the way to the end
    // of the LRAM.
    .ui32RAMBase = (IOS_WRAPAROUND_MAX_LRAM_SIZE + 0x8),         // FIFO MAX?

    // FIFO Threshold - set to half the size
    .ui32FIFOThreshold = IOS_FIFO_THRESHOLD,

    .pui8SRAMBuffer = g_pui8TxFifoBuffer,
    .ui32SRAMBufferCap = AM_IOS_TX_BUFSIZE_MAX,

    // Direct access wraparound enable.
    .ui8WrapEnabled = 1,
};

#if SLINT_GPIO
const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_OUTPUT =
{
    .GP.cfg_b.uFuncSel            = AM_HAL_PIN_4_GPIO,
    .GP.cfg_b.eDriveStrength      = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
    .GP.cfg_b.eCEpol              = AM_HAL_GPIO_PIN_CEPOL_ACTIVEHIGH,
    .GP.cfg_b.eGPOutCfg           = AM_HAL_GPIO_PIN_OUTCFG_OPENDRAIN,
    .GP.cfg_b.ePullup             = AM_HAL_GPIO_PIN_PULLUP_24K,
};
#else
const am_hal_gpio_pincfg_t g_AM_BSP_SLINT =
{
    .GP.cfg_b.uFuncSel            = SLINT_FUN_SEL,
    .GP.cfg_b.eDriveStrength      = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
    .GP.cfg_b.eCEpol              = AM_HAL_GPIO_PIN_CEPOL_ACTIVEHIGH,
    .GP.cfg_b.eGPOutCfg           = AM_HAL_GPIO_PIN_OUTCFG_OPENDRAIN,
};
#endif

void ios_clean_up(uint8_t ui8RwFlag);

//*****************************************************************************
//
// Interrupt handler for timer0 to emulate sensor0 new data.
//
//*****************************************************************************
void timer0_handler(void)
{

    // Inform main loop of sensor 0 Data availability
    g_bSensor0Data = true;
}

//*****************************************************************************
//
// Timer0 Interrupt Service Routine (ISR)
//
//*****************************************************************************
void am_timer00_isr(void)
{
    //
    // Clear Timer0 Interrupt (write to clear).
    //
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE0));
    am_hal_timer_clear(0);

    timer0_handler();
}

void stop_sensors(void)
{
    //
    // Stop timer 0
    //
    am_hal_timer_stop(0);
}

void start_sensors(void)
{
    stop_sensors(); // Just in case host died without sending STOP last time
    // Initialize Data Buffer Index
    g_sendIdx = 0;
    //
    // Start timer 0
    //
    am_hal_timer_start(0);
    g_iosState = IOS_STATE_HOST_READ_NODATA;
}

void init_sensors(void)
{
    //
    // Set up timer 0.
    //
    am_hal_timer_config_t       Timer0Config;
    am_hal_timer_default_config_set(&Timer0Config);
    Timer0Config.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV4K;   // 96MHz/4K = 24KHz
    Timer0Config.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    Timer0Config.ui32Compare0 = 24000 / SENSOR0_FREQ ;          // Sensor 0 Freq

    am_hal_timer_config(0, &Timer0Config);
    am_hal_timer_clear_stop(0);

    //
    // Clear the timer Interrupt
    //
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE0));

    //
    // Enable the timer Interrupt.
    //
    am_hal_timer_interrupt_enable(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE0));

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_SetPriority(TIMER0_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(TIMER0_IRQn);
    am_hal_interrupt_master_enable();
}

// Inform host of new data available to read
void inform_host(void)
{
    uint32_t ui32Arg = AM_IOSTEST_IOSTOHOST_DATAAVAIL_INTMASK;
    // Update FIFOCTR for host to read
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_FIFO_UPDATE_CTR, NULL);
    // Interrupt the host
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_HOST_INTSET, &ui32Arg);
#if SLINT_GPIO
    am_hal_gpio_state_write(HANDSHAKE_PIN, AM_HAL_GPIO_OUTPUT_SET);
#endif // #if SLINT_GPIO
}

//*****************************************************************************
//
// IO Slave Main ISR.
//
//*****************************************************************************
void ios_isr(void)
{
    uint32_t ui32Status;
    uint32_t ui32UsedSpace = 0;
    uint32_t ui32data;
    //
    // Check to see what caused this interrupt, then clear the bit from the
    // interrupt register.
    //

    am_hal_ios_interrupt_status_get(g_pIOSHandle, false, &ui32Status);

    am_hal_ios_interrupt_clear(g_pIOSHandle, ui32Status);

#if SLINT_GPIO
    am_hal_gpio_state_write(HANDSHAKE_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);   // clear SLINT anyway
#endif

    if (ui32Status & AM_HAL_IOS_INT_ERR)
    {
        // We should never hit this case
        // ERROR!
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
        if ( g_iosState == IOS_STATE_IDLE )
        {
            uint8_t *pui8Packet = (uint8_t *) lram_array;
            switch(pui8Packet[0])
            {
                case HANDSHAKE_CMD_WRITE_DATA:
                    ui32data = AM_HAL_IOS_ACCESS_INT_01;
                    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTEN, &ui32data);
                    pui8Packet[0] = HANDSHAKE_CMD_ACK;
                    g_iosState = IOS_STATE_HOST_WRITE_START;
                    am_util_stdio_printf("Start Host Write\n");
                    break;

                case HANDSHAKE_CMD_READ_DATA:
                    // Set up the IOSINT interrupt pin
#if SLINT_GPIO
                    am_hal_gpio_pinconfig(HANDSHAKE_PIN, g_AM_BSP_GPIO_OUTPUT);
#else
                    am_hal_gpio_pinconfig(HANDSHAKE_PIN, g_AM_BSP_SLINT);
#endif
                    am_hal_gpio_state_write(HANDSHAKE_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
                    init_sensors();
                    g_iosState = IOS_STATE_HOST_READ_NODATA;
                    start_sensors();
                    am_util_stdio_printf("Start Host Read\n");
                    break;

                default:
                    break;
            }
        }
        //Transfer to register space complete interrupt.
        else if ( (g_iosState == IOS_STATE_HOST_WRITE_START) || (g_iosState == IOS_STATE_HOST_WRITE_STREAM) )
        {
            bIosRxComplete = true;
        }
        else if ((g_iosState == IOS_STATE_HOST_READ_DATA) || (g_iosState == IOS_STATE_HOST_READ_NODATA))
        {
            //
            // Set up a pointer for writing 32-bit aligned packets through the IO slave
            // interface.
            //
            uint8_t *pui8Packet = (uint8_t *) lram_array;
            switch(pui8Packet[0])
            {
                case HANDSHAKE_CMD_STOP:
                    // Host no longer interested in data from us
                    // Stop the Sensor emulation
                    stop_sensors();
                    ios_clean_up(IOS_MODE_HOST_READ);
                    g_iosState = IOS_STATE_IDLE;
                    am_util_stdio_printf("Stop FIFO Data\n");
                    break;

                case HANDSHAKE_CMD_ACK:
                    // Host done reading the last block signalled
                    // Check if any more data available
                    am_hal_ios_fifo_space_used(g_pIOSHandle, &ui32UsedSpace);
                    if (ui32UsedSpace)
                    {
                        g_iosState = IOS_STATE_HOST_READ_DATA;
                        inform_host();
                    }
                    else
                    {
                        g_iosState = IOS_STATE_HOST_READ_NODATA;
                    }
                    break;

                default:
                    break;
            }
        }
    }
}

//*****************************************************************************
//
// IO Slave Access ISR.
//
//*****************************************************************************
void ios_acc_isr(void)
{
    uint32_t ui32Status;

    //
    // Check to see what caused this interrupt, then clear the bit from the
    // interrupt register.
    //
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTGET, &ui32Status);
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTCLR, &ui32Status);

    if (ui32Status & AM_HAL_IOS_ACCESS_INT_01)
    {
        bIosRegAcc01 = true;
        // Read packet length
        // change state to Accumulate data
    }

    if (ui32Status & IOS_REGACC_THRESHOLD_01)
    {
        bIosRegAccThre1 = true;
        // Read 1st half data from LRAM.
    }

    if (ui32Status & IOS_REGACC_THRESHOLD_02)
    {
        bIosRegAccThre2 = true;
        // Read 2nd half data from LRAM.
    }
}

//*****************************************************************************
//
// Configure the SPI slave.
//
//*****************************************************************************
static void ios_set_up(uint8_t ui8RwFlag)
{
    // Configure SPI interface
    am_bsp_ios_pins_enable(TEST_IOS_MODULE, AM_HAL_IOS_USE_SPI);

    //
    // Configure the IOS interface and LRAM structure.
    //
    am_hal_ios_initialize(TEST_IOS_MODULE, &g_pIOSHandle);
    am_hal_ios_power_ctrl(g_pIOSHandle, AM_HAL_SYSCTRL_WAKE, false);

    am_hal_ios_configure(g_pIOSHandle, &g_sIosConfig);

    //
    // Clear out any IOS register-access interrupts that may be active, and
    // enable interrupts for the registers we're interested in.
    //
    am_hal_ios_interrupt_clear(g_pIOSHandle, AM_HAL_IOS_INT_ALL);
    am_hal_ios_interrupt_enable(g_pIOSHandle, AM_HAL_IOS_INT_ERR | AM_HAL_IOS_INT_FSIZE);

#ifdef TEST_IOS_XCMP_INT
    am_hal_ios_interrupt_enable(g_pIOSHandle, AM_HAL_IOS_XCMP_INT);
#endif

    //
    // Set the bit in the NVIC to accept access interrupts from the IO Slave.
    //
    NVIC_EnableIRQ(IOS_IRQ);

    if ( ui8RwFlag == IOS_MODE_HOST_WRITE )
    {
        uint32_t ui32Status = AM_HAL_IOS_ACCESS_INT_01;
        am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTEN, &ui32Status);

        NVIC_EnableIRQ(IOS_ACC_IRQ);
        // Initialize RO & AHB-RAM data with pattern
        for (uint8_t i = 0; i < 8; i++)
        {
            lram_array[0x78 + i] = ROBUFFER_INIT;
        }
    }
}

//*****************************************************************************
//
// Clean up IO Slave
//
//*****************************************************************************
void ios_clean_up(uint8_t ui8RwFlag)
{
    uint32_t ui32Arg = AM_HAL_IOS_ACCESS_INT_ALL;

    // Configure SPI interface
    am_bsp_ios_pins_disable(TEST_IOS_MODULE, AM_HAL_IOS_USE_SPI);

    if ( ui8RwFlag == IOS_MODE_HOST_WRITE )
    {
        //
        // Clear out any IOS register-access interrupts that may be active, and
        // enable interrupts for the registers we're interested in.
        //
        am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTDIS, &ui32Arg);
        NVIC_DisableIRQ(IOS_ACC_IRQ);
    }

    am_hal_ios_interrupt_disable(g_pIOSHandle, AM_HAL_IOS_INT_ALL);
    NVIC_DisableIRQ(IOS_IRQ);

    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_FIFO_BUF_CLR, NULL);
    am_hal_ios_uninitialize(g_pIOSHandle);
    am_hal_pwrctrl_periph_disable((am_hal_pwrctrl_periph_e)(IOS_PWRCTRL));
}

#if IOM_IOS_CHECKSUM
static uint8_t get_checksum(uint8_t* pData, uint8_t len)
{
    uint32_t checksum = 0;
    for ( uint8_t i = 0; i < len; i++ )
    {
        checksum += pData[i];
    }

    return (uint8_t)(checksum & 0xff);
}
#endif

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int
main(void)
{
    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    //
    // Enable the ITM print interface.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

    am_util_stdio_printf("IOS Burst Example\n");
    am_util_stdio_printf("The pin jumpers are as follows:\n\n");
    am_util_stdio_printf("SPI:        Slave                   Master           \n\n");
    am_util_stdio_printf("Pins  :     IOS (Apollo5_eb)        IOM1 (Apollo5_eb)\n");
    am_util_stdio_printf("%d to %d :  IOS SPI SCK             to  IOM1 SPI SCK\n", AM_BSP_GPIO_IOS_SCK, AM_BSP_GPIO_IOM1_SCK);
    am_util_stdio_printf("%d to %d :  IOS SPI MOSI            to  IOM1 SPI MOSI\n", AM_BSP_GPIO_IOS_MOSI, AM_BSP_GPIO_IOM1_MOSI);
    am_util_stdio_printf("%d to %d :  IOS SPI MISO            to  IOM1 SPI MOSI\n", AM_BSP_GPIO_IOS_MISO, AM_BSP_GPIO_IOM1_MISO);
    am_util_stdio_printf("%d to %d :  IOS SPI nCE             to  IOM1 SPI nCE\n", AM_BSP_GPIO_IOS_CE, AM_BSP_GPIO_IOM1_CS);
    am_util_stdio_printf("%d to 4  :  IOS SPI INT             to  HOST gpio interrupt\n", HANDSHAKE_PIN);

#if IOM_IOS_CHECKSUM
    uint8_t ui8CheckSum = 0;
#endif

    uint32_t ui32Status;
    bool bTestPass = true;

    uint32_t numWritten = 0;
    uint32_t numWritten1 = 0;
    uint32_t chunk1;
    uint32_t ui32UsedSpace = 0;

    // Initialize Test Data
    for (int i = 0; i < AM_TEST_REF_BUF_SIZE; i++)
    {
        g_pui8TestBuf[i] = (i & 0xFF) ^ XOR_BYTE;
    }

    ios_set_up(IOS_MODE_HOST_WRITE);
    lram_array[0] = HANDSHAKE_CMD_ACK;

    //
    // Enable interrupts so we can receive messages from the boot host.
    //
    am_hal_interrupt_master_enable();

    while(1)
    {
        switch(g_iosState)
        {
            case IOS_STATE_HOST_WRITE_START:
                // Read size of length and change state to IOS_STATE_STREAM
                if ( bIosRegAcc01 )
                {
                    bIosRegAcc01 = false;
                    g_ui16IosRxLength = *((uint16_t *)lram_array);
#if IOS_DBG_MSG
                    am_util_stdio_printf("Len = 0x%x\n", g_ui16IosRxLength);
#endif
                    g_iosState = IOS_STATE_HOST_WRITE_STREAM;
                    g_ui16LRamReadPtr = 2;

                    ui32Status = AM_HAL_IOS_ACCESS_INT_01;
                    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTDIS, &ui32Status);

                    ui32Status = IOS_REGACC_THRESHOLD_01 | IOS_REGACC_THRESHOLD_02;
                    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTEN, &ui32Status);
                }
                break;

            case IOS_STATE_HOST_WRITE_STREAM:
                // Process the data wrote from the host
                if ( bIosRegAccThre1 )
                {
                    bIosRegAccThre1 = false;
                    if ( g_ui16RecvBufIdx == 0 )
                    {
                        if ( IOS_WRAP_START_ADDRESS > g_ui16LRamReadPtr )
                        {
                            ios_memcpy(&g_pIosRecvBuf[g_ui16RecvBufIdx], (uint8_t*)&lram_array[g_ui16LRamReadPtr], IOS_WRAP_START_ADDRESS - g_ui16LRamReadPtr);
                            g_ui16RecvBufIdx = g_ui16RecvBufIdx + IOS_WRAP_START_ADDRESS - g_ui16LRamReadPtr;
                            g_ui16LRamReadPtr = IOS_WRAP_START_ADDRESS;
                        }
                    }
                    else if ( (g_ui16RecvBufIdx + (IOS_WRAPAROUND_MAX_LRAM_SIZE - IOS_WRAP_HALF_ADDRESS)) < g_ui16IosRxLength )
                    {
                        ios_memcpy(&g_pIosRecvBuf[g_ui16RecvBufIdx], (uint8_t*)&lram_array[g_ui16LRamReadPtr], (IOS_WRAPAROUND_MAX_LRAM_SIZE - IOS_WRAP_HALF_ADDRESS));
                        g_ui16RecvBufIdx = g_ui16RecvBufIdx + (IOS_WRAPAROUND_MAX_LRAM_SIZE - IOS_WRAP_HALF_ADDRESS);
                        g_ui16LRamReadPtr = IOS_WRAP_START_ADDRESS;
                    }
#if IOS_DBG_MSG
                    am_util_stdio_printf("1 Idx=%d LIdx=%d\n", g_ui16RecvBufIdx, g_ui16LRamReadPtr);
#endif
                }
                else if ( bIosRegAccThre2 )
                {
                    bIosRegAccThre2 = false;
                    if ( (g_ui16RecvBufIdx + IOS_WRAP_HALF_ADDRESS - g_ui16LRamReadPtr) < g_ui16IosRxLength )
                    {
                        if ( g_ui16RecvBufIdx == 0 )
                        {
                            ios_memcpy(&g_pIosRecvBuf[g_ui16RecvBufIdx], (uint8_t*)&lram_array[g_ui16LRamReadPtr], IOS_WRAP_HALF_ADDRESS - g_ui16LRamReadPtr);
                            g_ui16RecvBufIdx = g_ui16RecvBufIdx + IOS_WRAP_HALF_ADDRESS - g_ui16LRamReadPtr;
                            g_ui16LRamReadPtr = IOS_WRAP_HALF_ADDRESS;
                        }
                        else
                        {
                            ios_memcpy(&g_pIosRecvBuf[g_ui16RecvBufIdx], (uint8_t*)&lram_array[g_ui16LRamReadPtr], (IOS_WRAP_HALF_ADDRESS - IOS_WRAP_START_ADDRESS));
                            g_ui16RecvBufIdx += (IOS_WRAP_HALF_ADDRESS - IOS_WRAP_START_ADDRESS);
                            g_ui16LRamReadPtr = IOS_WRAP_HALF_ADDRESS;
                        }
                    }
#if IOS_DBG_MSG
                    am_util_stdio_printf("2 Idx=%d LIdx=%d\n", g_ui16RecvBufIdx, g_ui16LRamReadPtr);
#endif
                }
                else if ( bIosRxComplete )
                {
                    bIosRxComplete = false;
                    if ( (IOS_WRAPAROUND_MAX_LRAM_SIZE - g_ui16LRamReadPtr) >= (g_ui16IosRxLength - g_ui16RecvBufIdx) )
                    {
                        ios_memcpy(&g_pIosRecvBuf[g_ui16RecvBufIdx], (uint8_t*)&lram_array[g_ui16LRamReadPtr], g_ui16IosRxLength - g_ui16RecvBufIdx);
                        g_ui16LRamReadPtr += (g_ui16IosRxLength - g_ui16RecvBufIdx);
                    }
                    else
                    {
                        ios_memcpy(&g_pIosRecvBuf[g_ui16RecvBufIdx], (uint8_t*)&lram_array[g_ui16LRamReadPtr], IOS_WRAPAROUND_MAX_LRAM_SIZE - g_ui16LRamReadPtr);
                        g_ui16RecvBufIdx = g_ui16RecvBufIdx + (IOS_WRAPAROUND_MAX_LRAM_SIZE - g_ui16LRamReadPtr);
                        g_ui16LRamReadPtr = IOS_WRAP_START_ADDRESS;
                        ios_memcpy(&g_pIosRecvBuf[g_ui16RecvBufIdx], (uint8_t*)&lram_array[g_ui16LRamReadPtr], g_ui16IosRxLength - g_ui16RecvBufIdx);
                        g_ui16LRamReadPtr += (g_ui16IosRxLength - g_ui16RecvBufIdx);
                    }
                    g_ui16RecvBufIdx = g_ui16IosRxLength;
#if IOS_DBG_MSG
                    am_util_stdio_printf("3 Idx=%d LIdx=%d\n", g_ui16RecvBufIdx, g_ui16LRamReadPtr);
#endif
                    g_ui16RecvBufIdx = g_ui16LRamReadPtr = 0;

#if IOM_IOS_CHECKSUM
                    ui8CheckSum = get_checksum(g_pIosRecvBuf, g_ui16IosRxLength - 1);
                    g_ui16TestCnt++;
                    if ( ui8CheckSum != g_pIosRecvBuf[g_ui16IosRxLength - 1] )
                    {
                        am_util_stdio_printf("Chk = 0x%x Buf[] = 0x%x\n", ui8CheckSum, g_pIosRecvBuf[g_ui16IosRxLength - 1]);
                        bTestPass = false;
                    }
                    else
                    {
                        g_ui16TestPassCnt++;
#if IOS_DBG_MSG
                        am_util_stdio_printf("R %d\n", g_ui16IosRxLength);
#endif
                    }
#endif
                    ui32Status = AM_HAL_IOS_ACCESS_INT_01 | IOS_REGACC_THRESHOLD_01 | IOS_REGACC_THRESHOLD_02;
                    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTDIS, &ui32Status);
                    am_util_stdio_printf(".");
                    g_iosState = IOS_STATE_IDLE;
                    if ( (g_ui16TestCnt % IOS_TEST_ITERATION) == 0 )
                    {
                        am_util_stdio_printf("%s %d/%d passed.\n", spiSpeedString[IOM_SPEED_SPI].MHzString, g_ui16TestPassCnt, IOS_TEST_ITERATION);
                        g_ui16TestPassCnt = 0;
                    }
                    if ( g_ui16TestCnt == IOS_TEST_ITERATION )
                    {
                        if ( bTestPass )
                        {
                            g_ui16TestCnt = 0;
                            am_util_stdio_printf("IOS LRAM Receive Test Passed!\n\n");
                        }
                        else
                        {
                            am_util_stdio_printf("IOS LRAM Receive Test Failed!\n\n");
                        }
                        ios_clean_up(IOS_MODE_HOST_WRITE);
                        ios_set_up(IOS_MODE_HOST_READ);
                    }
                    lram_array[0] = HANDSHAKE_CMD_ACK;
                }
                break;

            case IOS_STATE_HOST_READ_NODATA:
            case IOS_STATE_HOST_READ_DATA:
                // Received read req, and prepare data for the host
                if (g_bSensor0Data)
                {
                    chunk1 = AM_TEST_REF_BUF_SIZE - g_sendIdx;
                    if (chunk1 > SENSOR0_DATA_SIZE)
                    {
                        am_hal_ios_fifo_write(g_pIOSHandle, &g_pui8TestBuf[g_sendIdx], SENSOR0_DATA_SIZE, &numWritten);
                    }
                    else
                    {
                        am_hal_ios_fifo_write(g_pIOSHandle, &g_pui8TestBuf[g_sendIdx], chunk1, &numWritten);
                        if (numWritten == chunk1)
                        {
                            am_hal_ios_fifo_write(g_pIOSHandle, &g_pui8TestBuf[0], SENSOR0_DATA_SIZE - chunk1, &numWritten1);
                            numWritten += numWritten1;
                        }
                    }

                    g_sendIdx += numWritten;
                    g_sendIdx %= AM_TEST_REF_BUF_SIZE;
                    g_bSensor0Data = false;
                }

                // If we were Idle - need to inform Host if there is new data
                if (g_iosState == IOS_STATE_HOST_READ_NODATA)
                {
                    am_hal_ios_fifo_space_used(g_pIOSHandle, &ui32UsedSpace);
                    if (ui32UsedSpace)
                    {
                        g_iosState = IOS_STATE_HOST_READ_DATA;
                        inform_host();
                    }
                }
                break;

            default:
                break;
        }
    }
}

