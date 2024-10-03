//*****************************************************************************
//
//! @file ios_boot_host.c
//!
//! @brief An example to drive the IO Slave on a second board.
//!
//! Purpose: This example acts as the boot host for either SPI or I2C on
//! Apollo3 MCUs. It will deliver a predefined firmware image to a boot
//! slave over an I2C/SPI protocol. The purpose of this demo is to show how a host
//! processor might store, load, and update the firmware on an Apollo3
//! device that is connected as a slave.
//!
//! Additional Information:
//! Prepare the example as follows:
//!     1. Generate hello_world example to load
//!     2. Copy /examples/hello_world/iar/bin/hello_world.bin into /boards/common3/examples/ios_boot_host/
//!     3. Create the .h file
//!        ./pack_for_boot.py hello_world.bin -o apollo3_boot_demo -l 0x0000C000
//!     4. Include the generated apollo3_boot_demo.h in ios_boot_host.c
//!     5. Build the project and download to the master board
//!
//! @verbatim
//! PIN fly lead connections assumed by this example:
//!     HOST                                    SLAVE
//!     --------                                --------
//!     GPIO[2]  GPIO Interrupt (slave to host) GPIO[4]  GPIO interrupt
//!     GPIO[4]  OVERRIDE pin   (host to slave) GPIO[47] Override pin or n/c
//!     GPIO[5]  IOM0 SPI CLK/I2C SCL           GPIO[0]  IOS SPI SCK/I2C SCL
//!     GPIO[6]  IOM0 SPI MISO/I2C SDA          GPIO[2]  IOS SPI MISO/I2C SDA
//!     GPIO[7]  IOM0 SPI MOSI                  GPIO[1]  IOS SPI MOSI
//!     GPIO[11] IOM0 SPI nCE                   GPIO[3]  IOS SPI nCE
//!     GPIO[17] Slave reset (host to slave)    nRST     Reset Pin or n/c
//!     GND                                     GND
//! Reset and Override pin connections from Host are optional
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
#include <string.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "ios_boot_host.h"
#include "apollo3_boot_demo.h"

//*****************************************************************************
//
// Download image details.
//
// This header file represents the binary image that the boot host will try to
// load onto the slave device.
//
// By default, the demo image used is the same as the host device.  That is,
// the slave device to which the demo image is downloaded is assumed to be
// the same device as the host.  If the 2 devices are not the same, the
// appropriate image to be downloaded to the slave can be selected here.
//
//*****************************************************************************
// Slave interrupt pin is connected here
#define BOOTLOADER_HANDSHAKE_PIN           2
// This pin is connected to RESET pin of slave
#define DRIVE_SLAVE_RESET_PIN              17
// This pin is connected to the 'Override' pin of slave
#define DRIVE_SLAVE_OVERRIDE_PIN           4

#define IOSOFFSET_WRITE_INTEN       0xF8
#define IOSOFFSET_WRITE_INTCLR      0xFA
#define IOSOFFSET_WRITE_CMD         0x80
#define IOSOFFSET_READ_INTSTAT      0x79
#define IOSOFFSET_READ_FIFO         0x7F
#define IOSOFFSET_READ_FIFOCTR      0x7C

//*****************************************************************************
//
// I2C slave address.
//
//*****************************************************************************
#define SLAVE_ADDRESS                   0x10
#define IOM_MODULE                      0
#define USE_SPI                         1   // 0 = I2C, 1 = SPI
#define MAX_IOS_LRAM_SIZE               120     // LRAM can only accept 120 bytes at a time.

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
void *g_IOMHandle;
volatile bool bIosInt = false;
uint32_t g_numUpdates = (IMAGE_SIZE + MAX_UPDATE_SIZE - 1) / MAX_UPDATE_SIZE;
uint32_t g_UpdateStart = 0;
uint32_t g_UpdateEnd = IMAGE_SIZE;
uint32_t g_Applen = 0;
uint32_t g_SentDataSize = 0;

struct
{
  am_secboot_ios_pkthdr_t       header;
  uint8_t                       data[MAX_IOS_LRAM_SIZE - sizeof(am_secboot_ios_pkthdr_t)];
} g_IosPktData;

