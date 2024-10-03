//*****************************************************************************
//
//! @file am_widget_audio.c
//!
//! @brief Widget to test IOM, IOS and UART for audio use cases.
//!
//! This program initiates a SPI/I2C transaction at specified address/size,
//! For access to LRAM locations (address != 0x7F), IOM sends a pattern to IOS,
//! IOS receives it, manipulates the data and send it back to IOM for
//! verification.
//! For access to FIFO (address == 0x7F), a pattern is send from IOS to IOM,
//! which is then verified for accuracy.
//!
//! The test utilizes an IOM<->IOS transfer within the same device (basically
//! a loopback configuration). Running the test requires jumpering pins in
//! order to connect the IOM to the IOS.
//!
//! @verbatim
//! In IOM<-->IOS mode, the pin jumpers are as follows:
//! SPI:
//! Pins    :   IOS                         IOM2
//! 0 to 25 :   IOS SPI SCK             to  IOM2 SPI SCK
//! 1 to 26 :   IOS SPI MOSI            to  IOM2 SPI MOSI
//! 2 to 27 :   IOS SPI MISO            to  IOM2 SPI MISO
//! 3 to 84 :   IOS SPI nCE             to  IOM2 SPI nCE
//! 4 to 8  :   IOS Interrupt           to  Handshake Interrupt
//!
//! UART loopback:
//! Pins    :   UART0                       UART0
//! 30 to 32:   TX                      to  RX
//! 77 to 76:   CTS                     to  RTS
//!
//! In IOM<-->CODEC mode, the pin jumpers are as follows:
//! SPI:
//! Pins    :   IOM2
//! 25      :   IOM2 SPI SCK
//! 26      :   IOM2 SPI MOSI
//! 27      :   IOM2 SPI MISO
//! 84      :   IOM2 SPI nCE
//! 8       :   Interrupt input (rising edge)
//! 5       :   Output a signal (rising edge) to trigger codec interrupt
//!
//! UART loopback:
//! Pins    :   UART0                       UART0
//! 30 to 32:   TX                      to  RX
//! 77 to 76:   CTS                     to  RTS
//! @endverbatim
//!
//! A simple communication protocol between Apollo4 and Codec:
//! Two 24-bit words in Codec DSP1 X-mem for transfer control.
//! Word 0 @ 0x000A7FFC:
//! bit[0]    - Set by slave - request master to read and provide master the buffer address to read and the packet length.
//!             Clear by master - accept read request.
//! bit[1]    - Set by master - notify slave that read complete.
//!             Clear by slave - acknowledgement.
//! bit[2]    - Set by master - request to write to slave and provided slave the packet length.
//!             Clear by slave - accept write request and provide master the buffer address to write.
//! bit[3]    - Set by master - notify slave that write complete.
//!             Clear by slave - acknowledgement.
//! bit[7:4]  - Reserved.
//! bit[23:8] - Transfer length (number of bytes)
//!
//! Word 1 @ 0x000A7FFE:
//! bit[23:0] - Buffer address in CODEC for SPI transfer.
//!
//! Communication process:
//! Apollo4 reads packet from Codec
//! 1. Codec sets bit-0 in word-0 to request master to read and set buffer address and transfer length
//! 2. Codec outputs a signal (rising edge) to trigger Apollo4 interrupt.
//! 3. Apollo4 polls bit-0 in word-0 to check if slave requested master to read.
//!    If bit-0 in word-0 was set, Apollo4 gets buffer address and transfer length
//!    from control words and clears bit-0 in word-0 to acknowledge.
//! 4. Apollo4 reads packet from Codec.
//! 5. Apollo4 sets bit-1 in word-0 to notify slave that read complete.
//! 6. Codec clears bit-1 in word-0 to acknowledge.
//! 
//! Apollo4 writes packet to Codec:
//! 1. Apollo4 sets bit-2 in word-0 to request to write to Codec and set transfer length
//! 2. Apollo4 outputs a signal (rising edge) to trigger Codec interrupt.
//! 3. Codec polls bit-2 in word-0 to check if master requested to write.
//!    If bit-2 in word-0 was set, Codec creates a buffer and set buffer address
//!    in control words. Codec clears bit-2 in word-0 to acknowledge.
//! 4. Codec polls bit-2 in word-0 to check if slave acknowledged the write request.
//!    If bit-2 in word-0 was cleared, Apollo4 gets buffer address from control words.
//! 5. Apollo4 writes packet to Codec.
//! 6. Apollo4 sets bit-3 in word-0 to notify slave that write complete.
//! 7. Codec clears bit-3 in word-0 to acknowledge.
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
#include <ctype.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "unity.h"
#include "am_widget_audio.h"

#define NO_SHELBY_1653

//*****************************************************************************
//
// GPIO Configuration
//
//*****************************************************************************
const am_hal_gpio_pincfg_t g_AM_BSP_IOM_GPIO_ENABLE =
{
    .GP.cfg_b.uFuncSel            = AM_HAL_PIN_8_GPIO,
    .GP.cfg_b.eDriveStrength      = AM_HAL_GPIO_PIN_DRIVESTRENGTH_12MA,
    .GP.cfg_b.eIntDir             = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
    .GP.cfg_b.eGPInput            = AM_HAL_GPIO_PIN_INPUT_ENABLE,
};
//const am_hal_gpio_pincfg_t g_AM_BSP_IOS_GPIO_ENABLE =
//{
//    .GP.cfg_b.uFuncSel            = AM_HAL_PIN_4_SLINT,
//    .GP.cfg_b.eDriveStrength      = AM_HAL_GPIO_PIN_DRIVESTRENGTH_12MA,
//    .GP.cfg_b.eCEpol              = AM_HAL_GPIO_PIN_CEPOL_ACTIVEHIGH,
//    .GP.cfg_b.eGPOutCfg           = AM_HAL_GPIO_PIN_OUTCFG_OPENDRAIN,
//};

const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_DISABLE =
{
    .GP.cfg_b.uFuncSel            = AM_HAL_PIN_4_GPIO,
    .GP.cfg_b.eGPOutCfg           = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
    .GP.cfg_b.eGPInput            = AM_HAL_GPIO_PIN_INPUT_NONE,
};

//*****************************************************************************
//
// Widget Config parameters
//
//*****************************************************************************
#define TEST_XOR_BYTE       0x0 // 0 Will not change, 0xFF will invert
#ifdef NO_SHELBY_1653
#define TEST_IOINTCTL      // if defined, uses IOINTCTL - SHELBY-1653 interferes with this
#endif
#define TEST_IOS_XCMP_INT  // XCMP flags defined only for Apollo2 onwards

//*****************************************************************************
//
// Message buffers.
//
// data from the IOS interface, which is only 8 bits wide.
//
//*****************************************************************************
#ifdef CODEC_MODE
    #define CODEC_PADDING_BYTES  2
    #define IOM_IOS_AVAILABLE_BUFSIZE_MAX 1024 // must be multiple of 4
    #define IOM_IOS_BUFSIZE_MAX  (IOM_IOS_AVAILABLE_BUFSIZE_MAX + CODEC_PADDING_BYTES) // multiple of 4, plus 2 padding bytes
#else
    #define IOM_IOS_BUFSIZE_MAX  1024 // multiple of 4
#endif
#define PACKET_SIZE_MAX 2048
uint8_t g_pui8TxFifoBuffer[IOM_IOS_BUFSIZE_MAX];
uint8_t g_pIosSendBuf[IOM_IOS_BUFSIZE_MAX];
AM_SHARED_RW uint8_t g_pIomSendBuf[IOM_IOS_BUFSIZE_MAX];
AM_SHARED_RW uint8_t g_pIomRecvBuf[IOM_IOS_BUFSIZE_MAX];
AM_SHARED_RW uint8_t g_pIosRecvBuf[IOM_IOS_BUFSIZE_MAX];
AM_SHARED_RW uint8_t g_pIomRecvPacket[PACKET_SIZE_MAX];
AM_SHARED_RW uint8_t g_pIosRecvPacket[PACKET_SIZE_MAX];
AM_SHARED_RW uint8_t g_pUartRecvPacket[PACKET_SIZE_MAX];

struct {
    uint32_t size;
    uint32_t totalWritten;
} ios_feed_state;

#ifdef TEST_IOM_QUEUE
am_hal_iom_queue_entry_t g_psQueueMemory[32];
#endif

static void *g_pIOMHandle;
static void *g_pIOSHandle;
static void *UART;
volatile uint32_t ui32TXDoneFlag = false;
//*****************************************************************************
//
//! IOM interrupts.
//
//*****************************************************************************
static const IRQn_Type iomaster_interrupt[] = {
    IOMSTR0_IRQn,
    IOMSTR1_IRQn,
    IOMSTR2_IRQn,
    IOMSTR3_IRQn,
    IOMSTR4_IRQn,
    IOMSTR5_IRQn,
    };

typedef struct
{
    uint8_t                 directSize;
    uint8_t                 roSize;
    uint8_t                 fifoSize;
    uint8_t                 ahbRamSize;
    uint8_t                 roBase;
    uint8_t                 ahbRamBase;
    am_widget_audio_spi_test_t testCfg;
    uint32_t                iom;
    uint32_t                i2cAddr;
    bool                    bSpi;
} am_widget_audio_spi_t;

am_widget_audio_spi_t g_sIosWidget;
//
// Status flags.
// Clear buffers by setting to this value when empty.
//
#define BUFFER_EMPTY_EE  (0xEE)
#define BUFFER_EMPTY_ED (0xED)
#define BUFFER_EMPTY_EC (0xEC)
//
// Value for one byte write/read test (188).
//
#define WRITE_READ_BYTE (0xBC)
#define ROBUFFER_INIT   (0x55)
#define AHBBUF_INIT     (0xAA)

