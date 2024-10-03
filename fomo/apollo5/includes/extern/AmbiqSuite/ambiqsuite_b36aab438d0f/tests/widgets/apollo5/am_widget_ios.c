//*****************************************************************************
//
//! @file am_widget_ios.c
//!
//! @brief Widget to test IOS using IOM as stimulus
//!
//! This program initiates a SPI/I2C transaction at specified address/size,
//! For access to LRAM locations (address != 0x7F), IOM sends a pattern to IOS,
//! IOS receives it, manipulates the data and send it back to IOM for
//! verification.
//! For access to FIFO (address == 0x7F), a pattern is send from IOS to IOM,
//! which is then verified for accuracy.
//!
//! SWO is configured in 1M baud, 8-n-1 mode.
//!
//! The test utilizes an IOM<->IOS transfer within the same device (basically
//! a loopback configuration). Running the test requires jumpering pins in
//! order to connect the IOM to the IOS.
//!
//! @verbatim
//! The pin jumpers should be connected as follows, as defined in bsp_pins.src:
//! SPI:
//! IOS_SCK  to IOM0_SCK
//! IOS_MOSI to IOM0_MOSI
//! IOS_MISO to IOM0_MISO
//! IOS_CE   to IOM0_CS
//! IOS_INT  to GPIO46 (as defined below)
//!
//! I2C:
//! IOS_SCL to IOM0_SCL
//! IOS_SDA to IOM0_SDA
//! IOS_INT to GPIO46 (as defined below)
//! @endverbatim
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

#include "am_widget_ios.h"

// Hardware Issues - fixed in B2
// SHELBY_1643
// SHELBY_1652
// SHELBY_1653
// SHELBY_1657
// SHELBY_1654

#define NO_SHELBY_1643
#define NO_SHELBY_1652
#define NO_SHELBY_1653
#define NO_SHELBY_1657
#define NO_SHELBY_1654

// Not fixed in B2
//  SHELBY_1655
//  SHELBY_1658

#define NO_SHELBY_1655
#define NO_SHELBY_1658

// IOINT bits for handshake
#define HANDSHAKE_IOM_TO_IOS     0x1
#define HANDSHAKE_IOS_TO_IOM     0x2

//*****************************************************************************
//
// GPIO Configuration
//
//*****************************************************************************
const am_hal_gpio_pincfg_t g_AM_BSP_IOM_GPIO_ENABLE =
{
    .GP.cfg_b.uFuncSel            = AM_HAL_PIN_46_GPIO,
    .GP.cfg_b.eDriveStrength      = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
    .GP.cfg_b.eIntDir             = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
    .GP.cfg_b.eGPInput            = AM_HAL_GPIO_PIN_INPUT_ENABLE,
};

const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_SLINT =
{
    .GP.cfg_b.uFuncSel            = SLINT_FUN_SEL,
    .GP.cfg_b.eDriveStrength      = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
    .GP.cfg_b.eCEpol              = AM_HAL_GPIO_PIN_CEPOL_ACTIVEHIGH,
    .GP.cfg_b.eGPOutCfg           = AM_HAL_GPIO_PIN_OUTCFG_OPENDRAIN,
};

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
#define AM_TEST_TIMER       0
#define AM_TEST_TIMEOUT     1 // 1 Seconds
#define TEST_XOR_BYTE       0x0 // 0 Will not change, 0xFF will invert
//#define TEST_IOM_QUEUE      // if not defined uses IOM in polling mode - would cause underflows for higher speeds
#ifdef NO_SHELBY_1655
#define TEST_ACC_INT      // if defined uses ACC ISR. SHELBY-1655 interferes with this
#endif
#ifdef NO_SHELBY_1653
#define TEST_IOINTCTL      // if defined, uses IOINTCTL - SHELBY-1653 interferes with this
#endif
#define TEST_IOS_XCMP_INT  // XCMP flags defined only for Apollo2 onwards
//#define TEST_ACC_ISR_RANDOMIZE

//*****************************************************************************
//
// Message buffers.
//
// data from the IOS interface, which is only 8 bits wide.
//
//*****************************************************************************
#define AM_IOS_TX_BUFSIZE_MAX  1023
AM_SHARED_RW uint8_t g_pui8TxFifoBuffer[AM_IOS_TX_BUFSIZE_MAX] __attribute__((aligned(32)));
#ifdef TEST_IOS_DMA
AM_SHARED_RW uint8_t g_pIosSendBuf[DMA_PACK_SIZE]  __attribute__((aligned(32)));
AM_SHARED_RW uint8_t g_pRefData[DMA_PACK_SIZE] __attribute__((aligned(32)));
AM_SHARED_RW uint8_t g_pIomRecvBuf[DMA_PACK_SIZE] __attribute__((aligned(32)));
#else
AM_SHARED_RW uint8_t g_pIosSendBuf[AM_IOS_TX_BUFSIZE_MAX] __attribute__((aligned(32)));
AM_SHARED_RW uint8_t g_pRefData[AM_IOS_TX_BUFSIZE_MAX] __attribute__((aligned(32)));
AM_SHARED_RW uint8_t g_pIomRecvBuf[AM_IOS_TX_BUFSIZE_MAX] __attribute__((aligned(32)));
#endif
bool    bContinuous = false;

#ifdef TEST_IOS_DMA

#define IOS_DMA_READ_TEST
#define IOS_DMA_WRITE_TEST
#define IOS_DMA_HANDSHAKE_TEST

bool    bDMACMP = false;

#define AM_DMATEST_CMD_START_DATA   0xFD
#define AM_DMATEST_CMD_ACK_DATA     0xFE
#define AM_DMATEST_CMD_STOP_DATA    0xFF

typedef enum
{
    DMA_DATA_TAG,
    DMA_DATA_LENGTH = 2,
}eDMA_Data;

typedef struct
{
    uint16_t ui16Tag;
    uint16_t ui16Size;
}stHandshake;

static uint32_t ios_dma_read(uint32_t size);
static uint32_t ios_dma_send(uint32_t size);
#endif

struct {
    uint32_t size;
    uint32_t totalWritten;
} ios_feed_state;

#ifdef TEST_IOM_QUEUE
am_hal_iom_queue_entry_t g_psQueueMemory[32];
#endif

static void *g_pIOMHandle;
static void *g_pIOSHandle;

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
    am_widget_iosspi_test_t testCfg;
    uint32_t                iom;
    uint32_t                i2cAddr;
    bool                    bSpi;
} am_widget_iosspi_t;

am_widget_iosspi_t g_sIosWidget;

// Status flags.
//! Input buffer set to this value when empty.
#define INBUFFER_EMPTY  (0xEE)
//! Output buffer set to this value when empty.
#define OUTBUFFER_EMPTY (0xED)
//! Value for one byte write/read test (188).
#define WRITE_READ_BYTE (0xBC)
#define ROBUFFER_INIT   (0x55)
#define AHBBUF_INIT     (0xAA)

static void
iom_clean_up(void)
{
    uint32_t                    ioIntEnable = 0;
    am_hal_iom_transfer_t       Transaction;
    am_hal_gpio_mask_t IntStatus;
    uint32_t IntNum = HANDSHAKE_IOM_PIN;

    // Disable IOCTL interrupts
    if (g_sIosWidget.bSpi)
    {
      Transaction.ui32InstrLen    = 1;
      Transaction.ui64Instr = 0xF8;
      Transaction.eDirection      = AM_HAL_IOM_TX;
      Transaction.uPeerInfo.ui32SpiChipSelect = AM_HAL_GPIO_NCE_IOM0CE0 + 4 * g_sIosWidget.iom;
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
        // Set up IOM SPI pins. Attributes are set in am_bsp_gpio.h.
        //
        am_bsp_iom_pins_disable(g_sIosWidget.iom, AM_HAL_IOM_SPI_MODE);

        //
        // Enable the chip-select and data-ready pin.
        //! @note You can enable pins in the HAL or BSP.
        //
        am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_CS, g_AM_BSP_GPIO_IOM0_CS);
    }
    else
    {
        //
        // Set up IOM I2C pins. Attributes are set in am_bsp_gpio.h.
        //
        am_bsp_iom_pins_disable(g_sIosWidget.iom, AM_HAL_IOM_I2C_MODE);
    }

    // Disable interrupts for NB send to work
    am_hal_iom_interrupt_disable(g_pIOMHandle,
        AM_HAL_IOM_INT_CMDCMP);
    NVIC_DisableIRQ(iomaster_interrupt[g_sIosWidget.iom]);

    // Set up the host IO interrupt
    am_hal_gpio_state_write(HANDSHAKE_IOM_PIN,AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_pinconfig(HANDSHAKE_IOM_PIN, g_AM_BSP_GPIO_DISABLE);
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_INT_CHANNEL_0, &IntStatus);
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                                  AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                                  (void *)&IntNum);
    NVIC_DisableIRQ(GPIO0_203F_IRQn);

    //
    // Disable power to IOM.
    //
    am_hal_pwrctrl_periph_disable((am_hal_pwrctrl_periph_e)(AM_HAL_PWRCTRL_PERIPH_IOM0 + g_sIosWidget.iom));
}