//*****************************************************************************
//
// Configuration structure for the IO Master.
//
//*****************************************************************************
static am_hal_gpio_pincfg_t g_AM_BSP_GPIO_BOOT_HANDSHAKE =
{
    .uFuncSel       = AM_HAL_PIN_2_GPIO,
    .eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA,
    .eGPOutcfg      = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
    .eGPInput       = AM_HAL_GPIO_PIN_INPUT_ENABLE,
    .eIntDir        = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
};

static am_hal_gpio_pincfg_t g_AM_BSP_GPIO_RESET =
{
    .uFuncSel       = AM_HAL_PIN_17_GPIO,
    .eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA,
    .eGPOutcfg      = AM_HAL_GPIO_PIN_OUTCFG_DISABLE
};

static am_hal_gpio_pincfg_t g_AM_BSP_GPIO_OVERRIDE =
{
    .uFuncSel       = AM_HAL_PIN_4_GPIO,
    .eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA,
    .eGPOutcfg      = AM_HAL_GPIO_PIN_OUTCFG_DISABLE
};

static am_hal_iom_config_t g_sIOMI2cConfig =
{
    .eInterfaceMode = AM_HAL_IOM_I2C_MODE,
    .ui32ClockFreq  = AM_HAL_IOM_100KHZ,
};

static am_hal_iom_config_t g_sIOMSpiConfig =
{
    .eInterfaceMode = AM_HAL_IOM_SPI_MODE,
    .ui32ClockFreq = AM_HAL_IOM_8MHZ,
    .eSpiMode = AM_HAL_IOM_SPI_MODE_0,
};

typedef enum
{
    AM_SECBOOT_WIRED_HOST_NOT_START = 0,
    AM_SECBOOT_WIRED_HOST_HELLO,
    AM_SECBOOT_WIRED_HOST_UPDATE_INFO0,
    AM_SECBOOT_WIRED_HOST_INFO0_DATA,
    AM_SECBOOT_WIRED_HOST_OTADESC,
    AM_SECBOOT_WIRED_HOST_UPDATE_MAIN,
    AM_SECBOOT_WIRED_HOST_MAIN_DATA,
    AM_SECBOOT_WIRED_HOST_UPDATE_CHILD,
    AM_SECBOOT_WIRED_HOST_CHILD_DATA,
    AM_SECBOOT_WIRED_HOST_RESET,
}am_secboot_host_proc_e;

uint32_t g_ui32HostState = AM_SECBOOT_WIRED_HOST_NOT_START;

//*****************************************************************************
//
//! @brief  Performs CRC validation using the hardware engine
//! For protection, it implements finite wait for the CRC engine
//!
//! @param  addr start of the blob
//! @param  size indicates size of blob
//! @param  crc is the value to compare against
//!
//! @return Returns true on success
//
//*****************************************************************************
bool
am_secboot_validate_crc(uint32_t addr, uint32_t size, uint32_t crc)
{
    uint32_t computedCrc;
    bool status;
    status = am_hal_crc32(addr, size, &computedCrc);
    if (!status && (crc == computedCrc))
    {
        return true;
    }
    return false;
}

//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************
void am_gpio_isr(void)
{
    //
    // Read and clear the GPIO interrupt status.
    //
#if defined(AM_PART_APOLLO3P)
    AM_HAL_GPIO_MASKCREATE(GpioIntStatusMask);

    am_hal_gpio_interrupt_status_get(false, pGpioIntStatusMask);
    am_hal_gpio_interrupt_clear(pGpioIntStatusMask);
    am_hal_gpio_interrupt_service(pGpioIntStatusMask);
#elif defined(AM_PART_APOLLO3)
    uint64_t ui64Status;

    am_hal_gpio_interrupt_status_get(false, &ui64Status);
    am_hal_gpio_interrupt_clear(ui64Status);
    am_hal_gpio_interrupt_service(ui64Status);
#else
    #error Unknown device.
#endif
}

// ISR callback for the host IOINT
static void hostint_handler(void)
{
    bIosInt = true;
}