//*****************************************************************************
//
// Disable and clean up IOM
//
//*****************************************************************************
static void
iom_clean_up(void)
{
    uint32_t                    ioIntEnable = 0;
    am_hal_iom_transfer_t       Transaction;
    am_hal_gpio_mask_t IntStatus;
    uint32_t IntNum = HANDSHAKE_IOM_PIN;
    //
    // Disable IOCTL interrupts
    //
    if (g_sIosWidget.bSpi)
    {
        Transaction.ui32InstrLen    = 1;
        Transaction.ui64Instr = 0xF8;
        Transaction.eDirection      = AM_HAL_IOM_TX;
        Transaction.uPeerInfo.ui32SpiChipSelect = 0;
        Transaction.ui32NumBytes    = 1;
        Transaction.pui32TxBuffer   = &ioIntEnable;
        Transaction.bContinue       = false;
        Transaction.ui8RepeatCount  = 0;
        Transaction.ui32PauseCondition = 0;
        Transaction.ui32StatusSetClr = 0;
        am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
    }
    else
    {
        Transaction.ui32InstrLen    = 1;
        Transaction.ui64Instr = 0x78;
        Transaction.eDirection      = AM_HAL_IOM_TX;
        Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
        Transaction.ui32NumBytes    = 1;
        Transaction.pui32TxBuffer   = &ioIntEnable;
        Transaction.bContinue       = false;
        Transaction.ui8RepeatCount  = 0;
        Transaction.ui32PauseCondition = 0;
        Transaction.ui32StatusSetClr = 0;
        am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
    }
    //
    // Turn off the IOM for this operation.
    //
    am_hal_iom_disable(g_pIOMHandle);

    if (g_sIosWidget.bSpi)
    {
        //
        // Deinit IOM SPI pins. Attributes are set in am_bsp_gpio.h.
        //
        am_bsp_iom_pins_disable(g_sIosWidget.iom, AM_HAL_IOM_SPI_MODE);
    }
    else
    {
        //
        // Deinit IOM I2C pins. Attributes are set in am_bsp_gpio.h.
        //
        am_bsp_iom_pins_disable(g_sIosWidget.iom, AM_HAL_IOM_I2C_MODE);
    }
    //
    // Disable interrupts for NB send to work
    //
    am_hal_iom_interrupt_disable(g_pIOMHandle, AM_HAL_IOM_INT_CMDCMP);
    NVIC_DisableIRQ(iomaster_interrupt[g_sIosWidget.iom]);
    //
    // Set up the host IO interrupt
    //
    am_hal_gpio_state_write(HANDSHAKE_IOM_PIN,AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_pinconfig(HANDSHAKE_IOM_PIN, g_AM_BSP_GPIO_DISABLE);
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_INT_CHANNEL_0, &IntStatus);
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                                  AM_HAL_GPIO_INT_CTRL_INDV_DISABLE,
                                  (void *)&IntNum);
    NVIC_DisableIRQ(GPIO0_001F_IRQn);
    //
    // Disable power to IOM.
    //
    am_hal_pwrctrl_periph_disable((am_hal_pwrctrl_periph_e)(AM_HAL_PWRCTRL_PERIPH_IOM0 + g_sIosWidget.iom));
}

//*****************************************************************************
//
// Set up IOM
//
//*****************************************************************************
static void
iom_set_up(am_widget_audio_stimulus_iom_t *pCfg)
{
#ifndef CODEC_MODE
    am_hal_iom_transfer_t       Transaction;
    uint32_t ioIntEnable = 0x3;
#endif
    uint32_t IntNum = HANDSHAKE_IOM_PIN;
    //
    // Enable power to IOM.
    //
    am_hal_pwrctrl_periph_enable((am_hal_pwrctrl_periph_e)(AM_HAL_PWRCTRL_PERIPH_IOM0 + pCfg->iomModule));
    //
    // Set the required configuration settings for the IOM.
    //
    am_hal_iom_initialize(pCfg->iomModule,&g_pIOMHandle);
    am_hal_iom_configure(g_pIOMHandle, &pCfg->iomHalCfg);

    if (g_sIosWidget.bSpi)
    {
        //
        // Set up IOM SPI pins. Attributes are set in am_bsp_gpio.h.
        //
        am_bsp_iom_pins_enable(pCfg->iomModule,AM_HAL_IOM_SPI_MODE);
    }
    else
    {
        am_bsp_iom_pins_enable(pCfg->iomModule,AM_HAL_IOM_I2C_MODE);
    }
#ifdef TEST_IOM_QUEUE
    am_hal_iom_queue_init(pCfg->iomModule,
        g_psQueueMemory, sizeof(g_psQueueMemory));
#endif
    am_hal_iom_interrupt_enable(g_pIOMHandle, 0xFF);
    NVIC_EnableIRQ(iomaster_interrupt[pCfg->iomModule]);
    //
    // Turn on the IOM for this operation.
    //
    am_hal_iom_enable(g_pIOMHandle);
    //
    // Set up the host IO interrupt
    //
    am_hal_gpio_pinconfig(HANDSHAKE_IOM_PIN, g_AM_BSP_IOM_GPIO_ENABLE);
    am_hal_gpio_state_write(HANDSHAKE_IOM_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                                  AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                                  (void *)&IntNum);
    NVIC_EnableIRQ(GPIO0_001F_IRQn);
#ifdef CODEC_MODE
    am_hal_gpio_pinconfig(MST_SEND_INT_TO_SLV_PIN, am_hal_gpio_pincfg_output);
    am_hal_gpio_state_write(MST_SEND_INT_TO_SLV_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
#endif
#ifndef CODEC_MODE
    //
    // Set up IOCTL interrupts
    //
    if (g_sIosWidget.bSpi)
    {
        Transaction.ui32InstrLen    = 1;
        Transaction.ui64Instr = 0xF8;
        Transaction.eDirection      = AM_HAL_IOM_TX;
        Transaction.uPeerInfo.ui32SpiChipSelect = 0;
        Transaction.ui32NumBytes    = 1;
        Transaction.pui32TxBuffer   = &ioIntEnable;
        Transaction.bContinue       = false;
        Transaction.ui8RepeatCount  = 0;
        Transaction.ui32PauseCondition = 0;
        Transaction.ui32StatusSetClr = 0;
        am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
    }
    else
    {
        Transaction.ui32InstrLen    = 1;
        Transaction.ui64Instr = 0x78;
        Transaction.eDirection      = AM_HAL_IOM_TX;
        Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
        Transaction.ui32NumBytes    = 1;
        Transaction.pui32TxBuffer   = &ioIntEnable;
        Transaction.bContinue       = false;
        Transaction.ui8RepeatCount  = 0;
        Transaction.ui32PauseCondition = 0;
        Transaction.ui32StatusSetClr = 0;
        am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
    }
#endif
}

//*****************************************************************************
//
// Set up UART
//
//*****************************************************************************
static void
uart_clean_up(void)
{
    uint32_t ui32ErrorStatus = AM_HAL_STATUS_FAIL;

    am_hal_gpio_pinconfig(GPIO_COM_UART_TX, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(GPIO_COM_UART_RX, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(GPIO_COM_UART_RTS, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(GPIO_COM_UART_CTS, am_hal_gpio_pincfg_disabled);
    //
    // Clear out any IOS register-access interrupts that may be active, and
    // enable interrupts for the registers we're interested in.
    //
    am_hal_uart_interrupt_clear(UART, 0xFFFFFFFF);
    am_hal_uart_interrupt_disable(UART, AM_HAL_UART_INT_TXCMP);
    //
    // Set the bit in the NVIC to accept access interrupts from the IO Slave.
    //
    NVIC_DisableIRQ((IRQn_Type) (UART0_IRQn));
    ui32ErrorStatus = am_hal_uart_deinitialize(UART);
    TEST_ASSERT(ui32ErrorStatus == 0);
    am_hal_pwrctrl_periph_disable((am_hal_pwrctrl_periph_e)(AM_HAL_PWRCTRL_PERIPH_UART0));
}

//*****************************************************************************
//
// Set up UART
//
//*****************************************************************************
static void
uart_set_up(am_widget_audio_stimulus_uart_t *pCfg)
{
    uint32_t ui32ErrorStatus = AM_HAL_STATUS_FAIL;

    am_hal_gpio_pinconfig(GPIO_COM_UART_TX, g_AM_BSP_GPIO_COM_UART_TX);
    am_hal_gpio_pinconfig(GPIO_COM_UART_RX, g_AM_BSP_GPIO_COM_UART_RX);
    am_hal_gpio_pinconfig(GPIO_COM_UART_RTS, g_AM_BSP_GPIO_COM_UART_RTS);
    am_hal_gpio_pinconfig(GPIO_COM_UART_CTS, g_AM_BSP_GPIO_COM_UART_CTS);

    ui32ErrorStatus = am_hal_uart_initialize(pCfg->uartModule, &UART);
    TEST_ASSERT(ui32ErrorStatus == 0);

    ui32ErrorStatus = am_hal_uart_power_control(UART, AM_HAL_SYSCTRL_WAKE, false);
    TEST_ASSERT(ui32ErrorStatus == 0);
    
    ui32ErrorStatus = am_hal_uart_configure(UART, &pCfg->uartHalCfg);
    TEST_ASSERT(ui32ErrorStatus == 0);
    //
    // Clear out any IOS register-access interrupts that may be active, and
    // enable interrupts for the registers we're interested in.
    //
    am_hal_uart_interrupt_clear(UART, 0xFFFFFFFF);
    am_hal_uart_interrupt_enable(UART, (AM_HAL_UART_INT_TX |
                                           AM_HAL_UART_INT_RX |
                                           AM_HAL_UART_INT_RX_TMOUT |
                                           AM_HAL_UART_INT_OVER_RUN |
                                           AM_HAL_UART_INT_TXCMP));
    //
    // Set the bit in the NVIC to accept access interrupts from the IO Slave.
    //
    NVIC_EnableIRQ((IRQn_Type) (UART0_IRQn + pCfg->uartModule));
}

//*****************************************************************************
//
// Disable and clean up IOS
//
//*****************************************************************************
static void
ios_clean_up(void)
{
    uint32_t ui32Arg = AM_HAL_IOS_ACCESS_INT_ALL;

    if (g_sIosWidget.bSpi)
    {
        //
        // Deinit SPI interface
        //
        am_bsp_ios_pins_disable(0, AM_HAL_IOS_USE_SPI);
    }
    else
    {
        //
        // Deinit I2C interface
        //
        am_bsp_ios_pins_disable(0, AM_HAL_IOS_USE_I2C);
    }
    //
    // Clear out any IOS register-access interrupts that may be active, and
    // enable interrupts for the registers we're interested in.
    //
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTDIS, &ui32Arg);
    am_hal_ios_interrupt_disable(g_pIOSHandle, AM_HAL_IOS_INT_ALL);
    //
    // Preparation of FIFO
    //
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_FIFO_BUF_CLR, NULL);
    //
    // Clear the bit in the NVIC
    //
    NVIC_DisableIRQ(IOSLAVEACC_IRQn);
    NVIC_DisableIRQ(IOSLAVE_IRQn);
    //
    // Clean up the interrupt pin
    //
    am_hal_gpio_pinconfig(HANDSHAKE_IOS_PIN, g_AM_BSP_GPIO_DISABLE);
    //
    // Disable IOS
    //
    am_hal_ios_disable(g_pIOSHandle);
}

#define AM_HAL_IOS_INT_ERR  (AM_HAL_IOS_INT_FOVFL | AM_HAL_IOS_INT_FUNDFL | AM_HAL_IOS_INT_FRDERR | AM_HAL_IOS_INT_GENAD)
#define AM_HAL_IOS_XCMP_INT (AM_HAL_IOS_INT_XCMPWR | AM_HAL_IOS_INT_XCMPWF | AM_HAL_IOS_INT_XCMPRR | AM_HAL_IOS_INT_XCMPRF)
//*****************************************************************************
//
// Configure the SPI slave.
//
//*****************************************************************************
static void
ios_set_up(am_widget_audio_stimulus_ios_t *pCfg)
{
    uint32_t ui32Arg = AM_HAL_IOS_ACCESS_INT_ALL;

    if (g_sIosWidget.bSpi)
    {
        //
        // Configure SPI interface
        //
        am_bsp_ios_pins_enable(pCfg->iosModule, AM_HAL_IOS_USE_SPI);
    }
    else
    {
        //
        // Configure I2C interface
        //
        am_bsp_ios_pins_enable(pCfg->iosModule, AM_HAL_IOS_USE_I2C);
    }
    //
    // Configure the IOS interface and LRAM structure.
    //
    am_hal_ios_initialize(pCfg->iosModule,&g_pIOSHandle);
    am_hal_ios_power_ctrl(g_pIOSHandle, AM_HAL_SYSCTRL_WAKE, false);
    am_hal_ios_configure(g_pIOSHandle, &pCfg->iosHalCfg);
    //
    // Clear out any IOS register-access interrupts that may be active, and
    // enable interrupts for the registers we're interested in.
    //
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTCLR, &ui32Arg);
    am_hal_ios_interrupt_clear(g_pIOSHandle, AM_HAL_IOS_INT_ALL);
#ifdef TEST_ACC_INT
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTEN, &ui32Arg);
#endif
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
    NVIC_EnableIRQ(IOSLAVEACC_IRQn);
    NVIC_EnableIRQ(IOSLAVE_IRQn);
    //
    // Set up the IOS interrupt pin (A GPIO)
    //
    am_hal_gpio_pinconfig(HANDSHAKE_IOS_PIN, am_hal_gpio_pincfg_output);

}