//*****************************************************************************
//
// Internal Helper functions
//
//*****************************************************************************

static void
iom_set_up(am_widget_iosspi_stimulus_iom_t *pCfg)
{
    am_hal_iom_transfer_t       Transaction;
    uint32_t ioIntEnable = 0x3;
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

        //
        // Enable the chip-select and data-ready pin.
        //! @note You can enable pins in the HAL or BSP.
        //
        am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_CS, g_AM_BSP_GPIO_IOM0_CS);
    }
    else
    {
        am_bsp_iom_pins_enable(pCfg->iomModule,AM_HAL_IOM_I2C_MODE);
    }
#ifdef TEST_IOM_QUEUE
    am_hal_iom_queue_init(pCfg->iomModule,
        g_psQueueMemory, sizeof(g_psQueueMemory));
#endif
    // Enable interrupts for NB send to work
    am_hal_iom_interrupt_enable(g_pIOMHandle, 0xFF);
    NVIC_EnableIRQ(iomaster_interrupt[pCfg->iomModule]);

    //
    // Turn on the IOM for this operation.
    //
    am_hal_iom_enable(g_pIOMHandle);

    // Set up the host IO interrupt
    am_hal_gpio_pinconfig(HANDSHAKE_IOM_PIN, g_AM_BSP_IOM_GPIO_ENABLE);
    am_hal_gpio_state_write(HANDSHAKE_IOM_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                                  AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                                  (void *)&IntNum);
    NVIC_EnableIRQ(GPIO0_203F_IRQn);

    // Set up IOCTL interrupts
    // IOS ==> IOM & IOM ==> IOS
    if (g_sIosWidget.bSpi)
    {
      Transaction.ui32InstrLen    = 1;
      Transaction.ui64Instr = 0xF8;
      Transaction.eDirection      = AM_HAL_IOM_TX;
      Transaction.uPeerInfo.ui32SpiChipSelect = AM_HAL_GPIO_NCE_IOM0CE0 + 4 * pCfg->iomModule;
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
}

static void
ios_clean_up(void)
{
    uint32_t ui32Arg = AM_HAL_IOS_ACCESS_INT_ALL;

    if (g_sIosWidget.bSpi)
    {
        // Configure SPI interface
        am_bsp_ios_pins_disable(TEST_IOS_MODULE, AM_HAL_IOS_USE_SPI);
    }
    else
    {
        // Configure I2C interface
        am_bsp_ios_pins_disable(TEST_IOS_MODULE, AM_HAL_IOS_USE_I2C);
    }

    //
    // Clear out any IOS register-access interrupts that may be active, and
    // enable interrupts for the registers we're interested in.
    //
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTDIS, &ui32Arg);
    am_hal_ios_interrupt_disable(g_pIOSHandle, AM_HAL_IOS_INT_ALL);

    // Preparation of FIFO
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_FIFO_BUF_CLR, NULL);

    am_hal_ios_disable(g_pIOSHandle);

    //
    // Set the bit in the NVIC to accept access interrupts from the IO Slave.
    //
    NVIC_DisableIRQ(IOS_ACC_IRQ);
    NVIC_DisableIRQ(IOS_IRQ);

    // Set up the IOSINT interrupt pin
    am_hal_gpio_pinconfig(HANDSHAKE_IOS_PIN, g_AM_BSP_GPIO_DISABLE);

#if defined(AM_PART_APOLLO5B)
    am_hal_pwrctrl_periph_disable((am_hal_pwrctrl_periph_e)(AM_HAL_PWRCTRL_PERIPH_IOS0 + TEST_IOS_MODULE));
#else
    am_hal_pwrctrl_periph_disable((am_hal_pwrctrl_periph_e)(AM_HAL_PWRCTRL_PERIPH_IOS));
#endif
}

#define AM_HAL_IOS_INT_ERR  (AM_HAL_IOS_INT_FOVFL | AM_HAL_IOS_INT_FUNDFL | AM_HAL_IOS_INT_FRDERR | AM_HAL_IOS_INT_GENAD)
#define AM_HAL_IOS_XCMP_INT (AM_HAL_IOS_INT_XCMPWR | AM_HAL_IOS_INT_XCMPWF | AM_HAL_IOS_INT_XCMPRR | AM_HAL_IOS_INT_XCMPRF)
#define AM_HAL_IOS_DMA_INT  (AM_HAL_IOS_INT_DCMP | AM_HAL_IOS_INT_DERR)
//*****************************************************************************
//
// Configure the SPI slave.
//
//*****************************************************************************
static void
ios_set_up(am_widget_iosspi_stimulus_ios_t *pCfg)
{
    uint32_t ui32Arg = AM_HAL_IOS_ACCESS_INT_ALL;

    if (g_sIosWidget.bSpi)
    {
        // Configure SPI interface
        am_bsp_ios_pins_enable(pCfg->iosModule, AM_HAL_IOS_USE_SPI);
    }
    else
    {
        // Configure I2C interface
        am_bsp_ios_pins_enable(pCfg->iosModule, AM_HAL_IOS_USE_I2C);
    }

    //
    // Configure the IOS interface and LRAM structure.
    //
    am_hal_ios_initialize(pCfg->iosModule, &g_pIOSHandle);
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
#ifdef TEST_IOS_DMA
    am_hal_ios_interrupt_enable(g_pIOSHandle, AM_HAL_IOS_DMA_INT);
#endif

    //
    // Set the bit in the NVIC to accept access interrupts from the IO Slave.
    //
    NVIC_EnableIRQ(IOS_ACC_IRQ);
    NVIC_EnableIRQ(IOS_IRQ);

    // Set up the IOSINT interrupt pin
#if SLINT_GPIO
    am_hal_gpio_pinconfig(HANDSHAKE_IOS_PIN, am_hal_gpio_pincfg_output);
#else
    am_hal_gpio_pinconfig(HANDSHAKE_IOS_PIN, g_AM_BSP_GPIO_SLINT);
#endif
}

volatile uint32_t g_ui32AccIsr = 0;
volatile uint32_t g_ui32IosIsr = 0;
volatile bool bIomSendComplete = false;
volatile bool bIosSendComplete = false;
volatile bool bIomRecvComplete = false;
uint32_t accIsrStatus[32] = {0};
uint32_t accIsrIdx = 0;

uint32_t missingAccIsr[32] = {0};

uint8_t g_random[] =
{
    0x15, 0x2f, 0x8f, 0xdf, 0xc3, 0xa0, 0x6c, 0x16, 0x19, 0xa5, 0x18, 0xc1, 0x18, 0x65, 0xa8, 0x44,
    0xd2, 0x53, 0x3b, 0x14, 0xb0, 0x59, 0xae, 0x00, 0x67, 0xf0, 0x6e, 0x30, 0x35, 0x21, 0x50, 0x02,
    0xf1, 0x5b, 0x1c, 0xcc, 0x4d, 0x1e, 0xd1, 0x5c, 0x30, 0xea, 0xdf, 0xf9, 0xf6, 0x39, 0x06, 0x78,
    0xd1, 0xf3, 0xef, 0x79, 0xc8, 0x34, 0x95, 0xe5, 0x38, 0x2a, 0x75, 0xb4, 0xc9, 0x76, 0x58, 0xc3,
    0xe0, 0x7f, 0x7e, 0x82, 0xa0, 0xd0, 0xac, 0x02, 0x43, 0x2d, 0xdc, 0x3d, 0x45, 0xb2, 0x97, 0x82,
    0xb5, 0x7f, 0x9c, 0x5a, 0xcf, 0x46, 0x81, 0xd4, 0x4d, 0xf9, 0xa7, 0xcc, 0xcb, 0x6e, 0xb3, 0x1d,
    0x09, 0x04, 0x6d, 0x36, 0x57, 0xc0, 0x52, 0xa6, 0x8a, 0x0c, 0xd5, 0x66, 0xeb, 0x56, 0xaa, 0x68,
    0xec, 0x25, 0x78, 0x01, 0x88, 0x63, 0x7b, 0xbf, 0xe2, 0x01, 0xbf, 0xed, 0xc5, 0x9a, 0x3c, 0xbb,
    0x45, 0x2f, 0xac, 0x8e, 0x4e, 0x57, 0x10, 0x0c, 0x29, 0x4d, 0xfe, 0xd2, 0xd6, 0x2b, 0x48, 0xad,
    0x06, 0xef, 0x51, 0xd6, 0x95, 0xa4, 0xb6, 0xe2, 0x5a, 0x30, 0x63, 0x56, 0xcd, 0x28, 0x52, 0xf6,
    0x60, 0x79, 0xa3, 0x71, 0xde, 0x02, 0xf5, 0x63, 0x5c, 0xe6, 0xe3, 0x59, 0x71, 0x7a, 0xfa, 0xd9,
    0x9d, 0x45, 0x25, 0xbc, 0xe7, 0xbe, 0x84, 0xf7, 0x48, 0xf5, 0x60, 0x37, 0x72, 0x59, 0xc6, 0xbb,
    0x22, 0xa4, 0xdd, 0xb0, 0xcb, 0x73, 0x84, 0xed, 0xa2, 0x74, 0xb6, 0x94, 0x01, 0x20, 0xcb, 0xa4,
    0xf5, 0x9d, 0xab, 0x20, 0xb8, 0xff, 0x07, 0x71, 0x51, 0xee, 0x9b, 0x08, 0x4a, 0xd8, 0x81, 0x31,
    0xd7, 0xfb, 0xa9, 0xef, 0x93, 0x13, 0x7c, 0xc6, 0xb8, 0x13, 0x53, 0x83, 0x49, 0xd9, 0xc3, 0x84,
    0xe6, 0x54, 0x43, 0x54, 0x24, 0xea, 0x32, 0xbb, 0x62, 0x4d, 0x08, 0x0c, 0x70, 0x43, 0xc5, 0xcb,
};
uint8_t g_randomIdx = 0;

