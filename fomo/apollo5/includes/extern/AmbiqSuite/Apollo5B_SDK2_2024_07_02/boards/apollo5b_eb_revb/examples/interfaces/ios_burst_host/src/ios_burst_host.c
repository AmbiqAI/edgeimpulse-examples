//*****************************************************************************
//
//! @file ios_burst_host.c
//!
//! @brief Example host used for demonstrating the use of the IOS burst mode.
//!
//! Purpose: This host component runs on one EB and is used in conjunction with
//! the companion slave example, ios_burst, which runs on a second EB.
//!
//! In this pair of examples, IOM and IOS are switched as below for each mode:
//! Host burst write : IOM burst write - IOS LRAM burst read
//! (Host writes unlimited size of data to slave)
//! Host read : IOM FIFO read - IOS FIFO write
//! (Host reads data from slave max. 1023 bytes)
//!
//! The host example uses the ITM SWO to let the user know progress and
//! status of the demonstration.  The SWO is configured at 1M baud.
//!
//! Printing takes place over the ITM at 1M Baud.
//!
//! Additional Information:
//! In order to run this example, a slave device (e.g. a second EB) must be set
//! up to run the companion example, ios_burst.  The two boards can be connected
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
//!
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
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "ios_test_data.h"

#define IOM_MODULE                  1
#define USE_NONBLOCKING             0

#define INBUFFER_EMPTY              (0xEE)
#define XOR_BYTE                    0

// Handshake pins
#define HANDSHAKE_IOM_PIN          4

#define HANDSHAKE_CMD_STOP         0xFC
#define HANDSHAKE_CMD_WRITE_DATA   0xFD
#define HANDSHAKE_CMD_READ_DATA    0xFE
#define HANDSHAKE_CMD_ACK          0xFF

// Register space
#define IOSOFFSET_WRITE_INTEN       0xF8
#define IOSOFFSET_WRITE_INTCLR      0xFA
#define IOSOFFSET_WRITE_CMD         0x80
#define IOSOFFSET_READ_INTSTAT      0x79
#define IOSOFFSET_READ_FIFO         0x7F
#define IOSOFFSET_READ_FIFOCTR      0x7C

#define AM_IOSTEST_IOSTOHOST_DATAAVAIL_INTMASK  1

#define AM_IOM_TX_UNIT_SIZE        2048
//#define AM_IOM_TX_UNIT_SIZE        3072

//#define AM_IOS_LRAM_TEST_LENGTH         ((AM_IOM_TX_UNIT_SIZE - 3))          // single transfer
#define AM_IOS_LRAM_TEST_LENGTH         ((AM_IOM_TX_UNIT_SIZE - 3) * 5)       // 5 packets

#define MAX_FIFO_READ_SIZE          1023
#define AM_TEST_RCV_BUF_SIZE        1024        // Max Size we can receive is 1023 by FIFO.
#define HOST_READ_TEST_SIZE_TOTAL   50000       // How much data to read from Slave before ending the test

#define IOM_DMA_IDLE                0
#define IOM_DMA_WRITE               1
#define IOM_DMA_READ                2

//*****************************************************************************
//
// Global variables for the IO master write / read.
//
//*****************************************************************************
void *g_IOMHandle;
volatile uint8_t g_iomMode = 0;
volatile bool bIomReqAckInt = false;

AM_SHARED_RW uint8_t g_pIomSendBuf[AM_IOM_TX_UNIT_SIZE] __attribute__((aligned(32))); //User buffer for host write
AM_SHARED_RW uint32_t DMATCBBuffer[AM_IOM_TX_UNIT_SIZE];     // DMA buffer for IOM TX

AM_SHARED_RW uint8_t g_pui8RcvBuf[AM_TEST_RCV_BUF_SIZE] __attribute__((aligned(32))); // User buffer for host read
volatile uint32_t g_recvDataIndex = 0;
uint32_t g_ui32LastUpdate = 0;

#if USE_NONBLOCKING
volatile uint8_t g_iomDmaStatus = IOM_DMA_IDLE;

am_hal_mpu_region_config_t sMPUCfg =
{
    .ui32RegionNumber = 6,
    .ui32BaseAddress = (uint32_t)DMATCBBuffer,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)DMATCBBuffer + sizeof(DMATCBBuffer) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
};
am_hal_mpu_attr_t sMPUAttr =
{
    .ui8AttrIndex = 0,
    .bNormalMem = true,
    .sOuterAttr = {
                    .bNonTransient = false,
                    .bWriteBack = true,
                    .bReadAllocate = false,
                    .bWriteAllocate = false
                  },
    .sInnerAttr = {
                    .bNonTransient = false,
                    .bWriteBack = true,
                    .bReadAllocate = false,
                    .bWriteAllocate = false
                  },
    .eDeviceAttr = 0,
};
#endif