volatile uint32_t g_ui32AccIsr = 0;
volatile uint32_t g_ui32IosIsr = 0;
volatile bool bIomSendComplete = false;
volatile bool bIomSendToCodecComplete = false;
volatile bool bIosSendComplete = false;
volatile bool bIomRecvComplete = false;
uint32_t accIsrStatus[32] = {0};
uint32_t accIsrIdx = 0;

uint32_t missingAccIsr[32] = {0};
uint8_t g_randomIdx = 0;

//*****************************************************************************
//
// IO Slave Register Access ISR.
//
//*****************************************************************************
void
am_ioslave_acc_isr(void)
{
    uint32_t ui32Status;

#ifdef TEST_ACC_ISR_RANDOMIZE
    static uint8_t shift = 0;
    uint8_t randomVal = (g_random[g_randomIdx] ^ g_random[(g_randomIdx + 5) & 0xFF]) >> (shift++);
    g_randomIdx +=7 ;
    shift &= 0x3;
    ui32Status = AM_REGVAL(0x50000214);
    if (randomVal)
    {
        ((void (*)(uint32_t)) 0x0800009d)(randomVal);
    }
    AM_REGVAL(0x50000218) = ui32Status;
#else
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTGET, &ui32Status);
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTCLR, &ui32Status);
#endif
    accIsrStatus[accIsrIdx++] = ui32Status;
    g_ui32AccIsr |= ui32Status;
}

//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************
void
am_gpio0_001f_isr(void)
{
    am_hal_gpio_mask_t IntStatus;
    uint32_t    ui32IntStatus;

    am_hal_gpio_interrupt_status_get(AM_HAL_GPIO_INT_CHANNEL_0,
                                     false,
                                     &IntStatus);
    am_hal_gpio_interrupt_irq_status_get(GPIO0_001F_IRQn, false, &ui32IntStatus);
    am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);
    am_hal_gpio_interrupt_service(GPIO0_001F_IRQn,ui32IntStatus);
}

bool bHandshake = false;
uint32_t numHostInt = 0, numIoInt = 0;
//
// ISR callback for the host IOINT
//
static void hostint_handler(void *pArg)
{
    bHandshake = true;
    numHostInt++;
}

uint32_t ioInt[8];
uint32_t ioIntIdx = 0;
uint32_t numXCMPRR = 0;
uint32_t numXCMPWR = 0;
uint32_t numXCMPRF = 0;
uint32_t numXCMPWF = 0;
uint32_t numGenAd = 0;

//*****************************************************************************
//
// IO Slave Main ISR.
//
//*****************************************************************************
void
am_ioslave_ios_isr(void)
{
    uint32_t ui32Status;
    uint32_t ui32IntStatus;
    uint32_t ui32Arg = HANDSHAKE_IOM_TO_IOS;

    //
    // Check to see what caused this interrupt, then clear the bit from the
    // interrupt register.
    //
    am_hal_ios_interrupt_status_get(g_pIOSHandle, true, &ui32Status);

    am_hal_ios_interrupt_clear(g_pIOSHandle, ui32Status);

    g_ui32IosIsr |= ui32Status;

    if (ui32Status & AM_HAL_IOS_INT_XCMPWR)
    {
        //
        // Accumulate data from Register space
        //
        numXCMPWR++;
    }
    if (ui32Status & AM_HAL_IOS_INT_XCMPWF)
    {
        //
        // Accumulate data from FIFO
        //
        numXCMPWF++;
    }
    if (ui32Status & AM_HAL_IOS_INT_XCMPRR)
    {
        //
        // Host completed Read from register space
        //
        numXCMPRR++;
    }
    if (ui32Status & AM_HAL_IOS_INT_XCMPRF)
    {
        //
        // Host completed Read from FIFO
        //
        numXCMPRF++;
    }
    if (ui32Status & AM_HAL_IOS_INT_IOINTW)
    {
        //
        // Host interrupting us
        // Read the IOINT register for appropriate bits
        // Set bIomSendComplete based on this
        //
        am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_HOST_INTGET, &ui32IntStatus);
        if (ui32IntStatus & ui32Arg)
        {
            bIomSendComplete = true;
            am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_HOST_INTCLR, &ui32Arg);
        }
    }
    if (ui32Status & AM_HAL_IOS_INT_FUNDFL)
    {
        am_util_stdio_printf("Hitting underflow for the requested IOS transfer.\n");
    }
    if (ui32Status & AM_HAL_IOS_INT_FOVFL)
    {
        am_util_stdio_printf("Hitting overflow for the requested IOS transfer.\n");
    }
    if (ui32Status & AM_HAL_IOS_INT_FRDERR)
    {
        am_util_stdio_printf("Hitting Read Error for the requested IOS transfer.\n");
    }

    if (ui32Status & AM_HAL_IOS_INT_FSIZE)
    {
        //
        // Service the I2C slave FIFO if necessary.
        //
        am_hal_ios_interrupt_service(g_pIOSHandle, ui32Status);
    }

    if (ui32Status & AM_HAL_IOS_INT_GENAD)
    {
        numGenAd++;
    }

}

//*****************************************************************************
//
// Take over default ISR for IOM 0. (Queue mode service)
//
//*****************************************************************************
void
am_iomaster0_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle, true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle, ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle, ui32Status);
        }
    }
}

//*****************************************************************************
//
// Take over default ISR for IOM 1. (Queue mode service)
//
//*****************************************************************************
void
am_iomaster1_isr(void)
{
    uint32_t ui32Status;

    am_hal_iom_interrupt_status_get(g_pIOMHandle, true, &ui32Status);

    am_hal_iom_interrupt_clear(g_pIOMHandle, ui32Status);
}

//*****************************************************************************
//
// Take over default ISR for IOM 2. (Queue mode service)
//
//*****************************************************************************
void
am_iomaster2_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle, true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle, ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle, ui32Status);
        }
    }
}

//*****************************************************************************
//
// Take over default ISR for IOM 3. (Queue mode service)
//
//*****************************************************************************
void
am_iomaster3_isr(void)
{
    uint32_t ui32Status;

    am_hal_iom_interrupt_status_get(g_pIOMHandle, true, &ui32Status);

    am_hal_iom_interrupt_clear(g_pIOMHandle, ui32Status);
}

//*****************************************************************************
//
// Take over default ISR for IOM 4. (Queue mode service)
//
//*****************************************************************************
void
am_iomaster4_isr(void)
{
    uint32_t ui32Status;

    am_hal_iom_interrupt_status_get(g_pIOMHandle, true, &ui32Status);

    am_hal_iom_interrupt_clear(g_pIOMHandle, ui32Status);
}

//*****************************************************************************
//
// Take over default ISR for IOM 5. (Queue mode service)
//
//*****************************************************************************
void
am_iomaster5_isr(void)
{
    uint32_t ui32Status;

    am_hal_iom_interrupt_status_get(g_pIOMHandle, true, &ui32Status);

    am_hal_iom_interrupt_clear(g_pIOMHandle, ui32Status);
}

//*****************************************************************************
//
// UART0 interrupt handler.
//
//*****************************************************************************
void
am_uart_isr(void)
{
    //
    // Service the FIFOs as necessary, and clear the interrupts.
    //
    uint32_t ui32Status;

    am_hal_uart_interrupt_status_get(UART, &ui32Status, true);
    am_hal_uart_interrupt_clear(UART, ui32Status);
    am_hal_uart_interrupt_service(UART, ui32Status);

    ui32TXDoneFlag = true;
}

void
receive_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("\nIOM Read Failed 0x%x\n", transactionStatus);
    }
    else
    {
        bIomRecvComplete = true;
    }
}