//*****************************************************************************
//
// IO Slave Register Access ISR.
//
//*****************************************************************************
void ios_acc_isr(void)
{
    uint32_t ui32Status;

#ifdef TEST_ACC_ISR_RANDOMIZE
    static uint8_t shift = 0;
    uint8_t randomVal = (g_random[g_randomIdx] ^ g_random[(g_randomIdx + 5) & 0xFF]) >> (shift++);
    g_randomIdx +=7 ;
    shift &= 0x3;
    ui32Status = AM_REGVAL(0x50000214);

    if (randomVal) ((void (*)(uint32_t)) 0x0800009d)(randomVal);
    AM_REGVAL(0x50000218) = ui32Status;

#else
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTGET, &ui32Status);
//    am_hal_gpio_out_bit_set(23);
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTCLR, &ui32Status);
//    am_hal_gpio_out_bit_clear(23);

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
am_gpio0_203f_isr(void)
{
    am_hal_gpio_mask_t IntStatus;
    uint32_t    ui32IntStatus;

    am_hal_gpio_interrupt_status_get(AM_HAL_GPIO_INT_CHANNEL_0,
                                     false,
                                     &IntStatus);
//    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_INT_CHANNEL_0, &IntStatus);
    am_hal_gpio_interrupt_irq_status_get(GPIO0_203F_IRQn, true, &ui32IntStatus);
    am_hal_gpio_interrupt_irq_clear(GPIO0_203F_IRQn, ui32IntStatus);
    am_hal_gpio_interrupt_service(GPIO0_203F_IRQn,ui32IntStatus);
}

bool bHandshake = false;
uint32_t numHostInt = 0, numIoInt = 0;

// ISR callback for the host IOINT
static void hostint_handler(void *pArg)
{
    bHandshake = true;
    numHostInt++;
#if 0
    // TODO: Do we need to clear the bit by writing to IOS?
    if (g_sIosWidget.bSpi)
    {
        am_hal_iom_spi_read(g_sIosWidget.iom, 0,
            &data,1,AM_HAL_IOM_OFFSET(0xF9));
        if (data & HANDSHAKE_IOS_TO_IOM)
        {
            // Set bIosSendComplete
            bIosSendComplete = true;
        }
        am_hal_iom_spi_write_nb(g_sIosWidget.iom, 0,
            &data,1,AM_HAL_IOM_OFFSET(0xFA), 0);
    }
    else
    {
        am_hal_iom_i2c_write(g_sIosWidget.iom,
            g_sIosWidget.i2cAddr,
            &data,1,AM_HAL_IOM_OFFSET(0xFA), 0);
    }
#endif
}

uint32_t ioInt[8];
uint32_t ioIntIdx = 0;
uint32_t numXCMPRR = 0;
uint32_t numXCMPWR = 0;
uint32_t numXCMPRF = 0;
uint32_t numXCMPWF = 0;
uint32_t numDCMP = 0;

uint32_t numGenAd = 0;
//*****************************************************************************
//
// IO Slave Main ISR.
//
//*****************************************************************************
void ios_isr(void)
{
    uint32_t ui32Status;
    uint32_t ui32IntStatus;
    uint32_t ui32Arg;

    //
    // Check to see what caused this interrupt, then clear the bit from the
    // interrupt register.
    //
    am_hal_ios_interrupt_status_get(g_pIOSHandle, true, &ui32Status);

    am_hal_ios_interrupt_clear(g_pIOSHandle, ui32Status);

    g_ui32IosIsr |= ui32Status;

#ifdef TEST_IOS_DMA
    if (ui32Status & AM_HAL_IOS_INT_DCMP)
    {
        uint32_t ui32DMADir = 0;
        am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_GET_DMA_DIR, &ui32DMADir);
        if(ui32DMADir == AM_HAL_IOS_READ)
        {
            bDMACMP = true;
            am_hal_ios_interrupt_service(g_pIOSHandle, ui32Status);
            am_hal_cachectrl_range_t sRange;
            sRange.ui32StartAddr = (uint32_t)g_pIosSendBuf;
            sRange.ui32Size = DMA_PACK_SIZE;
            am_hal_cachectrl_dcache_invalidate(&sRange, false);
        }
        numDCMP++;
    }
#endif
    if (ui32Status & AM_HAL_IOS_INT_XCMPWR)
    {
#ifdef TEST_IOS_DMA
        uint8_t cmd = lram_array[DMA_DATA_TAG];
        uint16_t size = (lram_array[DMA_DATA_LENGTH]) | (lram_array[DMA_DATA_LENGTH + 1] << 8);
        switch(cmd)
        {
            case AM_DMATEST_CMD_START_DATA:
                // Host wants to start data exchange
                ios_dma_read(size);
                ui32Arg = HANDSHAKE_IOS_TO_IOM;
                // Notify the host
                am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_HOST_INTSET, &ui32Arg);
#if SLINT_GPIO
                am_hal_gpio_state_write(HANDSHAKE_IOS_PIN,AM_HAL_GPIO_OUTPUT_SET);
                am_hal_gpio_state_write(HANDSHAKE_IOS_PIN,AM_HAL_GPIO_OUTPUT_CLEAR);
#endif
                break;

            case AM_DMATEST_CMD_STOP_DATA:
                // Host no longer interested in data from us
                break;

            case AM_DMATEST_CMD_ACK_DATA:
                // Host done reading the last block signalled
                // Check if any more data available
                break;

            default:
                break;
        }
#endif
        // Accumulate data from Register space
        numXCMPWR++;
    }
    if (ui32Status & AM_HAL_IOS_INT_XCMPWF)
    {
        // Accumulate data from FIFO
        numXCMPWF++;
    }
    if (ui32Status & AM_HAL_IOS_INT_XCMPRR)
    {
        // Host completed Read from register space
        numXCMPRR++;
    }
    if (ui32Status & AM_HAL_IOS_INT_XCMPRF)
    {
#ifdef TEST_IOS_DMA
        uint32_t ui32DMADir = 0;
        am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_GET_DMA_DIR, &ui32DMADir);
        if(ui32DMADir == AM_HAL_IOS_WRITE)
        {
            bDMACMP = true;
            am_hal_ios_interrupt_service(g_pIOSHandle, ui32Status);
        }