//*****************************************************************************
//
// Configuration structure for the IO Master.
//
//*****************************************************************************
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

am_hal_iom_config_t g_sIomHostConfig =
{
    .eInterfaceMode = AM_HAL_IOM_SPI_MODE,
    .ui32ClockFreq = AM_HAL_IOM_1MHZ,
    .eSpiMode = AM_HAL_IOM_SPI_MODE_0,
    .pNBTxnBuf          = &DMATCBBuffer[0],
    .ui32NBTxnBufLength = sizeof(DMATCBBuffer) / 4
};

const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_IOM_REQ_ACK =
{
    .GP.cfg_b.uFuncSel       = AM_HAL_PIN_4_GPIO,
    .GP.cfg_b.eIntDir        = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
    .GP.cfg_b.eGPInput       = AM_HAL_GPIO_PIN_INPUT_ENABLE,
};

// ISR callback for the host IOINT
static void iom_req_ack_handler(void)
{
    bIomReqAckInt = true;
}

#if USE_NONBLOCKING
static void pfnIOM_LRAM_Callback(void *pCallbackCtxt, uint32_t status)
{
    // Set the DMA complete flag.
    if ( g_iomDmaStatus == IOM_DMA_READ )
    {
        am_hal_cachectrl_range_t sRange;
        sRange.ui32StartAddr = (uint32_t)g_pui8RcvBuf;
        sRange.ui32Size = AM_TEST_RCV_BUF_SIZE;
        am_hal_cachectrl_dcache_invalidate(&sRange, false);
    }
    g_iomDmaStatus = IOM_DMA_IDLE;
}
#endif

//*****************************************************************************
//
// Clear Rx Buffer for comparison
//
//*****************************************************************************
void clear_rx_buf(void)
{
    uint32_t i;
    for ( i = 0; i < AM_TEST_RCV_BUF_SIZE; i++ )
    {
        g_pui8RcvBuf[i] = INBUFFER_EMPTY;
    }
}

//*****************************************************************************
//
// Validate Rx Buffer
// Returns 0 for success case
//
//*****************************************************************************
uint32_t validate_rx_buf(uint32_t rxSize)
{
    uint32_t i;
    for ( i = 0; i < rxSize; i++ )
    {
        if ( g_pui8RcvBuf[i] != (((g_recvDataIndex + i) & 0xFF) ^ XOR_BYTE) )
        {
            am_util_stdio_printf("Failed to compare buffers at index %d %d %d\n",
                i, g_pui8RcvBuf[i], (((g_recvDataIndex + i) & 0xFF) ^ XOR_BYTE));
            break;
        }
    }
    // Set the reference for next chunk
    g_recvDataIndex += rxSize;
    return (i == rxSize);
}

//*****************************************************************************
//
// Print a progress message.
//
//*****************************************************************************
void update_progress(uint32_t ui32NumPackets)
{
    //
    // Print a dot every 10000 packets.
    //
    if ( (ui32NumPackets - g_ui32LastUpdate) >= 10000 )
    {
        am_util_stdio_printf(".");
        g_ui32LastUpdate = ui32NumPackets;
    }
}

//*****************************************************************************
//
// Interrupt handler for IOM.
//
//*****************************************************************************
void
am_iomaster1_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_IOMHandle, true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_IOMHandle, ui32Status);
            am_hal_iom_interrupt_service(g_IOMHandle, ui32Status);
        }
    }
}

//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************
void am_gpio0_001f_isr(void)
{
    //
    // Read and clear the GPIO interrupt status.
    //
    uint32_t    ui32IntStatus;
    AM_CRITICAL_BEGIN
    am_hal_gpio_interrupt_irq_status_get(GPIO0_001F_IRQn, false, &ui32IntStatus);
    am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);
    AM_CRITICAL_END
    am_hal_gpio_interrupt_service(GPIO0_001F_IRQn, ui32IntStatus);
}