//*****************************************************************************
//
// Empty the buffers.
// Load output buffer with repeated text string.
//
//*****************************************************************************
uint32_t clear_buffers(void)
{
    uint32_t i = 0;
    for (i = 0; i < sizeof(g_pIomRecvBuf); i++)
    {
        g_pIomRecvBuf[i] = BUFFER_EMPTY_ED;
    }
    for (i = 0; i < sizeof(g_pIosSendBuf); i++)
    {
        g_pIosSendBuf[i] = BUFFER_EMPTY_EC;
    }
    for (i = 0; i < sizeof(g_pIosRecvBuf); i++)
    {
        g_pIosRecvBuf[i] = BUFFER_EMPTY_ED;
    }
    for (i = 0; i < sizeof(g_pIomSendBuf); i++)
    {
        g_pIomSendBuf[i] = BUFFER_EMPTY_EE;
    }
    for (i = 0; i < sizeof(g_pIomRecvPacket); i++)
    {
    g_pIomRecvPacket[i] = BUFFER_EMPTY_EE;
    }
    for (i = 0; i < sizeof(g_pUartRecvPacket); i++)
    {
        g_pUartRecvPacket[i] = BUFFER_EMPTY_ED;
    }
    for (i = 0; i < sizeof(g_pIosRecvPacket); i++)
    {
        g_pIosRecvPacket[i] = BUFFER_EMPTY_EC;
    }
    return i; // Return number of bytes loaded.
}

//*****************************************************************************
//
// Prepare data for IOS sending
//
//*****************************************************************************
static uint32_t load_ios_send_data(uint8_t * pui8SrcData, uint32_t ui32Size)
{
    uint32_t i = 0;
    g_pIosSendBuf[0] = (uint8_t)ui32Size; // first byte and second byte are length bytes
    g_pIosSendBuf[1] = (uint8_t)(ui32Size >> 8);
    for (i = 0; i < ui32Size; i++)
    {
        g_pIosSendBuf[i + 2] = pui8SrcData[i];
    }
    return i; // Return number of bytes loaded.
}

//*****************************************************************************
//
// Store IOS received data from IOS RX buffer to another region
//
//*****************************************************************************
static uint32_t store_ios_recv_data(uint8_t * pui8StoreAddr, uint32_t ui32Size)
{
    uint32_t i = 0;
    for (i = 0; i < ui32Size; i++)
    {
        pui8StoreAddr[i] = g_pIosRecvBuf[i + 2];
    }
    return i; // Return number of bytes storeed.
}

//*****************************************************************************
//
// Prepare data for IOM sending
//
//*****************************************************************************
static uint32_t load_iom_send_data(uint8_t * pui8SrcData, uint32_t ui32Size)
{
    uint32_t i = 0;
    g_pIomSendBuf[0] = (uint8_t)ui32Size; // first byte and second byte are length bytes
    g_pIomSendBuf[1] = (uint8_t)(ui32Size >> 8);
    for (i = 0; i < ui32Size; i++)
    {
        g_pIomSendBuf[i + 2] = pui8SrcData[i];
    }
    return i; // Return number of bytes loaded.
}

#ifdef CODEC_MODE
//*****************************************************************************
//
// Prepare data for IOM sending to codec
//
//*****************************************************************************
static uint32_t load_iom_send_data_to_codec(uint8_t * pui8SrcData, uint32_t ui32Size)
{
    uint32_t i = 0;
    uint32_t ui32Offset;

    if (g_sIosWidget.bSpi)
    {
        ui32Offset = CODEC_PADDING_BYTES;
        for (i = 0; i < CODEC_PADDING_BYTES; i++)
        {
            g_pIomSendBuf[i] = 0x0; // padding bytes
        }
    }
    else
    {
        ui32Offset = 0;
    }

    for (i = 0; i < ui32Size / 3; i++)
    {
        //
        // fill 1 most significant byte of a 32-bit word to 0x00, change endian. 
        //
        g_pIomSendBuf[i * 4 + ui32Offset] = 0x0;
        g_pIomSendBuf[i * 4 + ui32Offset + 1] = pui8SrcData[i * 3 + 2];
        g_pIomSendBuf[i * 4 + ui32Offset + 2] = pui8SrcData[i * 3 + 1];
        g_pIomSendBuf[i * 4 + ui32Offset + 3] = pui8SrcData[i * 3];
    }
    if ((ui32Size % 3) == 1)
    {
        g_pIomSendBuf[i * 4 + ui32Offset] = 0x0;
        g_pIomSendBuf[i * 4 + ui32Offset + 1] = 0x0;
        g_pIomSendBuf[i * 4 + ui32Offset + 2] = 0x0;
        g_pIomSendBuf[i * 4 + ui32Offset + 3] = pui8SrcData[i * 3];
    }
    if ((ui32Size % 3) == 2)
    {
        g_pIomSendBuf[i * 4 + ui32Offset] = 0x0;
        g_pIomSendBuf[i * 4 + ui32Offset + 1] = 0x0;
        g_pIomSendBuf[i * 4 + ui32Offset + 2] = pui8SrcData[i * 3 + 1];
        g_pIomSendBuf[i * 4 + ui32Offset + 3] = pui8SrcData[i * 3];
    }
    return i; // Return number of bytes loaded.
}
#endif

//*****************************************************************************
//
// Store IOM received data from IOM RX buffer to another region
//
//*****************************************************************************
static uint32_t store_iom_recv_data(uint8_t * pui8StoreAddr, uint32_t ui32Size)
{
    uint32_t i = 0; // Return number of bytes storeed.
    for (i = 0; i < ui32Size; i++)
    {
        pui8StoreAddr[i] = g_pIomRecvBuf[i + 2];
    }
    return i;
}

#ifdef CODEC_MODE
//*****************************************************************************
//
// Store IOM received data from IOM RX buffer to another region
//
//*****************************************************************************
static uint32_t store_iom_recv_data_from_codec(uint8_t * pui8StoreAddr, uint32_t ui32Size)
{
    uint32_t i = 0; // Return number of bytes storeed.
    uint32_t ui32Offset;

    if (g_sIosWidget.bSpi)
    {
        ui32Offset = CODEC_PADDING_BYTES;
    }
    else
    {
        ui32Offset = 0;
    }

    for (i = 0; i < ui32Size / 3; i++)
    {
        //
        // remove unused 2 padding bytes, 1 most significant byte of a 32-bit word, change endian  
        //
        pui8StoreAddr[i * 3] = g_pIomRecvBuf[i * 4 + ui32Offset + 3];
        pui8StoreAddr[i * 3 + 1] = g_pIomRecvBuf[i * 4 + ui32Offset + 2];
        pui8StoreAddr[i * 3 + 2] = g_pIomRecvBuf[i * 4 + ui32Offset + 1];
    }
    if ((ui32Size % 3) == 1)
    {
        pui8StoreAddr[i * 3] = g_pIomRecvBuf[i * 4 + ui32Offset + 3];
    }
    if ((ui32Size % 3) == 2)
    {
        pui8StoreAddr[i * 3] = g_pIomRecvBuf[i * 4 + ui32Offset + 3];
        pui8StoreAddr[i * 3 + 1] = g_pIomRecvBuf[i * 4  + ui32Offset + 2];
    }
    return i;
}
#endif

//*****************************************************************************
//
// Verify received data against the reference
// return non-zero if fail
//
//*****************************************************************************
uint32_t
verify_result(uint32_t ui32Length, uint8_t * pui8Input, uint8_t * pui8Output)
{
    uint32_t i;
    for (i = 0; i < ui32Length; i++)
    {
        if (pui8Input[i] != pui8Output[i])
        {
            return 1;
        }
    }

    return 0;
}
//*****************************************************************************
//
// Notify the slave
//
//*****************************************************************************
static void iom_send_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
#ifdef CODEC_MODE
    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("\nIOM Write Failed 0x%x\n", transactionStatus);
    }
    else
    {
        bIomSendToCodecComplete = true;
    }
#else
    am_hal_iom_transfer_t       Transaction;

    uint32_t data = 0x1;
    //
    // Send data using the FIFO
    // In the completion callback - write to IOS to trigger IOINT
    //
    if (g_sIosWidget.bSpi)
    {
      Transaction.ui32InstrLen    = 1;
      Transaction.ui64Instr = 0xFB;
      Transaction.eDirection      = AM_HAL_IOM_TX;
      Transaction.uPeerInfo.ui32SpiChipSelect = 0;
      Transaction.ui32NumBytes    = 1;
      Transaction.pui32TxBuffer   = &data;
      Transaction.bContinue       = false;
      Transaction.ui8RepeatCount  = 0;
      Transaction.ui32PauseCondition = 0;
      Transaction.ui32StatusSetClr = 0;
      am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
    }
    else
    {
      Transaction.ui32InstrLen    = 1;
      Transaction.ui64Instr = 0x7B;
      Transaction.eDirection      = AM_HAL_IOM_TX;
      Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
      Transaction.ui32NumBytes    = 1;
      Transaction.pui32TxBuffer   = &data;
      Transaction.bContinue       = false;
      Transaction.ui8RepeatCount  = 0;
      Transaction.ui32PauseCondition = 0;
      Transaction.ui32StatusSetClr = 0;
      am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
    }
#endif
}

//*****************************************************************************
//
// IOM receives data
//
//*****************************************************************************
static void
iom_recv(uint32_t address, uint32_t size)
{
  am_hal_iom_transfer_t       Transaction;

  if (g_sIosWidget.bSpi)
  {
#ifdef CODEC_MODE
    Transaction.ui32InstrLen    = 4;
    Transaction.ui32NumBytes    = size + CODEC_PADDING_BYTES;
#else
    Transaction.ui32InstrLen    = 1;
    Transaction.ui32NumBytes    = size;
#endif
    Transaction.ui64Instr = address;
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.uPeerInfo.ui32SpiChipSelect = 0;
    Transaction.pui32RxBuffer   = (uint32_t *)g_pIomRecvBuf;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    am_hal_iom_nonblocking_transfer(g_pIOMHandle, &Transaction, receive_complete, 0);
  }
  else
  {
#ifdef CODEC_MODE
    Transaction.ui32InstrLen    = 4;
#else
    Transaction.ui32InstrLen    = 1;
#endif
    Transaction.ui64Instr = address;
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
    Transaction.ui32NumBytes    = size;
    Transaction.pui32RxBuffer   = (uint32_t *)g_pIomRecvBuf;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    am_hal_iom_nonblocking_transfer(g_pIOMHandle, &Transaction, receive_complete, 0);
  }
}