#endif
        // Host completed Read from FIFO
        numXCMPRF++;
    }
    if (ui32Status & AM_HAL_IOS_INT_IOINTW)
    {
        // Host interrupting us
        // Read the IOINT register for appropriate bits
        // Set bIomSendComplete based on this
        ui32Arg = HANDSHAKE_IOM_TO_IOS;
        am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_HOST_INTGET, &ui32IntStatus);
        if (ui32IntStatus & ui32Arg)
        {
            bIomSendComplete = true;
            am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_HOST_INTCLR, &ui32Arg);
        }
    }
    if (ui32Status & AM_HAL_IOS_INT_FUNDFL)
    {
        // We should never hit this case unless the threshold has beeen set
        // incorrect
        // ERROR!
//        am_hal_debug_assert_msg(0,
//            "Hitting underflow for the requested IOS FIFO transfer.");
    }
    if (ui32Status & AM_HAL_IOS_INT_FOVFL)
    {
        // We should never hit this case unless the threshold has beeen set
        // incorrect
        // ERROR!
//        am_hal_debug_assert_msg(0,
//            "Hitting overflow for the requested IOS FIFO transfer.");
    }
    if (ui32Status & AM_HAL_IOS_INT_FRDERR)
    {
        // We should never hit this case
//        am_hal_debug_assert_msg(0,
//            "Hitting Read Error for the requested IOS FIFO transfer.");
    }

    if (ui32Status & AM_HAL_IOS_INT_FSIZE)
    {
        if (bContinuous)
        {
            // Add more data to SRAM
        }
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

//
//! Take over default ISR for IOM 0. (Queue mode service)
//
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

//
//! Take over default ISR for IOM 1. (Queue mode service)
//
void
am_iomaster1_isr(void)
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

//
//! Take over default ISR for IOM 1. (Queue mode service)
//
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

//
//! Take over default ISR for IOM 3. (Queue mode service)
//
void
am_iomaster3_isr(void)
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

//
//! Take over default ISR for IOM 4. (Queue mode service)
//
void
am_iomaster4_isr(void)
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

//
//! Take over default ISR for IOM 5. (Queue mode service)
//
void
am_iomaster5_isr(void)
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

//
//! Take over default ISR for IOM 6. (Queue mode service)
//
void
am_iomaster6_isr(void)
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

//
//! Take over default ISR for IOM 7. (Queue mode service)
//
void
am_iomaster7_isr(void)
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


void
receive_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    am_hal_cachectrl_range_t sRange;
    sRange.ui32StartAddr = (uint32_t)g_pIomRecvBuf;
    sRange.ui32Size = AM_IOS_TX_BUFSIZE_MAX;
    am_hal_cachectrl_dcache_invalidate(&sRange, false);

    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("\nIOS Read Failed 0x%x\n", transactionStatus);
    }
    else
    {
        bIomRecvComplete = true;
    }
}

// Returns expected ACC ISR value based on packet size and address for received
// data on LRAM region
static uint32_t expectedAccIsr(uint8_t address, uint8_t size)
{
    uint32_t bitMask = 0, temp = 0;
    int32_t i;
    // Identify 16 most significant bits
    if (address < 16)
    {
        for (i = address; ((i < 16) && (i < (address + size))); i++)
        {
            bitMask |= (1 << (15 - i));
        }
        temp = 0 ;
    }
    else
    {
        bitMask = 0;
        temp = (address - 16) >> 2;
    }
    bitMask <<= 16;

    // Now the least significant 16 bits
    for (i = temp + 1; (i <= (address + size - 16)>>2); i++)
    {
        bitMask |= 0x8000 >> (i-1);
    }
    return bitMask;
}

//
//! Empty the buffers.
//! Load output buffer with repeated text string.
//! Text string without 0 termination.
//
static int load_buffers(int num_addr)
{
    int i = 0; // Return number of bytes loaded.
    for (i = 0; i < sizeof(g_pIomRecvBuf); i++)
    {
        g_pIomRecvBuf[i] = INBUFFER_EMPTY;
    }
    for (i = 0; i < sizeof(g_pIosSendBuf); i++)
    {
        g_pIosSendBuf[i] = INBUFFER_EMPTY;
    }
    for (i = 0; i < sizeof(g_pRefData); i++)
    {
        g_pRefData[i] = INBUFFER_EMPTY;
    }
    for (i = 0; i < num_addr; i++)
    {
        g_pRefData[i] = i & 0xFF;
    }
    return i;
}

// Prepare IOS data with a pattern
static void
ios_buf_init(uint32_t size)
{
    uint32_t i;
    size = (size > AM_IOS_TX_BUFSIZE_MAX) ? AM_IOS_TX_BUFSIZE_MAX: size;
    for (i = 0; i < size; i++)
    {
        // Read data and prepare to be sent back after processing
        g_pIosSendBuf[i] = g_pRefData[i] ^ TEST_XOR_BYTE;
    }
}

// Verify received data from IOS against the reference
// return non-zero if fail
static bool
verify_result(am_widget_iosspi_t *pIosSpiWidget)
{
    uint32_t i;
    uint32_t address = pIosSpiWidget->testCfg.address;
    uint32_t size = pIosSpiWidget->testCfg.size;
    uint32_t fifoCtr, fifoSize;
    // Special handling for RO Data space
    if ((address != 0x7F) && ((address + size) > pIosSpiWidget->directSize))
    {
        // Check the trailing data portion here, and remaining will be checked later
        for (i = pIosSpiWidget->directSize; i < address + size; i++)
        {
            if (g_pIomRecvBuf[i] != ROBUFFER_INIT)
            {
                return true;
            }
        }
        // Remaining data to be verified
        size = pIosSpiWidget->directSize - address;
    }
    for (i = 0; i < size; i++)
    {
        if (g_pRefData[i] != (g_pIomRecvBuf[i] ^ TEST_XOR_BYTE))
        {
            am_util_stdio_printf("Buffer Validation failed @i=%d Rcvd 0x%x Expected 0x%x\n",
                i, g_pIomRecvBuf[i], g_pRefData[i] ^ TEST_XOR_BYTE);
            return true;
        }
    }
    // Verify that the RO & AHBRAM data is intact
    for (i = 0; i < pIosSpiWidget->roSize; i++)
    {
#ifndef NO_SHELBY_1643
        // SHELBY-1643
        if (((pIosSpiWidget->roBase + i) >= 0x78) && ((pIosSpiWidget->roBase + i) <= 0x7B))
        {
            continue;
        }
#endif
        if (lram_array[pIosSpiWidget->roBase + i] != ROBUFFER_INIT)
        {
            return true;
        }
    }
    for (i = 0; i < pIosSpiWidget->ahbRamSize; i++)
    {
        if (lram_array[pIosSpiWidget->ahbRamBase + i] != AHBBUF_INIT)
        {
            return true;
        }
    }
    // Make sure the Hardware FIFOSIZ is 0
    fifoCtr = IOSLAVEn(TEST_IOS_MODULE)->FIFOCTR_b.FIFOCTR;
    fifoSize = IOSLAVEn(TEST_IOS_MODULE)->FIFOPTR_b.FIFOSIZ;
    if (fifoCtr || fifoSize)
    {
        am_util_stdio_printf("Hardware Pointers not correct FIFOCTR 0x%x FIFOSIZ 0x%x\n",
            fifoCtr, fifoSize);
        return true;
    }
    return false;
}

// Indicate the slave
static void iom_send_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    am_hal_iom_transfer_t       Transaction;

    uint32_t data = 0x1;
    // Send data using the FIFO
    // In the completion callback - write to IOS to cause IOINT
    Transaction.ui32InstrLen    = 1;
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes    = 1;
    Transaction.pui32TxBuffer   = &data;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    if (g_sIosWidget.bSpi)
    {
      Transaction.ui64Instr = 0xFB;
      Transaction.uPeerInfo.ui32SpiChipSelect = AM_HAL_GPIO_NCE_IOM0CE0  + 4 * g_sIosWidget.iom;
    }
    else
    {
      Transaction.ui64Instr = 0x7B;
      Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
    }
    am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
}

// Send test data to IOS
static void
iom_recv(uint32_t address, uint32_t size)
{
    am_hal_iom_transfer_t       Transaction;

    Transaction.ui32InstrLen    = 1;
    Transaction.ui64Instr = address;
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32NumBytes    = size;
    Transaction.pui32RxBuffer   = (uint32_t *)g_pIomRecvBuf;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    if (g_sIosWidget.bSpi)
    {
        Transaction.uPeerInfo.ui32SpiChipSelect = AM_HAL_GPIO_NCE_IOM0CE0 + 4 * g_sIosWidget.iom;
    }
    else
    {
        Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
    }
    am_hal_iom_nonblocking_transfer(g_pIOMHandle, &Transaction, receive_complete, 0);
}

// Send test data to IOS
static void
iom_send(uint32_t address, uint32_t size)
{
    am_hal_iom_transfer_t       Transaction;

    Transaction.ui32InstrLen    = 1;
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes    = size;
    Transaction.pui32TxBuffer   = (uint32_t *)g_pRefData;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    if (g_sIosWidget.bSpi)
    {
        Transaction.ui64Instr = (0x80|address);
        Transaction.uPeerInfo.ui32SpiChipSelect = AM_HAL_GPIO_NCE_IOM0CE0 + 4 * g_sIosWidget.iom;
    }
    else
    {
        Transaction.ui64Instr = address;
        Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
    }
    am_hal_iom_nonblocking_transfer(g_pIOMHandle, &Transaction, iom_send_complete, 0);
}