//*****************************************************************************
//
// Reset the slave device and force it into boot mode.
//
//*****************************************************************************
void start_boot_mode(void)
{
    //
    // Drive RESET low.
    //
    am_hal_gpio_state_write(DRIVE_SLAVE_RESET_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_pinconfig(DRIVE_SLAVE_RESET_PIN, g_AM_BSP_GPIO_RESET);
    //
    // Drive the override pin low to force the slave into boot mode.
    //
    am_hal_gpio_state_write(DRIVE_SLAVE_OVERRIDE_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_pinconfig(DRIVE_SLAVE_OVERRIDE_PIN, g_AM_BSP_GPIO_OVERRIDE);
    //
    // Short delay.
    //
    am_util_delay_us(5);
    //
    // Release RESET.
    //
    am_hal_gpio_state_write(DRIVE_SLAVE_RESET_PIN, AM_HAL_GPIO_OUTPUT_SET);

    //
    // Short delay.
    //
    am_util_delay_us(5);
}

void iom_slave_read(bool bSpi, uint32_t offset, uint32_t *pBuf, uint32_t size)
{
    am_hal_iom_transfer_t       Transaction;

    Transaction.ui32InstrLen    = 1;
    Transaction.ui32Instr       = offset;
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32NumBytes    = size;
    Transaction.pui32RxBuffer   = pBuf;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    if ( bSpi )
    {
        Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;
    }
    else
    {
        Transaction.uPeerInfo.ui32I2CDevAddr = SLAVE_ADDRESS;
    }
    am_hal_iom_blocking_transfer(g_IOMHandle, &Transaction);
}

void iom_slave_write(bool bSpi, uint32_t offset, uint32_t *pBuf, uint32_t size)
{
    am_hal_iom_transfer_t       Transaction;

    Transaction.ui32InstrLen    = 1;
    Transaction.ui32Instr       = offset;
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes    = size;
    Transaction.pui32TxBuffer   = pBuf;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    if ( bSpi )
    {
        Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;
    }
    else
    {
        Transaction.uPeerInfo.ui32I2CDevAddr = SLAVE_ADDRESS;
    }
    am_hal_iom_blocking_transfer(g_IOMHandle, &Transaction);
}


void send_hello(bool bSpi)
{
    struct
    {
      am_secboot_ios_pkthdr_t   hdr;
      am_secboot_wired_msghdr_t msg;
    } pkt;

    pkt.hdr.bStart = 1;
    pkt.hdr.bEnd = 1;
    pkt.hdr.length = sizeof(pkt);
    pkt.msg.msgType = AM_SECBOOT_WIRED_MSGTYPE_HELLO;
    pkt.msg.length = sizeof(am_secboot_wired_msghdr_t);
    // Compute CRC
    am_hal_crc32((uint32_t)&pkt.msg.msgType, pkt.msg.length - sizeof(uint32_t), &pkt.msg.crc32);
    iom_slave_write(bSpi, 0x80, (uint32_t*)&pkt, sizeof(pkt));
    g_ui32HostState = AM_SECBOOT_WIRED_HOST_HELLO;
}

void send_update(bool bSpi, uint32_t ui32Crc, uint8_t *pData, uint32_t ui32Size)
{
    struct
    {
      am_secboot_ios_pkthdr_t   hdr;
      am_secboot_wired_msg_update_t msg;
    } pkt;

    if (IMAGE_SIZE < ui32Size)
    {
        am_util_stdio_printf("Incorrect input parameters!\n");
        return;
    }
    pkt.hdr.bStart = 1;
    pkt.hdr.bEnd = 1;
    pkt.hdr.length = sizeof(pkt);
    pkt.msg.hdr.msgType = AM_SECBOOT_WIRED_MSGTYPE_UPDATE;
    pkt.msg.hdr.length = sizeof(am_secboot_wired_msg_update_t);
    pkt.msg.totalSize = IMAGE_SIZE;
    pkt.msg.crc32 = ui32Crc;
    pkt.msg.size = 0;
    if (ui32Size)
    {
        // no data here in this example
        return;
    }
    am_hal_crc32((uint32_t)&pkt.msg.hdr.msgType, pkt.msg.hdr.length - sizeof(uint32_t), &pkt.msg.hdr.crc32);
    iom_slave_write(bSpi, 0x80, (uint32_t*)&pkt, sizeof(pkt));
}

struct
{
  am_secboot_ios_pkthdr_t   hdr;
  am_secboot_wired_msg_data_t msg;
  uint8_t data[MAX_CHUNK_SIZE];
} data_pkt;

void send_data(bool bSpi, uint32_t ui32SeqNum, uint8_t *pData, uint32_t ui32Size)
{
    uint32_t ui32Offset = 0;

    if (MAX_CHUNK_SIZE < ui32Size)
    {
        am_util_stdio_printf("Incorrect input parameters!\n");
        return;
    }
    // The first packet, set the Start flag.
    data_pkt.hdr.bStart = 1;
    if (ui32Size > sizeof(g_IosPktData.data) - sizeof(am_secboot_wired_msg_data_t)) //116-12
    {
        data_pkt.hdr.bEnd = 0;
        data_pkt.hdr.length = sizeof(g_IosPktData); //120
        ui32Offset = sizeof(g_IosPktData.data) - sizeof(am_secboot_wired_msg_data_t);
    }
    else
    {
        data_pkt.hdr.bEnd = 1;
        data_pkt.hdr.length = sizeof(am_secboot_ios_pkthdr_t) + sizeof(am_secboot_wired_msg_data_t) + ui32Size;
        ui32Offset = ui32Size;
    }
    memcpy(data_pkt.data, pData, ui32Size);
    data_pkt.msg.hdr.msgType = AM_SECBOOT_WIRED_MSGTYPE_DATA;
    data_pkt.msg.hdr.length = sizeof(am_secboot_wired_msg_data_t) + ui32Size;
    data_pkt.msg.seqNum = ui32SeqNum;
    am_hal_crc32((uint32_t)&data_pkt.msg.hdr.msgType, data_pkt.msg.hdr.length - sizeof(uint32_t), &data_pkt.msg.hdr.crc32);
    bIosInt = false;
    iom_slave_write(bSpi, 0x80, (uint32_t*)&data_pkt, data_pkt.hdr.length);
    while(!bIosInt);

    // The rest packets
    for (uint32_t index = ui32Offset; index < ui32Size; index += sizeof(g_IosPktData.data))
    {
        g_IosPktData.header.bStart = 0;
        g_IosPktData.header.bEnd = 0;
        // If this this the last packet, then set the End flag.
        if ((index + sizeof(g_IosPktData.data)) >= ui32Size)
        {
            g_IosPktData.header.bEnd = 1;
        }
        // Build and Send the next packet.
        g_IosPktData.header.length = ((ui32Size - index) < sizeof(g_IosPktData.data)) ? (ui32Size - index) : sizeof(g_IosPktData.data);
        memcpy(&g_IosPktData.data[0], pData + index, g_IosPktData.header.length);
        g_IosPktData.header.length += sizeof(am_secboot_ios_pkthdr_t);
        bIosInt = false;
        iom_slave_write(bSpi, 0x80, (uint32_t*)&g_IosPktData, g_IosPktData.header.length);
        // Wait for the GPIO Interrupt before sending the next packet.
        while(!bIosInt);
    }
}

void send_otadesc(bool bSpi, uint32_t ui32Desc)
{
    struct
    {
      am_secboot_ios_pkthdr_t   hdr;
      am_secboot_wired_msg_ota_desc_t msg;
    } pkt;

    pkt.hdr.bStart = 1;
    pkt.hdr.bEnd = 1;
    pkt.hdr.length = sizeof(pkt);
    pkt.msg.hdr.msgType = AM_SECBOOT_WIRED_MSGTYPE_OTADESC;
    pkt.msg.hdr.length = sizeof(am_secboot_wired_msg_ota_desc_t);
    pkt.msg.otaDescAddr = ui32Desc;

    am_hal_crc32((uint32_t)&pkt.msg.hdr.msgType, pkt.msg.hdr.length - sizeof(uint32_t), &pkt.msg.hdr.crc32);
    iom_slave_write(bSpi, 0x80, (uint32_t*)&pkt, sizeof(pkt));
}

void send_reset(bool bSpi, uint32_t ui32Option)
{
    struct
    {
      am_secboot_ios_pkthdr_t   hdr;
      am_secboot_wired_msg_reset_t msg;
    } pkt;

    if ((AM_SECBOOT_MSG_RESET_OPTIONS_POI != ui32Option) && (AM_SECBOOT_MSG_RESET_OPTIONS_POR != ui32Option))
    {
        am_util_stdio_printf("Incorrect input parameters!\n");
        return;
    }
    pkt.hdr.bStart = 1;
    pkt.hdr.bEnd = 1;
    pkt.hdr.length = sizeof(pkt);
    pkt.msg.hdr.msgType = AM_SECBOOT_WIRED_MSGTYPE_RESET;
    pkt.msg.options = ui32Option;
    pkt.msg.hdr.length = sizeof(am_secboot_wired_msg_reset_t);
    am_hal_crc32((uint32_t)&pkt.msg.hdr.msgType, pkt.msg.hdr.length - sizeof(uint32_t), &pkt.msg.hdr.crc32);
    iom_slave_write(bSpi, 0x80, (uint32_t*)&pkt, sizeof(pkt));
}

void host_process_packet(bool bSpi)
{
    uint32_t crc = 0;
    uint32_t chunklen = 0;
    uint8_t *pdata = NULL;

    if (AM_SECBOOT_WIRED_HOST_NOT_START == g_ui32HostState)
    {
        return;    // ota finished or not started
    }
    else if (AM_SECBOOT_WIRED_HOST_HELLO == g_ui32HostState)
    {
        am_util_stdio_printf("Received ACK of HELLO\n");
        am_secboot_wired_msg_status_t status;
        iom_slave_read(bSpi, IOSOFFSET_READ_FIFO, (uint32_t*)&status, sizeof(am_secboot_wired_msg_status_t));

        if ((AM_SECBOOT_WIRED_MSGTYPE_STATUS != status.hdr.msgType) || (IMAGE_SIZE > status.maxImageSize))
        {
            am_util_stdio_printf("Wrong status! msgtype is: 0x%x imagesize is: 0x%x\n", status.hdr.msgType, status.maxImageSize);
            return;
        }
        // Check CRC
        if (am_secboot_validate_crc((uint32_t)(&status.hdr.msgType), sizeof(am_secboot_wired_msg_status_t) - 4, status.hdr.crc32))
        {
            send_otadesc(bSpi, 0xFE000); //Default is 0xFE000 - at the end of main flash
            g_ui32HostState = AM_SECBOOT_WIRED_HOST_OTADESC;
        }
        else
        {
            am_util_stdio_printf("Wrong CRC!\n");
        }
    }
    else
    {
        am_secboot_wired_msg_ack_t ack;
        iom_slave_read(bSpi, IOSOFFSET_READ_FIFO, (uint32_t*)&ack, sizeof(am_secboot_wired_msg_ack_t));

        if (AM_SECBOOT_WIRED_ACK_STATUS_SUCCESS != ack.status)
        {
            am_util_stdio_printf("NACK received!\n");
            return;
        }

        // Check CRC
        if (am_secboot_validate_crc((uint32_t)(&ack.hdr.msgType), sizeof(am_secboot_wired_msg_ack_t) - sizeof(uint32_t), ack.hdr.crc32))
        {
            switch(g_ui32HostState)
            {
                case AM_SECBOOT_WIRED_HOST_OTADESC:
                    am_util_stdio_printf("Received ACK of OTADESC\n");
                    am_util_stdio_printf("number of updates needed = %d\n", g_numUpdates);
                    g_UpdateStart = (g_numUpdates - 1) * MAX_UPDATE_SIZE;
                    if (g_numUpdates > 1)
                    {
                        am_hal_crc32((uint32_t)(IMAGE_ARRAY + g_UpdateStart), MAX_UPDATE_SIZE, &crc);
                    }
                    else
                    {
                        am_hal_crc32((uint32_t)(IMAGE_ARRAY + g_UpdateStart), IMAGE_SIZE % MAX_UPDATE_SIZE, &crc);
                    }
                    g_Applen = g_UpdateEnd - g_UpdateStart;
                    am_util_stdio_printf("Sending block of size 0x%x from 0x%x to 0x%x\n", g_Applen, g_UpdateStart, g_UpdateEnd);
                    g_UpdateEnd = g_UpdateEnd - g_Applen;
                    //Size = 0 => We're not piggybacking any data to IMAGE command
                    send_update(bSpi, crc, NULL, 0);
                    g_numUpdates--;
                    g_SentDataSize = 0;
                    g_ui32HostState = AM_SECBOOT_WIRED_HOST_UPDATE_MAIN;
                    break;
                case AM_SECBOOT_WIRED_HOST_UPDATE_MAIN:
                case AM_SECBOOT_WIRED_HOST_MAIN_DATA:
                    am_util_stdio_printf("Received ACK of main update\n");
                    if (g_SentDataSize >= g_Applen)
                    {
                        if (0 == g_numUpdates)
                        {
                            send_reset(bSpi, 1); //POI
                            g_ui32HostState = AM_SECBOOT_WIRED_HOST_RESET;
                            break;
                        }
                        else
                        {
                            g_ui32HostState = AM_SECBOOT_WIRED_HOST_OTADESC;
                            break;
                        }
                    }
                    if (g_SentDataSize + MAX_CHUNK_SIZE > g_Applen)
                    {
                        chunklen = g_Applen - g_SentDataSize;
                    }
                    else
                    {
                        chunklen = MAX_CHUNK_SIZE;
                    }
                    pdata = (uint8_t*)&IMAGE_ARRAY[g_UpdateStart + g_SentDataSize];
                    am_util_stdio_printf("Sending Data Packet of length %d\n", chunklen);
                    send_data(bSpi, g_SentDataSize, pdata, chunklen);
                    g_ui32HostState = AM_SECBOOT_WIRED_HOST_MAIN_DATA;
                    g_SentDataSize += chunklen;
                    break;
                case AM_SECBOOT_WIRED_HOST_RESET:
                    am_util_stdio_printf("Received ACK of reset\n");
                    g_ui32HostState = AM_SECBOOT_WIRED_HOST_NOT_START;
                    break;
                default:
                    am_util_stdio_printf("Received Unknown message\n");
                    g_ui32HostState = AM_SECBOOT_WIRED_HOST_NOT_START;
                    break;
            }
        }
        else
        {
            am_util_stdio_printf("Wrong CRC!\n");
        }
    }
}

static void iom_set_up(uint32_t iomModule, bool bSpi)
{
    //
    // Initialize the IOM.
    //
    am_hal_iom_initialize(iomModule, &g_IOMHandle);

    am_hal_iom_power_ctrl(g_IOMHandle, AM_HAL_SYSCTRL_WAKE, false);

    if ( bSpi )
    {
        //
        // Set the required configuration settings for the IOM.
        //
        am_hal_iom_configure(g_IOMHandle, &g_sIOMSpiConfig);

        //
        // Configure the IOM pins.
        //
        am_bsp_iom_pins_enable(iomModule, AM_HAL_IOM_SPI_MODE);
    }
    else
    {
        //
        // Set the required configuration settings for the IOM.
        //
        am_hal_iom_configure(g_IOMHandle, &g_sIOMI2cConfig);

        //
        // Configure the IOM pins.
        //
        am_bsp_iom_pins_enable(iomModule, AM_HAL_IOM_I2C_MODE);
    }

    //
    // Enable the IOM.
    //
    am_hal_iom_enable(g_IOMHandle);

    am_hal_gpio_pinconfig(BOOTLOADER_HANDSHAKE_PIN, g_AM_BSP_GPIO_BOOT_HANDSHAKE);

    // Set up the host IO interrupt
    AM_HAL_GPIO_MASKCREATE(GpioIntMask);
    am_hal_gpio_interrupt_clear( AM_HAL_GPIO_MASKBIT(pGpioIntMask, BOOTLOADER_HANDSHAKE_PIN));
    // Register handler for IOS => IOM interrupt
    am_hal_gpio_interrupt_register(BOOTLOADER_HANDSHAKE_PIN, hostint_handler);
    am_hal_gpio_interrupt_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask, BOOTLOADER_HANDSHAKE_PIN));
    NVIC_EnableIRQ(GPIO_IRQn);
}

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int main(void)
{
    uint32_t iom = IOM_MODULE;
    bool bSpi = USE_SPI;

    //
    // Set the clock frequency.
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Enable the ITM print interface.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("IOS boot host example.\n");

    iom_set_up(iom, bSpi);

    //
    // Force the slave into boot mode.
    //
    start_boot_mode();

    // Wait for initial handshake signal to know that IOS interface is alive
    while(!bIosInt);
    bIosInt = false;

    //
    // Short delay.
    //
    am_util_delay_ms(1);

    send_hello(bSpi);

    // Read the "STATUS" response from the IOS.
    //iom_slave_read(bSpi, IOSOFFSET_READ_FIFO, (uint32_t*)&g_psReadData, 88);

    //
    // Loop forever.
    //
    while(1)
    {
        //
        // Disable interrupt while we decide whether we're going to sleep.
        //
        uint32_t ui32IntStatus = am_hal_interrupt_master_disable();

        if ( bIosInt == true )
        {
            //
            // Enable interrupts
            //
            am_hal_interrupt_master_set(ui32IntStatus);
            bIosInt = false;
            host_process_packet(bSpi);
        }
        else
        {
            am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
            //
            // Enable interrupts
            //
            am_hal_interrupt_master_set(ui32IntStatus);
        }
    }
}