//*****************************************************************************
//
// IOM sends data
//
//*****************************************************************************
static void
iom_send(uint32_t address, uint32_t size)
{
    am_hal_iom_transfer_t       Transaction;

    if (g_sIosWidget.bSpi)
    {
#ifdef CODEC_MODE
      Transaction.ui32InstrLen    = 4;
      Transaction.ui32NumBytes    = size + CODEC_PADDING_BYTES;
      Transaction.ui64Instr = (0x80000000|address);
#else
      Transaction.ui32InstrLen    = 1;
      Transaction.ui64Instr = (0x80|address);
      Transaction.ui32NumBytes    = size;
#endif
      Transaction.eDirection      = AM_HAL_IOM_TX;
      Transaction.uPeerInfo.ui32SpiChipSelect = 0;
      Transaction.pui32TxBuffer   = (uint32_t *)g_pIomSendBuf;
      Transaction.bContinue       = false;
      Transaction.ui8RepeatCount  = 0;
      Transaction.ui32PauseCondition = 0;
      Transaction.ui32StatusSetClr = 0;
      am_hal_iom_nonblocking_transfer(g_pIOMHandle, &Transaction, iom_send_complete, 0);
    }
    else
    {
#ifdef CODEC_MODE
      Transaction.ui32InstrLen    = 4;
#else
      Transaction.ui32InstrLen    = 1;
#endif
      Transaction.ui64Instr       = address;
      Transaction.eDirection      = AM_HAL_IOM_TX;
      Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
      Transaction.ui32NumBytes    = size;
      Transaction.pui32TxBuffer   = (uint32_t *)g_pIomSendBuf;
      Transaction.bContinue       = false;
      Transaction.ui8RepeatCount  = 0;
      Transaction.ui32PauseCondition = 0;
      Transaction.ui32StatusSetClr = 0;
      am_hal_iom_nonblocking_transfer(g_pIOMHandle, &Transaction, iom_send_complete, 0);
    }

}

//*****************************************************************************
//
// Move data from IOS LRAM to a buffer
//
//*****************************************************************************
static void ios_read(uint32_t address, uint32_t size)
{
    uint32_t i;

    for (i = 0; i < size; i++)
    {
        g_pIosRecvBuf[i] = am_hal_ios_pui8LRAM[address+i];
    }
}

//*****************************************************************************
//
// Prepare data form slave (IOS) to host (IOM)
//
//*****************************************************************************
static uint32_t ios_send(uint32_t address, uint32_t size)
{
    uint32_t ui32SentSize = 0;
    uint32_t ui32Arg = HANDSHAKE_IOS_TO_IOM;
    //
    // Send data using the LRAM or FIFO
    //
    if (address == 0x7F)
    {
        am_hal_ios_fifo_write(g_pIOSHandle, &g_pIosSendBuf[0], size, &ui32SentSize);
        am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_FIFO_UPDATE_CTR, NULL);
    }
    else
    {
        while (size--)
        {
            am_hal_ios_pui8LRAM[address+size] = g_pIosSendBuf[size];
        }
    }
    //
    // Notify the host
    //
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_HOST_INTSET, &ui32Arg);
    am_hal_gpio_state_write(HANDSHAKE_IOS_PIN,AM_HAL_GPIO_OUTPUT_SET);
    am_hal_gpio_state_write(HANDSHAKE_IOS_PIN,AM_HAL_GPIO_OUTPUT_CLEAR);

    return 0;
}

//*****************************************************************************
//
// UART sends data
//
//*****************************************************************************
static uint32_t uart_send(am_widget_uart_send_config_t *psConfig)
{
    am_hal_uart_transfer_t  sTransaction;
    uint32_t ui32Status;
    ui32TXDoneFlag = false;
    //
    // Data location to use for this transaction.
    //
    sTransaction.pui8Data = psConfig->pui8SpaceTx;
    //
    // TX buffer size
    //
    sTransaction.ui32NumBytes = psConfig->txbufferSize;
    //
    // When the transaction is complete, this will be set to the number of
    // bytes we read.
    //
    sTransaction.pui32BytesTransferred = &psConfig->txBytesTransferred;
    //
    // For blocking transactions, this determines how long the UART HAL should
    // wait before aborting the transaction.
    //
    if (psConfig->txblocking)
    {
        sTransaction.eType = AM_HAL_UART_BLOCKING_WRITE;
        sTransaction.ui32TimeoutMs = AM_HAL_UART_WAIT_FOREVER;
    }
    else
    {
        sTransaction.eType = AM_HAL_UART_NONBLOCKING_WRITE;
        sTransaction.ui32TimeoutMs = 0;
    }
    sTransaction.pfnCallback = NULL;
    sTransaction.ui32ErrorStatus=0;
    ui32Status = am_hal_uart_transfer(UART, &sTransaction);
    ui32Status = am_hal_uart_tx_flush(UART); //FIXME

    if ( *sTransaction.pui32BytesTransferred != psConfig->length  )
    {
        //
        // Didn't send all of the data we were supposed to.
        //
        return 2;
    }
    while (ui32TXDoneFlag == false);
    return ui32Status;
}

//*****************************************************************************
//
// UART receives data
//
//*****************************************************************************
static uint32_t uart_recv(am_widget_uart_recv_config_t *psConfig)
{
    am_hal_uart_transfer_t  sTransaction;
    uint32_t ui32Status;
    //
    // Data location to use for this transaction.
    //
    sTransaction.pui8Data = psConfig->pui8SpaceRx;
    //
    // RX buffer size
    //
    sTransaction.ui32NumBytes = psConfig->rxbufferSize;
    //
    // When the transaction is complete, this will be set to the number of
    // bytes we read.
    //
    sTransaction.pui32BytesTransferred = &psConfig->rxBytesTransferred;
    //
    // For blocking transactions, this determines how long the UART HAL should
    // wait before aborting the transaction.
    //
    if (psConfig->rxblocking)
    {
        sTransaction.eType = AM_HAL_UART_BLOCKING_READ;
        sTransaction.ui32TimeoutMs = AM_HAL_UART_WAIT_FOREVER;
    }
    else
    {
        sTransaction.eType = AM_HAL_UART_NONBLOCKING_READ;
        sTransaction.ui32TimeoutMs = 0;
    }

    ui32Status = am_hal_uart_transfer(UART, &sTransaction);
    return ui32Status;
}

//
// We need to shift one extra
//
#define GET_I2CADDR(cfg)    \
    (((cfg) & IOSLAVE_CFG_I2CADDR_Msk) >> (IOSLAVE_CFG_I2CADDR_Pos+1))

//*****************************************************************************
//
// IOM, IOS and UART setup
//
//*****************************************************************************
uint32_t am_widget_audio_setup(am_widget_audio_config_t *pAudioCfg,
    void **ppWidget)
{
    uint32_t ui32Status = 0, i;
    g_sIosWidget.directSize = g_sIosWidget.roBase =
        pAudioCfg->stimulusCfgIos.iosHalCfg.ui32ROBase;
    g_sIosWidget.roSize =
        pAudioCfg->stimulusCfgIos.iosHalCfg.ui32FIFOBase - pAudioCfg->stimulusCfgIos.iosHalCfg.ui32ROBase;
    g_sIosWidget.fifoSize =
        pAudioCfg->stimulusCfgIos.iosHalCfg.ui32RAMBase - pAudioCfg->stimulusCfgIos.iosHalCfg.ui32FIFOBase;
    g_sIosWidget.ahbRamBase = pAudioCfg->stimulusCfgIos.iosHalCfg.ui32RAMBase;
    g_sIosWidget.ahbRamSize = 0x100 - g_sIosWidget.ahbRamBase;
    g_sIosWidget.iom = pAudioCfg->stimulusCfgIom.iomModule;
    pAudioCfg->stimulusCfgIos.iosHalCfg.pui8SRAMBuffer = g_pui8TxFifoBuffer;
    pAudioCfg->stimulusCfgIos.iosHalCfg.ui32SRAMBufferCap = IOM_IOS_BUFSIZE_MAX;
    g_sIosWidget.bSpi =
        (_FLD2VAL(IOSLAVE_CFG_IFCSEL, pAudioCfg->stimulusCfgIos.iosHalCfg.ui32InterfaceSelect));
    g_sIosWidget.i2cAddr =
        GET_I2CADDR(pAudioCfg->stimulusCfgIos.iosHalCfg.ui32InterfaceSelect);
    am_hal_interrupt_master_enable();
    //
    // Set up the IOS
    //
    ios_set_up(&pAudioCfg->stimulusCfgIos);
    //
    // Set up the stimulus IOM
    //
    iom_set_up(&pAudioCfg->stimulusCfgIom);
    //
    // Set up the stimulus UART
    //
    uart_set_up(&pAudioCfg->stimulusCfgUart);
#ifdef CODEC_MODE
    //
    // Init Codec here if needed
    //

#endif
    //
    // transfer configurations
    //
    *ppWidget = &g_sIosWidget;
    //
    // Register handler for IOS => IOM interrupt
    //
    am_hal_gpio_interrupt_register(AM_HAL_GPIO_INT_CHANNEL_0, HANDSHAKE_IOM_PIN, hostint_handler, NULL);
    //
    // Initialize RO & AHB-RAM data with pattern
    //
    for (i = 0; i < g_sIosWidget.roSize; i++)
    {
        am_hal_ios_pui8LRAM[g_sIosWidget.roBase + i] = ROBUFFER_INIT;
    }
    for (i = 0; i < g_sIosWidget.ahbRamSize; i++)
    {
        am_hal_ios_pui8LRAM[g_sIosWidget.ahbRamBase + i] = AHBBUF_INIT;
    }
    return ui32Status;
}

//*****************************************************************************
//
// Change Endian of a 4-byte word
//
//*****************************************************************************
uint32_t change_endian(uint32_t ui32Input)
{
    uint32_t ui32Output = 0;
    ui32Output += (ui32Input & 0xFF000000) >> 24;
    ui32Output += (ui32Input & 0x00FF0000) >> 8;
    ui32Output += (ui32Input & 0x0000FF00) << 8;
    ui32Output += (ui32Input & 0x000000FF) << 24;
    
    return ui32Output;
}

//*****************************************************************************
//
// Remove 2 padding bytes for communication with CODEC
//
//*****************************************************************************
void remove_padding_bytes(uint8_t * pui8Data)
{
    uint32_t i;
    for (i = 0; i < 8; i++)
    {
        pui8Data[i] = pui8Data[i + 2];
    }
}

//*****************************************************************************
//
// Add 2 padding bytes for communication with CODEC
//
//*****************************************************************************
void add_padding_bytes(uint8_t * pui8Data)
{
    int32_t i;
    for (i = 7; i >= 0; i--)
    {
        pui8Data[i + 2] = pui8Data[i];
    }
    pui8Data[1] = 0x0;
    pui8Data[0] = 0x0;
}