static void
iom_recv_continue(uint32_t address, uint32_t size)
{
    am_hal_iom_transfer_t       Transaction;
    uint32_t dataPtr = 0;
    uint32_t ui32Size = 0;
    uint32_t ui32TotSize = size;

    Transaction.ui64Instr = address;
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    if (g_sIosWidget.bSpi)
    {
        Transaction.uPeerInfo.ui32SpiChipSelect = AM_HAL_GPIO_NCE_IOM0CE0 + 4 * g_sIosWidget.iom;
    }
    else
    {
        Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
    }
    do
    {
        ui32Size = (size > 2048) ? 2048 : size;
        Transaction.ui32NumBytes    = ui32Size;
        Transaction.pui32RxBuffer   = (uint32_t *)&g_pIomRecvBuf[dataPtr];

        if(ui32TotSize == size)
        {
            Transaction.ui32InstrLen    = 1;
        }
        else
        {
            Transaction.ui32InstrLen    = 0;
            Transaction.ui64Instr = 0;
        }

        if(size <= 2048)
        {
            Transaction.bContinue       = false;
            am_hal_iom_nonblocking_transfer(g_pIOMHandle, &Transaction, receive_complete, 0);
        }
        else
        {
            Transaction.bContinue       = true;
            am_hal_iom_nonblocking_transfer(g_pIOMHandle, &Transaction, 0, 0);
        }
        size -= ui32Size;
        dataPtr += ui32Size;
    }while(size);
}

static void
iom_send_continue(uint32_t address, uint32_t size)
{
    am_hal_iom_transfer_t       Transaction;
    uint32_t dataPtr = 0;
    uint32_t ui32Size = 0;
    uint32_t ui32TotSize = size;

    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    if (g_sIosWidget.bSpi)
    {
        Transaction.ui64Instr = (0x80|address);
        Transaction.uPeerInfo.ui32SpiChipSelect = AM_HAL_GPIO_NCE_IOM0CE0 + 4 * g_sIosWidget.iom;
    }
    else
    {
        Transaction.ui64Instr = address;
        Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
    }

    do
    {
        ui32Size = (size > 2048) ? 2048 : size;
        Transaction.ui32NumBytes    = ui32Size;
        Transaction.pui32TxBuffer   = (uint32_t *)&g_pRefData[dataPtr];

        if(ui32TotSize == size)
        {
            Transaction.ui32InstrLen    = 1;
        }
        else
        {
            Transaction.ui32InstrLen    = 0;
            Transaction.ui64Instr = 0;
        }

        if(size <= 2048)
        {
            Transaction.bContinue       = false;
            am_hal_iom_nonblocking_transfer(g_pIOMHandle, &Transaction, iom_send_complete, 0);
        }
        else
        {
            Transaction.bContinue       = true;
            am_hal_iom_nonblocking_transfer(g_pIOMHandle, &Transaction, 0, 0);
        }
        size -= ui32Size;
        dataPtr += ui32Size;
    }while(size);
}

// Receive data from host and prepare the loop back data
static void ios_read(uint32_t address, uint32_t size)
{
    uint32_t i;
    uint8_t readByte;
    // Read only supported from LRAM
    for (i = 0; i < size; i++)
    {
        readByte = lram_array[address+i];
        // Read data and prepare to be sent back after processing
        g_pIosSendBuf[i] = readByte ^ TEST_XOR_BYTE;
    }
}

// Send data to host (IOM)
static uint32_t ios_send(uint32_t address, uint32_t size)
{
    uint32_t ui32SentSize = 0;
    uint32_t ui32Arg = HANDSHAKE_IOS_TO_IOM;
    // Send data using the LRAM or FIFO
    if (address == 0x7F)
    {
        am_hal_ios_fifo_write(g_pIOSHandle, &g_pIosSendBuf[0], size, &ui32SentSize);
        am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_FIFO_UPDATE_CTR, NULL);
    }
    else
    {
        while (size--)
        {
            lram_array[address+size] = g_pIosSendBuf[size];
        }
    }
    // Notify the host
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_HOST_INTSET, &ui32Arg);
#if SLINT_GPIO
    am_hal_gpio_state_write(HANDSHAKE_IOS_PIN,AM_HAL_GPIO_OUTPUT_SET);
    am_hal_gpio_state_write(HANDSHAKE_IOS_PIN,AM_HAL_GPIO_OUTPUT_CLEAR);
#endif
    return 0;
}

#ifdef TEST_IOS_DMA
static uint32_t ios_dma_read(uint32_t size)
{
    uint32_t ui32ReadSize = 0;
    am_hal_ios_transfer_t Transaction;
    // Read data using FIFO only
    bDMACMP = false;
    Transaction.eDirection = AM_HAL_IOS_READ;
    Transaction.pui32RxBuffer = (uint32_t *)g_pIosSendBuf;
    Transaction.ui32NumBytes[AM_HAL_IOS_READ] = size;
    Transaction.pui32BytesTransferred[AM_HAL_IOS_READ] = &ui32ReadSize;
    Transaction.ui8Priority = 0;
    Transaction.pfnCallback[AM_HAL_IOS_READ] = NULL;
    am_hal_ios_dma_transfer(g_pIOSHandle, &Transaction);

    return 0;
}

static uint32_t ios_dma_send(uint32_t size)
{
    uint32_t ui32SentSize = 0;
    am_hal_ios_transfer_t Transaction;
    // Send data using FIFO only
    bDMACMP = false;
    Transaction.eDirection = AM_HAL_IOS_WRITE;
    Transaction.pui32TxBuffer = (uint32_t *)g_pIosSendBuf;
    Transaction.ui32NumBytes[AM_HAL_IOS_WRITE] = size;
    Transaction.pui32BytesTransferred[AM_HAL_IOS_WRITE] = &ui32SentSize;
    Transaction.ui8Priority = 0;
    Transaction.pfnCallback[AM_HAL_IOS_WRITE] = NULL;
    am_hal_ios_dma_transfer(g_pIOSHandle, &Transaction);

    return 0;
}
#endif
//*****************************************************************************
//
// Timer handling to implement timeout
//
//*****************************************************************************
static volatile bool g_bTimeOut = 0;

// Function to initialize Timer A0 to interrupt every X second.
static void
timeout_check_init(uint32_t seconds)
{
    am_hal_timer_config_t TimerCfg;

    //
    // Configure the test timer.
    //
    am_hal_timer_reset_config(AM_TEST_TIMER);
    am_hal_timer_default_config_set(&TimerCfg);
    TimerCfg.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
// #### INTERNAL BEGIN ####
#if defined(APOLLO5_FPGA)
    TimerCfg.ui32Compare0 = (APOLLO5_FPGA * 1000000) / (1000000 * seconds);
#else
// #### INTERNAL END ####
    TimerCfg.ui32Compare0 = 96000000 / (1000000 * seconds);  // FIXME - Need to check this setting.
// #### INTERNAL BEGIN ####
#endif
// #### INTERNAL END ####
    am_hal_timer_config(AM_TEST_TIMER, &TimerCfg);

    //
    // Clear the timer interrupt
    //
    am_hal_timer_interrupt_clear(3 << (2 * AM_TEST_TIMER));

    g_bTimeOut = 0;

    //
    // Enable the timer interrupt.
    //
    am_hal_timer_enable(AM_TEST_TIMER);

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_EnableIRQ(TIMER_IRQn);
    am_hal_interrupt_master_enable();

    //
    // Start test timer
    //
    am_hal_timer_start(AM_TEST_TIMER);
}

// Timer Interrupt Service Routine (ISR)
void
am_ctimer_isr(void)
{
    //
    // Clear the test timer interrupt (write to clear).
    //
    am_hal_timer_clear(AM_TEST_TIMER);

    // Set Timeout flag
    g_bTimeOut = 1;
}


// We need to shift one extra
#define GET_I2CADDR(cfg)    \
    (((cfg) & IOSLAVE_CFG_I2CADDR_Msk) >> (IOSLAVE_CFG_I2CADDR_Pos+1))