//*****************************************************************************
//
// IOM reads IOS function
//
//*****************************************************************************
void iom_slave_read(uint32_t offset, uint8_t *pBuf, uint32_t size)
{
    am_hal_iom_transfer_t       Transaction;

    Transaction.ui8Priority     = 1;        // High priority for now.
    Transaction.ui32InstrLen    = 1;
    Transaction.ui64Instr       = offset;
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32NumBytes    = size;
    Transaction.pui32RxBuffer   = (uint32_t *)pBuf;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;

#if USE_NONBLOCKING
    if ( offset == IOSOFFSET_READ_FIFO )
    {
        g_iomDmaStatus = IOM_DMA_READ;
        am_hal_iom_nonblocking_transfer(g_IOMHandle, &Transaction, pfnIOM_LRAM_Callback, NULL);
        while (g_iomDmaStatus == IOM_DMA_READ);
    }
    else
    {   // Keep using blocking for register read
        am_hal_iom_blocking_transfer(g_IOMHandle, &Transaction);
    }
#else
    am_hal_iom_blocking_transfer(g_IOMHandle, &Transaction);
#endif
}

//*****************************************************************************
//
// IOM writes to IOS function
//
//*****************************************************************************
void iom_slave_write(uint32_t offset, uint8_t *pBuf, uint32_t size)
{
    am_hal_iom_transfer_t       Transaction;

    Transaction.ui8Priority     = 1;        // High priority for now.
    Transaction.ui32InstrLen    = 1;
    Transaction.ui64Instr       = offset;
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes    = size;
    Transaction.pui32TxBuffer   = (uint32_t *)pBuf;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;

#if USE_NONBLOCKING
    if ( offset == IOSOFFSET_WRITE_CMD )
    {
        g_iomDmaStatus = IOM_DMA_WRITE;
        am_hal_iom_nonblocking_transfer(g_IOMHandle, &Transaction, pfnIOM_LRAM_Callback, NULL);
        while (g_iomDmaStatus == IOM_DMA_WRITE);
    }
    else
    {   // Keep using blocking for register write
        am_hal_iom_blocking_transfer(g_IOMHandle, &Transaction);
    }
#else
    am_hal_iom_blocking_transfer(g_IOMHandle, &Transaction);
#endif
}

//*****************************************************************************
//
// Config handshake pins
//
//*****************************************************************************
void iom_gpio_config(void)
{
    uint32_t IntNum = HANDSHAKE_IOM_PIN;

    // Set up the IOM_REQ_ACK_PIN interrupt
    am_hal_gpio_pinconfig(HANDSHAKE_IOM_PIN, g_AM_BSP_GPIO_IOM_REQ_ACK);

    // Set up the host IO interrupt
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_INT_CHANNEL_0, (am_hal_gpio_mask_t*)&IntNum);
    // Register handler for IOS => IOM interrupt
    am_hal_gpio_interrupt_register(AM_HAL_GPIO_INT_CHANNEL_0, HANDSHAKE_IOM_PIN,
                                    (am_hal_gpio_handler_t)iom_req_ack_handler, NULL);
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                                  AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                                  (void *)&IntNum);
    NVIC_SetPriority(GPIO0_001F_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(GPIO0_001F_IRQn);
}

//*****************************************************************************
//
// Set up IOM
//
//*****************************************************************************
static void iom_set_up(uint32_t iomModule)
{
    //
    // Initialize the IOM.
    //
    am_hal_iom_initialize(iomModule, &g_IOMHandle);
    am_hal_iom_power_ctrl(g_IOMHandle, AM_HAL_SYSCTRL_WAKE, false);

    am_hal_iom_configure(g_IOMHandle, &g_sIomHostConfig);

    //
    // Configure the IOM pins.
    //
    am_bsp_iom_pins_enable(iomModule, AM_HAL_IOM_SPI_MODE);


    // Enable interrupts for NB send to work
    am_hal_iom_interrupt_enable(g_IOMHandle, 0xFF);
    NVIC_EnableIRQ(IOMSTR1_IRQn);

    //
    // Enable the IOM.
    //
    am_hal_iom_enable(g_IOMHandle);
}

//*****************************************************************************
//
// Clean up IOM
//
//*****************************************************************************
static void iom_clean_up(uint32_t iomModule)
{
    am_hal_iom_interrupt_disable(g_IOMHandle, 0xFF);
    NVIC_DisableIRQ(IOMSTR1_IRQn);

    //
    // Turn off the IOM for this operation.
    //
    am_hal_iom_disable(g_IOMHandle);

    //
    // Clean up IOM SPI pins. Attributes are set in am_bsp_gpio.h.
    //
    am_bsp_iom_pins_disable(iomModule, AM_HAL_IOM_SPI_MODE);

    //
    // Disable power to IOM.
    //
    am_hal_iom_power_ctrl(g_IOMHandle, AM_HAL_SYSCTRL_DEEPSLEEP, false);
}