#ifdef CODEC_MODE
//*****************************************************************************
//
// IOM reads packet from CODEC
//
//*****************************************************************************
uint32_t
am_widget_audio_receive_packet_from_codec(uint8_t * pui8PacketAddr, uint32_t * pui32BuffSize)
{
    am_hal_iom_transfer_t       Transaction;
    uint32_t i, ui32Loop, ui32FrameSize, ui32Status = 0;
    uint32_t ui32CodecBuffAddr;
    uint32_t data[CONTROL_WORDS_LENGTH + 1] = {0}; // control words and 2 padding bytes
    //
    // Handshake will use the interrupt IOCTL - to signal host that data is ready
    //
    while (1)
    {
        //
        // Disable interrupts before checking the flags
        //
        uint32_t ui32IntStatus = am_hal_interrupt_master_disable();
        if (bIosSendComplete)
        {
            //
            // re-enable interrupts
            //
            am_hal_interrupt_master_set(ui32IntStatus);
            break;
        }
#ifdef DEBUG_WITHOUT_CODEC
        if (1)
#else
        if (bHandshake == true)
#endif
        {
            //
            // re-enable interrupts
            //
            am_hal_interrupt_master_set(ui32IntStatus);
            bHandshake = false;
            //
            // Read & Clear the slave data ready bit in control word in codec
            //
            if (g_sIosWidget.bSpi)
            {
                Transaction.ui32InstrLen    = 4;
                Transaction.ui64Instr = CONTROL_WORD_ADDR | RW_BIT_MASK;
                Transaction.eDirection      = AM_HAL_IOM_RX;
                Transaction.uPeerInfo.ui32SpiChipSelect = 0;
                Transaction.ui32NumBytes    = CONTROL_WORDS_LENGTH * 4 + CODEC_PADDING_BYTES;
                Transaction.pui32RxBuffer   = data;
                Transaction.bContinue       = false;
                Transaction.ui8RepeatCount  = 0;
                Transaction.ui32PauseCondition = 0;
                Transaction.ui32StatusSetClr = 0;
                am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
                remove_padding_bytes((uint8_t *)data);
                //
                // change endian
                //
                data[0] = change_endian(data[0]);
                data[1] = change_endian(data[1]);
#ifdef DEBUG_WITHOUT_CODEC
                data[0] = 0x0002FF01;
                data[1] = 0x00123456;
#endif
                //
                // We need to clear the bit by writing to contol word in codec
                //
                if (data[0] & SLAVE_REQUEST_READ_MSK)
                {
                    *pui32BuffSize = (data[0] >> 8); // number of bytes
                    ui32CodecBuffAddr = data[1];
                    data[0] &= ~SLAVE_REQUEST_READ_MSK;
                    data[0] = change_endian(data[0]);
                    add_padding_bytes((uint8_t *)data);
                    Transaction.ui32InstrLen    = 4;
                    Transaction.ui64Instr = CONTROL_WORD_ADDR & ~RW_BIT_MASK;
                    Transaction.eDirection      = AM_HAL_IOM_TX;
                    Transaction.uPeerInfo.ui32SpiChipSelect = 0;
                    Transaction.ui32NumBytes    = 4 + CODEC_PADDING_BYTES;
                    Transaction.pui32TxBuffer   = data;
                    Transaction.bContinue       = false;
                    Transaction.ui8RepeatCount  = 0;
                    Transaction.ui32PauseCondition = 0;
                    Transaction.ui32StatusSetClr = 0;
                    am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
                    //
                    // Set bIosSendComplete
                    //
                    bIosSendComplete = true;
                    break;
                }
            }
            else
            {
                Transaction.ui32InstrLen    = 4;
                Transaction.ui64Instr = CONTROL_WORD_ADDR;
                Transaction.eDirection      = AM_HAL_IOM_RX;
                Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
                Transaction.ui32NumBytes    = CONTROL_WORDS_LENGTH * 4;
                Transaction.pui32RxBuffer   = data;
                Transaction.bContinue       = false;
                Transaction.ui8RepeatCount  = 0;
                Transaction.ui32PauseCondition = 0;
                Transaction.ui32StatusSetClr = 0;
                am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
                //
                // change endian
                //
                data[0] = change_endian(data[0]);
                data[1] = change_endian(data[1]);
                //
                // We need to clear the bit by writing to IOS
                //
                if (data[0] & SLAVE_REQUEST_READ_MSK)
                {
                    *pui32BuffSize = (data[0] >> 8);
                    ui32CodecBuffAddr = data[1];
                    data[0] &= ~SLAVE_REQUEST_READ_MSK;
                    data[0] = change_endian(data[0]);
                    Transaction.ui32InstrLen    = 4;
                    Transaction.ui64Instr = CONTROL_WORD_ADDR;
                    Transaction.eDirection      = AM_HAL_IOM_TX;
                    Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
                    Transaction.ui32NumBytes    = 4;
                    Transaction.pui32TxBuffer   = data;
                    Transaction.bContinue       = false;
                    Transaction.ui8RepeatCount  = 0;
                    Transaction.ui32PauseCondition = 0;
                    Transaction.ui32StatusSetClr = 0;
                    am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
                    //
                    // Set bIosSendComplete
                    //
                    bIosSendComplete = true;
                    break;
                }
            }
        }
        else
        {
            am_hal_interrupt_master_set(ui32IntStatus);
        }
    }
    bIosSendComplete = false;
    ui32Loop = ((*pui32BuffSize + 2) / 3 * 4 + IOM_IOS_AVAILABLE_BUFSIZE_MAX - 1) / IOM_IOS_AVAILABLE_BUFSIZE_MAX;
    for (i = 0; i < ui32Loop;i++)
    {
        bIomRecvComplete = false;
        if (i < ui32Loop - 1)
        {
            ui32FrameSize = IOM_IOS_AVAILABLE_BUFSIZE_MAX;
        }
        else
        {
            ui32FrameSize = ((*pui32BuffSize + 2) / 3 * 4)  - (ui32Loop - 1) * IOM_IOS_AVAILABLE_BUFSIZE_MAX;
        }
        iom_recv(ui32CodecBuffAddr + i * (IOM_IOS_AVAILABLE_BUFSIZE_MAX / 2), ui32FrameSize);
        
        while (1)
        {
            //
            // Disable interrupts before checking the flags
            //
            uint32_t ui32IntStatus = am_hal_interrupt_master_disable();
            if (bIomRecvComplete)
            {
                if (i == ui32Loop - 1)
                {
                    //
                    // Set master read complete flag bit.
                    //
                    if (g_sIosWidget.bSpi)
                    {
                        Transaction.ui32InstrLen    = 4;
                        Transaction.ui64Instr = CONTROL_WORD_ADDR | RW_BIT_MASK;
                        Transaction.eDirection      = AM_HAL_IOM_RX;
                        Transaction.uPeerInfo.ui32SpiChipSelect = 0;
                        Transaction.ui32NumBytes    = 4 + CODEC_PADDING_BYTES;
                        Transaction.pui32RxBuffer   = data;
                        Transaction.bContinue       = false;
                        Transaction.ui8RepeatCount  = 0;
                        Transaction.ui32PauseCondition = 0;
                        Transaction.ui32StatusSetClr = 0;
                        am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);

                        remove_padding_bytes((uint8_t *)data);
                        data[0] |= change_endian(MASTER_READ_COMPLETE_MSK);
                        add_padding_bytes((uint8_t *)data);
                        Transaction.ui32InstrLen    = 4;
                        Transaction.ui64Instr = CONTROL_WORD_ADDR & ~RW_BIT_MASK;
                        Transaction.eDirection      = AM_HAL_IOM_TX;
                        Transaction.uPeerInfo.ui32SpiChipSelect = 0;
                        Transaction.ui32NumBytes    = 4 + CODEC_PADDING_BYTES;
                        Transaction.pui32TxBuffer   = data;
                        Transaction.bContinue       = false;
                        Transaction.ui8RepeatCount  = 0;
                        Transaction.ui32PauseCondition = 0;
                        Transaction.ui32StatusSetClr = 0;
                        am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
                    }
                    else
                    {
                        Transaction.ui32InstrLen    = 4;
                        Transaction.ui64Instr = CONTROL_WORD_ADDR;
                        Transaction.eDirection      = AM_HAL_IOM_RX;
                        Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
                        Transaction.ui32NumBytes    = 4;
                        Transaction.pui32RxBuffer   = data;
                        Transaction.bContinue       = false;
                        Transaction.ui8RepeatCount  = 0;
                        Transaction.ui32PauseCondition = 0;
                        Transaction.ui32StatusSetClr = 0;
                        am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);

                        data[0] |= change_endian(MASTER_READ_COMPLETE_MSK);
                        Transaction.ui32InstrLen    = 4;
                        Transaction.ui64Instr = CONTROL_WORD_ADDR;
                        Transaction.eDirection      = AM_HAL_IOM_TX;
                        Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
                        Transaction.ui32NumBytes    = 4;
                        Transaction.pui32TxBuffer   = data;
                        Transaction.bContinue       = false;
                        Transaction.ui8RepeatCount  = 0;
                        Transaction.ui32PauseCondition = 0;
                        Transaction.ui32StatusSetClr = 0;
                        am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
                    }
                }
                //
                // re-enable interrupts
                //
                am_hal_interrupt_master_set(ui32IntStatus);
                break;
            }
            //
            // re-enable interrupts
            //
            am_hal_interrupt_master_set(ui32IntStatus);
        }
#ifdef DEBUG_WITHOUT_CODEC
        uint32_t j;
        for (j = 0; j < IOM_IOS_BUFSIZE_MAX; j++)
        {
            g_pIomRecvBuf[j] = 0x5A; // empty flag
        }
        g_pIomRecvBuf[0] = 0;
        g_pIomRecvBuf[1] = 0;
        for (j = 0; j < ui32FrameSize; j++)
        {
            g_pIomRecvBuf[j + 2] = j % 0xFF;
        }
#endif
        if ((*pui32BuffSize % 3) == 0)
        {
            store_iom_recv_data_from_codec(g_pIomRecvPacket + i * (IOM_IOS_BUFSIZE_MAX / 4 * 3), ui32FrameSize / 4 * 3);
        }
        else
        {
            store_iom_recv_data_from_codec(g_pIomRecvPacket + i * (IOM_IOS_BUFSIZE_MAX / 4 * 3), ui32FrameSize / 4 * 3 - (3 - (*pui32BuffSize % 3)));
        }
    }

    return ui32Status;
}