uint32_t am_widget_iosspi_setup(am_widget_iosspi_config_t *pIosCfg,
    void **ppWidget, char *pErrStr)
{
    am_hal_pwrctrl_sram_memcfg_t  SRAMCfg =
    {
      .eSRAMCfg       = AM_HAL_PWRCTRL_SRAM_3M,
      .eActiveWithMCU = AM_HAL_PWRCTRL_SRAM_3M,
      .eSRAMRetain    = AM_HAL_PWRCTRL_SRAM_3M
    };

    uint32_t ui32Status = 0, i;
    g_sIosWidget.directSize = g_sIosWidget.roBase =
        pIosCfg->stimulusCfgIos.iosHalCfg.ui32ROBase;
    g_sIosWidget.roSize =
        pIosCfg->stimulusCfgIos.iosHalCfg.ui32FIFOBase - pIosCfg->stimulusCfgIos.iosHalCfg.ui32ROBase;
    g_sIosWidget.fifoSize =
        pIosCfg->stimulusCfgIos.iosHalCfg.ui32RAMBase - pIosCfg->stimulusCfgIos.iosHalCfg.ui32FIFOBase;
    g_sIosWidget.ahbRamBase = pIosCfg->stimulusCfgIos.iosHalCfg.ui32RAMBase;
#if (TEST_IOS_MODULE == 0)
    g_sIosWidget.ahbRamSize = 0x100 - g_sIosWidget.ahbRamBase;
#else
    g_sIosWidget.ahbRamSize = 0x40 - g_sIosWidget.ahbRamBase;
#endif
    g_sIosWidget.iom = pIosCfg->stimulusCfgIom.iomModule;
    pIosCfg->stimulusCfgIos.iosHalCfg.pui8SRAMBuffer = g_pui8TxFifoBuffer;
    pIosCfg->stimulusCfgIos.iosHalCfg.ui32SRAMBufferCap = AM_IOS_TX_BUFSIZE_MAX;
    g_sIosWidget.bSpi =
        (_FLD2VAL(IOSLAVE_CFG_IFCSEL, pIosCfg->stimulusCfgIos.iosHalCfg.ui32InterfaceSelect));
    g_sIosWidget.i2cAddr =
        GET_I2CADDR(pIosCfg->stimulusCfgIos.iosHalCfg.ui32InterfaceSelect);

    am_hal_interrupt_master_enable();
    // Set up the IOS
    ios_set_up(&pIosCfg->stimulusCfgIos);
    // Set up the stimulus IOM
    iom_set_up(&pIosCfg->stimulusCfgIom);
    // Set up interrupts
    *ppWidget = &g_sIosWidget;
    // Register handler for IOS => IOM interrupt

    am_hal_gpio_interrupt_register(AM_HAL_GPIO_INT_CHANNEL_0, HANDSHAKE_IOM_PIN, hostint_handler, NULL);

    ui32Status = am_hal_pwrctrl_sram_config(&SRAMCfg);

    // Initialize RO & AHB-RAM data with pattern
    for (i = 0; i < g_sIosWidget.roSize; i++)
    {
        lram_array[g_sIosWidget.roBase + i] = ROBUFFER_INIT;
    }
    for (i = 0; i < g_sIosWidget.ahbRamSize; i++)
    {
        lram_array[g_sIosWidget.ahbRamBase + i] = AHBBUF_INIT;
    }
    return ui32Status;
}


uint32_t am_widget_iosspi_cleanup(void *pWidget, char *pErrStr)
{
    memset(&g_sIosWidget, 0, sizeof(am_widget_iosspi_t));
    // Set up the stimulus IOM
    iom_clean_up();
    // Set up the IOS
    ios_clean_up();
    am_hal_interrupt_master_disable();
    return 0;
}

// IOS interrupt bits to verify
// Mask off FSIZE and IOINT bits
#define IOS_ISR_CHK_MASK (~(AM_HAL_IOS_INT_IOINTW | AM_HAL_IOS_INT_FSIZE | AM_HAL_IOS_INT_GENAD))
uint32_t
am_widget_iosspi_test(void *pWidget, void *pTestCfg, char *pErrStr)
{
    am_widget_iosspi_t *pIosSpiWidget = (am_widget_iosspi_t *)pWidget;
    am_hal_iom_transfer_t       Transaction;
    uint32_t expectedIosIsr = 0;
    uint32_t ui32Status = 0;
    uint32_t checkMask;
#ifndef TEST_IOINTCTL
    uint32_t ui32HostStatus = 0;
    uint32_t ui32Arg = HANDSHAKE_IOM_TO_IOS;
#endif

    pIosSpiWidget->testCfg = *((am_widget_iosspi_test_t *)pTestCfg);

    // Initialize Data Buffers
    load_buffers(pIosSpiWidget->testCfg.size);
    am_hal_sysctrl_bus_write_flush();

    // Use address to determine the mode - LRAM or FIFO
    if (pIosSpiWidget->testCfg.address != 0x7F)
    {
        g_ui32AccIsr = 0;
        for (int i = 0; i < 32; i++)
        {
            accIsrStatus[i] = 0;
        }
        accIsrIdx = 0;
        g_ui32IosIsr = 0;
        bIomSendComplete = false;
        // Trigger IOM to Write at specified address
        // handshake between IOM and IOS using IOCTL interrupts
        // IOM indicates IOS on completion of each batch
        iom_send(pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size);
        if (AM_TEST_TIMEOUT)
        {
            timeout_check_init(AM_TEST_TIMEOUT);
        }
        while (1)
        {
            uint32_t ui32IntStatus = am_hal_interrupt_master_disable();
#ifndef TEST_IOINTCTL
            am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_HOST_INTGET, &ui32HostStatus);

            if (ui32HostStatus & ui32Arg)
            {
                bIomSendComplete = true;
                am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_HOST_INTCLR, &ui32Arg);
            }
#endif
            if (bIomSendComplete || g_bTimeOut)
            {
                am_hal_interrupt_master_set(ui32IntStatus);
                break;
            }
#ifdef TEST_IOINTCTL
            // Wait for interrupt indicating IOM write complete
            am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
#endif
            am_hal_interrupt_master_set(ui32IntStatus);
#ifndef TEST_IOINTCTL
            am_util_delay_us(1);
#endif
        }
        if (g_bTimeOut)
        {
            // Timeout!
            am_util_stdio_sprintf(pErrStr, "Transaction timed out doing iom_send\n");
            ui32Status = 0xFFFFFFFF;
            return ui32Status;
        }
        bIomSendComplete = false;
        expectedIosIsr = 0;
        // IOS to verify the ACC interrupts for the size received for LRAM writes
#ifndef TEST_ACC_INT
        am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTGET, &g_ui32AccIsr);
        am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTCLR, &g_ui32AccIsr);
#endif
        if (g_ui32AccIsr != expectedAccIsr(pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size))
        {
#if 1
            // ACC Interrupts fail!
            am_util_stdio_sprintf(pErrStr, "Unexpected ACC Interrupt 0x%x: expected 0x%x\n",
                g_ui32AccIsr, expectedAccIsr(pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size));
            {
                uint32_t bitMiss = g_ui32AccIsr ^ expectedAccIsr(pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size);
                for (int i = 0; i < 32; i++)
                {
                    if (bitMiss & 0x1)
                    {
                        missingAccIsr[31-i]++;
                    }
                    bitMiss >>= 1;
                    if (bitMiss == 0) break;
                }
            }
            return 1;
#else
            am_util_stdio_printf("Addr:0x%x, Size:0x%x - Unexpected ACC Interrupt 0x%x:Expected 0x%x\n",
                pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size,
                g_ui32AccIsr, expectedAccIsr(pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size));
#endif
        }
        checkMask = ~(AM_HAL_IOS_INT_IOINTW | AM_HAL_IOS_INT_FSIZE);
        // For Apollo2 - IOS to also verify the CMP interrupts
        expectedIosIsr |= IOSLAVE_INTSTAT_XCMPWR_Msk;
#ifdef TEST_ACC_INT
#ifndef NO_SHELBY_1658
        // SHELBY-1658
        checkMask &= ~AM_REG_IOSLAVE_INTSTAT_XCMPWR_M;
#endif
#endif
#ifndef TEST_IOS_XCMP_INT
        am_hal_ios_interrupt_status_get(g_pIOSHandle, false, &g_ui32IosIsr);
        am_hal_ios_interrupt_clear(g_pIOSHandle, g_ui32IosIsr);