//*****************************************************************************
//
// Checksum calculation
//
//*****************************************************************************
static uint8_t get_checksum(uint8_t* pData, uint8_t len)
{
    uint32_t checksum = 0;
    for ( uint8_t i = 0; i < len; i++ )
    {
        checksum += pData[i];
    }

    return (uint8_t)(checksum & 0xff);
}

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int main(void)
{
    uint32_t iom = IOM_MODULE;
    bool bTestDone = false;
    uint32_t data;

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

#if USE_NONBLOCKING
    am_hal_cachectrl_range_t sRange;
    sRange.ui32StartAddr = (uint32_t)g_pIomSendBuf;
    sRange.ui32Size = AM_IOM_TX_UNIT_SIZE;

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);
    //
    // Set up the attributes.
    //
    am_hal_mpu_attr_configure(&sMPUAttr, 1);
    //
    // Clear the MPU regions.
    //
    am_hal_mpu_region_clear();
    //
    // Set up the regions.
    //
    am_hal_mpu_region_configure(&sMPUCfg, 1);
    //
    // Invalidate and clear DCACHE, this is required by CM55 TRF.
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    //
    // MPU enable
    //
    am_hal_mpu_enable(true, true);
#endif
    //
    // Enable the ITM print interface.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("IOS Burst Example Host.\n\n");
    am_util_stdio_printf("The pin jumpers are as follows:\n\n");
    am_util_stdio_printf("SPI:        Slave                   Master           \n\n");
    am_util_stdio_printf("Pins  :     IOS (Apollo5_eb)        IOM1 (Apollo5_eb)\n");
    am_util_stdio_printf("%d to %d :  IOS SPI SCK             to  IOM1 SPI SCK\n", AM_BSP_GPIO_IOS_SCK, AM_BSP_GPIO_IOM1_SCK);
    am_util_stdio_printf("%d to %d :  IOS SPI MOSI            to  IOM1 SPI MOSI\n", AM_BSP_GPIO_IOS_MOSI, AM_BSP_GPIO_IOM1_MOSI);
    am_util_stdio_printf("%d to %d :  IOS SPI MISO            to  IOM1 SPI MOSI\n", AM_BSP_GPIO_IOS_MISO, AM_BSP_GPIO_IOM1_MISO);
    am_util_stdio_printf("%d to %d :  IOS SPI nCE             to  IOM1 SPI nCE\n", AM_BSP_GPIO_IOS_CE, AM_BSP_GPIO_IOM1_CS);
    am_util_stdio_printf("%d to 4  :  IOS SPI INT             to  HOST gpio interrupt\n", AM_BSP_GPIO_IOS_INT);

    //
    // Enable Interrupts.
    //
    am_hal_interrupt_master_enable();

    // Init buffer
    for (int i = 0; i < AM_IOM_TX_UNIT_SIZE; i++)
    {
        g_pIomSendBuf[i] = INBUFFER_EMPTY;
    }

    uint32_t index = 0;

    iom_gpio_config();

    // Host Write Test (Burst) - 2K bytes data write
    g_sIomHostConfig.ui32ClockFreq = spiSpeedString[IOM_SPEED_SPI].MHz;
    am_util_stdio_printf("Write SPI Speed %s\n", spiSpeedString[IOM_SPEED_SPI].MHzString);
    iom_set_up(iom);

    while(bTestDone == false)
    {
        uint16_t ui16DataLength = AM_IOM_TX_UNIT_SIZE - 3;
        uint16_t ui16TxLength = AM_IOM_TX_UNIT_SIZE - 2;
        uint16_t *pui16TxLength = (uint16_t *)g_pIomSendBuf;

        *pui16TxLength = ui16TxLength;      // first 2bytes is indicating size of length.

        memcpy(&g_pIomSendBuf[2], &g_pui8TestArray[index], ui16DataLength);

        g_pIomSendBuf[AM_IOM_TX_UNIT_SIZE - 1] = get_checksum(&g_pIomSendBuf[2], ui16DataLength);
#if USE_NONBLOCKING
        am_hal_cachectrl_dcache_clean(&sRange);
#endif
        iom_slave_read(0, (uint8_t *)&data, 1);
        if ( (data & 0xFF) == HANDSHAKE_CMD_ACK )
        {
            // Send the START slave data collection.
            data = HANDSHAKE_CMD_WRITE_DATA;
            iom_slave_write(IOSOFFSET_WRITE_CMD, (uint8_t *)&data, 1);
            iom_slave_read(0, (uint8_t *)&data, 1);
            while((data & 0xFF) != HANDSHAKE_CMD_ACK)
            {
                am_util_delay_ms(1);
                iom_slave_read(0, (uint8_t *)&data, 1);
            }
            // Send data
            iom_slave_write(IOSOFFSET_WRITE_CMD, g_pIomSendBuf, AM_IOM_TX_UNIT_SIZE);
            index += ui16DataLength;
        }

        if ( index >= AM_IOS_LRAM_TEST_LENGTH )
        {
            iom_clean_up(iom);
            index = 0;
            bTestDone = true;
        }
    }

    bTestDone = false;
    am_util_stdio_printf("IOS Burst Write (Host) Done.\n\n");

    // Host Read Test (FIFO)
    uint32_t ui32FifoCnt = 0;
    uint32_t ioIntEnable = AM_IOSTEST_IOSTOHOST_DATAAVAIL_INTMASK;

    g_sIomHostConfig.ui32ClockFreq = spiSpeedString[IOM_SPEED_SPI].MHz;

    am_util_stdio_printf("Read SPI Speed %s \n", spiSpeedString[IOM_SPEED_SPI].MHzString);

    iom_set_up(iom);

    am_util_delay_ms(500);

    iom_slave_read(0, (uint8_t *)&data, 1);
    while((data & 0xFF) != HANDSHAKE_CMD_ACK)
    {
        am_util_delay_ms(1);
        iom_slave_read(0, (uint8_t *)&data, 1);
    }

    // Set up IOCTL interrupts
    // IOS ==> IOM
    iom_slave_write(IOSOFFSET_WRITE_INTEN, (uint8_t *)&ioIntEnable, 1);

    // Send the START slave data collection.
    data = HANDSHAKE_CMD_READ_DATA;
    iom_slave_write(IOSOFFSET_WRITE_CMD, (uint8_t *)&data, 1);

    while(!bTestDone)
    {
        if ( bIomReqAckInt == true )
        {
            bIomReqAckInt = false;

            // Read & Clear the IOINT status
            iom_slave_read(IOSOFFSET_READ_INTSTAT, (uint8_t *)&data, 1);

            // We need to clear the bit by writing to IOS
            if ( data & AM_IOSTEST_IOSTOHOST_DATAAVAIL_INTMASK )
            {
                data = AM_IOSTEST_IOSTOHOST_DATAAVAIL_INTMASK;
                iom_slave_write(IOSOFFSET_WRITE_INTCLR, (uint8_t *)&data, 1);

                // Read the Data Size
                iom_slave_read(IOSOFFSET_READ_FIFOCTR, (uint8_t *)&ui32FifoCnt, 2);
                //am_util_stdio_printf("iosSize %d\n", iosSize);

                ui32FifoCnt = (ui32FifoCnt > MAX_FIFO_READ_SIZE)? MAX_FIFO_READ_SIZE: ui32FifoCnt;

                // Initialize Rx Buffer for later comparison
                clear_rx_buf();

                // Read the data
                iom_slave_read(IOSOFFSET_READ_FIFO,
                    (uint8_t *)g_pui8RcvBuf, ui32FifoCnt);

                // Validate Content
                if ( !validate_rx_buf(ui32FifoCnt) )
                {
                    am_util_stdio_printf("\nData Verification failed Accum:%lu rx=%d\n",
                        g_recvDataIndex, ui32FifoCnt);
                }
                // Send the ACK/STOP
                data = HANDSHAKE_CMD_ACK;
                iom_slave_write(IOSOFFSET_WRITE_CMD, (uint8_t *)&data, 1);

                update_progress(g_recvDataIndex);

                if ( g_recvDataIndex >= HOST_READ_TEST_SIZE_TOTAL )
                {
                    bTestDone = true;
                    data = HANDSHAKE_CMD_STOP;
                    iom_slave_write(IOSOFFSET_WRITE_CMD, (uint8_t *)&data, 1);
                    am_util_stdio_printf("\n%s Read %dK bytes\n", spiSpeedString[IOM_SPEED_SPI].MHzString,
                        HOST_READ_TEST_SIZE_TOTAL / 1000);
                    iom_clean_up(iom);
                    am_util_delay_ms(500);
                }
            }
        }
        else
        {
            am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_NORMAL);
        }
    }

    am_util_stdio_printf("IOS FIFO Read (Host) Done.\n");
}