//*****************************************************************************
//
// IOM writes packet to CODEC
//
//*****************************************************************************
uint32_t
am_widget_audio_send_packet_to_codec(uint8_t * pui8PacketAddr, uint32_t ui32BuffSize)
{
    am_hal_iom_transfer_t       Transaction;
    uint32_t ui32Status = 0;
    uint32_t ui32Loop, i;
    uint32_t ui32BuffSizeIomSend, ui32FrameSize, ui32ValidDataSize;
    uint32_t ui32CodecBuffAddr;
    uint32_t data[CONTROL_WORDS_LENGTH] = {0};
    //
    // Initialize Data Buffers
    //
    ui32FrameSize = IOM_IOS_AVAILABLE_BUFSIZE_MAX;
    ui32Loop = ((ui32BuffSize + 2) / 3 * 4  + ui32FrameSize - 1) / ui32FrameSize;
    if (g_sIosWidget.bSpi)
    {
        Transaction.ui32InstrLen    = 4;
        Transaction.ui64Instr = CONTROL_WORD_ADDR | RW_BIT_MASK;
        Transaction.eDirection      = AM_HAL_IOM_RX;
        Transaction.uPeerInfo.ui32SpiChipSelect = 0;
        Transaction.ui32NumBytes    = 4 + CODEC_PADDING_BYTES;
        Transaction.pui32RxBuffer   = data;
        Transaction.bContinue       = false;
        Transaction.ui8RepeatCount  = 0;
        Transaction.ui32PauseCondition = 0;
        Transaction.ui32StatusSetClr = 0;
        am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
        remove_padding_bytes((uint8_t *)data);
        //
        // change endian
        //
        data[0] = change_endian(data[0]);
        data[0] |= MASTER_REQUEST_WRITE_MSK;
        data[0] &= ~TRANSFER_LENGTH_MSK;
        data[0] += ui32BuffSize << 8;
        //
        // change endian back
        //
        data[0] = change_endian(data[0]);
        add_padding_bytes((uint8_t *)data);
        Transaction.ui32InstrLen    = 4;
        Transaction.ui64Instr = CONTROL_WORD_ADDR & ~RW_BIT_MASK;
        Transaction.eDirection      = AM_HAL_IOM_TX;
        Transaction.uPeerInfo.ui32SpiChipSelect = 0;
        Transaction.ui32NumBytes    = 4 + CODEC_PADDING_BYTES;
        Transaction.pui32TxBuffer   = data;
        Transaction.bContinue       = false;
        Transaction.ui8RepeatCount  = 0;
        Transaction.ui32PauseCondition = 0;
        Transaction.ui32StatusSetClr = 0;
        am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
    }
    else
    {
        Transaction.ui32InstrLen    = 4;
        Transaction.ui64Instr = CONTROL_WORD_ADDR;
        Transaction.eDirection      = AM_HAL_IOM_RX;
        Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
        Transaction.ui32NumBytes    = 4;
        Transaction.pui32RxBuffer   = data;
        Transaction.bContinue       = false;
        Transaction.ui8RepeatCount  = 0;
        Transaction.ui32PauseCondition = 0;
        Transaction.ui32StatusSetClr = 0;
        am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
        //
        // change endian
        //
        data[0] = change_endian(data[0]);
        data[0] |= MASTER_REQUEST_WRITE_MSK;
        data[0] &= ~TRANSFER_LENGTH_MSK;
        data[0] += ui32BuffSize << 8;
        //
        // change endian back
        //
        data[0] = change_endian(data[0]);
        Transaction.ui32InstrLen    = 4;
        Transaction.ui64Instr = CONTROL_WORD_ADDR;
        Transaction.eDirection      = AM_HAL_IOM_TX;
        Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
        Transaction.ui32NumBytes    = 4;
        Transaction.pui32TxBuffer   = data;
        Transaction.bContinue       = false;
        Transaction.ui8RepeatCount  = 0;
        Transaction.ui32PauseCondition = 0;
        Transaction.ui32StatusSetClr = 0;
        am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
    }
    am_hal_gpio_state_write(MST_SEND_INT_TO_SLV_PIN, AM_HAL_GPIO_OUTPUT_SET); // send a rising edge to codec interrupt input pin
    while (1)
    {
        if (g_sIosWidget.bSpi)
        {
            Transaction.ui32InstrLen    = 4;
            Transaction.ui64Instr = CONTROL_WORD_ADDR | RW_BIT_MASK;
            Transaction.eDirection      = AM_HAL_IOM_RX;
            Transaction.uPeerInfo.ui32SpiChipSelect = 0;
            Transaction.ui32NumBytes    = CONTROL_WORDS_LENGTH * 4 + CODEC_PADDING_BYTES;
            Transaction.pui32RxBuffer   = data;
            Transaction.bContinue       = false;
            Transaction.ui8RepeatCount  = 0;
            Transaction.ui32PauseCondition = 0;
            Transaction.ui32StatusSetClr = 0;
            am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
            remove_padding_bytes((uint8_t *)data);
            //
            // change endian
            //
            data[0] = change_endian(data[0]);
            data[1] = change_endian(data[1]);
#ifdef DEBUG_WITHOUT_CODEC
            data[0] = 0x0;
            data[1] = 0x00345678;
#endif
            if ((data[0] & MASTER_REQUEST_WRITE_MSK) == 0)
            {
                ui32CodecBuffAddr = data[1];
                am_hal_gpio_state_write(MST_SEND_INT_TO_SLV_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
                break;
            }
        }
        else
        {
            Transaction.ui32InstrLen    = 4;
            Transaction.ui64Instr = CONTROL_WORD_ADDR;
            Transaction.eDirection      = AM_HAL_IOM_RX;
            Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
            Transaction.ui32NumBytes    = CONTROL_WORDS_LENGTH * 4;
            Transaction.pui32RxBuffer   = data;
            Transaction.bContinue       = false;
            Transaction.ui8RepeatCount  = 0;
            Transaction.ui32PauseCondition = 0;
            Transaction.ui32StatusSetClr = 0;
            am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
            //
            // change endian
            //
            data[0] = change_endian(data[0]);
            data[1] = change_endian(data[1]);
            if ((data[0] & MASTER_REQUEST_WRITE_MSK) == 0)
            {
                ui32CodecBuffAddr = data[1];
                am_hal_gpio_state_write(MST_SEND_INT_TO_SLV_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
                break;
            }
        }
    }
    for (i = 0; i < ui32Loop; i++)
    {
        if (i < ui32Loop -1)
        {
            ui32BuffSizeIomSend = ui32FrameSize;
            ui32ValidDataSize = ui32BuffSizeIomSend / 4 * 3;
        }
        else
        {
            ui32BuffSizeIomSend = (ui32BuffSize + 2) / 3 * 4 - (ui32Loop -1) * ui32FrameSize;
            ui32ValidDataSize = ui32BuffSizeIomSend / 4 * 3 - (3 - (ui32BuffSize % 3));
        }
        load_iom_send_data_to_codec(pui8PacketAddr + i * ui32FrameSize, ui32ValidDataSize);
        bIomSendToCodecComplete = false;
        //
        // Trigger IOM to Write at specified address
        // handshake between IOM and IOS using IOCTL interrupts
        // IOM indicates IOS on completion of each batch
        //
        iom_send(ui32CodecBuffAddr + i * ui32FrameSize / 4 * 2, ui32BuffSizeIomSend);
        while (1)
        {
            if (bIomSendToCodecComplete)
            {
                if (i == ui32Loop - 1)
                {
                    //
                    // Set master write complete flag bit.
                    //
                    if (g_sIosWidget.bSpi)
                    {
                        Transaction.ui32InstrLen    = 4;
                        Transaction.ui64Instr = CONTROL_WORD_ADDR | RW_BIT_MASK;
                        Transaction.eDirection      = AM_HAL_IOM_RX;
                        Transaction.uPeerInfo.ui32SpiChipSelect = 0;
                        Transaction.ui32NumBytes    = 4 + CODEC_PADDING_BYTES;
                        Transaction.pui32RxBuffer   = data;
                        Transaction.bContinue       = false;
                        Transaction.ui8RepeatCount  = 0;
                        Transaction.ui32PauseCondition = 0;
                        Transaction.ui32StatusSetClr = 0;
                        am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);

                        remove_padding_bytes((uint8_t *)data);
                        data[0] |= change_endian(MASTER_WRITE_COMPLETE_MSK);
                        add_padding_bytes((uint8_t *)data);
                        Transaction.ui32InstrLen    = 4;
                        Transaction.ui64Instr = CONTROL_WORD_ADDR & ~RW_BIT_MASK;
                        Transaction.eDirection      = AM_HAL_IOM_TX;
                        Transaction.uPeerInfo.ui32SpiChipSelect = 0;
                        Transaction.ui32NumBytes    = 4 + CODEC_PADDING_BYTES;
                        Transaction.pui32TxBuffer   = data;
                        Transaction.bContinue       = false;
                        Transaction.ui8RepeatCount  = 0;
                        Transaction.ui32PauseCondition = 0;
                        Transaction.ui32StatusSetClr = 0;
                        am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
                    }
                    else
                    {
                        Transaction.ui32InstrLen    = 4;
                        Transaction.ui64Instr = CONTROL_WORD_ADDR;
                        Transaction.eDirection      = AM_HAL_IOM_RX;
                        Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
                        Transaction.ui32NumBytes    = 4;
                        Transaction.pui32RxBuffer   = data;
                        Transaction.bContinue       = false;
                        Transaction.ui8RepeatCount  = 0;
                        Transaction.ui32PauseCondition = 0;
                        Transaction.ui32StatusSetClr = 0;
                        am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);

                        data[0] |= change_endian(MASTER_WRITE_COMPLETE_MSK);
                        Transaction.ui32InstrLen    = 4;
                        Transaction.ui64Instr = CONTROL_WORD_ADDR;
                        Transaction.eDirection      = AM_HAL_IOM_TX;
                        Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
                        Transaction.ui32NumBytes    = 4;
                        Transaction.pui32TxBuffer   = data;
                        Transaction.bContinue       = false;
                        Transaction.ui8RepeatCount  = 0;
                        Transaction.ui32PauseCondition = 0;
                        Transaction.ui32StatusSetClr = 0;
                        am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
                    }
                }
                break;
            }
            am_hal_delay_us(1000);
        }
        bIomSendToCodecComplete = false;
    }
    
    return ui32Status;
}
#endif