#endif
        if ((g_ui32IosIsr & checkMask) != (expectedIosIsr & checkMask))
        {
#if 0
            // IOS Interrupts fail!
            am_util_stdio_sprintf(pErrStr, "Unexpected IOS Interrupt 0x%x\n",
                g_ui32IosIsr);
            return 1;
#else
            DIAG_SUPPRESS_VOLATILE_ORDER()

            am_util_stdio_printf("\nAddr:0x%x, Size:0x%x - Unexpected IOS Interrupt on Write 0x%x: mask 0x%x: Actual 0x%x: expected 0x%x\n",
                                   pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size,
                                   g_ui32IosIsr, checkMask, (g_ui32IosIsr & checkMask), expectedIosIsr);

            DIAG_DEFAULT_VOLATILE_ORDER()
#endif
        }
        // IOS will accumulate the data if needed on READ side
        // When complete, the data will be turned around back to IOM
        // Read the received data
        ios_read(pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size);
        // Write the reply (bit inverted of received data)
    }
    else
    {
        // Prepare a patterned data from IOS
        ios_buf_init(pIosSpiWidget->testCfg.size);
        if (pIosSpiWidget->testCfg.size > AM_IOS_TX_BUFSIZE_MAX)
        {
            // This is a special test, to test host-slave continuous data transfer
            // with handshake based on FIFOCTR
            bContinuous = true;
            // Initilize state for the continuous feeding
            ios_feed_state.size = pIosSpiWidget->testCfg.size;
            ios_feed_state.totalWritten = 0;
        }
    }

    if (bContinuous == false)
    {
        bIosSendComplete = false;
        ios_send(pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size);
        // Handshake will use the interrupt IOCTL - to signal host that data is ready
        if (AM_TEST_TIMEOUT)
        {
            timeout_check_init(AM_TEST_TIMEOUT);
        }
        while (1)
        {
            // Disable interrupts before checking the flags
            uint32_t ui32IntStatus = am_hal_interrupt_master_disable();
            if (bIosSendComplete || g_bTimeOut)
            {
                // re-enable interrupts
                am_hal_interrupt_master_set(ui32IntStatus);
                break;
            }

            if (bHandshake == true)
            {
                uint32_t data = 0;
                // re-enable interrupts
                am_hal_interrupt_master_set(ui32IntStatus);
                bHandshake = false;
                // Read & Clear the IOINT status
                if (g_sIosWidget.bSpi)
                {
                  Transaction.ui32InstrLen    = 1;
                  Transaction.ui64Instr = 0x79;
                  Transaction.eDirection      = AM_HAL_IOM_RX;
                  Transaction.uPeerInfo.ui32SpiChipSelect = AM_HAL_GPIO_NCE_IOM0CE0 + 4 * g_sIosWidget.iom;
                  Transaction.ui32NumBytes    = 1;
                  Transaction.pui32RxBuffer   = &data;
                  Transaction.bContinue       = false;
                  Transaction.ui8RepeatCount  = 0;
                  Transaction.ui32PauseCondition = 0;
                  Transaction.ui32StatusSetClr = 0;
                  am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
                  // We need to clear the bit by writing to IOS
                  if (data & HANDSHAKE_IOS_TO_IOM)
                  {
                    data = HANDSHAKE_IOS_TO_IOM;
                    Transaction.ui32InstrLen    = 1;
                    Transaction.ui64Instr = 0xFA;
                    Transaction.eDirection      = AM_HAL_IOM_TX;
                    Transaction.uPeerInfo.ui32SpiChipSelect = AM_HAL_GPIO_NCE_IOM0CE0 + 4 * g_sIosWidget.iom;
                    Transaction.ui32NumBytes    = 1;
                    Transaction.pui32TxBuffer   = &data;
                    Transaction.bContinue       = false;
                    Transaction.ui8RepeatCount  = 0;
                    Transaction.ui32PauseCondition = 0;
                    Transaction.ui32StatusSetClr = 0;
                    am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
                     // Set bIosSendComplete
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
                     // We need to clear the bit by writing to IOS
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
                        // Set bIosSendComplete
                        bIosSendComplete = true;
                        break;
                    }
                }
            }
            else
            {
                am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
                // re-enable interrupts
                am_hal_interrupt_master_set(ui32IntStatus);
            }
        }
        if (g_bTimeOut)
        {
            // Timeout!
            am_util_stdio_sprintf(pErrStr, "Transaction timed out doing ios_send\n");
            ui32Status = 0xFFFFFFFF;
            return ui32Status;
        }
        bIosSendComplete = false;

        // For FIFO case, verify the FIFOCTR
        if (pIosSpiWidget->testCfg.address == 0x7F)
        {
            uint32_t iosSize = 0;
            if (g_sIosWidget.bSpi)
            {
              Transaction.ui32InstrLen    = 1;
              Transaction.ui64Instr = 0x7C;
              Transaction.eDirection      = AM_HAL_IOM_RX;
              Transaction.uPeerInfo.ui32SpiChipSelect = AM_HAL_GPIO_NCE_IOM0CE0 + 4 * g_sIosWidget.iom;
              Transaction.ui32NumBytes    = 2;
              Transaction.pui32RxBuffer   = &iosSize;
              Transaction.bContinue       = false;
              Transaction.ui8RepeatCount  = 0;
              Transaction.ui32PauseCondition = 0;
              Transaction.ui32StatusSetClr = 0;
              am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
            }
            else
            {
              Transaction.ui32InstrLen    = 1;
              Transaction.ui64Instr = 0x7C;
              Transaction.eDirection      = AM_HAL_IOM_RX;
              Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
              Transaction.ui32NumBytes    = 2;
              Transaction.pui32RxBuffer   = &iosSize;
              Transaction.bContinue       = false;
              Transaction.ui8RepeatCount  = 0;
              Transaction.ui32PauseCondition = 0;
              Transaction.ui32StatusSetClr = 0;
              am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
            }
            if (iosSize != pIosSpiWidget->testCfg.size)
            {
#if 0
                am_util_stdio_sprintf(pErrStr, "FIFOCTR incorrect Actual=0x%x, Expected=0x%x\n",
                    iosSize, pIosSpiWidget->testCfg.size);
                return 0xFFFFFFFF;
#else
                am_util_stdio_printf("\nFIFOCTR incorrect Actual=0x%x, Expected=0x%x\n",
                    iosSize, pIosSpiWidget->testCfg.size);
#endif
            }
        }

        g_ui32IosIsr = 0;
        expectedIosIsr = 0;
        bIomRecvComplete = false;

        iom_recv(pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size);

        if (AM_TEST_TIMEOUT)
        {
            timeout_check_init(AM_TEST_TIMEOUT);
        }
        while (1)
        {
            // Disable interrupts before checking the flags
            uint32_t ui32IntStatus = am_hal_interrupt_master_disable();
            if (bIomRecvComplete || g_bTimeOut)
            {
                // re-enable interrupts
                am_hal_interrupt_master_set(ui32IntStatus);
                break;
            }
            am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
            // re-enable interrupts
            am_hal_interrupt_master_set(ui32IntStatus);
        }

        if (g_bTimeOut)
        {
            // Timeout!
            am_util_stdio_sprintf(pErrStr, "Transaction timed out doing iom_recv\n");
            ui32Status = 0xFFFFFFFF;
            return ui32Status;
        }

        checkMask = ~(AM_HAL_IOS_INT_IOINTW | AM_HAL_IOS_INT_FSIZE);
        // For Apollo2 - IOS to also verify the CMP interrupts
        if (pIosSpiWidget->testCfg.address == 0x7F)
        {
            expectedIosIsr |= IOSLAVE_INTSTAT_XCMPRF_Msk;
        }
        else
        {
            expectedIosIsr |= (IOSLAVE_INTSTAT_XCMPRR_Msk);
        }
#ifndef NO_SHELBY_1652
        // SHELBY-1652
        if (pIosSpiWidget->testCfg.address == 0x77)
        {
            checkMask &= ~(AM_REG_IOSLAVE_INTSTAT_XCMPRR_M);
        }
#endif
#ifndef NO_SHELBY_1657
        // SHELBY-1657
        if (pIosSpiWidget->testCfg.address <= 1)
        {
            checkMask &= ~(AM_REG_IOSLAVE_INTSTAT_GENAD_M);
        }
#endif
#ifndef TEST_IOS_XCMP_INT
        am_hal_ios_interrupt_status_get(g_pIOSHandle, false, &g_ui32IosIsr);
        am_hal_ios_interrupt_clear(g_pIOSHandle, g_ui32IosIsr);
#endif
        // Check for the interrupt status
        if ((g_ui32IosIsr & checkMask) != (expectedIosIsr & checkMask))
        {
#if 0
            // IOS Interrupts fail!
            am_util_stdio_sprintf(pErrStr, "Unexpected IOS Interrupt 0x%x:0x%x\n",
                g_ui32IosIsr, expectedIosIsr);
            return 1;
#else
            DIAG_SUPPRESS_VOLATILE_ORDER()

            am_util_stdio_printf("\nAddr:0x%x, Size:0x%x - Unexpected IOS Interrupt on Read 0x%x: mask 0x%x: Actual 0x%x: expected 0x%x\n",
                pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size,
                g_ui32IosIsr, checkMask, (g_ui32IosIsr & checkMask), expectedIosIsr);

            DIAG_DEFAULT_VOLATILE_ORDER()
#endif
        }
        // Verify received data at IOM
        ui32Status = verify_result(pIosSpiWidget);
        if (ui32Status)
        {
            am_util_stdio_sprintf(pErrStr, "Buffer verification failed\n");
            return ui32Status;
        }
    }
    else
    {
        // We implement a continous loop to feed data from Slave to host
        // up to given size
    }
    return ui32Status;
}

#ifdef TEST_IOS_DMA
uint32_t
am_widget_iosspi_dma_test(void *pWidget, void *pTestCfg, char *pErrStr)
{
    am_widget_iosspi_t *pIosSpiWidget = (am_widget_iosspi_t *)pWidget;
    am_hal_iom_transfer_t       Transaction;
    stHandshake stHandshake = {0};
    uint32_t ui32Arg = 0;
    // Don't need acc irq in this case
    NVIC_DisableIRQ(IOS_ACC_IRQ);

    pIosSpiWidget->testCfg = *((am_widget_iosspi_test_t *)pTestCfg);

    // Initialize Data Buffers
    load_buffers(pIosSpiWidget->testCfg.size);
    am_hal_sysctrl_bus_write_flush();

#ifdef IOS_DMA_READ_TEST
    bIomSendComplete = false;

#ifdef IOS_DMA_HANDSHAKE_TEST
    // First handshake
    stHandshake.ui16Tag = AM_DMATEST_CMD_START_DATA;
    stHandshake.ui16Size = pIosSpiWidget->testCfg.size;
    Transaction.ui32InstrLen    = 1;
    Transaction.eDirection      = AM_HAL_IOM_TX;
    if (g_sIosWidget.bSpi)
    {
        Transaction.ui64Instr = 0x80;
        Transaction.uPeerInfo.ui32SpiChipSelect = AM_HAL_GPIO_NCE_IOM0CE0 + 4 * g_sIosWidget.iom;
    }
    else
    {
        Transaction.ui64Instr = 0;
        Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
    }
    Transaction.ui32NumBytes    = sizeof(stHandshake);
    Transaction.pui32TxBuffer   = (uint32_t*)&stHandshake;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);

    while (1)
    {
        // Disable interrupts before checking the flags
        uint32_t ui32IntStatus = am_hal_interrupt_master_disable();
        if (bHandshake == true)
        {
            // re-enable interrupts
            am_hal_interrupt_master_set(ui32IntStatus);
            bHandshake = false;
            break;
        }
        else
        {
            am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
            // re-enable interrupts
            am_hal_interrupt_master_set(ui32IntStatus);
        }
    }
#else
    ios_dma_read(pIosSpiWidget->testCfg.size);
#endif
    iom_send_continue(pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size);

    while (1)
    {
        // Disable interrupts before checking the flags
        uint32_t ui32IntStatus = am_hal_interrupt_master_disable();
        if ((bIomSendComplete) && (bDMACMP))
        {
            uint32_t data = 0;
            // re-enable interrupts
            am_hal_interrupt_master_set(ui32IntStatus);
            // Clear the IOINT status
            data = 0xFF;
            Transaction.ui32InstrLen    = 1;
            Transaction.eDirection      = AM_HAL_IOM_TX;
            if (g_sIosWidget.bSpi)
            {
                Transaction.ui64Instr = 0xFA;
                Transaction.uPeerInfo.ui32SpiChipSelect = AM_HAL_GPIO_NCE_IOM0CE0 + 4 * g_sIosWidget.iom;
            }
            else
            {
                Transaction.ui64Instr = 0x7A;
                Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
            }
            Transaction.ui32NumBytes    = 1;
            Transaction.pui32TxBuffer   = &data;
            Transaction.bContinue       = false;
            Transaction.ui8RepeatCount  = 0;
            Transaction.ui32PauseCondition = 0;
            Transaction.ui32StatusSetClr = 0;
            am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
            break;
        }
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
        // re-enable interrupts
        am_hal_interrupt_master_set(ui32IntStatus);
    }

    for(uint32_t i = 0; i < pIosSpiWidget->testCfg.size; i++)
    {
        if(g_pRefData[i] != g_pIosSendBuf[i])
        {
            am_util_stdio_printf("Read TEST: Buffer Validation failed @i=%d Rcvd 0x%x Expected 0x%x\n",
                i, g_pIosSendBuf[i], g_pRefData[i]);
            return 1;
        }
        g_pIosSendBuf[i] ^= ~TEST_XOR_BYTE;
    }
    //ui32Arg = pIosSpiWidget->roBase + pIosSpiWidget->roSize;
    //am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_SET_FIFO_PTR, &ui32Arg);
#else
    ios_buf_init(pIosSpiWidget->testCfg.size);
#endif

#ifdef IOS_DMA_WRITE_TEST
    bIomRecvComplete = false;

#ifdef IOS_DMA_HANDSHAKE_TEST
    // First handshake
    stHandshake.ui16Tag = AM_DMATEST_CMD_ACK_DATA;
    stHandshake.ui16Size = pIosSpiWidget->testCfg.size;
    memcpy(&lram_array[sizeof(stHandshake)], &stHandshake, sizeof(stHandshake));
    memset(&stHandshake, 0, sizeof(stHandshake));

    ui32Arg = HANDSHAKE_IOS_TO_IOM;
    // Notify the host
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_HOST_INTSET, &ui32Arg);
#if SLINT_GPIO
    am_hal_gpio_state_write(HANDSHAKE_IOS_PIN,AM_HAL_GPIO_OUTPUT_SET);
    am_hal_gpio_state_write(HANDSHAKE_IOS_PIN,AM_HAL_GPIO_OUTPUT_CLEAR);
#endif
    while (1)
    {
        // Disable interrupts before checking the flags
        uint32_t ui32IntStatus = am_hal_interrupt_master_disable();
        if (bHandshake == true)
        {
            // re-enable interrupts
            am_hal_interrupt_master_set(ui32IntStatus);
            bHandshake = false;
            break;
        }
        else
        {
            am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
            // re-enable interrupts
            am_hal_interrupt_master_set(ui32IntStatus);
        }
    }

    Transaction.ui32InstrLen    = 1;
    Transaction.ui64Instr = sizeof(stHandshake);
    Transaction.eDirection      = AM_HAL_IOM_RX;
    if (g_sIosWidget.bSpi)
    {
        Transaction.uPeerInfo.ui32SpiChipSelect = AM_HAL_GPIO_NCE_IOM0CE0 + 4 * g_sIosWidget.iom;
    }
    else
    {
        Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
    }
    Transaction.ui32NumBytes    = sizeof(stHandshake);
    Transaction.pui32RxBuffer   = (uint32_t*)&stHandshake;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);

    if(stHandshake.ui16Tag == AM_DMATEST_CMD_ACK_DATA)
    {
        am_hal_cachectrl_range_t sRange;
        sRange.ui32StartAddr = (uint32_t)g_pIosSendBuf;
        sRange.ui32Size = DMA_PACK_SIZE;
        am_hal_cachectrl_dcache_clean(&sRange);
        ios_dma_send(pIosSpiWidget->testCfg.size);
        iom_recv_continue(0x7F, stHandshake.ui16Size);
    }
    else
    {
        return 1;
    }
#else
    am_hal_cachectrl_range_t sRange;
    sRange.ui32StartAddr = (uint32_t)g_pIosSendBuf;
    sRange.ui32Size = DMA_PACK_SIZE;
    am_hal_cachectrl_dcache_clean(&sRange);
    ios_dma_send(pIosSpiWidget->testCfg.size);
    iom_recv_continue(0x7F, pIosSpiWidget->testCfg.size);
#endif

    while (1)
    {
        // Disable interrupts before checking the flags
        uint32_t ui32IntStatus = am_hal_interrupt_master_disable();
        if ((bIomRecvComplete) && (bDMACMP))
        {
            uint32_t data = 0;
            // re-enable interrupts
            am_hal_interrupt_master_set(ui32IntStatus);
            // Clear the IOINT status
            data = 0xFF;
            Transaction.ui32InstrLen    = 1;
            Transaction.eDirection      = AM_HAL_IOM_TX;
            if (g_sIosWidget.bSpi)
            {
                Transaction.ui64Instr = 0xFA;
                Transaction.uPeerInfo.ui32SpiChipSelect = AM_HAL_GPIO_NCE_IOM0CE0 + 4 * g_sIosWidget.iom;
            }
            else
            {
                Transaction.ui64Instr = 0x7A;
                Transaction.uPeerInfo.ui32I2CDevAddr = g_sIosWidget.i2cAddr;
            }
            Transaction.ui32NumBytes    = 1;
            Transaction.pui32TxBuffer   = &data;
            Transaction.bContinue       = false;
            Transaction.ui8RepeatCount  = 0;
            Transaction.ui32PauseCondition = 0;
            Transaction.ui32StatusSetClr = 0;
            am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
            break;
        }
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
        // re-enable interrupts
        am_hal_interrupt_master_set(ui32IntStatus);
    }

    // Verify received data at IOM
    for(uint32_t i = 0; i < pIosSpiWidget->testCfg.size; i++)
    {
        if(g_pIomRecvBuf[i] != g_pIosSendBuf[i])
        {
            am_util_stdio_printf("Write TEST: Buffer Validation failed @i=%d Rcvd 0x%x Expected 0x%x\n",
                i, g_pIomRecvBuf[i], g_pIosSendBuf[i]);
            return 1;
        }
    }
#endif
    return 0;
}
#endif