uint32_t
am_widget_audio_ios2iom_xfer(void *pWidget, void *pTestCfg)
{
    am_widget_audio_spi_t *pIosSpiWidget = (am_widget_audio_spi_t *)pWidget;
    am_hal_iom_transfer_t       Transaction;
    uint32_t ui32Status = 0;
    uint32_t ui32Loop, i;
    uint32_t ui32BuffSizeIosSend, ui32BuffSizeIomRecv,ui32FrameSize;

    pIosSpiWidget->testCfg = *((am_widget_audio_spi_test_t *)pTestCfg);
    //
    // Initialize Data Buffers
    //
    ui32FrameSize = pIosSpiWidget->testCfg.ui32LramSize - 2; // reserve 2 bytes for data length
    ui32Loop = (pIosSpiWidget->testCfg.ui32PacketSize + ui32FrameSize - 1) / ui32FrameSize;
    
    for (i = 0; i < ui32Loop; i++)
    {
        bIosSendComplete = false;
        if (i < ui32Loop -1)
        {
            ui32BuffSizeIosSend = ui32FrameSize;
        }
        else
        {
            ui32BuffSizeIosSend = pIosSpiWidget->testCfg.ui32PacketSize - (ui32Loop -1) * ui32FrameSize;
        }
        load_ios_send_data((uint8_t *)pIosSpiWidget->testCfg.ui32PacketAddress + i * ui32FrameSize, ui32BuffSizeIosSend);
        ios_send(pIosSpiWidget->testCfg.ui8LramAddress, ui32BuffSizeIosSend + 2);
        //
        // Handshake will use the interrupt IOCTL or a GPIO - to signal host that data is ready
        //
        while (1)
        {
            //
            // Disable interrupts before checking the flags
            //
            uint32_t ui32IntStatus = am_hal_interrupt_master_disable();
            if (bIosSendComplete)
            {
                //
                // re-enable interrupts
                //
                am_hal_interrupt_master_set(ui32IntStatus);
                break;
            }

            if (bHandshake == true)
            {
                uint32_t data = 0;
                //
                // re-enable interrupts
                //
                am_hal_interrupt_master_set(ui32IntStatus);
                bHandshake = false;
                //
                // Read & Clear the IOINT status
                //
                if (g_sIosWidget.bSpi)
                {
                    Transaction.ui32InstrLen    = 1;
                    Transaction.ui64Instr = 0x79;
                    Transaction.eDirection      = AM_HAL_IOM_RX;
                    Transaction.uPeerInfo.ui32SpiChipSelect = 0;
                    Transaction.ui32NumBytes    = 1;
                    Transaction.pui32RxBuffer   = &data;
                    Transaction.bContinue       = false;
                    Transaction.ui8RepeatCount  = 0;
                    Transaction.ui32PauseCondition = 0;
                    Transaction.ui32StatusSetClr = 0;
                    am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
                    //
                    // We need to clear the bit by writing to IOS
                    //
                    if (data & HANDSHAKE_IOS_TO_IOM)
                    {
                        data = HANDSHAKE_IOS_TO_IOM;
                        Transaction.ui32InstrLen    = 1;
                        Transaction.ui64Instr = 0xFA;
                        Transaction.eDirection      = AM_HAL_IOM_TX;
                        Transaction.uPeerInfo.ui32SpiChipSelect = 0;
                        Transaction.ui32NumBytes    = 1;
                        Transaction.pui32TxBuffer   = &data;
                        Transaction.bContinue       = false;
                        Transaction.ui8RepeatCount  = 0;
                        Transaction.ui32PauseCondition = 0;
                        Transaction.ui32StatusSetClr = 0;
                        am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
                        //
                        // Set bIosSendComplete
                        //
                        bIosSendComplete = true;
                        break;
                    }
                }
                else
                {
                    Transaction.ui32InstrLen    = 1;
                    Transaction.ui64Instr = 0x79;
                    Transaction.eDirection      = AM_HAL_IOM_RX;
                    Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
                    Transaction.ui32NumBytes    = 1;
                    Transaction.pui32RxBuffer   = &data;
                    Transaction.bContinue       = false;
                    Transaction.ui8RepeatCount  = 0;
                    Transaction.ui32PauseCondition = 0;
                    Transaction.ui32StatusSetClr = 0;
                    am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
                    //
                    // We need to clear the bit by writing to IOS
                    //
                    if (data & HANDSHAKE_IOS_TO_IOM)
                    {
                        data = HANDSHAKE_IOS_TO_IOM;
                        Transaction.ui32InstrLen    = 1;
                        Transaction.ui64Instr = 0x7A;
                        Transaction.eDirection      = AM_HAL_IOM_TX;
                        Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
                        Transaction.ui32NumBytes    = 1;
                        Transaction.pui32TxBuffer   = &data;
                        Transaction.bContinue       = false;
                        Transaction.ui8RepeatCount  = 0;
                        Transaction.ui32PauseCondition = 0;
                        Transaction.ui32StatusSetClr = 0;
                        am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
                        //
                        // Set bIosSendComplete
                        //
                        bIosSendComplete = true;
                        break;
                    }
                }
            }
            else
            {
                am_hal_interrupt_master_set(ui32IntStatus);
            }
        }
        bIosSendComplete = false;

        g_ui32IosIsr = 0;
        bIomRecvComplete = false;

        iom_recv(pIosSpiWidget->testCfg.ui8LramAddress, pIosSpiWidget->testCfg.ui32LramSize);

        while (1)
        {
            //
            // Disable interrupts before checking the flags
            //
            uint32_t ui32IntStatus = am_hal_interrupt_master_disable();
            if (bIomRecvComplete)
            {
                //
                // re-enable interrupts
                //
                am_hal_interrupt_master_set(ui32IntStatus);
                break;
            }
            //
            // re-enable interrupts
            //
            am_hal_interrupt_master_set(ui32IntStatus);
        }
        ui32BuffSizeIomRecv = g_pIomRecvBuf[0] + (g_pIomRecvBuf[1] << 8);
        store_iom_recv_data(g_pIomRecvPacket + i * ui32FrameSize, ui32BuffSizeIomRecv);
    }
    return ui32Status;
}

uint32_t
am_widget_audio_iom2ios_xfer(void *pWidget, void *pTestCfg)
{
    am_widget_audio_spi_t *pIosSpiWidget = (am_widget_audio_spi_t *)pWidget;
    uint32_t ui32Status = 0;
    uint32_t ui32Loop, i;
    uint32_t ui32BuffSizeIomSend, ui32BuffSizeIosRecv,ui32FrameSize;

    pIosSpiWidget->testCfg = *((am_widget_audio_spi_test_t *)pTestCfg);
    //
    // Initialize Data Buffers
    //
    ui32FrameSize = pIosSpiWidget->testCfg.ui32LramSize - 2; // reserve 2 bytes for data length
    ui32Loop = (pIosSpiWidget->testCfg.ui32PacketSize + ui32FrameSize - 1) / ui32FrameSize;

    for (i = 0; i < ui32Loop; i++)
    {
        if (i < ui32Loop -1)
        {
            ui32BuffSizeIomSend = ui32FrameSize;
        }
        else
        {
            ui32BuffSizeIomSend = pIosSpiWidget->testCfg.ui32PacketSize - (ui32Loop -1) * ui32FrameSize;
        }
        load_iom_send_data((uint8_t *)pIosSpiWidget->testCfg.ui32PacketAddress + i * ui32FrameSize, ui32BuffSizeIomSend);
        g_ui32IosIsr = 0;
        bIomSendComplete = false;
        //
        // Trigger IOM to Write at specified address
        // handshake between IOM and IOS using IOCTL interrupts
        // IOM indicates IOS on completion of each batch
        //
        iom_send(pIosSpiWidget->testCfg.ui8LramAddress, ui32BuffSizeIomSend + 2);
        while (1)
        {
            if (bIomSendComplete)
            {
                break;
            }

            am_hal_delay_us(1000);
        }
        bIomSendComplete = false;
        //
        // IOS will accumulate the data if needed on READ side
        // When complete, the data will be turned around back to IOM
        // Read the received data
        //
        ios_read(pIosSpiWidget->testCfg.ui8LramAddress, pIosSpiWidget->testCfg.ui32LramSize);
 
        ui32BuffSizeIosRecv = g_pIosRecvBuf[0] + (g_pIosRecvBuf[1] << 8);
        store_ios_recv_data(g_pIosRecvPacket + i * ui32FrameSize, ui32BuffSizeIosRecv);
        
    }
    
    return ui32Status;
}

uint32_t
am_widget_audio_uart_loopback(void *pWidget, am_widget_uart_loopback_config_t *psConfig)
{
    am_widget_uart_send_config_t sTxConfig;
    am_widget_uart_recv_config_t sRxConfig;
    uint32_t ui32Status = 0;
    int32_t TxQueueLength, RxQueueLength;

    if (psConfig->txQueueEnable)
    {
        TxQueueLength = sizeof(psConfig->pui8QueueTx);
    }
    else
    {
        TxQueueLength = 0;
    }

    if (psConfig->rxQueueEnable)
    {
        RxQueueLength = sizeof(psConfig->pui8QueueRx);
    }
    else
    {
        RxQueueLength = 0;
    }

    am_hal_uart_buffer_configure(UART, psConfig->pui8QueueTx, TxQueueLength, psConfig->pui8QueueRx, RxQueueLength);

    sTxConfig.pui8QueueTx = psConfig->pui8QueueTx;
    sTxConfig.txblocking =  psConfig->txblocking;
    sTxConfig.txbufferSize =  psConfig->txbufferSize;
    sTxConfig.txQueueEnable =  psConfig->txQueueEnable;
    sTxConfig.length =  psConfig->length;
    sTxConfig.pui8SpaceTx =  psConfig->pui8SpaceTx;
    uart_send(&sTxConfig);

    sRxConfig.pui8QueueRx = psConfig->pui8QueueRx;
    sRxConfig.rxblocking =  psConfig->rxblocking;
    sRxConfig.rxbufferSize =  psConfig->rxbufferSize;
    sRxConfig.rxQueueEnable =  psConfig->rxQueueEnable;
    sRxConfig.pui8SpaceRx =  psConfig->pui8SpaceRx;
    uart_recv(&sRxConfig);
    return ui32Status;
}

void am_widget_audio_cleanup(void)
{
    //
    // Clean up the IOM
    //
    iom_clean_up();
    //
    // Clean up the IOS
    //
    ios_clean_up();
    //
    // UART clean up
    //
    uart_clean_up();
#ifdef CODEC_MODE
    //
    // CODEC clean up
    //

#endif
    am_hal_interrupt_master_disable();
}

